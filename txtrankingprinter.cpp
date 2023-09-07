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

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include "txtrankingprinter.h"
#include "chronoracedata.h"
#include "lbcrexception.h"

void TXTRankingPrinter::printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath)
{
    QString textFileName = QFileDialog::getSaveFileName(parent, tr("Select Start List File"),
        lastSelectedPath.absolutePath(), tr("Plain Text (*.txt)"));

    //NOSONAR qDebug("Path: %s", qUtf8Printable(outFileName));
    if (!textFileName.isEmpty()) {

        // append the .txt extension if missing
        checkOutFileNameExtension(textFileName);

        QFile outFile(textFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw(ChronoRaceException(tr("Error: cannot open %1").arg(textFileName)));
        }
        QTextStream outStream(&outFile);

        if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
            outStream.setEncoding(QStringConverter::Utf8);
        else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
            outStream.setEncoding(QStringConverter::Latin1);

        auto nWidth = CRLoader::getStartListNameWidthMax();
        auto tWidth = CRLoader::getTeamNameWidthMax();

        int offset;
        int i = 0;
        ChronoRaceData const *raceInfo = getRaceInfo();
        QTime startTime = raceInfo->getStartTime();
        outStream << *raceInfo << Qt::endl; // add header
        outStream << tr("Start List") << Qt::endl;
        for (auto const &competitor : startList) {
            i++;
            outStream.setFieldWidth(getIndexFieldWidth());
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << i;
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream.setFieldWidth(getBibFieldWidth());
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << competitor.getBib();
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream << competitor.getName(nWidth);
            outStream << " - ";
            outStream << competitor.getTeam(tWidth);
            outStream << " - ";
            outStream << competitor.getYear();
            outStream << " - ";
            outStream << Competitor::toSexString(competitor.getSex());
            outStream << " - ";
            outStream.setFieldWidth(15);
            outStream.setFieldAlignment(QTextStream::AlignRight);
            offset = competitor.getOffset();
            if (offset >= 0) {
                offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                outStream << Competitor::toOffsetString(offset);
            } else if (CRLoader::getStartListLegs() == 1) {
                offset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                outStream << Competitor::toOffsetString(offset);
            } else {
                outStream << tr("Leg %n", "", qAbs(offset));
            }
            outStream.setFieldWidth(0);
            outStream << Qt::endl;
        }
        outStream << Qt::endl;

        QFileInfo outFileInfo(textFileName);
        emit info(tr("Generated Start List: %1").arg(QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));
        lastSelectedPath = outFileInfo.absoluteDir();

        outStream.flush();
        outFile.close();
    }
}

void TXTRankingPrinter::printRanking(const Category &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName)
{
    QString const &individualRankingFileName = buildOutFileName(outFileBaseName);
    QFile outFile(individualRankingFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(individualRankingFileName)));
    }
    QTextStream outStream(&outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    printTxtRanking(ranking, category.getFullDescription(), outStream);

    QFileInfo outFileInfo(outFile);
    emit info(tr("Generated Results '%1': %2").arg(category.getFullDescription(), QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));

    outStream.flush();
    outFile.close();
}

void TXTRankingPrinter::printRanking(const Category &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName)
{
    QString const &teamRankingFileName = buildOutFileName(outFileBaseName);
    QFile outFile(teamRankingFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(teamRankingFileName)));
    }
    QTextStream outStream(&outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    printTxtTeamRanking(ranking, category.getFullDescription(), outStream);

    QFileInfo outFileInfo(outFile);
    emit info(tr("Generated Results '%1': %2").arg(category.getFullDescription(), QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));

    outStream.flush();
    outFile.close();
}

void TXTRankingPrinter::printTxtRanking(QList<ClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const
{
    static Position position;

    int i = 0;
    QString currTime;
    ChronoRaceData const *raceInfo = getRaceInfo();
    outStream << *raceInfo << Qt::endl; // add header
    outStream << description << Qt::endl;
    for (auto const c : ranking) {
        i++;
        currTime = c->getTotalTimeTxt();
        outStream.setFieldWidth(getIndexFieldWidth());
        outStream.setFieldAlignment(QTextStream::AlignRight);
        outStream << position.getCurrentPositionNumber(i, currTime);
        outStream.setFieldWidth(0);
        outStream << " - ";
        outStream.setFieldWidth(getBibFieldWidth());
        outStream.setFieldAlignment(QTextStream::AlignRight);
        outStream << c->getBib();
        outStream.setFieldWidth(0);
        outStream << " - ";
        outStream << c->getNamesTxt();
        outStream << " - ";
        if (CRLoader::getStartListLegs() > 1)
            outStream << c->getTimesTxt(getIndexFieldWidth()) << " - ";
        outStream << currTime << Qt::endl;
    }
    outStream << Qt::endl;
}

void TXTRankingPrinter::printTxtTeamRanking(QList<TeamClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const
{
    int i = 0;
    ChronoRaceData const *raceInfo = getRaceInfo();
    outStream << *raceInfo << Qt::endl; // add header
    outStream << description << Qt::endl;
    for (auto const r : ranking) {
        i++;
        for (int j = 0; j < r->getClassEntryCount(); j++) {
            if (j == 0) {
                outStream.setFieldWidth(getIndexFieldWidth());
                outStream.setFieldAlignment(QTextStream::AlignRight);
                outStream << i;
                outStream.setFieldWidth(0);
                outStream << " - ";
            } else {
                outStream.setFieldWidth(getIndexFieldWidth() + 3);
                outStream << "";
            }
            outStream.setFieldWidth(getBibFieldWidth());
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << r->getClassEntry(j)->getBib();
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream << r->getClassEntry(j)->getNamesTxt();
            outStream << " - ";
            if (CRLoader::getStartListLegs() > 1)
                outStream << r->getClassEntry(j)->getTimesTxt(getIndexFieldWidth()) << " - ";
            outStream << r->getClassEntry(j)->getTotalTimeTxt() << Qt::endl;
        }
        outStream << Qt::endl;
    }
    outStream << Qt::endl;
}

QString &TXTRankingPrinter::buildOutFileName(QString &outFileBaseName)
{
    return outFileBaseName.append(".txt");
}

QString &TXTRankingPrinter::checkOutFileNameExtension(QString &outFileBaseName)
{
    return (!outFileBaseName.endsWith(".txt", Qt::CaseInsensitive)) ? buildOutFileName(outFileBaseName) : outFileBaseName;
}
