#ifndef COMPETITOR_H
#define COMPETITOR_H

#include <QCoreApplication>
#include <QDataStream>
#include <QSet>
#include <QString>

namespace ChronoRace {
class Competitor;
class CompetitorSorter;
}

class Competitor
{
    Q_DECLARE_TR_FUNCTIONS(Competitor)

public:
    enum Sex
        {
            UNDEFINED,
            MALE,
            FEMALE,
            MISC
        };

    enum Field
        {
            CMF_FIRST  = 0,
            CMF_BIB    = 0,
            CMF_NAME   = 1,
            CMF_SEX    = 2,
            CMF_YEAR   = 3,
            CMF_TEAM   = 4,
            CMF_OFFSET = 5,
            CMF_LAST   = 5,
            CMF_COUNT  = 6
        };

private:

    uint    bib;
    QString name;
    Sex     sex;
    uint    year;
    QString team;
    uint    leg;
    int     offset;

public:
    Competitor();
    Competitor(const uint bib);

    friend QDataStream &operator<<(QDataStream &out, const Competitor &comp);
    friend QDataStream &operator>>(QDataStream &in, Competitor &comp);

    const QString& getName() const;
    const QString  getName(int width) const;
    void setName(const QString& name);
    uint getBib() const;
    void setBib(uint bib);
    Sex getSex() const;
    void setSex(const Sex sex);
    const QString& getTeam() const;
    const QString  getTeam(int width) const;
    void setTeam(const QString& team);
    uint getYear() const;
    void setYear(uint year);
    uint getLeg() const;
    void setLeg(uint leg);
    int getOffset() const;
    void setOffset(int offset);
    bool isValid();

    static Sex toSex(const QString& sex, const bool strict = false);
    static QString toSexString(const Sex sex);
    static int toOffset(const QString& offset);
    static QString toOffsetString(int offset);

    bool operator<  (const Competitor& rhs) const;
    bool operator>  (const Competitor& rhs) const;
    bool operator<= (const Competitor& rhs) const;
    bool operator>= (const Competitor& rhs) const;
};

Competitor::Field& operator++(Competitor::Field& field);
Competitor::Field  operator++(Competitor::Field& field, int);

class CompetitorSorter {

private:
    static Qt::SortOrder     sortingOrder;
    static Competitor::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(const Qt::SortOrder &value);
    static Competitor::Field getSortingField();
    static void setSortingField(const Competitor::Field &value);

    bool operator() (const Competitor& lhs, const Competitor& rhs);
};

#endif // COMPETITOR_H
