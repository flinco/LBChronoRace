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

#ifndef RANKINGPRINTER_H
#define RANKINGPRINTER_H

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "crloader.h"
#include "chronoracedata.h"
#include "category.h"
#include "classentry.h"
#include "teamclassentry.h"

class RankingPrinter : public QObject
{
    Q_OBJECT
public:
    explicit RankingPrinter(uint indexFieldWidth, uint bibFieldWidth) : indexFieldWidth(indexFieldWidth), bibFieldWidth(bibFieldWidth) { };

    static QScopedPointer<RankingPrinter> getRankingPrinter(CRLoader::Format format, uint indexFieldWidth, uint bibFieldWidth);

    virtual void printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath) = 0;
    virtual void printRanking(Category const &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName) = 0;
    virtual void printRanking(Category const &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName) = 0;

    uint getIndexFieldWidth() const;
    uint getBibFieldWidth() const;

    ChronoRaceData const *getRaceInfo() const;
    void setRaceInfo(ChronoRaceData const *newRaceInfo);

protected:
    virtual QString &buildOutFileName(QString &outFileBaseName) = 0;
    virtual QString &checkOutFileNameExtension(QString &outFileBaseName) = 0;

private:
    uint indexFieldWidth { 0u };
    uint bibFieldWidth { 0u };

    ChronoRaceData const *raceInfo { Q_NULLPTR };

signals:
    void error(QString const &message);
    void info(QString const &message);
};

class Position
{
public:
    int getCurrentPositionNumber(int posIndex, QString const &currTime);
    QString getCurrentPositionString(int posIndex, QString const &currTime);
private:
    int position { 0 };
    QString time { };
};

class PositionNumber
{
public:
    uint getCurrentPosition(uint posIndex, uint currTime);
private:
    uint position { 0u };
    uint time { 0u };
};

#endif // RANKINGPRINTER_H
