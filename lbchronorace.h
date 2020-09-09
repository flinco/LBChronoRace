#ifndef LBCHRONORACE_H
#define LBCHRONORACE_H

#include <QMainWindow>
#include <QTranslator>
#include <QAbstractButton>
#include <QStandardPaths>
#include <QDir>

#include "chronoracetable.h"

#define LBCHRONORACE_NAME "LBChronoRace"
#define LBCHRONORACE_VERSION "1.0.0"
#define LBCHRONORACE_STARTLIST_DEFAULT    "startlist.csv"
#define LBCHRONORACE_TEAMLIST_DEFAULT     "teamlist.csv"
#define LBCHRONORACE_TIMINGS_DEFAULT      "timings.csv"
#define LBCHRONORACE_CATEGORIES_DEFAULT   "categories.csv"

namespace Ui {
class LBChronoRace;
}

class LBChronoRace : public QMainWindow
{
    Q_OBJECT

public:
    explicit LBChronoRace(QWidget *parent = 0);
    ~LBChronoRace();

private slots:
    void on_loadStartList_clicked();
    void on_loadCategories_clicked();
    void on_loadTimings_clicked();
    void on_makeRankings_clicked();
    void on_actionQuit_triggered();
    void on_actionStartListView_triggered();
    void on_actionTeamsView_triggered();
    void on_actionCategoriesView_triggered();
    void on_actionTimingsView_triggered();
    void on_selectorEncoding_activated(const QString &arg1);
    void on_selectorFormat_activated(const QString &arg1);

    void on_viewStartList_clicked();
    void on_viewTeamsList_clicked();
    void on_viewCategories_clicked();
    void on_viewTimings_clicked();

    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

public slots:
    void show();

    void on_actionSave_triggered();

    void set_counterTeams(int count);
    void set_counterCompetitors(int count);
    void set_counterCategories(int count);
    void set_counterTimings(int count);

private:
    Ui::LBChronoRace *ui;

    QDir    lastSelectedPath;
    QString startListFileName;
    QString timingsFileName;
    QString categoriesFileName;
    QString teamsFileName;

    ChronoRaceTable startListTable;
    ChronoRaceTable teamsTable;
    ChronoRaceTable timingsTable;
    ChronoRaceTable categoriesTable;

    void save_startList();
    void save_teamList();
    void save_categoriesList();
    void save_timingsList();

    //void enable_makeRankings();
};

#endif // LBCHRONORACE_H
