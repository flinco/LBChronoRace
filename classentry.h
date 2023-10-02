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

#ifndef CLASSENTRY_H
#define CLASSENTRY_H

#include <QCoreApplication>
#include <QVector>
#include <QString>

#include "competitor.h"
#include "timing.h"

namespace placement {
class ClassEntryElement;
class ClassEntry;
}

class ClassEntryElement {
public:
    Competitor const *competitor { Q_NULLPTR };
    uint              time { 0u };
    Timing::Status    status { Timing::Status::CLASSIFIED };
    uint              legRanking { 0u };

    QString formatNameCSV(bool first, QString const &name, QString const &sex, QString const &year) const;
    QString formatNameTxt(bool first, QString const &name, QString const &sex, QString const &year) const;
    void addNames(bool csvFormat, bool first, QString &entryString, QString const &emptyName) const;
};

class ClassEntry {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry)

private:
    static QString             empty;

    uint                       bib { 0u };
    QVector<ClassEntryElement> entries { };
    uint                       totalTime { 0u };
    QString                    category { "" };

    QString getNamesCommon(bool csvFormat) const;

public:
    explicit ClassEntry(uint const bib = 0u) : bib(bib) {}

    uint getBib() const;
    void setBib(uint newBib);
    QString getName(uint legIdx) const;
    QString getNamesCSV() const;
    QString getNamesTxt() const;
    uint getYear(uint legIdx) const;
    Competitor::Sex getSex() const;
    Competitor::Sex getSex(uint legIdx) const;
    QString getTimesCSV() const;
    QString getTimesTxt(int legRankWidth) const;
    QString getTime(uint legIdx) const;
    uint getTimeValue(uint legIdx) const;
    uint countEntries() const;
    void setTime(Competitor const *comp, Timing const &timing, QStringList &messages);
    uint getLegRanking(uint legIdx) const;
    void setLegRanking(uint legIdx, uint ranking);
    uint getFromYear() const;
    uint getToYear() const;
    QString const &getTeam() const;
    uint getTotalTime() const;
    QString getTotalTimeCSV() const;
    QString getTotalTimeTxt() const;
    QString getDiffTimeTxt(uint referenceTime) const;
    bool isDnf() const;
    bool isDns() const;

    QString const &getCategory() const;
    QString const &getCategory(uint legIdx) const;
    void setCategory(QString const &value);

    static bool compare(ClassEntry const &a, ClassEntry const &b);

    bool operator< (ClassEntry const &rhs) const;
    bool operator> (ClassEntry const &rhs) const;
    bool operator<=(ClassEntry const &rhs) const;
    bool operator>=(ClassEntry const &rhs) const;
};


#endif // CLASSENTRY_H
