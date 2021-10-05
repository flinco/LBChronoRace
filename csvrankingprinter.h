#ifndef CSVRANKINGPRINTER_H
#define CSVRANKINGPRINTER_H

#include "rankingprinter.h"

class CSVRankingPrinter final  : public RankingPrinter
{
    Q_OBJECT
    using RankingPrinter::RankingPrinter;

public:
    void printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath) override;
    void printRanking(Category const &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName) override;
    void printRanking(Category const &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName) override;

private:
    void printCSVRanking(QList<ClassEntry const *> const &ranking, QTextStream &outStream) const;
    void printCSVRanking(QList<TeamClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const;

    QString &buildOutFileName(QString &outFileBaseName) override;
    QString &checkOutFileNameExtension(QString &outFileBaseName) override;
};

#endif // CSVRANKINGPRINTER_H