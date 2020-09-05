#include <algorithm>

#include "startlistmodel.h"
#include "lbcrexception.h"

QDataStream &operator<<(QDataStream &out, const StartListModel &data)
{
    out << data.startList;

    return out;
}

QDataStream &operator>>(QDataStream &in, StartListModel &data)
{
    in >> data.startList;

    return in;
}

int StartListModel::rowCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent)

    return startList.count();
}

int StartListModel::columnCount(const QModelIndex &parent) const
{

    Q_UNUSED(parent)

    return Competitor::CMF_COUNT;
}

QVariant StartListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= startList.size())
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
        case Competitor::CMF_BIB:
            return QVariant(startList.at(index.row()).getBib());
        case Competitor::CMF_NAME:
            return QVariant(startList.at(index.row()).getName());
        case Competitor::CMF_SEX:
            return QVariant(Competitor::toSexString(startList.at(index.row()).getSex()));
        case Competitor::CMF_YEAR:
            return QVariant(startList.at(index.row()).getYear());
        case Competitor::CMF_TEAM:
            return QVariant(startList.at(index.row()).getTeam());
        case Competitor::CMF_OFFSET:
            return QVariant(Competitor::toOffsetString(startList.at(index.row()).getOffset()));
        default:
            return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
        case Competitor::CMF_BIB:
            return QVariant(tr("Bib number (not 0)"));
        case Competitor::CMF_NAME:
            return QVariant(tr("Competitor name"));
        case Competitor::CMF_SEX:
            return QVariant(tr("Male (M) or Female (F)"));
        case Competitor::CMF_YEAR:
            return QVariant(tr("Year of birth (i.e. 1982)"));
        case Competitor::CMF_TEAM:
            return QVariant(tr("Team name"));
        case Competitor::CMF_OFFSET:
            return QVariant(tr("Start time offset"));
        default:
            return QVariant();
        }

    return QVariant();
}

bool StartListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    bool retval = false;
    if (index.isValid() && role == Qt::EditRole) {

        uint uval;
        switch (index.column()) {
        case Competitor::CMF_BIB:
            uval = value.toUInt(&retval);
            if (retval && uval)
                startList[index.row()].setBib(uval);
            else
                retval = false;
            break;
        case Competitor::CMF_NAME:
            startList[index.row()].setName(value.toString().simplified());
            retval = true;
            break;
        case Competitor::CMF_SEX:
            try {
                Competitor::Sex sex = Competitor::toSex(value.toString().trimmed(), true);
                retval = (sex != Competitor::UNDEFINED);
                if (retval) startList[index.row()].setSex(sex);
            } catch (ChronoRaceException &ex) {
                retval = false;
            }
            break;
        case Competitor::CMF_YEAR:
            uval = value.toUInt(&retval);
            if (retval) startList[index.row()].setYear(uval);
            break;
        case Competitor::CMF_TEAM:
            startList[index.row()].setTeam(value.toString().simplified());
            emit newTeam(startList[index.row()].getTeam());
            retval = true;
            break;
        case Competitor::CMF_OFFSET:
            startList[index.row()].setOffset(Competitor::toOffset(value.toString().simplified()));
            retval = true;
            break;
        default:
            break;
        }

        if (retval) emit dataChanged(index, index);
    }
    return retval;
}

QVariant StartListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case Competitor::CMF_BIB:
            return QString("%1").arg(tr("Bib"));
        case Competitor::CMF_NAME:
            return QString("%1").arg(tr("Competitor"));
        case Competitor::CMF_SEX:
            return QString("%1").arg(tr("Sex"));
        case Competitor::CMF_YEAR:
            return QString("%1").arg(tr("Year"));
        case Competitor::CMF_TEAM:
            return QString("%1").arg(tr("Team"));
        case Competitor::CMF_OFFSET:
            return QString("%1").arg(tr("Start time"));
        default:
            return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags StartListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool StartListModel::insertRows(int position, int rows, const QModelIndex &parent)
{

    Q_UNUSED(parent)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        startList.insert(position, Competitor());
    }

    endInsertRows();
    return true;
}

bool StartListModel::removeRows(int position, int rows, const QModelIndex &parent)
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
    endResetModel();
}
