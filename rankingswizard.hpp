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

#ifndef RANKINGSWIZARD_H
#define RANKINGSWIZARD_H

#include <QDir>
#include <QWizard>
#include <QList>

#include "chronoracedata.hpp"
#include "rankingsbuilder.hpp"
#include "rankingswizardformat.hpp"
#include "rankingswizardmode.hpp"
#include "rankingswizardselection.hpp"

class RankingsWizard : public QWizard
{
    Q_OBJECT

public:
    enum class RankingsWizardTarget
    {
        StartList,
        Rankings
    };

    enum class RankingsWizardPage
    {
        Page_Format = 0,
        Page_Mode = 1,
        Page_Selection = 2
    };

    explicit RankingsWizard(ChronoRaceData *data, QDir *path, RankingsWizardTarget target = RankingsWizardTarget::Rankings);

    bool getPdfSingleMode() const;
    void setPdfSingleMode(bool newPdfSingleMode);
    QList<RankingsWizardSelection::RankingsWizardItem> *getRankingsList();
    RankingsWizardTarget getTarget() const;
    void setTarget(RankingsWizardTarget newTarget);

private:
    ChronoRaceData *raceData;
    QDir *lastSelectedPath;

    uint numberOfCompetitors { 0 };

    RankingsBuilder rankingsBuilder;

    RankingsWizardFormat formatPage;
    RankingsWizardMode modePage;
    RankingsWizardSelection selectionPage;

    bool pdfSingleMode { true };
    QList<RankingsWizardSelection::RankingsWizardItem> rankingsList { };
    RankingsWizardTarget target { RankingsWizardTarget::Rankings };

    void buildStartList();
    void buildRankings();

    void printStartList();
    void printRankingsSingleFile();
    void printRankingsMultiFile();

    bool openFileAtEnd { true };

    QStringList messages { };

private slots:
    void setOpenFileAtEnd(bool open);
    void forwardInfoMessage(QString const &message);
    void forwardErrorMessage(QString const &message);
    void storeErrorMessage(QString const &message);
    void print(bool checked);

signals:
    void info(QString const &);
    void error(QString const &);
};

#endif // RANKINGSWIZARD_H
