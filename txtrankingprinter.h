#ifndef TXTRANKINGPRINTER_H
#define TXTRANKINGPRINTER_H

#include <QTextStream>

#include "rankingprinter.h"

class TXTRankingPrinter final : public RankingPrinter
{
    Q_OBJECT
    using RankingPrinter::RankingPrinter;

public:
    void printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath) override;
    void printRanking(Category const &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName) override;
    void printRanking(Category const &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName) override;

private:
    void printTxtRanking(QList<ClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const;
    void printTxtRanking(QList<TeamClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const;

    QString &buildOutFileName(QString &outFileBaseName) override;
    QString &checkOutFileNameExtension(QString &outFileBaseName) override;
};

#endif // TXTRANKINGPRINTER_H
