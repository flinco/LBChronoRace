#include "startlistmodel.h"
#include "lbcrexception.h"

QDataStream &operator<<(QDataStream &out, const StartListModel &data)
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

    competitorNameMaxWidth = 0;
    for (const auto &comp : startList) {
        bib = comp.getBib();
        offset = comp.getOffset();
        leg = (uint) ((offset < 0) ? qAbs(offset) : 1);
        nameWidth = (uint) comp.getName().length();

        if (bib > maxBib)
            maxBib = bib;
        if (leg > legCount)
            legCount = leg;
        if (nameWidth > competitorNameMaxWidth)
            competitorNameMaxWidth = nameWidth;
    }
}

int StartListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(startList.count());
}

int StartListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return static_cast<int>(Competitor::Field::CMF_COUNT);
}

QVariant StartListModel::data(const QModelIndex &index, int role) const
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

bool StartListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool retval = false;
    if (index.isValid() && role == Qt::EditRole) {

        uint uval;
        switch (index.column()) {
        case static_cast<int>(Competitor::Field::CMF_BIB):
            uval = value.toUInt(&retval);
            if (retval && uval)
                startList[index.row()].setBib(uval);
            else
                retval = false;
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
            startList[index.row()].setTeam(value.toString().simplified());
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
    legCount = 0;
    maxBib = 0;
    competitorNameMaxWidth = 0;
    endResetModel();
}

const QList<Competitor>& StartListModel::getStartList() const
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
