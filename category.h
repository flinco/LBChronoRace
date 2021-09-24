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
    bool team { false };
    Competitor::Sex sex { Competitor::Sex::UNDEFINED };
    uint toYear { 0u };
    uint fromYear { 0u };
    QString fullDescription { "" };
    QString shortDescription { "" };

public:
    Category() = default;
    explicit Category(const QString& team);

    friend QDataStream &operator<<(QDataStream &out, const Category &category);
    friend QDataStream &operator>>(QDataStream &in, Category &category);

    bool isTeam() const;
    void setTeam(bool newTeam);
    uint getFromYear() const;
    void setFromYear(uint newFromYear);
    const QString& getFullDescription() const;
    void setFullDescription(QString const &newFullDescription);
    Competitor::Sex getSex() const;
    void setSex(Competitor::Sex const newSex);
    const QString& getShortDescription() const;
    void setShortDescription(QString const &newShortDescription);
    uint getToYear() const;
    void setToYear(unsigned int newToYear);
    bool isValid() const;

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
    static void setSortingOrder(Qt::SortOrder const &value);
    static Category::Field getSortingField();
    static void setSortingField(Category::Field const &value);

    bool operator() (Category const &lhs, Category const &rhs) const;
};

#endif // CATEGORY_H
