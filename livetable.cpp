/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include <QLocale>
#include <QHeaderView>
#include <QInputDialog>

#include "livetable.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"
#include "crsettings.hpp"

LiveTable::LiveTable(QWidget *parent) :
    QWidget(parent),
    demoModeTimer(this)
{
    ui->setupUi(this);

#ifdef Q_OS_WIN
    this->setWindowState(Qt::WindowFullScreen);
#endif

    // Make our window without panels
    this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    this->setFocusPolicy(Qt::NoFocus);

    highProxyModel->setSourceModel(model.data());
    this->ui->highTable->setModel(highProxyModel.data());
    lowProxyModel->setSourceModel(model.data());
    this->ui->lowTable->setModel(lowProxyModel.data());
    model->setSortRole(Qt::UserRole);

    highProxyModel->setMinRow(0);
    highProxyModel->setMaxRow(2);
    lowProxyModel->setMinRow(3);

    this->ui->highTable->setSortingEnabled(false);
    this->ui->highTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    this->ui->highTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    this->ui->highTable->verticalHeader()->setVisible(true);
    this->ui->lowTable->setSortingEnabled(false);
    this->ui->lowTable->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
    this->ui->lowTable->verticalHeader()->setVisible(true);
}

void LiveTable::setRaceInfo(const ChronoRaceData *raceData)
{
    if (raceData) {
        this->title = raceData->getField(ChronoRaceData::StringField::EVENT);
        this->place = raceData->getField(ChronoRaceData::StringField::PLACE);
        this->date = raceData->getDate();

        this->leftLogo = raceData->getField(ChronoRaceData::LogoField::LEFT);
        this->rightLogo = raceData->getField(ChronoRaceData::LogoField::RIGHT);
    } else {
        this->title = QStringLiteral("LBChronoRace");
        this->place = QString();
        this->date = QDate::currentDate();

        this->leftLogo = QPixmap(":/images/lbchronorace.png");
        this->rightLogo = QPixmap(":/images/lbchronorace.png");
    }

    this->ui->raceTitle->setText(this->title);
    setSubtitle();
    setLogos();
}

void LiveTable::addEntry(quint64 values)
{
    setEntry(values, true);
}

void LiveTable::removeEntry(quint64 values)
{
    setEntry(values, false);
}

void LiveTable::setStartList(QList<Competitor> const &newStartList)
{
    uint bib = 0;

    auto rowCount = model->rowCount();
    auto columnCount = model->columnCount();
    model->removeRows(0, rowCount);

    this->startList.clear();
    this->lastRowItems.clear();
    for (auto &competitor : newStartList) {
        bib = competitor.getBib();
        this->startList.insert(bib, competitor);
    }

    auto values = this->startList.values(bib);
    auto count = static_cast<int>(values.count());
    if (count <= 0) {
        throw(ChronoRaceException(tr("Enter competitors to use the Live Rankings")));
    }

    if (int columns = (2 * count) + ((count == 1) ? 1 : 2); columnCount > columns)
        model->removeColumns(columns, columnCount - columns);
    else if (columnCount < columns)
        model->insertColumns(columnCount, columns - columnCount);

    if (count == 1) {
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::INDIVIDUAL : LiveMode::CHRONO;
    } else {
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::RELAY : LiveMode::CHRONO_RELAY;
    }

    if (demoModeTimer.isActive()) {
        demoModeTimer.stop();
        QObject::disconnect(&demoModeTimer, &QTimer::timeout, this, &LiveTable::demoStep);
    }

    //NOSONAR TODO do the same for MAC and Linux
#ifdef Q_OS_WIN
    if (liveScreen == Q_NULLPTR)
        this->execState = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
#endif

    if (liveScreen != Q_NULLPTR)
        resizeColumns();
}

void LiveTable::reset() const
{
    model->clear();
}

QScreen const *LiveTable::getLiveScreen() const
{
    return liveScreen;
}

void LiveTable::setLiveScreen(QScreen const *screen)
{
    this->liveScreen = screen;

    //NOSONAR TODO do the same for MAC and Linux
#ifdef Q_OS_WIN
    if (this->liveScreen != Q_NULLPTR) {
        if (this->execState == static_cast<EXECUTION_STATE>(NULL))
            this->execState = SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);
    } else {
        this->execState = SetThreadExecutionState(ES_CONTINUOUS | this->execState);
        this->execState = static_cast<EXECUTION_STATE>(NULL);
    }
#endif
}

void LiveTable::setEntry(quint64 values, bool add)
{
    auto bib = static_cast<uint>(values & Q_UINT64_C(0xffffffff));
    auto timing = static_cast<uint>(values >> 32);

    if ((bib == 0) || (timing == 0))
        return;

    if (!this->startList.contains(bib))
        return;

    // Restore highlighted entry roles
    highlightLastEntry(false);

    switch (this->mode) {
        using enum LiveTable::LiveMode;

    case INDIVIDUAL:
        if (add)
            addTimingIndividual(bib, timing, false);
        else
            removeTimingIndividual(bib, timing, false);
        break;
    case CHRONO:
        if (add)
            addTimingIndividual(bib, timing, true);
        else
            removeTimingIndividual(bib, timing, true);
        break;
    case RELAY:
        if (add)
            addTimingRelay(bib, timing, false);
        else
            removeTimingRelay(bib, timing, false);
        break;
    case CHRONO_RELAY:
        if (add)
            addTimingRelay(bib, timing, true);
        else
            removeTimingRelay(bib, timing, true);
        break;
    }

    // Set highlighted entry roles
    highlightLastEntry(true);

    model->sort(0, Qt::SortOrder::AscendingOrder);

    if (!this->lastRowItems.isEmpty()) {
        auto const &index = lowProxyModel->mapFromSource(model->indexFromItem(this->lastRowItems[0]));
        auto row = index.isValid() ? index.row() : -1;

        if (row < 3)
            this->ui->lowTable->scrollToTop();
        else if (row + 1 == lowProxyModel->rowCount())
            this->ui->lowTable->scrollToBottom();
        else
            this->ui->lowTable->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void LiveTable::highlightLastEntry(bool set)
{
    if (set) {
        QColor highlightedColor(Qt::red);

        // Set bold font roles
        for (QStandardItem *item : std::as_const(this->lastRowItems)) {
            //NOSONAR highlightedFont = QFont(item->font());
            //NOSONAR highlightedFont.setBold(true);
            //NOSONAR item->setData(highlightedFont, Qt::FontRole);
            item->setData(highlightedColor, Qt::ForegroundRole);
        }
    } else {
        //NOSONAR QVariant fontRole;
        QVariant colorRole;

        // Restore non-bold font roles
        for (QStandardItem *item : std::as_const(this->lastRowItems)) {
            //NOSONAR fontRole = item->data(Qt::FontRole);
            //NOSONAR fontRole.clear();
            //NOSONAR item->setData(fontRole, Qt::FontRole);
            colorRole = item->data(Qt::ForegroundRole);
            colorRole.clear();
            item->setData(colorRole, Qt::ForegroundRole);
        }
    }
}

void LiveTable::addTimingIndividual(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Adding " << (chrono ? "chrono " : "individual ") << bib << " - " << timing;
#endif

    this->lastRowItems.clear();
    if (auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly); !indexes.empty()) {
        this->lastRowItems = model->takeRow(indexes.first().row());
    } else {
        Competitor const &comp = this->startList.value(bib);

        this->lastRowItems.reserve(3);

        this->lastRowItems << new QStandardItem(QString::number(bib));
        this->lastRowItems << new QStandardItem(comp.getCompetitorName(CRHelper::nameComposition));
        this->lastRowItems << new QStandardItem(QString());

        this->lastRowItems[1]->setData(QVariant(1000 * comp.getOffset()), Qt::UserRole);
    }

    if (chrono)
        timing -= this->lastRowItems[1]->data(Qt::UserRole).toUInt();

    if (this->lastRowItems[0]->data(Qt::UserRole).isNull()) {
        /* Timing not present for this bib */
        this->lastRowItems[0]->setData(QVariant(timing), Qt::UserRole);
    } else {
        QList<QVariant> extraTimings;
        if (!this->lastRowItems[2]->data(Qt::UserRole).isNull())
            extraTimings = this->lastRowItems[2]->data(Qt::UserRole).toList();

        if (uint prevTiming = this->lastRowItems[0]->data(Qt::UserRole).toUInt(); timing < prevTiming) {
            /* Higher timing already present for this bib */
            LiveTable::pushTiming(extraTimings, prevTiming); // push the previous timing in the list
            this->lastRowItems[0]->setData(QVariant(timing), Qt::UserRole); // store new timing
        } else {
            /* Lower timing already present for this bib */
            LiveTable::pushTiming(extraTimings, timing); // push the new timing in the extra list
            timing = prevTiming; // keep the previous timing in the user interface
        }

        this->lastRowItems[2]->setData(QVariant(extraTimings), Qt::UserRole);
    }

    this->lastRowItems[2]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));

    model->appendRow(this->lastRowItems);
}

void LiveTable::removeTimingIndividual(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing " << (chrono ? "chrono " : "individual ") << bib << " - " << timing;
#endif

    auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly);

    if (indexes.empty()) {
        /* This should never happen... in any case do nothing else */
        return;
    }

    QList<QVariant> extraTimings;

    this->lastRowItems.clear();
    this->lastRowItems = model->takeRow(indexes.first().row());

    if (!this->lastRowItems[2]->data(Qt::UserRole).isNull())
        extraTimings = this->lastRowItems[2]->data(Qt::UserRole).toList();

    if (chrono)
        timing -= this->lastRowItems[1]->data(Qt::UserRole).toUInt();

    /* A timing MUST be present for this bib */
    if (this->lastRowItems[0]->data(Qt::UserRole).toUInt() == timing) {
        /* Timing match found; remove it from the item */
        this->lastRowItems[0]->setData(QVariant(), Qt::UserRole);
        timing = 0;
    }

    if (!extraTimings.isEmpty()) {
        if (timing == 0) {
            /* The timing has been removed from the item, try to
             * replace it with the first extra timing available */
            timing = extraTimings.takeAt(0).toUInt();
        } else if (LiveTable::popTiming(extraTimings, timing)) {
            /* The timing has NOT been removed from the item, but
             * it has been removed from the extra timings */
            timing = 0;
        } else {
            /* The timing has NOT been removed from the item, but
             * it is NOT present among the extra timings; this
             * should never happen... in any case do nothing else */
            timing = 0;
        }

        /* Push the extra timings back */
        if (extraTimings.isEmpty())
            this->lastRowItems[2]->setData(QVariant(), Qt::UserRole);
        else
            this->lastRowItems[2]->setData(QVariant(extraTimings), Qt::UserRole);
    }

    auto nullItem = this->lastRowItems[0]->data(Qt::UserRole).isNull();
    if (!nullItem || (timing != 0)) {
        if (nullItem) {
            /* A timing has been extracted from the extra
             * timings; set it as the displayed timing */
            this->lastRowItems[0]->setData(QVariant(timing), Qt::UserRole);
            this->lastRowItems[2]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
        }

        /* A timing is still present in the item */
        model->appendRow(this->lastRowItems);
    }
}

void LiveTable::addTimingRelay(uint bib, uint timing, bool chrono)
{
    int column;
    auto columnCount = model->columnCount();

#ifdef QT_QML_DEBUG
    qDebug() << "Adding " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif

    QList<QVariant> extraTimings;

    this->lastRowItems.clear();
    if (auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly); !indexes.empty()) {
        this->lastRowItems = model->takeRow(indexes.first().row());
    } else {
        QList<Competitor> competitors(this->startList.values(bib));
        QMap<int, Competitor const *> offsets;

        for (auto const &comp : std::as_const(competitors))
            offsets.insert(chrono ? (1000 * comp.getOffset()) : -comp.getOffset(), &comp);

        this->lastRowItems.reserve(columnCount);

        this->lastRowItems << new QStandardItem(QString::number(bib));

        column = 1;
        for (auto item = offsets.constBegin(); item != offsets.constEnd(); item++) {
            if (column + 2 < columnCount) {
                this->lastRowItems << new QStandardItem(item.value()->getCompetitorName(CRHelper::nameComposition));
                this->lastRowItems << new QStandardItem(QString());

                this->lastRowItems[column]->setData(QVariant(item.key()), Qt::UserRole);
            }
            column += 2;
        }

        this->lastRowItems << new QStandardItem(QString());
    }

    if (!this->lastRowItems[columnCount - 1]->data(Qt::UserRole).isNull()) {
        extraTimings = this->lastRowItems[columnCount - 1]->data(Qt::UserRole).toList();
    }

    insertTimingRelay(timing, chrono, extraTimings);

    updateTimingRelay(chrono);

    /* Push the extra timings back */
    if (extraTimings.isEmpty())
        this->lastRowItems[columnCount - 1]->setData(QVariant(), Qt::UserRole);
    else
        this->lastRowItems[columnCount - 1]->setData(QVariant(extraTimings), Qt::UserRole);

    model->appendRow(this->lastRowItems);
}

void LiveTable::removeTimingRelay(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif

    auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly);

    if (indexes.empty()) {
        /* This should never happen... in any case do nothing else */
        return;
    }

    auto columnCount = model->columnCount();
    QList<QVariant> extraTimings;

    this->lastRowItems.clear();
    this->lastRowItems = model->takeRow(indexes.first().row());

    if (!this->lastRowItems[columnCount - 1]->data(Qt::UserRole).isNull()) {
        extraTimings = this->lastRowItems[columnCount - 1]->data(Qt::UserRole).toList();
        if (LiveTable::popTiming(extraTimings, timing)) {
            /* The timing is present in (and removed from) the extra timings */
            timing = 0;
        }
    }

    if (timing != 0) {
        eraseTimingRelay(timing, chrono, extraTimings);
    }

    if (this->lastRowItems[2]->data(Qt::UserRole).isNull()) {
        this->lastRowItems.clear();
    } else {
        updateTimingRelay(chrono);

        /* Push the extra timings back */
        if (extraTimings.isEmpty())
            this->lastRowItems[columnCount - 1]->setData(QVariant(), Qt::UserRole);
        else
            this->lastRowItems[columnCount - 1]->setData(QVariant(extraTimings), Qt::UserRole);

        model->appendRow(this->lastRowItems);
    }
}

void LiveTable::insertTimingRelay(uint timing, bool chrono, QList<QVariant> &extraTimings) const
{
    auto columnCount = model->columnCount();
    for (qsizetype column = 2; column < columnCount; column += 2) {
        if (this->lastRowItems[column]->data(Qt::UserRole).isNull()) {
            this->lastRowItems[column]->setData(QVariant(timing), Qt::UserRole);
            if (chrono) {
                timing -= this->lastRowItems[column - 1]->data(Qt::UserRole).toUInt();
            } else {
                timing -= (column == 2) ? 0 : this->lastRowItems[column - 2]->data(Qt::UserRole).toUInt();
            }
            this->lastRowItems[column]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
            timing = 0;
            break;
        } else if (uint prevTiming = this->lastRowItems[column]->data(Qt::UserRole).toUInt(); timing < prevTiming) {
            this->lastRowItems[column]->setData(QVariant(timing), Qt::UserRole);
            if (chrono) {
                timing -= this->lastRowItems[column - 1]->data(Qt::UserRole).toUInt();
            } else {
                timing -= (column == 2) ? 0 : this->lastRowItems[column - 2]->data(Qt::UserRole).toUInt();
            }
            this->lastRowItems[column]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
            timing = prevTiming;
        }
    }

    if (timing != 0) {
        LiveTable::pushTiming(extraTimings, timing); // push the timing in the extra list
    }
}

void LiveTable::eraseTimingRelay(uint timing, bool chrono, QList<QVariant> &extraTimings)
{
    int column;
    auto columnCount = model->columnCount();

    /* Find a matching item */
    for (column = 2; column < columnCount; column += 2) {
        if (this->lastRowItems[column]->data(Qt::UserRole).toUInt() == timing) {
            /* Timing match found */
            break;
        }
    }

    /* Shift the contents */
    while (column < columnCount) {
        if ((column + 2 < columnCount) && !this->lastRowItems[column + 2]->data(Qt::UserRole).isNull()) {
            timing = this->lastRowItems[column + 2]->data(Qt::UserRole).toUInt();
        } else if (!extraTimings.isEmpty()) {
            timing = extraTimings.takeAt(0).toUInt();
        } else {
            timing = 0;
        }

        if (timing != 0) {
            this->lastRowItems[column]->setData(QVariant(timing), Qt::UserRole);
            if (chrono) {
                timing -= this->lastRowItems[column - 1]->data(Qt::UserRole).toUInt();
            } else {
                timing -= (column == 2) ? 0 : this->lastRowItems[column - 2]->data(Qt::UserRole).toUInt();
            }
            this->lastRowItems[column]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
        } else {
            this->lastRowItems[column]->setData(QVariant(), Qt::UserRole);
            this->lastRowItems[column]->setText(QString());
        }

        column += 2;
    }
}

void LiveTable::updateTimingRelay(bool chrono) const
{
    auto columnCount = model->columnCount();

    uint timing = 0;
    auto sortTiming = Q_UINT64_C(0);

    for (qsizetype column = 2; column < columnCount; column += 2) {
        if (!this->lastRowItems[column]->data(Qt::UserRole).isNull()) {
            if (chrono)
                timing += (this->lastRowItems[column]->data(Qt::UserRole).toUInt() - this->lastRowItems[column - 1]->data(Qt::UserRole).toUInt());
            else
                timing = this->lastRowItems[column]->data(Qt::UserRole).toUInt();
            sortTiming = static_cast<qulonglong>(timing);
        } else {
            sortTiming += Q_UINT64_C(1) << (64 - (column / 2));
        }
    }

    this->lastRowItems[columnCount - 1]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
    this->lastRowItems[0]->setData(QVariant(sortTiming), Qt::UserRole);
}

void LiveTable::setSubtitle()
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    QString raceDate = QLocale::system().toString(this->date, (this->liveScreen->availableSize().width() < 1280) ? QLocale::FormatType::ShortFormat : QLocale::FormatType::LongFormat);
    QString subTitle = this->place % (this->place.isEmpty() ? "" : " - ") % raceDate;

    this->ui->raceInfo->setText(subTitle);
}

void LiveTable::setLogos()
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    int height = this->ui->titleLayout->geometry().height();
    int width;
    int lWidth;
    int rWidth;

    /* Prepare left logo */
    if (this->leftLogo.isNull()) {
        this->leftLogo = QPixmap(height, height);
        this->leftLogo.fill(Qt::transparent);
    } else {
        this->leftLogo = this->leftLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->leftLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    lWidth = this->leftLogo.width();

    /* Prepare right logo */
    if (this->rightLogo.isNull()) {
        this->rightLogo = QPixmap(height, height);
        this->rightLogo.fill(Qt::transparent);
    } else {
        this->rightLogo = this->rightLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->rightLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    rWidth = this->rightLogo.width();

    /* Set labels width */
    width = (lWidth > rWidth) ? lWidth : rWidth;
    this->ui->leftLogo->setFixedWidth(width);
    this->ui->rightLogo->setFixedWidth(width);

    /* Set the pixmaps */
    this->ui->leftLogo->setPixmap(this->leftLogo);
    this->ui->rightLogo->setPixmap(this->rightLogo);
}

void LiveTable::resizeColumns()
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    auto columns = model->columnCount();
    auto margins = this->ui->highTable->contentsMargins();
    auto fullWidth = this->liveScreen->availableSize().width();

    int count = ((columns + 1) / 2) - 1;

    if (count < 1)
        return;

    auto availableWidth = fullWidth - ((columns + 1) * (margins.left() + margins.right()));
    if (count == 1) {
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "long"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->highTable->setColumnWidth(0, availableWidth / 10);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(availableWidth / 30);
        this->ui->lowTable->setColumnWidth(0, availableWidth / 10);
        model->setHeaderData(2, Qt::Horizontal, tr("Timing", "long"), Qt::DisplayRole);
        this->ui->highTable->setColumnWidth(2, 3 * availableWidth / 20);
        this->ui->lowTable->setColumnWidth(2, 3 * availableWidth / 20);
        model->setHeaderData(1, Qt::Horizontal, tr("Competitor", "long"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Stretch);
    } else if (fullWidth < 1680) {
        auto bibWidth = availableWidth / 20;
        auto timeWidth = (availableWidth * 56) / 1000;
        auto sNameWidth = (availableWidth - ((3 * bibWidth / 2) + (timeWidth * (count + 1)))) / count;
        model->setHeaderData(0, Qt::Horizontal, tr("Bib", "short"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->highTable->setColumnWidth(0, bibWidth);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->lowTable->setColumnWidth(0, bibWidth);
        do {
            model->setHeaderData((2 * count) - 1, Qt::Horizontal, tr("Competitor %1", "short").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count) - 1, sNameWidth);
            this->ui->lowTable->setColumnWidth((2 * count) - 1, sNameWidth);
            model->setHeaderData((2 * count), Qt::Horizontal, tr("Timing %1", "short").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count), timeWidth);
            this->ui->lowTable->setColumnWidth((2 * count), timeWidth);
        } while (--count);
        model->setHeaderData(columns - 1, Qt::Horizontal, tr("Timing", "short"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
    } else {
        auto bibWidth = availableWidth / 20;
        auto timeWidth = (availableWidth * 56) / 1000;
        auto lNameWidth = (availableWidth - ((3 * bibWidth / 2) + (timeWidth * (count + 1)))) / count;
        model->setHeaderData(0, Qt::Horizontal, (fullWidth < 1920) ? tr("Bib", "short") : tr("Bib", "long"), Qt::DisplayRole);
        this->ui->highTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->highTable->setColumnWidth(0, bibWidth);
        this->ui->lowTable->verticalHeader()->setMinimumWidth(bibWidth / 2);
        this->ui->lowTable->setColumnWidth(0, bibWidth);
        do {
            model->setHeaderData((2 * count) - 1, Qt::Horizontal, tr("Competitor %1", "long").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count) - 1, lNameWidth);
            this->ui->lowTable->setColumnWidth((2 * count) - 1, lNameWidth);
            model->setHeaderData((2 * count), Qt::Horizontal, tr("Timing %1", "long").arg(count), Qt::DisplayRole);
            this->ui->highTable->setColumnWidth((2 * count), timeWidth);
            this->ui->lowTable->setColumnWidth((2 * count), timeWidth);
        } while (--count);
        model->setHeaderData(columns - 1, Qt::Horizontal, tr("Timing", "long"), Qt::DisplayRole);
        this->ui->highTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
        this->ui->lowTable->horizontalHeader()->setSectionResizeMode(columns - 1, QHeaderView::ResizeMode::Stretch);
    }

}

void LiveTable::pushTiming(QList<QVariant> &list, uint timing)
{
    QVariant value = QVariant::fromValue(timing);
    qsizetype index;

    /* Walk backwards to find the correct insertion point */
    for (index = list.isEmpty() ? 0 : list.size(); index > 0; index--) {
        if (timing >= list[index - 1].toUInt()) {
                break;
        }
    }

    /* Insert the value at the computed position */
    list.insert(index, value);
}

bool LiveTable::popTiming(QList<QVariant> &list, uint timing)
{
    bool retval = false;

    QList<QVariant>::ConstIterator it = list.constBegin();
    while (it != list.constEnd()) {

        /* Try to find a matching timing */
        if (it->toUInt() == timing) {
            list.erase(it);
            retval = true;
            break;
        }

        it++;
    }

    return retval;
}

void LiveTable::show()
{
    ui->retranslateUi(this);
    QWidget::show();
    ui->raceTitle->setText(this->title);
    setSubtitle();
    setLogos();
    resizeColumns();
}

void LiveTable::setMode(int code)
{
    Q_UNUSED(code)
    //0 --> rejected
    //1 --> accepted

    if (this->liveScreen == Q_NULLPTR) {
        //NOSONAR TODO do the same for MAC and Linux
#ifdef Q_OS_WIN
        SetThreadExecutionState(ES_CONTINUOUS | this->execState);
        this->execState = static_cast<EXECUTION_STATE>(NULL);
#endif
    }

    if (!demoModeTimer.isActive()) {

        // Restore highlighted entry roles
        highlightLastEntry(false);

        QObject::connect(&demoModeTimer, &QTimer::timeout, this, &LiveTable::demoStep);
        this->demoIndex = QModelIndex();
        demoModeTimer.start(CRSettings::getLiveScrollSeconds() * 1000);
    }
}

void LiveTable::demoStep()
{
    if (this->liveScreen == Q_NULLPTR)
        return;

    auto const &viewPortRect = this->ui->lowTable->viewport()->rect();
    while (this->demoIndex.isValid()) {
        this->demoIndex = lowProxyModel->index(demoIndex.row() + 1, 0);

        // element is not the bottom one but it is not visible
        if (this->demoIndex.isValid() && !viewPortRect.contains(this->ui->lowTable->visualRect(this->demoIndex)))
            break;
    }

    if (!this->demoIndex.isValid()) {
        this->demoIndex = lowProxyModel->index(0, 0);
        this->ui->lowTable->scrollToTop();
    } else {
        this->ui->lowTable->scrollTo(this->demoIndex, QAbstractItemView::ScrollHint::PositionAtTop);
    }
}

void LiveTable::setInterval() const
{
    bool ok = false;
    int i = QInputDialog::getInt(qobject_cast<QWidget *>(this->parent()), tr("Live Rankings"),
                                 tr("Rotation interval (sec):"), CRSettings::getLiveScrollSeconds(), 1, 60, 1, &ok);
    if (ok)
        CRSettings::setLiveScrollSeconds(i);
}
