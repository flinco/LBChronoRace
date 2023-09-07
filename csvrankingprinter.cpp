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

#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include "csvrankingprinter.h"
#include "chronoracedata.h"
#include "lbcrexception.h"

void CSVRankingPrinter::printStartList(QList<Competitor> const &startList, QWidget *parent, QDir &lastSelectedPath)
{
    QString cvsFileName = QFileDialog::getSaveFileName(parent, tr("Select Start List File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    //NOSONAR qDebug("Path: %s", qUtf8Printable(outFileName));
    if (!cvsFileName.isEmpty()) {

        // append the .csv extension if missing
        checkOutFileNameExtension(cvsFileName);

        QFile outFile(cvsFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw(ChronoRaceException(tr("Error: cannot open %1").arg(cvsFileName)));
        }
        QTextStream outStream(&outFile);

        if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
            outStream.setEncoding(QStringConverter::Utf8);
        else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
            outStream.setEncoding(QStringConverter::Latin1);

        int offset;
        int i = 0;
        QTime startTime = getRaceInfo()->getStartTime();
        for (auto const &competitor : startList) {
            i++;
            outStream << i << ",";
            outStream << competitor.getBib() << ",";
            outStream << competitor.getName() << ",";
            outStream << competitor.getTeam() << ",";
            outStream << competitor.getYear() << ",";
            outStream << Competitor::toSexString(competitor.getSex()) << ",";
            offset = competitor.getOffset();
            if (offset >= 0) {
                offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                outStream << Competitor::toOffsetString(offset);
            } else {
                outStream << qAbs(offset);
            }
            outStream << Qt::endl;
        }
        outStream << Qt::endl;

        QFileInfo outFileInfo(cvsFileName);
        emit info(tr("Generated Start List: %1").arg(QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));
        lastSelectedPath = outFileInfo.absoluteDir();

        outStream.flush();
        outFile.close();
    }
}

void CSVRankingPrinter::printRanking(const Category &category, QList<ClassEntry const *> const &ranking, QString &outFileBaseName)
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

    printCSVRanking(ranking, outStream);

    QFileInfo outFileInfo(outFile);
    emit info(tr("Generated Results '%1': %2").arg(category.getFullDescription(), QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));

    outStream.flush();
    outFile.close();
}

void CSVRankingPrinter::printRanking(const Category &category, QList<TeamClassEntry const *> const &ranking, QString &outFileBaseName)
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

    printCSVTeamRanking(ranking, category.getShortDescription(), outStream);

    QFileInfo outFileInfo(outFile);
    emit info(tr("Generated Results '%1': %2").arg(category.getFullDescription(), QDir::toNativeSeparators(outFileInfo.absoluteFilePath())));

    outStream.flush();
    outFile.close();
}

void CSVRankingPrinter::printCSVRanking(QList<ClassEntry const *> const &ranking, QTextStream &outStream) const
{
    static Position position;

    int i = 0;
    QString currTime;
    for (auto const c : ranking) {
        i++;
        currTime = c->getTotalTimeTxt();
        outStream << position.getCurrentPositionNumber(i, currTime) << ",";
        outStream << c->getBib() << ",";
        outStream << c->getNamesCSV() << ",";
        if (CRLoader::getStartListLegs() > 1)
            outStream << c->getTimesCSV() << ",";
        outStream << currTime << Qt::endl;
    }
    outStream << Qt::endl;
}

void CSVRankingPrinter::printCSVTeamRanking(QList<TeamClassEntry const *> const &ranking, QString const &description, QTextStream &outStream) const
{
    int i = 0;
    outStream << description << Qt::endl;
    for (auto const r : ranking) {
        i++;
        for (int j = 0; j < r->getClassEntryCount(); j++) {
            outStream << i << ",";
            outStream << r->getClassEntry(j)->getBib() << ",";
            outStream << r->getClassEntry(j)->getNamesCSV() << ",";
            if (CRLoader::getStartListLegs() > 1)
                outStream << r->getClassEntry(j)->getTimesCSV() << ",";
            outStream << r->getClassEntry(j)->getTotalTimeCSV() << Qt::endl;
        }
    }
    outStream << Qt::endl;
}

QString &CSVRankingPrinter::buildOutFileName(QString &outFileBaseName)
{
    return outFileBaseName.append(".csv");
}

QString &CSVRankingPrinter::checkOutFileNameExtension(QString &outFileBaseName)
{
    return (!outFileBaseName.endsWith(".csv", Qt::CaseInsensitive)) ? buildOutFileName(outFileBaseName) : outFileBaseName;
}
