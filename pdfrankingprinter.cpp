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

#include <QLocale>
#include <QPointF>
#include <QFontDatabase>
#include <QUuid>

#include "pdfrankingprinter.hpp"
#include "chronoracedata.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

static constexpr qreal RANKING_TOP_MARGIN    = 10.0;
static constexpr qreal RANKING_LEFT_MARGIN   = 10.0;
static constexpr qreal RANKING_RIGHT_MARGIN  = 10.0;
static constexpr qreal RANKING_BOTTOM_MARGIN =  6.0;

static constexpr int RANKING_PORTRAIT_FIRST_PAGE_LIMIT  = 44;
static constexpr int RANKING_PORTRAIT_SECOND_PAGE_LIMIT = 50;

//NOSONAR static constexpr int RANKING_LANDSCAPE_FIRST_PAGE_LIMIT  = 23;
//NOSONAR static constexpr int RANKING_LANDSCAPE_SECOND_PAGE_LIMIT = 29;

PDFRankingPrinter::PDFRankingPrinter(uint indexFieldWidth, uint bibFieldWidth) : RankingPrinter(indexFieldWidth, bibFieldWidth)
{
    // Fonts
    this->rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
    this->rnkFontItalic = QFontDatabase::font("Liberation Sans", "Italic", 14);
    this->rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
    this->rnkFontBoldItal = QFontDatabase::font("Liberation Sans", "Bold Italic", 22);
}


void PDFRankingPrinter::init(QString *outFileName, QString const &title, QString const &subject)
{
    QPdfWriter *w;

    Q_ASSERT(!painter.isActive());

    if (outFileName == Q_NULLPTR) {
        emit error(tr("Error: no file name supplied"));
        return;
    }

    // append the .pdf extension if missing
    checkOutFileNameExtension(*outFileName);

    if (writer.isNull())
        writer.reset(new QPdfWriter(*outFileName));

    w = writer.data();
    w->setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);
    w->setPageSize(QPageSize::A4);
    w->setPageOrientation(QPageLayout::Portrait);
    w->setPageMargins(QMarginsF(RANKING_LEFT_MARGIN, RANKING_TOP_MARGIN, RANKING_RIGHT_MARGIN, RANKING_BOTTOM_MARGIN), QPageLayout::Millimeter);
    //NOSONAR w->setTitle(title);
    //NOSONAR w->setCreator(LBCHRONORACE_NAME);

    // append more details to the .pdf
    QString metaData = QStringLiteral(
        "<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#' xmlns:iX='http://ns.adobe.com/iX/1.0/'>"
        ""
        "  <rdf:Description rdf:about='' xmlns:pdf='http://ns.adobe.com/pdf/1.3/'>"
        "    <pdf:Producer>Qt %1</pdf:Producer>"
        "    <pdf:Keywords>%2</pdf:Keywords>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:photoshop='http://ns.adobe.com/photoshop/1.0/'>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:tiff='http://ns.adobe.com/tiff/1.0/'>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:xap='http://ns.adobe.com/xap/1.0/'>"
        "    <xap:CreatorTool>%3</xap:CreatorTool>"
        "    <xap:CreateDate>%4</xap:CreateDate>"
        "    <xap:ModifyDate>%4</xap:ModifyDate>"
        "    <xap:MetadataDate>%4</xap:MetadataDate>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:xapMM='http://ns.adobe.com/xap/1.0/mm/'>"
        "    <xapMM:DocumentID>uuid:%5</xapMM:DocumentID>"
        "    <xapMM:InstanceID>uuid:%6</xapMM:InstanceID>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:dc='http://purl.org/dc/elements/1.1/'>"
        "    <dc:format>application/pdf</dc:format>"
        "    <dc:title>"
        "      <rdf:Alt>"
        "        <rdf:li xml:lang='x-default'>%7</rdf:li>"
        "      </rdf:Alt>"
        "    </dc:title>"
        "    <dc:description>"
        "      <rdf:Alt>"
        "        <rdf:li xml:lang='x-default'>%8</rdf:li>"
        "      </rdf:Alt>"
        "    </dc:description>"
        "    <dc:creator>"
        "      <rdf:Seq>"
        "        <rdf:li>%3</rdf:li>"
        "      </rdf:Seq>"
        "    </dc:creator>"
        "     <dc:subject>"
        "      <rdf:Bag>"
        "        <rdf:li>%2</rdf:li>"
        "      </rdf:Bag>"
        "    </dc:subject>"
        "  </rdf:Description>"
        ""
        "  <rdf:Description rdf:about='' xmlns:pdfaid='http://www.aiim.org/pdfa/ns/id/'>"
        "    <pdfaid:part>1</pdfaid:part>"
        "    <pdfaid:conformance>B</pdfaid:conformance>"
        "  </rdf:Description>"
        ""
        "</rdf:RDF>"
        ).arg(QT_VERSION_STR,
              tr("Rankings"), // keywords (can be a comma separated)
              LBCHRONORACE_NAME,
              QDateTime::currentDateTime().toString(Qt::ISODate),
              QUuid::createUuid().toByteArray(QUuid::WithoutBraces),
              QUuid::createUuid().toByteArray(QUuid::WithoutBraces),
              title,
              subject);
    w->setDocumentXmpMetadata(metaData.toUtf8());

    // Set global values to convert from mm to dots
    this->ratioX = w->logicalDpiX() / 25.4;
    this->ratioY = w->logicalDpiY() / 25.4;
    //NOSONAR this->ratioX = w->physicalDpiX() / 25.4;
    //NOSONAR this->ratioY = w->physicalDpiY() / 25.4;
    this->areaWidth = w->width();
    this->areaHeight = w->height();

    currentPage = 0;
    if (!painter.begin(w)) {
        emit error(tr("Error: cannot start drawing"));
        return;
    }
}

void PDFRankingPrinter::printStartList(QList<Competitor const *> const &startList)
{
    if (!painter.isActive()) {
        emit error(tr("Error: drawing attempt on inactive painter"));
        return;
    }

    Category const *category = Q_NULLPTR;

    QRectF writeRect;

    ChronoRaceData const *raceInfo = getRaceInfo();
    QTime startTime = raceInfo->getStartTime();

    QStringList clubAndTeam { };

    auto pdfWriter = static_cast<QPdfWriter *>(painter.device());

    // Split the list into pages
    QList<QList<Competitor const *>> pages = splitStartList(startList);
    int offset;
    int i = 1;
    int p = 1;
    auto pp = static_cast<int>(pages.size());
    for (auto page = pages.constBegin(); page < pages.constEnd(); page++, p++) {

        if (currentPage++) // this is not the first loop, add a new pages
            pdfWriter->newPage();

        drawTemplatePortrait(tr("Start List"), p, pp, true);

        // Prepare fonts
        rnkFont.setPointSize(7);
        rnkFontBold.setPointSize(7);
        painter.setFont(rnkFontBold);

        // Rankings header
        printHeaderSingleLeg(writeRect, p, RankingType::START_LIST);

        writeRect.setTop(toVdots((p == 1) ? 57.0 : 34.0));
        writeRect.setHeight(toVdots(4.0));
        for (auto c = (*page).constBegin(); c < (*page).constEnd(); c++) {

            if (*c == Q_NULLPTR) {// separator
                // move down and go to the next competitor
                writeRect.translate(0.0, toVdots(4.0));
                continue;
            }

            category = (*c)->getCategory();

            clubAndTeam.clear();
            clubAndTeam.append((*c)->getClub());
            clubAndTeam.append((*c)->getTeam());
            clubAndTeam.removeAll("");

            // Move down
            writeRect.translate(0.0, toVdots(4.0));
            writeRect.setLeft(toHdots(0.0));
            // Set the background
            if ((i % 2) == 0) {
                writeRect.setWidth(this->areaWidth);
                painter.fillRect(writeRect, Qt::lightGray);
            }
            // Ranking placement
            painter.setFont(rnkFontBold);
            writeRect.setLeft(toHdots(1.0));
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(i));
            i++;
            // Bib
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(7.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number((*c)->getBib()));
            // Name
            painter.setFont(rnkFontBold);
            writeRect.translate(toHdots(8.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getCompetitorName(CRHelper::nameComposition, 0));
            // Team
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(45.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, clubAndTeam.join(" - "));
            // Year
            writeRect.translate(toHdots(45.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number((*c)->getYear()));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, CRHelper::toSexString((*c)->getSex()));
            // Category
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(28.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, category ? category->getFullDescription() : "---");
            // Start Time / Leg
            offset = (*c)->getOffset();
            painter.setFont(rnkFontBold);
            writeRect.translate(toHdots(28.0), 0.0);
            writeRect.setWidth(toHdots(27.0));
            if (offset >= 0) {
                offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, CRHelper::toOffsetString(offset));
            } else if (CRLoader::getStartListLegs() == 1) {
                offset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, CRHelper::toOffsetString(offset));
            } else {
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, tr("Leg %n", "", qAbs(offset)));
            }
        }
    }
}

void PDFRankingPrinter::printRanking(Ranking const &categories, QList<ClassEntry const *> const &ranking)
{
    if (!painter.isActive()) {
        emit error(tr("Error: drawing attempt on inactive painter"));
        return;
    }

    uint individualLegs = CRLoader::getStartListLegs();

    if (individualLegs == 0) {
        emit error(tr("Error: cannot generate results for 0 legs"));
        return;
    }

    uint referenceTime = 0u;
    QRectF writeRect;

    auto pdfWriter = static_cast<QPdfWriter *>(painter.device());

    // Split the list into pages
    QList<QList<ClassEntry const *>> pages = splitIndividualRanking(ranking);

    // Initialize the reference time
    if (!pages.empty() && !pages.at(0).empty())
        referenceTime = pages.at(0).at(0)->getTotalTime();

    int i = 1;
    int p = 1;
    auto pp = static_cast<int>(pages.size());
    for (auto page = pages.constBegin(); page < pages.constEnd(); page++, p++) {

        if (currentPage++) // this is not the first loop, add a new page
            pdfWriter->newPage();

        drawTemplatePortrait(tr("%1 Results").arg(categories.getFullDescription()), p, pp);

        // Prepare fonts
        rnkFont.setPointSize(7);
        rnkFontBold.setPointSize(7);
        painter.setFont(rnkFontBold);

        // Rankings header
        if (individualLegs != 1) // multileg
            printHeaderMultiLeg(writeRect, p, RankingType::INDIVIDUAL_MULTI);
        else
            printHeaderSingleLeg(writeRect, p, RankingType::INDIVIDUAL_SINGLE);

        // Page content
        writeRect.setTop(toVdots((p == 1) ? 57.0 : 34.0));
        writeRect.setHeight(toVdots(4.0));
        if (individualLegs != 1) // multileg
            printPageMultiLeg(writeRect, *page, i, referenceTime);
        else
            printPageSingleLeg(writeRect, *page, i, referenceTime);
    }
}

void PDFRankingPrinter::printRanking(Ranking const &categories, QList<TeamClassEntry const *> const &ranking)
{
    if (!painter.isActive()) {
        emit error(tr("Error: drawing attempt on inactive painter"));
        return;
    }

    uint teamLegs = CRLoader::getStartListLegs();

    if (teamLegs == 0) {
        emit error(tr("Error: cannot generate results for 0 legs"));
        return;
    }

    QRectF writeRect;

    auto pdfWriter = static_cast<QPdfWriter *>(painter.device());

    // Split the list into pages
    QList<QList<TeamClassEntry const *>> pages = splitTeamRanking(ranking);
    int j = 1;
    int p = 1;
    auto pp = static_cast<int>(pages.size());
    for (auto page = pages.constBegin(); page < pages.constEnd(); page++, p++) {

        if (currentPage++) // this is not the first loop, add a new pages
            pdfWriter->newPage();

        drawTemplatePortrait(tr("%1 Results").arg(categories.getFullDescription()), p, pp);

        // Prepare fonts
        rnkFont.setPointSize(7);
        rnkFontBold.setPointSize(7);
        painter.setFont(rnkFontBold);

        // Rankings header
        if (teamLegs != 1) // multileg
            printHeaderMultiLeg(writeRect, p, RankingType::TEAM_MULTI);
        else
            printHeaderSingleLeg(writeRect, p, RankingType::TEAM_SINGLE);

        // Page content
        writeRect.setTop(toVdots((p == 1) ? 57.0 : 34.0));
        writeRect.setHeight(toVdots(4.0));
        if (teamLegs != 1) // multileg
            printPageMultiLeg(writeRect, *page, j);
        else
            printPageSingleLeg(writeRect, *page, j);
    }
}

bool PDFRankingPrinter::finalize()
{
    bool result = false;

    if (!painter.isActive())
        result = true;
    else if (painter.end())
        result = true;
    else
        emit error(tr("Cannot write to PDF"));

    if (!writer.isNull())
        writer.reset(Q_NULLPTR);

    return result;
}

QString PDFRankingPrinter::getFileFilter()
{
    return tr("PDF (*.pdf)");
}

QString &PDFRankingPrinter::buildOutFileName(QString &outFileBaseName)
{
    return outFileBaseName.append(".pdf");
}

QString &PDFRankingPrinter::checkOutFileNameExtension(QString &outFileBaseName)
{
    return (!outFileBaseName.endsWith(".pdf", Qt::CaseInsensitive)) ? buildOutFileName(outFileBaseName) : outFileBaseName;
}

qreal PDFRankingPrinter::toHdots(qreal mm) const
{
    qreal dots = mm * this->ratioX;
    return (mm >= 0.0) ? dots : this->areaWidth + dots;
}

qreal PDFRankingPrinter::toVdots(qreal mm) const
{
    qreal dots = mm * this->ratioY;
    return (mm >= 0.0) ? dots : this->areaHeight + dots;
}

void PDFRankingPrinter::fitRectToLogo(QRectF &rect, QPixmap const &pixmap) const
{
    qreal shift;
    QSizeF pixmapSize(pixmap.size());
    pixmapSize.scale(rect.width(), rect.height(), Qt::KeepAspectRatio);
    if (rect.width() > pixmapSize.width()) {
        shift = rect.x() + ((rect.width() - pixmapSize.width()) / 2.0);
        rect.setX(shift);
        rect.setWidth(pixmapSize.width());
    } else {
        shift = rect.y() + ((rect.height() - pixmapSize.height()) / 2.0);
        rect.setY(shift);
        rect.setHeight(pixmapSize.height());
    }
}

QList<QList<Competitor const *>> PDFRankingPrinter::splitStartList(QList<Competitor const *> const &startList) const
{
    return (CRLoader::getStartListLegs() > 1) ? splitStartListMultiLeg(startList) : splitStartListSingleLeg(startList);
}

QList<QList<Competitor const *>> PDFRankingPrinter::splitStartListSingleLeg(QList<Competitor const *> const &startList) const
{
    QList<QList<Competitor const *>> pages;

    pages.emplaceBack();
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
    for (auto c = startList.constBegin(); c < startList.constEnd(); c++) {
        if (availableEntriesOnPage <= 0) {
            // go to a new page
            pages.emplaceBack();
            availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
        }
        availableEntriesOnPage--;
        pages.last().append(*c);
    }

    return pages;
}

QList<QList<Competitor const *>> PDFRankingPrinter::splitStartListMultiLeg(QList<Competitor const *> const &startList) const
{
    QList<QList<Competitor const *>> pages;
    int offset;

    pages.emplaceBack();
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;

    auto c = startList.constBegin();
    int prevOffset = (c < startList.constEnd()) ? (*c)->getOffset() : 0;
    while (c < startList.constEnd()) {
        if ((offset = (*c)->getOffset()) < 0) {
            if (prevOffset != offset) {
                availableEntriesOnPage--;
                pages.last().append(Q_NULLPTR); // separator
            }
            prevOffset = offset;
        }

        if (availableEntriesOnPage <= 0) {
            // go to a new page
            pages.emplaceBack();
            availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
        }

        availableEntriesOnPage--;
        pages.last().append(*c);
        c++;
    }

    return pages;
}

QList<QList<ClassEntry const *>> PDFRankingPrinter::splitIndividualRanking(QList<ClassEntry const *> const ranking) const
{
    return (CRLoader::getStartListLegs() > 1) ? splitIndividualRankingMultiLeg(ranking) : splitIndividualRankingSingleLeg(ranking);
}

QList<QList<ClassEntry const *>> PDFRankingPrinter::splitIndividualRankingSingleLeg(QList<ClassEntry const *> const ranking) const
{
    QList<QList<ClassEntry const *>> pages;

    pages.emplaceBack();
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
    for (auto c = ranking.constBegin(); c < ranking.constEnd(); c++) {
        if (availableEntriesOnPage <= 0) {
            // go to a new page
            pages.emplaceBack();
            availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
        }
        availableEntriesOnPage--;
        pages.last().append(*c);
    }

    return pages;
}

QList<QList<ClassEntry const *>> PDFRankingPrinter::splitIndividualRankingMultiLeg(QList<ClassEntry const *> const ranking) const
{
    QList<QList<ClassEntry const *>> pages;

    pages.emplaceBack();
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;

    auto entriesPerBlock = static_cast<int>(CRLoader::getStartListLegs() + 1);
    for (auto c = ranking.constBegin(); c < ranking.constEnd(); c++) {
        if (entriesPerBlock > availableEntriesOnPage) {
            // go to a new page
            pages.emplaceBack();
            availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
        }
        availableEntriesOnPage -= entriesPerBlock;
        pages.last().append(*c);
    }

    return pages;
}

QList<QList<TeamClassEntry const *>> PDFRankingPrinter::splitTeamRanking(QList<TeamClassEntry const *> const ranking) const
{
    return (CRLoader::getStartListLegs() > 1) ? splitTeamRankingMultiLeg(ranking) : splitTeamRankingSingleLeg(ranking);
}

QList<QList<TeamClassEntry const *>> PDFRankingPrinter::splitTeamRankingSingleLeg(QList<TeamClassEntry const *> const ranking) const
{
    QList<QList<TeamClassEntry const *>> pages;

    pages.emplaceBack();
    int count;
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
    for (auto t = ranking.constBegin(); t < ranking.constEnd(); t++) {
        count = (*t)->getClassEntryCount();
        for (int j = 0; j < count; j++) {
            if (availableEntriesOnPage <= 0) {
                pages.emplaceBack();
                availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
            }
            availableEntriesOnPage--;
            pages.last().append(*t);
        }
        // Leave a blank line before next team (but not at page bottom)
        availableEntriesOnPage--;
        pages.last().append(Q_NULLPTR);
    }

    return pages;
}


QList<QList<TeamClassEntry const *>> PDFRankingPrinter::splitTeamRankingMultiLeg(QList<TeamClassEntry const *> const ranking) const
{
    QList<QList<TeamClassEntry const *>> pages;

    pages.emplaceBack();
    int count;
    int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
    auto entriesPerBlock = static_cast<int>(CRLoader::getStartListLegs() + 1);
    for (auto t = ranking.constBegin(); t < ranking.constEnd(); t++) {
        count = (*t)->getClassEntryCount();
        for (int j = 0; j < count; j++) {
            if (entriesPerBlock > availableEntriesOnPage) {
                // go to a new page
                pages.emplaceBack();
                availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
            }
            availableEntriesOnPage -= entriesPerBlock;
            pages.last().append(*t);
        }
        // Leave a blank line before next team (but not at page bottom)
        availableEntriesOnPage--;
        pages.last().append(Q_NULLPTR);
    }

    return pages;
}

void PDFRankingPrinter::printHeaderSingleLeg(QRectF &writeRect, int page, RankingType type)
{
    switch (type) {
    case RankingType::START_LIST:
    case RankingType::INDIVIDUAL_SINGLE:
    case RankingType::TEAM_SINGLE:
        // Rankings header
        writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
        writeRect.setHeight(toVdots(8.0));
        // Ranking placement
        writeRect.setWidth(toHdots(5.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
        // Team placement (team ranking only)
        if (type == RankingType::TEAM_SINGLE) {
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
        }
        // Bib
        writeRect.translate(toHdots(5.0), 0.0);
        writeRect.setWidth(toHdots(7.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
        // Name
        writeRect.translate(toHdots(8.0), 0.0);
        writeRect.setWidth(toHdots(60.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
        // Team
        writeRect.translate(toHdots(60.0), 0.0);
        writeRect.setWidth(toHdots(45.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
        // Year
        writeRect.translate(toHdots(45.0), 0.0);
        writeRect.setWidth(toHdots(9.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
        // Sex
        writeRect.translate(toHdots(9.0), 0.0);
        writeRect.setWidth(toHdots(6.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
        // Category
        writeRect.translate(toHdots(6.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::TEAM_SINGLE) ? 23.0 : 28.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
        // Time
        writeRect.translate(toHdots((type == RankingType::TEAM_SINGLE) ? 23.0 : 28.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::INDIVIDUAL_SINGLE) ? 12.0 : 27.0));
        if (type == RankingType::START_LIST) {
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, (CRLoader::getStartListLegs() == 1) ? tr("Start Time") : tr("Leg", "long label"));
        } else { // individual or team single
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));
        }
        // Time difference (individual ranking only)
        if (type == RankingType::INDIVIDUAL_SINGLE) {
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Diff"));
        }
        break;
    default:
        emit error(tr("Error: ranking type not allowed"));
        break;
    }
}

void PDFRankingPrinter::printHeaderMultiLeg(QRectF &writeRect, int page, RankingType type)
{
    switch (type) {
    case RankingType::INDIVIDUAL_MULTI:
    case RankingType::TEAM_MULTI:
        // First header line
        writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
        writeRect.setHeight(toVdots(4.0));
        // Ranking placement
        writeRect.setWidth(toHdots(5.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
        // Team placement (team ranking only)
        if (type == RankingType::TEAM_MULTI) {
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
        }
        // Bib
        writeRect.translate(toHdots(5.0), 0.0);
        writeRect.setWidth(toHdots(7.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
        // Team
        writeRect.translate(toHdots(8.0), 0.0);
        writeRect.setWidth(toHdots(66.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
        // Category
        writeRect.translate(toHdots(66.0), 0.0);
        writeRect.setWidth(toHdots(82.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
        // Time
        writeRect.translate(toHdots(82.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::INDIVIDUAL_MULTI) ? 12.0 : 22.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));
        // Time difference (individual only)
        if (type == RankingType::INDIVIDUAL_MULTI) {
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Diff"));
        }

        // Second header line
        writeRect.setTopLeft(QPointF(toHdots((type == RankingType::INDIVIDUAL_MULTI) ? 14.0 : 19.0), toVdots((page == 1) ? 52.5 : 29.5)));
        writeRect.setHeight(toVdots(4.0));
        // Leg
        writeRect.setWidth(toHdots(5.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg", "short label"));
        // Name
        writeRect.translate(toHdots(6.0), 0.0);
        writeRect.setWidth(toHdots(60.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
        // Year
        writeRect.translate(toHdots(60.0), 0.0);
        writeRect.setWidth(toHdots(9.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
        // Sex
        writeRect.translate(toHdots(9.0), 0.0);
        writeRect.setWidth(toHdots(6.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
        // Time (and ranking) in Leg
        writeRect.translate(toHdots(6.0), 0.0);
        writeRect.setWidth(toHdots(67.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg Time (and position)"));
        break;
    default:
        emit error(tr("Error: ranking type not allowed"));
        break;
    }
}

void PDFRankingPrinter::printEntrySingleLeg(QRectF &writeRect, ClassEntry const *c, int &posIndex, int cIndex, uint referenceTime, RankingType type)
{
    static Position position;

    Category const *category = c->getCategory(0);

    QString currTime = c->getTotalTime(CRLoader::Format::PDF);

    switch (type) {
    case RankingType::INDIVIDUAL_SINGLE:
    case RankingType::TEAM_SINGLE:
        // Move down
        writeRect.translate(0.0, toVdots(4.0));
        writeRect.setLeft(toHdots(0.0));
        // Set the background
        if ((((type == RankingType::INDIVIDUAL_SINGLE) ? posIndex : cIndex) % 2) == 0) {
            writeRect.setWidth(this->areaWidth);
            painter.fillRect(writeRect, Qt::lightGray);
        }
        // Ranking placement
        painter.setFont(rnkFontBold);
        writeRect.setLeft(toHdots(1.0));
        writeRect.setWidth(toHdots(5.0));
        if (type == RankingType::INDIVIDUAL_SINGLE) {
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(position.getCurrentPositionString(posIndex, currTime)));
            // Increment the position index
            posIndex++;
        } else if (cIndex == 1) {
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(posIndex));
            // Increment the position index
            posIndex++;
        }
        // Team placement
        if (type == RankingType::TEAM_SINGLE) {
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(position.getCurrentPositionString(cIndex, currTime)));
        }
        // Bib
        painter.setFont(rnkFont);
        writeRect.translate(toHdots(5.0), 0.0);
        writeRect.setWidth(toHdots(7.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(c->getBib()));
        // Name
        painter.setFont(rnkFontBold);
        writeRect.translate(toHdots(8.0), 0.0);
        writeRect.setWidth(toHdots(60.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getName(0));
        // Team
        painter.setFont(rnkFont);
        writeRect.translate(toHdots(60.0), 0.0);
        writeRect.setWidth(toHdots(45.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getClubsAndTeam());
        // Year
        writeRect.translate(toHdots(45.0), 0.0);
        writeRect.setWidth(toHdots(9.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number(c->getYear(0)));
        // Sex
        writeRect.translate(toHdots(9.0), 0.0);
        writeRect.setWidth(toHdots(6.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, CRHelper::toSexString(c->getSex()));
        // Category
        writeRect.translate(toHdots(6.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::INDIVIDUAL_SINGLE) ? 28.0 : 23.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, category ? category->getFullDescription() : "---");
        // Time
        painter.setFont(rnkFontBold);
        writeRect.translate(toHdots((type == RankingType::INDIVIDUAL_SINGLE) ? 28.0 : 23.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::INDIVIDUAL_SINGLE) ? 12.0 : 27.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, currTime);
        // Time difference
        if (type == RankingType::INDIVIDUAL_SINGLE) {
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, c->getDiffTimeTxt(referenceTime));
        }
        break;
    default:
        emit error(tr("Error: ranking type not allowed"));
        break;
    }
}

void PDFRankingPrinter::printPageSingleLeg(QRectF &writeRect, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime)
{
    for (auto c = page.constBegin(); c < page.constEnd(); c++) {
        try {
            printEntrySingleLeg(writeRect, *c, posIndex, -1, referenceTime, RankingType::INDIVIDUAL_SINGLE);
        } catch (ChronoRaceException &e) {
            emit error(e.getMessage());
        }
    }
}

void PDFRankingPrinter::printPageSingleLeg(QRectF &writeRect, QList<TeamClassEntry const *> const &page, int &posIndex)
{
    static TeamClassEntry const *lastTeam = Q_NULLPTR;
    static int j = 0;

    for (auto t = page.constBegin(); t < page.constEnd(); t++) {

        if (*t == Q_NULLPTR) { // separator
            // move down and go to the next team
            writeRect.translate(0.0, toVdots(4.0));
            continue;
        }

        if (*t == lastTeam) {
            j++;
        } else {
            lastTeam = *t;
            j = 0;
        }

        try {
            printEntrySingleLeg(writeRect, (*t)->getClassEntry(j), posIndex, j + 1, 0, RankingType::TEAM_SINGLE);
        } catch (ChronoRaceException &e) {
            emit error(e.getMessage());
        }
    }
}

void PDFRankingPrinter::printEntryMultiLeg(QRectF &writeRect, ClassEntry const *r, int &posIndex, int rIndex, uint referenceTime, RankingType type)
{
    static Position position;

    auto entriesPerBlock = static_cast<int>(CRLoader::getStartListLegs() + 1);

    Category const *category = r->getCategory();

    QString currTime = r->getTotalTime(CRLoader::Format::PDF);

    switch (type) {
    case RankingType::INDIVIDUAL_MULTI:
    case RankingType::TEAM_MULTI:
        // Move down
        writeRect.translate(0.0, toVdots(4.0));
        writeRect.setLeft(toHdots(0.0));
        // Set the background
        if ((((type == RankingType::INDIVIDUAL_MULTI) ? posIndex : rIndex) % 2) == 0) {
            writeRect.setWidth(this->areaWidth);
            writeRect.setHeight(toVdots(4.0 * entriesPerBlock));
            painter.fillRect(writeRect, Qt::lightGray);
            writeRect.setHeight(toVdots(4.0));
        }
        // Ranking placement
        painter.setFont(rnkFontBold);
        writeRect.setLeft(toHdots(1.0));
        writeRect.setWidth(toHdots(5.0));
        if (type == RankingType::INDIVIDUAL_MULTI) {
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(position.getCurrentPositionString(posIndex, currTime)));
            // Increment the position index
            posIndex++;
        } else if (rIndex == 1) {
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(posIndex));
            // Increment the position index
            posIndex++;
        }
        // Team placement
        if (type == RankingType::TEAM_MULTI) {
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(position.getCurrentPositionString(rIndex, currTime)));
        }
        // Bib
        painter.setFont(rnkFont);
        writeRect.translate(toHdots(5.0), 0.0);
        writeRect.setWidth(toHdots(7.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(r->getBib()));
        // Team
        painter.setFont(rnkFontBold);
        writeRect.translate(toHdots(8.0), 0.0);
        writeRect.setWidth(toHdots(66.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, r->getClubsAndTeam());
        // Category
        painter.setFont(rnkFont);
        writeRect.translate(toHdots(66.0), 0.0);
        writeRect.setWidth(toHdots(82.0));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, category ? category->getFullDescription() : "---");
        // Time
        painter.setFont(rnkFontBold);
        writeRect.translate(toHdots(82.0), 0.0);
        writeRect.setWidth(toHdots((type == RankingType::INDIVIDUAL_MULTI) ? 12.0 : 22.0));
        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, currTime);
        // Time difference
        if (type == RankingType::INDIVIDUAL_MULTI) {
            painter.setFont(rnkFont);
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, r->getDiffTimeTxt(referenceTime));
        }

        // Competitor lines
        for (uint j = 0; j < CRLoader::getStartListLegs(); j++) {
            // Move down
            writeRect.translate(0.0, toVdots(4.0));
            writeRect.setLeft(toHdots((type == RankingType::INDIVIDUAL_MULTI) ? 14.0 : 19.0));

            // Leg
            painter.setFont(rnkFont);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(j + 1));
            // Name
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, r->getName(j));
            // Year
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number(r->getYear(j)));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, CRHelper::toSexString(r->getSex(j)));
            // Time in Leg
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(61.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, r->getTime(j));
            // Ranking in Leg
            writeRect.translate(toHdots(62.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(r->getLegRanking(j)));
        }
        break;
    default:
        emit error(tr("Error: ranking type not allowed"));
        break;
    }
}

void PDFRankingPrinter::printPageMultiLeg(QRectF &writeRect, QList<ClassEntry const *> const &page, int &posIndex, uint referenceTime)
{
    for (auto c = page.constBegin(); c < page.constEnd(); c++) {
        try {
            printEntryMultiLeg(writeRect, *c, posIndex, -1, referenceTime, RankingType::INDIVIDUAL_MULTI);
        } catch (ChronoRaceException &e) {
            emit error(e.getMessage());
        }
    }
}

void PDFRankingPrinter::printPageMultiLeg(QRectF &writeRect, QList<TeamClassEntry const *> const &page, int &posIndex)
{
    static TeamClassEntry const *lastTeam = Q_NULLPTR;
    static int j = 0;

    for (auto t = page.constBegin(); t < page.constEnd(); t++) {

        if (*t == Q_NULLPTR) { // separator
            // move down and go to the next team
            writeRect.translate(0.0, toVdots(4.0));
            continue;
        }

        if (*t == lastTeam) {
            j++;
        } else {
            lastTeam = *t;
            j = 0;
        }

        try {
            printEntryMultiLeg(writeRect, (*t)->getClassEntry(j), posIndex, j + 1, 0, RankingType::TEAM_MULTI);
        } catch (ChronoRaceException &e) {
            emit error(e.getMessage());
        }
    }

}

void PDFRankingPrinter::drawTemplatePortrait(QString const &fullDescription, int page, int pages, bool startList)
{
    QRect boundingRect;
    QRectF writeRect;
    qreal rectWidth;

    // Rage information
    ChronoRaceData const *raceInfo = getRaceInfo();
    Q_ASSERT(raceInfo);

    // Text options for wrapping
    QTextOption textOptions;

    // Sponsors
    QVector<QPixmap> sponsors = raceInfo->getSponsorLogos();

    // Publishing time
    QString editingTimestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");

    // Created with
    QString createdWith = tr("Created with %1 %2").arg(QStringLiteral(LBCHRONORACE_NAME),
                                                       QStringLiteral(LBCHRONORACE_VERSION));

    // Horizontal lines
    if (page == 1) {
        // Page 1
        painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 39.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 48.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 57.0), this->areaWidth, toVdots(0.2)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots(-41.0), this->areaWidth, toVdots(0.5)), Qt::black);
    } else {
        // Page 2, 3, ...
        painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 34.0), this->areaWidth, toVdots(0.2)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots(-41.0), this->areaWidth, toVdots(0.5)), Qt::black);
    }
    // Left and Right logo
    if (page == 1) {
        writeRect.setX(toHdots(0.0));
        writeRect.setY(toVdots(1.0));
        writeRect.setWidth(toHdots(23.5));
        writeRect.setHeight(toVdots(23.5));
        this->fitRectToLogo(writeRect, raceInfo->getLeftLogo());
        painter.drawPixmap(writeRect.toRect(), raceInfo->getLeftLogo());
        writeRect.setX(toHdots(-23.5));
        writeRect.setY(toVdots(1.0));
        writeRect.setWidth(toHdots(23.5));
        writeRect.setHeight(toVdots(23.5));
        this->fitRectToLogo(writeRect, raceInfo->getRightLogo());
        painter.drawPixmap(writeRect.toRect(), raceInfo->getRightLogo());
    }
    // Sponsor logos
    int l = 0;
    rectWidth = this->areaWidth / static_cast<qreal>(sponsors.size());
    for (auto const &sponsor : sponsors) {
        writeRect.setX((rectWidth * l++) + toHdots(1.0));
        writeRect.setY(toVdots(-35.0));
        writeRect.setWidth(rectWidth  - toHdots(2.0));
        writeRect.setHeight(toVdots(29.0));
        this->fitRectToLogo(writeRect, sponsor);
        painter.drawPixmap(writeRect.toRect(), sponsor);
    }
    // Created with
    rnkFont.setPointSize(6);
    painter.setFont(rnkFont);
    writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-6.0)));
    writeRect.setBottomRight(QPointF(this->areaWidth, this->areaHeight));
    painter.drawText(writeRect.toRect(), Qt::AlignCenter, createdWith);
    // Title and subtitle
    if (page == 1) {
        rnkFontBoldItal.setPointSize(22);
        painter.setFont(rnkFontBoldItal);
        writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(0.5)));
        writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(15.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getEvent());
        painter.setFont(rnkFontItalic);
        writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(15.0)));
        writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(25.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getPlace() + " - " + QLocale::system().toString(raceInfo->getDate(), "dddd dd/MM/yyyy"));
    } else {
        rnkFontBoldItal.setPointSize(14);
        painter.setFont(rnkFontBoldItal);
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(0.5)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(15.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getEvent() + " - " + raceInfo->getPlace() + " - " + raceInfo->getDate().toString("dd/MM/yyyy"));
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(15.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(25.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription);
    }
    // Results, page and editing timestamp
    painter.setFont(rnkFont);
    writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-40.5)));
    if (!startList) {
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-35.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignHCenter | Qt::AlignTop, tr("Results") + ":\u00a0" + raceInfo->getResults());
    }
    writeRect.setBottomRight(QPointF(toHdots(30.0), toVdots(-35.0)));
    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Page %n", "", page) + " " + tr("of %n", "", pages));
    writeRect.setTopLeft(QPointF(toHdots(-30.0), toVdots(-40.5)));
    writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-35.0)));
    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignTop, editingTimestamp);
    if (page == 1) {
        // Organization
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(26.0)));
        writeRect.setBottomRight(QPointF((this->areaWidth * 0.5) - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Organization") + ": ", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        textOptions.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        painter.drawText(writeRect, raceInfo->getOrganization(), textOptions);
        // Type, start time, length and elevation gin
        writeRect.setTopLeft(QPointF(this->areaWidth * 0.5, toVdots(26.0)));
        writeRect.setBottomRight(QPointF((this->areaWidth * 0.75) - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Race Type") + ":\u00a0\n" + tr("Start Time") + ":\u00a0\n" + tr("Length") + ":\u00a0\n" + tr("Elevation Gain") + ":\u00a0", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo->getRaceType() + "\n" + raceInfo->getStartTime().toString("hh:mm") + "\n" + raceInfo->getLength() + "\n" + raceInfo->getElevationGain());
        // Referee and Timekeeper 1, 2 and 3
        writeRect.setTopLeft(QPointF(this->areaWidth * 0.75, toVdots(26.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Referee") + ":\u00a0\n" + tr("Timekeeper 1") + ":\u00a0\n" + tr("Timekeeper 2") + ":\u00a0\n" + tr("Timekeeper 3") + ":\u00a0", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo->getReferee() + "\n" + raceInfo->getTimeKeeper(0) + "\n" + raceInfo->getTimeKeeper(1) + "\n" + raceInfo->getTimeKeeper(2));
        // Description
        rnkFontBold.setPointSize(18);
        painter.setFont(rnkFontBold);
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(39.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(47.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription.toUpper());
    }
}

//NOSONAR void PDFRankingPrinter::drawTemplateLandscape(QString const &fullDescription, int page, int pages, bool startList)
//NOSONAR {
//NOSONAR     QRect boundingRect;
//NOSONAR     QRectF writeRect;
//NOSONAR     qreal rectWidth;

//NOSONAR     // Rage information
//NOSONAR     ChronoRaceData const *raceInfo = getRaceInfo();
//NOSONAR     Q_ASSERT(raceInfo);

//NOSONAR     // Text options for wrapping
//NOSONAR     QTextOption textOptions;

//NOSONAR     // Sponsors
//NOSONAR     QVector<QPixmap> sponsors = raceInfo->getSponsorLogos();

//NOSONAR     // Publishing time
//NOSONAR     QString editingTimestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");

//NOSONAR     // Created with
//NOSONAR     QString createdWith = tr("Created with %1 %2").arg(QStringLiteral(LBCHRONORACE_NAME),
//NOSONAR                                                        QStringLiteral(LBCHRONORACE_VERSION));

//NOSONAR     // Horizontal lines
//NOSONAR     if (page == 1) {
//NOSONAR         // Page 1
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 39.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 48.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 57.0), this->areaWidth, toVdots(0.2)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(-41.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR     } else {
//NOSONAR         // Page 2, 3, ...
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 34.0), this->areaWidth, toVdots(0.2)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(-41.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR     }
//NOSONAR     // Left and Right logo
//NOSONAR     if (page == 1) {
//NOSONAR         writeRect.setX(toHdots(0.0));
//NOSONAR         writeRect.setY(toVdots(1.0));
//NOSONAR         writeRect.setWidth(toHdots(23.5));
//NOSONAR         writeRect.setHeight(toVdots(23.5));
//NOSONAR         this->fitRectToLogo(writeRect, raceInfo->getLeftLogo());
//NOSONAR         painter.drawPixmap(writeRect.toRect(), raceInfo->getLeftLogo());
//NOSONAR         writeRect.setX(toHdots(-23.5));
//NOSONAR         writeRect.setY(toVdots(1.0));
//NOSONAR         writeRect.setWidth(toHdots(23.5));
//NOSONAR         writeRect.setHeight(toVdots(23.5));
//NOSONAR         this->fitRectToLogo(writeRect, raceInfo->getRightLogo());
//NOSONAR         painter.drawPixmap(writeRect.toRect(), raceInfo->getRightLogo());
//NOSONAR     }
//NOSONAR     // Sponsor logos
//NOSONAR     rectWidth = this->areaWidth / sponsors.size();
//NOSONAR     for (int l = 0; l < sponsors.size(); l++) {
//NOSONAR         writeRect.setX((rectWidth * l) + toHdots(1.0));
//NOSONAR         writeRect.setY(toVdots(-35.0));
//NOSONAR         writeRect.setWidth(rectWidth  - toHdots(2.0));
//NOSONAR         writeRect.setHeight(toVdots(29.0));
//NOSONAR         this->fitRectToLogo(writeRect, sponsors[l]);
//NOSONAR         painter.drawPixmap(writeRect.toRect(), sponsors[l]);
//NOSONAR     }
//NOSONAR     // Created with
//NOSONAR     rnkFont.setPointSize(6);
//NOSONAR     painter.setFont(rnkFont);
//NOSONAR     writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-6.0)));
//NOSONAR     writeRect.setBottomRight(QPointF(this->areaWidth, this->areaHeight));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignCenter, createdWith);
//NOSONAR     // Title and subtitle
//NOSONAR     if (page == 1) {
//NOSONAR         rnkFontBoldItal.setPointSize(22);
//NOSONAR         painter.setFont(rnkFontBoldItal);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(0.5)));
//NOSONAR         writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(15.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getEvent());
//NOSONAR         painter.setFont(rnkFontItalic);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(15.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(25.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getPlace() + " - " + QLocale::system().toString(raceInfo->getDate(), "dddd dd/MM/yyyy"));
//NOSONAR     } else {
//NOSONAR         rnkFontBoldItal.setPointSize(14);
//NOSONAR         painter.setFont(rnkFontBoldItal);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(0.5)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(15.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo->getEvent() + " - " + raceInfo->getPlace() + " - " + raceInfo->getDate().toString("dd/MM/yyyy"));
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(15.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(25.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription);
//NOSONAR     }
//NOSONAR     // Results, page and editing timestamp
//NOSONAR     painter.setFont(rnkFont);
//NOSONAR     writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-40.5)));
//NOSONAR     if (!startList) {
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-35.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignHCenter | Qt::AlignTop, tr("Results") + ":\u00a0" + raceInfo->getResults());
//NOSONAR     }
//NOSONAR     writeRect.setBottomRight(QPointF(toHdots(30.0), toVdots(-35.0)));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Page %n", "", page) + " " + tr("of %n", "", pages));
//NOSONAR     writeRect.setTopLeft(QPointF(toHdots(-30.0), toVdots(-40.5)));
//NOSONAR     writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-35.0)));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignTop, editingTimestamp);
//NOSONAR     if (page == 1) {
//NOSONAR         // Organization
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF((this->areaWidth * 0.5) - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Organization") + ": ", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         textOptions.setAlignment(Qt::AlignLeft | Qt::AlignTop);
//NOSONAR         painter.drawText(writeRect, raceInfo->getOrganization(), textOptions);
//NOSONAR         // Type, start time, length and elevation gin
//NOSONAR         writeRect.setTopLeft(QPointF(this->areaWidth * 0.5, toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF((this->areaWidth * 0.75) - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Race Type") + ":\u00a0\n" + tr("Start Time") + ":\u00a0\n" + tr("Length") + ":\u00a0\n" + tr("Elevation Gain") + ":\u00a0", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo->getRaceType() + "\n" + raceInfo->getStartTime().toString("hh:mm") + "\n" + raceInfo->getLength() + "\n" + raceInfo->getElevationGain());
//NOSONAR         // Referee and Timekeeper 1, 2 and 3
//NOSONAR         writeRect.setTopLeft(QPointF(this->areaWidth * 0.75, toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Referee") + ":\u00a0\n" + tr("Timekeeper 1") + ":\u00a0\n" + tr("Timekeeper 2") + ":\u00a0\n" + tr("Timekeeper 3") + ":\u00a0", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo->getReferee() + "\n" + raceInfo->getTimeKeeper1() + "\n" + raceInfo->getTimeKeeper2() + "\n" + raceInfo->getTimeKeeper3());
//NOSONAR         // Description
//NOSONAR         rnkFontBold.setPointSize(18);
//NOSONAR         painter.setFont(rnkFontBold);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(39.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(47.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription.toUpper());
//NOSONAR     }
//NOSONAR }
