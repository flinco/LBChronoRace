#ifndef CATEGORY_H
#define CATEGORY_H

#include <QCoreApplication>
#include <QDataStream>

#include "competitor.h"

namespace ChronoRace {
class Category;
class CategorySorter;
}

class Category {
    Q_DECLARE_TR_FUNCTIONS(Category)

public:
    enum Field
        {
            CTF_FIRST       = 0,
            CTF_TEAM        = 0,
            CTF_SEX         = 1,
            CTF_TO_YEAR     = 2,
            CTF_FROM_YEAR   = 3,
            CTF_FULL_DESCR  = 4,
            CTF_SHORT_DESCR = 5,
            CTF_LAST        = 5,
            CTF_COUNT       = 6
        };

private:
    bool            team;
    Competitor::Sex sex;
    uint            toYear;
    uint            fromYear;
    QString         fullDescription;
    QString         shortDescription;

public:
    Category();
    Category(const QString& team);

    friend QDataStream &operator<<(QDataStream &out, const Category &category);
    friend QDataStream &operator>>(QDataStream &in, Category &category);

    bool isTeam() const;
    void setTeam(bool team);
    uint getFromYear() const;
    void setFromYear(uint fromYear);
    const QString& getFullDescription() const;
    void setFullDescription(const QString& fullDescription);
    Competitor::Sex getSex() const;
    void setSex(const Competitor::Sex sex);
    const QString& getShortDescription() const;
    void setShortDescription(const QString& shortDescription);
    uint getToYear() const;
    void setToYear(unsigned int toYear);
    bool isValid();

    bool operator<  (const Category& rhs) const;
    bool operator>  (const Category& rhs) const;
    bool operator<= (const Category& rhs) const;
    bool operator>= (const Category& rhs) const;
};

Category::Field& operator++(Category::Field& field);
Category::Field  operator++(Category::Field& field, int);

class CategorySorter {

private:
    static Qt::SortOrder   sortingOrder;
    static Category::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(const Qt::SortOrder &value);
    static Category::Field getSortingField();
    static void setSortingField(const Category::Field &value);

    bool operator() (const Category& lhs, const Category& rhs);
};

#endif // CATEGORY_H
