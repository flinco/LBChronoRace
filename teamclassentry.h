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
    QVector<ClassEntry *> entryList { };

public:
    QString const &getTeam() const;
    ClassEntry const *getClassEntry(int index) const;
    void setClassEntry(ClassEntry* entry);
    int getClassEntryCount() const;

    bool operator< (TeamClassEntry const &rhs);
    bool operator> (TeamClassEntry const &rhs);
    bool operator<=(TeamClassEntry const &rhs);
    bool operator>=(TeamClassEntry const &rhs);
};

#endif // TEAMCLASSENTRY_H
