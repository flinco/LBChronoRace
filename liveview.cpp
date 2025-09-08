/*****************************************************************************
 * Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include <QWindow>
#include <QInputDialog>
#include <QDate>
#include <QPixmap>
#include <QColor>
#include <QTime>

#include "liveview.hpp"
#include "crloader.hpp"
#include "crhelper.hpp"
#include "crsettings.hpp"

LiveView::LiveView(QWidget *startListParent, QWidget *rankingsParent) :
    QObject(Q_NULLPTR),
    demoModeTimer(this)
{
    liveStartList.reset(new LiveStartList(startListParent));
    liveRankings.reset(new LiveRankings(rankingsParent));

    this->setLiveMode(LiveView::LiveMode::NONE);
}

void LiveView::addEntry(quint64 values)
{
    setEntry(values, true);
}

void LiveView::removeEntry(quint64 values)
{
    setEntry(values, false);
}

void LiveView::setRaceInfo(ChronoRaceData const *newRaceData)
{
    this->raceData = newRaceData;
}

void LiveView::setStartList(QList<Competitor> const &newStartList)
{
    using enum LiveView::LiveMode;

    uint bib = 0;

    // Cleanup and fill with new data
    this->startList.clear();
    this->lastRowItems.clear();
    for (auto &competitor : newStartList) {
        bib = competitor.getBib();
        this->startList.insert(bib, competitor);
    }

    // Detect race mode
    if (auto legs = CRLoader::getStartListLegs(); legs == 0) {
        this->setLiveMode(LiveView::LiveMode::NONE);
    } else {
        using enum LiveView::RaceMode;

        auto values = this->startList.values(bib);
        auto count = static_cast<uint>(values.count());
        if (count == 1u) {
            this->raceMode = (values.at(0).getOffset() < 0) ? INDIVIDUAL : CHRONO;
        } else {
            this->raceMode = (values.at(0).getOffset() < 0) ? RELAY : CHRONO_RELAY;
        }
        this->liveStartList->setTimeColumns((this->raceMode == CHRONO) || (this->raceMode == CHRONO_RELAY));
        this->setLiveMode(LiveView::LiveMode::STARTLIST);

        prepareStartListModel(count);
        prepareRankingsModel(count);
    }

    updateView();
}

void LiveView::prepareStartListModel(uint legs)
{
    using enum LiveView::RaceMode;

    // Set columns for "startlist"
    int columns;
    auto *model = liveStartList->getModel();
    auto rowCount = model->rowCount();
    auto columnCount = model->columnCount();
    auto startTime = (this->raceData != Q_NULLPTR) ? this->raceData->getStartTime() : QTime(0, 0);

    model->removeRows(0, rowCount);
    switch (this->raceMode) {
        case INDIVIDUAL:
            [[fallthrough]];
        case RELAY:
            columns = 1 + (3 * legs);
            break;
        case CHRONO:
            [[fallthrough]];
        case CHRONO_RELAY:
            columns = 1 + (4 * legs);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
    if (columnCount > columns)
        model->removeColumns(columns, columnCount - columns);
    else if (columnCount < columns)
        model->insertColumns(columnCount, columns - columnCount);

    // Fill the model
    auto timeColumns = (this->raceMode == CHRONO) || (this->raceMode == CHRONO_RELAY);
    auto startTimeOffset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
    QList<Competitor> comp;
    auto const keys = this->startList.uniqueKeys();
    for (auto const &bibKey : keys) {

        this->lastRowItems.reserve(columns);
        this->lastRowItems << new QStandardItem(QString::number(bibKey));

        comp = this->startList.values(bibKey);
        for (auto item = comp.crbegin(); item != comp.crend(); item++) {
            this->lastRowItems << new QStandardItem(item->getCompetitorName(CRHelper::nameComposition));
            this->lastRowItems << new QStandardItem(CRHelper::toSexString(item->getSex()));
            this->lastRowItems << new QStandardItem(QString::number(item->getYear()));
            if (!timeColumns)
                continue;
            else if (auto offset = item->getOffset(); offset >= 0)
                this->lastRowItems << new QStandardItem(CRHelper::toOffsetString(startTimeOffset + offset));
            else
                this->lastRowItems << new QStandardItem(CRHelper::toOffsetString(startTimeOffset));
        }

        this->lastRowItems[0]->setData(QVariant(bibKey), Qt::UserRole);
        model->appendRow(this->lastRowItems);
        this->lastRowItems.clear();
    }
    model->sort(0, Qt::SortOrder::AscendingOrder);
}

void LiveView::prepareRankingsModel(uint legs)
{
    // Set columns for "timekeeper"
    auto *model = liveRankings->getModel();
    auto rowCount = model->rowCount();
    auto columnCount = model->columnCount();

    model->removeRows(0, rowCount);
    if (int columns = ((legs == 1) ? 1 : 2) + (2 * legs); columnCount > columns)
        model->removeColumns(columns, columnCount - columns);
    else if (columnCount < columns)
        model->insertColumns(columnCount, columns - columnCount);
}

QScreen const *LiveView::getLiveScreen() const
{
    return liveScreen;
}

void LiveView::setLiveScreen(QScreen const *screen)
{
    this->liveScreen = screen;
    liveStartList->setLiveScreen(screen);
    liveRankings->setLiveScreen(screen);
}

void LiveView::setLiveMode(LiveView::LiveMode newMode)
{
    switch (this->liveMode = newMode) {
        using enum LiveView::LiveMode;

        case NONE:
            stopDemo();
            break;
        case STARTLIST:
            startDemo();
            break;
        case TIMEKEEPER:
            stopDemo();
            break;
        case RANKINGS:
            highlightLastEntry(false); // remove highlighting
            startDemo();
            break;
        default:
            Q_UNREACHABLE();
            break;
    }

    updateView();
}

void LiveView::startDemo()
{
    if (demoModeTimer.isActive())
        return;

    QObject::connect(&demoModeTimer, &QTimer::timeout, liveStartList.data(), &LiveStartList::demoStep);
    QObject::connect(&demoModeTimer, &QTimer::timeout, liveRankings.data(), &LiveRankings::demoStep);
    demoModeTimer.start(CRSettings::getLiveScrollSeconds() * 1000);
}

void LiveView::stopDemo()
{
    if (!demoModeTimer.isActive())
        return;

    demoModeTimer.stop();
    QObject::disconnect(&demoModeTimer, &QTimer::timeout, liveRankings.data(), &LiveRankings::demoStep);
    QObject::disconnect(&demoModeTimer, &QTimer::timeout, liveStartList.data(), &LiveStartList::demoStep);
}

void LiveView::setEntry(quint64 values, bool add)
{
    auto bib = static_cast<uint>(values & Q_UINT64_C(0xffffffff));
    auto timing = static_cast<uint>(values >> 32);

    if ((bib == 0) || (timing == 0))
        return;

    if (!this->startList.contains(bib))
        return;

    // Restore highlighted entry roles
    highlightLastEntry(false);

    switch (this->raceMode) {
        using enum LiveView::RaceMode;

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

    auto *model = this->liveRankings->getModel();
    model->sort(0, Qt::SortOrder::AscendingOrder);
    this->liveRankings->scrollToLastItem(this->lastRowItems.isEmpty() ? Q_NULLPTR : this->lastRowItems[0]);
}

void LiveView::highlightLastEntry(bool set)
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

void LiveView::addTimingIndividual(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Adding " << (chrono ? "chrono " : "individual ") << bib << " - " << timing;
#endif
    auto *model = liveRankings->getModel();

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
        QVariantList extraTimings;
        if (!this->lastRowItems[2]->data(Qt::UserRole).isNull())
            extraTimings = this->lastRowItems[2]->data(Qt::UserRole).toList();

        if (uint prevTiming = this->lastRowItems[0]->data(Qt::UserRole).toUInt(); timing < prevTiming) {
            /* Higher timing already present for this bib */
            CRHelper::pushTiming(extraTimings, prevTiming); // push the previous timing in the list
            this->lastRowItems[0]->setData(QVariant(timing), Qt::UserRole); // store new timing
        } else {
            /* Lower timing already present for this bib */
            CRHelper::pushTiming(extraTimings, timing); // push the new timing in the extra list
            timing = prevTiming; // keep the previous timing in the user interface
        }

        this->lastRowItems[2]->setData(QVariant(extraTimings), Qt::UserRole);
    }

    this->lastRowItems[2]->setText(CRHelper::toTimeString(timing, ChronoRaceData::Accuracy::SECOND));

    model->appendRow(this->lastRowItems);
}

void LiveView::removeTimingIndividual(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing " << (chrono ? "chrono " : "individual ") << bib << " - " << timing;
#endif
    auto *model = liveRankings->getModel();

    auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly);

    if (indexes.empty()) {
        /* This should never happen... in any case do nothing else */
        return;
    }

    QVariantList extraTimings;

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
        } else if (CRHelper::popTiming(extraTimings, timing)) {
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

void LiveView::addTimingRelay(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Adding " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif
    auto *model = liveRankings->getModel();

    int column;
    auto columnCount = model->columnCount();

    QVariantList extraTimings;

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

    insertTimingRelay(timing, columnCount, chrono, extraTimings);

    updateTimingRelay(columnCount, chrono);

    /* Push the extra timings back */
    if (extraTimings.isEmpty())
        this->lastRowItems[columnCount - 1]->setData(QVariant(), Qt::UserRole);
    else
        this->lastRowItems[columnCount - 1]->setData(QVariant(extraTimings), Qt::UserRole);

    model->appendRow(this->lastRowItems);
}

void LiveView::removeTimingRelay(uint bib, uint timing, bool chrono)
{
#ifdef QT_QML_DEBUG
    qDebug() << "Removing " << (chrono ? "chrono relay " : "relay ") << bib << " - " << timing;
#endif
    auto *model = liveRankings->getModel();

    auto indexes = model->match(model->index(0, 0), Qt::DisplayRole, QString::number(bib), 1, Qt::MatchExactly);

    if (indexes.empty()) {
        /* This should never happen... in any case do nothing else */
        return;
    }

    auto columnCount = model->columnCount();
    QVariantList extraTimings;

    this->lastRowItems.clear();
    this->lastRowItems = model->takeRow(indexes.first().row());

    if (!this->lastRowItems[columnCount - 1]->data(Qt::UserRole).isNull()) {
        extraTimings = this->lastRowItems[columnCount - 1]->data(Qt::UserRole).toList();
        if (CRHelper::popTiming(extraTimings, timing)) {
            /* The timing is present in (and removed from) the extra timings */
            timing = 0;
        }
    }

    if (timing != 0) {
        eraseTimingRelay(timing, columnCount, chrono, extraTimings);
    }

    if (this->lastRowItems[2]->data(Qt::UserRole).isNull()) {
        this->lastRowItems.clear();
    } else {
        updateTimingRelay(columnCount, chrono);

        /* Push the extra timings back */
        if (extraTimings.isEmpty())
            this->lastRowItems[columnCount - 1]->setData(QVariant(), Qt::UserRole);
        else
            this->lastRowItems[columnCount - 1]->setData(QVariant(extraTimings), Qt::UserRole);

        model->appendRow(this->lastRowItems);
    }
}

void LiveView::insertTimingRelay(uint timing, int columnCount, bool chrono, QVariantList &extraTimings) const
{
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
        CRHelper::pushTiming(extraTimings, timing); // push the timing in the extra list
    }
}

void LiveView::eraseTimingRelay(uint timing, int columnCount, bool chrono, QVariantList &extraTimings)
{
    int column;

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

void LiveView::updateTimingRelay(int columnCount, bool chrono) const
{
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

void LiveView::updateView()
{
    if (liveScreen == Q_NULLPTR) {
        liveStartList->hide();
        liveRankings->hide();
    } else switch (this->liveMode) {
        using enum LiveView::LiveMode;

        case NONE:
            [[fallthrough]];
        case STARTLIST:
            liveStartList->activate();
            liveStartList->windowHandle()->setGeometry(liveScreen->geometry());
            updateHeaderData();
            liveStartList->resizeColumns();
            liveRankings->hide();
            break;
        case TIMEKEEPER:
            [[fallthrough]];
        case RANKINGS:
            liveRankings->activate();
            liveRankings->windowHandle()->setGeometry(liveScreen->geometry());
            updateHeaderData();
            liveRankings->resizeColumns();
            liveStartList->hide();
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

void LiveView::updateHeaderData()
{
    auto title = raceData->getField(ChronoRaceData::StringField::EVENT);
    if (title.isEmpty())
        title = QStringLiteral("LBChronoRace");

    auto place = raceData->getField(ChronoRaceData::StringField::PLACE);
    // it can be empty

    auto date = raceData->getDate();
    if (!date.isValid())
        date = QDate::currentDate();

    auto leftLogo = raceData->getField(ChronoRaceData::LogoField::LEFT);
    if (leftLogo.isNull())
        leftLogo = QPixmap(":/images/lbchronorace.png");

    auto rightLogo = raceData->getField(ChronoRaceData::LogoField::RIGHT);
    if (rightLogo.isNull())
        rightLogo = QPixmap(":/images/lbchronorace.png");

    switch (this->liveMode) {
        using enum LiveView::LiveMode;

        case NONE:
            [[fallthrough]];
        case STARTLIST:
            liveStartList->setHeaderData(title, place, date, leftLogo, rightLogo);
            break;
        case TIMEKEEPER:
            [[fallthrough]];
        case RANKINGS:
            liveRankings->setHeaderData(title, place, date, leftLogo, rightLogo);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

void LiveView::toggleStayOnTop(bool onTop)
{
    auto visible = this->liveRankings->isVisible();
    this->liveRankings->setWindowFlag(Qt::WindowStaysOnTopHint, onTop);
    if (visible)
        this->liveRankings->show();
}

void LiveView::toggleCompetitors(int code)
{
    Q_UNUSED(code)

    setStartList(CRLoader::getStartList());
}

void LiveView::toggleTimigns(int code)
{
    using enum LiveView::LiveMode;

    // code: 0 == close+reject; 1 == close+accept; other == open
    switch (code) {
        case static_cast<int>(QDialog::DialogCode::Accepted): // close timing recorder
            if (this->liveMode == TIMEKEEPER)
                this->setLiveMode(RANKINGS);
            break;
        case static_cast<int>(QDialog::DialogCode::Rejected): // close timing recorder
            if (this->liveMode == TIMEKEEPER)
                this->setLiveMode(STARTLIST);
            break;
        default: // open timing recorder
            if (this->liveMode == STARTLIST)
                this->setLiveMode(TIMEKEEPER);
            break;
    }

    if (this->screenSaver != Q_NULLPTR)
        this->screenSaver->inhibit(this->liveMode == TIMEKEEPER);
}

void LiveView::reloadStartList(QModelIndex const &topLeft, QModelIndex const &bottomRight, QList<int> const &roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)

    this->toggleCompetitors(0);
}

void LiveView::setInterval() const
{
    bool ok = false;
    int i = QInputDialog::getInt(qobject_cast<QWidget *>(this->parent()), tr("Live Rankings"),
                                 tr("Rotation interval (sec):"), CRSettings::getLiveScrollSeconds(), 1, 60, 1, &ok);
    if (ok)
        CRSettings::setLiveScrollSeconds(i);
}

void LiveView::setScreenSaver(ScreenSaver *newScreenSaver)
{
    screenSaver = newScreenSaver;
}
