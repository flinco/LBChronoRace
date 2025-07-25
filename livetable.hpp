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

#ifndef LIVETABLE_HPP
#define LIVETABLE_HPP

#include <QWidget>
#include <QStandardItemModel>
#include <QScreen>

#include "competitor.hpp"
#include "livetablefilterproxymodel.hpp"
#include "ui_livetable.h"

class LiveTable : public QWidget
{
    Q_OBJECT

private:
    enum class LiveMode
    {
        INDIVIDUAL,   // competitor + mass start
        CHRONO,       // competitor + timed start
        RELAY,        // team + mass start
        CHRONO_RELAY  // team + timed start
    };

public:
    explicit LiveTable(QWidget *parent = Q_NULLPTR);

    void setRaceInfo(ChronoRaceData const *raceData);
    void addEntry(quint64 values);
    void removeEntry(quint64 values);

    void setStartList(QList<Competitor> const &newStartList);
    void reset() const;

    QScreen const *getLiveScreen() const;
    void setLiveScreen(QScreen const *screen);

private:
    QMultiHash<uint, Competitor> startList { };
    QList<QStandardItem *> lastRowItems { };

    QScopedPointer<Ui::LiveTable> ui { new Ui::LiveTable };
    QScopedPointer<QStandardItemModel> model { new QStandardItemModel };
    QScopedPointer<LiveTableFilterProxyModel> lowProxyModel { new LiveTableFilterProxyModel };
    QScopedPointer<LiveTableFilterProxyModel> highProxyModel { new LiveTableFilterProxyModel };

    LiveMode mode { LiveMode::INDIVIDUAL };

    QScreen const *liveScreen { Q_NULLPTR };

    void setEntry(quint64 values, bool add);

    void addTimingIndividual(uint bib, uint timing, bool chrono);
    void removeTimingIndividual(uint bib, uint timing, bool chrono);

    void addTimingRelay(uint bib, uint timing, bool chrono);
    void removeTimingRelay(uint bib, uint timing, bool chrono);

    void insertTimingRelay(uint timing, bool chrono, QList<QVariant> &extraTimings) const;
    void eraseTimingRelay(uint timing, bool chrono, QList<QVariant> &extraTimings);
    void updateTimingRelay(bool chrono) const;

    static void pushTiming(QList<QVariant> &list, uint timing);
    static bool popTiming(QList<QVariant> &list, uint timing);
};

#endif // LIVETABLE_HPP
