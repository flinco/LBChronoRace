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

#include "categoriesmodel.hpp"
#include "lbcrexception.hpp"

QDataStream &operator<<(QDataStream &out, CategoriesModel const &data)
{
    out << data.categories;

    return out;
}

QDataStream &operator>>(QDataStream &in, CategoriesModel &data)
{
    in >> data.categories;

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

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TEAM):
            return QVariant(categories.at(index.row()).isTeam() ? tr("T") : tr("I"));
        case static_cast<int>(Category::Field::CTF_SEX):
            return QVariant(Competitor::toSexString(categories.at(index.row()).getSex()));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QVariant(categories.at(index.row()).getToYear());
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QVariant(categories.at(index.row()).getFromYear());
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QVariant(categories.at(index.row()).getFullDescription());
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QVariant(categories.at(index.row()).getShortDescription());
        default:
            return QVariant();
        }
    else if (role == Qt::ToolTipRole)
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TEAM):
            return QVariant(tr("Individual (I) or Team (T)"));
        case static_cast<int>(Category::Field::CTF_SEX):
            return QVariant(tr("Male (M), Female (F), Misc (X) or Unspecified (U)"));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QVariant(tr("The category will include competitors born up to and including this year (i.e. 2000); 0 to disable"));
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QVariant(tr("The category will include competitors born from this year (i.e. 1982); to disable"));
        case static_cast<int>(Category::Field::CTF_FULL_DESCR):
            return QVariant(tr("Full category name"));
        case static_cast<int>(Category::Field::CTF_SHORT_DESCR):
            return QVariant(tr("Short category name"));
        default:
            return QVariant();
        }

    return QVariant();
}

bool CategoriesModel::setData(QModelIndex const &index, QVariant const &value, int role)
{

    bool retval = false;
    if (index.isValid() && role == Qt::EditRole) {

        uint uval;
        switch (index.column()) {
        case static_cast<int>(Category::Field::CTF_TEAM):
            categories[index.row()].setTeam(QString::compare(value.toString().trimmed(), "T", Qt::CaseInsensitive) == 0);
            break;
        case static_cast<int>(Category::Field::CTF_SEX):
            try {
                auto sex = Competitor::toSex(value.toString().trimmed());
                categories[index.row()].setSex(sex);
                retval = true;
            } catch (ChronoRaceException &ex) {
                emit error(ex.getMessage());
                retval = false;
            }
            break;
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            uval = value.toUInt(&retval);
            if (retval) categories[index.row()].setToYear(uval);
            break;
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            uval = value.toUInt(&retval);
            if (retval) categories[index.row()].setFromYear(uval);
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
    }
    return retval;
}

QVariant CategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case static_cast<int>(Category::Field::CTF_TEAM):
            return QString("%1").arg(tr("Individual/Team"));
        case static_cast<int>(Category::Field::CTF_SEX):
            return QString("%1").arg(tr("Sex"));
        case static_cast<int>(Category::Field::CTF_TO_YEAR):
            return QString("%1").arg(tr("Up to"));
        case static_cast<int>(Category::Field::CTF_FROM_YEAR):
            return QString("%1").arg(tr("From"));
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
}

QList<Category> const &CategoriesModel::getCategories() const
{
    return categories;
}
