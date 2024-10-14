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

#ifndef LBRANKINGSMODEL_HPP
#define LBRANKINGSMODEL_HPP

#include <QObject>
#include <QDataStream>

#include "crtablemodel.hpp"
#include "ranking.hpp"

class RankingsModel : public CRTableModel
{
    Q_OBJECT
    using CRTableModel::CRTableModel;

public:
    explicit RankingsModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };
    RankingsModel(QList<Ranking> const &rankingsList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), rankings(rankingsList) { };

    friend QDataStream &operator<<(QDataStream &out, RankingsModel const &data);
    friend QDataStream &operator>>(QDataStream &in, RankingsModel &data);

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void parseCategories();

    QList<Ranking> const &getRankings() const;

    void reset();

public slots:
    void refreshCounters(int r) override;

private:
    QList<Ranking> rankings;

signals:
    void error(QString const &);
};

#endif // LBRANKINGSMODEL_HPP
