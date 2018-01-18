#include <QString>
#include <QFileDialog>
#include <QLinkedList>
#include <QMap>
#include <QMultiMap>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>

#include "lbchronorace.h"
#include "ui_chronorace.h"
#include "crloader.h"
#include "classentry.h"
#include "teamclassentry.h"
#include "lbcrexception.h"

LBChronoRace::LBChronoRace(QWidget *parent) : QMainWindow(parent), ui(new Ui::LBChronoRace) {
    lastSelectedPath   = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    startListFileName  = lastSelectedPath.filePath(LBCHRONORACE_STARTLIST_DEFAULT);
    timingsFileName    = lastSelectedPath.filePath(LBCHRONORACE_TIMINGS_DEFAULT);
    categoriesFileName = lastSelectedPath.filePath(LBCHRONORACE_CATEGORIES_DEFAULT);
    teamsFileName      = lastSelectedPath.filePath(LBCHRONORACE_TEAMLIST_DEFAULT);

    ui->setupUi(this);

    startListTable.setWindowTitle(tr("Start List"));
    startListTable.setModel(CRLoader::getStartListModel());
    this->connect(&startListTable, &ChronoRaceTable::modelSave, this, &LBChronoRace::save_startList);
    this->connect(&startListTable, &ChronoRaceTable::modelSave, this, &LBChronoRace::save_teamList);
    this->connect(&startListTable, SIGNAL(newRowCount(int)), this, SLOT(set_counterCompetitors(int)));

    teamsTable.disableButtons();
    teamsTable.setWindowTitle(tr("Teams List"));
    teamsTable.setModel(CRLoader::getTeamsListModel());
    this->connect(&teamsTable, SIGNAL(newRowCount(int)), this, SLOT(set_counterTeams(int)));

    StartListModel* startListModel = (StartListModel*) CRLoader::getStartListModel();
    TeamsListModel* teamsListModel = (TeamsListModel*) CRLoader::getTeamsListModel();
    this->connect(startListModel, SIGNAL(newTeam(const QString&)), teamsListModel, SLOT(addTeam(const QString&)));

    categoriesTable.setWindowTitle(tr("Categories"));
    categoriesTable.setModel(CRLoader::getCategoriesModel());
    this->connect(&categoriesTable, &ChronoRaceTable::modelSave, this, &LBChronoRace::save_categoriesList);
    this->connect(&categoriesTable, SIGNAL(newRowCount(int)), this, SLOT(set_counterCategories(int)));

    timingsTable.setWindowTitle(tr("Timings List"));
    timingsTable.setModel(CRLoader::getTimingsModel());
    this->connect(&timingsTable, &ChronoRaceTable::modelSave, this, &LBChronoRace::save_timingsList);
    this->connect(&timingsTable, SIGNAL(newRowCount(int)), this, SLOT(set_counterTimings(int)));
}

LBChronoRace::~LBChronoRace() {
    delete ui;
}

void LBChronoRace::on_loadStartList_clicked() {
    startListFileName = QFileDialog::getOpenFileName(this,
        tr("Select Start List"), lastSelectedPath.filePath(QFileInfo(startListFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(startListFileName).absoluteDir();

    if (!startListFileName.isEmpty()) {
        QPair<int, int> count(0, 0);
        ui->infoDisplay->appendPlainText(tr("Start List File: %1").arg(startListFileName));
        try {
            count = CRLoader::loadStartList(startListFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n competitor(s)", "", count.first));
            ui->infoDisplay->appendPlainText(tr("Loaded: %n team(s)", "", count.second));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        set_counterCompetitors(count.first);
        set_counterTeams(count.second);
        //enable_makeRankings();
    }
}

void LBChronoRace::on_loadCategories_clicked() {
    categoriesFileName = QFileDialog::getOpenFileName(this,
        tr("Select Categories File"), lastSelectedPath.filePath(QFileInfo(categoriesFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();

    if (!categoriesFileName.isEmpty()) {
        int count = 0;
        ui->infoDisplay->appendPlainText(tr("Categories File: %1").arg(categoriesFileName));
        try {
            count = CRLoader::loadCategories(categoriesFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n category(es)", "", count));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        set_counterCategories(count);
        //enable_makeRankings();
    }
}

void LBChronoRace::on_loadTimings_clicked() {
    timingsFileName = QFileDialog::getOpenFileName(this,
        tr("Select Timings File"), lastSelectedPath.filePath(QFileInfo(timingsFileName).baseName()), tr("CSV (*.csv)"));
    lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();

    if (!timingsFileName.isEmpty()) {
        int count = 0;
        ui->infoDisplay->appendPlainText(tr("Timings File: %1").arg(timingsFileName));
        try {
            count = CRLoader::loadTimings(timingsFileName);
            ui->infoDisplay->appendPlainText(tr("Loaded: %n timing(s)", "", count));
        } catch (ChronoRaceException& e) {
            ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
        }
        set_counterTimings(count);
        //enable_makeRankings();
    }
}

void LBChronoRace::on_makeRankings_clicked() {

    try {
        // Compute the classifications
        QStringList messages;
        QMultiMap<uint, Competitor>& startList = CRLoader::getStartList(messages);
        QVector<Timing>& timings = CRLoader::getTimings(messages);
        QVector<Category>& rankings = CRLoader::getCategories(messages);

        uint bib, leg, i, bWidth, sWidth, rWidth, k;

        for (auto message : messages)
            ui->infoDisplay->appendPlainText(tr("Warning: %1").arg(message));
        messages.clear();

        QString rankingsBasePath = QFileDialog::getExistingDirectory(this,
            tr("Select Rankings Destination Folder"), lastSelectedPath.absolutePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

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
            QPair<QMap<uint, Competitor>::iterator, QMap<uint, Competitor>::iterator> compItPair;
            compItPair = startList.equal_range(bib);

            if (compItPair.first == compItPair.second) {
                // should never happen
                ui->infoDisplay->appendPlainText(tr("Competitor not found for bib %1").arg(bib));
                continue;
            }

            QMap<uint, ClassEntry>::iterator classEntryIt = rankingByBib.find(bib);
            if (classEntryIt != rankingByBib.end()) {
                leg = classEntryIt->countTimes() + 1;
            } else {
                leg = 1;
            }
            Competitor *comp = NULL;
            for (auto compIt = compItPair.first; compIt != compItPair.second; compIt++) {
                if (compIt->getLeg() == leg) {
                    comp = &(*compIt);
                    break;
                }
            }
            Q_ASSERT(comp);

            if (classEntryIt != rankingByBib.end()) {
                classEntryIt->setTime(comp, timing);
            } else {
                rankingByBib.insert(bib, ClassEntry(bib))->setTime(comp, timing);
            }
        }

        // sort by time
        QLinkedList<ClassEntry*> rankingByTime;
        QLinkedList<ClassEntry*>::iterator c;
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
                QLinkedList<TeamClassEntry*> sortedTeamRanking;
                QLinkedList<TeamClassEntry*>::iterator c;
                for (auto teamClassEntry = teamRankingByTeam.begin(); teamClassEntry != teamRankingByTeam.end(); teamClassEntry++) {
                    c = sortedTeamRanking.begin();
                    while ((c != sortedTeamRanking.end()) && (**c < teamClassEntry.value())) ++c;
                    sortedTeamRanking.insert(c, &(*teamClassEntry));
                }

                // print the ranking
                bool plainText = (CRLoader::getFormat() == CRLoader::TEXT);
                QString outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.%3").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()).arg((plainText ? "txt" : "csv")));
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

                if (plainText) {

                    outStream << ranking.getFullDescription() << endl;
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
                            outStream << teamRanking->getClassEntry(j)->getTotalTimeTxt() << endl;
                        }
                        outStream << endl;
                    }
                    outStream << endl;
                } else {
                    outStream << ranking.getShortDescription() << endl;
                    i = 0;
                    for (auto teamRanking : sortedTeamRanking) {
                        i++;
                        for (int j = 0; j < teamRanking->getClassEntryCount(); j++) {
                            outStream << i << ",";
                            outStream << teamRanking->getClassEntry(j)->getBib() << ",";
                            outStream << teamRanking->getClassEntry(j)->getNamesCSV() << ",";
                            if (CRLoader::getStartListLegs() > 1)
                                outStream << teamRanking->getClassEntry(j)->getTimesCSV() << ",";
                            outStream << teamRanking->getClassEntry(j)->getTotalTimeCSV() << endl;
                        }
                    }
                    outStream << endl;
                }
                ui->infoDisplay->appendPlainText(tr("Generated Ranking '%1': %2").arg(ranking.getFullDescription()).arg(outFileInfo.absoluteFilePath()));

                outStream.flush();
                outFile.close();

            } else {
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
                    QMap<uint, ClassEntry*> sortedLegClassification;
                    for (auto classEntry : individualRanking) {
                        sortedLegClassification.insertMulti(classEntry->getTime(leg), classEntry);
                    }
                    i = 0;
                    for (auto classEntry : sortedLegClassification) {
                        classEntry->setLegRanking(++i);
                    }
                }

                // print the ranking
                bool plainText = (CRLoader::getFormat() == CRLoader::TEXT);
                QString outFileName = QDir(rankingsBasePath).filePath(QString("class%1_%2.%3").arg(k, rWidth, 10, QLatin1Char('0')).arg(ranking.getShortDescription()).arg((plainText ? "txt" : "csv")));
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

                if (plainText) {

                    outStream << ranking.getFullDescription() << endl;
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
                        outStream << c->getTotalTimeTxt() << endl;
                    }
                    outStream << endl;
                } else {
                    i = 1;
                    for (auto c : individualRanking) {
                        outStream << i++ << ",";
                        outStream << c->getBib() << ",";
                        outStream << c->getNamesCSV() << ",";
                        if (CRLoader::getStartListLegs() > 1)
                            outStream << c->getTimesCSV() << ",";
                        outStream << c->getTotalTimeTxt() << endl;
                    }
                    outStream << endl;
                }
                ui->infoDisplay->appendPlainText(tr("Generated Ranking '%1': %2").arg(ranking.getFullDescription()).arg(outFileInfo.absoluteFilePath()));

                outStream.flush();
                outFile.close();
            }
        }
    } catch (ChronoRaceException& e) {
        ui->infoDisplay->appendPlainText(tr("Error: %1").arg(e.getMessage()));
    }

}

void LBChronoRace::set_counterTeams(int count) {
    ui->counterTeams->display(count);
}

void LBChronoRace::set_counterCompetitors(int count) {
    ui->counterCompetitors->display(count);
}

void LBChronoRace::set_counterCategories(int count) {
    ui->counterCategories->display(count);
}

void LBChronoRace::set_counterTimings(int count) {
    ui->counterTimings->display(count);
}

void LBChronoRace::save_startList() {
    CRLoader::saveStartList(startListFileName);
    ui->infoDisplay->appendPlainText(tr("Start List File saved: %1").arg(startListFileName));
}

void LBChronoRace::save_teamList() {
    teamsFileName = lastSelectedPath.filePath(QFileInfo(teamsFileName).baseName());
    CRLoader::saveTeams(teamsFileName);
    ui->infoDisplay->appendPlainText(tr("Teams File saved: %1").arg(teamsFileName));
}

void LBChronoRace::save_categoriesList() {
    CRLoader::saveCategories(categoriesFileName);
    ui->infoDisplay->appendPlainText(tr("Categories File saved: %1").arg(categoriesFileName));
}

void LBChronoRace::save_timingsList() {
    CRLoader::saveTimings(timingsFileName);
    ui->infoDisplay->appendPlainText(tr("Timings File saved: %1").arg(timingsFileName));
}

//void LBChronoRace::enable_makeRankings() {
//    ui->makeRankings->setEnabled(ui->clearCategories->isEnabled() && ui->clearStartList->isEnabled() && ui->clearTimings->isEnabled());
//}


void LBChronoRace::on_actionQuit_triggered() {
    QApplication::quit();
}

void LBChronoRace::on_actionStartListView_triggered() {
    startListTable.show();
}

void LBChronoRace::on_actionTeamsView_triggered() {
    teamsTable.show();
}

void LBChronoRace::on_actionCategoriesView_triggered() {
    categoriesTable.show();
}

void LBChronoRace::on_actionTimingsView_triggered() {
    timingsTable.show();
}

void LBChronoRace::on_actionSave_triggered() {
    save_startList();
    save_teamList();
    save_timingsList();
    save_categoriesList();
}

void LBChronoRace::on_selectorEncoding_activated(const QString &arg1) {
    if (arg1.compare(tr("UTF-8"), Qt::CaseInsensitive) == 0) {
        CRLoader::setEncoding(CRLoader::UTF8);
    } else {
        CRLoader::setEncoding(CRLoader::LATIN1);
    }
    ui->infoDisplay->appendPlainText(tr("Selected encoding: %1").arg(arg1));
}

void LBChronoRace::on_selectorFormat_activated(const QString &arg1) {
    if (arg1.compare(tr("Plain Text"), Qt::CaseInsensitive) == 0) {
        CRLoader::setFormat(CRLoader::TEXT);
    } else {
        CRLoader::setFormat(CRLoader::CSV);
    }
    ui->infoDisplay->appendPlainText(tr("Output mode: %1").arg(arg1));
}

void LBChronoRace::on_viewStartList_clicked() {
    on_actionStartListView_triggered();
}

void LBChronoRace::on_viewTeamsList_clicked() {
    on_actionTeamsView_triggered();
}

void LBChronoRace::on_viewCategories_clicked() {
    on_actionCategoriesView_triggered();
}

void LBChronoRace::on_viewTimings_clicked() {
    on_actionTimingsView_triggered();
}

void LBChronoRace::on_actionAbout_triggered() {
    QMessageBox::about(this, tr("Informations"), tr("\n%1\n\nAuthor: Lorenzo Buzzi (lorenzo.buzzi@gmail.com)\n\nVersion: %2\n").arg(LBCHRONORACE_NAME).arg(LBCHRONORACE_VERSION));
}

