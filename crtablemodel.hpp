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

#ifndef CRTABLEMODEL_HPP
#define CRTABLEMODEL_HPP

#include <QAbstractTableModel>
#include <QLCDNumber>

class CRTableModel : public QAbstractTableModel
{
    Q_OBJECT
    using QAbstractTableModel::QAbstractTableModel;

public:
    void setCounter(QLCDNumber *newCounter);

private:
    QLCDNumber *counter { Q_NULLPTR };

protected:
    virtual void refreshDisplayCounter() final;

public slots:
    virtual void refreshCounters(int r) = 0;
};

inline void CRTableModel::setCounter(QLCDNumber *newCounter)
{
    counter = newCounter;
}

inline void CRTableModel::refreshDisplayCounter()
{
    if (counter != Q_NULLPTR)
        counter->display(rowCount(QModelIndex()));
}

#endif // CRTABLEMODEL_HPP
