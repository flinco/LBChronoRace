#include <QString>
#include <QFileDialog>
#include <QMap>
#include <QMultiMap>
#include <QLocale>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>
#include <QPdfWriter>
#include <QPainter>
#include <QPointF>
#include <QLineF>
#include <QRectF>
#include <QScopedPointer>

#include <QDebug>

#include "lbchronorace.h"
#include "crloader.h"
#include "lbcrexception.h"

static constexpr qreal RANKING_TOP_MARGIN    = 10.0;
static constexpr qreal RANKING_LEFT_MARGIN   = 10.0;
static constexpr qreal RANKING_RIGHT_MARGIN  = 10.0;
static constexpr qreal RANKING_BOTTOM_MARGIN = 12.0;

static constexpr int RANKING_PORTRAIT_FIRST_PAGE_LIMIT  = 44;
static constexpr int RANKING_PORTRAIT_SECOND_PAGE_LIMIT = 50;

//NOSONAR static constexpr int RANKING_LANDSCAPE_FIRST_PAGE_LIMIT  = 23;
//NOSONAR static constexpr int RANKING_LANDSCAPE_SECOND_PAGE_LIMIT = 29;

// static members initialization
QDir LBChronoRace::lastSelectedPath(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));

LBChronoRace::LBChronoRace(QWidget *parent, QGuiApplication const *app) :
    QMainWindow(parent),
    raceInfo(parent),
    startListTable(parent),
    teamsTable(parent),
    categoriesTable(parent),
    timingsTable(parent)
{
    startListFileName  = lastSelectedPath.filePath(LBCHRONORACE_STARTLIST_DEFAULT);
    timingsFileName    = lastSelectedPath.filePath(LBCHRONORACE_TIMINGS_DEFAULT);
    categoriesFileName = lastSelectedPath.filePath(LBCHRONORACE_CATEGORIES_DEFAULT);
    teamsFileName      = lastSelectedPath.filePath(LBCHRONORACE_TEAMLIST_DEFAULT);

    ui->setupUi(this);

    startListTable.setWindowTitle(tr("Start List"));
    startListTable.setModel(CRLoader::getStartListModel());
    QObject::connect(&startListTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importStartList);
    QObject::connect(&startListTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportStartList);
    QObject::connect(&startListTable, &ChronoRaceTable::newRowCount, this, &LBChronoRace::setCounterCompetitors);

    teamsTable.disableButtons();
    teamsTable.setWindowTitle(tr("Teams List"));
    teamsTable.setModel(CRLoader::getTeamsListModel());
    QObject::connect(&teamsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportTeamList);
    QObject::connect(&teamsTable, &ChronoRaceTable::newRowCount, this, &LBChronoRace::setCounterTeams);

    auto const *startListModel = (StartListModel const *) CRLoader::getStartListModel();
    auto const *teamsListModel = (TeamsListModel const *) CRLoader::getTeamsListModel();
    QObject::connect(startListModel, &StartListModel::newTeam, teamsListModel, &TeamsListModel::addTeam);
    QObject::connect(startListModel, &StartListModel::error, this, &LBChronoRace::appendErrorMessage);

    auto *categoriesModel = (CategoriesModel *) CRLoader::getCategoriesModel();
    categoriesTable.setWindowTitle(tr("Categories"));
    categoriesTable.setModel(categoriesModel);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importCategoriesList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportCategoriesList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::newRowCount, this, &LBChronoRace::setCounterCategories);
    QObject::connect(categoriesModel, &CategoriesModel::error, this, &LBChronoRace::appendErrorMessage);

    auto *timingsModel = (TimingsModel *) CRLoader::getTimingsModel();
    timingsTable.setWindowTitle(tr("Timings List"));
    timingsTable.setModel(timingsModel);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importTimingsList);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportTimingsList);
    QObject::connect(&timingsTable, &ChronoRaceTable::newRowCount, this, &LBChronoRace::setCounterTimings);
    QObject::connect(timingsModel, &TimingsModel::error, this, &LBChronoRace::appendErrorMessage);

    // react to screen change and resize
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, this, &LBChronoRace::resizeDialogs);
    resizeDialogs(QGuiApplication::primaryScreen());

    //NOSONAR ui->makeRankingsPDF->setEnabled(false);

    QObject::connect(ui->loadRace, &QPushButton::clicked, this, &LBChronoRace::loadRace);
    QObject::connect(ui->saveRace, &QPushButton::clicked, this, &LBChronoRace::saveRace);
    QObject::connect(ui->editRace, &QPushButton::clicked, this, &LBChronoRace::editRace);
    QObject::connect(ui->editStartList, &QPushButton::clicked, this, &LBChronoRace::editStartList);
    QObject::connect(ui->editTeamsList, &QPushButton::clicked, this, &LBChronoRace::editTeamsList);
    QObject::connect(ui->editCategories, &QPushButton::clicked, this, &LBChronoRace::editCategories);
    QObject::connect(ui->editTimings, &QPushButton::clicked, this, &LBChronoRace::editTimings);
    QObject::connect(ui->makeStartList, &QPushButton::clicked, this, &LBChronoRace::createStartList);
    QObject::connect(ui->makeRankings, &QPushButton::clicked, this, &LBChronoRace::createRankings);

    QObject::connect(ui->actionLoadRace, &QAction::triggered, this, &LBChronoRace::actionLoadRace);
    QObject::connect(ui->actionSaveRace, &QAction::triggered, this, &LBChronoRace::actionSaveRace);
    QObject::connect(ui->actionSaveRaceAs, &QAction::triggered, this, &LBChronoRace::actionSaveRaceAs);
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &LBChronoRace::actionQuit);
    QObject::connect(ui->actionEditRace, &QAction::triggered, this, &LBChronoRace::actionEditRace);
    QObject::connect(ui->actionEditStartList, &QAction::triggered, this, &LBChronoRace::actionEditStartList);
    QObject::connect(ui->actionEditTeams, &QAction::triggered, this, &LBChronoRace::actionEditTeams);
    QObject::connect(ui->actionEditCategories, &QAction::triggered, this, &LBChronoRace::actionEditCategories);
    QObject::connect(ui->actionEditTimings, &QAction::triggered, this, &LBChronoRace::actionEditTimings);
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &LBChronoRace::actionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, this, &LBChronoRace::actionAboutQt);

    QObject::connect(ui->selectorEncoding, &QComboBox::currentTextChanged, this, &LBChronoRace::selectorEncoding);
    QObject::connect(ui->selectorFormat, &QComboBox::currentTextChanged, this, &LBChronoRace::selectorFormat);
}

qreal LBChronoRace::toHdots(qreal mm) const
{
    qreal dots = mm * this->ratioX;
    return (mm >= 0.0) ? dots : this->areaWidth + dots;
}

qreal LBChronoRace::toVdots(qreal mm) const
{
    qreal dots = mm * this->ratioY;
    return (mm >= 0.0) ? dots : this->areaHeight + dots;
}

void LBChronoRace::fitRectToLogo(QRectF &rect, QPixmap const &pixmap) const
{
    qreal shift;
    QSizeF pixmapSize(pixmap.size());
    pixmapSize.scale(rect.width(), rect.height(), Qt::KeepAspectRatio);
    if (rect.width() > pixmapSize.width()) {
        shift = rect.x() + ((rect.width() - pixmapSize.width()) / 2.0);
        rect.setX(shift);
        rect.setWidth(pixmapSize.width());
    } else {
        shift = rect.y() + ((rect.height() - pixmapSize.height()) / 2.0);
        rect.setY(shift);
        rect.setHeight(pixmapSize.height());
    }
}

void LBChronoRace::makeRankings(CRLoader::Format format)
{
    ui->errorDisplay->clear();

    try {
        // Compute the classifications
        QStringList messages;
        QMultiMap<uint, Competitor> startList = CRLoader::getCompetitors(messages);
        QVector<Timing> timings = CRLoader::getTimings();
        QVector<Category> rankings = CRLoader::getCategories();

        uint bib;
        uint leg;
        uint i;
        uint bWidth;
        uint sWidth;
        uint rWidth;
        uint k;

        switch (format) {
            case CRLoader::Format::TEXT:
            case CRLoader::Format::CSV:
            case CRLoader::Format::PDF:
                break;
            default:
                throw(ChronoRaceException(tr("Error: unknown rankings format %1").arg(static_cast<int>(format))));
                break;
        }

        QString rankingsBasePath = QFileDialog::getExistingDirectory(this,
            tr("Select Results Destination Folder"), lastSelectedPath.absolutePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!rankingsBasePath.isEmpty()) {
            lastSelectedPath.setPath(rankingsBasePath);

            i = static_cast<uint>(startList.size());
            sWidth = 1;
            while (i /= 10)
                sWidth++;

            bib = CRLoader::getStartListBibMax();
            bWidth = 1;
            while (bib /= 10)
                bWidth++;

            i = static_cast<uint>(rankings.size());
            rWidth = 1;
            while (i /= 10)
                rWidth++;

            // compute individual general classifications (all included, sorted by bib)
            QMap<uint, ClassEntry> rankingByBib = {};
            for (auto timing : timings) {

                bib = timing.getBib();
                leg = timing.getLeg();
                QPair<QMultiMap<uint, Competitor>::iterator, QMultiMap<uint, Competitor>::iterator> compItPair;
                compItPair = startList.equal_range(bib);

                if (compItPair.first == compItPair.second) {
                    // should never happen
                    appendErrorMessage(tr("Competitor not found for bib %1").arg(bib));
                    continue;
                }

                QMap<uint, ClassEntry>::iterator classEntryIt = rankingByBib.find(bib);
                if (leg == 0) { // perform leg auto detection only if no manual leg hint is present
                    leg = (classEntryIt != rankingByBib.end()) ? (classEntryIt->countEntries() + 1) : 1;
                    CRLoader::setStartListLegs(leg);
                }

                Competitor *comp = Q_NULLPTR;
                for (auto compIt = compItPair.first; compIt != compItPair.second; compIt++) {
                    if (compIt->getLeg() == leg) {
                        comp = &(*compIt);
                        break;
                    }
                }
                if (!comp) {
                    appendErrorMessage(tr("Bib %1 not inserted in results; check for possible duplicated entries").arg(bib));
                    continue;
                } else {
                    // Set the category for the competitor (if any)
                    for (auto const &ranking : rankings) {
                        if (ranking.isTeam())
                            continue;

                        if (comp->getSex() != ranking.getSex())
                            continue;

                        if (comp->getYear() < ranking.getFromYear())
                            continue;

                        if (comp->getYear() > ranking.getToYear())
                            continue;

                        comp->setCategory(ranking.getFullDescription());
                        break;
                    }
                }

                if (classEntryIt != rankingByBib.end()) {
                    classEntryIt->setTime(comp, timing, messages);
                } else {
                    rankingByBib.insert(bib, ClassEntry(bib))->setTime(comp, timing, messages);
                }
            }

            if (startList.size() != timings.size())
                appendErrorMessage(tr("Warning: the number of timings (%1) is not match the expected (%2); check for possible missing or duplicated entries").arg(timings.size()).arg(startList.size()));

            // sort by time
            QList<ClassEntry *> rankingByTime;
            QList<ClassEntry *>::const_iterator c;
            for (auto classEntry = rankingByBib.begin(); classEntry != rankingByBib.end(); classEntry++) {
                c = rankingByTime.constBegin();
                while ((c != rankingByTime.constEnd()) && (*(*c) < classEntry.value())) ++c;
                rankingByTime.insert(c, &(*classEntry));
            }

            // now fill each ranking
            k = 0;
            for (auto const &ranking : rankings) {
                k++;

                if (ranking.isTeam()) {
                    // Team ranking

                    QMap<QString, TeamClassEntry> teamRankingByTeam;
                    for (auto classEntry : rankingByTime) {

                        // exclude DNS and DNF
                        if (classEntry->isDns() || classEntry->isDnf()) {
                            continue;
                        }

                        // exclude competitors without team
                        if (classEntry->getTeam().isEmpty()) {
                            continue;
                        }

                        // Sex
                        if ((ranking.getSex() != Competitor::Sex::UNDEFINED) &&
                            (ranking.getSex() != classEntry->getSex())) {
                            continue;
                        }

                        // To Year
                        if (ranking.getToYear() &&
                            (ranking.getToYear() < classEntry->getToYear())) {
                            continue;
                        }

                        // From Year
                        if (ranking.getFromYear() &&
                            (ranking.getFromYear() > classEntry->getFromYear())) {
                            continue;
                        }

                        QString const &team = classEntry->getTeam();

                        QMap<QString, TeamClassEntry>::iterator const teamRankingIt = teamRankingByTeam.find(team);
                        if (teamRankingIt == teamRankingByTeam.end()) {
                            teamRankingByTeam.insert(team, TeamClassEntry())->setClassEntry(classEntry);
                        } else {
                            teamRankingIt->setClassEntry(classEntry);
                        }
                    }

                    // sort the team rankings
                    QList<TeamClassEntry *> sortedTeamRanking;
                    QList<TeamClassEntry *>::const_iterator t;
                    for (auto teamClassEntry = teamRankingByTeam.begin(); teamClassEntry != teamRankingByTeam.end(); teamClassEntry++) {
                        t = sortedTeamRanking.constBegin();
                        while ((t != sortedTeamRanking.constEnd()) && (*(*t) < teamClassEntry.value())) ++t;
                        sortedTeamRanking.insert(t, &(*teamClassEntry));
                    }

                    // print the ranking
                    QString outFileName;
                    switch (format) {
                        case CRLoader::Format::TEXT:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.txt").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makeTextRanking(outFileName, ranking.getFullDescription(), sortedTeamRanking, bWidth, sWidth);
                            break;
                        case CRLoader::Format::CSV:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.csv").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makeCSVRanking(outFileName, ranking.getFullDescription(), sortedTeamRanking, ranking.getShortDescription());
                            break;
                        case CRLoader::Format::PDF:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.pdf").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makePDFRanking(outFileName, ranking.getFullDescription(), sortedTeamRanking);
                            break;
                        default:
                            Q_ASSERT_X(false, __FUNCTION__, "unknown format");
                            break;
                    }
                } else {
                    // Individual ranking
                    QList<ClassEntry *> individualRanking;
                    for (auto classEntry : rankingByTime) {

                        // Sex
                        if ((ranking.getSex() != Competitor::Sex::UNDEFINED) &&
                            (ranking.getSex() != classEntry->getSex())) {
                            continue;
                        }

                        // To Year
                        if (ranking.getToYear() &&
                            (ranking.getToYear() < classEntry->getToYear())) {
                            continue;
                        }

                        // From Year
                        if (ranking.getFromYear() &&
                            (ranking.getFromYear() > classEntry->getFromYear())) {
                            continue;
                        }

                        classEntry->setCategory(ranking.getFullDescription());

                        individualRanking.push_back(classEntry);
                    }

                    // do the sorting of the single leg times
                    for (uint legIdx = 0u; legIdx < CRLoader::getStartListLegs(); legIdx++) {
                        QMultiMap<uint, ClassEntry *> sortedLegClassification;
                        for (auto classEntry : individualRanking) {
                            sortedLegClassification.insert(classEntry->getTimeValue(legIdx), classEntry);
                        }
                        i = 0;
                        for (auto classEntry : sortedLegClassification) {
                            i++;
                            classEntry->setLegRanking(legIdx, i);
                        }
                    }

                    // print the ranking
                    QString outFileName;
                    switch (format) {
                        case CRLoader::Format::TEXT:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.txt").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makeTextRanking(outFileName, ranking.getFullDescription(), individualRanking, bWidth, sWidth);
                            break;
                        case CRLoader::Format::CSV:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.csv").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makeCSVRanking(outFileName, ranking.getFullDescription(), individualRanking);
                            break;
                        case CRLoader::Format::PDF:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.pdf").arg(k, rWidth, 10, QChar('0')).arg(ranking.getShortDescription()));
                            makePDFRanking(outFileName, ranking.getFullDescription(), individualRanking);
                            break;
                        default:
                            Q_ASSERT_X(false, __FUNCTION__, "unknown format");
                            break;
                    }
                }
            }

            for (auto const &message : messages)
                appendErrorMessage(tr("Warning: %1").arg(message));
            messages.clear();
        }
    } catch (ChronoRaceException& e) {
        appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
    }
}

void LBChronoRace::makeTextRanking(QString const &outFileName, QString const &fullDescription, QList<ClassEntry *> const individualRanking, uint bWidth, uint sWidth) const
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFileName)));
    }
    QTextStream outStream(&outFile);
    QFileInfo outFileInfo(outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    int i = 0;
    outStream << raceInfo << Qt::endl; // add header
    outStream << fullDescription << Qt::endl;
    for (auto c : individualRanking) {
        i++;
        outStream.setFieldWidth(sWidth);
        outStream.setFieldAlignment(QTextStream::AlignRight);
        outStream << i;
        outStream.setFieldWidth(0);
        outStream << " - ";
        outStream.setFieldWidth(bWidth);
        outStream.setFieldAlignment(QTextStream::AlignRight);
        outStream << c->getBib();
        outStream.setFieldWidth(0);
        outStream << " - ";
        outStream << c->getNamesTxt();
        outStream << " - ";
        if (CRLoader::getStartListLegs() > 1)
            outStream << c->getTimesTxt(sWidth) << " - ";
        outStream << c->getTotalTimeTxt() << Qt::endl;
    }
    outStream << Qt::endl;

    appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, outFileInfo.absoluteFilePath()));

    outStream.flush();
    outFile.close();
}

void LBChronoRace::makeTextRanking(QString const &outFileName, QString const &fullDescription, QList<TeamClassEntry *> const teamRanking, uint bWidth, uint sWidth) const
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(QString("Error: cannot open %1").arg(outFileName)));
    }
    QTextStream outStream(&outFile);
    QFileInfo outFileInfo(outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    int i = 0;
    outStream << raceInfo << Qt::endl; // add header
    outStream << fullDescription << Qt::endl;
    for (auto ranking : teamRanking) {
        for (int j = 0; j < ranking->getClassEntryCount(); j++) {
            if (j == 0) {
                i++;
                outStream.setFieldWidth(sWidth);
                outStream.setFieldAlignment(QTextStream::AlignRight);
                outStream << i;
                outStream.setFieldWidth(0);
                outStream << " - ";
            } else {
                outStream.setFieldWidth(sWidth + 3);
                outStream << "";
            }
            outStream.setFieldWidth(bWidth);
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << ranking->getClassEntry(j)->getBib();
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream << ranking->getClassEntry(j)->getNamesTxt();
            outStream << " - ";
            if (CRLoader::getStartListLegs() > 1)
                outStream << ranking->getClassEntry(j)->getTimesTxt(sWidth) << " - ";
            outStream << ranking->getClassEntry(j)->getTotalTimeTxt() << Qt::endl;
        }
        outStream << Qt::endl;
    }
    outStream << Qt::endl;

    appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, outFileInfo.absoluteFilePath()));

    outStream.flush();
    outFile.close();
}

void LBChronoRace::makeCSVRanking(QString const &outFileName, QString const &fullDescription, QList<ClassEntry *> const individualRanking) const
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFileName)));
    }
    QTextStream outStream(&outFile);
    QFileInfo outFileInfo(outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    int i = 0;
    for (auto c : individualRanking) {
        i++;
        outStream << i << ",";
        outStream << c->getBib() << ",";
        outStream << c->getNamesCSV() << ",";
        if (CRLoader::getStartListLegs() > 1)
            outStream << c->getTimesCSV() << ",";
        outStream << c->getTotalTimeTxt() << Qt::endl;
    }
    outStream << Qt::endl;

    appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, outFileInfo.absoluteFilePath()));

    outStream.flush();
    outFile.close();
}

void LBChronoRace::makeCSVRanking(QString const &outFileName, QString const &fullDescription, QList<TeamClassEntry *> const teamRanking, QString const &shortDescription) const
{
    QFile outFile(outFileName);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw(ChronoRaceException(QString("Error: cannot open %1").arg(outFileName)));
    }
    QTextStream outStream(&outFile);
    QFileInfo outFileInfo(outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    int i = 0;
    outStream << shortDescription << Qt::endl;
    for (auto ranking : teamRanking) {
        i++;
        for (int j = 0; j < ranking->getClassEntryCount(); j++) {
            outStream << i << ",";
            outStream << ranking->getClassEntry(j)->getBib() << ",";
            outStream << ranking->getClassEntry(j)->getNamesCSV() << ",";
            if (CRLoader::getStartListLegs() > 1)
                outStream << ranking->getClassEntry(j)->getTimesCSV() << ",";
            outStream << ranking->getClassEntry(j)->getTotalTimeCSV() << Qt::endl;
        }
    }
    outStream << Qt::endl;

    appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, outFileInfo.absoluteFilePath()));

    outStream.flush();
    outFile.close();
}

void LBChronoRace::makePDFRanking(QString const &outFileName, QString const &fullDescription, QList<ClassEntry *> const individualRanking)
{
    switch (CRLoader::getStartListLegs()) {
        case 0:
            appendErrorMessage(tr("Error: cannot generate results for 0 legs"));
            break;
        case 1:
            makePDFRankingSingle(outFileName, fullDescription, individualRanking);
            break;
        default:
            makePDFRankingMulti(outFileName, fullDescription, individualRanking);
            break;
    }
}

void LBChronoRace::makePDFRanking(QString const &outFileName, QString const &fullDescription, QList<TeamClassEntry *> const teamRanking)
{
    switch (CRLoader::getStartListLegs()) {
        case 0:
            appendErrorMessage(tr("Error: cannot generate results for 0 legs"));
            break;
        case 1:
            makePDFRankingSingle(outFileName, fullDescription, teamRanking);
            break;
        default:
            makePDFRankingMulti(outFileName, fullDescription, teamRanking);
            break;
    }
}

void LBChronoRace::makePDFRankingSingle(QString const &outFileName, QString const &fullDescription, QList<ClassEntry *> const individualRanking)
{
    QPainter painter;

    if (initPDFPainter(painter, outFileName)) {
        int i;
        int page;
        int pages = 1;
        auto rankingCount = static_cast<int>(individualRanking.size());
        uint referenceTime;
        QRectF writeRect;

        QScopedPointer<QPdfWriter> rankingPdfwriter((QPdfWriter *) painter.device());
        rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Results") + " - " + fullDescription);

        // Fonts
        QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
        QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));

        // Compute the number of pages
        int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        while (rankingCount--) {
            if (availableEntriesOnPage <= 0) {
                // go to a new page
                pages++;
                availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
            }
            availableEntriesOnPage--;
        }

        QList<ClassEntry *>::const_iterator c = individualRanking.constBegin();
        availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        referenceTime = (individualRanking.empty()) ? 0:  (*c)->getTotalTime();
        for (page = 1, i = 1; page <= pages; page++) {
            drawPDFTemplatePortrait(painter, tr("%1 Results").arg(fullDescription), page, pages);

            // Prepare fonts
            rnkFont.setPointSize(7);
            rnkFontBold.setPointSize(7);

            // Rankings header
            painter.setFont(rnkFontBold);
            writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
            writeRect.setHeight(toVdots(8.0));
            // Ranking placement
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Bib
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(7.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
            // Name
            writeRect.translate(toHdots(8.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
            // Team
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(45.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
            // Year
            writeRect.translate(toHdots(45.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
            // Category
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(28.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
            // Time
            writeRect.translate(toHdots(28.0), 0.0);
            writeRect.setWidth(toHdots(12.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));
            // Time diffeence
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Diff"));

            writeRect.setTop(toVdots((page == 1) ? 57.0 : 34.0));
            writeRect.setHeight(toVdots(4.0));
            for ( ; c < individualRanking.constEnd(); c++) {
                if (availableEntriesOnPage <= 0) {
                    // add a new page
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                    rankingPdfwriter->newPage();
                    break;
                }

                // Move down
                writeRect.translate(0.0, toVdots(4.0));
                writeRect.setLeft(toHdots(0.0));
                // Set the background
                if ((i % 2) == 0) {
                    writeRect.setWidth(this->areaWidth);
                    painter.fillRect(writeRect, Qt::lightGray);
                }
                // Ranking placement
                painter.setFont(rnkFontBold);
                writeRect.setLeft(toHdots(1.0));
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(i));
                i++;
                // Bib
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(7.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number((*c)->getBib()));
                // Name
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(8.0), 0.0);
                writeRect.setWidth(toHdots(60.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getName(0));
                // Team
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(60.0), 0.0);
                writeRect.setWidth(toHdots(45.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getTeam());
                // Year
                writeRect.translate(toHdots(45.0), 0.0);
                writeRect.setWidth(toHdots(9.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number((*c)->getYear(0)));
                // Sex
                writeRect.translate(toHdots(9.0), 0.0);
                writeRect.setWidth(toHdots(6.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, Competitor::toSexString((*c)->getSex()));
                // Category
                writeRect.translate(toHdots(6.0), 0.0);
                writeRect.setWidth(toHdots(28.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getCategory(0));
                // Time
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(28.0), 0.0);
                writeRect.setWidth(toHdots(12.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (*c)->getTotalTimeTxt());
                // Time diffeence
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(12.0), 0.0);
                writeRect.setWidth(toHdots(15.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (*c)->getDiffTimeTxt(referenceTime));

                availableEntriesOnPage--;
            }
        }
        if (!painter.end())
            appendErrorMessage(tr("Error: cannot write to %1").arg(outFileName));
        else
            appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, QFileInfo(outFileName).absoluteFilePath()));
    } else {
        appendErrorMessage(tr("Error: cannot open %1").arg(outFileName));
    }
}

void LBChronoRace::makePDFRankingSingle(QString const &outFileName, QString const &fullDescription, QList<TeamClassEntry *> const teamRanking)
{
    QPainter painter;

    if (initPDFPainter(painter, outFileName)) {
        int i;
        int j;
        int page;
        int pages = 1;
        QRectF writeRect;
        const ClassEntry *c;

        QScopedPointer<QPdfWriter> rankingPdfwriter((QPdfWriter *) painter.device());
        rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Results") + " - " + fullDescription);

        // Fonts
        QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
        QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));

        // Compute the number of pages
        QList<TeamClassEntry *>::const_iterator t;
        int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        for (t = teamRanking.constBegin(); t < teamRanking.constEnd(); t++) {
            for (j = 0; j < (*t)->getClassEntryCount(); j++) {
                if (availableEntriesOnPage <= 0) {
                    pages++;
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                }
                availableEntriesOnPage--;
            }
            // Leave a blank line before next team (but not at page bottom)
            availableEntriesOnPage--;
        }

        t = teamRanking.constBegin();
        availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        for (page = 1, i = 1, j = 0; page <= pages; page++) {
            drawPDFTemplatePortrait(painter, tr("%1 Results").arg(fullDescription), page, pages);

            // Prepare fonts
            rnkFont.setPointSize(7);
            rnkFontBold.setPointSize(7);

            // Rankings header
            painter.setFont(rnkFontBold);
            writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
            writeRect.setHeight(toVdots(8.0));
            // Ranking placement
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Team placement
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Bib
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(7.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
            // Name
            writeRect.translate(toHdots(8.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
            // Team
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(45.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
            // Year
            writeRect.translate(toHdots(45.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
            // Category
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(23.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
            // Time
            writeRect.translate(toHdots(23.0), 0.0);
            writeRect.setWidth(toHdots(27.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));

            writeRect.setTop(toVdots((page == 1) ? 57.0 : 34.0));
            writeRect.setHeight(toVdots(4.0));
            while (t < teamRanking.constEnd()) {
                if (availableEntriesOnPage <= 0) {
                    // add a new page
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                    rankingPdfwriter->newPage();
                    break;
                }

                c = (*t)->getClassEntry(j);
                j++;

                // Move down
                writeRect.translate(0.0, toVdots(4.0));
                writeRect.setLeft(toHdots(0.0));
                // Set the background
                if ((j % 2) == 0) {
                    writeRect.setWidth(this->areaWidth);
                    painter.fillRect(writeRect, Qt::lightGray);
                }
                // Ranking placement
                painter.setFont(rnkFontBold);
                writeRect.setLeft(toHdots(1.0));
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (j == 1) ? QString("%1.").arg(i) : "");
                i++;
                // Team placement
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(j));
                // Bib
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(7.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(c->getBib()));
                // Name
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(8.0), 0.0);
                writeRect.setWidth(toHdots(60.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getName(0));
                // Team
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(60.0), 0.0);
                writeRect.setWidth(toHdots(45.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getTeam());
                // Year
                writeRect.translate(toHdots(45.0), 0.0);
                writeRect.setWidth(toHdots(9.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number(c->getYear(0)));
                // Sex
                writeRect.translate(toHdots(9.0), 0.0);
                writeRect.setWidth(toHdots(6.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, Competitor::toSexString(c->getSex()));
                // Category
                writeRect.translate(toHdots(6.0), 0.0);
                writeRect.setWidth(toHdots(23.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getCategory(0));
                // Time
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(23.0), 0.0);
                writeRect.setWidth(toHdots(27.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, c->getTotalTimeTxt());

                availableEntriesOnPage--;
                if (j >= (*t)->getClassEntryCount()) {
                    j = 0;

                    if (++t == teamRanking.constEnd())
                        break;

                    // Skip a line before next team
                    writeRect.translate(0.0, toVdots(4.0));
                    availableEntriesOnPage--;
                }
            }
        }
        if (!painter.end())
            appendErrorMessage(tr("Error: cannot write to %1").arg(outFileName));
        else
            appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, QFileInfo(outFileName).absoluteFilePath()));
    } else {
        appendErrorMessage(tr("Error: cannot open %1").arg(outFileName));
    }
}

void LBChronoRace::makePDFRankingMulti(QString const &outFileName, QString const &fullDescription, QList<ClassEntry *> const individualRanking)
{
    QPainter painter;

    if (initPDFPainter(painter, outFileName)) {
        int i;
        int page;
        int pages = 1;
        uint referenceTime;
        QRectF writeRect;

        QScopedPointer<QPdfWriter> rankingPdfwriter((QPdfWriter *) painter.device());
        rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Results") + " - " + fullDescription);

        // Fonts
        QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
        QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));

        // Compute the number of pages
        QList<ClassEntry *>::const_iterator c;
        int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        int entriesPerBlock = (int) CRLoader::getStartListLegs() + 1;
        for (c = individualRanking.constBegin(); c < individualRanking.constEnd(); c++) {
            if (entriesPerBlock > availableEntriesOnPage) {
                // go to a new page
                pages++;
                availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
            }
            availableEntriesOnPage -= entriesPerBlock;
        }

        availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        c = individualRanking.constBegin();
        referenceTime = (individualRanking.empty()) ? 0 : (*c)->getTotalTime();
        for (page = 1, i = 1; page <= pages; page++) {
            drawPDFTemplatePortrait(painter, tr("%1 Results").arg(fullDescription), page, pages);

            // Prepare fonts
            rnkFont.setPointSize(7);
            rnkFontBold.setPointSize(7);

            // Rankings header
            painter.setFont(rnkFontBold);
            // First header line
            writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
            writeRect.setHeight(toVdots(4.0));
            // Ranking placement
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Bib
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(7.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
            // Team
            writeRect.translate(toHdots(8.0), 0.0);
            writeRect.setWidth(toHdots(66.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
            // Category
            writeRect.translate(toHdots(66.0), 0.0);
            writeRect.setWidth(toHdots(82.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
            // Time
            writeRect.translate(toHdots(82.0), 0.0);
            writeRect.setWidth(toHdots(12.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));
            // Time difference
            writeRect.translate(toHdots(12.0), 0.0);
            writeRect.setWidth(toHdots(15.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Diff"));
            // Second header line
            writeRect.setTopLeft(QPointF(toHdots(14.0), toVdots((page == 1) ? 52.5 : 29.5)));
            writeRect.setHeight(toVdots(4.0));
            // Leg
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg", "short label"));
            // Name
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
            // Year
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
            // Time (and ranking) in Leg
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(67.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg Time (and position)"));

            writeRect.setTop(toVdots((page == 1) ? 57.0 : 34.0));
            writeRect.setHeight(toVdots(4.0));
            for ( ; c < individualRanking.constEnd(); c++) {
                if (entriesPerBlock > availableEntriesOnPage) {
                    // add a new page
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                    rankingPdfwriter->newPage();
                    break;
                }

                // Move down
                writeRect.translate(0.0, toVdots(4.0));
                writeRect.setLeft(toHdots(0.0));
                // Set the background
                if ((i % 2) == 0) {
                    writeRect.setWidth(this->areaWidth);
                    writeRect.setHeight(toVdots(4.0 * entriesPerBlock));
                    painter.fillRect(writeRect, Qt::lightGray);
                    writeRect.setHeight(toVdots(4.0));
                }

                // Ranking placement
                painter.setFont(rnkFontBold);
                writeRect.setLeft(toHdots(1.0));
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(i));
                i++;
                // Bib
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(7.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number((*c)->getBib()));
                // Team
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(8.0), 0.0);
                writeRect.setWidth(toHdots(66.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getTeam());
                // Category
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(66.0), 0.0);
                writeRect.setWidth(toHdots(82.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getCategory());
                // Time
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(82.0), 0.0);
                writeRect.setWidth(toHdots(12.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (*c)->getTotalTimeTxt());
                // Time diffeence
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(12.0), 0.0);
                writeRect.setWidth(toHdots(15.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (*c)->getDiffTimeTxt(referenceTime));

                // Competitor lines
                for (uint j = 0; j < CRLoader::getStartListLegs(); j++) {
                    // Move down
                    writeRect.translate(0.0, toVdots(4.0));
                    writeRect.setLeft(toHdots(14.0));

                    // Leg
                    painter.setFont(rnkFont);
                    writeRect.setWidth(toHdots(5.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(j + 1));
                    // Name
                    writeRect.translate(toHdots(6.0), 0.0);
                    writeRect.setWidth(toHdots(60.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c)->getName(j));
                    // Year
                    writeRect.translate(toHdots(60.0), 0.0);
                    writeRect.setWidth(toHdots(9.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number((*c)->getYear(j)));
                    // Sex
                    writeRect.translate(toHdots(9.0), 0.0);
                    writeRect.setWidth(toHdots(6.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, Competitor::toSexString((*c)->getSex(j)));
                    // Time in Leg
                    writeRect.translate(toHdots(6.0), 0.0);
                    writeRect.setWidth(toHdots(61.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (*c)->getTime(j));
                    // Ranking in Leg
                    writeRect.translate(toHdots(62.0), 0.0);
                    writeRect.setWidth(toHdots(5.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number((*c)->getLegRanking(j)));
                }

                availableEntriesOnPage -= entriesPerBlock;
            }
        }
        if (!painter.end())
            appendErrorMessage(tr("Error: cannot write to %1").arg(outFileName));
        else
            appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, QFileInfo(outFileName).absoluteFilePath()));
    } else {
        appendErrorMessage(tr("Error: cannot open %1").arg(outFileName));
    }
}

void LBChronoRace::makePDFRankingMulti(QString const &outFileName, QString const &fullDescription, QList<TeamClassEntry *> const teamRanking)
{
    QPainter painter;

    if (initPDFPainter(painter, outFileName)) {
        int i;
        int j;
        int page;
        int pages = 1;
        QRectF writeRect;
        const ClassEntry *c;

        QScopedPointer<QPdfWriter> rankingPdfwriter((QPdfWriter *) painter.device());
        rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Results") + " - " + fullDescription);

        // Fonts
        QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
        QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
        //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));

        // Compute the number of pages
        QList<TeamClassEntry *>::const_iterator t;
        int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        int entriesPerBlock = (int) CRLoader::getStartListLegs() + 1;
        for (t = teamRanking.constBegin(); t < teamRanking.constEnd(); t++) {
            for (j = 0; j < (*t)->getClassEntryCount(); j++) {
                if (entriesPerBlock > availableEntriesOnPage) {
                    // go to a new page
                    pages++;
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                }
                availableEntriesOnPage -= entriesPerBlock;
            }
            // Leave a blank line before next team (but not at page bottom)
            availableEntriesOnPage--;
        }

        t = teamRanking.constBegin();
        availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
        for (page = 1, i = 1, j = 0; page <= pages; page++) {
            drawPDFTemplatePortrait(painter, tr("%1 Results").arg(fullDescription), page, pages);

            // Prepare fonts
            rnkFont.setPointSize(7);
            rnkFontBold.setPointSize(7);

            // Rankings header
            painter.setFont(rnkFontBold);
            // First header line
            writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
            writeRect.setHeight(toVdots(4.0));
            // Ranking placement
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Team placement
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
            // Bib
            writeRect.translate(toHdots(5.0), 0.0);
            writeRect.setWidth(toHdots(7.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
            // Team
            writeRect.translate(toHdots(8.0), 0.0);
            writeRect.setWidth(toHdots(66.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
            // Category
            writeRect.translate(toHdots(66.0), 0.0);
            writeRect.setWidth(toHdots(82.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
            // Time
            writeRect.translate(toHdots(82.0), 0.0);
            writeRect.setWidth(toHdots(22.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Time"));
            // Second header line
            writeRect.setTopLeft(QPointF(toHdots(19.0), toVdots((page == 1) ? 52.5 : 29.5)));
            writeRect.setHeight(toVdots(4.0));
            // Leg
            writeRect.setWidth(toHdots(5.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg", "short label"));
            // Name
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(60.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
            // Year
            writeRect.translate(toHdots(60.0), 0.0);
            writeRect.setWidth(toHdots(9.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
            // Sex
            writeRect.translate(toHdots(9.0), 0.0);
            writeRect.setWidth(toHdots(6.0));
            painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
            // Time (and ranking) in Leg
            writeRect.translate(toHdots(6.0), 0.0);
            writeRect.setWidth(toHdots(67.0));
            painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg Time (and position)"));

            writeRect.setTop(toVdots((page == 1) ? 57.0 : 34.0));
            writeRect.setHeight(toVdots(4.0));
            while (t < teamRanking.constEnd()) {
                if (entriesPerBlock > availableEntriesOnPage) {
                    // add a new page
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                    rankingPdfwriter->newPage();
                    break;
                }

                c = (*t)->getClassEntry(j);
                j++;

                // Move down
                writeRect.translate(0.0, toVdots(4.0));
                writeRect.setLeft(toHdots(0.0));
                // Set the background
                if ((j % 2) == 0) {
                    writeRect.setWidth(this->areaWidth);
                    writeRect.setHeight(toVdots(4.0 * entriesPerBlock));
                    painter.fillRect(writeRect, Qt::lightGray);
                    writeRect.setHeight(toVdots(4.0));
                }

                // Ranking placement
                painter.setFont(rnkFontBold);
                writeRect.setLeft(toHdots(1.0));
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, (j == 1) ? QString("%1.").arg(i) : "");
                i++;
                // Team placement
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(j));
                // Bib
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(7.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(c->getBib()));
                // Team
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(8.0), 0.0);
                writeRect.setWidth(toHdots(66.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getTeam());
                // Category
                painter.setFont(rnkFont);
                writeRect.translate(toHdots(66.0), 0.0);
                writeRect.setWidth(toHdots(82.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getCategory());
                // Time
                painter.setFont(rnkFontBold);
                writeRect.translate(toHdots(82.0), 0.0);
                writeRect.setWidth(toHdots(22.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, c->getTotalTimeTxt());

                // Competitor lines
                for (int k = 0; k < (int) CRLoader::getStartListLegs(); k++) {
                    // Move down
                    writeRect.translate(0.0, toVdots(4.0));
                    writeRect.setLeft(toHdots(19.0));

                    // Leg
                    painter.setFont(rnkFont);
                    writeRect.setWidth(toHdots(5.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1").arg(k + 1));
                    // Name
                    writeRect.translate(toHdots(6.0), 0.0);
                    writeRect.setWidth(toHdots(60.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, c->getName(k));
                    // Year
                    writeRect.translate(toHdots(60.0), 0.0);
                    writeRect.setWidth(toHdots(9.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number(c->getYear(k)));
                    // Sex
                    writeRect.translate(toHdots(9.0), 0.0);
                    writeRect.setWidth(toHdots(6.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, Competitor::toSexString(c->getSex(k)));
                    // Time in Leg
                    writeRect.translate(toHdots(6.0), 0.0);
                    writeRect.setWidth(toHdots(61.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, c->getTime(k));
                    // Ranking in Leg
                    writeRect.translate(toHdots(62.0), 0.0);
                    writeRect.setWidth(toHdots(5.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number(c->getLegRanking(k)));
                }

                availableEntriesOnPage -= entriesPerBlock;
                if (j >= (*t)->getClassEntryCount()) {
                    j = 0;

                    if (++t == teamRanking.constEnd())
                        break;

                    // Skip a line before next team
                    writeRect.translate(0.0, toVdots(4.0));
                    availableEntriesOnPage--;
                }
            }
        }
        if (!painter.end())
            appendErrorMessage(tr("Error: cannot write to %1").arg(outFileName));
        else
            appendInfoMessage(tr("Generated Results '%1': %2").arg(fullDescription, QFileInfo(outFileName).absoluteFilePath()));
    } else {
        appendErrorMessage(tr("Error: cannot open %1").arg(outFileName));
    }
}

void LBChronoRace::drawPDFTemplatePortrait(QPainter &painter, QString const &fullDescription, int page, int pages) const
{
    QRect boundingRect;
    QRectF writeRect;
    qreal rectWidth;

    // Fonts
    QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
    //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
    QFont rnkFontItalic = QFontDatabase::font("Liberation Sans", "Italic", 14);
    //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontItalic.toString()), qUtf8Printable(rnkFontItalic.family()));
    QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
    //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));
    QFont rnkFontBoldItal = QFontDatabase::font("Liberation Sans", "Bold Italic", 22);
    //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBoldItal.toString()), qUtf8Printable(rnkFontBoldItal.family()));

    // Text options for wrapping
    QTextOption textOptions;

    // Sponsors
    QVector<QPixmap> sponsors = raceInfo.getSponsorLogos();

    // Publishing time
    QString editingTimestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");

    // Horizontal lines
    if (page == 1) {
        // Page 1
        painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 39.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 48.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 57.0), this->areaWidth, toVdots(0.2)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots(-35.0), this->areaWidth, toVdots(0.5)), Qt::black);
    } else {
        // Page 2, 3, ...
        painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots( 34.0), this->areaWidth, toVdots(0.2)), Qt::black);
        painter.fillRect(QRectF(toHdots(0.0), toVdots(-35.0), this->areaWidth, toVdots(0.5)), Qt::black);
    }
    // Left and Right logo
    if (page == 1) {
        writeRect.setX(toHdots(0.0));
        writeRect.setY(toVdots(1.0));
        writeRect.setWidth(toHdots(23.5));
        writeRect.setHeight(toVdots(23.5));
        this->fitRectToLogo(writeRect, raceInfo.getLeftLogo());
        painter.drawPixmap(writeRect.toRect(), raceInfo.getLeftLogo());
        writeRect.setX(toHdots(-23.5));
        writeRect.setY(toVdots(1.0));
        writeRect.setWidth(toHdots(23.5));
        writeRect.setHeight(toVdots(23.5));
        this->fitRectToLogo(writeRect, raceInfo.getRightLogo());
        painter.drawPixmap(writeRect.toRect(), raceInfo.getRightLogo());
    }
    // Sponsor logos
    rectWidth = this->areaWidth / static_cast<double>(sponsors.size());
    for (int l = 0; l < static_cast<int>(sponsors.size()); l++) {
        writeRect.setX((rectWidth * l) + toHdots(1.0));
        writeRect.setY(toVdots(-29.0));
        writeRect.setWidth(rectWidth  - toHdots(2.0));
        writeRect.setHeight(toVdots(29.0));
        this->fitRectToLogo(writeRect, sponsors[l]);
        painter.drawPixmap(writeRect.toRect(), sponsors[l]);
    }
    // Title and subtitle
    if (page == 1) {
        painter.setFont(rnkFontBoldItal);
        writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(0.5)));
        writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(15.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getEvent());
        painter.setFont(rnkFontItalic);
        writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(15.0)));
        writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(25.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getPlace() + " - " + QLocale::system().toString(raceInfo.getDate(), "dddd dd/MM/yyyy"));
    } else {
        rnkFontBoldItal.setPointSize(14);
        painter.setFont(rnkFontBoldItal);
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(0.5)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(15.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getEvent() + " - " + raceInfo.getPlace() + " - " + raceInfo.getDate().toString("dd/MM/yyyy"));
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(15.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(25.0)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription);
    }
    // Results, page and editing timestamp
    painter.setFont(rnkFont);
    writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-34.5)));
    writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-29.0)));
    painter.drawText(writeRect.toRect(), Qt::AlignHCenter | Qt::AlignTop, tr("Results") + ":\u00a0" + raceInfo.getResults());
    writeRect.setBottomRight(QPointF(toHdots(30.0), toVdots(-29.0)));
    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Page %n", "", page) + " " + tr("of %n", "", pages));
    writeRect.setTopLeft(QPointF(toHdots(-30.0), toVdots(-34.5)));
    writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-29.0)));
    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignTop, editingTimestamp);
    if (page == 1) {
        // Organization
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(26.0)));
        writeRect.setBottomRight(QPointF((this->areaWidth * 0.5) - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Organization") + ": ", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        textOptions.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        painter.drawText(writeRect, raceInfo.getOrganization(), textOptions);
        // Type, start time, length and elevation gin
        writeRect.setTopLeft(QPointF(this->areaWidth * 0.5, toVdots(26.0)));
        writeRect.setBottomRight(QPointF((this->areaWidth * 0.75) - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Race Type") + ":\u00a0\n" + tr("Start Time") + ":\u00a0\n" + tr("Length") + ":\u00a0\n" + tr("Elevation Gain") + ":\u00a0", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo.getRaceType() + "\n" + raceInfo.getStartTime().toString("hh:mm") + "\n" + raceInfo.getLength() + "\n" + raceInfo.getElevationGain());
        // Referee and Timekeeper 1, 2 and 3
        writeRect.setTopLeft(QPointF(this->areaWidth * 0.75, toVdots(26.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth - toHdots(5.0), toVdots(38.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Referee") + ":\u00a0\n" + tr("Timekeeper 1") + ":\u00a0\n" + tr("Timekeeper 2") + ":\u00a0\n" + tr("Timekeeper 3") + ":\u00a0", &boundingRect);
        writeRect.setTopLeft(boundingRect.topRight());
        textOptions.setWrapMode(QTextOption::WordWrap);
        painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo.getReferee() + "\n" + raceInfo.getTimeKeeper(0) + "\n" + raceInfo.getTimeKeeper(1) + "\n" + raceInfo.getTimeKeeper(2));
        // Description
        painter.setFont(rnkFontBold);
        writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(39.0)));
        writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(47.5)));
        painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription.toUpper());
    }
}

//NOSONAR void LBChronoRace::drawPDFTemplateLandscape(QPainter &painter, QString const &fullDescription, int page, int pages)
//NOSONAR {
//NOSONAR     QRect boundingRect;
//NOSONAR     QRectF writeRect;
//NOSONAR     qreal rectWidth;

//NOSONAR     // Fonts
//NOSONAR     QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
//NOSONAR     //qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
//NOSONAR     QFont rnkFontItalic = QFontDatabase::font("Liberation Sans", "Italic", 14);
//NOSONAR     //qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontItalic.toString()), qUtf8Printable(rnkFontItalic.family()));
//NOSONAR     QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
//NOSONAR     //qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));
//NOSONAR     QFont rnkFontBoldItal = QFontDatabase::font("Liberation Sans", "Bold Italic", 22);
//NOSONAR     //qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBoldItal.toString()), qUtf8Printable(rnkFontBoldItal.family()));

//NOSONAR     // Text options for wrapping
//NOSONAR     QTextOption textOptions;

//NOSONAR     // Sponsors
//NOSONAR     QVector<QPixmap> sponsors = raceInfo.getSponsorLogos();

//NOSONAR     // Publishing time
//NOSONAR     QString editingTimestamp = QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm");

//NOSONAR     // Horizontal lines
//NOSONAR     if (page == 1) {
//NOSONAR         // Page 1
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 39.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 48.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 57.0), this->areaWidth, toVdots(0.2)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(-35.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR     } else {
//NOSONAR         // Page 2, 3, ...
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(  0.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 25.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots( 34.0), this->areaWidth, toVdots(0.2)), Qt::black);
//NOSONAR         painter.fillRect(QRectF(toHdots(0.0), toVdots(-35.0), this->areaWidth, toVdots(0.5)), Qt::black);
//NOSONAR     }
//NOSONAR     // Left and Right logo
//NOSONAR     if (page == 1) {
//NOSONAR         writeRect.setX(toHdots(0.0));
//NOSONAR         writeRect.setY(toVdots(1.0));
//NOSONAR         writeRect.setWidth(toHdots(23.5));
//NOSONAR         writeRect.setHeight(toVdots(23.5));
//NOSONAR         this->fitRectToLogo(writeRect, raceInfo.getLeftLogo());
//NOSONAR         painter.drawPixmap(writeRect.toRect(), raceInfo.getLeftLogo());
//NOSONAR         writeRect.setX(toHdots(-23.5));
//NOSONAR         writeRect.setY(toVdots(1.0));
//NOSONAR         writeRect.setWidth(toHdots(23.5));
//NOSONAR         writeRect.setHeight(toVdots(23.5));
//NOSONAR         this->fitRectToLogo(writeRect, raceInfo.getRightLogo());
//NOSONAR         painter.drawPixmap(writeRect.toRect(), raceInfo.getRightLogo());
//NOSONAR     }
//NOSONAR     // Sponsor logos
//NOSONAR     rectWidth = this->areaWidth / sponsors.size();
//NOSONAR     for (int l = 0; l < sponsors.size(); l++) {
//NOSONAR         writeRect.setX((rectWidth * l) + toHdots(1.0));
//NOSONAR         writeRect.setY(toVdots(-29.0));
//NOSONAR         writeRect.setWidth(rectWidth  - toHdots(2.0));
//NOSONAR         writeRect.setHeight(toVdots(29.0));
//NOSONAR         this->fitRectToLogo(writeRect, sponsors[l]);
//NOSONAR         painter.drawPixmap(writeRect.toRect(), sponsors[l]);
//NOSONAR     }
//NOSONAR     // Title and subtitle
//NOSONAR     if (page == 1) {
//NOSONAR         painter.setFont(rnkFontBoldItal);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(0.5)));
//NOSONAR         writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(15.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getEvent());
//NOSONAR         painter.setFont(rnkFontItalic);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(24.0), toVdots(15.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(toHdots(-24.0), toVdots(25.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getPlace() + " - " + QLocale::system().toString(raceInfo.getDate(), "dddd dd/MM/yyyy"));
//NOSONAR     } else {
//NOSONAR         rnkFontBoldItal.setPointSize(14);
//NOSONAR         painter.setFont(rnkFontBoldItal);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(0.5)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(15.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, raceInfo.getEvent() + " - " + raceInfo.getPlace() + " - " + raceInfo.getDate().toString("dd/MM/yyyy"));
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(15.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(25.0)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription);
//NOSONAR     }
//NOSONAR     // Results, page and editing timestamp
//NOSONAR     painter.setFont(rnkFont);
//NOSONAR     writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(-34.5)));
//NOSONAR     writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-29.0)));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignHCenter | Qt::AlignTop, tr("Results") + ":\u00a0" + raceInfo.getResults());
//NOSONAR     writeRect.setBottomRight(QPointF(toHdots(30.0), toVdots(-29.0)));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Page %n", "", page) + " " + tr("of %n", "", pages));
//NOSONAR     writeRect.setTopLeft(QPointF(toHdots(-30.0), toVdots(-34.5)));
//NOSONAR     writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(-29.0)));
//NOSONAR     painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignTop, editingTimestamp);
//NOSONAR     if (page == 1) {
//NOSONAR         // Organization
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF((this->areaWidth * 0.5) - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Organization") + ": ", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         textOptions.setAlignment(Qt::AlignLeft | Qt::AlignTop);
//NOSONAR         painter.drawText(writeRect, raceInfo.getOrganization(), textOptions);
//NOSONAR         // Type, start time, length and elevation gin
//NOSONAR         writeRect.setTopLeft(QPointF(this->areaWidth * 0.5, toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF((this->areaWidth * 0.75) - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Race Type") + ":\u00a0\n" + tr("Start Time") + ":\u00a0\n" + tr("Length") + ":\u00a0\n" + tr("Elevation Gain") + ":\u00a0", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo.getRaceType() + "\n" + raceInfo.getStartTime().toString("hh:mm") + "\n" + raceInfo.getLength() + "\n" + raceInfo.getElevationGain());
//NOSONAR         // Referee and Timekeeper 1, 2 and 3
//NOSONAR         writeRect.setTopLeft(QPointF(this->areaWidth * 0.75, toVdots(26.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth - toHdots(5.0), toVdots(38.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, tr("Referee") + ":\u00a0\n" + tr("Timekeeper 1") + ":\u00a0\n" + tr("Timekeeper 2") + ":\u00a0\n" + tr("Timekeeper 3") + ":\u00a0", &boundingRect);
//NOSONAR         writeRect.setTopLeft(boundingRect.topRight());
//NOSONAR         textOptions.setWrapMode(QTextOption::WordWrap);
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignTop, raceInfo.getReferee() + "\n" + raceInfo.getTimeKeeper1() + "\n" + raceInfo.getTimeKeeper2() + "\n" + raceInfo.getTimeKeeper3());
//NOSONAR         // Description
//NOSONAR         painter.setFont(rnkFontBold);
//NOSONAR         writeRect.setTopLeft(QPointF(toHdots(0.0), toVdots(39.0)));
//NOSONAR         writeRect.setBottomRight(QPointF(this->areaWidth, toVdots(47.5)));
//NOSONAR         painter.drawText(writeRect.toRect(), Qt::AlignCenter, fullDescription.toUpper());
//NOSONAR     }
//NOSONAR }

bool LBChronoRace::initPDFPainter(QPainter &painter, QString const &outFileName)
{
    bool retval = false;

    //NOSONAR qDebug("Path: %s", qUtf8Printable(outFileName));
    if (!outFileName.isEmpty()) {
        QPdfWriter *rankingPdfwriter = new QPdfWriter(outFileName);
        rankingPdfwriter->setPdfVersion(QPagedPaintDevice::PdfVersion_A1b);
        //NOSONAR rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Results") + " - " + fullDescription);
        rankingPdfwriter->setPageSize(QPageSize(QPageSize::A4));
        rankingPdfwriter->setPageOrientation(QPageLayout::Portrait);
        rankingPdfwriter->setPageMargins(QMarginsF(RANKING_LEFT_MARGIN, RANKING_TOP_MARGIN, RANKING_RIGHT_MARGIN, RANKING_BOTTOM_MARGIN), QPageLayout::Millimeter);
        rankingPdfwriter->setCreator(LBCHRONORACE_NAME);

        // Set global values to convert from mm to dots
        this->ratioX = rankingPdfwriter->logicalDpiX() / 25.4;
        this->ratioY = rankingPdfwriter->logicalDpiY() / 25.4;
        //NOSONAR this->ratioX = rankingPdfwriter->physicalDpiX() / 25.4;
        //NOSONAR this->ratioY = rankingPdfwriter->physicalDpiY() / 25.4;
        this->areaWidth = rankingPdfwriter->width();
        this->areaHeight = rankingPdfwriter->height();

        retval = painter.begin(rankingPdfwriter);
    } else {
        appendErrorMessage(tr("Error: no file name"));
    }

    return retval;
}

void LBChronoRace::makeStartList(CRLoader::Format format)
{
    ui->errorDisplay->clear();

    try {
        // Compute the start list
        uint bib;
        uint i;
        uint bWidth;
        uint sWidth;
        uint nWidth = 0;
        uint tWidth = 0;
        uint maxBib = 0;

        switch (format) {
            case CRLoader::Format::TEXT:
            case CRLoader::Format::CSV:
            case CRLoader::Format::PDF:
                break;
            default:
                throw(ChronoRaceException(tr("Error: unknown rankings format %1").arg(static_cast<int>(format))));
                break;
        }

        // compute start list
        QList<Competitor> sortedStartList;
        for (auto const &c : CRLoader::getStartList()) {
            sortedStartList.push_back(c);
            if ((bib = c.getBib()) > maxBib)
                maxBib = bib;
            if ((i = static_cast<uint>(c.getName().length())) > nWidth)
                nWidth = i;
            if ((i = static_cast<uint>(c.getTeam().length())) > tWidth)
                tWidth = i;
        }
        CompetitorSorter::setSortingField(Competitor::Field::CMF_BIB);
        CompetitorSorter::setSortingOrder(Qt::AscendingOrder);
        std::stable_sort(sortedStartList.begin(), sortedStartList.end(), CompetitorSorter());

        i = static_cast<uint>(sortedStartList.size());
        sWidth = 1;
        while (i /= 10) sWidth++;

        bWidth = 1;
        while (maxBib /= 10) bWidth++;

        // print the start list
        switch (format) {
            case CRLoader::Format::TEXT:
                makeTextStartList(sortedStartList, bWidth, sWidth, nWidth, tWidth);
                break;
            case CRLoader::Format::CSV:
                makeCSVStartList(sortedStartList);
                break;
            case CRLoader::Format::PDF:
                makePDFStartList(sortedStartList);
                break;
            default:
                Q_ASSERT_X(false, __FUNCTION__, "unknown format");
                break;
        }
    } catch (ChronoRaceException& e) {
        appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
    }
}

void LBChronoRace::makeTextStartList(QList<Competitor> const &startList, uint bWidth, uint sWidth, uint nWidth, uint tWidth)
{
    QString outFileName = QFileDialog::getSaveFileName(this, tr("Select Start List File"),
        lastSelectedPath.absolutePath(), tr("Plain Text (*.txt)"));

    //NOSONAR qDebug("Path: %s", qUtf8Printable(outFileName));
    if (!outFileName.isEmpty()) {

        QTime startTime = raceInfo.getStartTime();

        if (!outFileName.endsWith(".txt", Qt::CaseInsensitive))
            outFileName.append(".txt");

        QFileInfo outFileInfo(outFileName);
        lastSelectedPath = outFileInfo.absoluteDir();

        QFile outFile(outFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw(ChronoRaceException(QString("Error: cannot open %1").arg(outFileName)));
        }
        QTextStream outStream(&outFile);

        if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
            outStream.setEncoding(QStringConverter::Utf8);
        else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
            outStream.setEncoding(QStringConverter::Latin1);

        int offset;
        int i = 0;
        outStream << raceInfo << Qt::endl; // add header
        outStream << tr("Start List") << Qt::endl;
        for (auto const &competitor : startList) {
            i++;
            outStream.setFieldWidth(sWidth);
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << i;
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream.setFieldWidth(bWidth);
            outStream.setFieldAlignment(QTextStream::AlignRight);
            outStream << competitor.getBib();
            outStream.setFieldWidth(0);
            outStream << " - ";
            outStream << competitor.getName(nWidth);
            outStream << " - ";
            outStream << competitor.getTeam(tWidth);
            outStream << " - ";
            outStream << competitor.getYear();
            outStream << " - ";
            outStream << Competitor::toSexString(competitor.getSex());
            outStream << " - ";
            outStream.setFieldWidth(15);
            outStream.setFieldAlignment(QTextStream::AlignRight);
            offset = competitor.getOffset();
            if (offset >= 0) {
                offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                outStream << Competitor::toOffsetString(offset);
            } else {
                outStream << tr("Leg %n", "", qAbs(offset));
            }
            outStream.setFieldWidth(0);
            outStream << Qt::endl;
        }
        outStream << Qt::endl;

        appendInfoMessage(tr("Generated Start List: %1").arg(outFileInfo.absoluteFilePath()));

        outStream.flush();
        outFile.close();
    }
}

void LBChronoRace::makeCSVStartList(QList<Competitor> const &startList)
{
    QString outFileName = QFileDialog::getSaveFileName(this, tr("Select Start List File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    //NOSONAR qDebug("Path: %s", qUtf8Printable(outFileName));
    if (!outFileName.isEmpty()) {

        QTime startTime = raceInfo.getStartTime();

        if (!outFileName.endsWith(".csv", Qt::CaseInsensitive))
            outFileName.append(".csv");

        QFileInfo outFileInfo(outFileName);
        lastSelectedPath = outFileInfo.absoluteDir();

        QFile outFile(outFileName);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw(ChronoRaceException(QString("Error: cannot open %1").arg(outFileName)));
        }
        QTextStream outStream(&outFile);

        if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
            outStream.setEncoding(QStringConverter::Utf8);
        else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
            outStream.setEncoding(QStringConverter::Latin1);

        int offset;
        int i = 0;
        for (auto const &competitor : startList) {
            i++;
            outStream << i << ",";
            outStream << competitor.getBib() << ",";
            outStream << competitor.getName() << ",";
            outStream << competitor.getTeam() << ",";
            outStream << competitor.getYear() << ",";
            outStream << Competitor::toSexString(competitor.getSex()) << ",";
            offset = competitor.getOffset();
            if (offset >= 0) {
                offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                outStream << Competitor::toOffsetString(offset);
            } else {
                outStream << qAbs(offset);
            }
            outStream << Qt::endl;
        }
        outStream << Qt::endl;

        appendInfoMessage(tr("Generated Start List: %1").arg(outFileInfo.absoluteFilePath()));

        outStream.flush();
        outFile.close();
    }
}

void LBChronoRace::makePDFStartList(QList<Competitor> const &startList)
{
    QString outFileName = QFileDialog::getSaveFileName(this, tr("Select Start List File"),
        lastSelectedPath.absolutePath(), tr("PDF (*.pdf)"));

    if (!outFileName.isEmpty()) {
        QPainter painter;

        if (!outFileName.endsWith(".pdf", Qt::CaseInsensitive))
            outFileName.append(".pdf");

        if (initPDFPainter(painter, outFileName)) {
            int i;
            int page;
            int pages = 1;
            int prevOffset;
            int offset;
            uint legs = CRLoader::getStartListLegs();
            QRectF writeRect;
            QTime startTime = raceInfo.getStartTime();

            QFileInfo outFileInfo(outFileName);
            lastSelectedPath = outFileInfo.absoluteDir();

            QScopedPointer<QPdfWriter> rankingPdfwriter((QPdfWriter *) painter.device());
            rankingPdfwriter->setTitle(raceInfo.getEvent() + " - " + tr("Start List"));

            // Fonts
            QFont rnkFont = QFontDatabase::font("Liberation Sans", "Regular", 7);
            //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFont.toString()), qUtf8Printable(rnkFont.family()));
            QFont rnkFontBold = QFontDatabase::font("Liberation Sans", "Bold", 18);
            //NOSONAR qDebug("Default font: %s (%s)", qUtf8Printable(rnkFontBold.toString()), qUtf8Printable(rnkFontBold.family()));

            // Compute the number of pages
            QList<Competitor>::const_iterator c;
            int availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
            for (c = startList.constBegin(), prevOffset = (*c).getOffset(); c < startList.constEnd(); c++) {
                if (availableEntriesOnPage <= 0) {
                    // go to a new page
                    pages++;
                    availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                }
                availableEntriesOnPage--;
                if (legs > 1) {
                    offset = (*c).getOffset();
                    if (offset < 0) {
                        if (prevOffset != offset)
                            availableEntriesOnPage--;
                        prevOffset = offset;
                    }
                }
            }

            c = startList.constBegin();
            prevOffset = (c != startList.constEnd()) ? (*c).getOffset() : 0;
            availableEntriesOnPage = RANKING_PORTRAIT_FIRST_PAGE_LIMIT;
            for (page = 1, i = 1; page <= pages; page++) {
                drawPDFTemplatePortrait(painter, tr("Start List"), page, pages);

                // Prepare fonts
                rnkFont.setPointSize(7);
                rnkFontBold.setPointSize(7);

                // Rankings header
                painter.setFont(rnkFontBold);
                writeRect.setTopLeft(QPointF(toHdots(1.0), toVdots((page == 1) ? 48.5 : 25.5)));
                writeRect.setHeight(toVdots(8.0));
                // Ranking placement
                writeRect.setWidth(toHdots(5.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("#"));
                // Bib
                writeRect.translate(toHdots(5.0), 0.0);
                writeRect.setWidth(toHdots(7.0));
                painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Bib"));
                // Name
                writeRect.translate(toHdots(8.0), 0.0);
                writeRect.setWidth(toHdots(60.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Name"));
                // Team
                writeRect.translate(toHdots(60.0), 0.0);
                writeRect.setWidth(toHdots(45.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Team"));
                // Year
                writeRect.translate(toHdots(45.0), 0.0);
                writeRect.setWidth(toHdots(9.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Year"));
                // Sex
                writeRect.translate(toHdots(9.0), 0.0);
                writeRect.setWidth(toHdots(6.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Sex"));
                // Category
                writeRect.translate(toHdots(6.0), 0.0);
                writeRect.setWidth(toHdots(28.0));
                painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignBottom, tr("Class"));
                // Time
                writeRect.translate(toHdots(28.0), 0.0);
                writeRect.setWidth(toHdots(27.0));
                if (CRLoader::getStartListLegs() == 1)
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Start Time"));
                else
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignBottom, tr("Leg", "long label"));

                writeRect.setTop(toVdots((page == 1) ? 57.0 : 34.0));
                writeRect.setHeight(toVdots(4.0));
                for ( ; c < startList.constEnd(); c++) {
                    if (legs > 1) {
                        offset = (*c).getOffset();
                        if (offset < 0) {
                            if (prevOffset != offset) {
                                // Move down
                                writeRect.translate(0.0, toVdots(4.0));
                                availableEntriesOnPage--;
                            }
                            prevOffset = offset;
                        }
                    }

                    if (availableEntriesOnPage <= 0) {
                        // add a new page
                        availableEntriesOnPage = RANKING_PORTRAIT_SECOND_PAGE_LIMIT;
                        rankingPdfwriter->newPage();
                        break;
                    }

                    // Move down
                    writeRect.translate(0.0, toVdots(4.0));
                    writeRect.setLeft(toHdots(0.0));
                    // Set the background
                    if ((i % 2) == 0) {
                        writeRect.setWidth(this->areaWidth);
                        painter.fillRect(writeRect, Qt::lightGray);
                    }
                    // Ranking placement
                    painter.setFont(rnkFontBold);
                    writeRect.setLeft(toHdots(1.0));
                    writeRect.setWidth(toHdots(5.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString("%1.").arg(i));
                    i++;
                    // Bib
                    painter.setFont(rnkFont);
                    writeRect.translate(toHdots(5.0), 0.0);
                    writeRect.setWidth(toHdots(7.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, QString::number((*c).getBib()));
                    // Name
                    painter.setFont(rnkFontBold);
                    writeRect.translate(toHdots(8.0), 0.0);
                    writeRect.setWidth(toHdots(60.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c).getName(0));
                    // Team
                    painter.setFont(rnkFont);
                    writeRect.translate(toHdots(60.0), 0.0);
                    writeRect.setWidth(toHdots(45.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c).getTeam());
                    // Year
                    writeRect.translate(toHdots(45.0), 0.0);
                    writeRect.setWidth(toHdots(9.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, QString::number((*c).getYear()));
                    // Sex
                    writeRect.translate(toHdots(9.0), 0.0);
                    writeRect.setWidth(toHdots(6.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, Competitor::toSexString((*c).getSex()));
                    // Category
                    writeRect.translate(toHdots(6.0), 0.0);
                    writeRect.setWidth(toHdots(28.0));
                    painter.drawText(writeRect.toRect(), Qt::AlignLeft | Qt::AlignVCenter, (*c).getCategory());
                    // Start Time / Leg
                    offset = (*c).getOffset();
                    painter.setFont(rnkFontBold);
                    writeRect.translate(toHdots(28.0), 0.0);
                    writeRect.setWidth(toHdots(27.0));
                    if (offset >= 0) {
                        offset += (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, Competitor::toOffsetString(offset));
                    } else if (CRLoader::getStartListLegs() == 1) {
                        offset = (3600 * startTime.hour()) + (60 * startTime.minute()) + startTime.second();
                        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, Competitor::toOffsetString(offset));
                    } else {
                        painter.drawText(writeRect.toRect(), Qt::AlignRight | Qt::AlignVCenter, tr("Leg %n", "", qAbs(offset)));
                    }

                    availableEntriesOnPage--;
                }
            }
            if (!painter.end())
                appendErrorMessage(tr("Error: cannot write to %1").arg(outFileName));
            else
                appendInfoMessage(tr("Generated Start List: %1").arg(outFileInfo.absoluteFilePath()));
        } else {
            appendErrorMessage(tr("Error: cannot open %1").arg(outFileName));
        }
    }
}

void LBChronoRace::setCounterTeams(int count) const
{
    ui->counterTeams->display(count);
}

void LBChronoRace::setCounterCompetitors(int count) const
{
    ui->counterCompetitors->display(count);
}

void LBChronoRace::setCounterCategories(int count) const
{
    ui->counterCategories->display(count);
}

void LBChronoRace::setCounterTimings(int count) const
{
    ui->counterTimings->display(count);
}

void LBChronoRace::appendInfoMessage(QString const &message) const
{
    ui->infoDisplay->appendPlainText(message);
}

void LBChronoRace::appendErrorMessage(QString const &message) const
{
    ui->errorDisplay->appendPlainText(message);
}

void LBChronoRace::importStartList()
{
    startListFileName = QFileDialog::getOpenFileName(this, tr("Select Start List"),
       lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!startListFileName.isEmpty()) {
        QPair<int, int> count(0, 0);
        appendInfoMessage(tr("Start List File: %1").arg(startListFileName));
        try {
            count = CRLoader::importStartList(startListFileName);
            appendInfoMessage(tr("Loaded: %n competitor(s)", "", count.first));
            appendInfoMessage(tr("Loaded: %n team(s)", "", count.second));
            lastSelectedPath = QFileInfo(startListFileName).absoluteDir();
        } catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterCompetitors(count.first);
        setCounterTeams(count.second);
    }
}

void LBChronoRace::importCategoriesList()
{
    categoriesFileName = QFileDialog::getOpenFileName(this, tr("Select Categories File"),
         lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!categoriesFileName.isEmpty()) {
        int count = 0;
        appendInfoMessage(tr("Categories File: %1").arg(categoriesFileName));
        try {
            count = CRLoader::importCategories(categoriesFileName);
            appendInfoMessage((count == 1) ? tr("Loaded: %n category", "", count) : tr("Loaded: %n category", "", count));
            lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();
        } catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterCategories(count);
    }
}

void LBChronoRace::importTimingsList()
{
    timingsFileName = QFileDialog::getOpenFileName(this, tr("Select Timings File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!timingsFileName.isEmpty()) {
        int count = 0;
        appendInfoMessage(tr("Timings File: %1").arg(timingsFileName));
        try {
            count = CRLoader::importTimings(timingsFileName);
            appendInfoMessage(tr("Loaded: %n timing(s)", "", count));
            lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();
        } catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterTimings(count);
    }
}

void LBChronoRace::exportStartList()
{
    startListFileName = QFileDialog::getSaveFileName(this, tr("Select Start List"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!startListFileName.isEmpty()) {
        try {
            CRLoader::exportStartList(startListFileName);
            appendInfoMessage(tr("Start List File saved: %1").arg(startListFileName));
            lastSelectedPath = QFileInfo(startListFileName).absoluteDir();
        }  catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTeamList()
{
    teamsFileName = QFileDialog::getSaveFileName(this, tr("Select Teams List"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!teamsFileName.isEmpty()) {
        try {
            CRLoader::exportTeams(teamsFileName);
            appendInfoMessage(tr("Teams File saved: %1").arg(teamsFileName));
            lastSelectedPath = QFileInfo(teamsFileName).absoluteDir();
        }  catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportCategoriesList()
{
    categoriesFileName = QFileDialog::getSaveFileName(this, tr("Select Categories File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!categoriesFileName.isEmpty()) {
        try {
            CRLoader::exportCategories(categoriesFileName);
            appendInfoMessage(tr("Categories File saved: %1").arg(categoriesFileName));
            lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();
        }  catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTimingsList()
{
    timingsFileName = QFileDialog::getSaveFileName(this, tr("Select Timings File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!timingsFileName.isEmpty()) {
        try {
            CRLoader::exportTimings(timingsFileName);
            appendInfoMessage(tr("Timings File saved: %1").arg(timingsFileName));
            lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();
        }  catch (ChronoRaceException& e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::actionLoadRace()
{
    raceDataFileName = QFileDialog::getOpenFileName(this, tr("Select Race Data File"),
        lastSelectedPath.absolutePath(), tr("ChronoRace Data (*.crd)"));
    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);
        lastSelectedPath = QFileInfo(raceDataFileName).absoluteDir();

        if (!raceDataFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        } else {
            quint32 binFmt;
            QFileInfo raceDataFileInfo(raceDataFileName);

            QDataStream in(&raceDataFile);
            in.setVersion(QDataStream::Qt_5_15);
            in >> binFmt;

            switch (binFmt) {
                case LBCHRONORACE_BIN_FMT_v1:
                case LBCHRONORACE_BIN_FMT_v2: {
                    QAbstractTableModel const *table;
                    qint16 encodingIdx;
                    qint16 formatIdx;
                    int tableCount;

                    in >> encodingIdx;
                    ui->selectorEncoding->setCurrentIndex(encodingIdx);
                    selectorEncoding(ui->selectorEncoding->currentText());
                    in >> formatIdx;
                    ui->selectorFormat->setCurrentIndex(formatIdx);
                    selectorFormat(ui->selectorFormat->currentText());
                    raceInfo.setBinFormat(binFmt);
                    in >> raceInfo;
                    CRLoader::loadRaceData(in);

                    // Set useful information
                    lastSelectedPath = raceDataFileInfo.absoluteDir();
                    setWindowTitle(QString(tr(LBCHRONORACE_NAME) + " - " + raceDataFileInfo.fileName()));

                    table = CRLoader::getStartListModel();
                    tableCount = table->rowCount();
                    setCounterCompetitors(tableCount);
                    appendInfoMessage(tr("Loaded: %n competitor(s)", "", tableCount));
                    table = CRLoader::getTeamsListModel();
                    tableCount = table->rowCount();
                    setCounterTeams(tableCount);
                    appendInfoMessage(tr("Loaded: %n team(s)", "", tableCount));
                    table = CRLoader::getCategoriesModel();
                    tableCount = table->rowCount();
                    setCounterCategories(tableCount);
                    appendInfoMessage((tableCount == 1) ? tr("Loaded: %n category", "", tableCount) : tr("Loaded: %n category", "", tableCount));
                    table = CRLoader::getTimingsModel();
                    tableCount = table->rowCount();
                    setCounterTimings(tableCount);
                    appendInfoMessage(tr("Loaded: %n timing(s)", "", tableCount));

                    appendInfoMessage(tr("Race loaded: %1").arg(raceDataFileName));
                }
                break;
                default:
                    QMessageBox::information(this, tr("Race Data File Error"), tr("Format version %1 not supported").arg(binFmt));
                break;
            }
        }
    }
}

void LBChronoRace::actionSaveRace()
{
    if (raceDataFileName.isEmpty()) {
        raceDataFileName = QFileDialog::getSaveFileName(this, tr("Select Race Data File"),
            lastSelectedPath.absolutePath(), tr("ChronoRace Data (*.crd)"));

        if (!raceDataFileName.endsWith(".crd", Qt::CaseInsensitive))
            raceDataFileName.append(".crd");
    }

    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);

        if (!raceDataFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        } else {
            QFileInfo raceDataFileInfo(raceDataFileName);
            QDataStream out(&raceDataFile);

            out.setVersion(QDataStream::Qt_5_15);
            out << quint32(LBCHRONORACE_BIN_FMT)
                << qint16(ui->selectorEncoding->currentIndex())
                << qint16(ui->selectorFormat->currentIndex())
                << raceInfo;
            CRLoader::saveRaceData(out);

            // Set useful information
            lastSelectedPath = raceDataFileInfo.absoluteDir();
            setWindowTitle(QString(tr(LBCHRONORACE_NAME) + " - " + raceDataFileInfo.fileName()));
            appendInfoMessage(tr("Race saved: %1").arg(raceDataFileName));
        }

    }
}

void LBChronoRace::actionSaveRaceAs()
{
    raceDataFileName.clear();
    actionSaveRace();
}

void LBChronoRace::actionQuit() const
{
    QApplication::quit();
}

void LBChronoRace::actionEditRace()
{
    raceInfo.show();
}

void LBChronoRace::actionEditStartList()
{
    startListTable.show();
}

void LBChronoRace::actionEditTeams()
{
    teamsTable.show();
}

void LBChronoRace::actionEditCategories()
{
    categoriesTable.show();
}

void LBChronoRace::actionEditTimings()
{
    timingsTable.show();
}

void LBChronoRace::show()
{
    ui->retranslateUi(this);
    QWidget::show();
}

void LBChronoRace::resizeDialogs(QScreen const *screen)
{
    QRect screenGeometry = screen->availableGeometry();

    //NOSONAR this->setMaximumHeight(screenGeometry.height());
    raceInfo.setMaximumHeight(screenGeometry.height());
    // 15/16 is about 94% of the height; this is an ugly
    // workaround since at present time on high resolution
    // screen Qt::AA_EnableHighDpiScaling still does not
    // scale nicely
    startListTable.setMaximumHeight(screenGeometry.height() * 15 / 16);
    teamsTable.setMaximumHeight(screenGeometry.height() * 15 / 16);
    categoriesTable.setMaximumHeight(screenGeometry.height() * 15 / 16);
    timingsTable.setMaximumHeight(screenGeometry.height() * 15 / 16);
}

void LBChronoRace::selectorEncoding(QString const &arg1) const
{
    if (arg1.compare(tr("UTF-8"), Qt::CaseInsensitive) == 0) {
        CRLoader::setEncoding(CRLoader::Encoding::UTF8);
    } else {
        CRLoader::setEncoding(CRLoader::Encoding::LATIN1);
    }
    appendInfoMessage(tr("Selected encoding: %1").arg(arg1));
}

void LBChronoRace::selectorFormat(QString const &arg1) const
{
    if (arg1.compare(tr("PDF"), Qt::CaseInsensitive) == 0) {
        CRLoader::setFormat(CRLoader::Format::PDF);
    } else if (arg1.compare(tr("CSV"), Qt::CaseInsensitive) == 0) {
        CRLoader::setFormat(CRLoader::Format::CSV);
    } else {
        CRLoader::setFormat(CRLoader::Format::TEXT);
    }
    appendInfoMessage(tr("Selected format: %1").arg(arg1));
}

void LBChronoRace::loadRace()
{
    actionLoadRace();
}

void LBChronoRace::saveRace()
{
    actionSaveRace();
}

void LBChronoRace::editRace()
{
    actionEditRace();
}

void LBChronoRace::editStartList()
{
    actionEditStartList();
}

void LBChronoRace::editTeamsList()
{
    actionEditTeams();
}

void LBChronoRace::editCategories()
{
    actionEditCategories();
}

void LBChronoRace::editTimings()
{
    actionEditTimings();
}

void LBChronoRace::createStartList()
{
    makeStartList(CRLoader::getFormat());
}

void LBChronoRace::createRankings()
{
    makeRankings(CRLoader::getFormat());
}

void LBChronoRace::actionAbout()
{
    QMessageBox::about(this, tr("Informations"), tr("\n%1\n\nAuthor: Lorenzo Buzzi (lorenzo.buzzi@gmail.com)\n\nVersion: %2\n").arg(LBCHRONORACE_NAME, LBCHRONORACE_VERSION));
}

void LBChronoRace::actionAboutQt()
{
    QMessageBox::aboutQt(this, tr("About &Qt"));
}
