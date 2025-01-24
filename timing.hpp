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

#ifndef TIMING_HPP
#define TIMING_HPP

#include <QCoreApplication>
#include <QDataStream>
#include <QString>
#include <QRegularExpression>

namespace timing {
class Timing;
class TimingSorter;
}

class Timing {
    Q_DECLARE_TR_FUNCTIONS(Timing)

public:
    enum class Status
    {
        DNS        = 0,
        DNF        = 1,
        DSQ        = 2,
        CLASSIFIED = 3
    };

    enum class Field
    {
        TMF_FIRST  = 0,
        TMF_BIB    = 0,
        TMF_LEG    = 1,
        TMF_TIME   = 2,
        TMF_STATUS = 3,
        TMF_LAST   = 3,
        TMF_COUNT  = 4
    };

private:
    uint   bib { 0u };
    uint   leg { 0u };
    uint   milliseconds { 0u };
    Status status { Status::CLASSIFIED };

    uint toMillis(QString const &token, bool countDigits = false) const;

public:
    Timing() = default;
    explicit Timing(uint const bib) : bib(bib) {  };

    QDataStream &tSerialize(QDataStream &out) const;
    friend QDataStream &operator<<(QDataStream &out, Timing const &data)
    {
        return data.tSerialize(out);
    }

    QDataStream &tDeserialize(QDataStream &in);
    friend QDataStream &operator>>(QDataStream &in, Timing &data)
    {
        return data.tDeserialize(in);
    }

    uint getBib() const;
    void setBib(uint newBib);
    uint getLeg() const;
    void setLeg(uint newLeg);
    bool isDnf() const;
    bool isDns() const;
    bool isDsq() const;
    Status getStatus() const;
    void setStatus(Status newStatus);
    void setStatus(QString const &newStatus);
    uint getMilliseconds() const;
    QString getTiming() const;
    void setTiming(QString const &timing);
    void setTiming(char const *timing);
    void addOffset(int offset);
    bool isValid() const;

    friend bool operator< (Timing const &lhs, Timing const &rhs)
    {
        bool lessThen = false;

        //NOSONAR switch (lhs.status) {
        //NOSONAR case Status::CLASSIFIED:
        //NOSONAR     lessThen = ((rhs.status != Status::CLASSIFIED) || (lhs.milliseconds < rhs.seconds));
        //NOSONAR     break;
        //NOSONAR case Status::DSQ:
        //NOSONAR     lessThen = ((rhs.status == Status::DNF) || (rhs.status == Status::DNS));
        //NOSONAR     break;
        //NOSONAR case Status::DNF:
        //NOSONAR     lessThen = (rhs.status == Status::DNS);
        //NOSONAR     break;
        //NOSONAR case Status::DNS:
        //NOSONAR     // nothing to assign
        //NOSONAR     break;
        //NOSONAR default:
        //NOSONAR     Q_UNREACHABLE();
        //NOSONAR     break;
        //NOSONAR }

        switch (lhs.status) {
        case Status::CLASSIFIED:
            [[fallthrough]];
        case Status::DSQ:
            lessThen = (rhs.status == Status::DNS) || (rhs.status == Status::DNF) || (lhs.milliseconds < rhs.milliseconds);
            break;
        case Status::DNF:
            lessThen = (rhs.status == Status::DNS);
            break;
        case Status::DNS:
            // nothing to assign
            break;
        default:
            Q_UNREACHABLE();
            break;
        }

        return lessThen;
    }

    friend bool operator> (Timing const &lhs, Timing const &rhs)
    {
        bool greaterThen = false;

        //NOSONAR switch (lhs.status) {
        //NOSONAR case Status::CLASSIFIED:
        //NOSONAR     greaterThen = ((rhs.status == Status::CLASSIFIED) && (lhs.milliseconds > rhs.milliseconds));
        //NOSONAR     break;
        //NOSONAR case Status::DSQ:
        //NOSONAR     greaterThen = (rhs.status == Status::CLASSIFIED);
        //NOSONAR     break;
        //NOSONAR case Status::DNF:
        //NOSONAR     greaterThen = ((rhs.status == Status::CLASSIFIED) || (rhs.status == Status::DSQ));
        //NOSONAR     break;
        //NOSONAR case Status::DNS:
        //NOSONAR     greaterThen = (rhs.status != Status::DNS);
        //NOSONAR     break;
        //NOSONAR default:
        //NOSONAR     Q_UNREACHABLE();
        //NOSONAR     break;
        //NOSONAR }

        switch (lhs.status) {
        case Status::CLASSIFIED:
            [[fallthrough]];
        case Status::DSQ:
            greaterThen = ((rhs.status == Status::CLASSIFIED) || (rhs.status == Status::DSQ)) && (lhs.milliseconds > rhs.milliseconds);
            break;
        case Status::DNF:
            greaterThen = ((rhs.status == Status::CLASSIFIED) || (rhs.status == Status::DSQ));
            break;
        case Status::DNS:
            greaterThen = (rhs.status != Status::DNS);
            break;
        default:
            Q_UNREACHABLE();
            break;
        }

        return greaterThen;
    }

    friend bool operator<=(Timing const &lhs, Timing const &rhs)
    {
        return !(lhs > rhs);
    }

    friend bool operator>=(Timing const &lhs, Timing const &rhs)
    {
        return !(lhs < rhs);
    }

    static QRegularExpression validatorS;
    static QRegularExpression validatorMS;
    static QRegularExpression validatorHMS;
};

Timing::Field &operator++(Timing::Field &field);
Timing::Field  operator++(Timing::Field &field, int);

class TimingSorter {

private:
    static Qt::SortOrder sortingOrder;
    static Timing::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(Qt::SortOrder const &value);
    static Timing::Field getSortingField();
    static void setSortingField(Timing::Field const &value);

    bool operator() (Timing const &lhs, Timing const &rhs) const;
};

#endif // TIMING_HPP
