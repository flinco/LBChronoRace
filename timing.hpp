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

    friend auto operator<=>(Timing const &lhs, Timing const &rhs)
    {
        /*         +-------------------------------+ *
         *         |              LHS              | *
         *         +-------+-------+-------+-------+ *
         *         | CLASS |  DSQ  |  DNF  |  DNS  | *
         * +-+-----+-------+-------+-------+-------+ *
         * | |CLASS|ms<=>ms|ms<=>ms|   >   |   >   | *
         * | +-----+-------+-------+-------+-------+ *
         * |R| DSQ |ms<=>ms|ms<=>ms|   >   |   >   | *
         * |H+-----+-------+-------+-------+-------+ *
         * |S| DNF |   <   |   <   |   =   |   >   | *
         * | +-----+-------+-------+-------+-------+ *
         * | | DNS |   <   |   <   |   <   |   =   | *
         * +-+-----+-------+-------+-------+-------+ */

        switch (lhs.status) {
            using enum Timing::Status;

            case CLASSIFIED:
                [[fallthrough]];
            case DSQ:
                if ((rhs.status == DNS) || (rhs.status == DNF))
                    return std::strong_ordering::less;
                else
                    return (lhs.milliseconds <=> rhs.milliseconds);
            case DNF:
                if (rhs.status == DNS)
                    return std::strong_ordering::less;
                else if (rhs.status == DNF)
                    return std::strong_ordering::equal;
                else
                    return std::strong_ordering::greater;
            case DNS:
                return (rhs.status == DNS) ? std::strong_ordering::equal : std::strong_ordering::greater;
            default:
                Q_UNREACHABLE();
                break;
        }
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
