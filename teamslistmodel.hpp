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

#ifndef LBTEAMSLISTMODEL_HPP
#define LBTEAMSLISTMODEL_HPP

#include <QObject>
#include <QList>
#include <QDataStream>

#include "crtablemodel.hpp"

class TeamsListModel : public CRTableModel
{
    Q_OBJECT

public:
    explicit TeamsListModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };

    QDataStream &tlmSerialize(QDataStream &out) const;
    friend QDataStream &operator<<(QDataStream &out, TeamsListModel const &data)
    {
        return data.tlmSerialize(out);
    }

    QDataStream &tlmDeserialize(QDataStream &in);
    friend QDataStream &operator>>(QDataStream &in, TeamsListModel &data)
    {
        return data.tlmDeserialize(in);
    }

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QList<QString> const &getTeamsList() const;

    void reset();

public slots:
    void addTeam(QString const &team);
    void refreshCounters(int r) override;

private:
    QList<QString> teamsList;
};

#endif // LBTEAMSLISTMODEL_HPP
