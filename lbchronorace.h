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
#include <QTranslator>
#include <QAbstractButton>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QList>
#include <QFontDatabase>

#include "ui_chronorace.h"

#include "chronoracetable.h"
#include "chronoracedata.h"
#include "chronoracetimings.h"

#ifndef LBCHRONORACE_NAME
#error "LBCHRONORACE_NAME not set"
#endif
#ifndef LBCHRONORACE_VERSION
#error "LBCHRONORACE_VERSION not set"
#endif

constexpr char LBCHRONORACE_STARTLIST_DEFAULT[]  = "startlist.csv";
constexpr char LBCHRONORACE_TEAMLIST_DEFAULT[]   = "teamlist.csv";
constexpr char LBCHRONORACE_TIMINGS_DEFAULT[]    = "timings.csv";
constexpr char LBCHRONORACE_CATEGORIES_DEFAULT[] = "categories.csv";

constexpr int LBCHRONORACE_BIN_FMT_v1 = 1;
constexpr int LBCHRONORACE_BIN_FMT_v2 = 2;
#define LBCHRONORACE_BIN_FMT LBCHRONORACE_BIN_FMT_v2

class LBChronoRace : public QMainWindow
{
    Q_OBJECT

public:
    explicit LBChronoRace(QWidget *parent = Q_NULLPTR, QGuiApplication const *app = Q_NULLPTR);

    static QDir lastSelectedPath;

public slots:
    void initialize();
    void show(); //NOSONAR
    void resizeDialogs(QScreen const *screen);

    void setCounterTeams(int count) const;
    void setCounterCompetitors(int count) const;
    void setCounterCategories(int count) const;
    void setCounterTimings(int count) const;

    void appendInfoMessage(QString const &message) const;
    void appendErrorMessage(QString const &message) const;

private:
    QScopedPointer<Ui::LBChronoRace> ui { new Ui::LBChronoRace };

    QString raceDataFileName { "" };
    QString startListFileName;
    QString timingsFileName;
    QString categoriesFileName;
    QString teamsFileName;

    ChronoRaceData raceInfo;

    ChronoRaceTable startListTable;
    ChronoRaceTable teamsTable;
    ChronoRaceTable categoriesTable;
    ChronoRaceTable timingsTable;

    ChronoRaceTimings timings;

    bool loadRaceFile(QString const &fileName);

private slots:
    void actionAbout();
    void actionAboutQt();

    void loadRace();
    void saveRace();
    void saveRaceAs();

    void selectorEncoding(QString const &arg1) const;
    void selectorFormat(QString const &arg1) const;
    void makeStartList();
    void makeRankings();

    void importStartList();
    void importCategoriesList();
    void importTimingsList();

    void exportStartList();
    void exportTeamList();
    void exportCategoriesList();
    void exportTimingsList();
};

#endif // LBCHRONORACE_H
