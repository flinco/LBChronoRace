#include "categoriesmodel.h"
#include "lbcrexception.h"

int CategoriesModel::rowCount(const QModelIndex &parent) const {

    Q_UNUSED(parent);

    return categories.count();
}

int CategoriesModel::columnCount(const QModelIndex &parent) const {

    Q_UNUSED(parent);

    return Category::CTF_COUNT;
}

QVariant CategoriesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= categories.size())
        return QVariant();

    if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
        switch (index.column()) {
        case Category::CTF_TEAM:
            return QVariant(categories.at(index.row()).isTeam() ? tr("T") : tr("I"));
        case Category::CTF_SEX:
            return QVariant(Competitor::toSexString(categories.at(index.row()).getSex()));
        case Category::CTF_TO_YEAR:
            return QVariant(categories.at(index.row()).getToYear());
        case Category::CTF_FROM_YEAR:
            return QVariant(categories.at(index.row()).getFromYear());
        case Category::CTF_FULL_DESCR:
            return QVariant(categories.at(index.row()).getFullDescription());
        case Category::CTF_SHORT_DESCR:
            return QVariant(categories.at(index.row()).getShortDescription());
        default:
            return QVariant();
        }
    else if ((role == Qt::ToolTipRole))
        switch (index.column()) {
        case Category::CTF_TEAM:
            return QVariant(tr("Individual (I) or Team (T)"));
        case Category::CTF_SEX:
            return QVariant(tr("Male (M), Female (F) or Misc (X)"));
        case Category::CTF_TO_YEAR:
            return QVariant(tr("The category will include competitors born up to and including this year (i.e. 2000); 0 to disable"));
        case Category::CTF_FROM_YEAR:
            return QVariant(tr("The category will include competitors born from this year (i.e. 1982); to disable"));
        case Category::CTF_FULL_DESCR:
            return QVariant(tr("Full category name"));
        case Category::CTF_SHORT_DESCR:
            return QVariant(tr("Short category name"));
        default:
            return QVariant();
        }

    return QVariant();
}

bool CategoriesModel::setData(const QModelIndex &index, const QVariant &value, int role) {

    bool retval = false;
    if (index.isValid() && role == Qt::EditRole) {

        uint uval;
        switch (index.column()) {
        case Category::CTF_TEAM:
            categories[index.row()].setTeam(QString::compare(value.toString().trimmed(), "T", Qt::CaseInsensitive) == 0);
            break;
        case Category::CTF_SEX:
            try {
                Competitor::Sex sex = Competitor::toSex(value.toString().trimmed());
                retval = (sex != Competitor::UNDEFINED);
                if (retval) categories[index.row()].setSex(sex);
            } catch (ChronoRaceException ex) {
                retval = false;
            }
            break;
        case Category::CTF_TO_YEAR:
            uval = value.toUInt(&retval);
            if (retval) categories[index.row()].setToYear(uval);
            break;
        case Category::CTF_FROM_YEAR:
            uval = value.toUInt(&retval);
            if (retval) categories[index.row()].setFromYear(uval);
            break;
        case Category::CTF_FULL_DESCR:
            categories[index.row()].setFullDescription(value.toString().simplified());
            retval = true;
            break;
        case Category::CTF_SHORT_DESCR:
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

QVariant CategoriesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        switch (section) {
        case Category::CTF_TEAM:
            return QString("%1").arg(tr("Individual/Team"));
        case Category::CTF_SEX:
            return QString("%1").arg(tr("Sex"));
        case Category::CTF_TO_YEAR:
            return QString("%1").arg(tr("Up to"));
        case Category::CTF_FROM_YEAR:
            return QString("%1").arg(tr("From"));
        case Category::CTF_FULL_DESCR:
            return QString("%1").arg(tr("Category Full Name"));
        case Category::CTF_SHORT_DESCR:
            return QString("%1").arg(tr("Category Short Name"));
        default:
            return QString("%1").arg(section + 1);
        }
    else
        return QString("%1").arg(section + 1);
}

Qt::ItemFlags CategoriesModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

bool CategoriesModel::insertRows(int position, int rows, const QModelIndex &parent) {

    Q_UNUSED(parent);

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        categories.insert(position, Category());
    }

    endInsertRows();
    return true;
}

bool CategoriesModel::removeRows(int position, int rows, const QModelIndex &parent) {

    Q_UNUSED(parent);

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        categories.removeAt(position);
    }

    endRemoveRows();
    return true;
}

void CategoriesModel::sort(int column, Qt::SortOrder order) {

    CategorySorter::setSortingField((Category::Field) column);
    CategorySorter::setSortingOrder(order);
    std::stable_sort(categories.begin(), categories.end(), CategorySorter());
    emit dataChanged(QModelIndex(), QModelIndex());
}

void CategoriesModel::reset() {
    beginResetModel();
    categories.clear();
    endResetModel();
}
