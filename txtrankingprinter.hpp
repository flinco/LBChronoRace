/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#ifndef TXTRANKINGPRINTER_H
#define TXTRANKINGPRINTER_H

#include <QTextStream>
#include <QFile>

#include "rankingprinter.hpp"

class TXTRankingPrinter final : public RankingPrinter
{
    Q_OBJECT
    using RankingPrinter::RankingPrinter;

public:
    void init(QString *outFileName, QString const &title) override;

    void printStartList(QList<Competitor const *> const &startList) override;
    void printRanking(Ranking const &categories, QList<ClassEntry const *> const &ranking) override;
    void printRanking(Ranking const &categories, QList<TeamClassEntry const *> const &ranking) override;

    bool finalize() override;

    QString getFileFilter() override;

private:
    QTextStream txtStream;
    QFile txtFile;

    QString &buildOutFileName(QString &outFileBaseName) override;
    QString &checkOutFileNameExtension(QString &outFileBaseName) override;
};

#endif // TXTRANKINGPRINTER_H
