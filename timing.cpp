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

#include "timing.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

Timing::Field TimingSorter::sortingField = Timing::Field::TMF_TIME;
Qt::SortOrder TimingSorter::sortingOrder = Qt::AscendingOrder;

QDataStream &operator<<(QDataStream &out, Timing const &timing)
{
    out << quint32(timing.bib)
        << quint32(timing.leg)
        << quint32(timing.seconds)
        << qint32(timing.status);

    return out;
}

QDataStream &operator>>(QDataStream &in, Timing &timing)
{
    quint32 bib32;
    quint32 leg32;
    quint32 seconds32;
    qint32  status32;

    in >> bib32
       >> leg32
       >> seconds32
       >> status32;

    timing.bib     = bib32;
    timing.leg     = leg32;
    timing.seconds = seconds32;
    timing.status  = (Timing::Status) status32;

    return in;
}

uint Timing::getBib() const
{
    return bib;
}

void Timing::setBib(uint newBib)
{
    this->bib = newBib;
}

uint Timing::getLeg() const
{
    return leg;
}

void Timing::setLeg(uint newLeg)
{
    this->leg = newLeg;
}

bool Timing::isDnf() const
{
    return (status == Status::DNF);
}

bool Timing::isDns() const
{
    return (status == Status::DNS);
}

uint Timing::getSeconds() const
{
    return seconds;
}

Timing::Status Timing::getStatus() const
{
    return status;
}

QString Timing::getTiming() const
{
    return CRHelper::toTimeStr(this->seconds, this->status);
}

void Timing::setTiming(QString const &timing)
{

    if (timing.length() == 3) {
        if (!timing.compare("DNS", Qt::CaseInsensitive)) {
            this->status = Status::DNS;
            this->seconds = 0u;
        } else if (!timing.compare("DNF", Qt::CaseInsensitive)) {
            this->status = Status::DNF;
            this->seconds = 0u;
        } else {
            throw(ChronoRaceException(tr("Illegal timing value '%1'").arg(timing)));
        }
    } else {
        uint val;
        uint secs = 0u;
        bool converted;
        QString sep(":");
        if (timing.contains(".")) {
            sep.clear();
            sep.append(".");
        }
        for (auto const &token : timing.split(sep, Qt::KeepEmptyParts, Qt::CaseInsensitive)) {
            converted = true;
            val = token.toUInt(&converted);
            if (!converted) {
                //NOSONAR seconds = 0u;
                throw(ChronoRaceException(tr("Illegal timing value '%1' for bib '%2'").arg(token).arg(this->bib)));
            } else {
                secs = (secs * 60) + val;
            }
        }
        this->seconds = secs;
    }
}

void Timing::setTiming(char const *timing)
{
    setTiming(QString(timing));
}

bool Timing::isValid() const
{
    return ((bib != 0u) && ((status == Status::DNS) || (status == Status::DNF) || ((status == Status::CLASSIFIED) && (seconds != 0u))));
}

bool Timing::operator< (Timing const &rhs) const
{
    return (((this->status == Status::DNF) && (rhs.status == Status::DNS)) || ((this->status == Status::CLASSIFIED) && ((rhs.status != Status::CLASSIFIED) || (this->seconds < rhs.seconds))));
}

bool Timing::operator> (Timing const &rhs) const
{
    return (((this->status == Status::DNS) && (rhs.status != Status::DNS)) || ((rhs.status == Status::CLASSIFIED) && ((this->status == Status::DNF) || (this->seconds > rhs.seconds))));
}

bool Timing::operator<=(Timing const &rhs) const
{
    return !(*this > rhs);
}

bool Timing::operator>=(Timing const &rhs) const
{
    return !(*this < rhs);
}

bool TimingSorter::operator() (Timing const &lhs, Timing const &rhs) const
{
    switch(sortingField) {
    case Timing::Field::TMF_BIB:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getBib() > rhs.getBib()) : (lhs.getBib() < rhs.getBib());
    case Timing::Field::TMF_LEG:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs.getLeg() > rhs.getLeg()) : (lhs.getLeg() < rhs.getLeg());
    case Timing::Field::TMF_TIME:
        [[fallthrough]];
    default:
        return (sortingOrder == Qt::DescendingOrder) ? (lhs > rhs) : (lhs < rhs);
    }

    return false;
}

Qt::SortOrder TimingSorter::getSortingOrder()
{
    return sortingOrder;
}

void TimingSorter::setSortingOrder(Qt::SortOrder const &value)
{
    sortingOrder = value;
}

Timing::Field TimingSorter::getSortingField()
{
    return sortingField;
}

void TimingSorter::setSortingField(Timing::Field const &value)
{
    sortingField = value;
}

Timing::Field &operator++(Timing::Field &field)
{
    field = static_cast<Timing::Field>(static_cast<int>(field) + 1);
    //NOSONAR if (field == Timing::TMF_COUNT)
    //NOSONAR     field = Timing::TMF_FIRST;
    return field;
}

Timing::Field operator++(Timing::Field &field, int)
{
    Timing::Field tmp = field;
    ++field;
    return tmp;
}
