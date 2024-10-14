/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#ifndef RANKINGSBUILDER_HPP
#define RANKINGSBUILDER_HPP

#include <QObject>
#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QString>

#include "classentry.hpp"
#include "teamclassentry.hpp"
#include "rankingprinter.hpp"

class RankingsBuilder : public QObject
{
    Q_OBJECT
    using QObject::QObject;

public:
    uint loadData();
    QList<ClassEntry const *> &fillRanking(QList<ClassEntry const *> &ranking, Ranking const *categories) const;
    QList<TeamClassEntry const *> &fillRanking(QList<TeamClassEntry const *> &ranking, Ranking const *categories);
    QList<Competitor const *> fillStartList() const;

private:
    void sortTeamRanking(QMap<QString, TeamClassEntry> &rankingByTeam, QList<TeamClassEntry *> &sortedTeamRanking) const;
    void sortLegTimes(QList<ClassEntry *> const &ranking, uint legs, PositionNumber &position) const;
    void sortLegTimes(TeamClassEntry const &teamClassEntry, uint legs, PositionNumber &position) const;
    void prepareStartList();
    void emitMessages(QStringList &messages);

    QMultiMap<uint, Competitor> startList { };
    QMap<uint, ClassEntry> rankingByBib { };
    QList<ClassEntry *> rankingByTime { };
    QList<QMap<QString, TeamClassEntry>> rankingsByTeam { };

signals:
    void error(QString const &);
};

#endif // RANKINGSBUILDER_HPP
