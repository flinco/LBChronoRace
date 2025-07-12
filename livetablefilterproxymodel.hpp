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

#ifndef LIVETABLEFILTERPROXYMODEL_H
#define LIVETABLEFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class LiveTableFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit LiveTableFilterProxyModel(QObject *parent = Q_NULLPTR);

    void setMinRow(int newMinRow);

    void setMaxRow(int newMaxRow);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    int minRow { -1 };
    int maxRow { -1 };
};

#endif // LIVETABLEFILTERPROXYMODEL_H
