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

#ifndef RANKING_HPP
#define RANKING_HPP

#include <QString>
#include <QStringList>
#include <QDataStream>

#include "category.hpp"

namespace ranking {
class Ranking;
class RankingSorter;
}

class Ranking {
    Q_DECLARE_TR_FUNCTIONS(Ranking)

public:
    enum class Type
    {
        INDIVIDUAL,
        CLUB,
    };

    enum class Field
    {
        RTF_FIRST       = 0,
        RTF_FULL_DESCR  = 0,
        RTF_SHORT_DESCR = 1,
        RTF_TEAM        = 2,
        RTF_CATEGORIES  = 3,
        RTF_LAST        = 3,
        RTF_COUNT       = 4
    };

private:
    QString fullDescription { "" };
    QString shortDescription { "" };
    bool team { false };
    QStringList categories { };

public:
    Ranking() = default;
    explicit Ranking(QString const &team);

    friend QDataStream &operator<<(QDataStream &out, Ranking const &ranking);
    friend QDataStream &operator>>(QDataStream &in, Ranking &ranking);

    QString const &getFullDescription() const;
    void setFullDescription(QString const &newFullDescription);
    QString const &getShortDescription() const;
    void setShortDescription(QString const &newShortDescription);
    bool isTeam() const;
    void setTeam(bool newTeam);
    QStringList const &getCategories() const;
    void setCategories(QStringList const &newCategories);
    void parseCategories();

    bool isValid() const;

    bool includes(Category const *category) const;

    bool operator<  (Ranking const &rhs) const;
    bool operator>  (Ranking const &rhs) const;
    bool operator<= (Ranking const &rhs) const;
    bool operator>= (Ranking const &rhs) const;
};

Ranking::Field &operator++(Ranking::Field &field);
Ranking::Field  operator++(Ranking::Field &field, int);

class RankingSorter {

private:
    static Qt::SortOrder  sortingOrder;
    static Ranking::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(Qt::SortOrder const &value);
    static Ranking::Field getSortingField();
    static void setSortingField(Ranking::Field const &value);

    bool operator() (Ranking const &lhs, Ranking const &rhs) const;
};

#endif // RANKING_HPP
