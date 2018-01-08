#ifndef CLASSENTRY_H
#define CLASSENTRY_H

#include <QCoreApplication>
#include <QVector>
#include <QString>

#include "competitor.h"
#include "timing.h"

namespace ChronoRace {
class ClassEntry;
}

class ClassEntry {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry)

private:
    uint                    bib;
    QVector<Competitor*>    competitors;
    QVector<uint>           times;
    QVector<Timing::Status> states;
    QVector<uint>           legRanking;
    uint                    totalTime;

public:
    ClassEntry();
    ClassEntry(const uint bib);
    uint getBib() const;
    void setBib(uint bib);
    const QString getNamesCSV() const;
    const QString getNamesTxt() const;
    Competitor::Sex getSex() const;
    const QString getTimesCSV() const;
    const QString getTimesTxt(int legRankWidth) const;
    uint getTime(uint i) const;
    uint countTimes() const;
    void setTime(Competitor* comp, const Timing& timing);
    void setLegRanking(const uint ranking);
    uint getFromYear() const;
    uint getToYear() const;
    const QString& getTeam() const;
    uint getTotalTime() const;
    const QString getTotalTimeCSV() const;
    const QString getTotalTimeTxt() const;
    bool isDnf() const;
    bool isDns() const;

    static bool compare (const ClassEntry& a, const ClassEntry& b);

    bool operator< (const ClassEntry& rhs);
    bool operator> (const ClassEntry& rhs);
    bool operator<=(const ClassEntry& rhs);
    bool operator>=(const ClassEntry& rhs);
};


#endif // CLASSENTRY_H
