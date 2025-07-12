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

#ifndef COMPETITOR_HPP
#define COMPETITOR_HPP

#include <QCoreApplication>
#include <QDataStream>
#include <QString>
#include <QList>

#include "chronoracedata.hpp"
#include "category.hpp"

namespace competitor {
class Competitor;
class CompetitorSorter;
}

class Competitor
{
    Q_DECLARE_TR_FUNCTIONS(Competitor)

public:
    enum class Sex
    {
        UNDEFINED,
        MALE,
        FEMALE
    };

    enum class Field
    {
        CMF_FIRST      = 0,
        CMF_BIB        = 0,
        CMF_SURNAME    = 1,
        CMF_NAME       = 2,
        CMF_SEX        = 3,
        CMF_YEAR       = 4,
        CMF_CLUB       = 5,
        CMF_TEAM       = 6,
        CMF_OFFSET_LEG = 7,
        CMF_LAST       = 7,
        CMF_COUNT      = 8
    };

private:
    static QString empty;

    uint    bib { 0u };
    QString surname { "" };
    QString name { "" };
    Sex     sex { Sex::UNDEFINED };
    uint    year { 1900 };
    QString club { "" };
    QString team { "" };
    uint    leg { 1u };
    int     offset { -1 };
    QList<Category const *> categories { };

    bool isInCategory(Category const *category) const;

public:
    Competitor() = default;
    explicit Competitor(uint const bib) : bib(bib) { };

    QDataStream &cSerialize(QDataStream &out) const;
    friend QDataStream &operator<<(QDataStream &out, Competitor const &data)
    {
        return data.cSerialize(out);
    }

    QDataStream &cDeserialize(QDataStream &in);
    friend QDataStream &operator>>(QDataStream &in, Competitor &data)
    {
        return data.cDeserialize(in);
    }

    QString getCompetitorName(ChronoRaceData::NameComposition nameComposition) const;
    QString getCompetitorName(ChronoRaceData::NameComposition nameComposition, int width) const;
    QString const &getSurname() const;
    void setSurname(QString const &newSurname);
    QString const &getName() const;
    void setName(QString const &newName);
    uint getBib() const;
    void setBib(uint newBib);
    Sex getSex() const;
    void setSex(Sex const newSex);
    QString const &getClub() const;
    QString getClub(int newWidth) const;
    void setClub(QString const &newClub);
    void setClub(QString const *newClub);
    QString const &getTeam() const;
    QString getTeam(int newWidth) const;
    void setTeam(QString const &newTeam);
    void setTeam(QString const *newTeam);
    uint getYear() const;
    void setYear(uint newYear);
    uint getLeg() const;
    void setLeg(uint newLeg);
    int getOffset() const;
    void setOffset(int newOffset, bool maxLeg = false);
    bool isValid() const;

    Category const *getCategory() const;
    QList<Category const *> &getCategories();
    void setCategories(QList<Category> const &newCategories);

    friend auto operator<=>(Competitor const &lhs, Competitor const &rhs)
    {
        if ((lhs.offset < 0) && (lhs.offset != rhs.offset))
            return qAbs(lhs.offset) <=> qAbs(rhs.offset);

        if (lhs.bib == rhs.bib) {
            if (lhs.offset != rhs.offset)
                return qAbs(lhs.offset) <=> qAbs(rhs.offset);
            else
                return lhs.leg <=> rhs.leg;
        }

        return lhs.bib <=> rhs.bib;
    }
};

Competitor::Field &operator++(Competitor::Field &field);
Competitor::Field  operator++(Competitor::Field &field, int);

class CompetitorSorter {

private:
    static Qt::SortOrder     sortingOrder;
    static Competitor::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(Qt::SortOrder const &value);
    static Competitor::Field getSortingField();
    static void setSortingField(Competitor::Field const &value);

    bool operator() (Competitor const &lhs, Competitor const &rhs) const;
};

#endif // COMPETITOR_HPP
