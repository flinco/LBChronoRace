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

#include <QHeaderView>

#include "livetable.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

LiveTable::LiveTable(QWidget *parent) : QWidget(parent)
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
    int height;
    int width;
    int lWidth;
    int rWidth;

    QString title = "";
    QString racePlace = "";
    QString raceDate = "";

    QPixmap leftLogo;
    QPixmap rightLogo;

    if (raceData) {
        title = raceData->getEvent();
        racePlace = raceData->getPlace();
        raceDate = QLocale::system().toString(raceData->getDate(), "dddd dd/MM/yyyy");

        leftLogo = raceData->getLeftLogo();
        rightLogo = raceData->getRightLogo();
    }

    QString subTitle = racePlace % (racePlace.isEmpty() ? "" : " - ") % raceDate;

    this->ui->raceTitle->setText(title.isEmpty() ? tr("The Race") : title);
    this->ui->raceInfo->setText(subTitle.isEmpty() ? tr("LBChronoRace") : subTitle);

    height = this->ui->titleLayout->geometry().height();

    /* Prepare left logo */
    if (leftLogo.isNull()) {
        leftLogo = QPixmap(height, height);
        leftLogo.fill(Qt::transparent);
    } else {
        leftLogo = leftLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->leftLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    lWidth = leftLogo.width();

    /* Prepare right logo */
    if (rightLogo.isNull()) {
        rightLogo = QPixmap(height, height);
        rightLogo.fill(Qt::transparent);
    } else {
        rightLogo = rightLogo.scaledToHeight(height, Qt::SmoothTransformation);
        this->ui->rightLogo->setStyleSheet("QLabel { background-color : transparent; }");
    }
    rWidth = rightLogo.width();

    /* Set labels width */
    width = (lWidth > rWidth) ? lWidth : rWidth;
    this->ui->leftLogo->setFixedWidth(width);
    this->ui->rightLogo->setFixedWidth(width);

    /* Set the pixmaps */
    this->ui->leftLogo->setPixmap(leftLogo);
    this->ui->rightLogo->setPixmap(rightLogo);
}

void LiveTable::addEntry(quint64 values)
{
    auto bib = static_cast<uint>(values & Q_UINT64_C(0xffffffff));
    auto timing = static_cast<uint>(values >> 32);

    if ((bib == 0) || (timing == 0))
        return;

    if (!this->startList.contains(bib))
        return;

    QList oldItems(this->lastRowItems);
    //NOSONAR QVariant fontRole;
    //NOSONAR QFont boldFont;
    QVariant colorRole;
    QColor highlightedColor(Qt::red);

    switch (this->mode) {
        using enum LiveTable::LiveMode;

        case INDIVIDUAL:
            setTimingIndividual(bib, timing, false);
            break;
        case CHRONO:
            setTimingIndividual(bib, timing, true);
            break;
        case RELAY:
            setTimingRelay(bib, timing, false);
            break;
        case CHRONO_RELAY:
            setTimingRelay(bib, timing, true);
            break;
    }

    // Restore non-bold font roles
    for (QStandardItem *item : oldItems) {
        //NOSONAR fontRole = item->data(Qt::FontRole);
        //NOSONAR fontRole.clear();
        //NOSONAR item->setData(fontRole, Qt::FontRole);
        colorRole = item->data(Qt::ForegroundRole);
        colorRole.clear();
        item->setData(colorRole, Qt::ForegroundRole);
    }

    // Set bold font roles
    for (QStandardItem *item : std::as_const(this->lastRowItems)) {
        //NOSONAR highlightedFont = QFont(item->font());
        //NOSONAR highlightedFont.setBold(true);
        //NOSONAR item->setData(highlightedFont, Qt::FontRole);
        item->setData(highlightedColor, Qt::ForegroundRole);
    }

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

void LiveTable::removeEntry(quint64 values)
{
    auto bib = static_cast<uint>(values & Q_UINT64_C(0xffffffff));
    auto timing = static_cast<uint>(values >> 32);

    if ((bib == 0) || (timing == 0))
        return;

    if (!this->startList.contains(bib))
        return;

    switch (this->mode) {
        using enum LiveTable::LiveMode;

        case INDIVIDUAL:
            [[fallthrough]];
        case CHRONO:
            removeTimingIndividual(bib);
            break;
        case RELAY:
            removeTimingRelay(bib, timing, false);
            break;
        case CHRONO_RELAY:
            removeTimingRelay(bib, timing, true);
            break;
    }

    model->sort(0, Qt::SortOrder::AscendingOrder);
}

void LiveTable::setStartList(QList<Competitor> const &newStartList)
{
    uint bib = 0;

    auto rowCount = model->rowCount();
    auto columnCount = model->columnCount();
    model->removeRows(0, rowCount);

    this->startList.clear();
    for (auto &competitor : newStartList) {
        bib = competitor.getBib();
        this->startList.insert(bib, competitor);
    }

    auto values = this->startList.values(bib);
    auto count = static_cast<int>(values.count());

    int columns = (2 * count) + ((count == 1) ? 1 : 2);
    if (columnCount > columns)
        model->removeColumns(columns, columnCount - columns);
    else if (columnCount < columns)
        model->insertColumns(columnCount, columns - columnCount);

    if (liveScreen == Q_NULLPTR)
        throw(ChronoRaceException(tr("No screen available for the Live View")));

    auto margins = this->ui->highTable->contentsMargins();
    auto fullWidth = liveScreen->availableSize().width();
    auto availableWidth = fullWidth - ((columns + 1) * (margins.left() + margins.right()));
    if (count <= 0) {
        throw(ChronoRaceException(tr("Enter competitors to use the Live View")));
    } else if (count == 1) {
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::INDIVIDUAL : LiveMode::CHRONO;
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
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::RELAY : LiveMode::CHRONO_RELAY;
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
        this->mode = (values.at(0).getOffset() < 0) ? LiveMode::RELAY : LiveMode::CHRONO_RELAY;
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
}

void LiveTable::setTimingIndividual(uint bib, uint timing, bool chrono)
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

    this->lastRowItems[2]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));
    this->lastRowItems[0]->setData(QVariant(timing), Qt::UserRole);

    model->appendRow(this->lastRowItems);
}

void LiveTable::removeTimingIndividual(uint bib)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing row " << bib;
#endif

    if (auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly); !indexes.empty()) {

        auto row = indexes.first().row();

        QList<QStandardItem *> rowItems = model->takeRow(row);
        for (QList<QStandardItem *>::const_iterator newItem = rowItems.constBegin(); newItem != rowItems.constEnd(); newItem++) {
            if (this->lastRowItems.contains(*newItem)) {
                this->lastRowItems.clear();
                break;
            }
        }

        model->removeRow(row);
    }
}

void LiveTable::setTimingRelay(uint bib, uint timing, bool chrono)
{
    int column;
    auto columnCount = model->columnCount();

#ifdef QT_QML_DEBUG
    qDebug() << "Adding " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif

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

    insertTimingRelay(timing, chrono);

    updateTimingRelay(chrono);

    model->appendRow(this->lastRowItems);
}

void LiveTable::removeTimingRelay(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif

    if (auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly); !indexes.empty()) {

        int column;
        auto columnCount = model->columnCount();

        auto rowItems = model->takeRow(indexes.first().row());

        for (auto const *newItem : std::as_const(rowItems)) {
            if (this->lastRowItems.contains(newItem)) {
                this->lastRowItems.clear();
                break;
            }
        }

        // Find a matching item
        for (column = 2; column < columnCount; column += 2)
            if (!rowItems[column]->data(Qt::UserRole).isNull() &&
                (rowItems[column]->data(Qt::UserRole).toUInt() == timing))
                break;

        // Shift the contents
        while (column < columnCount) {
            rowItems[column]->setData((column + 2 < columnCount) ? rowItems[column + 2]->data(Qt::UserRole) : QVariant(), Qt::UserRole);
            column += 2;
        }

        if (rowItems[2]->data(Qt::UserRole).isNull()) {
            rowItems.clear();
        } else {
            if (this->lastRowItems.isEmpty())
                this->lastRowItems = rowItems;
            updateTimingRelay(chrono);
            model->appendRow(rowItems);
        }
    }
}

void LiveTable::insertTimingRelay(uint timing, bool chrono) const
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
