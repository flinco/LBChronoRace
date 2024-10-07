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

#include <QIODevice>

#include "csvrankingprinter.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

void CSVRankingPrinter::init(QString *outFileName, QString const &title, QString const &subject)
{
    Q_ASSERT(!csvFile.isOpen());

    Q_UNUSED(title)
    Q_UNUSED(subject)

    if (outFileName == Q_NULLPTR) {
        throw(ChronoRaceException(tr("Error: no file name supplied")));
    }

    // append the .csv extension if missing
    checkOutFileNameExtension(*outFileName);

    csvFile.setFileName(*outFileName);
    if (!csvFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(*outFileName)));
    }
    csvStream.setDevice(&csvFile);
    csvStream.setEncoding(CRLoader::getEncoding());
}

void CSVRankingPrinter::printStartList(QList<Competitor const *> const &startList)
{
    if (!csvFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: writing attempt on closed file")));
    }

    int offset;
    int i = 0;
    QTime startTime = getRaceInfo()->getStartTime();
    for (auto const *competitor : startList) {
        i++;
        csvStream << i << ",";
        csvStream << competitor->getBib() << ",";
        csvStream << competitor->getCompetitorName(CRHelper::nameComposition) << ",";
        csvStream << competitor->getTeam() << ",";
        csvStream << competitor->getYear() << ",";
        csvStream << CRHelper::toSexString(competitor->getSex()) << ",";
        offset = competitor->getOffset();
        if (offset >= 0) {
            offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
            csvStream << CRHelper::toOffsetString(offset);
        } else if (CRLoader::getStartListLegs() == 1) {
            offset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
            csvStream << CRHelper::toOffsetString(offset);
        } else {
            csvStream << qAbs(offset);
        }
        csvStream << Qt::endl;
    }
    csvStream << Qt::endl;
}

void CSVRankingPrinter::printRanking(Ranking const &categories, QList<ClassEntry const *> const &ranking)
{
    static Position position;

    if (!csvFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: writing attempt on closed file")));
    }

    int i = 0;
    int prevPosNumber = 0;
    int currPosNumber = 0;
    QString currTime;
    csvStream << categories.getShortDescription() << Qt::endl;
    for (auto const c : ranking) {
        i++;
        currTime = c->getTotalTime(CRLoader::Format::CSV);
        if ((currPosNumber = position.getCurrentPositionNumber(i, currTime)) == 0)
            currPosNumber = prevPosNumber;
        else
            prevPosNumber = currPosNumber;
        csvStream << currPosNumber << ",";
        csvStream << c->getBib() << ",";
        csvStream << c->getNames(CRLoader::Format::CSV) << ",";
        if (CRLoader::getStartListLegs() > 1)
            csvStream << c->getTimes(CRLoader::Format::CSV) << ",";
        csvStream << currTime << Qt::endl;
    }
    csvStream << Qt::endl;
}

void CSVRankingPrinter::printRanking(Ranking const &categories, QList<TeamClassEntry const *> const &ranking)
{
    if (!csvFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: writing attempt on closed file")));
    }

    int i = 0;
    csvStream << categories.getShortDescription() << Qt::endl;
    for (auto const r : ranking) {
        i++;
        for (int j = 0; j < r->getClassEntryCount(); j++) {
            csvStream << i << ",";
            csvStream << r->getClassEntry(j)->getBib() << ",";
            csvStream << r->getClassEntry(j)->getNames(CRLoader::Format::CSV) << ",";
            if (CRLoader::getStartListLegs() > 1)
                csvStream << r->getClassEntry(j)->getTimes(CRLoader::Format::CSV) << ",";
            csvStream << r->getClassEntry(j)->getTotalTime(CRLoader::Format::CSV) << Qt::endl;
        }
    }
    csvStream << Qt::endl;
}

bool CSVRankingPrinter::finalize()
{
    bool result = true;

    if (QIODevice *device = csvStream.device(); device != Q_NULLPTR) {
        csvStream.flush();
        device->close();
    } else {
        result = false;
    }

    return result;
}

QString CSVRankingPrinter::getFileFilter()
{
    return tr("CSV (*.csv)");
}

QString &CSVRankingPrinter::buildOutFileName(QString &outFileBaseName)
{
    return outFileBaseName.append(".csv");
}

QString &CSVRankingPrinter::checkOutFileNameExtension(QString &outFileBaseName)
{
    return (!outFileBaseName.endsWith(".csv", Qt::CaseInsensitive)) ? buildOutFileName(outFileBaseName) : outFileBaseName;
}
