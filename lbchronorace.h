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

private slots:
    void actionLoadRace();
    void actionSaveRace();
    void actionSaveRaceAs();
    void actionQuit() const;

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

    void selectorEncoding(QString const &arg1) const;
    void selectorFormat(QString const &arg1) const;
    void createStartList();
    void createRankings();

public slots:
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

    qreal ratioX;
    qreal ratioY;
    qreal areaWidth;
    qreal areaHeight;

    qreal toHdots(qreal mm) const;
    qreal toVdots(qreal mm) const;
    void fitRectToLogo(QRectF &rect, QPixmap const &pixmap) const;

    void makeRankings(CRLoader::Format format);
    void makeTextRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking, uint bWidth, uint sWidth) const;
    void makeTextRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking, uint bWidth, uint sWidth) const;
    void makeCSVRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking) const;
    void makeCSVRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking, const QString &shortDescription) const;
    void makePDFRanking(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRanking(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void makePDFRankingSingle(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingSingle(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void makePDFRankingMulti(const QString &outFileName, const QString &fullDescription, const QList<ClassEntry*> individualRanking);
    void makePDFRankingMulti(const QString &outFileName, const QString &fullDescription, const QList<TeamClassEntry*> teamRanking);
    void drawPDFTemplatePortrait(QPainter &painter, const QString &fullDescription, int page, int pages) const;
    //NOSONAR void drawPDFTemplateLandscape(QPainter &painter, const QString &fullDescription, int page, int pages);
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
