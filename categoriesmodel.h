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

#ifndef LBCATEGORIESMODEL_H
#define LBCATEGORIESMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.h"
#include "category.h"

class CategoriesModel : public CRTableModel
{
    Q_OBJECT
    using CRTableModel::CRTableModel;

public:
    explicit CategoriesModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };
    CategoriesModel(QList<Category> const &categoriesList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), categories(categoriesList) { };

    friend QDataStream &operator<<(QDataStream &out, CategoriesModel const &data);
    friend QDataStream &operator>>(QDataStream &in, CategoriesModel &data);

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QList<Category> const &getCategories() const;

    void reset();

public slots:
    void refreshCounters(int r) override;

private:
    QList<Category> categories;

signals:
    void error(QString const &message);
};

#endif // LBCATEGORIESMODEL_H
