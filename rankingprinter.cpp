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

#include "rankingprinter.h"
#include "pdfrankingprinter.h"
#include "csvrankingprinter.h"
#include "txtrankingprinter.h"
#include "lbcrexception.h"

QScopedPointer<RankingPrinter> RankingPrinter::getRankingPrinter(CRLoader::Format format, uint indexFieldWidth, uint bibFieldWidth)
{
    switch (format) {
    case CRLoader::Format::PDF:
        return QScopedPointer<RankingPrinter>(new PDFRankingPrinter(indexFieldWidth, bibFieldWidth));
        break;
    case CRLoader::Format::CSV:
        return QScopedPointer<RankingPrinter>(new CSVRankingPrinter(indexFieldWidth, bibFieldWidth));
        break;
    case CRLoader::Format::TEXT:
        return QScopedPointer<RankingPrinter>(new TXTRankingPrinter(indexFieldWidth, bibFieldWidth));
        break;
    default:
        throw(ChronoRaceException(tr("Error: unknown rankings format %1").arg(static_cast<int>(format))));
        break;
    }
}

uint RankingPrinter::getIndexFieldWidth() const
{
    return indexFieldWidth;
}

uint RankingPrinter::getBibFieldWidth() const
{
    return bibFieldWidth;
}

ChronoRaceData const *RankingPrinter::getRaceInfo() const
{
    return raceInfo;
}

void RankingPrinter::setRaceInfo(ChronoRaceData const *newRaceInfo)
{
    raceInfo = newRaceInfo;
}

int Position::getCurrentPositionNumber(int posIndex, QString const &currTime)
{
    int returnedPosition = 0;

    // Return 0 if the position has not changed
    if ((posIndex == 1) || (time.compare(currTime) != 0)) {
        // First entry or Different time
        time = currTime;
        returnedPosition = position = posIndex;
    }

    return returnedPosition;
}

QString Position::getCurrentPositionString(int posIndex, QString const &currTime)
{
    QString returnedPosition;

    if (int currPos = getCurrentPositionNumber(posIndex, currTime); currPos != 0) {
        returnedPosition = QString::number(currPos).append(".");
    }

    return returnedPosition;
}

uint PositionNumber::getCurrentPosition(uint posIndex, uint currTime)
{
    // Always return the position value
    if ((posIndex == 1u) || (time != currTime)) {
        // First entry or Different time
        position = posIndex;
        time = currTime;
    }

    return position;
}
