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
    QObject::connect(ui->makeStartList, &QPushButton::clicked, this, &LBChronoRace::makeStartList);
    QObject::connect(ui->makeRankings, &QPushButton::clicked, this, &LBChronoRace::makeRankings);

    QObject::connect(ui->actionLoadRace, &QAction::triggered, this, &LBChronoRace::loadRace);
    QObject::connect(ui->actionSaveRace, &QAction::triggered, this, &LBChronoRace::saveRace);
    QObject::connect(ui->actionSaveRaceAs, &QAction::triggered, this, &LBChronoRace::saveRaceAs);
    QObject::connect(ui->actionQuit, &QAction::triggered, this, &LBChronoRace::actionQuit);
    QObject::connect(ui->actionEditRace, &QAction::triggered, this, &LBChronoRace::editRace);
    QObject::connect(ui->actionEditStartList, &QAction::triggered, this, &LBChronoRace::editStartList);
    QObject::connect(ui->actionEditTeams, &QAction::triggered, this, &LBChronoRace::editTeamsList);
    QObject::connect(ui->actionEditCategories, &QAction::triggered, this, &LBChronoRace::editCategories);
    QObject::connect(ui->actionEditTimings, &QAction::triggered, this, &LBChronoRace::editTimings);
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
            appendInfoMessage((count == 1) ? tr("Loaded: %n category", "", count) : tr("Loaded: %n category", "", count));
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

void LBChronoRace::actionQuit() const
{
    QApplication::quit();
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
                appendInfoMessage((tableCount == 1) ? tr("Loaded: %n category", "", tableCount) : tr("Loaded: %n category", "", tableCount));
                table = CRLoader::getTimingsModel();
                tableCount = table->rowCount();
                setCounterTimings(tableCount);
                appendInfoMessage(tr("Loaded: %n timing(s)", "", tableCount));

                appendInfoMessage(tr("Race loaded: %1").arg(raceDataFileName));
                break;
            default:
                QMessageBox::information(this, tr("Race Data File Error"), tr("Format version %1 not supported").arg(binFmt));
                break;
            }
        }
    }
}

void LBChronoRace::saveRace()
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

void LBChronoRace::saveRaceAs()
{
    raceDataFileName.clear();
    saveRace();
}

void LBChronoRace::editRace()
{
    raceInfo.show();
}

void LBChronoRace::editStartList()
{
    startListTable.show();
}

void LBChronoRace::editTeamsList()
{
    teamsTable.show();
}

void LBChronoRace::editCategories()
{
    categoriesTable.show();
}

void LBChronoRace::editTimings()
{
    timingsTable.show();
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
        QObject::connect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        QObject::connect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);
        printer->setRaceInfo(&raceInfo);

        // print the startlist
        printer->printStartList(startList, this, lastSelectedPath);

        // cleanup
        QObject::disconnect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        QObject::disconnect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);

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
        QObject::connect(&rankingsBuilder, &RankingsBuilder::error, this, &LBChronoRace::appendErrorMessage);

        lastSelectedPath.setPath(rankingsBasePath);

        // compute individual general classifications (all included, sorted by bib)
        uint numberOfCompetitors = rankingsBuilder.loadData();

        // this call can be done only after the rankingsBuilder.loadData() call
        auto sWidth = static_cast<uint>(QString::number(numberOfCompetitors).size());
        auto rWidth = static_cast<uint>(QString::number(categories.size()).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(format, sWidth, bWidth);
        QObject::connect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        QObject::connect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);
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

        QObject::disconnect(&rankingsBuilder, &RankingsBuilder::error, this, &LBChronoRace::appendErrorMessage);
        QObject::disconnect(printer.data(), &RankingPrinter::info, this, &LBChronoRace::appendInfoMessage);
        QObject::disconnect(printer.data(), &RankingPrinter::error, this, &LBChronoRace::appendErrorMessage);
    } catch (ChronoRaceException &e) {
        appendErrorMessage(tr("Error: %1").arg(e.getMessage()));
    }
}

void LBChronoRace::actionAbout()
{
    QMessageBox::about(this, tr("Informations"), tr("\n%1\n\nAuthor: Lorenzo Buzzi (lorenzo.buzzi@gmail.com)\n\nVersion: %2\n").arg(LBCHRONORACE_NAME, LBCHRONORACE_VERSION));
}

void LBChronoRace::actionAboutQt()
{
    QMessageBox::aboutQt(this, tr("About &Qt"));
}
