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
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QScreen>
#include <QScopedPointer>

#include <QDebug>

#include "lbchronorace.h"
#include "crloader.h"
#include "competitor.h"
#include "classentry.h"
#include "teamclassentry.h"
#include "crloader.h"
#include "rankingsbuilder.h"
#include "rankingprinter.h"
#include "lbcrexception.h"

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

    QObject::connect(&timings, &ChronoRaceTimings::newTimingsCount, this, &LBChronoRace::setCounterTimings);
    QObject::connect(&timings, &ChronoRaceTimings::error, this, &LBChronoRace::appendErrorMessage);

    // react to screen change and resize
    QObject::connect(app, &QGuiApplication::primaryScreenChanged, this, &LBChronoRace::resizeDialogs);
    resizeDialogs(QGuiApplication::primaryScreen());

    //NOSONAR ui->makeRankingsPDF->setEnabled(false);

    QObject::connect(ui->loadRace, &QPushButton::clicked, this, &LBChronoRace::loadRace);
    QObject::connect(ui->saveRace, &QPushButton::clicked, this, &LBChronoRace::saveRace);
    QObject::connect(ui->editRace, &QPushButton::clicked, &raceInfo, &ChronoRaceData::show);
    QObject::connect(ui->editStartList, &QPushButton::clicked, &startListTable, &ChronoRaceTable::show);
    QObject::connect(ui->editTeamsList, &QPushButton::clicked, &teamsTable, &ChronoRaceTable::show);
    QObject::connect(ui->editCategories, &QPushButton::clicked, &categoriesTable, &ChronoRaceTable::show);
    QObject::connect(ui->editTimings, &QPushButton::clicked, &timingsTable, &ChronoRaceTable::show);
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
    QObject::connect(ui->actionEditCategories, &QAction::triggered, &categoriesTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionEditTimings, &QAction::triggered, &timingsTable, &ChronoRaceTable::show);
    QObject::connect(ui->actionAbout, &QAction::triggered, this, &LBChronoRace::actionAbout);
    QObject::connect(ui->actionAboutQt, &QAction::triggered, this, &LBChronoRace::actionAboutQt);

    QObject::connect(ui->selectorEncoding, &QComboBox::currentTextChanged, this, &LBChronoRace::selectorEncoding);
    QObject::connect(ui->selectorFormat, &QComboBox::currentTextChanged, this, &LBChronoRace::selectorFormat);
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
        } catch (ChronoRaceException &e) {
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
            appendInfoMessage(tr("Loaded: %n category(s)", "", count));
            lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();
        } catch (ChronoRaceException &e) {
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
        } catch (ChronoRaceException &e) {
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

        if (!startListFileName.endsWith(".csv", Qt::CaseInsensitive))
            startListFileName.append(".csv");

        try {
            CRLoader::exportStartList(startListFileName);
            appendInfoMessage(tr("Start List File saved: %1").arg(startListFileName));
            lastSelectedPath = QFileInfo(startListFileName).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTeamList()
{
    teamsFileName = QFileDialog::getSaveFileName(this, tr("Select Teams List"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!teamsFileName.isEmpty()) {

        if (!teamsFileName.endsWith(".csv", Qt::CaseInsensitive))
            teamsFileName.append(".csv");

        try {
            CRLoader::exportTeams(teamsFileName);
            appendInfoMessage(tr("Teams File saved: %1").arg(teamsFileName));
            lastSelectedPath = QFileInfo(teamsFileName).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportCategoriesList()
{
    categoriesFileName = QFileDialog::getSaveFileName(this, tr("Select Categories File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!categoriesFileName.isEmpty()) {

        if (!categoriesFileName.endsWith(".csv", Qt::CaseInsensitive))
            categoriesFileName.append(".csv");

        try {
            CRLoader::exportCategories(categoriesFileName);
            appendInfoMessage(tr("Categories File saved: %1").arg(categoriesFileName));
            lastSelectedPath = QFileInfo(categoriesFileName).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
        }
    }
}

void LBChronoRace::exportTimingsList()
{
    timingsFileName = QFileDialog::getSaveFileName(this, tr("Select Timings File"),
        lastSelectedPath.absolutePath(), tr("CSV (*.csv)"));

    if (!timingsFileName.isEmpty()) {

        if (!timingsFileName.endsWith(".csv", Qt::CaseInsensitive))
            timingsFileName.append(".csv");

        try {
            CRLoader::exportTimings(timingsFileName);
            appendInfoMessage(tr("Timings File saved: %1").arg(timingsFileName));
            lastSelectedPath = QFileInfo(timingsFileName).absoluteDir();
        }  catch (ChronoRaceException &e) {
            appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
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

bool LBChronoRace::loadRaceFile(QString const &fileName)
{
    bool retval = false;

    if (!fileName.isEmpty()) {
        QFile raceDataFile(fileName);
        lastSelectedPath = QFileInfo(fileName).absoluteDir();

        if (raceDataFile.open(QIODevice::ReadOnly)) {
            quint32 binFmt;
            QFileInfo raceDataFileInfo(fileName);

            QDataStream in(&raceDataFile);
            in.setVersion(QDataStream::Qt_5_15);
            in >> binFmt;

            switch (binFmt) {
            case LBCHRONORACE_BIN_FMT_v1:
            case LBCHRONORACE_BIN_FMT_v2:
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
                appendInfoMessage(tr("Loaded: %n category(s)", "", tableCount));
                table = CRLoader::getTimingsModel();
                tableCount = table->rowCount();
                setCounterTimings(tableCount);
                appendInfoMessage(tr("Loaded: %n timing(s)", "", tableCount));

                appendInfoMessage(tr("Race loaded: %1").arg(fileName));
                retval = true;
                break;
            default:
                QMessageBox::information(this, tr("Race Data File Error"), tr("Format version %1 not supported").arg(binFmt));
                break;
            }
        } else {
            QMessageBox::information(this, tr("Unable to open file"), raceDataFile.errorString());
        }
    }

    return retval;
}

void LBChronoRace::loadRace()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Race Data File"),
                                                    lastSelectedPath.absolutePath(),
                                                    tr("ChronoRace Data (*.crd)"));

    if (loadRaceFile(fileName))
        raceDataFileName = fileName;
}

void LBChronoRace::saveRace()
{
    if (raceDataFileName.isEmpty()) {
        raceDataFileName = QFileDialog::getSaveFileName(this, tr("Select Race Data File"),
            lastSelectedPath.absolutePath(), tr("ChronoRace Data (*.crd)"));

        if (!raceDataFileName.isEmpty() && !raceDataFileName.endsWith(".crd", Qt::CaseInsensitive))
            raceDataFileName.append(".crd");
    }

    if (!raceDataFileName.isEmpty()) {
        QFile raceDataFile(raceDataFileName);

        if (raceDataFile.open(QIODevice::WriteOnly)) {
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

void LBChronoRace::makeStartList()
{
    //NOSONAR ui->errorDisplay->clear();

    try {
        CRLoader::Format format = CRLoader::getFormat();

        // compute start list
        QList<Competitor> startList = RankingsBuilder::makeStartList();

        auto sWidth = static_cast<uint>(QString::number(startList.size()).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(format, sWidth, bWidth);
        auto const &infoMessages = QObject::connect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        auto const &errorMessages = QObject::connect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);
        printer->setRaceInfo(&raceInfo);

        // print the startlist
        printer->printStartList(startList, this, lastSelectedPath);

        // cleanup
        QObject::disconnect(infoMessages);
        QObject::disconnect(errorMessages);

    } catch (ChronoRaceException &e) {
        appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
    }
}

void LBChronoRace::makeRankings()
{
    //NOSONAR ui->errorDisplay->clear();

    try {
        CRLoader::Format format = CRLoader::getFormat();

        QString rankingsBasePath = QFileDialog::getExistingDirectory(this,
            tr("Select Results Destination Folder"), lastSelectedPath.absolutePath(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (rankingsBasePath.isEmpty())
            throw(ChronoRaceException(tr("Warning: please select a destination folder")));

        QVector<Category> categories = CRLoader::getCategories();

        RankingsBuilder rankingsBuilder;
        auto const &builderErrorMessages = QObject::connect(&rankingsBuilder, &RankingsBuilder::error, this, &LBChronoRace::appendErrorMessage);

        lastSelectedPath.setPath(rankingsBasePath);

        // compute individual general classifications (all included, sorted by bib)
        uint numberOfCompetitors = rankingsBuilder.loadData();

        // this call can be done only after the rankingsBuilder.loadData() call
        auto sWidth = static_cast<uint>(QString::number(numberOfCompetitors).size());
        auto rWidth = static_cast<uint>(QString::number(categories.size()).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(format, sWidth, bWidth);
        auto const &printerInfoMessages = QObject::connect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        auto const &printerErrorMessages = QObject::connect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);
        printer->setRaceInfo(&raceInfo);

        // now print each ranking
        uint k = 0;
        QString outFileBaseName;
        for (auto const &category : categories) {
            k++;
            outFileBaseName = QDir(rankingsBasePath).filePath(QString("class%1_%2").arg(k, rWidth, 10, QChar('0')).arg(category.getShortDescription()));
            if (category.isTeam()) {
                // make and print the team ranking
                QList<TeamClassEntry const *> ranking;
                printer->printRanking(category, rankingsBuilder.fillRanking(ranking, category), outFileBaseName);
            } else {
                // make and print the individual ranking
                QList<ClassEntry const *> ranking;
                printer->printRanking(category, rankingsBuilder.fillRanking(ranking, category), outFileBaseName);
            }
        }

        QObject::disconnect(printerErrorMessages);
        QObject::disconnect(printerInfoMessages);
        QObject::disconnect(builderErrorMessages);
    } catch (ChronoRaceException &e) {
        appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
    }
}

void LBChronoRace::actionAbout()
{
    /* The purpose of the following object is to prevent the "About Qt"  *
     * translation from being removed from the .tr files when refreshed. */
    QString const translatedTextAboutQtMessage = QMessageBox::tr(
        "<p>Qt is a C++ toolkit for cross-platform application development.</p>"
        "<p>Qt provides single-source portability across all major desktop operating systems. "
        "It is also available for embedded Linux and other embedded and mobile operating systems.</p>"
        "<p>Qt is available under multiple licensing options designed to accommodate the needs of our various users.</p>"
        "<p>Qt licensed under our commercial license agreement is appropriate for development of "
        "proprietary/commercial software where you do not want to share any source code with "
        "third parties or otherwise cannot comply with the terms of GNU (L)GPL.</p>"
        "<p>Qt licensed under GNU (L)GPL is appropriate for the development of Qt&nbsp;applications "
        "provided you can comply with the terms and conditions of the respective licenses.</p>"
        "<p>Please see <a href=\"http://%2/\">%2</a> for an overview of Qt licensing.</p>"
        "<p>Copyright (C) %1 The Qt Company Ltd and other contributors.</p>"
        "<p>Qt and the Qt logo are trademarks of The Qt Company Ltd.</p>"
        "<p>Qt is The Qt Company Ltd product developed as an open source project. "
        "See <a href=\"http://%3/\">%3</a> for more information.</p>"
        );
    std::ignore = translatedTextAboutQtMessage;

    QString const translatedTextAboutQtCaption = QMessageBox::tr(
        "<h3>About %1</h3>"
        "<p>Software for producing the results of footraces.</p>"
        ).arg(QStringLiteral(LBCHRONORACE_NAME));
    QString const translatedTextAboutQtText = QMessageBox::tr(
        "<p>Copyright&copy; 2021-2022</p>"
        "<p>Version: %1 (source code on <a href=\"http://github.com/flinco/LBChronoRace\">GitHub</a>)</p>"
        "<p>Author: Lorenzo Buzzi (<a href=\"mailto:lorenzo@buzzi.pro\">lorenzo@buzzi.pro</a>)</p>"
        "<p>Site: <a href=\"http://www.buzzi.pro/\">http://www.buzzi.pro/</a></p>"
        "<p>%2 is free software: you can redistribute it and/or modify it under the terms of "
        "the GNU General Public License as published by the Free Software Foundation; either "
        "version 3 of the License, or (at your option) any later version.</p>"
        "<p>%2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; "
        "without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
        "See the GNU General Public License for more details.</p>"
        "<p>You should have received a copy of the GNU General Public License along with %2. "
        "If not, see: <a href=\"https://www.gnu.org/licenses/\">https://www.gnu.org/licenses/</a>.</p>"
        ).arg(QStringLiteral(LBCHRONORACE_VERSION), QStringLiteral(LBCHRONORACE_NAME));
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("About %1").arg(QStringLiteral(LBCHRONORACE_NAME)));
    msgBox.setText(translatedTextAboutQtCaption);
    msgBox.setInformativeText(translatedTextAboutQtText);
    if (QPixmap pm(QStringLiteral(":/icons/LBChronoRace.png")); !pm.isNull()) {
        msgBox.setIconPixmap(pm);
    }
    msgBox.exec();
}

void LBChronoRace::actionAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}
