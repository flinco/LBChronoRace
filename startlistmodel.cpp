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

#include "lbchronorace.hpp"
#include "startlistmodel.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

QDataStream &StartListModel::slmSerialize(QDataStream &out) const
{
    out << this->startList
        << quint32(this->legCount)
        << quint32(this->maxBib)
        << quint32(this->competitorNameWidthMax)
        << quint32(this->teamNameWidthMax);

    return out;
}

QDataStream &StartListModel::slmDeserialize(QDataStream &in){
    quint32 legCount32;
    quint32 maxBib32;
    quint32 competitorNameWidthMax32;
    quint32 teamNameWidthMax32;

    in >> this->startList
       >> legCount32
       >> maxBib32
       >> competitorNameWidthMax32;

    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v2) {
        in >> teamNameWidthMax32;
    } else {
        teamNameWidthMax32 = 0;
        this->refreshCounters(0);
    }

    this->legCount = legCount32;
    this->maxBib = maxBib32;
    this->competitorNameWidthMax = competitorNameWidthMax32;
    this->teamNameWidthMax = teamNameWidthMax32;

    return in;
}

void StartListModel::refreshCounters(int r)
{
    int offset;
    uint bib;
    uint leg;
    uint nameWidth;
    uint clubWidth;
    uint teamWidth;

    Q_UNUSED(r)

    legCount = 0;
    maxBib = 0;
    competitorNameWidthMax = 0;
    teamNameWidthMax = 0;
    for (auto const &comp : std::as_const(startList)) {
        bib = comp.getBib();
        offset = comp.getOffset();
        leg = static_cast<uint>((offset < 0) ? qAbs(offset) : 1);
        nameWidth = static_cast<uint>(comp.getCompetitorName(CRHelper::nameComposition).simplified().length());
        clubWidth = static_cast<uint>(comp.getClub().simplified().length());
        teamWidth = static_cast<uint>(comp.getTeam().simplified().length());

        if (clubWidth != 0) {
            teamWidth += clubWidth;
            if (teamWidth != 0)
                teamWidth++;
        }

        if (bib > maxBib)
            maxBib = bib;
        if (leg > legCount)
            legCount = leg;
        if (nameWidth > competitorNameWidthMax)
            competitorNameWidthMax = nameWidth;
        if (teamWidth > teamNameWidthMax)
            teamNameWidthMax = teamWidth;
    }
}

void StartListModel::resizeHeaders(QTableView *table)
{
    using enum Competitor::Field;

    Q_ASSERT(table != Q_NULLPTR);
    QHeaderView* header = table->horizontalHeader();

    // Start with ResizeToContents to calculate proper size hints
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->resizeColumnsToContents();

    int const totalWidth = table->viewport()->width();

    // Compute size hints for all columns
    QList<int> contentWidths;
    for (auto col = static_cast<int>(CMF_FIRST); col < static_cast<int>(CMF_COUNT); ++col)
        contentWidths << computeSizeHintForColumn(table, col);

    // Columns that should stay sized to content
    QList fixedCols = {
        static_cast<int>(CMF_BIB),
        static_cast<int>(CMF_SEX),
        static_cast<int>(CMF_YEAR),
        static_cast<int>(CMF_OFFSET_LEG)
    };

    for (int col : fixedCols)
        header->setSectionResizeMode(col, QHeaderView::ResizeToContents);

    // Columns that should expand and be user-adjustable
    QList flexCols = {
        static_cast<int>(CMF_SURNAME),
        static_cast<int>(CMF_NAME),
        static_cast<int>(CMF_CLUB),
        static_cast<int>(CMF_TEAM)
    };

    // Calculate remaining space after fixed columns
    int availableWidth = totalWidth;
    for (auto col = static_cast<int>(CMF_FIRST); col < static_cast<int>(CMF_COUNT); ++col) {
        if (!flexCols.contains(col))
            availableWidth -= header->sectionSize(col);
    }

    int flexWidth = availableWidth > 0 ? (availableWidth / flexCols.size()) : 100;

    // Assign calculated width to flexible columns
    for (int col : flexCols) {
        header->setSectionResizeMode(col, QHeaderView::Interactive);
        header->resizeSection(col, qMax(flexWidth, contentWidths[col]));
    }

    // Final adjustments
    header->setStretchLastSection(false);
    header->setSectionsMovable(false);
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
            using enum Competitor::Field;

            case static_cast<int>(CMF_BIB):
                return QVariant(startList.at(index.row()).getBib());
            case static_cast<int>(CMF_SURNAME):
                return QVariant(startList.at(index.row()).getSurname());
            case static_cast<int>(CMF_NAME):
                return QVariant(startList.at(index.row()).getName());
            case static_cast<int>(CMF_SEX):
                return QVariant(CRHelper::toSexString(startList.at(index.row()).getSex()));
            case static_cast<int>(CMF_YEAR):
                return QVariant(startList.at(index.row()).getYear());
            case static_cast<int>(CMF_CLUB):
                return QVariant(startList.at(index.row()).getClub());
            case static_cast<int>(CMF_TEAM):
                return QVariant(startList.at(index.row()).getTeam());
            case static_cast<int>(CMF_OFFSET_LEG):
                return QVariant(CRHelper::toOffsetString(startList.at(index.row()).getOffset()));
            default:
                return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
            using enum Competitor::Field;

            case static_cast<int>(CMF_BIB):
                return QVariant(tr("Bib number (not 0)"));
            case static_cast<int>(CMF_SURNAME):
                return QVariant(tr("Competitor surname"));
            case static_cast<int>(CMF_NAME):
                return QVariant(tr("Competitor name"));
            case static_cast<int>(CMF_SEX):
                return QVariant(tr("Male (M) or Female (F)"));
            case static_cast<int>(CMF_YEAR):
                return QVariant(tr("Year of birth (i.e. 1982)"));
            case static_cast<int>(CMF_CLUB):
                return QVariant(tr("Club name"));
            case static_cast<int>(CMF_TEAM):
                return QVariant(tr("Team name"));
            case static_cast<int>(CMF_OFFSET_LEG):
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

    if (value.toString().contains(LBChronoRace::csvFilter))
        return retval;

    switch (index.column()) {
        using enum Competitor::Field;

        case static_cast<int>(CMF_BIB):
            uval = value.toUInt(&retval);
            if (retval && uval) {
                int maxLeg = this->getMaxLeg(uval, index.row());
                startList[index.row()].setBib(uval);
                startList[index.row()].setClub(this->getClub(uval));
                startList[index.row()].setTeam(this->getTeam(uval));
                startList[index.row()].setOffset(maxLeg, true);
            } else {
                retval = false;
            }
            break;
        case static_cast<int>(CMF_SURNAME):
            startList[index.row()].setSurname(value.toString().simplified());
            retval = true;
            break;
        case static_cast<int>(CMF_NAME):
            startList[index.row()].setName(value.toString().simplified());
            retval = true;
            break;
        case static_cast<int>(CMF_SEX):
            try {
                Competitor::Sex sex = CRHelper::toSex(value.toString().trimmed());
                startList[index.row()].setSex(sex);
                retval = true;
            } catch (ChronoRaceException &e) {
                emit error(e.getMessage());
                retval = false;
            }
            break;
        case static_cast<int>(CMF_YEAR):
            uval = value.toUInt(&retval);
            if (retval) startList[index.row()].setYear(uval);
            break;
        case static_cast<int>(CMF_CLUB):
            //NOSONAR uval = startList[index.row()].getBib();
            //NOSONAR if (uval == 0) {
                startList[index.row()].setClub(value.toString().simplified());
            //NOSONAR } else {
            //NOSONAR     this->setClub(uval, value.toString().simplified());
            //NOSONAR }
            emit newClub(startList[index.row()].getClub());
            retval = true;
            break;
        case static_cast<int>(CMF_TEAM):
            uval = startList[index.row()].getBib();
            if (uval == 0) {
                startList[index.row()].setTeam(value.toString().simplified());
            } else {
                this->setTeam(uval, value.toString().simplified());
            }
            retval = true;
            break;
        case static_cast<int>(CMF_OFFSET_LEG):
            startList[index.row()].setOffset(CRHelper::toOffset(value.toString().simplified()));
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
            using enum Competitor::Field;

            case static_cast<int>(CMF_BIB):
                return QString("%1").arg(tr("Bib"));
            case static_cast<int>(CMF_SURNAME):
                return QString("%1").arg(tr("Last name"));
            case static_cast<int>(CMF_NAME):
                return QString("%1").arg(tr("First name"));
            case static_cast<int>(CMF_SEX):
                return QString("%1").arg(tr("Sex"));
            case static_cast<int>(CMF_YEAR):
                return QString("%1").arg(tr("Year"));
            case static_cast<int>(CMF_CLUB):
                return QString("%1").arg(tr("Club"));
            case static_cast<int>(CMF_TEAM):
                return QString("%1").arg(tr("Team"));
            case static_cast<int>(CMF_OFFSET_LEG):
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

bool StartListModel::insertRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        startList.insert(position, Competitor());
    }

    endInsertRows();

    refreshDisplayCounter();

    return true;
}

bool StartListModel::removeRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);
    for (int row = 0; row < rows; ++row) {
        startList.removeAt(position);
    }
    endRemoveRows();

    refreshDisplayCounter();

    return true;
}

void StartListModel::sort(int column, Qt::SortOrder order)
{
    CompetitorSorter::setSortingField((Competitor::Field) column);
    CompetitorSorter::setSortingOrder(order);
    std::ranges::stable_sort(startList, CompetitorSorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void StartListModel::reset()
{
    beginResetModel();
    startList.clear();
    legCount = 0;
    maxBib = 0;
    competitorNameWidthMax = 0;
    teamNameWidthMax = 0;
    endResetModel();

    CRTableModel::setResizing();
    refreshDisplayCounter();
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

uint StartListModel::getCompetitorNameWidthMax() const
{
    return competitorNameWidthMax;
}

uint StartListModel::getTeamNameWidthMax() const
{
    return teamNameWidthMax;
}

QString const *StartListModel::getClub(uint bib)
{
    QString const *clubPtr = Q_NULLPTR;

    for (qsizetype row = 0; row < startList.count(); row++) {
        if (startList[row].getBib() != bib)
            continue;

        if (startList[row].getClub().isEmpty())
            continue;

        if (!clubPtr) {
            clubPtr = &startList[row].getClub();
        } else if (*clubPtr != startList[row].getClub()) {
            clubPtr = Q_NULLPTR;
            break;
        }
    }

    return clubPtr;
}

QString const *StartListModel::getTeam(uint bib)
{
    for (qsizetype row = 0; row < startList.count(); row++)
        if (startList[row].getBib() == bib)
            return &startList[row].getTeam();

    return Q_NULLPTR;
}

void StartListModel::setClub(uint bib, QString const &club)
{
    for (qsizetype row = 0; row < startList.count(); row++)
        if (startList[row].getBib() == bib)
            startList[row].setClub(club);
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
