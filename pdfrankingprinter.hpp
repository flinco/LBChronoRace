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

#ifndef PDFRANKINGPRINTER_HPP
#define PDFRANKINGPRINTER_HPP

#include <QString>
#include <QList>
#include <QFont>
#include <QRectF>
#include <QPainter>
#include <QPdfWriter>
#include <QScopedPointer>

#include "rankingprinter.hpp"

class PDFRankingPrinter final : public RankingPrinter
{
    Q_OBJECT

public:
    explicit PDFRankingPrinter(uint indexFieldWidth, uint bibFieldWidth);

    void init(QString *outFileName, QString const &title, QString const &subject) override;

    void printStartList(QList<Competitor const *> const &startList) override;
    void printRanking(Ranking const &categories, QList<ClassEntry const *> const &ranking) override;
    void printRanking(Ranking const &categories, QList<TeamClassEntry const *> const &ranking) override;

    bool finalize() override;

    QString getFileFilter() override;

private:
    enum class RankingType
    {
        START_LIST,
        INDIVIDUAL_SINGLE,
        INDIVIDUAL_MULTI,
        TEAM_SINGLE,
        TEAM_MULTI
    };

    uint currentPage { 0 };
    QPainter painter;
    QScopedPointer<QPdfWriter> writer { Q_NULLPTR };

    QString &buildOutFileName(QString &outFileBaseName) override;
    QString &checkOutFileNameExtension(QString &outFileBaseName) override;

    qreal toHdots(qreal mm) const;
    qreal toVdots(qreal mm) const;
    void fitRectToLogo(QRectF &rect, QPixmap const &pixmap) const;

    QList<QList<Competitor const *>> splitStartList(QList<Competitor const *> const &startList) const;
    QList<QList<Competitor const *>> splitStartListSingleLeg(QList<Competitor const *> const &startList) const;
    QList<QList<Competitor const *>> splitStartListMultiLeg(QList<Competitor const *> const &startList) const;
    QList<QList<ClassEntry const *>> splitIndividualRanking(QList<ClassEntry const *> const ranking) const;
    QList<QList<ClassEntry const *>> splitIndividualRankingSingleLeg(QList<ClassEntry const *> const ranking) const;
    QList<QList<ClassEntry const *>> splitIndividualRankingMultiLeg(QList<ClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRanking(QList<TeamClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRankingSingleLeg(QList<TeamClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRankingMultiLeg(QList<TeamClassEntry const *> const ranking) const;

    void printHeaderSingleLeg(QRectF &writeRect, int page, RankingType type);
    void printHeaderMultiLeg(QRectF &writeRect, int page, RankingType type);

    void printEntrySingleLeg(QRectF &writeRect, ClassEntry const *c, int &posIndex, int cIndex, uint referenceTime, RankingType type);
    void printPageSingleLeg(QRectF &writeRect, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime);
    void printPageSingleLeg(QRectF &writeRect, QList<TeamClassEntry const *> const &page, int &posIndex);
    void printEntryMultiLeg(QRectF &writeRect, ClassEntry const *r, int &posIndex, int rIndex, uint referenceTime, RankingType type);
    void printPageMultiLeg(QRectF &writeRect, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime);
    void printPageMultiLeg(QRectF &writeRect, QList<TeamClassEntry const *> const &page, int &posIndex);

    void drawTemplatePortrait(QString const &fullDescription, int page, int pages, bool startList = false);
    //NOSONAR void drawTemplateLandscape(QString const &fullDescription, int page, int pages, bool startList = false);

    qreal ratioX;
    qreal ratioY;
    qreal areaWidth;
    qreal areaHeight;

    // Fonts
    QFont rnkFont;
    QFont rnkFontItalic;
    QFont rnkFontBold;
    QFont rnkFontBoldItal;
};

#endif // PDFRANKINGPRINTER_HPP
