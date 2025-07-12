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

#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

#include "crloader.hpp"
#include "lbcrexception.hpp"
#include "wizards/rankingswizard.hpp"
#include "rankingprinter.hpp"

RankingsWizard::RankingsWizard(ChronoRaceData *data, QDir *path, RankingsWizardTarget target) :
    QWizard(Q_NULLPTR),
    raceData(data),
    lastSelectedPath(path),
    formatPage(this),
    modePage(this),
    selectionPage(this),
    target(target)
{
    //NOSONAR setWindowFlags(Qt::Dialog | Qt::WindowTitleHint);
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    QObject::connect(&rankingsBuilder, &RankingsBuilder::error, this, &RankingsWizard::storeErrorMessage);

    switch (this->target) {
        using enum RankingsWizard::RankingsWizardTarget;

        case Rankings:
            buildRankings();
            break;
        case StartList:
            buildStartList();
            break;
        default:
            Q_UNREACHABLE();
            break;
    }

    QObject::connect(&formatPage, &RankingsWizardFormat::error, this, &RankingsWizard::forwardErrorMessage);
    QObject::connect(&formatPage, &RankingsWizardFormat::notifyOpenChange, this, &RankingsWizard::setOpenFileAtEnd);
    QObject::connect(&modePage, &RankingsWizardMode::error, this, &RankingsWizard::forwardErrorMessage);
    QObject::connect(&selectionPage, &RankingsWizardSelection::error, this, &RankingsWizard::forwardErrorMessage);

    setPage(static_cast<int>(RankingsWizardPage::Page_Format), &formatPage);
    setPage(static_cast<int>(RankingsWizardPage::Page_Mode), &modePage);
    setPage(static_cast<int>(RankingsWizardPage::Page_Selection), &selectionPage);

    QObject::connect(this->button(QWizard::FinishButton), &QAbstractButton::clicked, this, &RankingsWizard::print);

    //setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo.png"));

    setWindowTitle(tr("Rankings Wizard"));
    setStartId(static_cast<int>(RankingsWizardPage::Page_Format));
}

bool RankingsWizard::getPdfSingleMode() const
{
    return pdfSingleMode;
}

void RankingsWizard::setPdfSingleMode(bool newPdfSingleMode)
{
    pdfSingleMode = newPdfSingleMode;
}

QList<RankingsWizardSelection::RankingsWizardItem> *RankingsWizard::getRankingsList()
{
    return &rankingsList;
}

RankingsWizard::RankingsWizardTarget RankingsWizard::getTarget() const
{
    return target;
}

void RankingsWizard::setTarget(RankingsWizard::RankingsWizardTarget newTarget)
{
    target = newTarget;
}

void RankingsWizard::buildStartList()
{
    // compute the startlist
    rankingsBuilder.loadData();
}

void RankingsWizard::buildRankings()
{
    uint i = 0;

    QList<Ranking> const &rankings = CRLoader::getRankings();

    // compute individual general classifications (all included, sorted by bib)
    numberOfCompetitors = rankingsBuilder.loadData();

    rankingsList.resize(rankings.count());
    for (auto &rankingItem : rankingsList) {
        rankingItem.categories = &rankings.at(i);
        i++;
    }
}

void RankingsWizard::setOpenFileAtEnd(bool open)
{
    openFileAtEnd = open;
}

void RankingsWizard::forwardInfoMessage(QString const &message)
{
    emit info(message);
}

void RankingsWizard::forwardErrorMessage(QString const &message)
{
    emit error(message);
}

void RankingsWizard::storeErrorMessage(QString const &message)
{
    if (!message.isEmpty())
        messages.append(message);
}


void RankingsWizard::printStartList()
{
    try {
        CRLoader::Format format = CRLoader::getFormat();

        // this call can be done only after the rankingsBuilder.loadData() call
        QList<Competitor const *> startList = rankingsBuilder.fillStartList();

        auto sWidth = static_cast<uint>(QString::number(startList.size()).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(format, sWidth, bWidth);
        printer->setRaceInfo(raceData);

        QString startListFileName = QFileDialog::getSaveFileName(this, tr("Select Start List File"),
                                                           lastSelectedPath->absolutePath(),
                                                           printer->getFileFilter());

        if (!startListFileName.isEmpty()) {
            auto const &infoMessages = QObject::connect(printer.data(), &RankingPrinter::info, this, &RankingsWizard::forwardInfoMessage);
            auto const &errorMessages = QObject::connect(printer.data(), &RankingPrinter::error, this, &RankingsWizard::forwardErrorMessage);

            printer->init(&startListFileName, raceData->getField(ChronoRaceData::StringField::EVENT), tr("Start List"));

            // print the startlist
            printer->printStartList(startList);

            if (printer->finalize()) {
                QFileInfo outFileInfo(startListFileName);
                QString outFilePath = QDir::toNativeSeparators(outFileInfo.absoluteFilePath());
                emit info(tr("Generated Start List: %1").arg(outFilePath));
                lastSelectedPath->setPath(outFileInfo.absoluteDir().absolutePath());

                if (this->openFileAtEnd) {
                    emit info(tr("Trying to open: %1").arg(outFilePath));
                    QDesktopServices::openUrl(QUrl::fromLocalFile(outFilePath));
                }
            }

            // cleanup
            QObject::disconnect(infoMessages);
            QObject::disconnect(errorMessages);
        }
    } catch (ChronoRaceException &e) {
        emit error(e.getMessage());
    }
}

void RankingsWizard::printRankingsSingleFile()
{
    try {
        // this call can be done only after the rankingsBuilder.loadData() call
        auto sWidth = static_cast<uint>(QString::number(numberOfCompetitors).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(CRLoader::getFormat(), sWidth, bWidth);
        printer->setRaceInfo(raceData);

        QString rankingsFileName = QFileDialog::getSaveFileName(this, tr("Select Results Destination File"),
                                                                lastSelectedPath->absolutePath(),
                                                                printer->getFileFilter());

        if (rankingsFileName.isEmpty())
            throw(ChronoRaceException(tr("Warning: please select a destination file name")));

        auto const &printerInfoMessages = QObject::connect(printer.data(), &RankingPrinter::info, this, &RankingsWizard::forwardInfoMessage);
        auto const &printerErrorMessages = QObject::connect(printer.data(), &RankingPrinter::error, this, &RankingsWizard::forwardErrorMessage);

        printer->init(&rankingsFileName, raceData->getField(ChronoRaceData::StringField::EVENT), tr("Results"));

        // now print each ranking
        for (auto &rankingItem : rankingsList) {
            if (rankingItem.skip)
                continue;

            if (rankingItem.categories->isTeam()) {
                // build the ranking
                rankingsBuilder.fillRanking(rankingItem.teamRanking, rankingItem.categories);
                // print the team ranking
                printer->printRanking(*rankingItem.categories, rankingItem.teamRanking);
            } else {
                // build the ranking
                rankingsBuilder.fillRanking(rankingItem.ranking, rankingItem.categories);
                // print the individual ranking
                printer->printRanking(*rankingItem.categories, rankingItem.ranking);
            }
        }

        if (printer->finalize()) {
            QFileInfo outFileInfo(rankingsFileName);
            QString outFilePath = QDir::toNativeSeparators(outFileInfo.absoluteFilePath());
            emit info(tr("Generated Results: %1").arg(outFilePath));
            lastSelectedPath->setPath(outFileInfo.absoluteDir().absolutePath());

            if (this->openFileAtEnd) {
                emit info(tr("Trying to open: %1").arg(outFilePath));
                QDesktopServices::openUrl(QUrl::fromLocalFile(outFilePath));
            }
        }

        // cleanup
        QObject::disconnect(printerErrorMessages);
        QObject::disconnect(printerInfoMessages);
    } catch (ChronoRaceException &e) {
        emit error(e.getMessage());
    }
}

void RankingsWizard::printRankingsMultiFile()
{
    try {
        // this call can be done only after the rankingsBuilder.loadData() call
        auto sWidth = static_cast<uint>(QString::number(numberOfCompetitors).size());
        auto rWidth = static_cast<uint>(QString::number(rankingsList.size()).size());
        auto bWidth = static_cast<uint>(QString::number(CRLoader::getStartListBibMax()).size());

        // get a ranking printer
        QScopedPointer<RankingPrinter> printer = RankingPrinter::getRankingPrinter(CRLoader::getFormat(), sWidth, bWidth);
        printer->setRaceInfo(raceData);

        QString rankingsBasePath = QFileDialog::getExistingDirectory(this, tr("Select Results Destination Folder"),
                                                                     lastSelectedPath->absolutePath(),
                                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        if (rankingsBasePath.isEmpty())
            throw(ChronoRaceException(tr("Warning: please select a destination folder")));

        auto const &printerInfoMessages = QObject::connect(printer.data(), &RankingPrinter::info, this, &RankingsWizard::forwardInfoMessage);
        auto const &printerErrorMessages = QObject::connect(printer.data(), &RankingPrinter::error, this, &RankingsWizard::forwardErrorMessage);

        lastSelectedPath->setPath(rankingsBasePath);

        // now print each ranking
        uint k = 0;
        QString outFileBaseName;
        for (auto &rankingItem : rankingsList) {
            k++;

            if (rankingItem.skip)
                continue;

            outFileBaseName = QDir(rankingsBasePath).filePath(QString("class%1_%2").arg(k, rWidth, 10, QChar('0')).arg(rankingItem.categories->getShortDescription()));
            printer->init(&outFileBaseName, raceData->getField(ChronoRaceData::StringField::EVENT), tr("Results") + " - " + rankingItem.categories->getFullDescription());

            if (rankingItem.categories->isTeam()) {
                // build the ranking
                rankingsBuilder.fillRanking(rankingItem.teamRanking, rankingItem.categories);
                // print the team ranking
                printer->printRanking(*rankingItem.categories, rankingItem.teamRanking);
            } else {
                // build the ranking
                rankingsBuilder.fillRanking(rankingItem.ranking, rankingItem.categories);
                // print the individual ranking
                printer->printRanking(*rankingItem.categories, rankingItem.ranking);
            }

            if (printer->finalize()) {
                QFileInfo outFileInfo(outFileBaseName);
                QString outFilePath = QDir::toNativeSeparators(outFileInfo.absoluteFilePath());
                emit info(tr("Generated Results '%1': %2").arg(rankingItem.categories->getFullDescription(), outFilePath));

                if (this->openFileAtEnd) {
                    emit info(tr("Trying to open: %1").arg(outFilePath));
                    QDesktopServices::openUrl(QUrl::fromLocalFile(outFilePath));
                }
            }
        }

        // cleanup
        QObject::disconnect(printerErrorMessages);
        QObject::disconnect(printerInfoMessages);
    } catch (ChronoRaceException &e) {
        emit error(e.getMessage());
    }
}

void RankingsWizard::print(bool checked)
{
    Q_UNUSED(checked)

    for (auto const &message : std::as_const(messages))
        if (!message.isEmpty())
            emit error(message);
    messages.clear();

    switch (this->target) {
        using enum RankingsWizard::RankingsWizardTarget;

        case StartList:
            printStartList();
            break;
        case Rankings:
            if ((CRLoader::getFormat() != CRLoader::Format::PDF) || (pdfSingleMode == false))
                printRankingsMultiFile();
            else
                printRankingsSingleFile();
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}
