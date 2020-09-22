#include "teamclassentry.h"
#include "lbcrexception.h"

TeamClassEntry::TeamClassEntry()
{
    this->team = "";
    this->entryList = {};
}

const QString& TeamClassEntry::getTeam() const
{
    return team;
}

const ClassEntry* TeamClassEntry::getClassEntry(int index) const
{
    Q_ASSERT(index < this->entryList.size());
    return this->entryList[index];
}

void TeamClassEntry::setClassEntry(ClassEntry* entry)
{
    if (this->entryList.isEmpty()) {
        this->team = entry->getTeam();
    } else if (this->team.compare(entry->getTeam()) != 0) {
        throw(ChronoRaceException(tr("Unexpected team: expected %1 - found %2").arg(this->team).arg(entry->getTeam())));
    }
    this->entryList.push_back(entry);
}

int TeamClassEntry::getClassEntryCount()
{
    return this->entryList.size();
}

bool TeamClassEntry::operator< (const TeamClassEntry& rhs)
{
    int size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        for (int i = 0; i < size; i++) {
            if (*this->entryList[i] < *rhs.entryList.at(i)) return true;
        }
        return false;
    }
    return (size > rhs.entryList.size());
}

bool TeamClassEntry::operator> (const TeamClassEntry& rhs)
{
    int size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        for (int i = 0; i < size; i++) {
            if (*this->entryList[i] > *rhs.entryList.at(i)) return true;
        }
        return false;
    }
    return (size < rhs.entryList.size());
}

bool TeamClassEntry::operator<=(const TeamClassEntry& rhs)
{
    return !(*this > rhs);
}

bool TeamClassEntry::operator>=(const TeamClassEntry& rhs)
{
    return !(*this < rhs);
}
