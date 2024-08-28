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

#include "crloader.hpp"
#include "rankingsbuilder.hpp"

uint RankingsBuilder::loadData()
{
    QStringList messages;
    QList<Timing> timings { CRLoader::getTimings() };

    uint bib;
    uint leg;

    // reset previous data (if any)
    rankingByBib.clear();
    startList.clear();

    // reset and fill the start list
    prepareStartList();

    // sort timings
    std::sort(timings.begin(), timings.end(), [&](Timing const &t1, Timing const &t2) { return (t1 < t2); } );

    // compute individual general classifications (all included, sorted by bib)
    for (auto const &timing : timings) {

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
        }

        if (classEntryIt != rankingByBib.end()) {
            classEntryIt.value().setTime(comp, timing, messages);
        } else {
            rankingByBib.insert(bib, ClassEntry(bib)).value().setTime(comp, timing, messages);
        }
    }

    emitMessages(messages);

    if (startList.size() != timings.size())
        emit error(tr("Warning: the number of timings (%1) is not match the expected (%2); check for possible missing or duplicated entries").arg(timings.size()).arg(startList.size()));

    // sort by time
    QList<ClassEntry *>::const_iterator c;
    for (auto classEntry = rankingByBib.begin(); classEntry != rankingByBib.end(); classEntry++) {
        c = rankingByTime.constBegin();
        messages += classEntry->setCategory();
        while ((c != rankingByTime.constEnd()) && (*(*c) < classEntry.value()))
            ++c;
        rankingByTime.insert(c, &classEntry.value());
    }

    emitMessages(messages);

    return static_cast<uint>(rankingByBib.size());
}

QList<ClassEntry const *> &RankingsBuilder::fillRanking(QList<ClassEntry const *> &ranking, Ranking const *categories) const
{
    Q_ASSERT(!categories->isTeam());

    QList<ClassEntry *> tmpRanking;
    for (auto &classEntry : rankingByTime) {

        // exclude not included categories
        if (!categories->includes(classEntry->getCategory())) {
            continue;
        }

        tmpRanking.append(classEntry);
    }

    // do the sorting of the single leg times
    PositionNumber position;
    sortLegTimes(tmpRanking, CRLoader::getStartListLegs(), position);

    ranking.clear();
    ranking.reserve(tmpRanking.size());
    for (ClassEntry const *classEntry : tmpRanking) {
        ranking.append(classEntry);
    }

    return ranking;
}

QList<TeamClassEntry const *> &RankingsBuilder::fillRanking(QList<TeamClassEntry const *> &ranking, Ranking const *categories)
{
    Q_ASSERT(categories->isTeam());

    rankingsByTeam.emplaceBack();
    auto &rankingByTeam = rankingsByTeam.last();

    QString club;

    for (auto &classEntry : rankingByTime) {

        // exclude DNS, DNF, and DSQ
        if (classEntry->isDns() || classEntry->isDnf() || classEntry->isDsq()) {
            continue;
        }

        club = classEntry->getClub();

        // exclude competitors without club
        if (club.isEmpty()) {
            continue;
        }

        // exclude not included categories
        if (!categories->includes(classEntry->getCategory())) {
            continue;
        }

        QMap<QString, TeamClassEntry>::iterator const teamRankingIt = rankingByTeam.find(club);
        if (teamRankingIt == rankingByTeam.end()) {
            rankingByTeam.insert(club, TeamClassEntry()).value().setClassEntry(classEntry);
        } else {
            teamRankingIt.value().setClassEntry(classEntry);
        }
    }

    // sort the team rankings
    QList<TeamClassEntry *> sortedTeamRanking;
    sortTeamRanking(rankingByTeam, sortedTeamRanking);

    // copy and return the team rankings
    ranking.clear();
    ranking.reserve(sortedTeamRanking.size());
    for (TeamClassEntry const *teamClassEntry : sortedTeamRanking) {
        ranking.append(teamClassEntry);
    }

    return ranking;
}

void RankingsBuilder::sortTeamRanking(QMap<QString, TeamClassEntry> &rankingByTeam, QList<TeamClassEntry *> &sortedTeamRanking) const
{
    uint legs = CRLoader::getStartListLegs();
    PositionNumber position;
    QList<TeamClassEntry *>::const_iterator t;
    for (auto &teamClassEntry : rankingByTeam) {
        // do the sorting of the single leg times
        sortLegTimes(teamClassEntry, legs, position);

        // actually sort the team ranking
        t = sortedTeamRanking.constBegin();
        while ((t != sortedTeamRanking.constEnd()) && (*(*t) < teamClassEntry))
            ++t;
        sortedTeamRanking.insert(t, &teamClassEntry);
    }
}

void RankingsBuilder::sortLegTimes(QList<ClassEntry *> const &ranking, uint legs, PositionNumber &position) const
{
    uint i;
    for (uint legIdx = 0u; legIdx < legs; legIdx++) {
        QMultiMap<uint, ClassEntry *> sortedLegClassification;
        for (auto classEntry : ranking) {
            if (classEntry->countEntries() == legs)
                sortedLegClassification.insert(classEntry->getTimeValue(legIdx), classEntry);
        }
        i = 0;
        for (auto classEntry : sortedLegClassification) {
            i++;
            classEntry->setLegRanking(legIdx, position.getCurrentPosition(i, classEntry->getTimeValue(legIdx)));
        }
    }
}

void RankingsBuilder::sortLegTimes(TeamClassEntry const &teamClassEntry, uint legs, PositionNumber &position) const
{
    uint i;
    int count;
    for (uint legIdx = 0u; legIdx < legs; legIdx++) {
        QMultiMap<uint, ClassEntry *> sortedLegClassification;
        count = teamClassEntry.getClassEntryCount();
        for (int j = 0; j < count; j++) {
            auto *classEntry = teamClassEntry.getClassEntry(j);
            if (classEntry->countEntries() == legs)
                sortedLegClassification.insert(classEntry->getTimeValue(legIdx), classEntry);
        }
        i = 0;
        for (auto classEntry : sortedLegClassification) {
            i++;
            classEntry->setLegRanking(legIdx, position.getCurrentPosition(i, classEntry->getTimeValue(legIdx)));
        }
    }
}

QList<Competitor const *> RankingsBuilder::fillStartList() const
{
    // create the start list container
    QList<Competitor const *> sortedStartList;

    // fill (sorted) the start list
    Competitor const *comp;
    QMutableListIterator  l { sortedStartList };
    QMultiMapIterator i { startList };
    while (i.hasNext()) {
        comp = &i.next().value();

        l.toFront();
        while (l.hasNext()) {
            if (*comp < *l.next()) {
                l.previous();
                break;
            }
        }

        l.insert(comp);
    }

    return sortedStartList;
}

void RankingsBuilder::prepareStartList()
{
    uint bib;
    uint mask;
    QMap<uint, uint> legCount;

    int offset;

    QList<Category> const &categories = CRLoader::getCategories();

    rankingByTime.clear();
    for (auto &comp : CRLoader::getStartList()) {
        bib = comp.getBib();

        // check if there is a leg set for the competitor
        // otherwise set it automatically
        offset = comp.getOffset();
        comp.setLeg(static_cast<uint>((offset < 0) ? qAbs(offset) : (startList.count(bib) + 1)));

        // set a bit in the leg count array
        mask = 0x1 << (comp.getLeg() - 1);
        if (legCount.contains(bib))
            legCount[bib] |= mask;
        else
            legCount.insert(bib, mask);

        // Set the category for each competitor
        comp.setCategories(categories);

        startList.insert(bib, comp);
    }

    uint prevBib = 0;
    uint prevMask = 0;
    for (QMap<uint, uint>::const_key_value_iterator i = legCount.constKeyValueBegin(); i != legCount.constKeyValueEnd(); i++) {
        mask = i->second;
        if (prevBib && (mask != prevMask)) {
            emit error(tr("Warning: missing or extra legs for bib %1 or %2").arg(prevBib).arg(i->first));
        }
        prevBib = i->first;
        prevMask = mask;
    }
}

void RankingsBuilder::emitMessages(QStringList &messages)
{
    for (auto const &message : messages) {
        if (message.length())
            emit error(tr("Notice:: %1").arg(message));
    }
    messages.clear();
}
