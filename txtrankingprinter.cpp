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

#include "txtrankingprinter.hpp"
#include "chronoracedata.hpp"
#include "lbcrexception.hpp"
#include "crhelper.hpp"

void TXTRankingPrinter::init(QString *outTxtFileName, QString const &title, QString const &subject, QTranslator const *translator)
{
    Q_ASSERT(!txtFile.isOpen());

    Q_UNUSED(title)
    Q_UNUSED(subject)

    if (translator == Q_NULLPTR) {
        throw(ChronoRaceException(tr("Error: no translator provided")));
    }
    setTranslator(translator);

    if (outTxtFileName == Q_NULLPTR) {
        throw(ChronoRaceException(tr("Error: no file name provided")));
    }

    // append the .txt extension if missing
    checkOutFileNameExtension(*outTxtFileName);

    txtFile.setFileName(*outTxtFileName);
    if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: unable to open %1").arg(*outTxtFileName)));
    }
    txtStream.setDevice(&txtFile);
    txtStream.setEncoding(CRLoader::getEncoding());
}

void TXTRankingPrinter::printStartList(QList<Competitor const *> const &startList)
{
    if (!txtFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: attempted to write to a closed file")));
    }

    QTranslator const *translator = getTranslator();

    auto nWidth = CRLoader::getStartListNameWidthMax();
    auto tWidth = CRLoader::getTeamNameWidthMax();

    int offset;
    int i = 0;
    QString lastColumnValue;
    ChronoRaceData const *raceInfo = getRaceInfo();
    QTime startTime = raceInfo->getStartTime();
    txtStream << *raceInfo << Qt::endl; // add header
    txtStream << translator->translate("TXTRankingPrinter", "Start List") << Qt::endl;
    for (auto const &competitor : startList) {
        i++;

        offset = competitor->getOffset();
        if (offset >= 0) {
            offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
            lastColumnValue = CRHelper::toOffsetString(offset);
        } else if (CRLoader::getStartListLegs() == 1) {
            offset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
            lastColumnValue = CRHelper::toOffsetString(offset);
        } else {
            QString legValue = translator->translate("TXTRankingPrinter", "Leg %1").arg(qAbs(offset));
            if (!lastColumnValue.isEmpty() && (legValue != lastColumnValue))
                txtStream << Qt::endl;
            lastColumnValue = legValue;
        }

        txtStream.setFieldWidth(getIndexFieldWidth());
        txtStream.setFieldAlignment(QTextStream::AlignRight);
        txtStream << i;
        txtStream.setFieldWidth(0);
        txtStream << " - ";
        txtStream.setFieldWidth(getBibFieldWidth());
        txtStream.setFieldAlignment(QTextStream::AlignRight);
        txtStream << competitor->getBib();
        txtStream.setFieldWidth(0);
        txtStream << " - ";
        txtStream << competitor->getCompetitorName(CRHelper::nameComposition, nWidth);
        txtStream << " - ";
        txtStream << competitor->getTeam(tWidth);
        txtStream << " - ";
        txtStream << competitor->getYear();
        txtStream << " - ";
        txtStream << CRHelper::toSexString(competitor->getSex());
        txtStream << " - ";
        txtStream.setFieldWidth(15);
        txtStream.setFieldAlignment(QTextStream::AlignRight);
        txtStream << lastColumnValue;
        txtStream.setFieldWidth(0);
        txtStream << Qt::endl;
    }
    txtStream << Qt::endl;
}

void TXTRankingPrinter::printRanking(Ranking const &categories, QList<ClassEntry const *> const &ranking)
{
    static Position position;

    if (!txtFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: attempted to write to a closed file")));
    }

    int i = 0;
    int prevPosNumber = 0;
    int currPosNumber = 0;
    QString currTime;
    ChronoRaceData const *raceInfo = getRaceInfo();
    txtStream << *raceInfo << Qt::endl; // add header
    txtStream << categories.getFullDescription() << Qt::endl;
    for (auto const c : ranking) {
        i++;
        currTime = c->getTotalTime(CRLoader::Format::TEXT);
        if ((currPosNumber = position.getCurrentPositionNumber(i, currTime)) == 0)
            currPosNumber = prevPosNumber;
        else
            prevPosNumber = currPosNumber;
        txtStream.setFieldWidth(getIndexFieldWidth());
        txtStream.setFieldAlignment(QTextStream::AlignRight);
        txtStream << currPosNumber;
        txtStream.setFieldWidth(0);
        txtStream << " - ";
        txtStream.setFieldWidth(getBibFieldWidth());
        txtStream.setFieldAlignment(QTextStream::AlignRight);
        txtStream << c->getBib();
        txtStream.setFieldWidth(0);
        txtStream << " - ";
        txtStream << c->getNames(CRLoader::Format::TEXT);
        txtStream << " - ";
        if (CRLoader::getStartListLegs() > 1)
            txtStream << c->getTimes(CRLoader::Format::TEXT, getIndexFieldWidth()) << " - ";
        txtStream << currTime << Qt::endl;
    }
    txtStream << Qt::endl;
}

void TXTRankingPrinter::printRanking(Ranking const &categories, QList<TeamClassEntry const *> const &ranking)
{
    if (!txtFile.isOpen()) {
        throw(ChronoRaceException(tr("Error: attempted to write to a closed file")));
    }

    int count;
    int i = 0;
    ChronoRaceData const *raceInfo = getRaceInfo();
    txtStream << *raceInfo << Qt::endl; // add header
    txtStream << categories.getFullDescription() << Qt::endl;
    for (auto const r : ranking) {
        i++;
        count = r->getClassEntryCount();
        for (int j = 0; j < count; j++) {
            if (j == 0) {
                txtStream.setFieldWidth(getIndexFieldWidth());
                txtStream.setFieldAlignment(QTextStream::AlignRight);
                txtStream << i;
                txtStream.setFieldWidth(0);
                txtStream << " - ";
            } else {
                txtStream.setFieldWidth(getIndexFieldWidth() + 3);
                txtStream << "";
            }
            txtStream.setFieldWidth(getBibFieldWidth());
            txtStream.setFieldAlignment(QTextStream::AlignRight);
            txtStream << r->getClassEntry(j)->getBib();
            txtStream.setFieldWidth(0);
            txtStream << " - ";
            txtStream << r->getClassEntry(j)->getNames(CRLoader::Format::TEXT);
            txtStream << " - ";
            if (CRLoader::getStartListLegs() > 1)
                txtStream << r->getClassEntry(j)->getTimes(CRLoader::Format::TEXT, getIndexFieldWidth()) << " - ";
            txtStream << r->getClassEntry(j)->getTotalTime(CRLoader::Format::TEXT) << Qt::endl;
        }
        txtStream << Qt::endl;
    }
    txtStream << Qt::endl;
}

bool TXTRankingPrinter::finalize()
{
    bool result = true;

    if (QIODevice *device = txtStream.device(); device != Q_NULLPTR) {
        txtStream.flush();
        device->close();
    } else {
        result = false;
    }

    return result;
}

QString TXTRankingPrinter::getFileFilter()
{
    return tr("Plain Text (*.txt)");
}

QString &TXTRankingPrinter::buildOutFileName(QString &outFileBaseName)
{
    return outFileBaseName.append(".txt");
}

QString &TXTRankingPrinter::checkOutFileNameExtension(QString &outFileBaseName)
{
    return (!outFileBaseName.endsWith(".txt", Qt::CaseInsensitive)) ? buildOutFileName(outFileBaseName) : outFileBaseName;
}
