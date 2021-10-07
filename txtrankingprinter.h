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
