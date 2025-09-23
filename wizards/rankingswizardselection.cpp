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

#include <QPointer>
#include <QListWidget>
#include <QComboBox>

#include "crloader.hpp"
#include "wizards/rankingswizard.hpp"
#include "wizards/rankingswizardselection.hpp"

RankingsWizardSelection::RankingsWizardSelection(QWidget *parent) :
    QWizardPage(parent),
    layout(parent)
{
    Qt::ItemFlags flags;
    QListWidgetItem *item;
    uint i = 0;

    QPointer<QWidget> widgetPtr(Q_NULLPTR);

    QList<Ranking> const &rankings = CRLoader::getRankings();

    setTitle(tr("Rankings"));
    setSubTitle(tr("You can exclude some of the generated rankings."));

    widgetPtr = new QListWidget;
    layout.addRow(new QLabel(tr("Rankings")), widgetPtr.data());
    QListWidget *rankingsList = qobject_cast<QListWidget *>(layout.itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    for (auto const &ranking : rankings) {
        rankingsList->addItem(ranking.getFullDescription());
        item = rankingsList->item(i);

        flags = item->flags();
        flags |= Qt::ItemIsUserCheckable;
        item->setFlags(flags);

        i++;
    }
    widgetPtr = new QComboBox;
    layout.addRow(new QLabel(tr("Results")), widgetPtr.data());
    registerField("selection.results", widgetPtr.data());
    widgetPtr = Q_NULLPTR;

    setLayout(&layout);
}

void RankingsWizardSelection::initializePage()
{
    RankingsWizard *rankingsWizard = qobject_cast<RankingsWizard *>(wizard());

    QList<RankingsWizardSelection::RankingsWizardItem> *rankings = rankingsWizard->getRankingsList();
    QListWidget const *rankingsList = qobject_cast<QListWidget *>(layout.itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    for (uint i = 0; i < rankings->count(); i++) {
        rankingsList->item(i)->setCheckState((*rankings)[i].skip ? Qt::Unchecked : Qt::Checked);
    }
    QObject::connect(rankingsList, &QListWidget::itemChanged, this, &RankingsWizardSelection::toggleSkipRanking);

    ChronoRaceData *raceData = rankingsWizard->getRaceData();
    QComboBox *resultsMode = qobject_cast<QComboBox *>(layout.itemAt(1, QFormLayout::ItemRole::FieldRole)->widget());
    resultsMode->addItems(raceData->getFieldValues(ChronoRaceData::IndexField::RESULTS));
    resultsMode->setCurrentIndex(raceData->getFieldIndex(ChronoRaceData::IndexField::RESULTS));
}

void RankingsWizardSelection::toggleSkipRanking(QListWidgetItem const *item) const
{
    QListWidget const *rankingsList = qobject_cast<QListWidget *>(layout.itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    (*(qobject_cast<RankingsWizard *>(wizard())->getRankingsList()))[rankingsList->row(item)].skip = (item->checkState() != Qt::Checked);
}

