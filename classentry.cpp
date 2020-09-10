#include "classentry.h"
#include "crloader.h"
#include "lbcrexception.h"

ClassEntry::ClassEntry()
{
    this->bib         = 0u;
    this->competitors = {};
    this->times       = {};
    this->states      = {};
    this->legRanking  = {};
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

const QString ClassEntry::getNamesCSV() const
{

    QString retString;
    bool skip, first = true;
    uint i = 0, j;
    for (auto c : this->competitors) {
        // avoid duplicates
        for (j = 0, skip = false; !skip && (j < i); j++) {
            skip = (this->competitors[j]->getName().compare(c->getName()) == 0);
        }
        if (!skip) {
            retString += QString("%1%2,%3,%4").arg(first ? "" : ",").arg(c->getName()).arg(Competitor::toSexString(c->getSex())).arg(c->getYear());
            first = false;
        }
        i++;
    }
    retString += QString(",%1").arg(competitors[0]->getTeam());
    if (competitors.size() > 1) {
        retString += QString(",%1").arg(Competitor::toSexString(this->getSex()));
    }
    return retString;
}

const QString ClassEntry::getNamesTxt() const
{

    QString retString;
    bool skip, first = true;
    bool destroy = false;
    uint i = 0, j;
    for (auto c : this->competitors) {
        if (c == Q_NULLPTR) {
            c = new Competitor();
            c->setName("*UNKNOWN*");
            destroy = true;
        }

        // avoid duplicates
        for (j = 0, skip = false; c && !skip && (j < i); j++) {
            skip = (this->competitors[j]->getName().compare(c->getName()) == 0);
        }
        if (!skip) {
            retString += QString("%1%2 (%3,%4)").arg(first ? "" : " - ").arg(c->getName(), -CRLoader::getStartListNameWidthMax()).arg(Competitor::toSexString(c->getSex())).arg(c->getYear());
            first = false;
        }

        if (destroy) {
            delete c;
            destroy = false;
        }
        i++;
    }
    retString += QString(" - %1").arg(competitors[0]->getTeam(), -CRLoader::getTeamNameWidthMax());
    if (competitors.size() > 1) {
        retString += QString(" (%1)").arg(Competitor::toSexString(this->getSex()));
    }
    return retString;
}

Competitor::Sex ClassEntry::getSex() const
{

    Competitor::Sex sex = Competitor::Sex::UNDEFINED;
    for (auto c : competitors) {
        sex = (sex == Competitor::Sex::UNDEFINED) ? c->getSex() : ((sex == c->getSex()) ? c->getSex() : Competitor::Sex::MISC);
    }
    return sex;
}

const QString ClassEntry::getTimesCSV() const
{

    QString retString;
    bool first = true;
    uint i = 0;
    for (const auto t : times) {
        retString.append(QString("%1%2,%3").arg(((first) ? "" : ",")).arg(legRanking[i++]).arg(Timing::toTimeStr(t, Timing::CLASSIFIED)));
        first = false;
    }
    return retString;
}

const QString ClassEntry::getTimesTxt(int legRankWidth) const
{

    QString retString;
    uint i = 0;
    for (const auto t : times) {
        retString.append(QString("%1(%2) %3").arg(((i == 0) ? "" : " - ")).arg(legRanking[i], legRankWidth).arg(Timing::toTimeStr(t, states[i]), 7));
        i++;
    }
    return retString;
}

uint ClassEntry::getTime(uint i) const
{
    if ((int) i >= times.size()) {
        throw(ChronoRaceException(tr("Inconsistent leg index %1 for bib %2").arg(i).arg(this->bib)));
    }
    return this->times.at(i);
}

uint ClassEntry::countTimes() const
{
    return this->times.size();
}

void ClassEntry::setTime(Competitor* comp, const Timing& timing)
{

    Q_ASSERT(comp);

    uint legHint  = timing.getLeg();
    int  legIndex = (legHint > 0) ? (int) (legHint - 1) : this->times.size();
    int  offset   = comp->getOffset();

    while (this->competitors.size() <= legIndex) {
        // add slots to the competitors vector
        this->competitors.push_back(Q_NULLPTR);
        // add slots to the states vector
        this->states.push_back(Timing::CLASSIFIED);
        // add slots to the times vector
        this->times.push_back(0);
    }

    this->competitors[legIndex] = comp;
    this->states[legIndex] = timing.getStatus();
    if (isDns()) {
        this->totalTime = UINT_MAX;
    } else if (isDnf()) {
        this->totalTime = UINT_MAX - 1;
    } else {
        if (offset < 0) {
            // no offset; use standard timing logic
            // good for both individual and relay races
            this->times[legIndex] = timing.getSeconds() - this->totalTime;
            this->totalTime = timing.getSeconds();
        } else {
            // competitor with offset; maybe individual
            // race without mass start or relay race with
            // timings sum
            uint seconds = timing.getSeconds() - (uint) offset;
            this->times[legIndex] = seconds;
            this->totalTime += seconds;
        }
    }
}

void ClassEntry::setLegRanking(const uint ranking)
{
    this->legRanking.push_back(ranking);
}

uint ClassEntry::getFromYear() const
{

    uint fromYear = UINT_MAX;
    for (const auto c : competitors) {
        fromYear = (fromYear > c->getYear()) ? c->getYear() : fromYear;
    }
    return fromYear;
}

uint ClassEntry::getToYear() const
{

    uint toYear = 0u;
    for (const auto c : competitors) {
        toYear = (toYear < c->getYear()) ? c->getYear() : toYear;
    }
    return toYear;
}

const QString& ClassEntry::getTeam() const
{
    return competitors[0]->getTeam();
}

bool ClassEntry::isDnf() const
{
    for (auto it : states) {
        if (it == Timing::DNF)
            return true;
    }
    return false;
}

bool ClassEntry::isDns() const
{
    for (auto it : states) {
        if (it == Timing::DNS)
            return true;
    }
    return false;
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
    if (isDns()) return Timing::toTimeStr(this->totalTime, Timing::DNS);
    if (isDnf()) return Timing::toTimeStr(this->totalTime, Timing::DNF);
    return Timing::toTimeStr(this->totalTime, Timing::CLASSIFIED);
}

bool ClassEntry::operator< (const ClassEntry& rhs) { return totalTime <  rhs.totalTime; }
bool ClassEntry::operator> (const ClassEntry& rhs) { return totalTime >  rhs.totalTime; }
bool ClassEntry::operator<=(const ClassEntry& rhs) { return totalTime <= rhs.totalTime; }
bool ClassEntry::operator>=(const ClassEntry& rhs) { return totalTime >= rhs.totalTime; }
