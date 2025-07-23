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
#include "rankingsmodel.hpp"

QDataStream &RankingsModel::rmSerialize(QDataStream &out) const{
    out << this->rankings;

    return out;
}

QDataStream &RankingsModel::rmDeserialize(QDataStream &in){
    in >> this->rankings;

    return in;
}

void RankingsModel::resizeHeaders(QTableView *table)
{
    using enum Ranking::Field;

    Q_ASSERT(table != Q_NULLPTR);

    QHeaderView* header = table->horizontalHeader();

    // Start with ResizeToContents to calculate proper size hints
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->resizeColumnsToContents();

    int const totalWidth = table->viewport()->width();

    // Compute size hint widths for all columns
    QList<int> contentWidths;
    for (auto col = static_cast<int>(RTF_FIRST); col < static_cast<int>(RTF_COUNT); ++col)
        contentWidths << computeSizeHintForColumn(table, col);

    // --- Column 2 (RTF_TEAM): delegate controls width
    header->setSectionResizeMode(static_cast<int>(RTF_TEAM), QHeaderView::ResizeToContents);

    // --- Column 3 (RTF_CATEGORIES): at least 2× column 2, but still based on delegate
    int teamWidth = header->sectionSize(static_cast<int>(RTF_TEAM));
    int minCategoriesWidth = qMax(teamWidth * 2, contentWidths[static_cast<int>(RTF_CATEGORIES)]);
    header->setSectionResizeMode(static_cast<int>(RTF_CATEGORIES), QHeaderView::Fixed);
    header->resizeSection(static_cast<int>(RTF_CATEGORIES), minCategoriesWidth);

    // --- Column 1 (RTF_SHORT_DESCR): based on title only (fixed)
    int shortDescrWidth = qMax(header->sectionSize(static_cast<int>(RTF_SHORT_DESCR)), contentWidths[static_cast<int>(RTF_SHORT_DESCR)]);
    header->setSectionResizeMode(static_cast<int>(RTF_SHORT_DESCR), QHeaderView::Fixed);
    header->resizeSection(static_cast<int>(RTF_SHORT_DESCR), shortDescrWidth);

    // --- Column 0 (RTF_FULL_DESCR): takes remaining space, no smaller than content
    int availableWidth = totalWidth;
    for (auto col = static_cast<int>(RTF_SHORT_DESCR); col < static_cast<int>(RTF_COUNT); ++col)
        availableWidth -= header->sectionSize(col);

    int fullDescrWidth = qMax(availableWidth, contentWidths[static_cast<int>(RTF_FULL_DESCR)]);
    header->setSectionResizeMode(static_cast<int>(RTF_FULL_DESCR), QHeaderView::Interactive);
    header->resizeSection(static_cast<int>(RTF_FULL_DESCR), fullDescrWidth);

    // --- Final cleanup
    header->setStretchLastSection(false);
    header->setSectionsMovable(false);
}

void RankingsModel::refreshCounters(int r)
{
    Q_UNUSED(r)
}

int RankingsModel::rowCount(QModelIndex const &parent) const
{

    Q_UNUSED(parent)

    return static_cast<int>(rankings.count());
}

int RankingsModel::columnCount(QModelIndex const &parent) const
{

    Q_UNUSED(parent)

    return static_cast<int>(Ranking::Field::RTF_COUNT);
}

QVariant RankingsModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= rankings.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        switch (index.column()) {
            using enum Ranking::Field;

            case static_cast<int>(RTF_FULL_DESCR):
                return QVariant(rankings.at(index.row()).getFullDescription());
            case static_cast<int>(RTF_SHORT_DESCR):
                return QVariant(rankings.at(index.row()).getShortDescription());
            case static_cast<int>(RTF_TEAM):
                return QVariant(rankings.at(index.row()).isTeam() ? tr("T") : tr("I"));
            case static_cast<int>(RTF_CATEGORIES):
                return QVariant(rankings.at(index.row()).getCategories().join('+'));
            default:
                return QVariant();
        }
    else if (role == Qt::EditRole)
        switch (index.column()) {
            using enum Ranking::Field;

            case static_cast<int>(RTF_FULL_DESCR):
                return QVariant(rankings.at(index.row()).getFullDescription());
            case static_cast<int>(RTF_SHORT_DESCR):
                return QVariant(rankings.at(index.row()).getShortDescription());
            case static_cast<int>(RTF_TEAM):
                return QVariant(rankings.at(index.row()).isTeam() ? tr("T") : tr("I"));
            case static_cast<int>(RTF_CATEGORIES):
                return QVariant(rankings.at(index.row()).getCategories());
            default:
                return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
            using enum Ranking::Field;

            case static_cast<int>(RTF_FULL_DESCR):
                return QVariant(tr("Full ranking name"));
            case static_cast<int>(RTF_SHORT_DESCR):
                return QVariant(tr("Short ranking name"));
            case static_cast<int>(RTF_TEAM):
                return QVariant(tr("Individual/Relay (I) or Club (T)"));
            case static_cast<int>(RTF_CATEGORIES):
                return QVariant(tr("The ranking will include all the categories listed here"));
            default:
                return QVariant();
        }

    return QVariant();
}

bool RankingsModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    bool retval = false;

    if (!index.isValid())
        return retval;

    if (role != Qt::EditRole)
        return retval;

    if (value.toString().contains(LBChronoRace::csvFilter))
        return retval;

    switch (index.column()) {
        using enum Ranking::Field;

        case static_cast<int>(RTF_FULL_DESCR):
            rankings[index.row()].setFullDescription(value.toString().simplified());
            retval = true;
            break;
        case static_cast<int>(RTF_SHORT_DESCR):
            rankings[index.row()].setShortDescription(value.toString().simplified());
            retval = true;
            break;
        case static_cast<int>(RTF_TEAM):
            rankings[index.row()].setTeam(QString::compare(value.toString().trimmed(), "T", Qt::CaseInsensitive) == 0);
            break;
        case static_cast<int>(RTF_CATEGORIES):
            rankings[index.row()].setCategories(value.toStringList());
            break;
        default:
            break;
    }

    if (retval) emit dataChanged(index, index);

    return retval;
}

QVariant RankingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
            using enum Ranking::Field;

            case static_cast<int>(RTF_FULL_DESCR):
                return QString("%1").arg(tr("Ranking Full Name"));
            case static_cast<int>(RTF_SHORT_DESCR):
                return QString("%1").arg(tr("Ranking Short Name"));
            case static_cast<int>(RTF_TEAM):
                return QString("%1").arg(tr("Individual/Club"));
            case static_cast<int>(RTF_CATEGORIES):
                return QString("%1").arg(tr("Categories"));
            default:
                return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags RankingsModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool RankingsModel::insertRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        rankings.insert(position, Ranking());
    }

    endInsertRows();

    refreshDisplayCounter();

    return true;
}

bool RankingsModel::removeRows(int position, int rows, QModelIndex const &index)
{
    Q_UNUSED(index)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        rankings.removeAt(position);
    }

    endRemoveRows();

    refreshDisplayCounter();

    return true;
}

void RankingsModel::sort(int column, Qt::SortOrder order)
{

    RankingSorter::setSortingField((Ranking::Field) column);
    RankingSorter::setSortingOrder(order);
    std::ranges::stable_sort(rankings, RankingSorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void RankingsModel::reset()
{
    beginResetModel();
    rankings.clear();
    endResetModel();

    CRTableModel::setResizing();
    refreshDisplayCounter();
}

void RankingsModel::parseCategories()
{
    for (auto &ranking : rankings) {
        ranking.parseCategories();
    }
}

QList<Ranking> const &RankingsModel::getRankings() const
{
    return rankings;
}
