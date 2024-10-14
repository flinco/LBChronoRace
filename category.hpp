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

#ifndef CATEGORY_HPP
#define CATEGORY_HPP

#include <QCoreApplication>
#include <QDataStream>

namespace category {
class Category;
class CategorySorter;
}

class Category {
    Q_DECLARE_TR_FUNCTIONS(Category)

public:
    enum class Type
    {
        MALE     = 0,
        FEMALE   = 1,
        RELAY_MF = 2,
        RELAY_Y  = 3,
        RELAY_X  = 4
    };

    enum class Field
    {
        CTF_FIRST       = 0,
        CTF_FULL_DESCR  = 0,
        CTF_SHORT_DESCR = 1,
        CTF_TYPE        = 2,
        CTF_FROM_YEAR   = 3,
        CTF_TO_YEAR     = 4,
        CTF_FROM_BIB    = 5,
        CTF_TO_BIB      = 6,
        CTF_LAST        = 6,
        CTF_COUNT       = 7
    };

private:
    Type type { Type::MALE };
    uint toYear { 0u };
    uint fromYear { 0u };
    uint toBib { 0u };
    uint fromBib { 0u };
    QString fullDescription { "" };
    QString shortDescription { "" };

    void illegalType(quint32 value) const;

public:
    Category() = default;

    friend QDataStream &operator<<(QDataStream &out, Category const &category);
    friend QDataStream &operator>>(QDataStream &in, Category &category);

    QString const &getFullDescription() const;
    void setFullDescription(QString const &newFullDescription);
    Type getType() const;
    void setType(Type const newType);
    QString const &getShortDescription() const;
    void setShortDescription(QString const &newShortDescription);
    uint getFromYear() const;
    void setFromYear(uint newFromYear);
    uint getToYear() const;
    void setToYear(unsigned int newToYear);
    uint getFromBib() const;
    void setFromBib(uint newFromBib);
    uint getToBib() const;
    void setToBib(unsigned int newToBib);
    uint getWeight() const;
    bool isValid() const;

    bool operator<  (Category const &rhs) const;
    bool operator>  (Category const &rhs) const;
    bool operator<= (Category const &rhs) const;
    bool operator>= (Category const &rhs) const;
    bool operator== (Category const &rhs) const;
};

Category::Field &operator++(Category::Field &field);
Category::Field  operator++(Category::Field &field, int);

class CategorySorter {

private:
    static Qt::SortOrder   sortingOrder;
    static Category::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(Qt::SortOrder const &value);
    static Category::Field getSortingField();
    static void setSortingField(Category::Field const &value);

    bool operator() (Category const &lhs, Category const &rhs) const;
};

#endif // CATEGORY_HPP
