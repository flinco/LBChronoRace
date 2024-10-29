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

#include <algorithm>

#include "lbchronorace.hpp"
#include "teamslistmodel.hpp"

QDataStream &TeamsListModel::tlmSerialize(QDataStream &out) const{
    quint32 v2TeamNameWidthMax = 0;

    out << this->teamsList
        << quint32(v2TeamNameWidthMax);

    return out;
}

QDataStream &TeamsListModel::tlmDeserialize(QDataStream &in){
    quint32 v2TeamNameWidthMax;

    in >> this->teamsList
       >> v2TeamNameWidthMax;

    return in;
}

void TeamsListModel::refreshCounters(int r)
{
    Q_UNUSED(r)
}

int TeamsListModel::rowCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(teamsList.size());
}

int TeamsListModel::columnCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant TeamsListModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= teamsList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return QVariant(teamsList.at(index.row()));
    else if (role == Qt::ToolTipRole)
        return QVariant(tr("Club name"));
    else
        return QVariant();
}


QVariant TeamsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("%1").arg(tr("Club Name"));
    else
        return QString("%1").arg(section + 1);
}

bool TeamsListModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    bool retval = false;

    if (!index.isValid())
        return retval;

    if (role != Qt::EditRole)
        return retval;

    if (value.toString().contains(LBChronoRace::csvFilter))
        return retval;

    teamsList[index.row()] = value.toString().simplified();
    retval = true;

    emit dataChanged(index, index);

    return retval;
}

Qt::ItemFlags TeamsListModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TeamsListModel::insertRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        teamsList.insert(position, QString());
    }
    endInsertRows();

    refreshDisplayCounter();

    return true;
}

bool TeamsListModel::removeRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        teamsList.removeAt(position);
    }
    endRemoveRows();

    refreshDisplayCounter();

    return true;
}

void TeamsListModel::sort(int column, Qt::SortOrder order)
{

    if (column == 0) {
        std::ranges::stable_sort(teamsList);
        if (order == Qt::DescendingOrder)
            std::ranges::reverse(teamsList);
        emit dataChanged(QModelIndex(), QModelIndex());
    }
}

void TeamsListModel::reset() {
    beginResetModel();
    teamsList.clear();
    endResetModel();

    refreshDisplayCounter();
}

void TeamsListModel::addTeam(QString const &team)
{
    if (!team.isEmpty() && !teamsList.contains(team)) {
        int rowCount = this->rowCount();
        this->insertRow(rowCount, QModelIndex());
        this->setData(this->index(rowCount, 0, QModelIndex()), QVariant(team), Qt::EditRole);

        refreshDisplayCounter();
    }
}

QList<QString> const &TeamsListModel::getTeamsList() const
{
    return teamsList;
}
