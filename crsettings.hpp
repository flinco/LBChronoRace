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

#ifndef CRSETTINGS_HPP
#define CRSETTINGS_HPP

#include <QCoreApplication>
#include <QString>
#include <QSettings>

namespace crsettings {
    class CRSettings;
}

class CRSettings
{
    Q_DECLARE_TR_FUNCTIONS(CRSettings)

public:
    static QString getLanguage();
    static void setLanguage(QString const &language);
    static void readRecent(QStringList &recentRaces);
    static void writeRecent(QStringList const &recentRaces);

private:
    static QSettings settings;
};

#endif // CRSETTINGS_HPP
