/*****************************************************************************
 * Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include <QVariant>
#include <QFile>

#include "crsettings.hpp"

constexpr char UI_LANGUAGE_KEY[]  = "UILanguage";
constexpr char TRIGGER_KEY_KEY[]  = "TriggerKey";
constexpr char SCROLL_SEC_KEY[]  = "ScrollSeconds";
constexpr char RECENT_RACES_KEY[]  = "RecentRaces";
constexpr char RECENT_RACES_PATH_KEY[]  = "path";

constexpr char LS_TITLE_COLOR_KEY[] = "LiveStartListTitleColor";
constexpr char LS_TEXT_COLOR_KEY[] = "LiveStartListTextColor";
constexpr char LS_BACKGROUND_COLOR_KEY[] = "LiveStartListBackgroundColor";
constexpr char LR_TITLE_COLOR_KEY[] = "LiveRankingsTitleColor";
constexpr char LR_TEXT_COLOR_KEY[] = "LiveRankingsTextColor";
constexpr char LR_BACKGROUND_COLOR_KEY[] = "LiveRankingsBackgroundColor";

Qt::FocusPolicy CRSettings::focus { Qt::FocusPolicy::NoFocus };
QSettings CRSettings::settings { QSettings::Format::NativeFormat, QSettings::Scope::UserScope, QStringLiteral(LBCHRONORACE_ORGANIZATION), QStringLiteral(LBCHRONORACE_NAME) };

QString CRSettings::getLanguage()
{
    return settings.value(UI_LANGUAGE_KEY, QVariant("en")).toString();
}

void CRSettings::setLanguage(QString const &language)
{
    settings.setValue(UI_LANGUAGE_KEY, language);
}

QKeyCombination CRSettings::getTriggerKey()
{
    int key = settings.value(TRIGGER_KEY_KEY, QVariant(Qt::Key_unknown)).toInt();
    return (key != Qt::Key_unknown) ? QKeyCombination::fromCombined(key) : QKeyCombination(Qt::KeyboardModifier::AltModifier, Qt::Key::Key_F10);
}

void CRSettings::setTriggerKey(QKeyCombination const &key)
{
    settings.setValue(TRIGGER_KEY_KEY, QVariant(key.toCombined()));
}

void CRSettings::readRecent(QStringList &recentRaces)
{
    QFile recentFile;
    QString recentPath;

    recentRaces.clear();
    int size = settings.beginReadArray(RECENT_RACES_KEY);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        recentPath = settings.value(RECENT_RACES_PATH_KEY).toString();
        recentFile.setFileName(recentPath);

        if (!recentFile.exists())
            continue;

        recentRaces.append(recentPath);
    }
    settings.endArray();
}

void CRSettings::writeRecent(QStringList const &recentRaces)
{
    int i = 0;

    // Save recent races list
    settings.beginWriteArray(RECENT_RACES_KEY);
    for (auto const &recentRacePath : std::as_const(recentRaces)) {
        settings.setArrayIndex(i);
        settings.setValue(RECENT_RACES_PATH_KEY, recentRacePath);
        i++;
    }
    settings.endArray();
}

int CRSettings::getLiveScrollSeconds()
{
    bool ok = false;
    int seconds = settings.value(SCROLL_SEC_KEY, QVariant(10)).toInt(&ok);
    return ok ? seconds : 10;
}

void CRSettings::setLiveScrollSeconds(int seconds)
{
    settings.setValue(SCROLL_SEC_KEY, QVariant(seconds));
}

QColor CRSettings::getColor(CRSettings::Color color)
{
    switch (color) {
        using enum CRSettings::Color;

        case LiveStartListTitleColor:
            if (auto value = settings.value(LS_TITLE_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        case LiveStartListTextColor:
            if (auto value = settings.value(LS_TEXT_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        case LiveStartListBackgroundColor:
            if (auto value = settings.value(LS_BACKGROUND_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        case LiveRankingsTitleColor:
            if (auto value = settings.value(LR_TITLE_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        case LiveRankingsTextColor:
            if (auto value = settings.value(LR_TEXT_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        case LiveRankingsBackgroundColor:
            if (auto value = settings.value(LR_BACKGROUND_COLOR_KEY); value.isValid())
                return QColor::fromString(value.toString());
            break;
        default:
            Q_UNREACHABLE();
            break;
    }

    return CRSettings::getDefaultColor(color);
}

QColor CRSettings::getDefaultColor(CRSettings::Color color)
{
    switch (color) {
        using enum CRSettings::Color;

        case LiveStartListTitleColor:
            return QColor::fromRgb(qRgb(0xFF, 0x00, 0x00));
        case LiveStartListTextColor:
            return QColor::fromRgb(qRgb(0x00, 0x00, 0x00));
        case LiveStartListBackgroundColor:
            return QColor::fromRgb(qRgb(0xFF, 0xCC, 0x67));
        case LiveRankingsTitleColor:
            return QColor::fromRgb(qRgb(0xFF, 0x00, 0x00));
        case LiveRankingsTextColor:
            return QColor::fromRgb(qRgb(0x00, 0x00, 0x00));
        case LiveRankingsBackgroundColor:
            return QColor::fromRgb(qRgb(0x55, 0xAA, 0xFF));
        default:
            Q_UNREACHABLE();
            break;
    }
}

void CRSettings::setColor(CRSettings::Color color, QColor const &value)
{
    switch (color) {
        using enum CRSettings::Color;

        case LiveStartListTitleColor:
            settings.setValue(LS_TITLE_COLOR_KEY, QVariant(value.name()));
            break;
        case LiveStartListTextColor:
            settings.setValue(LS_TEXT_COLOR_KEY, QVariant(value.name()));
            break;
        case LiveStartListBackgroundColor:
            settings.setValue(LS_BACKGROUND_COLOR_KEY, QVariant(value.name()));
            break;
        case LiveRankingsTitleColor:
            settings.setValue(LR_TITLE_COLOR_KEY, QVariant(value.name()));
            break;
        case LiveRankingsTextColor:
            settings.setValue(LR_TEXT_COLOR_KEY, QVariant(value.name()));
            break;
        case LiveRankingsBackgroundColor:
            settings.setValue(LR_BACKGROUND_COLOR_KEY, QVariant(value.name()));
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

Qt::FocusPolicy CRSettings::popFocus()
{
    return CRSettings::focus;
}

void CRSettings::pushFocus(Qt::FocusPolicy newFocus)
{
    CRSettings::focus = newFocus;
}
