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

#include "category.hpp"
#include "lbchronorace.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

Category::Field CategorySorter::sortingField = Category::Field::CTF_FIRST;
Qt::SortOrder   CategorySorter::sortingOrder = Qt::AscendingOrder;

QDataStream &operator<<(QDataStream &out, Category const &category)
{
    out << category.fullDescription
        << category.shortDescription
        << quint32(category.type)
        << quint32(category.toYear)
        << quint32(category.fromYear);

    return out;
}

QDataStream &operator>>(QDataStream &in, Category &category)
{
    quint32 toYear32;
    quint32 fromYear32;

    if (LBChronoRace::binFormat < LBCHRONORACE_BIN_FMT_v4) {
        qint32  team32;
        QString sexStr;

        in >> team32
           >> sexStr
           >> toYear32
           >> fromYear32
           >> category.fullDescription
           >> category.shortDescription;

        switch (CRHelper::toSex(sexStr)) {
        case Competitor::Sex::MALE:
            category.type = Category::Type::MALE;
            break;
        case Competitor::Sex::FEMALE:
            category.type = Category::Type::FEMALE;
            break;
        case Competitor::Sex::UNDEFINED:
            category.type = Category::Type::RELAY_MF;
            break;
        }
    } else {
        qint32 type32;

        in >> category.fullDescription
           >> category.shortDescription
           >> type32
           >> toYear32
           >> fromYear32;

        switch (type32) {
        case static_cast<quint32>(Category::Type::MALE):
            [[fallthrough]];
        case static_cast<quint32>(Category::Type::FEMALE):
            [[fallthrough]];
        case static_cast<quint32>(Category::Type::RELAY_MF):
            [[fallthrough]];
        case static_cast<quint32>(Category::Type::RELAY_Y):
            [[fallthrough]];
        case static_cast<quint32>(Category::Type::RELAY_X):
            category.type = static_cast<Category::Type>(type32);
            break;
        default:
            category.illegalType(type32);
            break;
        }
    }

    category.toYear   = toYear32;
    category.fromYear = fromYear32;

    return in;
}

void Category::illegalType(quint32 value) const
{
    throw(ChronoRaceException(tr("Illegal category type '%1'").arg(value)));
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

Category::Type Category::getType() const
{
    return type;
}

void Category::setType(Category::Type const newType)
{
    this->type = newType;
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

uint Category::getWeight() const
{
    uint weight = 0u;

    if (this->fromYear)
        weight++;

    if (this->toYear)
        weight++;

    //NOSONAR switch (this->type) {
    //NOSONAR case Category::Type::RELAY_Y:
    //NOSONAR     [[fallthrough]];
    //NOSONAR case Category::Type::RELAY_X:
    //NOSONAR     weight++;
    //NOSONAR     break;
    //NOSONAR case Category::Type::MALE:
    //NOSONAR     [[fallthrough]];
    //NOSONAR case Category::Type::FEMALE:
    //NOSONAR     [[fallthrough]];
    //NOSONAR case Category::Type::RELAY_MF:
    //NOSONAR     // do nothing
    //NOSONAR     break;
    //NOSONAR default:
    //NOSONAR     Q_UNREACHABLE();
    //NOSONAR     break;
    //NOSONAR }

    return weight;
}

bool Category::isValid() const
{
    return (!fullDescription.isEmpty() && !shortDescription.isEmpty());
}

bool Category::operator< (Category const &rhs) const
{
    return (this->getFullDescription() < rhs.getFullDescription());
}

bool Category::operator> (Category const &rhs) const
{
    return (this->getFullDescription() > rhs.getFullDescription());
}

bool Category::operator<=(Category const &rhs) const
{
    return !(*this > rhs);
}

bool Category::operator>=(Category const &rhs) const
{
    return !(*this < rhs);
}

bool Category::operator== (Category const &rhs) const
{
    return ((this->getFullDescription() == rhs.getFullDescription())
         && (this->getShortDescription() == rhs.getShortDescription()));
}

bool CategorySorter::operator() (Category const &lhs, Category const &rhs) const
{
    switch(sortingField) {
    case Category::Field::CTF_TYPE:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getType() > rhs.getType()) : (lhs.getType() < rhs.getType());
    case Category::Field::CTF_TO_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getToYear() > rhs.getToYear()) : (lhs.getToYear() < rhs.getToYear());
    case Category::Field::CTF_FROM_YEAR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFromYear() > rhs.getFromYear()) : (lhs.getFromYear() < rhs.getFromYear());
    case Category::Field::CTF_FULL_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFullDescription() > rhs.getFullDescription()) : (lhs.getFullDescription() < rhs.getFullDescription());
    case Category::Field::CTF_SHORT_DESCR:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getShortDescription() > rhs.getShortDescription()) : (lhs.getShortDescription() < rhs.getShortDescription());
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
