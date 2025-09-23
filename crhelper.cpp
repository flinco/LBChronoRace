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

#include <QMessageBox>

#include "crhelper.hpp"
#include "lbcrexception.hpp"

QWidget *CRHelper::parentWidget = Q_NULLPTR;
ChronoRaceData::NameComposition CRHelper::nameComposition = ChronoRaceData::NameComposition::SURNAME_FIRST;
ChronoRaceData::Accuracy CRHelper::accuracy = ChronoRaceData::Accuracy::SECOND;

void CRHelper::setParent(QWidget *newParent)
{
    parentWidget = newParent;
}

QString CRHelper::encodingToLabel(QStringConverter::Encoding const &value)
{
    switch (value) {
        using enum QStringConverter::Encoding;

        case Utf8:
            return tr("UTF-8");
        case Latin1:
            return tr("ISO-8859-1 (Latin-1)");
        default:
            return tr("The encoding format %1 is not supported").arg(static_cast<int>(value));
    }
}

QString CRHelper::formatToLabel(CRLoader::Format const &value)
{
    switch (value) {
        using enum CRLoader::Format;

        case PDF:
            return tr("PDF");
        case TEXT:
            return tr("Plain text");
        case CSV:
            return tr("CSV");
        default:
            return tr("Unknown format %1").arg(static_cast<int>(value));
    }
}

Competitor::Sex CRHelper::toSex(QString const &sex)
{
    using enum Competitor::Sex;

    if (sex.length() != 1) {
        throw(ChronoRaceException(tr("Invalid sex '%1'").arg(sex)));
    } else {
        if (sex.compare("M", Qt::CaseInsensitive) == 0)
            return MALE;
        else if (sex.compare("F", Qt::CaseInsensitive) == 0)
            return FEMALE;
    }

    return UNDEFINED;
}

QString CRHelper::toSexString(Competitor::Sex const sex)
{
    switch (sex) {
        using enum Competitor::Sex;

        case MALE:
            return "M";
        case FEMALE:
            return "F";
        case UNDEFINED:
            return "U";
        default:
            throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}

QString CRHelper::toSexFullString(Competitor::Sex const sex)
{
    switch (sex) {
        using enum Competitor::Sex;

        case MALE:
            return tr("Male");
        case FEMALE:
            return tr("Female");
        case UNDEFINED:
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
    using enum Ranking::Type;

    if (type.compare("I", Qt::CaseInsensitive) == 0)
        return INDIVIDUAL;
    else if (type.compare("T", Qt::CaseInsensitive) == 0)
        return CLUB;
    else
        throw(ChronoRaceException(tr("Invalid type '%1'").arg(type)));
}

QString CRHelper::toTypeString(Ranking::Type const type)
{
    switch (type) {
        using enum Ranking::Type;

        case INDIVIDUAL:
            return "I";
        case CLUB:
            return "T";
        default:
            throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

Category::Type CRHelper::toCategoryType(QString const &type)
{
    using enum Category::Type;

    if (type.compare("M", Qt::CaseInsensitive) == 0)
        return MALE;
    else if (type.compare("F", Qt::CaseInsensitive) == 0)
        return FEMALE;
    else if (type.compare("U", Qt::CaseInsensitive) == 0)
        return RELAY_MF;
    else if (type.compare("Y", Qt::CaseInsensitive) == 0)
        return RELAY_Y;
    else if (type.compare("X", Qt::CaseInsensitive) == 0)
        return RELAY_X;
    else
        throw(ChronoRaceException(tr("Invalid type '%1'").arg(type)));
}

QString CRHelper::toTypeString(Category::Type const type)
{
    switch (type) {
        using enum Category::Type;

        case MALE:
            return "M";
        case FEMALE:
            return "F";
        case RELAY_MF:
            return "U";
        case RELAY_Y:
            return "Y";
        case RELAY_X:
            return "X";
        default:
            throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toRankingTypeString(Ranking::Type type)
{
    switch (type) {
        using enum Ranking::Type;

        case INDIVIDUAL:
            return tr("Individual/Relay");
        case CLUB:
            return tr("Club");
        default:
            throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toCategoryTypeString(Category::Type const type)
{
    switch (type) {
        using enum Category::Type;

        case MALE:
            return tr("Individual/Relay (M)");
        case FEMALE:
            return tr("Individual/Relay (F)");
        case RELAY_MF:
            return tr("Mixed Relay (M/F)");
        case RELAY_Y:
            return tr("Mixed Club Relay (M)");
        case RELAY_X:
            return tr("Mixed Club Relay (F)");
        default:
            throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}

QString CRHelper::toTimeString(uint milliseconds, Timing::Status const status, char const *prefix)
{
    QString retString { prefix ? prefix : "" };

    switch (status) {
        using enum Timing::Status;

        case CLASSIFIED:
            retString.append(CRHelper::toTimeString(milliseconds, accuracy));
            break;
        case DSQ:
            retString.append("DSQ");
            break;
        case DNF:
            retString.append("DNF");
            break;
        case DNS:
            retString.append("DNS");
            break;
        default:
            throw(ChronoRaceException(tr("Invalid status value %1").arg(static_cast<int>(status))));
    }

    return retString;
}

QString CRHelper::toTimeString(uint milliseconds, ChronoRaceData::Accuracy acc)
{
    QString retString { "" };

    auto hr = milliseconds / 3600000;
    milliseconds %= 3600000;
    auto min = milliseconds / 60000;
    milliseconds %= 60000;
    auto sec = milliseconds / 1000;
    milliseconds %= 1000;

    switch (acc) {
        using enum ChronoRaceData::Accuracy;

        case SECOND:
            retString.append(QString("%1:%2:%3").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec + ((milliseconds < 500) ? 0 : 1), 2, 10, QLatin1Char('0')));
            break;
        case TENTH:
            retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg((milliseconds + 50) / 100, 1, 10, QLatin1Char('0')));
            break;
        case HUNDREDTH:
            retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg((milliseconds + 5) / 10, 2, 10, QLatin1Char('0')));
            break;
        case THOUSANDTH:
            retString.append(QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0')).arg(milliseconds, 3, 10, QLatin1Char('0')));
            break;
        default:
            throw(ChronoRaceException(tr("Invalid accuracy value %1").arg(static_cast<int>(acc))));
    }

    return retString;
}

QString CRHelper::toTimeString(Timing const &timing)
{
    return toTimeString(timing.getMilliseconds(), timing.getStatus());
}

Timing::Status CRHelper::toTimingStatus(QString const &status)
{
    using enum Timing::Status;

    if (status.compare("CLS", Qt::CaseInsensitive) == 0)
        return CLASSIFIED;
    else if (status.compare("DSQ", Qt::CaseInsensitive) == 0)
        return DSQ;
    else if (status.compare("DNF", Qt::CaseInsensitive) == 0)
        return DNF;
    else if (status.compare("DNS", Qt::CaseInsensitive) == 0)
        return DNS;
    else
        throw(ChronoRaceException(tr("Invalid status value '%1'").arg(status)));
}

QString CRHelper::toStatusString(Timing::Status const status)
{
    QString retString { "" };

    switch (status) {
        using enum Timing::Status;

        case CLASSIFIED:
            retString.append("CLS");
            break;
        case DSQ:
            retString.append("DSQ");
            break;
        case DNF:
            retString.append("DNF");
            break;
        case DNS:
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
        using enum Timing::Status;

        case CLASSIFIED:
            return tr("Classified");
        case DSQ:
            return tr("Disqualified");
        case DNF:
            return tr("Did not finish");
        case DNS:
            return tr("Did not start");
        default:
            throw(ChronoRaceException(tr("Invalid status value %1").arg(static_cast<int>(status))));
    }
}

void CRHelper::pushTiming(QVariantList &list, uint timing)
{
    QVariant value = QVariant::fromValue(timing);
    qsizetype index;

    /* Walk backwards to find the correct insertion point */
    for (index = list.isEmpty() ? 0 : list.size(); index > 0; index--) {
        if (timing >= list[index - 1].toUInt()) {
            break;
        }
    }

    /* Insert the value at the computed position */
    list.insert(index, value);
}

bool CRHelper::popTiming(QVariantList &list, uint timing)
{
    bool retval = false;

    QVariantList::ConstIterator it = list.constBegin();
    while (it != list.constEnd()) {

        /* Try to find a matching timing */
        if (it->toUInt() == timing) {
            list.erase(it);
            retval = true;
            break;
        }

        it++;
    }

    return retval;
}

bool CRHelper::askForAppend(QWidget *parent)
{
    QMessageBox msgBox(QMessageBox::Icon::Question, tr("Import method"), tr("Do you want to replace the table data or add new entries?"), QMessageBox::NoButton, parent);

    auto *pButtonReplace = msgBox.addButton(tr("Replace"), QMessageBox::NoRole);
    QAbstractButton const *pButtonAppend = msgBox.addButton(tr("Append"), QMessageBox::YesRole);
    msgBox.setDefaultButton(pButtonReplace);

    msgBox.exec();
    return (msgBox.clickedButton() == pButtonAppend);
}

void CRHelper::updateGlobalData(ChronoRaceData::NameComposition newNameComposition, ChronoRaceData::Accuracy newAccuracy)
{
    CRHelper::nameComposition = newNameComposition;
    CRHelper::accuracy = newAccuracy;
}

void CRHelper::actionAbout()
{
    /* The purpose of the following object is to prevent the "About Qt"  *
     * translation from being removed from the .tr files when refreshed. */
    QString const translatedTextAboutQtMessage = QMessageBox::tr(
        "<p>Qt is a C++ toolkit for cross-platform application development.</p>" //NOSONAR
        "<p>Qt provides single-source portability across all major desktop operating systems. "
        "It is also available for embedded Linux and other embedded and mobile operating systems.</p>"
        "<p>Qt is available under multiple licensing options designed to accommodate the needs of our various users.</p>"
        "<p>Qt licensed under our commercial license agreement is appropriate for development of "
        "proprietary/commercial software where you do not want to share any source code with "
        "third parties or otherwise cannot comply with the terms of GNU (L)GPL.</p>"
        "<p>Qt licensed under GNU (L)GPL is appropriate for the development of Qt&nbsp;applications "
        "provided you can comply with the terms and conditions of the respective licenses.</p>"
        "<p>Please see <a href=\"https://%2/\">%2</a> for an overview of Qt licensing.</p>"
        "<p>Copyright (C) The Qt Company Ltd. and other contributors.</p>"
        "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>"
        "<p>Qt is The Qt Company Ltd. product developed as an open source project. "
        "See <a href=\"https://%3/\">%3</a> for more information.</p>"
        );
    std::ignore = translatedTextAboutQtMessage;

    QString const translatedTextAboutCaption = QMessageBox::tr(
                                                   "<h3>About %1</h3>"
                                                   "<p>Software for producing the results of footraces.</p>"
                                                   ).arg(QStringLiteral(LBCHRONORACE_NAME));
    QString const translatedTextAboutText = QMessageBox::tr(
                                                "<p>Copyright&copy; 2021-2025</p>" //NOSONAR
                                                "<p>Version: %1 (source code on <a href=\"http://github.com/flinco/LBChronoRace\">GitHub</a>)</p>"
                                                "<p>Author: Lorenzo Buzzi (<a href=\"mailto:lorenzo@buzzi.pro\">lorenzo@buzzi.pro</a>)</p>"
                                                "<p>Site: <a href=\"http://www.buzzi.pro/\">http://www.buzzi.pro/</a></p>"
                                                "<p>%2 is free software: you can redistribute it and/or modify it under the terms of "
                                                "the GNU General Public License as published by the Free Software Foundation; either "
                                                "version 3 of the License, or (at your option) any later version.</p>"
                                                "<p>%2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
                                                "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
                                                "See the GNU General Public License for more details.</p>"
                                                "<p>You should have received a copy of the GNU General Public License along with %2. "
                                                "If not, see: <a href=\"https://www.gnu.org/licenses/\">https://www.gnu.org/licenses/</a>.</p>"
                                                "<p><table><tbody><tr>"
                                                "<td>If you found this application useful<br>and want to support its development,<br>you can make a donation:</td>"
                                                "<td><a href=\"https://www.paypal.com/donate/?hosted_button_id=8NZWAMWPKCA7C\"><img src=\":/images/PayPal_Donate_en.gif\" /></a></td>"
                                                "</tr></tbody></table></p>"
                                                ).arg(QStringLiteral(LBCHRONORACE_VERSION), QStringLiteral(LBCHRONORACE_NAME));
    QMessageBox msgBox(parentWidget);
    msgBox.setWindowTitle(tr("About %1").arg(QStringLiteral(LBCHRONORACE_NAME)));
    msgBox.setText(translatedTextAboutCaption);
    msgBox.setInformativeText(translatedTextAboutText);
    if (QPixmap pm(QStringLiteral(":/icons/LBChronoRace.png")); !pm.isNull()) {
        msgBox.setIconPixmap(pm);
    }
    msgBox.exec();
}

void CRHelper::actionAboutQt()
{
    QMessageBox::aboutQt(parentWidget, tr("About Qt"));
}
