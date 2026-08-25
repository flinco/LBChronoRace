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

#include <QStandardItemModel>

bool LiveTableFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    int flags = 0;

    if ((minRow >= 0) && (maxRow >= 0)) {
        auto model = qobject_cast<QStandardItemModel *>(sourceModel());
        if (auto data = model->itemFromIndex(model->index(sourceRow, 0, sourceParent))->data(Qt::ItemDataRole::UserRole + 2); data.isValid()) {
            flags = data.value<int>();
        }
    }

    return (flags != 0) || (((minRow < 0) || (minRow <= sourceRow)) && ((maxRow < 0) || (sourceRow <= maxRow)));
}

void LiveTableFilterProxyModel::setMaxRow(int newMaxRow)
{
    maxRow = newMaxRow;
}

void LiveTableFilterProxyModel::setMinRow(int newMinRow)
{
    minRow = newMinRow;
}
