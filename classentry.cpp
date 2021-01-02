#include "classentry.h"
#include "crloader.h"
#include "lbcrexception.h"

// Static members
QString ClassEntry::empty("*** ??? ***");

ClassEntry::ClassEntry() : entries()
{
    this->bib         = 0u;
    this->totalTime   = 0u;
}

ClassEntry::ClassEntry(const uint bib) : ClassEntry()
{
    this->bib = bib;
}

uint ClassEntry::getBib() const
{
    return bib;
}

void ClassEntry::setBib(uint bib)
{
    this->bib = bib;
}
const QString ClassEntry::getName(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].competitor ? entries[legIdx].competitor->getName() : ClassEntry::empty;
}

const QString ClassEntry::getNamesCSV() const
{
    QString retString;
    bool skip, first = true;
    int i, j;
    Competitor *c;

    for (i = 0; i < entries.size(); i++) {
        c = entries[i].competitor;

        // avoid duplicates
        for (j = 0, skip = false; c && !skip && (j < i); j++) {
            skip = entries[j].competitor && (entries[j].competitor->getName().compare(c->getName()) == 0);
        }

        if (!skip) {
            if (c)
                retString += QString("%1%2,%3,%4").arg(first ? "" : ",", c->getName(), Competitor::toSexString(c->getSex()), QString::number(c->getYear()));
            else
                retString += QString("%1%2,%3,%4").arg(first ? "" : ",", ClassEntry::empty, Competitor::toSexString(Competitor::UNDEFINED), QString::number(0));
            first = false;
        }
    }

    for (i = 0; i < entries.size(); i++) {
        if ((c = entries[i].competitor)) {
            retString += QString(",%1").arg(c->getTeam());
            break;
        }
    }
    if (i == entries.size())
        retString += QString(",%1").arg(ClassEntry::empty);

    if (entries.size() > 1) {
        retString += QString(",%1").arg(Competitor::toSexString(getSex()));
    }

    return retString;
}

const QString ClassEntry::getNamesTxt() const
{
    QString retString;
    bool skip, first = true;
    int i, j;
    Competitor *c;

    for (i = 0; i < entries.size(); i++) {
        c = entries[i].competitor;

        // avoid duplicates
        for (j = 0, skip = false; c && !skip && (j < i); j++) {
            skip = entries[j].competitor && (entries[j].competitor->getName().compare(c->getName()) == 0);
        }

        if (!skip) {
            if (c)
                retString += QString("%1%2 (%3,%4)").arg((first) ? "" : " - ").arg(c->getName(), -CRLoader::getStartListNameWidthMax()).arg(Competitor::toSexString(c->getSex())).arg(c->getYear());
            else
                retString += QString("%1%2 (%3,%4)").arg((first) ? "" : " - ").arg(ClassEntry::empty, -CRLoader::getStartListNameWidthMax()).arg(Competitor::toSexString(Competitor::UNDEFINED)).arg(0, 4);
            first = false;
        }
    }

    for (i = 0; i < entries.size(); i++) {
        if ((c = entries[i].competitor)) {
            retString += QString(" - %1").arg(c->getTeam(), -CRLoader::getTeamNameWidthMax());
            break;
        }
    }
    if (i == entries.size())
        retString += QString(" - %1").arg(ClassEntry::empty, -CRLoader::getTeamNameWidthMax());

    if (entries.size() > 1) {
        retString += QString(" (%1)").arg(Competitor::toSexString(getSex()));
    }

    return retString;
}

uint ClassEntry::getYear(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getYear() : 0;
}

Competitor::Sex ClassEntry::getSex() const
{
    Competitor::Sex sex = Competitor::Sex::UNDEFINED;

    for (const auto e : entries) {
        sex = (sex == Competitor::Sex::UNDEFINED) ? e.competitor->getSex() : ((sex == e.competitor->getSex()) ? e.competitor->getSex() : Competitor::Sex::MISC);
    }

    return sex;
}

Competitor::Sex ClassEntry::getSex(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getSex() : Competitor::UNDEFINED;
}

const QString ClassEntry::getTimesCSV() const
{
    QString retString;

    for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
        retString.append(QString("%1%2,%3").arg(((it == entries.constBegin()) ? "" : ",")).arg(it->legRanking).arg(Timing::toTimeStr(it->time, Timing::CLASSIFIED)));

    return retString;
}

const QString ClassEntry::getTimesTxt(int legRankWidth) const
{
    QString retString;

    for (QVector<ClassEntryElement>::ConstIterator it = entries.constBegin(); it < entries.constEnd(); it++)
        retString.append(QString("%1(%2) %3").arg(((it == entries.constBegin()) ? "" : " - ")).arg(it->legRanking, legRankWidth).arg(Timing::toTimeStr(it->time, it->status), 7));

    return retString;
}

const QString ClassEntry::getTime(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return Timing::toTimeStr(entries[legIdx].time, entries[legIdx].status);
}

uint ClassEntry::getTimeValue(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].time;
}

uint ClassEntry::countEntries() const
{
    return (uint) entries.size();
}

void ClassEntry::setTime(Competitor* comp, const Timing& timing, QStringList &messages)
{
    Q_ASSERT(comp);

    uint legHint  = timing.getLeg();
    int  offset   = comp->getOffset();
    int  legIndex = (legHint > 0) ? (int) (legHint - 1) : entries.size();

    if ((offset < 0) && (legIndex + 1 != qAbs(offset)))
        messages << tr("Leg mismatch for bib %1: detected %2 overriding competitor declared %3").arg(bib).arg(legIndex + 1).arg(qAbs(offset));

    while (entries.size() <= legIndex) {
        // add slots to entry vector
        entries.push_back(ClassEntryElement());
        entries.last().competitor = Q_NULLPTR;
        entries.last().status = Timing::CLASSIFIED;
        entries.last().time = 0;
        entries.last().legRanking = 0;
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
            uint seconds = timing.getSeconds() - (uint) offset;
            entries[legIndex].time = seconds;
            totalTime += seconds;
        }
    }
}

uint ClassEntry::getLegRanking(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return entries[legIdx].legRanking;
}

void ClassEntry::setLegRanking(uint legIdx, uint ranking)
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    entries[legIdx].legRanking = ranking;
}

uint ClassEntry::getFromYear() const
{
    uint fromYear = UINT_MAX;

    for (const auto it : entries) {
        if (it.competitor)
            fromYear = (fromYear > it.competitor->getYear()) ? it.competitor->getYear() : fromYear;
    }

    return fromYear;
}

uint ClassEntry::getToYear() const
{
    uint toYear = 0u;

    for (const auto it : entries) {
        if (it.competitor)
            toYear = (toYear < it.competitor->getYear()) ? it.competitor->getYear() : toYear;
    }

    return toYear;
}

const QString& ClassEntry::getTeam() const
{
    for (auto it : entries) {
        if (it.competitor)
            return it.competitor->getTeam();
    }

    return ClassEntry::empty;
}

bool ClassEntry::isDnf() const
{
    for (auto it : entries) {
        if (it.status == Timing::DNF)
            return true;
    }
    return false;
}

bool ClassEntry::isDns() const
{
    for (auto it : entries) {
        if (it.status == Timing::DNS)
            return true;
    }
    return false;
}

const QString& ClassEntry::getCategory() const
{
    return category;
}

const QString& ClassEntry::getCategory(uint legIdx) const
{
    if ((int) legIdx >= entries.size())
        throw(ChronoRaceException(tr("Nonexistent leg %1 for bib %2").arg(legIdx + 1).arg(bib)));

    return (entries[legIdx].competitor) ? entries[legIdx].competitor->getCategory() : ClassEntry::empty;
}

void ClassEntry::setCategory(const QString &value)
{
    category = value;
}

uint ClassEntry::getTotalTime() const
{
    return totalTime;
}

const QString ClassEntry::getTotalTimeCSV() const
{
    return getTotalTimeTxt();
}

const QString ClassEntry::getTotalTimeTxt() const
{
    if (isDns()) return Timing::toTimeStr(totalTime, Timing::DNS);
    if (isDnf()) return Timing::toTimeStr(totalTime, Timing::DNF);
    return Timing::toTimeStr(totalTime, Timing::CLASSIFIED);
}

const QString ClassEntry::getDiffTimeTxt(uint referenceTime) const
{
    if (isDns() || isDnf() || (totalTime == referenceTime))
        return QString("");

    if (totalTime > referenceTime)
        return Timing::toTimeStr(totalTime - referenceTime, Timing::CLASSIFIED, "+");
    else
        return Timing::toTimeStr(referenceTime - totalTime, Timing::CLASSIFIED, "-");
}

bool ClassEntry::operator< (const ClassEntry& rhs) { return totalTime <  rhs.totalTime; }
bool ClassEntry::operator> (const ClassEntry& rhs) { return totalTime >  rhs.totalTime; }
bool ClassEntry::operator<=(const ClassEntry& rhs) { return totalTime <= rhs.totalTime; }
bool ClassEntry::operator>=(const ClassEntry& rhs) { return totalTime >= rhs.totalTime; }
