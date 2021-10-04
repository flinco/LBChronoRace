#include "teamclassentry.h"
#include "lbcrexception.h"

QString const &TeamClassEntry::getTeam() const
{
    return team;
}

ClassEntry const *TeamClassEntry::getClassEntry(int index) const
{
    if (index >= this->entryList.size())
        throw(ChronoRaceException(tr("Requested index %1 exceeds the number of available entries %2").arg(index).arg(this->entryList.size())));

    return this->entryList[index];
}

void TeamClassEntry::setClassEntry(ClassEntry *entry)
{
    if (this->entryList.isEmpty()) {
        this->team = entry->getTeam();
    } else if (this->team.compare(entry->getTeam()) != 0) {
        throw(ChronoRaceException(tr("Unexpected team: expected %1 - found %2").arg(this->team, entry->getTeam())));
    }
    this->entryList.push_back(entry);
}

int TeamClassEntry::getClassEntryCount() const
{
    return static_cast<int>(this->entryList.size());
}

bool TeamClassEntry::operator< (TeamClassEntry const &rhs)
{
    auto size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        for (int i = 0; i < size; i++) {
            if (*this->entryList[i] < *rhs.entryList.at(i)) return true;
        }
        return false;
    }
    return (size > rhs.entryList.size());
}

bool TeamClassEntry::operator> (TeamClassEntry const &rhs)
{
    auto size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        for (int i = 0; i < size; i++) {
            if (*this->entryList[i] > *rhs.entryList.at(i)) return true;
        }
        return false;
    }
    return (size < rhs.entryList.size());
}

bool TeamClassEntry::operator<=(TeamClassEntry const &rhs)
{
    return !(*this > rhs);
}

bool TeamClassEntry::operator>=(TeamClassEntry const &rhs)
{
    return !(*this < rhs);
}
