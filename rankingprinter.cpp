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
