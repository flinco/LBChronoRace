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

#include "chronoracetable.h"
#include "chronoracedata.h"
#include "crloader.h"
#include "competitor.h"
#include "classentry.h"
#include "teamclassentry.h"

#ifndef LBCHRONORACE_NAME
#error "LBCHRONORACE_NAME not set"
#endif
#ifndef LBCHRONORACE_VERSION
#error "LBCHRONORACE_VERSION not set"
#endif

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
    void actionLoadRace();
    void actionSaveRace();
    void actionSaveRaceAs();
    void actionQuit();

    void actionEditRace();
    void actionEditStartList();
    void actionEditTeams();
    void actionEditCategories();
    void actionEditTimings();

    void actionAbout();
    void actionAboutQt();

    void loadRace();
    void saveRace();
    void editRace();
    void editStartList();
    void editTeamsList();
    void editCategories();
    void editTimings();

    void selectorEncoding(const QString &arg1);
    void selectorFormat(const QString &arg1);
    void createStartList();
    void createRankings();

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
    void makePDFRankingSingle(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingSingle(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void makePDFRankingMulti(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingMulti(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void drawPDFTemplatePortrait(QPainter &painter, const QString &fullDescription, int page, int pages);
    //void drawPDFTemplateLandscape(QPainter &painter, const QString &fullDescription, int page, int pages);
    bool initPDFPainter(QPainter &painter, const QString &outFileName);

    void makeStartList(CRLoader::Format format);
    void makeTextStartList(const QList<Competitor>& startList, uint bWidth, uint sWidth, uint nWidth, uint tWidth);
    void makeCSVStartList(const QList<Competitor>& startList);
    void makePDFStartList(const QList<Competitor>& startList);

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
