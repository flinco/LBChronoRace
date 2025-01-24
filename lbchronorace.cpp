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

#include <QString>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScreen>
#include <QWindow>

//NOSONAR #include <QDebug>

#include "lbchronorace.hpp"
#include "lbcrexception.hpp"
#include "rankingswizard.hpp"
#include "crhelper.hpp"

// static members initialization
QDir LBChronoRace::lastSelectedPath(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
uint LBChronoRace::binFormat = LBCHRONORACE_BIN_FMT;
QRegularExpression LBChronoRace::csvFilter("[,;]");
QRegularExpression LBChronoRace::screenNameRegEx(R"(^[.\\]+(DISPLAY.*))");

LBChronoRace::LBChronoRace(QWidget *parent, QGuiApplication const *app) :
    QMainWindow(parent),
    raceInfo(parent),
    startListTable(parent),
    teamsTable(parent),
    rankingsTable(parent),
    categoriesTable(parent),
    timingsTable(parent),
    sexDelegate(&startListTable),
    clubDelegate(&startListTable),
    rankingTypeDelegate(&rankingsTable),
    rankingCatsDelegate(&rankingsTable),
    categoryTypeDelegate(&categoriesTable),
    timingStatusDelegate(&timingsTable)
{
    CRHelper::setParent(qobject_cast<QWidget *>(this));

    this->fileNames.resize(static_cast<int>(fileNameField::NUM_OF_FIELDS));

    this->fileNames[static_cast<int>(fileNameField::STARTLIST)]  = lastSelectedPath.filePath(LBCHRONORACE_STARTLIST_DEFAULT);
    this->fileNames[static_cast<int>(fileNameField::TIMINGS)]    = lastSelectedPath.filePath(LBCHRONORACE_TIMINGS_DEFAULT);
    this->fileNames[static_cast<int>(fileNameField::RANKINGS)]   = lastSelectedPath.filePath(LBCHRONORACE_RANKINGS_DEFAULT);
    this->fileNames[static_cast<int>(fileNameField::CATEGORIES)] = lastSelectedPath.filePath(LBCHRONORACE_CATEGORIES_DEFAULT);
    this->fileNames[static_cast<int>(fileNameField::TEAMS)]      = lastSelectedPath.filePath(LBCHRONORACE_TEAMLIST_DEFAULT);

    ui->setupUi(this);

    ui->liveViewSelector->setItemIcon(0, QIcon(":/material/icons/hide_image.svg"));

    QObject::connect(&raceInfo, &ChronoRaceData::globalDataChange, &CRHelper::updateGlobalData);

    auto *startListModel = dynamic_cast<StartListModel *>(CRLoader::getStartListModel());
    startListTable.setWindowTitle(tr("Competitors"));
    startListTable.setModel(startListModel);
    QObject::connect(&startListTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importStartList);
    QObject::connect(&startListTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportStartList);
    QObject::connect(&startListTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(startListModel, &StartListModel::error, this, &LBChronoRace::appendErrorMessage);
    startListModel->setCounter(ui->counterCompetitors);

    auto *teamsListModel = dynamic_cast<TeamsListModel *>(CRLoader::getTeamsListModel());
    teamsTable.disableButtons();
    teamsTable.setWindowTitle(tr("Clubs List"));
    teamsTable.setModel(teamsListModel);
    QObject::connect(&teamsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importTeamsList);
    QObject::connect(&teamsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportTeamList);
    QObject::connect(&teamsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    teamsListModel->setCounter(ui->counterTeams);

    // start list --> teams list
    QObject::connect(startListModel, &StartListModel::newClub, teamsListModel, &TeamsListModel::addTeam);

    auto *rankingsModel = dynamic_cast<RankingsModel *>(CRLoader::getRankingsModel());
    rankingsTable.setWindowTitle(tr("Rankings"));
    rankingsTable.setModel(rankingsModel);
    QObject::connect(&rankingsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importRankingsList);
    QObject::connect(&rankingsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportRankingsList);
    QObject::connect(&rankingsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(rankingsModel, &RankingsModel::error, this, &LBChronoRace::appendErrorMessage);
    rankingsModel->setCounter(ui->counterRankings);

    auto *categoriesModel = dynamic_cast<CategoriesModel *>(CRLoader::getCategoriesModel());
    categoriesTable.setWindowTitle(tr("Categories"));
    categoriesTable.setModel(categoriesModel);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importCategoriesList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportCategoriesList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(categoriesModel, &CategoriesModel::error, this, &LBChronoRace::appendErrorMessage);
    rankingCatsDelegate.setCategories(categoriesModel);
    categoriesModel->setCounter(ui->counterCategories);

    auto *timingsModel = dynamic_cast<TimingsModel *>(CRLoader::getTimingsModel());
    timingsTable.setWindowTitle(tr("Timings List"));
    timingsTable.setModel(timingsModel);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importTimingsList);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportTimingsList);
    QObject::connect(&timingsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(timingsModel, &TimingsModel::error, this, &LBChronoRace::appendErrorMessage);
    timingsModel->setCounter(ui->counterTimings);

    QObject::connect(&timings, &ChronoRaceTimings::info, this, &LBChronoRace::appendInfoMessage);
    QObject::connect(&timings, &ChronoRaceTimings::error, this, &LBChronoRace::appendErrorMessage);
    QObject::connect(app, &QGuiApplication::screenAdded, this, &LBChronoRace::screenAdded);
    QObject::connect(app, &QGuiApplication::screenRemoved, this, &LBChronoRace::screenRemoved);

    // react to screen change and resize
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, this, &LBChronoRace::resizeDialogs);
    resizeDialogs(QGuiApplication::primaryScreen());

    //NOSONAR ui->makeRankingsPDF->setEnabled(false);

    QObject::connect(ui->loadRace, &QPushButton::clicked, this, &LBChronoRace::loadRace);
    QObject::connect(ui->saveRace, &QPushButton::clicked, this, &LBChronoRace::saveRace);
    QObject::connect(ui->editRace, &QPushButton::clicked, &raceInfo, &ChronoRaceData::show);
    QObject::connect(ui->editStartList, &QPushButton::clicked, &startListTable, &ChronoRaceTable::show);
    QObject::connect(ui->editClubsList, &QPushButton::clicked, &teamsTable, &ChronoRaceTable::show);
    QObject::connect(ui->editRankings, &QPushButton::clicked, &rankingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->editCategories, &QPushButton::clicked, &categoriesTable, &ChronoRaceTable::show);
    QObject::connect(ui->editTimings, &QPushButton::clicked, &timingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->importTimings, &QPushButton::clicked, &timingsTable, &ChronoRaceTable::modelImport);
    QObject::connect(ui->exportTimings, &QPushButton::clicked, &timingsTable, &ChronoRaceTable::modelExport);
    QObject::connect(ui->makeStartList, &QPushButton::clicked, this, &LBChronoRace::makeStartList);
    QObject::connect(ui->collectTimings, &QPushButton::clicked, &timings, &ChronoRaceTimings::show);
    QObject::connect(ui->makeRankings, &QPushButton::clicked, this, &LBChronoRace::makeRankings);

    QObject::connect(ui->actionLoadRace, &QAction::triggered, this, &LBChronoRace::loadRace);
    QObject::connect(ui->actionSaveRace, &QAction::triggered, this, &LBChronoRace::saveRace);
    QObject::connect(ui->actionSaveRaceAs, &QAction::triggered, this, &LBChronoRace::saveRaceAs);
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &QApplication::quit);
    QObject::connect(ui->actionEditRace, &QAction::triggered, &raceInfo, &ChronoRaceData::show);
    QObject::connect(ui->actionEditStartList, &QAction::triggered, &startListTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditTeams, &QAction::triggered, &teamsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditRankings, &QAction::triggered, &rankingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditCategories, &QAction::triggered, &categoriesTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionImportTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::modelImport);
    QObject::connect(ui->actionExportTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::modelExport);
    QObject::connect(ui->actionSetEncoding, &QAction::triggered, this, &LBChronoRace::setEncoding);
    QObject::connect(ui->actionMakeStartList, &QAction::triggered, this, &LBChronoRace::makeStartList);
    QObject::connect(ui->actionCollectTimings, &QAction::triggered, &timings, &ChronoRaceTimings::show);
    QObject::connect(ui->actionMakeRankings, &QAction::triggered, this, &LBChronoRace::makeRankings);
    QObject::connect(ui->actionAddTimeSpan, &QAction::triggered, &timings, &ChronoRaceTimings::addTimeSpan);
    QObject::connect(ui->actionSubtractTimeSpan, &QAction::triggered, &timings, &ChronoRaceTimings::subtractTimeSpan);

    QObject::connect(ui->liveViewSelector, &QComboBox::currentIndexChanged, this, &LBChronoRace::live);

    QObject::connect(ui->actionAbout, &QAction::triggered, &CRHelper::actionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, &CRHelper::actionAboutQt);

    // tie the views with the related delegate instances
    startListTable.setItemDelegateForColumn(static_cast<int>(Competitor::Field::CMF_SEX), &sexDelegate);
    startListTable.setItemDelegateForColumn(static_cast<int>(Competitor::Field::CMF_CLUB), &clubDelegate);
    rankingsTable.setItemDelegateForColumn(static_cast<int>(Ranking::Field::RTF_TEAM), &rankingTypeDelegate);
    rankingsTable.setItemDelegateForColumn(static_cast<int>(Ranking::Field::RTF_CATEGORIES), &rankingCatsDelegate);
    categoriesTable.setItemDelegateForColumn(static_cast<int>(Category::Field::CTF_TYPE), &categoryTypeDelegate);
    timingsTable.setItemDelegateForColumn(static_cast<int>(Timing::Field::TMF_STATUS), &timingStatusDelegate);

    QRegularExpressionMatch match;
    QString liveText;
    QIcon liveIcon;
    bool screenEnabled;
    QStandardItemModel const *liveModel;
    auto liveIndex = this->ui->liveViewSelector->count();
    QList<QScreen *> screenList = QApplication::screens();
    for (QList<QScreen *>::const_iterator screenIt = screenList.constBegin(); screenIt != screenList.constEnd(); screenIt++) {
        screenEnabled = ((*screenIt)->size().width() >= 1280);

        match = screenNameRegEx.match((*screenIt)->name());
        liveText = match.hasMatch() ? match.captured(1) : (*screenIt)->name();
        liveIcon = QIcon(screenEnabled ? ":/material/icons/image.svg" : ":/material/icons/hide_image.svg");

        this->ui->liveViewSelector->insertItem(liveIndex, liveIcon, liveText, QVariant::fromValue(*screenIt));

        liveModel = qobject_cast<QStandardItemModel *>(this->ui->liveViewSelector->model());
        liveModel->item(liveIndex)->setEnabled(screenEnabled);

        liveIndex++;
    }

    if (liveIndex > 2)
        ui->liveViewSelector->setEnabled(true);
}

void LBChronoRace::appendInfoMessage(QString const &message) const
{
    if (auto sep = message.indexOf(':'); sep < 0) {
        ui->infoDisplay->appendHtml("<p><font color=\"blue\">" + message + "</font></p>");
    } else {
        ui->infoDisplay->appendHtml("<p><font color=\"blue\">" + message.first(sep) + "</font>" + message.sliced(sep) + "</p>");
    }
}

void LBChronoRace::appendErrorMessage(QString const &message) const
{
    if (auto sep = message.indexOf(':'); sep < 0) {
        ui->errorDisplay->appendHtml("<p><font color=\"red\">" + message + "</font></p>");
    } else if ((message.length() > (sep + 1)) && (message.at(sep + 1) == QChar(':'))) {
        ui->errorDisplay->appendHtml("<p><font color=\"orange\">" + message.first(sep) + "</font>" + message.sliced(sep + 1) + "</p>");
    } else {
        ui->errorDisplay->appendHtml("<p><font color=\"red\">" + message.first(sep) + "</font>" + message.sliced(sep) + "</p>");
    }
}

void LBChronoRace::importStartList()
{
    this->fileNames[static_cast<int>(fileNameField::STARTLIST)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Competitors"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::STARTLIST)].isEmpty()) {
        QPair<int, int> count(0, 0);
        bool append = CRHelper::askForAppend(this);
        appendInfoMessage(tr("Competitors File: %1").arg(this->fileNames[static_cast<int>(fileNameField::STARTLIST)]));
        try {
            count = CRLoader::importStartList(this->fileNames[static_cast<int>(fileNameField::STARTLIST)], append);
            if (append) {
                appendInfoMessage(tr("Appended: %n competitor(s)", "", count.first));
                appendInfoMessage(tr("Appended: %n club(s)", "", count.second));
            } else {
                appendInfoMessage(tr("Loaded: %n competitor(s)", "", count.first));
                appendInfoMessage(tr("Loaded: %n club(s)", "", count.second));
            }
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::STARTLIST)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importTeamsList()
{
    this->fileNames[static_cast<int>(fileNameField::TEAMS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Clubs File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::TEAMS)].isEmpty()) {
        int count = 0;
        bool append = true; // always append!
        appendInfoMessage(tr("Clubs File: %1").arg(this->fileNames[static_cast<int>(fileNameField::TEAMS)]));
        try {
            count = CRLoader::importTeams(this->fileNames[static_cast<int>(fileNameField::TEAMS)], append);
            if (append) {
                appendInfoMessage(tr("Appended: %n club(s)", "", count));
            } else {
                appendInfoMessage(tr("Loaded: %n club(s)", "", count));
            }
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::TEAMS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importRankingsList()
{
    this->fileNames[static_cast<int>(fileNameField::RANKINGS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Rankings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::RANKINGS)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(this);
        appendInfoMessage(tr("Rankings File: %1").arg(this->fileNames[static_cast<int>(fileNameField::RANKINGS)]));
        try {
            count = CRLoader::importRankings(this->fileNames[static_cast<int>(fileNameField::RANKINGS)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n ranking(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n ranking(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::RANKINGS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importCategoriesList()
{
    this->fileNames[static_cast<int>(fileNameField::CATEGORIES)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Categories File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::CATEGORIES)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(this);
        appendInfoMessage(tr("Categories File: %1").arg(this->fileNames[static_cast<int>(fileNameField::CATEGORIES)]));
        try {
            count = CRLoader::importCategories(this->fileNames[static_cast<int>(fileNameField::CATEGORIES)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n category(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n category(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::CATEGORIES)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importTimingsList()
{
    this->fileNames[static_cast<int>(fileNameField::TIMINGS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Timings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::TIMINGS)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(this);
        appendInfoMessage(tr("Timings File: %1").arg(this->fileNames[static_cast<int>(fileNameField::TIMINGS)]));
        try {
            count = CRLoader::importTimings(this->fileNames[static_cast<int>(fileNameField::TIMINGS)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n timing(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n timing(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::TIMINGS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportStartList()
{
    this->fileNames[static_cast<int>(fileNameField::STARTLIST)] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(this, tr("Select Competitors"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::STARTLIST)].isEmpty()) {

        if (!this->fileNames[static_cast<int>(fileNameField::STARTLIST)].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[static_cast<int>(fileNameField::STARTLIST)].append(".csv");

        try {
            CRLoader::exportModel(CRLoader::Model::STARTLIST, this->fileNames[static_cast<int>(fileNameField::STARTLIST)]);
            appendInfoMessage(tr("Competitors File saved: %1").arg(this->fileNames[static_cast<int>(fileNameField::STARTLIST)]));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::STARTLIST)]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportTeamList()
{
    this->fileNames[static_cast<int>(fileNameField::TEAMS)] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(this, tr("Select Clubs List"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::TEAMS)].isEmpty()) {

        if (!this->fileNames[static_cast<int>(fileNameField::TEAMS)].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[static_cast<int>(fileNameField::TEAMS)].append(".csv");

        try {
            CRLoader::exportModel(CRLoader::Model::TEAMSLIST, this->fileNames[static_cast<int>(fileNameField::TEAMS)]);
            appendInfoMessage(tr("Teams File saved: %1").arg(this->fileNames[static_cast<int>(fileNameField::TEAMS)]));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::TEAMS)]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportRankingsList()
{
    this->fileNames[static_cast<int>(fileNameField::RANKINGS)] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(this, tr("Select Rankings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::RANKINGS)].isEmpty()) {

        if (!this->fileNames[static_cast<int>(fileNameField::RANKINGS)].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[static_cast<int>(fileNameField::RANKINGS)].append(".csv");

        try {
            CRLoader::exportModel(CRLoader::Model::RANKINGS, this->fileNames[static_cast<int>(fileNameField::RANKINGS)]);
            appendInfoMessage(tr("Rankings File saved: %1").arg(this->fileNames[static_cast<int>(fileNameField::RANKINGS)]));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::RANKINGS)]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportCategoriesList()
{
    this->fileNames[static_cast<int>(fileNameField::CATEGORIES)] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(this, tr("Select Categories File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::CATEGORIES)].isEmpty()) {

        if (!this->fileNames[static_cast<int>(fileNameField::CATEGORIES)].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[static_cast<int>(fileNameField::CATEGORIES)].append(".csv");

        try {
            CRLoader::exportModel(CRLoader::Model::CATEGORIES, this->fileNames[static_cast<int>(fileNameField::CATEGORIES)]);
            appendInfoMessage(tr("Categories File saved: %1").arg(this->fileNames[static_cast<int>(fileNameField::CATEGORIES)]));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::CATEGORIES)]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportTimingsList()
{
    this->fileNames[static_cast<int>(fileNameField::TIMINGS)] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(this, tr("Select Timings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(fileNameField::TIMINGS)].isEmpty()) {

        if (!this->fileNames[static_cast<int>(fileNameField::TIMINGS)].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[static_cast<int>(fileNameField::TIMINGS)].append(".csv");

        try {
            CRLoader::exportModel(CRLoader::Model::TIMINGS, this->fileNames[static_cast<int>(fileNameField::TIMINGS)]);
            appendInfoMessage(tr("Timings File saved: %1").arg(this->fileNames[static_cast<int>(fileNameField::TIMINGS)]));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(fileNameField::TIMINGS)]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::initialize() {
    QStringList arguments = QCoreApplication::arguments();

    auto argument = arguments.constBegin();
    if (++argument != arguments.constEnd()) {
        if (loadRaceFile(*argument))
            raceDataFileName = *argument;
        while (++argument != arguments.constEnd())
            appendErrorMessage(tr("Warning: skipping file %1").arg(*argument));
    }
}

bool LBChronoRace::event(QEvent *event)
{
    bool retval = true;

    if (event->type() == QEvent::KeyPress) {
        auto const keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key::Key_Escape) &&
            (ui->liveViewSelector->currentIndex() > 0)) {
            ui->liveViewSelector->setCurrentIndex(0);
        } else {
            retval = QMainWindow::event(event);
        }
    } else {
        retval = QMainWindow::event(event);
    }

    return retval;
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

void LBChronoRace::encodingSelector(int idx) const
{
    switch (idx) {
    case 1:
        CRLoader::setEncoding(QStringConverter::Encoding::Utf8);
        break;
    case 0:
        CRLoader::setEncoding(QStringConverter::Encoding::Latin1);
        break;
    default:
        appendInfoMessage(tr("Unknown encoding %1; loaded default").arg(idx));
        CRLoader::setEncoding(QStringConverter::Encoding::Latin1);
        break;
    }

    appendInfoMessage(tr("Selected encoding: %1").arg(CRHelper::encodingToLabel(CRLoader::getEncoding())));
}

void LBChronoRace::formatSelector(int idx) const
{
    switch (idx) {
    case static_cast<int>(CRLoader::Format::PDF):
        CRLoader::setFormat(CRLoader::Format::PDF);
        break;
    case static_cast<int>(CRLoader::Format::TEXT):
        CRLoader::setFormat(CRLoader::Format::TEXT);
        break;
    case static_cast<int>(CRLoader::Format::CSV):
        CRLoader::setFormat(CRLoader::Format::CSV);
        break;
    default:
        CRLoader::setFormat(CRLoader::Format::PDF);
        break;
    }

    appendInfoMessage(tr("Selected format: %1").arg(CRHelper::formatToLabel(CRLoader::getFormat())));
}

bool LBChronoRace::loadRaceFile(QString const &fileName)
{
    bool retval = false;

    if (!fileName.isEmpty()) {
        QFile raceDataFile(fileName);
        lastSelectedPath = QFileInfo(fileName).absoluteDir();

        if (raceDataFile.open(QIODevice::ReadOnly)) {
            quint32 binFmt;
            ChronoRaceData::NameComposition nameComposition;
            ChronoRaceData::Accuracy accuracy;
            QFileInfo raceDataFileInfo(fileName);

            QDataStream in(&raceDataFile);
            in.setVersion(QDataStream::Qt_6_0);
            in >> binFmt;

            switch (binFmt) {
            case LBCHRONORACE_BIN_FMT_v1:
                [[fallthrough]];
            case LBCHRONORACE_BIN_FMT_v2:
                [[fallthrough]];
            case LBCHRONORACE_BIN_FMT_v3:
                QMessageBox::warning(this, tr("Race Data File Format"), tr("This Race Data File was saved with a previous release of the application.\nThe definitions of Categories and Rankings must be reviewed and corrected."));
                [[fallthrough]];
            case LBCHRONORACE_BIN_FMT_v4:
                in.setVersion(QDataStream::Qt_5_15);
                [[fallthrough]];
            case LBCHRONORACE_BIN_FMT_v5:
                QAbstractTableModel const *table;
                qint16 encodingIdx;
                qint16 formatIdx;
                int tableCount;

                binFormat = static_cast<int>(binFmt);

                in >> encodingIdx;
                encodingSelector(encodingIdx);
                in >> formatIdx;
                formatSelector(formatIdx);
                in >> raceInfo;
                raceInfo.getGlobalData(&nameComposition, &accuracy);
                CRHelper::updateGlobalData(nameComposition, accuracy);
                CRLoader::loadRaceData(in);

                // Set useful information
                lastSelectedPath = raceDataFileInfo.absoluteDir();
                setWindowTitle(QString(tr(LBCHRONORACE_NAME) + " - " + raceDataFileInfo.fileName()));

                table = CRLoader::getStartListModel();
                tableCount = table->rowCount();
                ui->counterCompetitors->display(tableCount);
                appendInfoMessage(tr("Loaded: %n competitor(s)", "", tableCount));
                table = CRLoader::getTeamsListModel();
                tableCount = table->rowCount();
                ui->counterTeams->display(tableCount);
                appendInfoMessage(tr("Loaded: %n club(s)", "", tableCount));
                table = CRLoader::getRankingsModel();
                tableCount = table->rowCount();
                ui->counterRankings->display(tableCount);
                appendInfoMessage(tr("Loaded: %n ranking(s)", "", tableCount));
                table = CRLoader::getCategoriesModel();
                tableCount = table->rowCount();
                ui->counterCategories->display(tableCount);
                appendInfoMessage(tr("Loaded: %n category(s)", "", tableCount));
                table = CRLoader::getTimingsModel();
                tableCount = table->rowCount();
                ui->counterTimings->display(tableCount);
                appendInfoMessage(tr("Loaded: %n timing(s)", "", tableCount));

                appendInfoMessage(tr("Race loaded: %1").arg(fileName));
                retval = true;
                break;
            default:
                QMessageBox::information(this, tr("Race Data File Error"), tr("Data format %1 not supported.\nPlease update the application.").arg(binFmt));
                break;
            }
        } else {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        }
    }

    return retval;
}

void LBChronoRace::toggleLiveView()
{
    auto const *liveScreen = liveTable->getLiveScreen();
    if (liveScreen != Q_NULLPTR) {

        timings.setLiveTable(liveTable.data());
        liveTable->setRaceInfo(&raceInfo);
        liveTable->show();

//NOSONAR #ifdef Q_OS_WIN
//NOSONAR         liveTable->windowHandle()->setLiveScreen(liveScreen);
//NOSONAR #else
        liveTable->windowHandle()->setGeometry(liveScreen->geometry());
//NOSONAR #endif


    } else {
        timings.setLiveTable(Q_NULLPTR);
        liveTable->hide();
    }
}

void LBChronoRace::loadRace()
{
    QString fileName = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Race Data File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("ChronoRace Data (*.crd)")));

    if (loadRaceFile(fileName))
        raceDataFileName = fileName;
}

void LBChronoRace::saveRace()
{
    if (raceDataFileName.isEmpty()) {
        raceDataFileName = QDir::toNativeSeparators(
            QFileDialog::getSaveFileName(this, tr("Select Race Data File"),
                                         lastSelectedPath.absolutePath(),
                                         tr("ChronoRace Data (*.crd)")));

        if (!raceDataFileName.isEmpty() && !raceDataFileName.endsWith(".crd", Qt::CaseInsensitive))
            raceDataFileName.append(".crd");
    }

    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);

        if (raceDataFile.open(QIODevice::WriteOnly)) {
            QFileInfo raceDataFileInfo(raceDataFileName);
            QDataStream out(&raceDataFile);

            out.setVersion(QDataStream::Qt_6_0);
            out << quint32(LBCHRONORACE_BIN_FMT);

            switch (CRLoader::getEncoding()) {
            case QStringConverter::Encoding::Utf8:
                out << qint16(1);
                break;
            case QStringConverter::Encoding::Latin1:
                out << qint16(0);
                break;
            default:
                appendInfoMessage(tr("Unknown encoding %1; default saved").arg(static_cast<int>(CRLoader::getEncoding())));
                out << qint16(0);
                break;
            }

            out << qint16(CRLoader::getFormat())
                << raceInfo;
            CRLoader::saveRaceData(out);

            // Set useful information
            lastSelectedPath = raceDataFileInfo.absoluteDir();
            setWindowTitle(QString(tr(LBCHRONORACE_NAME) + " - " + raceDataFileInfo.fileName()));
            appendInfoMessage(tr("Race saved: %1").arg(raceDataFileName));
        } else {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
            raceDataFileName.clear();
        }
    }
}

void LBChronoRace::saveRaceAs()
{
    QString oldRaceDataFileName(raceDataFileName);

    raceDataFileName.clear();
    saveRace();

    if (raceDataFileName.isEmpty())
        raceDataFileName = oldRaceDataFileName;
}

void LBChronoRace::setEncoding()
{
    bool ok = false;
    int current = 0;

    QStringList items = {
        CRHelper::encodingToLabel(QStringConverter::Encoding::Latin1),
        CRHelper::encodingToLabel(QStringConverter::Encoding::Utf8)
    };

    switch (CRLoader::getEncoding()) {
    case QStringConverter::Encoding::Latin1:
        current = 0;
        break;
    case QStringConverter::Encoding::Utf8:
        current = 1;
        break;
    default:
        appendErrorMessage(tr("Unexpected encoding value (fall back to the default)"));
        break;
    }

    QString item = QInputDialog::getItem(this,
                                         tr("Settings"),
                                         tr("CSV and Plain Text Encoding"),
                                         items,
                                         current,
                                         false,
                                         &ok);

    if (ok) {
        if (item == items[0])
            CRLoader::setEncoding(QStringConverter::Encoding::Latin1);
        else if (item == items[1])
            CRLoader::setEncoding(QStringConverter::Encoding::Utf8);
        else
            appendErrorMessage(tr("Unexpected encoding value (encoding not changed)"));
    }
}

void LBChronoRace::makeStartList()
{
    ui->errorDisplay->clear();

    try {
        RankingsWizard wizard(&raceInfo, &lastSelectedPath, RankingsWizard::RankingsWizardTarget::StartList);

        auto const &wizardInfoMessages = QObject::connect(&wizard, &RankingsWizard::info, this, &LBChronoRace::appendInfoMessage);
        auto const &wizardErrorMessages = QObject::connect(&wizard, &RankingsWizard::error, this, &LBChronoRace::appendErrorMessage);

        wizard.exec();

        QObject::disconnect(wizardErrorMessages);
        QObject::disconnect(wizardInfoMessages);

    } catch (ChronoRaceException &e) {
        appendErrorMessage(e.getMessage());
    }
}

void LBChronoRace::makeRankings()
{
    ui->errorDisplay->clear();

    try {
        RankingsWizard wizard(&raceInfo, &lastSelectedPath, RankingsWizard::RankingsWizardTarget::Rankings);

        auto const &wizardInfoMessages = QObject::connect(&wizard, &RankingsWizard::info, this, &LBChronoRace::appendInfoMessage);
        auto const &wizardErrorMessages = QObject::connect(&wizard, &RankingsWizard::error, this, &LBChronoRace::appendErrorMessage);

        wizard.exec();

        QObject::disconnect(wizardErrorMessages);
        QObject::disconnect(wizardInfoMessages);

    } catch (ChronoRaceException &e) {
        appendErrorMessage(e.getMessage());
    }
}

void LBChronoRace::screenRemoved(QScreen const *screen)
{
    if (auto screenIndex = this->ui->liveViewSelector->findData(QVariant::fromValue(screen)); screenIndex >= 0) {

        this->ui->liveViewSelector->removeItem(screenIndex);

        if (screen == liveTable->getLiveScreen()) {
            ui->liveViewSelector->setCurrentIndex(0);
        }

        if (this->ui->liveViewSelector->count() < 3) {
            ui->liveViewSelector->setCurrentIndex(0);
            ui->liveViewSelector->setEnabled(false);
        }
    }
}

void LBChronoRace::screenAdded(QScreen const *screen)
{
    if (auto screenIndex = this->ui->liveViewSelector->findData(QVariant::fromValue(screen)); screenIndex < 0) {
        bool screenEnabled = (screen->size().width() >= 1280);

        QRegularExpressionMatch match = screenNameRegEx.match(screen->name());
        this->ui->liveViewSelector->addItem(QIcon(screenEnabled ? ":/material/icons/image.svg" : ":/material/icons/hide_image.svg"), match.hasMatch() ? match.captured(1) : screen->name(), QVariant::fromValue(screen));

        auto itemCount = this->ui->liveViewSelector->count();

        auto const *liveModel = qobject_cast<QStandardItemModel *>(this->ui->liveViewSelector->model());
        liveModel->item(itemCount - 1)->setEnabled(screenEnabled);

        if (!screenEnabled)
            appendErrorMessage(tr("Notice:: Live View cannot be activated on screen %1 since %2px wide (min. required width 1280px)").arg(screen->name()).arg(screen->size().width()));

        if (itemCount > 2)
            ui->liveViewSelector->setEnabled(true);
    }
}

void LBChronoRace::live(int index)
{
    auto const *oldLiveScreen = liveTable->getLiveScreen();

    liveTable->setLiveScreen(Q_NULLPTR);
    if (index <= 0) {
        if (oldLiveScreen != Q_NULLPTR)
            appendInfoMessage(tr("Info: closing the Live View"));
    } else if (auto startList = CRLoader::getStartList(); startList.empty()) {
        appendErrorMessage(tr("Notice:: enter competitors to use the Live View"));
    } else try {
            liveTable->setLiveScreen(this->ui->liveViewSelector->currentData().value<QScreen const *>());
            liveTable->setStartList(startList);
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }

    toggleLiveView();

    if ((liveTable->getLiveScreen() == Q_NULLPTR) && (index > 0))
        ui->liveViewSelector->setCurrentIndex(0);
}
