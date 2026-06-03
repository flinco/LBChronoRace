/*****************************************************************************
 * Copyright (C) 2026 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#ifndef NAMESLOOKUP_HPP
#define NAMESLOOKUP_HPP

#include <QString>
#include <QSet>

#include "competitor.hpp"

namespace nameslookup {
class NamesLookup;
}

class NamesLookup
{
public:
    static Competitor::Sex getSex(QString const &name);

private:
    static QSet<QString> const males;
    static QSet<QString> const females;
};

#endif // NAMESLOOKUP_HPP
