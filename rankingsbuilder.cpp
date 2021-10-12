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

#include <QMultiMap>

#include "crloader.h"
#include "rankingsbuilder.h"
#include "rankingprinter.h"

#include "lbcrexception.h"

uint RankingsBuilder::loadData()
{
    QStringList messages;
    QVector<Timing> timings = CRLoader::getTimings();
    QVector<Category> categories = CRLoader::getCategories();

    uint bib;
    uint leg;

    // reset previous data (if any)
    rankingByBib.clear();
    startList.clear();

    // reset and fill the start list
    fillStartList();

    // compute individual general classifications (all included, sorted by bib)
    for (auto timing : timings) {

        bib = timing.getBib();
        leg = timing.getLeg();
        auto [firstComp, lastComp] = startList.equal_range(bib);

        if (firstComp == lastComp) {
            // should never happen
            emit error(tr("Competitor not found for bib %1").arg(bib));
            continue;
        }

        auto classEntryIt = rankingByBib.find(bib);
        if (leg == 0) { // perform leg auto detection only if no manual leg hint is present
            leg = (classEntryIt != rankingByBib.end()) ? (classEntryIt.value().countEntries() + 1) : 1;
            CRLoader::setStartListLegs(leg);
        }

        Competitor *comp = Q_NULLPTR;
        for (auto compIt = firstComp; compIt != lastComp; compIt++) {
            if (compIt.value().getLeg() == leg) {
                comp = &compIt.value();
                break;
            }
        }
        if (!comp) {
            emit error(tr("Bib %1 not inserted in results; check for possible duplicated entries").arg(bib));
            continue;
        } else {
            // Set the category for the competitor (if any)
            setCompetitorCategory(categories, comp);
        }

        if (classEntryIt != rankingByBib.end()) {
            classEntryIt.value().setTime(comp, timing, messages);
        } else {
            rankingByBib.insert(bib, ClassEntry(bib)).value().setTime(comp, timing, messages);
        }
    }

    for (auto const &message : messages)
        emit error(tr("Warning: %1").arg(message));
    messages.clear();

    if (startList.size() != timings.size())
        emit error(tr("Warning: the number of timings (%1) is not match the expected (%2); check for possible missing or duplicated entries").arg(timings.size()).arg(startList.size()));

    // sort by time
    QList<ClassEntry *>::const_iterator c;
    for (auto classEntry = rankingByBib.begin(); classEntry != rankingByBib.end(); classEntry++) {
        c = rankingByTime.constBegin();
        while ((c != rankingByTime.constEnd()) && (*(*c) < classEntry.value()))
            ++c;
        rankingByTime.insert(c, &classEntry.value());
    }

    return static_cast<uint>(rankingByBib.size());
}

QList<ClassEntry const *> &RankingsBuilder::fillRanking(QList<ClassEntry const *> &ranking, Category const &category) const
{
    Q_ASSERT(!category.isTeam());

    QList<ClassEntry *> tmpRanking;
    for (auto classEntry : rankingByTime) {

        // Sex
        if ((category.getSex() != Competitor::Sex::UNDEFINED) &&
            (category.getSex() != classEntry->getSex())) {
            continue;
        }

        // To Year
        if (category.getToYear() &&
            (category.getToYear() < classEntry->getToYear())) {
            continue;
        }

        // From Year
        if (category.getFromYear() &&
            (category.getFromYear() > classEntry->getFromYear())) {
            continue;
        }

        classEntry->setCategory(category.getFullDescription());

        tmpRanking.append(classEntry);
    }

    // do the sorting of the single leg times
    uint i;
    PositionNumber position;
    for (uint legIdx = 0u; legIdx < CRLoader::getStartListLegs(); legIdx++) {
        QMultiMap<uint, ClassEntry *> sortedLegClassification;
        for (auto classEntry : tmpRanking) {
            sortedLegClassification.insert(classEntry->getTimeValue(legIdx), classEntry);
        }
        i = 0;
        for (auto classEntry : sortedLegClassification) {
            i++;
            classEntry->setLegRanking(legIdx, position.getCurrentPosition(i, classEntry->getTimeValue(legIdx)));
        }
    }

    ranking.clear();
    ranking.reserve(tmpRanking.size());
    for (auto const classEntry : tmpRanking) {
        ranking.append(classEntry);
    }

    return ranking;
}

QList<TeamClassEntry const *> &RankingsBuilder::fillRanking(QList<TeamClassEntry const *> &ranking, Category const &category)
{
    Q_ASSERT(category.isTeam());

    // reset previous data (if any)
    rankingByTeam.clear();

    for (auto classEntry : rankingByTime) {

        // exclude DNS and DNF
        if (classEntry->isDns() || classEntry->isDnf()) {
            continue;
        }

        // exclude competitors without team
        if (classEntry->getTeam().isEmpty()) {
            continue;
        }

        // Sex
        if ((category.getSex() != Competitor::Sex::UNDEFINED) &&
            (category.getSex() != classEntry->getSex())) {
            continue;
        }

        // To Year
        if (category.getToYear() &&
            (category.getToYear() < classEntry->getToYear())) {
            continue;
        }

        // From Year
        if (category.getFromYear() &&
            (category.getFromYear() > classEntry->getFromYear())) {
            continue;
        }

        QString const &team = classEntry->getTeam();

        QMap<QString, TeamClassEntry>::iterator const teamRankingIt = rankingByTeam.find(team);
        if (teamRankingIt == rankingByTeam.end()) {
            rankingByTeam.insert(team, TeamClassEntry()).value().setClassEntry(classEntry);
        } else {
            teamRankingIt.value().setClassEntry(classEntry);
        }
    }

    // sort the team rankings
    QList<TeamClassEntry *> sortedTeamRanking;
    QList<TeamClassEntry *>::const_iterator t;
    for (auto teamClassEntry = rankingByTeam.begin(); teamClassEntry != rankingByTeam.end(); teamClassEntry++) {
        t = sortedTeamRanking.constBegin();
        while ((t != sortedTeamRanking.constEnd()) && (*(*t) < teamClassEntry.value()))
            ++t;
        sortedTeamRanking.insert(t, &teamClassEntry.value());
    }

    // copy and return the team rankings
    ranking.clear();
    ranking.reserve(sortedTeamRanking.size());
    for (auto const teamClassEntry : sortedTeamRanking) {
        ranking.append(teamClassEntry);
    }

    return ranking;
}

QList<Competitor> RankingsBuilder::makeStartList()
{
    // compute and sort the start list
    QList<Competitor> sortedStartList = CRLoader::getStartList();
    CompetitorSorter::setSortingField(Competitor::Field::CMF_BIB);
    CompetitorSorter::setSortingOrder(Qt::AscendingOrder);
    std::stable_sort(sortedStartList.begin(), sortedStartList.end(), CompetitorSorter());

    return sortedStartList;
}

void RankingsBuilder::fillStartList()
{
    uint bib;
    uint mask;
    QMap<uint, uint> legCount;

    int offset;

    rankingByTime.clear();
    QMultiMap<uint, Competitor>::const_iterator element;
    for (auto comp : CRLoader::getStartList()) {
        bib = comp.getBib();
        element = startList.constFind(bib);
        if (element != startList.constEnd()) {
            // check if there is a leg set for the competitor
            // otherwise set it automatically
            offset = comp.getOffset();
            comp.setLeg(static_cast<uint>((offset < 0) ? qAbs(offset) : (startList.count(bib) + 1)));

            // set a bit in the leg count array
            mask = 0x1 << comp.getLeg();
            if (legCount.contains(bib))
                legCount[bib] |= mask;
            else
                legCount.insert(bib, mask);
        }
        startList.insert(bib, comp);
    }

    uint prevMask = 0;
    for (QMap<uint, uint>::const_key_value_iterator i = legCount.constKeyValueBegin(); i != legCount.constKeyValueEnd(); i++) {
        mask = i->second;
        if (prevMask && (mask != prevMask)) {
            emit error(tr("Warning: missing or extra legs for bib %1").arg(i->first));
        }
        prevMask = mask;
    }
}

void RankingsBuilder::setCompetitorCategory(QVector<Category> const &categories, Competitor *competitor) const
{
    for (auto const &category : categories) {
        if (category.includes(competitor)) {
            competitor->setCategory(category.getFullDescription());
            break;
        }
    }
}
