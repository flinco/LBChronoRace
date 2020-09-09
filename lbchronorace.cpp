#include <QString>
#include <QFileDialog>
#include <QLinkedList>
#include <QMap>
#include <QMultiMap>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QScreen>

#include "lbchronorace.h"
#include "ui_chronorace.h"
#include "crloader.h"
#include "classentry.h"
#include "teamclassentry.h"
#include "lbcrexception.h"

LBChronoRace::LBChronoRace(QWidget *parent, QGuiApplication *app) :
    QMainWindow(parent),
    ui(new Ui::LBChronoRace),
    raceDataFileName()
{
    lastSelectedPath   = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    startListFileName  = lastSelectedPath.filePath(LBCHRONORACE_STARTLIST_DEFAULT);
    timingsFileName    = lastSelectedPath.filePath(LBCHRONORACE_TIMINGS_DEFAULT);
    categoriesFileName = lastSelectedPath.filePath(LBCHRONORACE_CATEGORIES_DEFAULT);
    teamsFileName      = lastSelectedPath.filePath(LBCHRONORACE_TEAMLIST_DEFAULT);

    ui->setupUi(this);

    startListTable.setWindowTitle(tr("Start List"));
    startListTable.setModel(CRLoader::getStartListModel());
    this->connect(&startListTable, &ChronoRaceTable::modelImport, this, &LBChronoRace::importStartList);
    this->connect(&startListTable, &ChronoRaceTable::modelExport, this, &LBChronoRace::exportStartList);
    this->connect(&startListTable, SIGNAL(newRowCount(int)), this, SLOT(setCounterCompetitors(int)));

    teamsTable.disableButtons();
    teamsTable.setWindowTitle(tr("Teams List"));
    teamsTable.setModel(CRLoader::getTeamsListModel());
    this->connect(&teamsTable, &ChronoRaceTable::modelExport, this, &LBChronoRace::exportTeamList);
    this->connect(&teamsTable, SIGNAL(newRowCount(int)), this, SLOT(setCounterTeams(int)));

    StartListModel* startListModel = (StartListModel*) CRLoader::getStartListModel();
    TeamsListModel* teamsListModel = (TeamsListModel*) CRLoader::getTeamsListModel();
    this->connect(startListModel, SIGNAL(newTeam(const QString&)), teamsListModel, SLOT(addTeam(const QString&)));

    categoriesTable.setWindowTitle(tr("Categories"));
    categoriesTable.setModel(CRLoader::getCategoriesModel());
    this->connect(&categoriesTable, &ChronoRaceTable::modelImport, this, &LBChronoRace::importCategoriesList);
    this->connect(&categoriesTable, &ChronoRaceTable::modelExport, this, &LBChronoRace::exportCategoriesList);
    this->connect(&categoriesTable, SIGNAL(newRowCount(int)), this, SLOT(setCounterCategories(int)));

    timingsTable.setWindowTitle(tr("Timings List"));
    timingsTable.setModel(CRLoader::getTimingsModel());
    this->connect(&timingsTable, &ChronoRaceTable::modelImport, this, &LBChronoRace::importTimingsList);
    this->connect(&timingsTable, &ChronoRaceTable::modelExport, this, &LBChronoRace::exportTimingsList);
    this->connect(&timingsTable, SIGNAL(newRowCount(int)), this, SLOT(setCounterTimings(int)));

    // react to screen change and resize
    this->connect(app, &QGuiApplication::primaryScreenChanged, this, &LBChronoRace::resizeDialogs);
    resizeDialogs(QGuiApplication::primaryScreen());

    ui->makeRankingsPDF->setEnabled(false);
}

LBChronoRace::~LBChronoRace()
{
    delete ui;
}

void LBChronoRace::on_makeRankingsText_clicked()
{
    makeRankings(CRLoader::TEXT);
}

void LBChronoRace::on_makeRankingsCSV_clicked()
{
    makeRankings(CRLoader::CSV);
}

void LBChronoRace::on_makeRankingsPDF_clicked()
{
    makeRankings(CRLoader::PDF);
}

void LBChronoRace::makeRankings(CRLoader::Format format)
{

    try {
        // Compute the classifications
        QStringList messages;
        QMultiMap<uint, Competitor>& startList = CRLoader::getStartList(messages);
        QVector<Timing>& timings = CRLoader::getTimings(messages);
        QVector<Category>& rankings = CRLoader::getCategories(messages);

        uint bib, leg, i, bWidth, sWidth, rWidth, k;

        switch (format) {
            case CRLoader::TEXT:
            case CRLoader::CSV:
            case CRLoader::PDF:
                break;
            default:
                throw(ChronoRaceException(tr("Error: unknown rankings format %1").arg(format)));
                break;
        }

        for (auto message : messages)
            ui->infoDisplay->appendPlainText(tr("Warning: %1").arg(message));
        messages.clear();

        QString rankingsBasePath = QFileDialog::getExistingDirectory(this,
            tr("Select Rankings Destination Folder"), lastSelectedPath.absolutePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (!rankingsBasePath.isEmpty()) {
            i = startList.size();
            sWidth = 1;
            while ((i /= 10)) sWidth++;

            bib = CRLoader::getStartListBibMax();
            bWidth = 1;
            while ((bib /= 10)) bWidth++;

            i = rankings.size();
            rWidth = 1;
            while ((i /= 10)) rWidth++;

            // compute individual general classifications (all included, sorted by bib)
            QMap<uint, ClassEntry> rankingByBib = {};
            for (auto timing : timings) {

                bib = timing.getBib();
                leg = timing.getLeg();
                QPair<QMap<uint, Competitor>::iterator, QMap<uint, Competitor>::iterator> compItPair;
                compItPair = startList.equal_range(bib);

                if (compItPair.first == compItPair.second) {
                    // should never happen
                    ui->infoDisplay->appendPlainText(tr("Competitor not found for bib %1").arg(bib));
                    continue;
                }

                QMap<uint, ClassEntry>::iterator classEntryIt = rankingByBib.find(bib);
                if (leg == 0) // perform leg auto detection only if no manual leg hint is present
                {
                    if (classEntryIt != rankingByBib.end()) {
                        leg = classEntryIt->countTimes() + 1;
                    } else {
                        leg = 1;
                    }
                }

                Competitor *comp = NULL;
                for (auto compIt = compItPair.first; compIt != compItPair.second; compIt++) {
                    if (compIt->getLeg() == leg) {
                        comp = &(*compIt);
                        break;
                    }
                }
                if (!comp)
                {
                    ui->infoDisplay->appendPlainText(tr("Bib %1 skipped; check for possible duplicated entries").arg(bib));
                    continue;
                }

                if (classEntryIt != rankingByBib.end()) {
                    classEntryIt->setTime(comp, timing);
                } else {
                    rankingByBib.insert(bib, ClassEntry(bib))->setTime(comp, timing);
                }
            }

            // sort by time
            std::list<ClassEntry*> rankingByTime;
            std::list<ClassEntry*>::iterator c;
            for (auto classEntry = rankingByBib.begin(); classEntry != rankingByBib.end(); classEntry++) {
                c = rankingByTime.begin();
                while ((c != rankingByTime.end()) && (**c < classEntry.value())) ++c;
                rankingByTime.insert(c, &(*classEntry));
            }

            // now fill each ranking
            k = 0;
            for (auto ranking : rankings) {
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
                        if ((ranking.getSex() != Competitor::UNDEFINED) &&
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

                        const QString& team = classEntry->getTeam();

                        const QMap<QString, TeamClassEntry>::iterator teamRankingIt = teamRankingByTeam.find(team);
                        if (teamRankingIt == teamRankingByTeam.end()) {
                            teamRankingByTeam.insert(team, TeamClassEntry())->setClassEntry(classEntry);
                        } else {
                            teamRankingIt->setClassEntry(classEntry);
                        }
                    }

                    // sort the team rankings
                    std::list<TeamClassEntry*> sortedTeamRanking;
                    std::list<TeamClassEntry*>::iterator c;
                    for (auto teamClassEntry = teamRankingByTeam.begin(); teamClassEntry != teamRankingByTeam.end(); teamClassEntry++) {
                        c = sortedTeamRanking.begin();
                        while ((c != sortedTeamRanking.end()) && (**c < teamClassEntry.value())) ++c;
                        sortedTeamRanking.insert(c, &(*teamClassEntry));
                    }

                    // print the ranking
                    QString outFileName;
                    switch (format) {
                        case CRLoader::TEXT:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.txt").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                        case CRLoader::CSV:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.csv").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                        case CRLoader::PDF:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.pdf").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                    }
                    QFile outFile(outFileName);
                    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        throw(ChronoRaceException(QString("Error: cannot open %1").arg(outFileName)));
                    }
                    QTextStream outStream(&outFile);
                    QFileInfo outFileInfo(outFile);

                    switch (CRLoader::getEncoding()) {
                        case CRLoader::UTF8:
                            outStream.setCodec("UTF-8");
                            break;
                        case CRLoader::LATIN1:
                            // no break here
                        default:
                            outStream.setCodec("ISO-8859-1");
                            break;
                    }

                    switch (format) {
                        case CRLoader::TEXT:
                            outStream << raceInfo << Qt::endl; // add header
                            outStream << ranking.getFullDescription() << Qt::endl;
                            i = 0;
                            for (auto teamRanking : sortedTeamRanking) {
                                for (int j = 0; j < teamRanking->getClassEntryCount(); j++) {
                                    if (j == 0) {
                                        outStream.setFieldWidth(sWidth);
                                        outStream.setFieldAlignment(QTextStream::AlignRight);
                                        outStream << ++i;
                                        outStream.setFieldWidth(0);
                                        outStream << " - ";
                                    } else {
                                        outStream.setFieldWidth(sWidth + 3);
                                        outStream << "";
                                    }
                                    outStream.setFieldWidth(bWidth);
                                    outStream.setFieldAlignment(QTextStream::AlignRight);
                                    outStream << teamRanking->getClassEntry(j)->getBib();
                                    outStream.setFieldWidth(0);
                                    outStream << " - ";
                                    outStream << teamRanking->getClassEntry(j)->getNamesTxt();
                                    outStream << " - ";
                                    if (CRLoader::getStartListLegs() > 1)
                                        outStream << teamRanking->getClassEntry(j)->getTimesTxt(sWidth) << " - ";
                                    outStream << teamRanking->getClassEntry(j)->getTotalTimeTxt() << Qt::endl;
                                }
                                outStream << Qt::endl;
                            }
                            outStream << Qt::endl;
                            break;
                        case CRLoader::CSV:
                            outStream << ranking.getShortDescription() << Qt::endl;
                            i = 0;
                            for (auto teamRanking : sortedTeamRanking) {
                                i++;
                                for (int j = 0; j < teamRanking->getClassEntryCount(); j++) {
                                    outStream << i << ",";
                                    outStream << teamRanking->getClassEntry(j)->getBib() << ",";
                                    outStream << teamRanking->getClassEntry(j)->getNamesCSV() << ",";
                                    if (CRLoader::getStartListLegs() > 1)
                                        outStream << teamRanking->getClassEntry(j)->getTimesCSV() << ",";
                                    outStream << teamRanking->getClassEntry(j)->getTotalTimeCSV() << Qt::endl;
                                }
                            }
                            outStream << Qt::endl;
                            break;
                        case CRLoader::PDF:
                            ui->infoDisplay->appendPlainText(tr("PDF format still not supported"));
                            break;
                    }

                    ui->infoDisplay->appendPlainText(tr("Generated Ranking '%1': %2").arg(ranking.getFullDescription()).arg(outFileInfo.absoluteFilePath()));

                    outStream.flush();
                    outFile.close();

                } else {
                    // Individual ranking

                    QList<ClassEntry*> individualRanking;
                    for (auto classEntry : rankingByTime) {

                        // Sex
                        if ((ranking.getSex() != Competitor::UNDEFINED) &&
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

                        individualRanking.push_back(classEntry);
                    }

                    // do the sorting of the single leg times
                    for (leg = 0; leg < CRLoader::getStartListLegs(); leg++) {
                        QMultiMap<uint, ClassEntry*> sortedLegClassification;
                        for (auto classEntry : individualRanking) {
                            sortedLegClassification.insert(classEntry->getTime(leg), classEntry);
                        }
                        i = 0;
                        for (auto classEntry : sortedLegClassification) {
                            classEntry->setLegRanking(++i);
                        }
                    }

                    // print the ranking
                    QString outFileName;
                    switch (format) {
                        case CRLoader::TEXT:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.txt").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                        case CRLoader::CSV:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.csv").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                        case CRLoader::PDF:
                            outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.pdf").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()));
                            break;
                    }
                    QFile outFile(outFileName);
                    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFileName)));
                    }
                    QTextStream outStream(&outFile);
                    QFileInfo outFileInfo(outFile);

                    switch (CRLoader::getEncoding()) {
                        case CRLoader::UTF8:
                            outStream.setCodec("UTF-8");
                            break;
                        case CRLoader::LATIN1:
                            // no break here
                        default:
                            outStream.setCodec("ISO-8859-1");
                            break;
                    }

                    switch (format) {
                        case CRLoader::TEXT:
                            outStream << raceInfo << Qt::endl; // add header
                            outStream << ranking.getFullDescription() << Qt::endl;
                            i = 1;
                            for (auto c : individualRanking) {
                                outStream.setFieldWidth(sWidth);
                                outStream.setFieldAlignment(QTextStream::AlignRight);
                                outStream << i++;
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
                            break;
                        case CRLoader::CSV:
                            i = 1;
                            for (auto c : individualRanking) {
                                outStream << i++ << ",";
                                outStream << c->getBib() << ",";
                                outStream << c->getNamesCSV() << ",";
                                if (CRLoader::getStartListLegs() > 1)
                                    outStream << c->getTimesCSV() << ",";
                                outStream << c->getTotalTimeTxt() << Qt::endl;
                            }
                            outStream << Qt::endl;
                            break;
                        case CRLoader::PDF:
                            ui->infoDisplay->appendPlainText(tr("PDF format still not supported"));
                            break;
                    }

                    ui->infoDisplay->appendPlainText(tr("Generated Ranking '%1': %2").arg(ranking.getFullDescription()).arg(outFileInfo.absoluteFilePath()));

                    outStream.flush();
                    outFile.close();
                }
            }
        }
    } catch (ChronoRaceException& e) {
        ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
    }

}

void LBChronoRace::setCounterTeams(int count)
{
    ui->counterTeams->display(count);
}

void LBChronoRace::setCounterCompetitors(int count)
{
    ui->counterCompetitors->display(count);
}

void LBChronoRace::setCounterCategories(int count)
{
    ui->counterCategories->display(count);
}

void LBChronoRace::setCounterTimings(int count)
{
    ui->counterTimings->display(count);
}

void LBChronoRace::importStartList()
{
    startListFileName = QFileDialog::getOpenFileName(this,
        tr("Select Start List"), lastSelectedPath.filePath(QFileInfo(startListFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(startListFileName).absoluteDir();

    if (!startListFileName.isEmpty()) {
        QPair<int, int> count(0, 0);
        ui->infoDisplay->appendPlainText(tr("Start List File: %1").arg(startListFileName));
        try {
            count = CRLoader::importStartList(startListFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n competitor(s)", "", count.first));
            ui->infoDisplay->appendPlainText(tr("Loaded: %n team(s)", "", count.second));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterCompetitors(count.first);
        setCounterTeams(count.second);
    }
}

void LBChronoRace::importCategoriesList()
{
    categoriesFileName = QFileDialog::getOpenFileName(this,
        tr("Select Categories File"), lastSelectedPath.filePath(QFileInfo(categoriesFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();

    if (!categoriesFileName.isEmpty()) {
        int count = 0;
        ui->infoDisplay->appendPlainText(tr("Categories File: %1").arg(categoriesFileName));
        try {
            count = CRLoader::importCategories(categoriesFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n category(es)", "", count));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterCategories(count);
    }
}

void LBChronoRace::importTimingsList()
{
    timingsFileName = QFileDialog::getOpenFileName(this,
        tr("Select Timings File"), lastSelectedPath.filePath(QFileInfo(timingsFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();

    if (!timingsFileName.isEmpty()) {
        int count = 0;
        ui->infoDisplay->appendPlainText(tr("Timings File: %1").arg(timingsFileName));
        try {
            count = CRLoader::importTimings(timingsFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n timing(s)", "", count));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        setCounterTimings(count);
    }
}

void LBChronoRace::exportStartList()
{
    startListFileName = QFileDialog::getSaveFileName(this,
        tr("Select Start List"), lastSelectedPath.filePath(QFileInfo(startListFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(startListFileName).absoluteDir();

    if (!startListFileName.isEmpty()) {
        try {
            CRLoader::exportStartList(startListFileName);
            ui->infoDisplay->appendPlainText(tr("Start List File saved: %1").arg(startListFileName));
        }  catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTeamList()
{
    teamsFileName = QFileDialog::getSaveFileName(this,
        tr("Select Teams List"), lastSelectedPath.filePath(QFileInfo(teamsFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(teamsFileName).absoluteDir();

    if (!teamsFileName.isEmpty()) {
        try {
            CRLoader::exportTeams(teamsFileName);
            ui->infoDisplay->appendPlainText(tr("Teams File saved: %1").arg(teamsFileName));
        }  catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportCategoriesList()
{
    categoriesFileName = QFileDialog::getSaveFileName(this,
        tr("Select Categories File"), lastSelectedPath.filePath(QFileInfo(categoriesFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();

    if (!categoriesFileName.isEmpty()) {
        try {
            CRLoader::exportCategories(categoriesFileName);
            ui->infoDisplay->appendPlainText(tr("Categories File saved: %1").arg(categoriesFileName));
        }  catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTimingsList()
{
    timingsFileName = QFileDialog::getSaveFileName(this,
        tr("Select Timings File"), lastSelectedPath.filePath(QFileInfo(timingsFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();

    if (!timingsFileName.isEmpty()) {
        try {
            CRLoader::exportTimings(timingsFileName);
            ui->infoDisplay->appendPlainText(tr("Timings File saved: %1").arg(timingsFileName));
        }  catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::on_actionLoadRace_triggered()
{
    raceDataFileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Race Data File"), "",
                                                    tr("ChronoRace Data (*.crd)"));
    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);

        if (!raceDataFile.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        } else {
            quint32 binFmt;

            QDataStream in(&raceDataFile);
            in.setVersion(QDataStream::Qt_5_15);
            in >> binFmt;

            switch (binFmt) {
                case LBCHRONORACE_BIN_FMT_v1: {
                    QAbstractTableModel *table;
                    qint32 encodingIdx;

                    in >> encodingIdx;
                    ui->selectorEncoding->setCurrentIndex(encodingIdx);
                    on_selectorEncoding_activated(ui->selectorEncoding->currentText());
                    in >> raceInfo;
                    CRLoader::loadRaceData(in);

                    // Create the FileInfo
                    QFileInfo raceDataFileInfo(raceDataFileName);

                    // now get the fileName
                    QString newTitle(tr(LBCHRONORACE_NAME) + " - " + raceDataFileInfo.fileName());

                    // Set the Title to the fileName
                    setWindowTitle(newTitle);

                    table = CRLoader::getStartListModel();
                    setCounterCompetitors(table->rowCount());
                    table = CRLoader::getTeamsListModel();
                    setCounterTeams(table->rowCount());
                    table = CRLoader::getCategoriesModel();
                    setCounterCategories(table->rowCount());
                    table = CRLoader::getTimingsModel();
                    setCounterTimings(table->rowCount());
                }
                break;
                default:
                    QMessageBox::information(this, tr("Race Data File Error"), tr("Format version %1 not supported").arg(binFmt));
                break;
            }
        }
    }
}

void LBChronoRace::on_actionSaveRace_triggered()
{
    if (raceDataFileName.isEmpty()) {
        raceDataFileName = QFileDialog::getSaveFileName(this,
                                                        tr("Select Race Data File"), "",
                                                        tr("ChronoRace Data (*.crd)"));

        if (!raceDataFileName.endsWith(".crd", Qt::CaseInsensitive))
            raceDataFileName.append(".crd");
    }

    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);
        if (!raceDataFile.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        } else {
            QDataStream out(&raceDataFile);
            out.setVersion(QDataStream::Qt_5_15);
            out << quint32(LBCHRONORACE_BIN_FMT)
                << qint32(ui->selectorEncoding->currentIndex())
                << raceInfo;
            CRLoader::saveRaceData(out);
        }

    }
}

void LBChronoRace::on_actionSaveRaceAs_triggered()
{
    raceDataFileName.clear();
    on_actionSaveRace_triggered();
}

void LBChronoRace::on_actionQuit_triggered()
{
    QApplication::quit();
}

void LBChronoRace::on_actionEditRace_triggered()
{
    raceInfo.show();
}

void LBChronoRace::on_actionEditStartList_triggered()
{
    startListTable.show();
}

void LBChronoRace::on_actionEditTeams_triggered()
{
    teamsTable.show();
}

void LBChronoRace::on_actionEditCategories_triggered()
{
    categoriesTable.show();
}

void LBChronoRace::on_actionEditTimings_triggered()
{
    timingsTable.show();
}

void LBChronoRace::show()
{
    ui->retranslateUi(this);
    QWidget::show();
}

void LBChronoRace::resizeDialogs(QScreen *screen)
{
    QRect screenGeometry = screen->geometry();
    bool hiRes = (screenGeometry.width() > LBCHRONORACE_HIRES_WIDTH) && (screenGeometry.height() > LBCHRONORACE_HIRES_HEIGHT);

    //qDebug("%s(): %i x %i", __FUNCTION__, screenGeometry.width(), screenGeometry.height());

    QSize size(hiRes ? LBCHRONORACE_HIRES_WIDTH : LBCHRONORACE_LOWRES_WIDTH, hiRes ?LBCHRONORACE_HIRES_HEIGHT : LBCHRONORACE_LOWRES_HEIGHT);
    this->setMinimumSize(size);
    raceInfo.setMinimumSize(size);
    startListTable.setMinimumSize(size);
    teamsTable.setMinimumSize(size);
    categoriesTable.setMinimumSize(size);
    timingsTable.setMinimumSize(size);
}

void LBChronoRace::on_selectorEncoding_activated(const QString &arg1)
{
    if (arg1.compare(tr("UTF-8"), Qt::CaseInsensitive) == 0) {
        CRLoader::setEncoding(CRLoader::UTF8);
    } else {
        CRLoader::setEncoding(CRLoader::LATIN1);
    }
    ui->infoDisplay->appendPlainText(tr("Selected encoding: %1").arg(arg1));
}

void LBChronoRace::on_loadRace_clicked()
{
    on_actionLoadRace_triggered();
}

void LBChronoRace::on_saveRace_clicked()
{
    on_actionSaveRace_triggered();
}

void LBChronoRace::on_editRace_clicked()
{
    on_actionEditRace_triggered();
}

void LBChronoRace::on_editStartList_clicked()
{
    on_actionEditStartList_triggered();
}

void LBChronoRace::on_editTeamsList_clicked()
{
    on_actionEditTeams_triggered();
}

void LBChronoRace::on_editCategories_clicked()
{
    on_actionEditCategories_triggered();
}

void LBChronoRace::on_editTimings_clicked()
{
    on_actionEditTimings_triggered();
}

void LBChronoRace::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("Informations"), tr("\n%1\n\nAuthor: Lorenzo Buzzi (lorenzo.buzzi@gmail.com)\n\nVersion: %2\n").arg(LBCHRONORACE_NAME).arg(LBCHRONORACE_VERSION));
}

void LBChronoRace::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About &Qt"));
}
