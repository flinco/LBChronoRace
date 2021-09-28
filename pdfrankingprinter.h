#ifndef PDFRANKINGPRINTER_H
#define PDFRANKINGPRINTER_H

#include <QString>
#include <QList>
#include <QFont>
#include <QRectF>
#include <QPainter>
#include <QPdfWriter>
#include <QScopedPointer>

#include "rankingprinter.h"

class PDFRankingPrinter final : public RankingPrinter
{
    Q_OBJECT

public:
    explicit PDFRankingPrinter(uint indexFieldWidth, uint bibFieldWidth);

    void printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath) override;
    void printRanking(Category const &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName) override;
    void printRanking(Category const &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName) override;

private:
    enum class RankingType {
        START_LIST,
        INDIVIDUAL_SINGLE,
        INDIVIDUAL_MULTI,
        TEAM_SINGLE,
        TEAM_MULTI
    };

    QString &buildOutFileName(QString &outFileBaseName) override;
    QString &checkOutFileNameExtension(QString &outFileBaseName) override;

    qreal toHdots(qreal mm) const;
    qreal toVdots(qreal mm) const;
    void fitRectToLogo(QRectF &rect, QPixmap const &pixmap) const;

    QList<QList<Competitor const *>> splitStartList(QList<Competitor> const &startList) const;
    QList<QList<Competitor const *>> splitStartListSingleLeg(QList<Competitor> const &startList) const;
    QList<QList<Competitor const *>> splitStartListMultiLeg(QList<Competitor> const &startList) const;
    QList<QList<ClassEntry const *>> splitIndividualRanking(QList<ClassEntry const *> const ranking) const;
    QList<QList<ClassEntry const *>> splitIndividualRankingSingleLeg(QList<ClassEntry const *> const ranking) const;
    QList<QList<ClassEntry const *>> splitIndividualRankingMultiLeg(QList<ClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRanking(QList<TeamClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRankingSingleLeg(QList<TeamClassEntry const *> const ranking) const;
    QList<QList<TeamClassEntry const *>> splitTeamRankingMultiLeg(QList<TeamClassEntry const *> const ranking) const;

    void printHeaderSingleLeg(QRectF &writeRect, QPainter &painter, int page, RankingType type) const;
    void printHeaderMultiLeg(QRectF &writeRect, QPainter &painter, int page, RankingType type) const;

    void printEntrySingleLeg(QRectF &writeRect, QPainter &painter, ClassEntry const *c, int &posIndex, int cIndex, uint referenceTime, RankingType type) const;
    void printPageSingleLeg(QRectF &writeRect, QPainter &painter, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime) const;
    void printPageSingleLeg(QRectF &writeRect, QPainter &painter, QList<TeamClassEntry const *> const &page, int &posIndex) const;
    void printEntryMultiLeg(QRectF &writeRect, QPainter &painter, ClassEntry const *c, int &posIndex, int tIndex, uint referenceTime, RankingType type) const;
    void printPageMultiLeg(QRectF &writeRect, QPainter &painter, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime) const;
    void printPageMultiLeg(QRectF &writeRect, QPainter &painter, QList<TeamClassEntry const *> const &page, int &posIndex) const;

    void makeStartList(QPainter &painter, QList<Competitor> const &startList);
    void makeRanking(QPainter &painter, QString const &fullDescription, QList<ClassEntry const *> const &ranking, bool multiLeg);
    void makeRanking(QPainter &painter, QString const &fullDescription, QList<TeamClassEntry  const*> const &ranking, bool multiLeg);

    void drawTemplatePortrait(QPainter &painter, QString const &fullDescription, int page, int pages);
    //NOSONAR void drawTemplateLandscape(QPainter &painter, QString const &fullDescription, int page, int pages);

    bool initPainter(QPainter &painter, QString const &outFileName);

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

#endif // PDFRANKINGPRINTER_H
