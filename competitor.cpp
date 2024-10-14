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
#include "competitor.hpp"
#include "crhelper.hpp"

// Static members
QString           Competitor::empty("");
Competitor::Field CompetitorSorter::sortingField = Competitor::Field::CMF_FIRST;
Qt::SortOrder     CompetitorSorter::sortingOrder = Qt::AscendingOrder;

QDataStream &operator<<(QDataStream &out, const Competitor &comp)
{
    out << quint32(comp.bib)
        << comp.surname
        << comp.name
        << CRHelper::toSexString(comp.sex)
        << quint32(comp.year)
        << comp.club
        << comp.team
        << quint32(comp.leg)
        << qint32(comp.offset);

    return out;
}

QDataStream &operator>>(QDataStream &in, Competitor &comp)
{
    quint32 bib32;
    quint32 year32;
    quint32 leg32;
    qint32  offset32;
    QString sexStr;

    in >> bib32
       >> comp.surname;
    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v4)
        in >> comp.name;
    in >> sexStr
       >> year32
       >> comp.club;
    if (LBChronoRace::binFormat > LBCHRONORACE_BIN_FMT_v2)
      in >> comp.team;
    in >> leg32
       >> offset32;

    comp.bib    = bib32;
    comp.sex    = CRHelper::toSex(sexStr);
    comp.year   = year32;
    comp.leg    = leg32;
    comp.offset = offset32;

    return in;
}

QString Competitor::getCompetitorName(ChronoRaceData::NameComposition nameComposition) const
{
    switch (nameComposition) {
    case ChronoRaceData::NameComposition::NAME_FIRST:
        return QString("%1 %2").arg(this->name, this->surname);
    case ChronoRaceData::NameComposition::SURNAME_FIRST:
        return QString("%1 %2").arg(this->surname, this->name);
    case ChronoRaceData::NameComposition::SURNAME_ONLY:
        return QString("%1").arg(this->surname);
    case ChronoRaceData::NameComposition::NAME_ONLY:
        return QString("%1").arg(this->name);
    default:
        Q_UNREACHABLE();
    }
}

QString Competitor::getCompetitorName(ChronoRaceData::NameComposition nameComposition, int width) const
{
    return QString("%1").arg(getCompetitorName(nameComposition), -width);
}

QString const &Competitor::getSurname() const
{
    return surname;
}

void Competitor::setSurname(QString const &newSurname)
{
    this->surname = newSurname;
}

QString const &Competitor::getName() const
{
    return name;
}

void Competitor::setName(QString const &newName)
{
    this->name = newName;
}

uint Competitor::getBib() const
{
    return bib;
}

void Competitor::setBib(uint newBib)
{
    this->bib = newBib;
}

Competitor::Sex Competitor::getSex() const
{
    return sex;
}

void Competitor::setSex(Competitor::Sex const newSex)
{
    this->sex = newSex;
}

QString const &Competitor::getClub() const
{
    return club;
}

QString Competitor::getClub(int newWidth) const
{
    return QString("%1").arg(this->club, -newWidth);
}

void Competitor::setClub(QString const &newClub)
{
    this->club = newClub;
}

void Competitor::setClub(QString const *newClub)
{
    if (newClub)
        this->club = *newClub;
}

QString const &Competitor::getTeam() const
{
    return team;
}

QString Competitor::getTeam(int newWidth) const
{
    return QString("%1").arg(this->team, -newWidth);
}

void Competitor::setTeam(QString const &newTeam)
{
    this->team = newTeam;
}

void Competitor::setTeam(QString const *newTeam)
{
    if (newTeam)
        this->team = *newTeam;
}

uint Competitor::getYear() const
{
    return year;
}

void Competitor::setYear(uint newYear)
{
    this->year = newYear;
}

uint Competitor::getLeg() const
{
    return leg;
}

void Competitor::setLeg(uint newLeg)
{
    this->leg = newLeg;
}

int Competitor::getOffset() const
{
    return offset;
}

void Competitor::setOffset(int newOffset, bool maxLeg)
{
    if (!maxLeg || (newOffset < 0))
        this->offset = newOffset;
}

bool Competitor::isValid() const
{
    return ((bib != 0u) && !name.isEmpty() && (sex != Sex::UNDEFINED));
}

Category const *Competitor::getCategory() const
{
    return this->categories.isEmpty() ? Q_NULLPTR : this->categories[0];
}

QList<Category const *> &Competitor::getCategories()
{
    return this->categories;
}

void Competitor::setCategories(QList<Category> const &newCategories)
{
    this->categories.clear();

    for (auto const &category : newCategories) {
        if (this->isInCategory(&category))
            this->categories.append(&category);
    }
}

bool Competitor::isInCategory(Category const *category) const
{
    if (!category)
        return false;

    uint fromYear = category->getFromYear();
    uint toYear = category->getToYear();
    uint fromBib = category->getFromBib();
    uint toBib = category->getToBib();

    if ((fromYear != 0) && (this->year < fromYear))
        return false;

    if ((toYear != 0) && (toYear < this->year))
        return false;

    if ((fromBib != 0) && (this->bib < fromBib))
        return false;

    if ((toBib != 0) && (toBib < this->bib))
        return false;

    Category::Type type = category->getType();
    switch (this->sex) {
    case Competitor::Sex::MALE:
        if ((type == Category::Type::FEMALE) || (type == Category::Type::RELAY_X))
            return false;
        break;
    case Competitor::Sex::FEMALE:
        if ((type == Category::Type::MALE) || (type == Category::Type::RELAY_Y))
            return false;
        break;
    case Competitor::Sex::UNDEFINED:
        return false;
    default:
        Q_UNREACHABLE();
    }

    return true;
}

bool Competitor::operator< (Competitor const &rhs) const
{
    if ((offset < 0) && (offset != rhs.offset))
        return qAbs(offset) < qAbs(rhs.offset);

    if (bib < rhs.bib)
        return true;

    if (bib == rhs.bib) {
        if (offset != rhs.offset)
            return qAbs(offset) < qAbs(rhs.offset);
        else
            return leg < rhs.leg;
    }

    return false;
}

bool Competitor::operator> (Competitor const &rhs) const
{
    if ((offset < 0) && (offset != rhs.offset))
        return qAbs(offset) > qAbs(rhs.offset);

    if (bib > rhs.bib)
        return true;

    if (bib == rhs.bib) {
        if (offset != rhs.offset)
            return qAbs(offset) > qAbs(rhs.offset);
        else
            return leg > rhs.leg;
    }

    return false;
}

bool Competitor::operator<=(Competitor const &rhs) const
{
    return !(*this > rhs);
}

bool Competitor::operator>=(Competitor const &rhs) const
{
    return !(*this < rhs);
}

bool CompetitorSorter::operator() (Competitor const &lhs, Competitor const &rhs) const
{
    switch(sortingField) {
        case Competitor::Field::CMF_SURNAME:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getSurname() > rhs.getSurname()) : (lhs.getSurname() < rhs.getSurname());
        case Competitor::Field::CMF_NAME:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getName() > rhs.getName()) : (lhs.getName() < rhs.getName());
        case Competitor::Field::CMF_SEX:
            return (sortingOrder == Qt::DescendingOrder) ? (CRHelper::toSexString(lhs.getSex()) > CRHelper::toSexString(rhs.getSex())) : (CRHelper::toSexString(lhs.getSex()) < CRHelper::toSexString(rhs.getSex()));
        case Competitor::Field::CMF_YEAR:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getYear() > rhs.getYear()) : (lhs.getYear() < rhs.getYear());
        case Competitor::Field::CMF_CLUB:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getClub() > rhs.getClub()) : (lhs.getClub() < rhs.getClub());
        case Competitor::Field::CMF_TEAM:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getTeam() > rhs.getTeam()) : (lhs.getTeam() < rhs.getTeam());
        case Competitor::Field::CMF_BIB:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getBib() > rhs.getBib()) : (lhs.getBib() < rhs.getBib());
        case Competitor::Field::CMF_OFFSET_LEG:
            [[fallthrough]];
        default:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs > rhs) : (lhs < rhs);
    }
}

Qt::SortOrder CompetitorSorter::getSortingOrder()
{
    return sortingOrder;
}

void CompetitorSorter::setSortingOrder(Qt::SortOrder const &value)
{
    sortingOrder = value;
}

Competitor::Field CompetitorSorter::getSortingField()
{
    return sortingField;
}

void CompetitorSorter::setSortingField(Competitor::Field const &value)
{
    sortingField = value;
}

Competitor::Field &operator++(Competitor::Field &field)
{
    field = static_cast<Competitor::Field>(static_cast<int>(field) + 1);
    //NOSONAR if (field == Competitor::Field::CMF_COUNT)
    //NOSONAR     field = Competitor::Field::CMF_FIRST;
    return field;
}

Competitor::Field operator++(Competitor::Field &field, int)
{
    Competitor::Field tmp = field;
    ++field;
    return tmp;
}
