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

#include "chronoracetable.h"
#include "chronoracedata.h"
#include "crloader.h"
#include "classentry.h"
#include "teamclassentry.h"

#define LBCHRONORACE_NAME                 "LBChronoRace"
#define LBCHRONORACE_VERSION              "1.9.1"

#define LBCHRONORACE_STARTLIST_DEFAULT    "startlist.csv"
#define LBCHRONORACE_TEAMLIST_DEFAULT     "teamlist.csv"
#define LBCHRONORACE_TIMINGS_DEFAULT      "timings.csv"
#define LBCHRONORACE_CATEGORIES_DEFAULT   "categories.csv"

#define LBCHRONORACE_BIN_FMT_v1           1
#define LBCHRONORACE_BIN_FMT_v2           2
#define LBCHRONORACE_BIN_FMT              LBCHRONORACE_BIN_FMT_v2

namespace Ui {
class LBChronoRace;
}

class LBChronoRace : public QMainWindow
{
    Q_OBJECT

public:
    explicit LBChronoRace(QWidget *parent = Q_NULLPTR, QGuiApplication *app = Q_NULLPTR);
    ~LBChronoRace();

    static QDir lastSelectedPath;

private slots:
    void on_actionLoadRace_triggered();
    void on_actionSaveRace_triggered();
    void on_actionSaveRaceAs_triggered();
    void on_actionQuit_triggered();

    void on_actionEditRace_triggered();
    void on_actionEditStartList_triggered();
    void on_actionEditTeams_triggered();
    void on_actionEditCategories_triggered();
    void on_actionEditTimings_triggered();

    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

    void on_loadRace_clicked();
    void on_saveRace_clicked();
    void on_editRace_clicked();
    void on_editStartList_clicked();
    void on_editTeamsList_clicked();
    void on_editCategories_clicked();
    void on_editTimings_clicked();

    void on_selectorEncoding_activated(const QString &arg1);
    void on_makeRankingsText_clicked();
    void on_makeRankingsCSV_clicked();
    void on_makeRankingsPDF_clicked();

public slots:
    void show();
    void resizeDialogs(QScreen *screen);

    void setCounterTeams(int count);
    void setCounterCompetitors(int count);
    void setCounterCategories(int count);
    void setCounterTimings(int count);

private:
    Ui::LBChronoRace *ui;

    QString raceDataFileName;
    QString startListFileName;
    QString timingsFileName;
    QString categoriesFileName;
    QString teamsFileName;

    ChronoRaceData raceInfo;

    ChronoRaceTable startListTable;
    ChronoRaceTable teamsTable;
    ChronoRaceTable categoriesTable;
    ChronoRaceTable timingsTable;

    qreal ratioX, ratioY;
    qreal areaWidth, areaHeight;

    qreal toHdots(qreal mm);
    qreal toVdots(qreal mm);
    void fitRectToLogo(QRectF &rect, QPixmap const &pixmap);

    void makeRankings(CRLoader::Format format);
    void makeTextRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking, uint bWidth, uint sWidth);
    void makeTextRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking, uint bWidth, uint sWidth);
    void makeCSVRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makeCSVRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking, const QString &shortDescription);
    void makePDFRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void makePDFRankingPortrait(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingPortrait(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void makePDFRankingLandscape(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingLandscape(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);

private slots:
    void importStartList();
    void importCategoriesList();
    void importTimingsList();

    void exportStartList();
    void exportTeamList();
    void exportCategoriesList();
    void exportTimingsList();
};

#endif // LBCHRONORACE_H
