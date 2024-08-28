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

#ifndef LBCHRONORACE_H
#define LBCHRONORACE_H

#include <QGuiApplication>
#include <QMainWindow>
#include <QDir>
#include <QScopedPointer>
#include <QRegularExpression>

#include "ui_chronorace.h"

#include "chronoracetable.hpp"
#include "chronoracedata.hpp"
#include "chronoracetimings.hpp"
#include "compsexdelegate.hpp"
#include "clubdelegate.hpp"
#include "rankingtypedelegate.hpp"
#include "rankingcatsdelegate.hpp"
#include "cattypedelegate.hpp"
#include "timingstatusdelegate.hpp"

#ifndef LBCHRONORACE_NAME
#error "LBCHRONORACE_NAME not set"
#endif
#ifndef LBCHRONORACE_VERSION
#error "LBCHRONORACE_VERSION not set"
#endif

constexpr char LBCHRONORACE_STARTLIST_DEFAULT[]  = "startlist.csv";
constexpr char LBCHRONORACE_TEAMLIST_DEFAULT[]   = "teamlist.csv";
constexpr char LBCHRONORACE_TIMINGS_DEFAULT[]    = "timings.csv";
constexpr char LBCHRONORACE_RANKINGS_DEFAULT[]   = "rankings.csv";
constexpr char LBCHRONORACE_CATEGORIES_DEFAULT[] = "categories.csv";

constexpr uint LBCHRONORACE_BIN_FMT_v1 = 1u;
constexpr uint LBCHRONORACE_BIN_FMT_v2 = 2u;
constexpr uint LBCHRONORACE_BIN_FMT_v3 = 3u;
constexpr uint LBCHRONORACE_BIN_FMT_v4 = 4u;
#define LBCHRONORACE_BIN_FMT LBCHRONORACE_BIN_FMT_v4

class LBChronoRace : public QMainWindow
{
    Q_OBJECT

public:
    explicit LBChronoRace(QWidget *parent = Q_NULLPTR, QGuiApplication const *app = Q_NULLPTR);

    static QDir lastSelectedPath;
    static uint binFormat;

    static QRegularExpression csvFilter;

public slots:
    void initialize();
    void show(); //NOSONAR
    void resizeDialogs(QScreen const *screen);

    void setCounterTeams(int count) const;
    void setCounterCompetitors(int count) const;
    void setCounterRankings(int count) const;
    void setCounterCategories(int count) const;
    void setCounterTimings(int count) const;

    void appendInfoMessage(QString const &message) const;
    void appendErrorMessage(QString const &message) const;

private:
    QScopedPointer<Ui::LBChronoRace> ui { new Ui::LBChronoRace };

    QString raceDataFileName { "" };
    QString startListFileName;
    QString timingsFileName;
    QString rankingsFileName;
    QString categoriesFileName;
    QString teamsFileName;

    ChronoRaceData raceInfo;

    ChronoRaceTable startListTable;
    ChronoRaceTable teamsTable;
    ChronoRaceTable rankingsTable;
    ChronoRaceTable categoriesTable;
    ChronoRaceTable timingsTable;

    ChronoRaceTimings timings;

    CompetitorSexDelegate sexDelegate;
    ClubDelegate clubDelegate;
    RankingTypeDelegate rankingTypeDelegate;
    RankingCategoriesDelegate rankingCatsDelegate;
    CategoryTypeDelegate categoryTypeDelegate;
    TimingStatusDelegate timingStatusDelegate;

    bool loadRaceFile(QString const &fileName);

private slots:
    void actionAbout();
    void actionAboutQt();

    void loadRace();
    void saveRace();
    void saveRaceAs();

    void setEncoding();

    void encodingSelector(int idx) const;
    void formatSelector(int idx) const;
    void makeStartList();
    void makeRankings();

    void importStartList();
    void importRankingsList();
    void importCategoriesList();
    void importTimingsList();

    void exportStartList();
    void exportTeamList();
    void exportRankingsList();
    void exportCategoriesList();
    void exportTimingsList();
};

#endif // LBCHRONORACE_H
