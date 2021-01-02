#include <algorithm>

#include "teamslistmodel.h"

QDataStream &operator<<(QDataStream &out, const TeamsListModel &data)
{
    out << data.teamsList
        << quint32(data.teamNameWidthMax);

    return out;
}

QDataStream &operator>>(QDataStream &in, TeamsListModel &data)
{
    quint32 teamNameWidthMax;

    in >> data.teamsList
       >> teamNameWidthMax;

    data.teamNameWidthMax = (uint) teamNameWidthMax;

    return in;
}

void TeamsListModel::refreshCounters(int r)
{
    uint teamNameLength;

    Q_UNUSED(r);

    teamNameWidthMax = 0;
    for (const auto &team : teamsList) {
        teamNameLength = (uint) team.length();
        if (teamNameLength > teamNameWidthMax)
            teamNameWidthMax = teamNameLength;
    }
}

int TeamsListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return teamsList.size();
}

int TeamsListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

QVariant TeamsListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= teamsList.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return QVariant(teamsList.at(index.row()));
    else if (role == Qt::ToolTipRole)
        return QVariant(tr("Team name"));
    else
        return QVariant();
}


QVariant TeamsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("%1").arg(tr("Team Name"));
    else
        return QString("%1").arg(section + 1);
}

bool TeamsListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        teamsList[index.row()] = value.toString().simplified();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags TeamsListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TeamsListModel::insertRows(int position, int rows, const QModelIndex &parent)
{

    Q_UNUSED(parent);

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        teamsList.insert(position, QString());
    }
    endInsertRows();

    return true;
}

bool TeamsListModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        teamsList.removeAt(position);
    }
    endRemoveRows();

    return true;
}

void TeamsListModel::sort(int column, Qt::SortOrder order)
{

    if (column == 0) {
        std::stable_sort(teamsList.begin(), teamsList.end());
        if (order == Qt::DescendingOrder) std::reverse(teamsList.begin(), teamsList.end());
        emit dataChanged(QModelIndex(), QModelIndex());
    }
}

void TeamsListModel::reset() {
    beginResetModel();
    teamsList.clear();
    teamNameWidthMax = 0;
    endResetModel();
}

void TeamsListModel::addTeam(const QString& team)
{
    if (!team.isEmpty() && !teamsList.contains(team)) {
        int rowCount = this->rowCount();
        this->insertRow(rowCount, QModelIndex());
        this->setData(this->index(rowCount, 0, QModelIndex()), QVariant(team), Qt::EditRole);
    }
}

uint TeamsListModel::getTeamNameWidthMax() const
{
    return teamNameWidthMax;
}

const QList<QString>& TeamsListModel::getTeamsList() const
{
    return teamsList;
}
