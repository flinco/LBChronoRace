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
#include <QTableView>
#include <QStyleOptionViewItem>
#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QSize>

class CRTableModel : public QAbstractTableModel
{
    Q_OBJECT
    using QAbstractTableModel::QAbstractTableModel;

public:
    void setCounter(QLCDNumber *newCounter);

    virtual void resizeHeaders(QTableView *table) = 0;

    bool needsResizing();
    void setResizing();

private:
    QLCDNumber *counter { Q_NULLPTR };
    bool resize { true };

protected:
    virtual void refreshDisplayCounter() final;
    int computeSizeHintForColumn(QTableView const *view, int column) const;

public slots:
    virtual void refreshCounters(int r) = 0;
};

inline void CRTableModel::setCounter(QLCDNumber *newCounter)
{
    counter = newCounter;
}

inline bool CRTableModel::needsResizing()
{
    bool retval = resize;

    resize = false;

    return retval;
}

inline void CRTableModel::setResizing()
{
    resize = true;
}

inline void CRTableModel::refreshDisplayCounter()
{
    if (counter != Q_NULLPTR)
        counter->display(rowCount(QModelIndex()));
}

inline int CRTableModel::computeSizeHintForColumn(QTableView const *view, int column) const
{
    int maxWidth = 0;
    QAbstractItemModel const *model = view->model();
    QStyleOptionViewItem option;
    option.initFrom(view);

    QAbstractItemDelegate const *delegate = view->itemDelegateForColumn(column);
    if (!delegate)
        delegate = view->itemDelegate(); // fallback

    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex idx = model->index(row, column);
        QSize size = delegate->sizeHint(option, idx);
        maxWidth = qMax(maxWidth, size.width());
    }

    return maxWidth;
}

#endif // CRTABLEMODEL_HPP
