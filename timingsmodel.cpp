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
#include "timingsmodel.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

QDataStream &TimingsModel::tmSerialize(QDataStream &out) const{
    out << this->timings;

    return out;
}

QDataStream &TimingsModel::tmDeserialize(QDataStream &in){
    in >> this->timings;

    return in;
}

void TimingsModel::refreshCounters(int r)
{
    Q_UNUSED(r)
}

int TimingsModel::rowCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(timings.count());
}

int TimingsModel::columnCount(QModelIndex const &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(Timing::Field::TMF_COUNT);
}

QVariant TimingsModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= timings.size())
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
            using enum Timing::Field;

            case static_cast<int>(TMF_BIB):
                return QVariant(timings.at(index.row()).getBib());
            case static_cast<int>(TMF_LEG):
                return QVariant(timings.at(index.row()).getLeg());
            case static_cast<int>(TMF_TIME):
                return QVariant(timings.at(index.row()).getTiming());
            case static_cast<int>(TMF_STATUS):
                return QVariant(CRHelper::toStatusString(timings.at(index.row()).getStatus()));
            default:
                return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
            using enum Timing::Field;

            case static_cast<int>(TMF_BIB):
                return QVariant(tr("Bib number (not 0)"));
            case static_cast<int>(TMF_LEG):
                return QVariant(tr("Leg number (0 for automatic detection)"));
            case static_cast<int>(TMF_TIME):
                return QVariant(tr("Timing (i.e. 0:45:23)"));
            case static_cast<int>(TMF_STATUS):
                return QVariant(tr("Classified (CLS), Disqualified (DSQ), Did not finish (DNF), or Did not start (DNS)"));
            default:
                return QVariant();
        }
    else
        return QVariant();
}

bool TimingsModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    bool retval = false;

    if (!index.isValid())
        return retval;

    if (role != Qt::EditRole)
        return retval;

    if (value.toString().contains(LBChronoRace::csvFilter))
        return retval;

    uint uval;
    switch (index.column()) {
        using enum Timing::Field;

        case static_cast<int>(TMF_BIB):
            uval = value.toUInt(&retval);
            if (retval && uval)
                timings[index.row()].setBib(uval);
            else
                retval = false;
            break;
        case static_cast<int>(TMF_LEG):
            uval = value.toUInt(&retval);
            if (retval)
               timings[index.row()].setLeg(uval);
           break;
        case static_cast<int>(TMF_TIME):
            try {
                timings[index.row()].setTiming(value.toString().trimmed());
                retval = true;
            } catch (ChronoRaceException &e) {
                emit error(e.getMessage());
                retval = false;
            }
            break;
        case static_cast<int>(TMF_STATUS):
            try {
                timings[index.row()].setStatus(value.toString().trimmed());
                retval = true;
            } catch (ChronoRaceException &e) {
                emit error(e.getMessage());
                retval = false;
            }
            break;
        default:
            break;
    }

    if (retval) emit dataChanged(index, index);

    return retval;
}

QVariant TimingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
            using enum Timing::Field;

            case static_cast<int>(TMF_BIB):
                return QString("%1").arg(tr("Bib"));
            case static_cast<int>(TMF_LEG):
                return QString("%1").arg(tr("Leg"));
            case static_cast<int>(TMF_TIME):
                return QString("%1").arg(tr("Timing"));
            case static_cast<int>(TMF_STATUS):
                return QString("%1").arg(tr("Status"));
            default:
                return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags TimingsModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TimingsModel::insertRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        timings.insert(position, Timing());
    }

    endInsertRows();

    refreshDisplayCounter();

    return true;
}

bool TimingsModel::removeRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        timings.removeAt(position);
    }

    endRemoveRows();

    refreshDisplayCounter();

    return true;
}

void TimingsModel::sort(int column, Qt::SortOrder order)
{
    TimingSorter::setSortingField((Timing::Field) column);
    TimingSorter::setSortingOrder(order);
    std::ranges::stable_sort(timings, TimingSorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void TimingsModel::reset() {
    beginResetModel();
    timings.clear();
    endResetModel();

    refreshDisplayCounter();
}

QList<Timing> const &TimingsModel::getTimings() const
{
    return timings;
}

void TimingsModel::addTimeSpan(int offset)
{
    for (auto r = 0; r < timings.count(); r++) {
        timings[r].addOffset(offset);
        setData(this->index(r, static_cast<int>(Timing::Field::TMF_TIME)), QVariant(timings[r].getTiming()), Qt::EditRole);
    }
}
