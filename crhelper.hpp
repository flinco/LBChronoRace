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

#ifndef CRHELPER_HPP
#define CRHELPER_HPP

#include <QCoreApplication>
#include <QString>
#include <QVariantList>

#include "crloader.hpp"
#include "chronoracedata.hpp"
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

private:
    static QWidget *parentWidget;

public:
    static ChronoRaceData::NameComposition nameComposition;
    static ChronoRaceData::Accuracy accuracy;

    static QString encodingToLabel(QStringConverter::Encoding const &value);
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

    static QString toTimeString(uint milliseconds, Timing::Status const status, char const *prefix = Q_NULLPTR);
    static QString toTimeString(uint milliseconds, ChronoRaceData::Accuracy acc);
    static QString toTimeString(Timing const &timing);

    static Timing::Status toTimingStatus(QString const &status);
    static QString toStatusString(Timing::Status const status);
    static QString toStatusFullString(Timing::Status const status);

    static bool askForAppend(QWidget *parent = Q_NULLPTR);

    static void setParent(QWidget *newParent);

    static void pushTiming(QVariantList &list, uint timing);
    static bool popTiming(QVariantList &list, uint timing);

public slots:
    static void updateGlobalData(ChronoRaceData::NameComposition newNameComposition, ChronoRaceData::Accuracy newAccuracy);

    static void actionAbout();
    static void actionAboutQt();
};

#endif // CRHELPER_HPP
