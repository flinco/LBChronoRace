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

#include "competitor.hpp"
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
    void addEntry(uint bib, uint timing) const;
    void removeEntry(uint bib, uint timing) const;

    void setStartList(QList<Competitor> const &newStartList);

private:
    QMultiHash<uint, Competitor> startList;

    QScopedPointer<Ui::LiveTable> ui { new Ui::LiveTable };

    LiveMode mode { LiveMode::INDIVIDUAL };
};

#endif // LIVETABLE_HPP
