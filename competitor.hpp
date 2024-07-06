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

#ifndef COMPETITOR_H
#define COMPETITOR_H

#include <QCoreApplication>
#include <QDataStream>
#include <QString>

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
        FEMALE,
        MISC
    };

    enum class Field
    {
        CMF_FIRST      = 0,
        CMF_BIB        = 0,
        CMF_NAME       = 1,
        CMF_SEX        = 2,
        CMF_YEAR       = 3,
        CMF_CLUB       = 4,
        CMF_TEAM       = 5,
        CMF_OFFSET_LEG = 6,
        CMF_LAST       = 6,
        CMF_COUNT      = 7
    };

private:

    uint    bib { 0u };
    QString name { "" };
    Sex     sex { Sex::UNDEFINED };
    uint    year { 1900 };
    QString club { "" };
    QString team { "" };
    uint    leg { 1u };
    int     offset { -1 };
    QString category { "" };

public:
    Competitor() = default;
    explicit Competitor(uint const bib) : bib(bib) { };

    friend QDataStream &operator<<(QDataStream &out, Competitor const &comp);
    friend QDataStream &operator>>(QDataStream &in, Competitor &comp);

    QString const &getName() const;
    QString getName(int width) const;
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
    void setOffset(int newOffset);
    void setOffset(int const *newOffset);
    bool isValid() const;

    QString const &getCategory() const;
    void setCategory(QString const &newCategory);

    static Sex toSex(QString const &sex, bool const strict = false);
    static QString toSexString(Sex const sex);
    static int toOffset(QString const &offset);
    static QString toOffsetString(int offset);

    bool operator<  (Competitor const &rhs) const;
    bool operator>  (Competitor const &rhs) const;
    bool operator<= (Competitor const &rhs) const;
    bool operator>= (Competitor const &rhs) const;
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

#endif // COMPETITOR_H
