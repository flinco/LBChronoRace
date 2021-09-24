#ifndef TEAMCLASSENTRY_H
#define TEAMCLASSENTRY_H

#include <QCoreApplication>
#include <QVector>

#include "classentry.h"

namespace placement {
class TeamClassEntry;
}

class TeamClassEntry
{
    Q_DECLARE_TR_FUNCTIONS(TeamClassEntry)

private:
    QString              team { "" };
    QVector<ClassEntry*> entryList { };

public:
    const QString& getTeam() const;
    const ClassEntry* getClassEntry(int index) const;
    void setClassEntry(ClassEntry* entry);
    int getClassEntryCount() const;

    bool operator< (const TeamClassEntry& rhs);
    bool operator> (const TeamClassEntry& rhs);
    bool operator<=(const TeamClassEntry& rhs);
    bool operator>=(const TeamClassEntry& rhs);
};

#endif // TEAMCLASSENTRY_H
