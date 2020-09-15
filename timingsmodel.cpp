#include <algorithm>

#include "timingsmodel.h"
#include "lbcrexception.h"

QDataStream &operator<<(QDataStream &out, const TimingsModel &data)
{
    out << data.timings;

    return out;
}

QDataStream &operator>>(QDataStream &in, TimingsModel &data)
{
    in >> data.timings;

    return in;
}

void TimingsModel::refreshCounters(int r)
{
    Q_UNUSED(r);
}

int TimingsModel::rowCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent);

    return timings.count();
}

int TimingsModel::columnCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent);

    return Timing::TMF_COUNT;
}

QVariant TimingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= timings.size())
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
        case Timing::TMF_BIB:
            return QVariant(timings.at(index.row()).getBib());
        case Timing::TMF_LEG:
            return QVariant(timings.at(index.row()).getLeg());
        case Timing::TMF_TIME:
            return QVariant(timings.at(index.row()).getTiming());
        default:
            return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
        case Timing::TMF_BIB:
            return QVariant(tr("Bib number (not 0)"));
        case Timing::TMF_LEG:
            return QVariant(tr("Leg number (0 for automatic detection)"));
        case Timing::TMF_TIME:
            return QVariant(tr("Timing (i.e. 0:45:23) or DNF or DNS"));
        default:
            return QVariant();
        }
    else
        return QVariant();
}

bool TimingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    bool retval = false;
    if (index.isValid() && role == Qt::EditRole) {

        uint uval;
        switch (index.column()) {
        case Timing::TMF_BIB:
            uval = value.toUInt(&retval);
            if (retval && uval)
                timings[index.row()].setBib(uval);
            else
                retval = false;
            break;
        case Timing::TMF_LEG:
            uval = value.toUInt(&retval);
            if (retval)
                timings[index.row()].setLeg(uval);
            break;
        case Timing::TMF_TIME:
            try {
                timings[index.row()].setTiming(value.toString().trimmed());
                retval = true;
            } catch (ChronoRaceException &ex) {
                retval = false;
            }
            break;
        default:
            break;
        }

        if (retval) emit dataChanged(index, index);
    }
    return retval;
}

QVariant TimingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case Timing::TMF_BIB:
            return QString("%1").arg(tr("Bib"));
        case Timing::TMF_LEG:
            return QString("%1").arg(tr("Leg"));
        case Timing::TMF_TIME:
            return QString("%1").arg(tr("Timing"));
        default:
            return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags TimingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool TimingsModel::insertRows(int position, int rows, const QModelIndex &parent) {

    Q_UNUSED(parent);

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        timings.insert(position, Timing());
    }

    endInsertRows();
    return true;
}

bool TimingsModel::removeRows(int position, int rows, const QModelIndex &parent)
{

    Q_UNUSED(parent);

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        timings.removeAt(position);
    }

    endRemoveRows();
    return true;
}

void TimingsModel::sort(int column, Qt::SortOrder order)
{

    TimingSorter::setSortingField((Timing::Field) column);
    TimingSorter::setSortingOrder(order);
    std::stable_sort(timings.begin(), timings.end(), TimingSorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void TimingsModel::reset() {
    beginResetModel();
    timings.clear();
    endResetModel();
}

const QList<Timing>& TimingsModel::getTimings() const
{
    return timings;
}
