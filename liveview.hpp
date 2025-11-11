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

#ifndef LIVEVIEW_HPP
#define LIVEVIEW_HPP

#include <QObject>
#include <QScreen>
#include <QTimer>
#include <QList>
#include <QHash>
#include <QMultiMap>
#include <QMultiHash>
#include <QVariantList>
#include <QStandardItem>
#include <QModelIndex>
#include <QPalette>

#include "livestartlist.hpp"
#include "liverankings.hpp"
#include "chronoracedata.hpp"
#include "competitor.hpp"
#include "screensaver.hpp"

constexpr char DISPLAY_CHRONO_ZERO[] = "0:00:00";

namespace liveview {
class LiveView;
class Highlighter;
}

class Highlighter : public QObject
{
    Q_OBJECT

public:
    explicit Highlighter(QList<QStandardItem *> *highlightedItemsList);

    void clear();
    void add(Competitor::Sex sex, qsizetype leg, uint timing, QStandardItem *item);
    void remove(Competitor::Sex sex, qsizetype leg, uint timing, QStandardItem *item);

    void unhighlight();
    void highlight();

    void setPalette(QPalette const &newPalette);

public slots:
    void handleHighlightRequest(QStandardItem const *item);
    void handleHighlightRequest(QModelIndex const &parent, int first, int last);

private:
    QPalette palette;
    bool highlightPending { false };

    QList<QStandardItem *> *highlightedRowItems;
    QHash<Competitor::Sex, QList<QStandardItem *>> bestRowItems;
    QHash<Competitor::Sex, QList<QMultiMap<uint, QStandardItem *>>> bestItemsIndex;

    void highlightItem(QStandardItem *item, QColor const &bestColor) const;
    void unhighlightItem(QStandardItem *item) const;

signals:
    void postRenderNeeded();
};


class LiveView : public QObject
{
    Q_OBJECT

private:
    enum class RaceMode
    {
        INDIVIDUAL,   // competitor + mass start
        CHRONO,       // competitor + timed start
        RELAY,        // team + mass start
        CHRONO_RELAY  // team + timed start
    };

    enum class LiveMode
    {
        NONE,         // no data (only header)
        STARTLIST,    // show startlist
        TIMEKEEPER,   // build live rankings
        RANKINGS      // show rankings
    };

public:
    enum class LiveHighlighting {
        LiveNotHighlighted  = 0x00,
        LiveHighlightedLast = 0x01,
        LiveHighlightedBest = 0x02
    };
    Q_DECLARE_FLAGS(LiveHighlightings, LiveHighlighting)

    explicit LiveView(QWidget *startListParent, QWidget *rankingsParent);

    void addEntry(quint64 values, bool add = true);
    void removeEntry(quint64 values);

    void setRaceInfo(ChronoRaceData const *newRaceData);
    void setStartList(QList<Competitor> const &newStartList);

    QScreen const *getLiveScreen() const;
    void setLiveScreen(QScreen const *screen);

    void updateView();

    void setScreenSaver(ScreenSaver *newScreenSaver);

    void toggleStayOnTop(bool onTop);

public slots:
    void toggleCompetitors(int code);
    void toggleTimigns(int code);
    void setInterval() const;

    void reloadStartList(QModelIndex const &topLeft, QModelIndex const &bottomRight, QList<int> const &roles = QList<int>());

    void setTimerValue(QString const &value);

private:
    QMultiHash<uint, Competitor> startList { };
    QList<QStandardItem *> lastRowItems { };
    Highlighter highlighter;

    QScopedPointer<LiveStartList> liveStartList;
    QScopedPointer<LiveRankings> liveRankings;

    RaceMode raceMode { RaceMode::INDIVIDUAL };
    LiveMode liveMode { LiveMode::NONE };

    ChronoRaceData const *raceData { Q_NULLPTR };
    QScreen const *liveScreen { Q_NULLPTR };

    QTimer demoModeTimer;

    ScreenSaver *screenSaver { Q_NULLPTR };

    void setLiveMode(LiveView::LiveMode newMode);

    void startDemo();
    void stopDemo();

    void prepareStartListModel(uint legs);
    void prepareRankingsModel(uint legs);

    void addTimingIndividual(uint bib, uint timing, bool chrono);
    void removeTimingIndividual(uint bib, uint timing, bool chrono);

    void addTimingRelay(uint bib, uint timing, bool chrono);
    void removeTimingRelay(uint bib, uint timing, bool chrono);

    void insertTimingRelay(uint timing, int columnCount, bool chrono, QVariantList &extraTimings);
    void eraseTimingRelay(uint timing, int columnCount, bool chrono, QVariantList &extraTimings);
    void updateTimingRelay(int columnCount, bool chrono) const;

    void updateHeaderData();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LiveView::LiveHighlightings)

#endif // LIVEVIEW_HPP
