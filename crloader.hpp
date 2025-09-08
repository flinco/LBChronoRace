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

#ifndef CRLOADER_HPP
#define CRLOADER_HPP

#include <QCoreApplication>
#include <QString>
#include <QStringConverter>
#include <QList>
#include <QVariantList>
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

    enum class Action
    {
        BEGIN,
        ADD,
        END
    };

private:
    static StartListModel              startListModel;
    static TeamsListModel              teamsListModel;
    static TimingsModel                timingsModel;
    static RankingsModel               rankingsModel;
    static CategoriesModel             categoriesModel;
    static QVariantList                standardItemList;
    static QStringConverter::Encoding  encoding;
    static Format                      format;

    static bool dirty;

    static void loadCSV(QString const &filePath, QAbstractTableModel *model);
    static void saveCSV(QString const &filePath, QAbstractTableModel const *model);

    static void checkString(QAbstractTableModel *model, QString &token, QChar character = '\0');

public:
    static void saveRaceData(QDataStream &out);
    static void loadRaceData(QDataStream &in);
    static QPair<int, int> importStartList(QString const &path, bool append = false);
    static QList<Competitor> getStartList();
    static uint getStartListLegs();
    static void setStartListLegs(uint leg);
    static uint getStartListBibMax();
    static uint getStartListNameWidthMax();
    static uint getTeamNameWidthMax();
    static void addTiming(Action action, QString const &bib = QString(), QString const &timing = QString());
    static QList<Timing> const &getTimings();
    static int importTeams(QString const &path, bool append = false);
    static int importCategories(QString const &path, bool append = false);
    static int importRankings(QString const &path, bool append = false);
    static int importTimings(QString const &path, bool append = false);
    static void exportModel(Model model, QString const &path);
    static QList<Ranking> const &getRankings();
    static QList<Category> const &getCategories();
    static CRTableModel *getStartListModel();
    static CRTableModel *getTeamsListModel();
    static CRTableModel *getTimingsModel();
    static CRTableModel *getRankingsModel();
    static CRTableModel *getCategoriesModel();
    static QStringConverter::Encoding getEncoding();
    static void setEncoding(QStringConverter::Encoding const &value);
    static Format getFormat();
    static void setFormat(Format const &value);

    static QStringList getClubs();

    static QString encodingSelector(int idx);
    static QString formatSelector(int idx);

    static bool isDirty();

public slots:
    static void setDirty(QModelIndex const &topLeft, QModelIndex const &bottomRight, QList<int> const &roles = QList<int>());
};

#endif // CRLOADER_HPP
