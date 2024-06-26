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

#include "startlistmodel.hpp"
#include "lbcrexception.hpp"

QDataStream &operator<<(QDataStream &out, StartListModel const &data)
{
    out << data.startList
        << quint32(data.legCount)
        << quint32(data.maxBib)
        << quint32(data.competitorNameMaxWidth);

    return out;
}

QDataStream &operator>>(QDataStream &in, StartListModel &data)
{
    quint32 legCount;
    quint32 maxBib;
    quint32 competitorNameMaxWidth;

    in >> data.startList
       >> legCount
       >> maxBib
       >> competitorNameMaxWidth;

    data.legCount = legCount;
    data.maxBib = maxBib;
    data.competitorNameMaxWidth = competitorNameMaxWidth;

    return in;
}

void StartListModel::refreshCounters(int r)
{
    int offset;
    uint bib;
    uint leg;
    uint nameWidth;

    Q_UNUSED(r)

    legCount = 0;
    maxBib = 0;
    competitorNameMaxWidth = 0;
    for (auto const &comp : startList) {
        bib = comp.getBib();
        offset = comp.getOffset();
        leg = static_cast<uint>((offset < 0) ? qAbs(offset) : 1);
        nameWidth = static_cast<uint>(comp.getName().length());

        if (bib > maxBib)
            maxBib = bib;
        if (leg > legCount)
            legCount = leg;
        if (nameWidth > competitorNameMaxWidth)
            competitorNameMaxWidth = nameWidth;
    }
}

int StartListModel::rowCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(startList.count());
}

int StartListModel::columnCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(Competitor::Field::CMF_COUNT);
}

QVariant StartListModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= startList.size())
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
        case static_cast<int>(Competitor::Field::CMF_BIB):
            return QVariant(startList.at(index.row()).getBib());
        case static_cast<int>(Competitor::Field::CMF_NAME):
            return QVariant(startList.at(index.row()).getName());
        case static_cast<int>(Competitor::Field::CMF_SEX):
            return QVariant(Competitor::toSexString(startList.at(index.row()).getSex()));
        case static_cast<int>(Competitor::Field::CMF_YEAR):
            return QVariant(startList.at(index.row()).getYear());
        case static_cast<int>(Competitor::Field::CMF_TEAM):
            return QVariant(startList.at(index.row()).getTeam());
        case static_cast<int>(Competitor::Field::CMF_OFFSET_LEG):
            return QVariant(Competitor::toOffsetString(startList.at(index.row()).getOffset()));
        default:
            return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
        case static_cast<int>(Competitor::Field::CMF_BIB):
            return QVariant(tr("Bib number (not 0)"));
        case static_cast<int>(Competitor::Field::CMF_NAME):
            return QVariant(tr("Competitor name"));
        case static_cast<int>(Competitor::Field::CMF_SEX):
            return QVariant(tr("Male (M) or Female (F)"));
        case static_cast<int>(Competitor::Field::CMF_YEAR):
            return QVariant(tr("Year of birth (i.e. 1982)"));
        case static_cast<int>(Competitor::Field::CMF_TEAM):
            return QVariant(tr("Team name"));
        case static_cast<int>(Competitor::Field::CMF_OFFSET_LEG):
            return QVariant(tr("Start time offset or leg (to set the leg put a '-' sign before the leg number)"));
        default:
            return QVariant();
        }

    return QVariant();
}

bool StartListModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    bool retval = false;
    uint uval;

    if (!index.isValid())
        return retval;

    if (role != Qt::EditRole)
        return retval;

    switch (index.column()) {
        case static_cast<int>(Competitor::Field::CMF_BIB):
            uval = value.toUInt(&retval);
            if (retval && uval) {
                int maxLeg = this->getMaxLeg(uval, index.row());
                startList[index.row()].setBib(uval);
                startList[index.row()].setTeam(this->getTeam(uval));
                startList[index.row()].setOffset((maxLeg < 0) ? &maxLeg : Q_NULLPTR);
            } else {
                retval = false;
            }
            break;
        case static_cast<int>(Competitor::Field::CMF_NAME):
            startList[index.row()].setName(value.toString().simplified());
            retval = true;
            break;
        case static_cast<int>(Competitor::Field::CMF_SEX):
            try {
                Competitor::Sex sex = Competitor::toSex(value.toString().trimmed(), true);
                retval = (sex != Competitor::Sex::UNDEFINED);
                if (retval) startList[index.row()].setSex(sex);
            } catch (ChronoRaceException &ex) {
                emit error(ex.getMessage());
                retval = false;
            }
            break;
        case static_cast<int>(Competitor::Field::CMF_YEAR):
            uval = value.toUInt(&retval);
            if (retval) startList[index.row()].setYear(uval);
            break;
        case static_cast<int>(Competitor::Field::CMF_TEAM):
            uval = startList[index.row()].getBib();
            if (uval == 0) {
                startList[index.row()].setTeam(value.toString().simplified());
            } else {
                this->setTeam(uval, value.toString().simplified());
            }
            emit newTeam(startList[index.row()].getTeam());
            retval = true;
            break;
        case static_cast<int>(Competitor::Field::CMF_OFFSET_LEG):
            startList[index.row()].setOffset(Competitor::toOffset(value.toString().simplified()));
            retval = true;
            break;
        default:
            break;
    }

    if (retval)
        emit dataChanged(index, index);

    return retval;
}

QVariant StartListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case static_cast<int>(Competitor::Field::CMF_BIB):
            return QString("%1").arg(tr("Bib"));
        case static_cast<int>(Competitor::Field::CMF_NAME):
            return QString("%1").arg(tr("Competitor"));
        case static_cast<int>(Competitor::Field::CMF_SEX):
            return QString("%1").arg(tr("Sex"));
        case static_cast<int>(Competitor::Field::CMF_YEAR):
            return QString("%1").arg(tr("Year"));
        case static_cast<int>(Competitor::Field::CMF_TEAM):
            return QString("%1").arg(tr("Team"));
        case static_cast<int>(Competitor::Field::CMF_OFFSET_LEG):
            return QString("%1").arg(tr("Start time/Leg"));
        default:
            return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags StartListModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool StartListModel::insertRows(int position, int rows, QModelIndex const &parent)
{

    Q_UNUSED(parent)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        startList.insert(position, Competitor());
    }

    endInsertRows();
    return true;
}

bool StartListModel::removeRows(int position, int rows, QModelIndex const &parent)
{
    Q_UNUSED(parent)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        startList.removeAt(position);
    }
    endRemoveRows();

    return true;
}

void StartListModel::sort(int column, Qt::SortOrder order)
{
    CompetitorSorter::setSortingField((Competitor::Field) column);
    CompetitorSorter::setSortingOrder(order);
    std::stable_sort(startList.begin(), startList.end(), CompetitorSorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void StartListModel::reset()
{
    beginResetModel();
    startList.clear();
    legCount = 0;
    maxBib = 0;
    competitorNameMaxWidth = 0;
    endResetModel();
}

QList<Competitor> const &StartListModel::getStartList() const
{
    return startList;
}

uint StartListModel::getLegCount() const
{
    return legCount;
}

void StartListModel::setLegCount(uint leg)
{
    if (legCount < leg)
        legCount = leg;
}

uint StartListModel::getMaxBib() const
{
    return maxBib;
}

uint StartListModel::getCompetitorNameMaxWidth() const
{
    return competitorNameMaxWidth;
}

QString const *StartListModel::getTeam(uint bib)
{
    for (qsizetype row = 0; row < startList.count(); row++)
        if (startList[row].getBib() == bib)
            return &startList[row].getTeam();

    return Q_NULLPTR;
}

void StartListModel::setTeam(uint bib, QString const &team)
{
    for (qsizetype row = 0; row < startList.count(); row++)
        if (startList[row].getBib() == bib)
            startList[row].setTeam(team);
}

int StartListModel::getMaxLeg(uint bib, int skip)
{
    int retval = 0;
    uint legs = 0;
    int offset;

    for (qsizetype row = 0; row < startList.count(); row++) {
        if (row == skip)
            continue;

        if (startList[row].getBib() != bib)
            continue;

        offset = startList[row].getOffset();
        if (offset >= 0)
            continue;
        legs |= 1 << (-1 - offset);
    }

    if (legs) {
        offset = startList[skip].getOffset();
        if (legs & (1 << (-1 - offset))) {
            for (retval = -1; legs & 1; legs >>= 1)
                retval--;
        }
    }

    return (retval < 0) ? retval : 0;
}
