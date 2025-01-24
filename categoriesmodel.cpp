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
#include "categoriesmodel.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

QDataStream &CategoriesModel::cmSerialize(QDataStream &out) const
{
    out << this->categories;

    return out;
}

QDataStream &CategoriesModel::cmDeserialize(QDataStream &in)
{
    in >> this->categories;

    return in;
}

void CategoriesModel::refreshCounters(int r)
{
    Q_UNUSED(r)
}

int CategoriesModel::rowCount(QModelIndex const &parent) const
{

    Q_UNUSED(parent)

    return static_cast<int>(categories.count());
}

int CategoriesModel::columnCount(QModelIndex const &parent) const
{

    Q_UNUSED(parent)

    return static_cast<int>(Category::Field::CTF_COUNT);
}

QVariant CategoriesModel::data(QModelIndex const &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= categories.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        uint uval;
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TYPE):
            return QVariant(CRHelper::toTypeString(categories.at(index.row()).getType()));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            if ((uval = categories.at(index.row()).getToYear()))
                return QVariant(uval);
            return QVariant("");
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            if ((uval = categories.at(index.row()).getFromYear()))
                return QVariant(uval);
            return QVariant("");
        case static_cast<int>(Category::Field::CTF_TO_BIB):
            if ((uval = categories.at(index.row()).getToBib()))
                return QVariant(uval);
            return QVariant("");
        case static_cast<int>(Category::Field::CTF_FROM_BIB):
            if ((uval = categories.at(index.row()).getFromBib()))
                return QVariant(uval);
            return QVariant("");
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QVariant(categories.at(index.row()).getFullDescription());
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QVariant(categories.at(index.row()).getShortDescription());
        default:
            return QVariant();
        }
    } else if (role == Qt::EditRole) {
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TYPE):
            return QVariant(CRHelper::toTypeString(categories.at(index.row()).getType()));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QVariant(categories.at(index.row()).getToYear());
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QVariant(categories.at(index.row()).getFromYear());
        case static_cast<int>(Category::Field::CTF_TO_BIB):
            return QVariant(categories.at(index.row()).getToBib());
        case static_cast<int>(Category::Field::CTF_FROM_BIB):
            return QVariant(categories.at(index.row()).getFromBib());
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QVariant(categories.at(index.row()).getFullDescription());
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QVariant(categories.at(index.row()).getShortDescription());
        default:
            return QVariant();
        }
    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TYPE):
            return QVariant(tr("Male Individual/Relay (M), Female Individual/Relay (F), Mixed M/F Relay (X), Male Mixed Clubs Relay (Y), or Female Mixed Clubs Relay (Y)"));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QVariant(tr("The category will include competitors born up to and including this year (i.e. 2000); set to 0 to disable this constraint"));
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QVariant(tr("The category will include competitors born from this year (i.e. 1982); set to 0 to disable this constraint"));
        case static_cast<int>(Category::Field::CTF_TO_BIB):
            return QVariant(tr("The category will include competitors with a bib number less than or equal to this (i.e. 200); set to 0 to disable this constraint"));
        case static_cast<int>(Category::Field::CTF_FROM_BIB):
            return QVariant(tr("The category will include competitors with a bib number greater than or equal to this (i.e. 200); set to 0 to disable this constraint"));
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QVariant(tr("Full category name"));
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QVariant(tr("Short category name"));
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool CategoriesModel::setData(QModelIndex const &index, QVariant const &value, int role)
{
    bool retval = false;

    if (!index.isValid())
        return retval;

    if (role != Qt::EditRole)
        return retval;

    if (value.toString().contains(LBChronoRace::csvFilter))
        return retval;

    // from now on, retval is already 'true' if value is an empty string
    retval = value.toString().isEmpty();

    uint uval;
    switch (index.column()) {
    case static_cast<int>(Category::Field::CTF_TYPE):
        try {
            auto type = CRHelper::toCategoryType(value.toString().trimmed());
            categories[index.row()].setType(type);
            retval = true;
        } catch (ChronoRaceException &e) {
            emit error(e.getMessage());
            retval = false;
        }
        break;
    case static_cast<int>(Category::Field::CTF_TO_YEAR):
        uval = retval ? 0 : value.toUInt(&retval);
        if (retval) categories[index.row()].setToYear(uval);
        break;
    case static_cast<int>(Category::Field::CTF_FROM_YEAR):
        uval = retval ? 0 : value.toUInt(&retval);
        if (retval) categories[index.row()].setFromYear(uval);
        break;
    case static_cast<int>(Category::Field::CTF_TO_BIB):
        uval = retval ? 0 : value.toUInt(&retval);
        if (retval) categories[index.row()].setToBib(uval);
        break;
    case static_cast<int>(Category::Field::CTF_FROM_BIB):
        uval = retval ? 0 : value.toUInt(&retval);
        if (retval) categories[index.row()].setFromBib(uval);
        break;
    case static_cast<int>(Category::Field::CTF_FULL_DESCR):
        categories[index.row()].setFullDescription(value.toString().simplified());
        retval = true;
        break;
    case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
        categories[index.row()].setShortDescription(value.toString().simplified());
        retval = true;
        break;
    default:
        break;
    }

    if (retval) emit dataChanged(index, index);

    return retval;
}

QVariant CategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case static_cast<int>(Category::Field::CTF_TYPE):
            return QString("%1").arg(tr("Type"));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QString("%1").arg(tr("Born up to"));
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QString("%1").arg(tr("Born from"));
        case static_cast<int>(Category::Field::CTF_TO_BIB):
            return QString("%1").arg(tr("Bib up to"));
        case static_cast<int>(Category::Field::CTF_FROM_BIB):
            return QString("%1").arg(tr("Bib from"));
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QString("%1").arg(tr("Category Full Name"));
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QString("%1").arg(tr("Category Short Name"));
        default:
            return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags CategoriesModel::flags(QModelIndex const &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool CategoriesModel::insertRows(int position, int rows, QModelIndex const &parent)
{
    Q_UNUSED(parent)

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        categories.insert(position, Category());
    }

    endInsertRows();

    refreshDisplayCounter();

    return true;
}

bool CategoriesModel::removeRows(int position, int rows, QModelIndex const &parent)
{
    Q_UNUSED(parent)

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        categories.removeAt(position);
    }

    endRemoveRows();

    refreshDisplayCounter();

    return true;
}

void CategoriesModel::sort(int column, Qt::SortOrder order)
{

    CategorySorter::setSortingField((Category::Field) column);
    CategorySorter::setSortingOrder(order);
    std::stable_sort(categories.begin(), categories.end(), CategorySorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void CategoriesModel::reset()
{
    beginResetModel();
    categories.clear();
    endResetModel();

    refreshDisplayCounter();
}

QList<Category> const &CategoriesModel::getCategories() const
{
    return categories;
}
