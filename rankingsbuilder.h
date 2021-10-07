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

#ifndef RANKINGSBUILDER_H
#define RANKINGSBUILDER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>

#include "lbchronorace.h"
#include "classentry.h"
#include "teamclassentry.h"
#include "category.h"

class RankingsBuilder : public QObject
{
    Q_OBJECT
    using QObject::QObject;

public:
    uint loadData();
    QList<ClassEntry const *> &fillRanking(QList<ClassEntry const *> &ranking, Category const &category) const;
    QList<TeamClassEntry const *> &fillRanking(QList<TeamClassEntry const *> &ranking, Category const &category);

    static QList<Competitor> makeStartList();

private:
    void fillStartList();
    void setCompetitorCategory(QVector<Category> const &categories, Competitor *competitor) const;

    QMultiMap<uint, Competitor> startList { };
    QMap<uint, ClassEntry> rankingByBib { };
    QList<ClassEntry *> rankingByTime { };
    QMap<QString, TeamClassEntry> rankingByTeam { };

signals:
    void error(QString const &message);
};

#endif // RANKINGSBUILDER_H
