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

#include <QFile>

#include "crsettings.hpp"

QSettings CRSettings::settings { QSettings::Format::NativeFormat, QSettings::Scope::UserScope, QStringLiteral(LBCHRONORACE_ORGANIZATION), QStringLiteral(LBCHRONORACE_NAME) };

QString CRSettings::getLanguage()
{
    return settings.value("UILanguage", QVariant("en")).toString();
}

void CRSettings::setLanguage(QString const &language)
{
    settings.setValue("UILanguage", language);
}

void CRSettings::readRecent(QStringList &recentRaces)
{
    QFile recentFile;
    QString recentPath;

    recentRaces.clear();
    int size = settings.beginReadArray("RecentRaces");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        recentPath = settings.value("path").toString();
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
    settings.beginWriteArray("RecentRaces");
    for (auto const &recentRacePath : std::as_const(recentRaces)) {
        settings.setArrayIndex(i);
        settings.setValue("path", recentRacePath);
        i++;
    }
    settings.endArray();
}
