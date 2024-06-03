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

#ifndef TIMING_H
#define TIMING_H

#include <QCoreApplication>
#include <QDataStream>
#include <QString>

namespace timing {
class Timing;
class TimingSorter;
}

class Timing {
    Q_DECLARE_TR_FUNCTIONS(Timing)

public:
    enum class Status
    {
        DNS,
        DNF,
        CLASSIFIED
    };

    enum class Field
    {
        TMF_FIRST = 0,
        TMF_BIB   = 0,
        TMF_LEG   = 1,
        TMF_TIME  = 2,
        TMF_LAST  = 2,
        TMF_COUNT = 3
    };

private:
    uint   bib { 0u };
    uint   leg { 0u };
    uint   seconds { 0u };
    Status status { Status::CLASSIFIED};

public:
    Timing() = default;
    explicit Timing(uint const bib) : bib(bib) {  };

    friend QDataStream &operator<<(QDataStream &out, Timing const &timing);
    friend QDataStream &operator>>(QDataStream &in, Timing &timing);

    uint getBib() const;
    void setBib(uint newBib);
    uint getLeg() const;
    void setLeg(uint newLeg);
    bool isDnf() const;
    bool isDns() const;
    Status getStatus() const;
    uint getSeconds() const;
    QString getTiming() const;
    void setTiming(QString const &timing);
    void setTiming(char const *timing);
    bool isValid() const;

    static QString toTimeStr(uint const seconds, Timing::Status const status, char const *prefix = Q_NULLPTR);
    static QString toTimeStr(Timing const &timing);

    bool operator<  (Timing const &rhs) const;
    bool operator>  (Timing const &rhs) const;
    bool operator<= (Timing const &rhs) const;
    bool operator>= (Timing const &rhs) const;
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

#endif // TIMING_H
