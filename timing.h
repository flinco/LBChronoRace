#ifndef TIMING_H
#define TIMING_H

#include <QCoreApplication>
#include <QtGlobal>
#include <QString>

namespace ChronoRace {
class Timing;
class TimingSorter;
}

class Timing {
    Q_DECLARE_TR_FUNCTIONS(Timing)

public:
    enum Status
        {
            DNS,
            DNF,
            CLASSIFIED
        };

    enum Field
        {
            TMF_FIRST = 0,
            TMF_BIB   = 0,
            TMF_TIME  = 1,
            TMF_LAST  = 1,
            TMF_COUNT = 2
        };

private:
    uint   bib;
    uint   seconds;
    Status status;

public:
    Timing();
    Timing(const uint bib);
    uint getBib() const;
    void setBib(uint bib);
    bool isDnf() const;
    bool isDns() const;
    Status getStatus() const;
    uint getSeconds() const;
    QString getTiming() const;
    void setTiming(const QString& timing);
    void setTiming(const char* timing);
    bool isValid();

    static const QString toTimeStr(const uint seconds, const Timing::Status status);
    static const QString toTimeStr(const Timing& timing);

    bool operator<  (const Timing& rhs) const;
    bool operator>  (const Timing& rhs) const;
    bool operator<= (const Timing& rhs) const;
    bool operator>= (const Timing& rhs) const;
};

Timing::Field& operator++(Timing::Field& field);
Timing::Field  operator++(Timing::Field& field, int);

class TimingSorter {

private:
    static Qt::SortOrder sortingOrder;
    static Timing::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(const Qt::SortOrder &value);
    static Timing::Field getSortingField();
    static void setSortingField(const Timing::Field &value);

    bool operator() (const Timing& lhs, const Timing& rhs);
};

#endif // TIMING_H
