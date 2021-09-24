#ifndef TIMING_H
#define TIMING_H

#include <QCoreApplication>
#include <QtGlobal>
#include <QDataStream>
#include <QString>

namespace timing {
class Timing;
class TimingSorter;
}

class Timing {
    Q_DECLARE_TR_FUNCTIONS(Timing)

public:
    enum class Status
    {
        DNS,
        DNF,
        CLASSIFIED
    };

    enum class Field
    {
        TMF_FIRST = 0,
        TMF_BIB   = 0,
        TMF_LEG   = 1,
        TMF_TIME  = 2,
        TMF_LAST  = 2,
        TMF_COUNT = 3
    };

private:
    uint   bib { 0u };
    uint   leg { 0u };
    uint   seconds { 0u };
    Status status { Status::CLASSIFIED};

public:
    Timing() = default;
    explicit Timing(const uint bib) : bib(bib) {  };

    friend QDataStream &operator<<(QDataStream &out, const Timing &timing);
    friend QDataStream &operator>>(QDataStream &in, Timing &timing);

    uint getBib() const;
    void setBib(uint newBib);
    uint getLeg() const;
    void setLeg(uint newLeg);
    bool isDnf() const;
    bool isDns() const;
    Status getStatus() const;
    uint getSeconds() const;
    QString getTiming() const;
    void setTiming(QString const &timing);
    void setTiming(char const *timing);
    bool isValid() const;

    static QString toTimeStr(uint const seconds, Timing::Status const status, char const *prefix = Q_NULLPTR);
    static QString toTimeStr(Timing const &timing);

    bool operator<  (Timing const &rhs) const;
    bool operator>  (Timing const &rhs) const;
    bool operator<= (Timing const &rhs) const;
    bool operator>= (Timing const &rhs) const;
};

Timing::Field& operator++(Timing::Field& field);
Timing::Field  operator++(Timing::Field& field, int);

class TimingSorter {

private:
    static Qt::SortOrder sortingOrder;
    static Timing::Field sortingField;

public:
    static Qt::SortOrder getSortingOrder();
    static void setSortingOrder(Qt::SortOrder const &value);
    static Timing::Field getSortingField();
    static void setSortingField(Timing::Field const &value);

    bool operator() (Timing const &lhs, Timing const &rhs) const;
};

#endif // TIMING_H
