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

#include "crhelper.hpp"
#include "lbcrexception.hpp"

ChronoRaceData::NameComposition CRHelper::nameComposition = ChronoRaceData::NameComposition::SURNAME_FIRST;
ChronoRaceData::Accuracy CRHelper::accuracy = ChronoRaceData::Accuracy::SECOND;

QString CRHelper::encodingToLabel(QStringConverter::Encoding const &value)
{
    switch (value) {
    case QStringConverter::Encoding::Utf8:
        return tr("UTF-8");
    case QStringConverter::Encoding::Latin1:
        return tr("ISO-8859-1 (Latin-1)");
    default:
        return tr("Unsupported encoding %1").arg(static_cast<int>(value));
    }
}

QString CRHelper::formatToLabel(CRLoader::Format const &value)
{
    switch (value) {
    case CRLoader::Format::PDF:
        return tr("PDF");
    case CRLoader::Format::TEXT:
        return tr("Text");
    case CRLoader::Format::CSV:
        return tr("CSV");
    default:
        return tr("Unknown format %1").arg(static_cast<int>(value));
    }
}

Competitor::Sex CRHelper::toSex(QString const &sex)
{
    if (sex.length() != 1) {
        throw(ChronoRaceException(tr("Illegal sex '%1'").arg(sex)));
    } else {
        if (sex.compare("M", Qt::CaseInsensitive) == 0)
            return Competitor::Sex::MALE;
        else if (sex.compare("F", Qt::CaseInsensitive) == 0)
            return Competitor::Sex::FEMALE;
    }

    return Competitor::Sex::UNDEFINED;
}

QString CRHelper::toSexString(Competitor::Sex const sex)
{
    switch (sex) {
    case Competitor::Sex::MALE:
        return "M";
    case Competitor::Sex::FEMALE:
        return "F";
    case Competitor::Sex::UNDEFINED:
        return "U";
    default:
        throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}

QString CRHelper::toSexFullString(Competitor::Sex const sex)
{
    switch (sex) {
    case Competitor::Sex::MALE:
        return tr("Male");
    case Competitor::Sex::FEMALE:
        return tr("Female");
    case Competitor::Sex::UNDEFINED:
        return tr("Not set");
    default:
        throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}

int CRHelper::toOffset(QString const &offset)
{
    QStringList list = offset.split(":");

    int retval = -1;
    int h;
    int m;
    int s;
    int l = 0;
    bool check_h;
    bool check_m;
    bool check_s;
    bool check_l;
    switch (list.count()) {
    case 3:
        h = list[0].toInt(&check_h, 10);
        m = list[1].toInt(&check_m, 10);
        s = list[2].toInt(&check_s, 10);
        if (check_h && check_m && check_s)
            retval = (h * 3600) + (m * 60) + s;
        break;
    case 2:
        m = list[0].toInt(&check_m, 10);
        s = list[1].toInt(&check_s, 10);
        if (check_m && check_s)
            retval = (m * 60) + s;
        break;
    case 1:
        l = list[0].toInt(&check_l, 10);
        if (check_l)
            retval = -qAbs(l);
        break;
    default:
        // do nothing
        break;
    }

    return retval;
}

QString CRHelper::toOffsetString(int offset)
{
    if (offset < 0)
        return QString("%1").arg(qAbs(offset));
    else
        return QString("%1:%2:%3").arg(((offset / 60) / 60)).arg(((offset / 60) % 60), 2, 10, QChar('0')).arg((offset % 60), 2, 10, QChar('0'));
}

Ranking::Type CRHelper::toRankingType(QString  const &type)
{
    if (type.compare("I", Qt::CaseInsensitive) == 0)
        return Ranking::Type::INDIVIDUAL;
    else if (type.compare("T", Qt::CaseInsensitive) == 0)
        return Ranking::Type::CLUB;
    else
        throw(ChronoRaceException(tr("Illegal type '%1'").arg(type)));
}

QString CRHelper::toTypeString(Ranking::Type const type)
{
    switch (type) {
    case Ranking::Type::INDIVIDUAL:
        return "I";
    case Ranking::Type::CLUB:
        return "T";
    default:
        throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

Category::Type CRHelper::toCategoryType(QString const &type)
{
    if (type.compare("M", Qt::CaseInsensitive) == 0)
        return Category::Type::MALE;
    else if (type.compare("F", Qt::CaseInsensitive) == 0)
        return Category::Type::FEMALE;
    else if (type.compare("U", Qt::CaseInsensitive) == 0)
        return Category::Type::RELAY_MF;
    else if (type.compare("Y", Qt::CaseInsensitive) == 0)
        return Category::Type::RELAY_Y;
    else if (type.compare("X", Qt::CaseInsensitive) == 0)
        return Category::Type::RELAY_X;
    else
        throw(ChronoRaceException(tr("Illegal type '%1'").arg(type)));
}

QString CRHelper::toTypeString(Category::Type const type)
{
    switch (type) {
    case Category::Type::MALE:
        return "M";
    case Category::Type::FEMALE:
        return "F";
    case Category::Type::RELAY_MF:
        return "U";
    case Category::Type::RELAY_Y:
        return "Y";
    case Category::Type::RELAY_X:
        return "X";
    default:
        throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toRankingTypeString(Ranking::Type type)
{
    switch (type) {
    case Ranking::Type::INDIVIDUAL:
        return tr("Individual/Relay");
    case Ranking::Type::CLUB:
        return tr("Club");
    default:
        throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toCategoryTypeString(Category::Type const type)
{
    switch (type) {
    case Category::Type::MALE:
        return tr("Individual/Relay (M)");
    case Category::Type::FEMALE:
        return tr("Individual/Relay (F)");
    case Category::Type::RELAY_MF:
        return tr("Mixed Relay (M/F)");
    case Category::Type::RELAY_Y:
        return tr("Mixed Clubs Relay (M)");
    case Category::Type::RELAY_X:
        return tr("Mixed Clubs Relay (F)");
    default:
        throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toTimeString(uint milliseconds, Timing::Status const status, char const *prefix)
{
    QString retString { prefix ? prefix : "" };

    switch (status) {
    case Timing::Status::CLASSIFIED:
        {
            auto hr = milliseconds / 3600000;
            milliseconds %= 3600000;
            auto min = milliseconds / 60000;
            milliseconds %= 60000;
            auto sec = milliseconds / 1000;
            milliseconds %= 1000;

            switch (accuracy) {
            case ChronoRaceData::Accuracy::SECOND:
                retString.append(QString("%1:%2:%3").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec + ((milliseconds < 500) ? 0 : 1), 2, 10, QLatin1Char('0')));
                break;
            case ChronoRaceData::Accuracy::TENTH:
                retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg((milliseconds + 50) / 100, 1, 10, QLatin1Char('0')));
                break;
            case ChronoRaceData::Accuracy::HUNDREDTH:
                retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg((milliseconds + 5) / 10, 2, 10, QLatin1Char('0')));
                break;
            case ChronoRaceData::Accuracy::THOUSANDTH:
                retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg(milliseconds, 3, 10, QLatin1Char('0')));
                break;
            default:
                throw(ChronoRaceException(tr("Invalid accuracy value %1").arg(static_cast<int>(accuracy))));
            }
        }
        break;
    case Timing::Status::DSQ:
        retString.append("DSQ");
        break;
    case Timing::Status::DNF:
        retString.append("DNF");
        break;
    case Timing::Status::DNS:
        retString.append("DNS");
        break;
    default:
        throw(ChronoRaceException(tr("Invalid status value %1").arg(static_cast<int>(status))));
    }

    return retString;
}

QString CRHelper::toTimeString(Timing const &timing)
{
    return toTimeString(timing.getMilliseconds(), timing.getStatus());
}

Timing::Status CRHelper::toTimingStatus(QString const &status)
{
    if (status.compare("CLS", Qt::CaseInsensitive) == 0)
        return Timing::Status::CLASSIFIED;
    else if (status.compare("DSQ", Qt::CaseInsensitive) == 0)
        return Timing::Status::DSQ;
    else if (status.compare("DNF", Qt::CaseInsensitive) == 0)
        return Timing::Status::DNF;
    else if (status.compare("DNS", Qt::CaseInsensitive) == 0)
        return Timing::Status::DNS;
    else
        throw(ChronoRaceException(tr("Illegal status value '%1'").arg(status)));
}

QString CRHelper::toStatusString(Timing::Status const status)
{
    QString retString { "" };

    switch (status) {
    case Timing::Status::CLASSIFIED:
        retString.append("CLS");
        break;
    case Timing::Status::DSQ:
        retString.append("DSQ");
        break;
    case Timing::Status::DNF:
        retString.append("DNF");
        break;
    case Timing::Status::DNS:
        retString.append("DNS");
        break;
    default:
        throw(ChronoRaceException(tr("Invalid status value %1").arg(static_cast<int>(status))));
    }

    return retString;
}

QString CRHelper::toStatusFullString(Timing::Status const status)
{
    switch (status) {
    case Timing::Status::CLASSIFIED:
        return tr("Classified");
    case Timing::Status::DSQ:
        return tr("Disqualified");
    case Timing::Status::DNF:
        return tr("Did not finish");
    case Timing::Status::DNS:
        return tr("Did not start");
    default:
        throw(ChronoRaceException(tr("Invalid status value %1").arg(static_cast<int>(status))));
    }
}

void CRHelper::updateGlobalData(ChronoRaceData::NameComposition newNameComposition, ChronoRaceData::Accuracy newAccuracy)
{
    CRHelper::nameComposition = newNameComposition;
    CRHelper::accuracy = newAccuracy;
}
