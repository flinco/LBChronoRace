/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include "livetablefilterproxymodel.hpp"

LiveTableFilterProxyModel::LiveTableFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool LiveTableFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    if ((minRow >= 0) && (maxRow >= 0))
        return ((minRow <= sourceRow) && (sourceRow <= maxRow));

    if (minRow < 0)
        return (sourceRow <= maxRow);

    if (maxRow < 0)
        return (minRow <= sourceRow);

    return true;
}

void LiveTableFilterProxyModel::setMaxRow(int newMaxRow)
{
    maxRow = newMaxRow;
}

void LiveTableFilterProxyModel::setMinRow(int newMinRow)
{
    minRow = newMinRow;
}
