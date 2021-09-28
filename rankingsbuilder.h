#ifndef RANKINGSBUILDER_H
#define RANKINGSBUILDER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>

#include "lbchronorace.h"
#include "classentry.h"
#include "teamclassentry.h"
#include "category.h"

class RankingsBuilder : public QObject
{
    Q_OBJECT
    using QObject::QObject;

public:
    uint loadData();
    QList<ClassEntry const *> &fillRanking(QList<ClassEntry const *> &ranking, Category const &category) const;
    QList<TeamClassEntry const *> &fillRanking(QList<TeamClassEntry const *> &ranking, Category const &category);

    static QList<Competitor> makeStartList();

private:
    void fillStartList();

    QMultiMap<uint, Competitor const> startList { };
    QMap<uint, ClassEntry> rankingByBib { };
    QList<ClassEntry *> rankingByTime { };
    QMap<QString, TeamClassEntry> rankingByTeam { };


signals:
    void error(QString const &message);
};

#endif // RANKINGSBUILDER_H
