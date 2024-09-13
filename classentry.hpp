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

#include <QVector>
#include <QString>

#include "crloader.hpp"
#include "category.hpp"
#include "competitor.hpp"
#include "timing.hpp"

namespace placement {
    class ClassEntryElement;
    class ClassEntry;
    class ClassEntryHelper;
}

class ClassEntryElement {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry);

public:
    Competitor     *competitor { Q_NULLPTR };
    uint            time { 0u };
    Timing::Status  status { Timing::Status::CLASSIFIED };
    uint            legRanking { 0u };

    QString formatNameCSV(bool first, QString const &name, QString const &sex, QString const &year) const;
    QString formatNameTxt(bool first, QString const &name, QString const &sex, QString const &year) const;
    void addNames(bool csvFormat, bool first, QString &entryString, QString const &emptyName) const;

    bool hasCategory(Category const *cat) const;
};

class ClassEntry {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry)

    friend class ClassEntryHelper;

private:
    static QString             empty;

    uint                       bib { 0u };
    QVector<ClassEntryElement> entries { };
    uint                       totalTime { 0u };
    Category const            *category { Q_NULLPTR };

    QString getNamesCommon(bool csvFormat) const;

public:
    explicit ClassEntry(uint const bib = 0u) : bib(bib) {}

    uint getBib() const;
    void setBib(uint newBib);
    QString getName(uint legIdx) const;
    QString getNames(CRLoader::Format format) const;
    uint getYear(uint legIdx) const;
    Competitor::Sex getSex() const;
    Competitor::Sex getSex(uint legIdx) const;
    QString getTimes(CRLoader::Format format, int legRankWidth = 0) const;
    QString getTime(uint legIdx) const;
    uint getTimeValue(uint legIdx) const;
    uint countEntries() const;
    void setTime(Competitor *comp, Timing const &timing, QStringList &messages);
    uint getLegRanking(uint legIdx) const;
    void setLegRanking(uint legIdx, uint ranking);
    uint getFromYear() const;
    uint getToYear() const;
    QString getClub() const;
    QString getTeam() const;
    QString getClubsAndTeam() const;
    uint getTotalTime() const;
    QString getTotalTime(CRLoader::Format format) const;
    QString getDiffTimeTxt(uint referenceTime) const;
    bool isDnf() const;
    bool isDns() const;
    bool isDsq() const;

    Category const *getCategory() const;
    Category const *getCategory(uint legIdx) const;
    QStringList setCategory();

    bool operator< (ClassEntry const &rhs) const;
    bool operator> (ClassEntry const &rhs) const;
    bool operator<=(ClassEntry const &rhs) const;
    bool operator>=(ClassEntry const &rhs) const;
};

class ClassEntryHelper {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry)

private:
    static bool allCompetitorsShareTheSameClub(QVector<ClassEntryElement> const &entries, qsizetype fromLeg, qsizetype toLeg, QString const &club);
    static bool allCompetitorsAreOfTheSameSex(QVector<ClassEntryElement> const &entries, qsizetype fromLeg, qsizetype toLeg, Competitor::Sex sex);
    static void removeImpossibleCategories(QVector<ClassEntryElement> &entries);
    static void removeLowerWeigthCategories(QList<Category const *> &categories, QString const &name, uint bib);

public:
    static void setCategorySingleLeg(ClassEntry *entry, QStringList &messages);
    static void setCategoryMultiLeg(ClassEntry *entry, QStringList &messages);
};

#endif // CLASSENTRY_H
