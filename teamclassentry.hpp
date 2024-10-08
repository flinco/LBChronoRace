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

#ifndef TEAMCLASSENTRY_H
#define TEAMCLASSENTRY_H

#include "classentry.hpp"

namespace placement {
class TeamClassEntry;
}

class TeamClassEntry
{
    Q_DECLARE_TR_FUNCTIONS(TeamClassEntry)

private:
    QString               club { "" };
    QVector<ClassEntry *> entryList { };

    quint64 totaltime { Q_UINT64_C(0) };

public:
    QString const &getClub() const;
    ClassEntry *getClassEntry(int index) const;
    void setClassEntry(ClassEntry *entry);
    int getClassEntryCount() const;
    uint getAverageTiming() const;

    bool operator< (TeamClassEntry const &rhs) const;
    bool operator> (TeamClassEntry const &rhs) const;
    bool operator<=(TeamClassEntry const &rhs) const;
    bool operator>=(TeamClassEntry const &rhs) const;
};

#endif // TEAMCLASSENTRY_H
