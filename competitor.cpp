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

#include "competitor.hpp"
#include "lbcrexception.hpp"

Competitor::Field CompetitorSorter::sortingField = Competitor::Field::CMF_FIRST;
Qt::SortOrder     CompetitorSorter::sortingOrder = Qt::AscendingOrder;

QDataStream &operator<<(QDataStream &out, const Competitor &comp)
{
    out << quint32(comp.bib)
        << comp.name
        << Competitor::toSexString(comp.sex)
        << quint32(comp.year)
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
       >> comp.name
       >> sexStr
       >> year32
       >> comp.team
       >> leg32
       >> offset32;

    comp.bib    = bib32;
    comp.sex    = Competitor::toSex(sexStr);
    comp.year   = year32;
    comp.leg    = leg32;
    comp.offset = offset32;

    return in;
}

QString const &Competitor::getName() const
{
    return name;
}

QString Competitor::getName(int width) const
{
    return QString("%1").arg(this->name, -width);
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

void Competitor::setOffset(int newOffset)
{
    this->offset = newOffset;
}

void Competitor::setOffset(int const *newOffset)
{
    if (newOffset)
        this->offset = *newOffset;
}

bool Competitor::isValid() const
{
    return ((bib != 0u) && !name.isEmpty() && (sex != Sex::UNDEFINED));
}

QString const &Competitor::getCategory() const
{
    return this->category;
}

void Competitor::setCategory(QString const &newCategory)
{
    this->category = newCategory;
}

Competitor::Sex Competitor::toSex(QString  const &sex, bool const strict)
{
    if (sex.length() != 1) {
        throw(ChronoRaceException(tr("Illegal sex '%1'").arg(sex)));
    } else {
        if (sex.compare("M", Qt::CaseInsensitive) == 0)
            return Sex::MALE;
        else if (sex.compare("F", Qt::CaseInsensitive) == 0)
            return Sex::FEMALE;
        else if (!strict && (sex.compare("X", Qt::CaseInsensitive) == 0))
            return Sex::MISC;
    }

    return Sex::UNDEFINED;
}

QString Competitor::toSexString(Sex const sex)
{
    switch (sex) {
        case Sex::MALE:
            return "M";
        case Sex::FEMALE:
            return "F";
        case Sex::MISC:
            return "X";
        case Sex::UNDEFINED:
            return "U";
        default:
            throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}

int Competitor::toOffset(QString const &offset)
{
    QStringList list = offset.split(":");

    int retval = -1;
    int h;
    int m;
    int s;
    int l = 0;
    bool check_h;
    bool check_m;
    bool check_s;
    bool check_l;
    switch (list.count()) {
        case 3:
            h = list[0].toInt(&check_h, 10);
            m = list[1].toInt(&check_m, 10);
            s = list[2].toInt(&check_s, 10);
            if (check_h && check_m && check_s)
                retval = (h * 3600) + (m * 60) + s;
        break;
        case 2:
            m = list[0].toInt(&check_m, 10);
            s = list[1].toInt(&check_s, 10);
            if (check_m && check_s)
                retval = (m * 60) + s;
        break;
        case 1:
            l = list[0].toInt(&check_l, 10);
            if (check_l)
                retval = -qAbs(l);
        break;
        default:
            // do nothing
        break;
    }

    return retval;
}

QString Competitor::toOffsetString(int offset)
{
    if (offset < 0)
        return QString("%1").arg(qAbs(offset));
    else
        return QString("%1:%2:%3").arg(((offset / 60) / 60)).arg(((offset / 60) % 60), 2, 10, QChar('0')).arg((offset % 60), 2, 10, QChar('0'));
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
        case Competitor::Field::CMF_NAME:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getName() > rhs.getName()) : (lhs.getName() < rhs.getName());
        case Competitor::Field::CMF_SEX:
            return (sortingOrder == Qt::DescendingOrder) ? (Competitor::toSexString(lhs.getSex()) > Competitor::toSexString(rhs.getSex())) : (Competitor::toSexString(lhs.getSex()) < Competitor::toSexString(rhs.getSex()));
        case Competitor::Field::CMF_YEAR:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getYear() > rhs.getYear()) : (lhs.getYear() < rhs.getYear());
        case Competitor::Field::CMF_TEAM:
            return (sortingOrder == Qt::DescendingOrder) ? (lhs.getTeam() > rhs.getTeam()) : (lhs.getTeam() < rhs.getTeam());
        case Competitor::Field::CMF_BIB:
            [[fallthrough]];
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
