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

#include <QStringList>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QScreen>
#include <QWindow>
#include <QActionGroup>
#include <QStandardItemModel>

#include "lbchronorace.hpp"
#include "lbcrexception.hpp"
#include "wizards/newracewizard.hpp"
#include "wizards/rankingswizard.hpp"
#include "crhelper.hpp"
#include "crsettings.hpp"
#include "recentraces.hpp"
#include "languages.hpp"
#include "triggerkeydialog.hpp"

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
    timings(this),
    sexDelegate(&startListTable),
    clubDelegate(&startListTable),
    rankingTypeDelegate(&rankingsTable),
    rankingCatsDelegate(&rankingsTable),
    categoryTypeDelegate(&categoriesTable),
    timingStatusDelegate(&timingsTable),
    liveView(this, &timings),
    liveColors(this),
    updater(this)
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
    QObject::connect(&raceInfo, &ChronoRaceData::error, this, &LBChronoRace::appendErrorMessage);

    auto *startListModel = dynamic_cast<StartListModel *>(CRLoader::getStartListModel());
    startListTable.setWindowTitle(tr("Competitors"));
    startListTable.setModel(startListModel);
    QObject::connect(&startListTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importStartList);
    QObject::connect(&startListTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportList);
    QObject::connect(&startListTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(startListModel, &StartListModel::dataChanged, &CRLoader::setDirty);
    QObject::connect(startListModel, &StartListModel::error, this, &LBChronoRace::appendErrorMessage);
    startListModel->setCounter(ui->counterCompetitors);

    auto *teamsListModel = dynamic_cast<TeamsListModel *>(CRLoader::getTeamsListModel());
    teamsTable.disableButtons();
    teamsTable.setWindowTitle(tr("Clubs List"));
    teamsTable.setModel(teamsListModel);
    QObject::connect(&teamsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importTeamsList);
    QObject::connect(&teamsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportList);
    QObject::connect(&teamsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    teamsListModel->setCounter(ui->counterTeams);

    // start list --> teams list
    QObject::connect(startListModel, &StartListModel::newClub, teamsListModel, &TeamsListModel::addTeam);

    auto *rankingsModel = dynamic_cast<RankingsModel *>(CRLoader::getRankingsModel());
    rankingsTable.setWindowTitle(tr("Rankings"));
    rankingsTable.setModel(rankingsModel);
    QObject::connect(&rankingsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importRankingsList);
    QObject::connect(&rankingsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportList);
    QObject::connect(&rankingsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(rankingsModel, &RankingsModel::dataChanged, &CRLoader::setDirty);
    QObject::connect(rankingsModel, &RankingsModel::error, this, &LBChronoRace::appendErrorMessage);
    rankingsModel->setCounter(ui->counterRankings);

    auto *categoriesModel = dynamic_cast<CategoriesModel *>(CRLoader::getCategoriesModel());
    categoriesTable.setWindowTitle(tr("Categories"));
    categoriesTable.setModel(categoriesModel);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importCategoriesList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportList);
    QObject::connect(&categoriesTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(categoriesModel, &CategoriesModel::dataChanged, &CRLoader::setDirty);
    QObject::connect(categoriesModel, &CategoriesModel::error, this, &LBChronoRace::appendErrorMessage);
    rankingCatsDelegate.setCategories(categoriesModel);
    categoriesModel->setCounter(ui->counterCategories);

    auto *timingsModel = dynamic_cast<TimingsModel *>(CRLoader::getTimingsModel());
    timingsTable.setWindowTitle(tr("Timings List"));
    timingsTable.setModel(timingsModel);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelImported, this, &LBChronoRace::importTimingsList);
    QObject::connect(&timingsTable, &ChronoRaceTable::modelExported, this, &LBChronoRace::exportList);
    QObject::connect(&timingsTable, &ChronoRaceTable::saveRaceData, this, &LBChronoRace::saveRace);
    QObject::connect(timingsModel, &TimingsModel::dataChanged, &CRLoader::setDirty);
    QObject::connect(timingsModel, &TimingsModel::error, this, &LBChronoRace::appendErrorMessage);
    timingsModel->setCounter(ui->counterTimings);

    QObject::connect(&timings, &ChronoRaceTimings::info, this, &LBChronoRace::appendInfoMessage);
    QObject::connect(&timings, &ChronoRaceTimings::error, this, &LBChronoRace::appendErrorMessage);
    QObject::connect(&timings, &ChronoRaceTimings::timerValue, &liveView, &LiveView::setTimerValue);
    QObject::connect(app, &QGuiApplication::screenAdded, this, &LBChronoRace::screenAdded);
    QObject::connect(app, &QGuiApplication::screenRemoved, this, &LBChronoRace::screenRemoved);

    // react to screen change and resize
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, this, &LBChronoRace::resizeDialogs);
    resizeDialogs(QGuiApplication::primaryScreen());

    //NOSONAR ui->makeRankingsPDF->setEnabled(false);

    QObject::connect(ui->newRace, &QPushButton::clicked, this, &LBChronoRace::newRace);
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
    QObject::connect(ui->collectTimings, &QPushButton::clicked, this, &LBChronoRace::showTimingRecorder);
    QObject::connect(ui->makeRankings, &QPushButton::clicked, this, &LBChronoRace::makeRankings);

    QObject::connect(ui->actionNewRace, &QAction::triggered, this, &LBChronoRace::newRace);
    QObject::connect(ui->actionLoadRace, &QAction::triggered, this, &LBChronoRace::loadRace);
    QObject::connect(ui->actionSaveRace, &QAction::triggered, this, &LBChronoRace::saveRace);
    QObject::connect(ui->actionSaveRaceAs, &QAction::triggered, this, &LBChronoRace::saveRace);
    QObject::connect(ui->actionEditRace, &QAction::triggered, &raceInfo, &ChronoRaceData::show);
    QObject::connect(ui->actionEditStartList, &QAction::triggered, &startListTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditTeams, &QAction::triggered, &teamsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditRankings, &QAction::triggered, &rankingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditCategories, &QAction::triggered, &categoriesTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionImportTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::modelImport);
    QObject::connect(ui->actionExportTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::modelExport);
    QObject::connect(ui->actionSetEncoding, &QAction::triggered, this, &LBChronoRace::setEncoding);
    QObject::connect(ui->actionConfigureTrigger, &QAction::triggered, this, &LBChronoRace::setupTrigger);
    QObject::connect(ui->actionMakeStartList, &QAction::triggered, this, &LBChronoRace::makeStartList);
    QObject::connect(ui->actionCollectTimings, &QAction::triggered, this, &LBChronoRace::showTimingRecorder);
    QObject::connect(ui->actionMakeRankings, &QAction::triggered, this, &LBChronoRace::makeRankings);
    QObject::connect(ui->actionAddTimeSpan, &QAction::triggered, &timings, &ChronoRaceTimings::addTimeSpan);
    QObject::connect(ui->actionSubtractTimeSpan, &QAction::triggered, &timings, &ChronoRaceTimings::subtractTimeSpan);
    QObject::connect(ui->actionClearTeamsList, &QAction::triggered, &CRLoader::clearTeamsList);
    QObject::connect(ui->actionLiveRankingsRotation, &QAction::triggered, &liveView, &LiveView::setInterval);
    QObject::connect(ui->actionLiveViewColors, &QAction::triggered, &liveColors, &LiveColors::show);

    QObject::connect(ui->liveViewSelector, &QComboBox::currentIndexChanged, this, &LBChronoRace::live);
    // NB: in case this binding is too heavy (the list is fully reloaded by the Live View every
    //     time a competitor is changed) just comment this line and uncomment the next.
    QObject::connect(startListModel, &StartListModel::dataChanged, &liveView, &LiveView::reloadStartList);
    //NOSONAR QObject::connect(&startListTable, &ChronoRaceTable::finished, &liveView, &LiveView::toggleCompetitors);
    QObject::connect(&timings, &QDialog::finished, this, &LBChronoRace::hideTimingRecorder);

    QObject::connect(ui->actionAbout, &QAction::triggered, &CRHelper::actionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, &CRHelper::actionAboutQt);
    QObject::connect(ui->actionUpdate, &QAction::triggered, &updater, &Updates::startCheck);

    QObject::connect(&updater, &Updates::info, this, &LBChronoRace::appendInfoMessage);
    QObject::connect(&updater, &Updates::error, this, &LBChronoRace::appendErrorMessage);

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

    // Race data for Live View
    this->liveView.setRaceInfo(&this->raceInfo);

    // ScreenSaver
    this->liveView.setScreenSaver(&this->screenSaver);

    // Recent races menu
    RecentRaces::loadMenu(ui->menuRecentRaces);
    QObject::connect(RecentRaces::actionGroup, &QActionGroup::triggered, this, &LBChronoRace::openRecentRace);

    // Languages menu
    Languages::loadMenu(ui->menuSetLanguage, QStringLiteral("lbchronorace"));

    // Quit action
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &QApplication::quit);

    // Other fields
    CRSettings::pushFocus(this->focusPolicy());
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
    using enum LBChronoRace::fileNameField;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    this->fileNames[static_cast<int>(STARTLIST)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(senderObject, tr("Select Competitors"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(STARTLIST)].isEmpty()) {
        QPair<int, int> count(0, 0);
        bool append = CRHelper::askForAppend(senderObject);
        appendInfoMessage(tr("Competitors File: %1").arg(this->fileNames[static_cast<int>(STARTLIST)]));
        try {
            count = CRLoader::importStartList(this->fileNames[static_cast<int>(STARTLIST)], append);
            if (append) {
                appendInfoMessage(tr("Appended: %n competitor(s)", "", count.first));
                appendInfoMessage(tr("Appended: %n club(s)", "", count.second));
            } else {
                appendInfoMessage(tr("Loaded: %n competitor(s)", "", count.first));
                appendInfoMessage(tr("Loaded: %n club(s)", "", count.second));
            }
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(STARTLIST)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importTeamsList()
{
    using enum LBChronoRace::fileNameField;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    this->fileNames[static_cast<int>(TEAMS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(senderObject, tr("Select Clubs File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(TEAMS)].isEmpty()) {
        int count = 0;
        bool append = true; // always append!
        appendInfoMessage(tr("Clubs File: %1").arg(this->fileNames[static_cast<int>(TEAMS)]));
        try {
            count = CRLoader::importTeams(this->fileNames[static_cast<int>(TEAMS)], append);
            if (append) {
                appendInfoMessage(tr("Appended: %n club(s)", "", count));
            } else {
                appendInfoMessage(tr("Loaded: %n club(s)", "", count));
            }
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(TEAMS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importRankingsList()
{
    using enum LBChronoRace::fileNameField;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    this->fileNames[static_cast<int>(RANKINGS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(senderObject, tr("Select Rankings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(RANKINGS)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(senderObject);
        appendInfoMessage(tr("Rankings File: %1").arg(this->fileNames[static_cast<int>(RANKINGS)]));
        try {
            count = CRLoader::importRankings(this->fileNames[static_cast<int>(RANKINGS)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n ranking(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n ranking(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(RANKINGS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importCategoriesList()
{
    using enum LBChronoRace::fileNameField;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    this->fileNames[static_cast<int>(CATEGORIES)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(senderObject, tr("Select Categories File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(CATEGORIES)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(senderObject);
        appendInfoMessage(tr("Categories File: %1").arg(this->fileNames[static_cast<int>(CATEGORIES)]));
        try {
            count = CRLoader::importCategories(this->fileNames[static_cast<int>(CATEGORIES)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n category(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n category(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(CATEGORIES)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::importTimingsList()
{
    using enum LBChronoRace::fileNameField;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    this->fileNames[static_cast<int>(TIMINGS)] = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(senderObject, tr("Select Timings File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[static_cast<int>(TIMINGS)].isEmpty()) {
        int count = 0;
        bool append = CRHelper::askForAppend(senderObject);
        appendInfoMessage(tr("Timings File: %1").arg(this->fileNames[static_cast<int>(TIMINGS)]));
        try {
            count = CRLoader::importTimings(this->fileNames[static_cast<int>(TIMINGS)], append);
            if (append)
                appendInfoMessage(tr("Appended: %n timing(s)", "", count));
            else
                appendInfoMessage(tr("Loaded: %n timing(s)", "", count));
            lastSelectedPath = QFileInfo(this->fileNames[static_cast<int>(TIMINGS)]).absoluteDir();
        } catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::exportList()
{
    using enum LBChronoRace::fileNameField;

    int tableIndex = -1;
    QString userHint;
    CRLoader::Model model;
    QString message;

    auto senderObject = qobject_cast<ChronoRaceTable *>(sender());

    if (&startListTable == senderObject) {
        tableIndex = static_cast<int>(STARTLIST);
        userHint = tr("Select Competitors");
        model = CRLoader::Model::STARTLIST;
        message = tr("Competitors File saved: %1");
    } else if (&teamsTable == senderObject) {
        tableIndex = static_cast<int>(TEAMS);
        userHint = tr("Select Clubs List");
        model = CRLoader::Model::TEAMSLIST;
        message = tr("Teams File saved: %1");
    } else if (&rankingsTable == senderObject) {
        tableIndex = static_cast<int>(RANKINGS);
        userHint = tr("Select Rankings File");
        model = CRLoader::Model::RANKINGS;
        message = tr("Rankings File saved: %1");
    } else if (&categoriesTable == senderObject) {
        tableIndex = static_cast<int>(CATEGORIES);
        userHint = tr("Select Categories File");
        model = CRLoader::Model::CATEGORIES;
        message = tr("Categories File saved: %1");
    } else if (&timingsTable == senderObject) {
        tableIndex = static_cast<int>(TIMINGS);
        userHint = tr("Select Timings File");
        model = CRLoader::Model::TIMINGS;
        message = tr("Timings File saved: %1");
    } else {
        appendErrorMessage(tr("Error: table to export is unknown"));
        return;
    }

    this->fileNames[tableIndex] = QDir::toNativeSeparators(
        QFileDialog::getSaveFileName(senderObject, userHint,
                                     lastSelectedPath.absolutePath(),
                                     tr("CSV (*.csv)")));

    if (!this->fileNames[tableIndex].isEmpty()) {

        if (!this->fileNames[tableIndex].endsWith(".csv", Qt::CaseInsensitive))
            this->fileNames[tableIndex].append(".csv");

        try {
            CRLoader::exportModel(model, this->fileNames[tableIndex]);
            appendInfoMessage(message.arg(this->fileNames[tableIndex]));
            lastSelectedPath = QFileInfo(this->fileNames[tableIndex]).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(e.getMessage());
        }
    }
}

void LBChronoRace::showTimingRecorder()
{
    if (liveView.getLiveScreen() != Q_NULLPTR) {
        if (QMessageBox::StandardButton reply = (liveView.getLiveScreen() == Q_NULLPTR) ? QMessageBox::StandardButton::Yes :
            QMessageBox::question(this, tr("Opening Race Timings Recorder"), tr("Opening the Race Timings Recorder will create a new Live Rankings table, replacing any existing data. Do you want to proceed?"), QMessageBox::Yes | QMessageBox::No);
            reply != QMessageBox::StandardButton::Yes) {
            return;
        }
    }

    CRSettings::pushFocus(this->focusPolicy());
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    timings.show();
    liveView.toggleTimigns(1 + static_cast<int>(QDialog::DialogCode::Accepted) + static_cast<int>(QDialog::DialogCode::Rejected));
}

void LBChronoRace::hideTimingRecorder(int code)
{
    this->liveView.toggleTimigns(code);

    this->setFocusPolicy(CRSettings::popFocus());
}

void LBChronoRace::initialize()
{
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

    if (auto eType = event->type(); eType == QEvent::KeyPress) {
        auto const keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key::Key_Escape) &&
            (ui->liveViewSelector->currentIndex() > 0)) {
            ui->liveViewSelector->setCurrentIndex(0);
        } else {
            retval = QMainWindow::event(event);
        }
    } else if (eType == QEvent::Close) {
        if (this->checkDirty()) {
            retval = QMainWindow::event(event);
        } else {
            event->ignore();
        }
    } else {
        retval = QMainWindow::event(event);
    }

    return retval;
}

void LBChronoRace::changeEvent(QEvent *event)
{
    if (QEvent::Type type = (event == Q_NULLPTR) ? QEvent::None : event->type();
        type == QEvent::LanguageChange) {
        // this event is send if a translator is loaded
        ui->retranslateUi(this);
    } else if (type == QEvent::LocaleChange) {
        // this event is send, if the system, language changes
        QString locale = QLocale::system().name();
        locale.truncate(locale.lastIndexOf('_'));
        Languages::loadLanguage(locale);
    }

    QMainWindow::changeEvent(event);
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
            QString message;

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
                    [[fallthrough]];
                case LBCHRONORACE_BIN_FMT_v6:
                    QAbstractTableModel const *table;
                    qint16 encodingIdx;
                    qint16 formatIdx;
                    int tableCount;

                    binFormat = static_cast<int>(binFmt);

                    in >> encodingIdx;
                    message = CRLoader::encodingSelector(encodingIdx);
                    appendInfoMessage(message);
                    in >> formatIdx;
                    message = CRLoader::formatSelector(formatIdx);
                    appendInfoMessage(message);
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

bool LBChronoRace::checkDirty()
{
    bool retval = true;

    QMessageBox::StandardButton reply = QMessageBox::StandardButton::No;

    // Ask the user to save data
    if (raceInfo.isDirty() || CRLoader::isDirty())
        reply = QMessageBox::question(this, tr("Unsaved Changes"), tr("You have unsaved changes. Do you want to save them?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    switch (reply) {
        using enum QMessageBox::StandardButton;
        case Yes:
            saveRace();
            retval = !raceDataFileName.isEmpty();
            [[fallthrough]];
        case No:
            // Always store recent races
            RecentRaces::store();
            break;
        default: // any other means Cancel
            retval = false;
    }

    return retval;
}

void LBChronoRace::newRace()
{
    if (!this->checkDirty())
        return;

    try {
        NewRaceWizard newRaceWizard(&raceInfo, this);
        newRaceWizard.exec();
    } catch (ChronoRaceException &e) {
        appendErrorMessage(e.getMessage());
    }
}

void LBChronoRace::loadRace()
{
    if (!this->checkDirty())
        return;

    QString fileName = QDir::toNativeSeparators(
        QFileDialog::getOpenFileName(this, tr("Select Race Data File"),
                                     lastSelectedPath.absolutePath(),
                                     tr("ChronoRace Data (*.crd)")));

    if (loadRaceFile(fileName)) {
        raceDataFileName = fileName;

        ui->errorDisplay->clear();

        // Update recent races list
        RecentRaces::update(raceDataFileName);

        liveView.toggleCompetitors(QDialog::DialogCode::Accepted);
    }
}

void LBChronoRace::openRecentRace(QAction const *action)
{
    if (action == Q_NULLPTR)
        return;

    if (!this->checkDirty())
        return;

    if (auto fileName = action->data().toString(); loadRaceFile(fileName)) {
        raceDataFileName = fileName;

        ui->errorDisplay->clear();

        // Update recent races list
        RecentRaces::update(raceDataFileName);

        liveView.toggleCompetitors(QDialog::DialogCode::Accepted);
    }
}

void LBChronoRace::saveRace()
{
    QString oldRaceDataFileName(raceDataFileName);
    bool saveRaceAs = (ui->actionSaveRaceAs == sender());

    if (saveRaceAs)
        raceDataFileName.clear();

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

            // Update recent races list
            RecentRaces::update(raceDataFileName);
        } else {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
            raceDataFileName.clear();
        }
    }

    if (saveRaceAs && raceDataFileName.isEmpty())
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

void LBChronoRace::setupTrigger()
{
    TriggerKeyDialog addDialog(this);

    addDialog.setModal(true);
    addDialog.exec();
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

        auto const *liveModel = qobject_cast<QStandardItemModel *>(this->ui->liveViewSelector->model());

        if (screen == this->liveView.getLiveScreen()) {
            CRHelper::setScreenSerial(screen->serialNumber());
            liveModel->item(screenIndex)->setEnabled(false);
            this->ui->liveViewSelector->setItemIcon(screenIndex, QIcon(":/material/icons/hide_image.svg"));
            this->ui->liveViewSelector->setItemData(screenIndex, QVariant::fromValue(Q_NULLPTR));
            this->live(screenIndex);
        } else {
            this->ui->liveViewSelector->removeItem(screenIndex);
            ui->liveViewSelector->setEnabled(this->ui->liveViewSelector->count() > 2);
        }
    }
}

void LBChronoRace::screenAdded(QScreen const *screen)
{
    QRegularExpressionMatch match = screenNameRegEx.match(screen->name());
    QString const &screenName = match.hasMatch() ? match.captured(1) : screen->name();

    auto itemCount = this->ui->liveViewSelector->count();
    auto const *liveModel = qobject_cast<QStandardItemModel *>(this->ui->liveViewSelector->model());

    if (auto screenIndex = this->ui->liveViewSelector->findText(screenName); (screenIndex < 0) || (CRHelper::getScreenSerial() != screen->serialNumber())) {
        /* It is either a brand new screen or it could be the screen selected
         * and detached/disappeared but the serial number does not match */
        bool screenEnabled = (screen->size().width() >= 1280);

        this->ui->liveViewSelector->addItem(QIcon(screenEnabled ? ":/material/icons/image.svg" : ":/material/icons/hide_image.svg"), screenName, QVariant::fromValue(screen));
        liveModel->item(itemCount)->setEnabled(screenEnabled);

        if (!screenEnabled)
            appendErrorMessage(tr("Notice:: Live Rankings cannot be activated on screen %1 since %2px wide (min. required width 1280px)").arg(screenName).arg(screen->size().width()));

        itemCount++;
    } else {
        /* It is the screen selected and detached/disappeared */
        this->ui->liveViewSelector->setItemIcon(screenIndex, QIcon(":/material/icons/image.svg"));
        this->ui->liveViewSelector->setItemData(screenIndex, QVariant::fromValue(screen));
        liveModel->item(screenIndex)->setEnabled(true);

        this->live(screenIndex);
    }

    ui->liveViewSelector->setEnabled(itemCount > 2);
}

void LBChronoRace::live(int index)
{
    QScreen const *liveScreen = Q_NULLPTR;

    if (index <= 0) {
        if (liveView.getLiveScreen() != Q_NULLPTR)
            appendInfoMessage(tr("Info: closing the Live Rankings"));

        liveView.setLiveScreen(Q_NULLPTR);
        timings.setLiveTables(Q_NULLPTR);
        screenSaver.inhibit(false);
    } else {
        liveScreen = this->ui->liveViewSelector->currentData().value<QScreen const *>();

        liveView.setLiveScreen(liveScreen);
        timings.setLiveTables(&liveView);
        screenSaver.inhibit(true);
    }

    if ((index <= 0) || (liveScreen != Q_NULLPTR)) {
        auto itemCount = this->ui->liveViewSelector->count();

        while (--itemCount > 0) {
            if (this->ui->liveViewSelector->itemData(itemCount).value<QScreen const *>() == Q_NULLPTR)
                this->ui->liveViewSelector->removeItem(itemCount);
        }

        ui->liveViewSelector->setEnabled(this->ui->liveViewSelector->count() > 2);
    }

    liveView.updateView();
}
