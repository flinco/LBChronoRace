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

#ifndef CRHELPER_H
#define CRHELPER_H

#include <QCoreApplication>
#include <QString>

#include "crloader.hpp"
#include "competitor.hpp"
#include "ranking.hpp"
#include "category.hpp"
#include "timing.hpp"

namespace helper {
class CRHelper;
}

class CRHelper
{
    Q_DECLARE_TR_FUNCTIONS(CRHelper)

public:
    static QString encodingToLabel(CRLoader::Encoding const &value);
    static QString formatToLabel(CRLoader::Format const &value);

    static Competitor::Sex toSex(QString const &sex);
    static QString toSexString(Competitor::Sex const sex);
    static QString toSexFullString(Competitor::Sex const sex);
    static int toOffset(QString const &offset);
    static QString toOffsetString(int offset);

    static Ranking::Type toRankingType(QString const &type);
    static QString toTypeString(Ranking::Type const type);

    static Category::Type toCategoryType(QString const &type);
    static QString toTypeString(Category::Type const type);

    static QString toRankingTypeString(Ranking::Type type);
    static QString toCategoryTypeString(Category::Type const type);

    static QString toTimeStr(uint const seconds, Timing::Status const status, char const *prefix = Q_NULLPTR);
    static QString toTimeStr(Timing const &timing);
};

#endif // CRHELPER_H
