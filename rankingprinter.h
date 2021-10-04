#ifndef RANKINGPRINTER_H
#define RANKINGPRINTER_H

#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "crloader.h"
#include "chronoracedata.h"
#include "category.h"
#include "classentry.h"
#include "teamclassentry.h"

class RankingPrinter : public QObject
{
    Q_OBJECT
public:
    explicit RankingPrinter(uint indexFieldWidth, uint bibFieldWidth) : indexFieldWidth(indexFieldWidth), bibFieldWidth(bibFieldWidth) { };

    static QScopedPointer<RankingPrinter> getRankingPrinter(CRLoader::Format format, uint indexFieldWidth, uint bibFieldWidth);

    virtual void printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath) = 0;
    virtual void printRanking(Category const &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName) = 0;
    virtual void printRanking(Category const &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName) = 0;

    uint getIndexFieldWidth() const;
    uint getBibFieldWidth() const;

    ChronoRaceData const *getRaceInfo() const;
    void setRaceInfo(ChronoRaceData const *newRaceInfo);

protected:
    virtual QString &buildOutFileName(QString &outFileBaseName) = 0;
    virtual QString &checkOutFileNameExtension(QString &outFileBaseName) = 0;

private:
    uint indexFieldWidth { 0u };
    uint bibFieldWidth { 0u };

    ChronoRaceData const *raceInfo { Q_NULLPTR };

signals:
    void error(QString const &message);
    void info(QString const &message);
};

#endif // RANKINGPRINTER_H
