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

#include "ranking.hpp"
#include "lbcrexception.hpp"

Ranking::Field RankingSorter::sortingField = Ranking::Field::RTF_FIRST;
Qt::SortOrder  RankingSorter::sortingOrder = Qt::AscendingOrder;

Ranking::Ranking(QString const &team)
{
    if (team.length() != 1) {
        throw(ChronoRaceException(tr("Invalid ranking type - expected 'I' or 'T' - found %1").arg(team)));
    } else {
        this->team = (team.compare("T", Qt::CaseInsensitive) == 0);
    }
}

QDataStream &Ranking::rSerialize(QDataStream &out) const{
    out << this->fullDescription
        << this->shortDescription
        << qint32(this->team)
        << this->categories;

    return out;
}

QDataStream &Ranking::rDeserialize(QDataStream &in){
    qint32  team32;

    in >> this->fullDescription
       >> this->shortDescription
       >> team32
       >> this->categories;

    this->team = static_cast<bool>(team32);

    return in;
}

QString const &Ranking::getFullDescription() const
{
    return fullDescription;
}

void Ranking::setFullDescription(QString const &newFullDescription)
{
    this->fullDescription = newFullDescription;
}

QString const &Ranking::getShortDescription() const
{
    return shortDescription;
}

void Ranking::setShortDescription(QString const &newShortDescription)
{
    this->shortDescription = newShortDescription;
}

bool Ranking::isTeam() const
{
    return team;
}

void Ranking::setTeam(bool newTeam)
{
    this->team = newTeam;
}

QStringList const &Ranking::getCategories() const
{
    return categories;
}

void Ranking::setCategories(QStringList const &newCategories)
{
    this->categories = newCategories;
}

void Ranking::parseCategories()
{
    if (this->categories.count() == 1) {

        QStringList list { this->categories[0].split('+') };

        this->categories.clear();
        this->categories.append(list);
    }
}

bool Ranking::isValid() const
{
    return (!fullDescription.isEmpty() && !shortDescription.isEmpty());
}

bool Ranking::includes(Category const *category) const
{
    return ((category != Q_NULLPTR) && (this->categories.indexOf(category->getShortDescription()) >= 0));
}

bool RankingSorter::operator() (Ranking const &lhs, Ranking const &rhs) const
{
    switch (sortingField) {
        using enum Ranking::Field;

        case RTF_FULL_DESCR:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getFullDescription() > rhs.getFullDescription()) : (lhs.getFullDescription() < rhs.getFullDescription());
        case RTF_SHORT_DESCR:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getShortDescription() > rhs.getShortDescription()) : (lhs.getShortDescription() < rhs.getShortDescription());
        case RTF_TEAM:
            [[fallthrough]];
        case RTF_CATEGORIES:
            [[fallthrough]];
        default:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs > rhs) : (lhs < rhs);
    }

    return false;
}

Qt::SortOrder RankingSorter::getSortingOrder()
{
    return sortingOrder;
}

void RankingSorter::setSortingOrder(Qt::SortOrder const &value)
{
    sortingOrder = value;
}

Ranking::Field RankingSorter::getSortingField()
{
    return sortingField;
}

void RankingSorter::setSortingField(Ranking::Field const &value)
{
    sortingField = value;
}

Ranking::Field &operator++(Ranking::Field &field)
{
    field = static_cast<Ranking::Field>(static_cast<int>(field) + 1);
    //NOSONAR if (field == Ranking::RTF_COUNT)
    //NOSONAR    field = Ranking::RTF_FIRST;
    return field;
}

Ranking::Field operator++(Ranking::Field &field, int)
{
    Ranking::Field tmp = field;
    ++field;
    return tmp;
}
