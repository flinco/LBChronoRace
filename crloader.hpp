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

#ifndef CRLOADER_H
#define CRLOADER_H

#include <QCoreApplication>
#include <QString>
#include <QList>
#include <QDataStream>

#include "timing.hpp"
#include "category.hpp"
#include "competitor.hpp"

#include "crtablemodel.hpp"
#include "startlistmodel.hpp"
#include "teamslistmodel.hpp"
#include "timingsmodel.hpp"
#include "rankingsmodel.hpp"
#include "categoriesmodel.hpp"

namespace loader {
class CRLoader;
}

class CRLoader
{
    Q_DECLARE_TR_FUNCTIONS(CRLoader)

public:
    enum class Encoding
    {
        LATIN1 = 0,
        UTF8 = 1
    };

    enum class Format
    {
        PDF = 0,
        TEXT = 1,
        CSV = 2
    };

    enum class Model
    {
        STARTLIST,
        TEAMSLIST,
        RANKINGS,
        CATEGORIES,
        TIMINGS
    };

private:
    static StartListModel              startListModel;
    static TeamsListModel              teamsListModel;
    static TimingsModel                timingsModel;
    static RankingsModel               rankingsModel;
    static CategoriesModel             categoriesModel;
    static QList<QVariant>             standardItemList;
    static Encoding                    encoding;
    static Format                      format;

    static void loadCSV(QString const &filePath, QAbstractTableModel *model);
    static void saveCSV(QString const &filePath, QAbstractTableModel const *model);

    static void checkString(QAbstractTableModel *model, QString &token, QChar character = '\0');

public:
    static void saveRaceData(QDataStream &out);
    static void loadRaceData(QDataStream &in);
    static QPair<int, int> importStartList(QString const &path);
    static QList<Competitor> getStartList();
    static uint getStartListLegs();
    static void setStartListLegs(uint leg);
    static uint getStartListBibMax();
    static uint getStartListNameWidthMax();
    static uint getTeamNameWidthMax();
    static void clearTimings();
    static void addTiming(QString const &bib, QString const &timing);
    static QList<Timing> const &getTimings();
    static int importModel(Model model, QString const &path);
    static void exportModel(Model model, QString const &path);
    static QList<Ranking> const &getRankings();
    static QList<Category> const &getCategories();
    static CRTableModel *getStartListModel();
    static CRTableModel *getTeamsListModel();
    static CRTableModel *getTimingsModel();
    static CRTableModel *getRankingsModel();
    static CRTableModel *getCategoriesModel();
    static Encoding getEncoding();
    static void setEncoding(Encoding const &value);
    static Format getFormat();
    static void setFormat(Format const &value);

    static QStringList getClubs();
};

#endif // CRLOADER_H
