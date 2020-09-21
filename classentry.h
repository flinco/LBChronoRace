#ifndef CLASSENTRY_H
#define CLASSENTRY_H

#include <QCoreApplication>
#include <QVector>
#include <QString>

#include "competitor.h"
#include "timing.h"

namespace ChronoRace {
class ClassEntryElement;
class ClassEntry;
}

class ClassEntryElement {
public:
    Competitor*    competitor;
    uint           time;
    Timing::Status status;
    uint           legRanking;
};

class ClassEntry {
    Q_DECLARE_TR_FUNCTIONS(ClassEntry)

private:
    uint                       bib;
    QVector<ClassEntryElement> entries;
    uint                       totalTime;
    QString                    category;
    static QString             empty;

public:
    ClassEntry();
    ClassEntry(const uint bib);
    uint getBib() const;
    void setBib(uint bib);
    const QString getName(uint legIdx) const;
    const QString getNamesCSV() const;
    const QString getNamesTxt() const;
    uint getYear(uint legIdx) const;
    Competitor::Sex getSex() const;
    Competitor::Sex getSex(uint legIdx) const;
    const QString getTimesCSV() const;
    const QString getTimesTxt(int legRankWidth) const;
    const QString getTime(uint legIdx) const;
    uint getTimeValue(uint legIdx) const;
    uint countEntries() const;
    void setTime(Competitor* comp, const Timing& timing, QStringList &messages);
    uint getLegRanking(uint legIdx) const;
    void setLegRanking(uint legIdx, uint ranking);
    uint getFromYear() const;
    uint getToYear() const;
    const QString& getTeam() const;
    uint getTotalTime() const;
    const QString getTotalTimeCSV() const;
    const QString getTotalTimeTxt() const;
    const QString getDiffTimeTxt(uint referenceTime) const;
    bool isDnf() const;
    bool isDns() const;

    const QString& getCategory() const;
    const QString& getCategory(uint legIdx) const;
    void setCategory(const QString &value);

    static bool compare (const ClassEntry& a, const ClassEntry& b);

    bool operator< (const ClassEntry& rhs);
    bool operator> (const ClassEntry& rhs);
    bool operator<=(const ClassEntry& rhs);
    bool operator>=(const ClassEntry& rhs);
};


#endif // CLASSENTRY_H
