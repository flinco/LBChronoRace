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

#include "teamclassentry.hpp"
#include "lbcrexception.hpp"

QString const &TeamClassEntry::getClub() const
{
    return club;
}

ClassEntry *TeamClassEntry::getClassEntry(int index) const
{
    if (index >= this->entryList.size())
        throw(ChronoRaceException(tr("Requested index %1 exceeds the number of available entries %2").arg(index).arg(this->entryList.size())));

    return this->entryList[index];
}

void TeamClassEntry::setClassEntry(ClassEntry *entry)
{
    if (this->entryList.isEmpty()) {
        this->club = entry->getClub();
    } else if (this->club.compare(entry->getClub()) != 0) {
        throw(ChronoRaceException(tr("Unexpected club: expected %1 - found %2").arg(this->club, entry->getClub())));
    }
    this->entryList.push_back(entry);
    this->totaltime += entry->getTotalTime();
}

int TeamClassEntry::getClassEntryCount() const
{
    return static_cast<int>(this->entryList.size());
}

uint TeamClassEntry::getAverageTiming() const
{
    return static_cast<uint>(this->totaltime / this->entryList.count());
}

bool TeamClassEntry::operator< (TeamClassEntry const &rhs) const
{
    auto size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        //NOSONAR for (int i = 0; i < size; i++) {
        //NOSONAR     if (*this->entryList[i] < *rhs.entryList.at(i)) return true;
        //NOSONAR }
        //NOSONAR return false;
        return (this->getAverageTiming() < rhs.getAverageTiming());
    }
    return (size > rhs.entryList.size());
}

bool TeamClassEntry::operator> (TeamClassEntry const &rhs) const
{
    auto size = this->entryList.size();
    if (size == rhs.entryList.size()) {
        //NOSONAR for (int i = 0; i < size; i++) {
        //NOSONAR     if (*this->entryList[i] > *rhs.entryList.at(i)) return true;
        //NOSONAR }
        //NOSONAR return false;
        return (this->getAverageTiming() > rhs.getAverageTiming());
    }
    return (size < rhs.entryList.size());
}

bool TeamClassEntry::operator<=(TeamClassEntry const &rhs) const
{
    return !(*this > rhs);
}

bool TeamClassEntry::operator>=(TeamClassEntry const &rhs) const
{
    return !(*this < rhs);
}
