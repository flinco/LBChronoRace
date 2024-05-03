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

#ifndef CATEGORY_H
#define CATEGORY_H

#include <QCoreApplication>
#include <QDataStream>

#include "competitor.h"

namespace category {
class Category;
class CategorySorter;
}

class Category {
    Q_DECLARE_TR_FUNCTIONS(Category)

public:
    enum class Field
    {
        CTF_FIRST       = 0,
        CTF_FULL_DESCR  = 0,
        CTF_SHORT_DESCR = 1,
        CTF_SEX         = 2,
        CTF_FROM_YEAR   = 3,
        CTF_TO_YEAR     = 4,
        CTF_TEAM        = 5,
        CTF_LAST        = 5,
        CTF_COUNT       = 6
    };

private:
    bool team { false };
    Competitor::Sex sex { Competitor::Sex::UNDEFINED };
    uint toYear { 0u };
    uint fromYear { 0u };
    QString fullDescription { "" };
    QString shortDescription { "" };

public:
    Category() = default;
    explicit Category(QString const &team);

    friend QDataStream &operator<<(QDataStream &out, Category const &category);
    friend QDataStream &operator>>(QDataStream &in, Category &category);

    bool isTeam() const;
    void setTeam(bool newTeam);
    uint getFromYear() const;
    void setFromYear(uint newFromYear);
    QString const &getFullDescription() const;
    void setFullDescription(QString const &newFullDescription);
    Competitor::Sex getSex() const;
    void setSex(Competitor::Sex const newSex);
    QString const &getShortDescription() const;
    void setShortDescription(QString const &newShortDescription);
    uint getToYear() const;
    void setToYear(unsigned int newToYear);
    bool isValid() const;

    bool includes(Competitor const *competitor) const;

    bool operator<  (Category const &rhs) const;
    bool operator>  (Category const &rhs) const;
    bool operator<= (Category const &rhs) const;
    bool operator>= (Category const &rhs) const;
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

#endif // CATEGORY_H
