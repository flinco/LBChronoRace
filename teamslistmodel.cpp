#include <algorithm>

#include "teamslistmodel.h"

QDataStream &operator<<(QDataStream &out, TeamsListModel const &data)
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

    data.teamNameWidthMax = teamNameWidthMax;

    return in;
}

void TeamsListModel::refreshCounters(int r)
{
    uint teamNameLength;

    Q_UNUSED(r)

    teamNameWidthMax = 0;
    for (auto const &team : teamsList) {
        teamNameLength = static_cast<uint>(team.length());
        if (teamNameLength > teamNameWidthMax)
            teamNameWidthMax = teamNameLength;
    }
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

bool TeamsListModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        teamsList[index.row()] = value.toString().simplified();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags TeamsListModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TeamsListModel::insertRows(int position, int rows, QModelIndex const &parent)
{

    Q_UNUSED(parent)

    beginInsertRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        teamsList.insert(position, QString());
    }
    endInsertRows();

    return true;
}

bool TeamsListModel::removeRows(int position, int rows, QModelIndex const &parent)
{
    Q_UNUSED(parent)

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
        if (order == Qt::DescendingOrder)
            std::reverse(teamsList.begin(), teamsList.end());
        emit dataChanged(QModelIndex(), QModelIndex());
    }
}

void TeamsListModel::reset() {
    beginResetModel();
    teamsList.clear();
    teamNameWidthMax = 0;
    endResetModel();
}

void TeamsListModel::addTeam(QString const &team)
{
    if (!team.isEmpty() && !teamsList.contains(team)) {
        int rowCount = this->rowCount();
        this->insertRow(rowCount, QModelIndex());
        this->setData(this->index(rowCount, 0, QModelIndex()), QVariant(team), Qt::EditRole);

        auto teamNameLength = static_cast<uint>(team.length());
        if (teamNameLength > teamNameWidthMax)
            teamNameWidthMax = teamNameLength;
    }
}

uint TeamsListModel::getTeamNameWidthMax() const
{
    return teamNameWidthMax;
}

QList<QString> const &TeamsListModel::getTeamsList() const
{
    return teamsList;
}
