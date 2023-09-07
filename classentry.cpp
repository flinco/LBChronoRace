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

#include "classentry.h"
#include "crloader.h"
#include "lbcrexception.h"

// Static members
QString ClassEntry::empty("*** ??? ***");


QString ClassEntryElement::formatNameCSV(bool first, QString const &name, QString const &sex, QString const &year) const
{
    return QString("%1%2,%3,%4").arg(first ? "" : ",", name, sex, year);
}

QString ClassEntryElement::formatNameTxt(bool first, QString const &name, QString const &sex, QString const &year) const
{
    return QString("%1%2 (%3,%4)").arg(first ? "" : ",", name, sex, year);
}

void ClassEntryElement::addNames(bool csvFormat, bool first, QString &entryString, QString const &emptyName) const
{
    Competitor const *c = this->competitor;

    if (c)
        entryString += csvFormat ?
                    formatNameCSV(first, c->getName(), Competitor::toSexString(c->getSex()), QString::number(c->getYear())) :
                    formatNameTxt(first, c->getName(static_cast<int>(emptyName.size())), Competitor::toSexString(c->getSex()), QString::number(c->getYear()));
    else
        entryString += csvFormat ?
                    formatNameCSV(first, emptyName, Competitor::toSexString(Competitor::Sex::UNDEFINED), QString("0")) :
                    formatNameTxt(first, emptyName, Competitor::toSexString(Competitor::Sex::UNDEFINED), QString("   0"));
}

uint ClassEntry::getBib() const
{
    return bib;
}

void ClassEntry::setBib(uint newBib)
{
    this->bib = newBib;
}

QString ClassEntry::getName(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].competitor ? entries[legIdx].competitor->getName() : ClassEntry::empty;
}

QString ClassEntry::getNamesCommon(bool csvFormat) const
{
    QString retString;
    bool skip;
    bool first = true;
    int i;
    Competitor const *c;

    int teamNameWidthMax = csvFormat ? 0 : static_cast<int>(CRLoader::getTeamNameWidthMax());
    int startListNameWidthMax = csvFormat ? 0 : static_cast<int>(CRLoader::getStartListNameWidthMax());

    QString emptyName = ClassEntry::empty;

    if (!csvFormat)
        emptyName.resize(startListNameWidthMax, ' ');

    for (i = 0; i < entries.size(); i++) {
        c = entries[i].competitor;

        // avoid duplicates
        skip = false;
        for (int j = 0; c && (j < i); j++) {
            if ((skip = entries[j].competitor && (entries[j].competitor->getName().compare(c->getName()) == 0)))
                break;
        }

        if (!skip) {
            entries[i].addNames(csvFormat, first, retString, emptyName);
            first = false;
        }
    }

    for (i = 0; i < entries.size(); i++) {
        if ((c = entries[i].competitor)) {
            retString += csvFormat ?
                        QString(",%1").arg(c->getTeam()) :
                        QString(" - %1").arg(c->getTeam(), -teamNameWidthMax);
            break;
        }
    }

    if (i == entries.size()) {
        retString += QString(csvFormat ? ",%1" : " - %1").arg(emptyName);
    }

    if (entries.size() > 1) {
        retString += QString(csvFormat ? ",%1" : " (%1)").arg(Competitor::toSexString(getSex()));
    }

    return retString;
}

QString ClassEntry::getNamesCSV() const
{
    return getNamesCommon(true);
}

QString ClassEntry::getNamesTxt() const
{
    return getNamesCommon(false);
}

uint ClassEntry::getYear(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getYear() : 0;
}

Competitor::Sex ClassEntry::getSex() const
{
    Competitor::Sex sex = Competitor::Sex::UNDEFINED;

    for (auto const &e : entries) {
        if (sex == Competitor::Sex::UNDEFINED)
            sex = e.competitor->getSex();
        else if (sex != e.competitor->getSex())
            sex = Competitor::Sex::MISC;
    }

    return sex;
}

Competitor::Sex ClassEntry::getSex(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getSex() : Competitor::Sex::UNDEFINED;
}

QString ClassEntry::getTimesCSV() const
{
    QString retString;

    for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
        retString.append(QString("%1%2,%3").arg((it == entries.constBegin()) ? "" : ",").arg(it->legRanking).arg(Timing::toTimeStr(it->time, Timing::Status::CLASSIFIED)));

    return retString;
}

QString ClassEntry::getTimesTxt(int legRankWidth) const
{
    QString retString;

    for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
        retString.append(QString("%1(%2) %3").arg((it == entries.constBegin()) ? "" : " - ").arg(it->legRanking, legRankWidth).arg(Timing::toTimeStr(it->time, it->status), 7));

    return retString;
}

QString ClassEntry::getTime(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return Timing::toTimeStr(entries[legIdx].time, entries[legIdx].status);
}

uint ClassEntry::getTimeValue(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].time;
}

uint ClassEntry::countEntries() const
{
    return static_cast<uint>(entries.size());
}

void ClassEntry::setTime(Competitor const *comp, Timing const &timing, QStringList &messages)
{
    Q_ASSERT(comp);

    uint legHint  = timing.getLeg();
    int  offset   = comp->getOffset();
    auto legIndex = static_cast<int>((legHint > 0) ? (legHint - 1) : entries.size());

    if ((offset < 0) && (legIndex + 1 != qAbs(offset)))
        messages << tr("Leg mismatch for bib %1: detected %2 overriding competitor declared %3").arg(bib).arg(legIndex + 1).arg(qAbs(offset));

    while (entries.size() <= legIndex) {
        // add slots to entry vector
        entries.emplaceBack();
    }

    if (entries[legIndex].competitor && (entries[legIndex].competitor != comp))
        messages << tr("Competitor mismatch for bib %1: found %2 replaced by %3").arg(QString::number(bib), entries[legIndex].competitor->getName(), comp->getName());

    entries[legIndex].competitor = comp;
    entries[legIndex].status = timing.getStatus();
    if (isDns()) {
        totalTime = UINT_MAX;
    } else if (isDnf()) {
        totalTime = UINT_MAX - 1;
    } else {
        if (offset < 0) {
            // no offset; use standard timing logic
            // good for both individual and relay races
            entries[legIndex].time = timing.getSeconds() - totalTime;
            totalTime = timing.getSeconds();
        } else {
            // competitor with offset; maybe individual
            // race without mass start or relay race with
            // timings sum
            uint seconds = timing.getSeconds() - static_cast<uint>(offset);
            entries[legIndex].time = seconds;
            totalTime += seconds;
        }
    }
}

uint ClassEntry::getLegRanking(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].legRanking;
}

void ClassEntry::setLegRanking(uint legIdx, uint ranking)
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    entries[legIdx].legRanking = ranking;
}

uint ClassEntry::getFromYear() const
{
    uint fromYear = UINT_MAX;

    for (auto const &it : entries) {
        if (it.competitor)
            fromYear = (fromYear > it.competitor->getYear()) ? it.competitor->getYear() : fromYear;
    }

    return fromYear;
}

uint ClassEntry::getToYear() const
{
    uint toYear = 0u;

    for (auto const &it : entries) {
        if (it.competitor)
            toYear = (toYear < it.competitor->getYear()) ? it.competitor->getYear() : toYear;
    }

    return toYear;
}

QString const &ClassEntry::getTeam() const
{
    for (auto const &it : entries) {
        if (it.competitor)
            return it.competitor->getTeam();
    }

    return ClassEntry::empty;
}

bool ClassEntry::isDnf() const
{
    return std::any_of(entries.constBegin(), entries.constEnd(), [&](ClassEntryElement const &el) {
        return (el.status == Timing::Status::DNF);
    });
}

bool ClassEntry::isDns() const
{
    return std::any_of(entries.constBegin(), entries.constEnd(), [&](ClassEntryElement const &el) {
        return (el.status == Timing::Status::DNS);
    });
}

QString const &ClassEntry::getCategory() const
{
    return category;
}

QString const &ClassEntry::getCategory(uint legIdx) const
{
    if (static_cast<qsizetype>(legIdx) >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getCategory() : ClassEntry::empty;
}

void ClassEntry::setCategory(QString const &value)
{
    category = value;
}

uint ClassEntry::getTotalTime() const
{
    return totalTime;
}

QString ClassEntry::getTotalTimeCSV() const
{
    return getTotalTimeTxt();
}

QString ClassEntry::getTotalTimeTxt() const
{
    if (isDns()) return Timing::toTimeStr(totalTime, Timing::Status::DNS);
    if (isDnf()) return Timing::toTimeStr(totalTime, Timing::Status::DNF);
    return Timing::toTimeStr(totalTime, Timing::Status::CLASSIFIED);
}

QString ClassEntry::getDiffTimeTxt(uint referenceTime) const
{
    if (isDns() || isDnf() || (totalTime == referenceTime))
        return QString("");

    if (totalTime > referenceTime)
        return Timing::toTimeStr(totalTime - referenceTime, Timing::Status::CLASSIFIED, "+");
    else
        return Timing::toTimeStr(referenceTime - totalTime, Timing::Status::CLASSIFIED, "-");
}

bool ClassEntry::operator< (ClassEntry const &rhs) const { return totalTime <  rhs.totalTime; }
bool ClassEntry::operator> (ClassEntry const &rhs) const { return totalTime >  rhs.totalTime; }
bool ClassEntry::operator<=(ClassEntry const &rhs) const { return totalTime <= rhs.totalTime; }
bool ClassEntry::operator>=(ClassEntry const &rhs) const { return totalTime >= rhs.totalTime; }
