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

#include "category.h"
#include "lbcrexception.h"

Category::Field CategorySorter::sortingField = Category::Field::CTF_FIRST;
Qt::SortOrder   CategorySorter::sortingOrder = Qt::AscendingOrder;

Category::Category(QString const &team)
{
    if (team.length() != 1) {
        throw(ChronoRaceException(tr("Illegal category type - expected 'I' or 'T' - found %1").arg(team)));
    } else {
        this->team = (team.compare("T", Qt::CaseInsensitive) == 0);
    }
}

QDataStream &operator<<(QDataStream &out, Category const &category)
{
    out << qint32(category.team)
        << Competitor::toSexString(category.sex)
        << quint32(category.toYear)
        << quint32(category.fromYear)
        << category.fullDescription
        << category.shortDescription;

    return out;
}

QDataStream &operator>>(QDataStream &in, Category &category)
{
    quint32 toYear32;
    quint32 fromYear32;
    qint32  team32;
    QString sexStr;

    in >> team32
       >> sexStr
       >> toYear32
       >> fromYear32
       >> category.fullDescription
       >> category.shortDescription;

    category.team     = (bool) team32;
    category.sex      = Competitor::toSex(sexStr);
    category.toYear   = toYear32;
    category.fromYear = fromYear32;

    return in;
}

bool Category::isTeam() const
{
    return team;
}

void Category::setTeam(bool newTeam)
{
    this->team = newTeam;
}

uint Category::getFromYear() const
{
    return fromYear;
}

void Category::setFromYear(uint newFromYear)
{
    this->fromYear = newFromYear;
}

QString const &Category::getFullDescription() const
{
    return fullDescription;
}

void Category::setFullDescription(QString const &newFullDescription)
{
    this->fullDescription = newFullDescription;
}

Competitor::Sex Category::getSex() const
{
    return sex;
}

void Category::setSex(Competitor::Sex const newSex)
{
    this->sex = newSex;
}

QString const &Category::getShortDescription() const
{
    return shortDescription;
}

void Category::setShortDescription(QString const &newShortDescription)
{
    this->shortDescription = newShortDescription;
}

uint Category::getToYear() const
{
    return toYear;
}

void Category::setToYear(uint newToYear)
{
    this->toYear = newToYear;
}

bool Category::isValid() const
{
    return (!fullDescription.isEmpty() && !shortDescription.isEmpty());
}

bool Category::includes(Competitor const *competitor) const
{
    return (competitor && !this->isTeam() &&
            (competitor->getSex() == this->getSex()) &&
            (competitor->getYear() >= this->getFromYear()) &&
            (competitor->getYear() <= this->getToYear()));
}

bool Category::operator< (Category const &rhs) const
{
    return (!this->isTeam() && rhs.isTeam());
}

bool Category::operator> (Category const &rhs) const
{
    return (this->isTeam() && !rhs.isTeam());
}

bool Category::operator<=(Category const &rhs) const
{
    return !(*this > rhs);
}

bool Category::operator>=(Category const &rhs) const
{
    return !(*this < rhs);
}

bool CategorySorter::operator() (Category const &lhs, Category const &rhs) const
{
    switch(sortingField) {
    case Category::Field::CTF_SEX:
        return (sortingOrder == Qt::DescendingOrder) ? (Competitor::toSexString(lhs.getSex()) > Competitor::toSexString(rhs.getSex())) : (Competitor::toSexString(lhs.getSex()) < Competitor::toSexString(rhs.getSex()));
    case Category::Field::CTF_TO_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getToYear() > rhs.getToYear()) : (lhs.getToYear() < rhs.getToYear());
    case Category::Field::CTF_FROM_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFromYear() > rhs.getFromYear()) : (lhs.getFromYear() < rhs.getFromYear());
    case Category::Field::CTF_FULL_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFullDescription() > rhs.getFullDescription()) : (lhs.getFullDescription() < rhs.getFullDescription());
    case Category::Field::CTF_SHORT_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getShortDescription() > rhs.getShortDescription()) : (lhs.getShortDescription() < rhs.getShortDescription());
    case Category::Field::CTF_TEAM: //NOSONAR
        // no break here
    default:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs > rhs) : (lhs < rhs);
    }

    return false;
}

Qt::SortOrder CategorySorter::getSortingOrder()
{
    return sortingOrder;
}

void CategorySorter::setSortingOrder(Qt::SortOrder const &value)
{
    sortingOrder = value;
}

Category::Field CategorySorter::getSortingField()
{
    return sortingField;
}

void CategorySorter::setSortingField(Category::Field const &value)
{
    sortingField = value;
}

Category::Field &operator++(Category::Field &field)
{
    field = static_cast<Category::Field>(static_cast<int>(field) + 1);
    //NOSONAR if (field == Category::CTF_COUNT)
    //NOSONAR    field = Category::CTF_FIRST;
    return field;
}

Category::Field operator++(Category::Field &field, int)
{
    Category::Field tmp = field;
    ++field;
    return tmp;
}
