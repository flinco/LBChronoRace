#ifndef COMPETITOR_H
#define COMPETITOR_H

#include <QCoreApplication>
#include <QDataStream>
#include <QSet>
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
        CMF_TEAM       = 4,
        CMF_OFFSET_LEG = 5,
        CMF_LAST       = 5,
        CMF_COUNT      = 6
    };

private:

    uint    bib { 0u };
    QString name { "" };
    Sex     sex { Sex::UNDEFINED };
    uint    year { 1900 };
    QString team { "" };
    uint    leg { 1u };
    int     offset { -1 };
    QString category { "" };

public:
    Competitor() = default;
    explicit Competitor(const uint bib) : bib(bib) { };

    friend QDataStream &operator<<(QDataStream &out, const Competitor &comp);
    friend QDataStream &operator>>(QDataStream &in, Competitor &comp);

    QString const &getName() const;
    QString getName(int width) const;
    void setName(const QString& newName);
    uint getBib() const;
    void setBib(uint newBib);
    Sex getSex() const;
    void setSex(const Sex newSex);
    QString const &getTeam() const;
    QString getTeam(int newWidth) const;
    void setTeam(const QString& newTeam);
    uint getYear() const;
    void setYear(uint newYear);
    uint getLeg() const;
    void setLeg(uint newLeg);
    int getOffset() const;
    void setOffset(int newOffset);
    bool isValid() const;

    const QString& getCategory() const;
    void setCategory(QString const &newCategory);

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

    bool operator() (const Competitor& lhs, const Competitor& rhs) const;
};

#endif // COMPETITOR_H
