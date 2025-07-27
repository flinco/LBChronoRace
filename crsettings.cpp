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
constexpr char RECENT_RACES_KEY[]  = "RecentRaces";
constexpr char RECENT_RACES_PATH_KEY[]  = "path";

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
