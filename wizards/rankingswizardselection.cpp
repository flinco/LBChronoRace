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

    QList<Ranking> const &rankings = CRLoader::getRankings();

    setTitle(tr("Rankings"));
    setSubTitle(tr("You can exclude some of the generated rankings."));

    for (auto const &ranking : rankings) {
        rankingsList.addItem(ranking.getFullDescription());
        item = rankingsList.item(i);

        flags = item->flags();
        flags |= Qt::ItemIsUserCheckable;
        item->setFlags(flags);

        i++;
    }

    layout.addWidget(&rankingsList);

    setLayout(&layout);
}

void RankingsWizardSelection::initializePage()
{
    QList<RankingsWizardSelection::RankingsWizardItem> *rankings = (qobject_cast<RankingsWizard *>(wizard()))->getRankingsList();

    for (uint i = 0; i < rankings->count(); i++) {
        rankingsList.item(i)->setCheckState((*rankings)[i].skip ? Qt::Unchecked : Qt::Checked);
    }

    QObject::connect(&rankingsList, &QListWidget::itemChanged, this, &RankingsWizardSelection::toggleSkipRanking);
}

void RankingsWizardSelection::cleanupPage()
{
    QObject::disconnect(&rankingsList, &QListWidget::itemChanged, this, &RankingsWizardSelection::toggleSkipRanking);
}

void RankingsWizardSelection::toggleSkipRanking(QListWidgetItem const *item) const
{
    (*(qobject_cast<RankingsWizard *>(wizard())->getRankingsList()))[rankingsList.row(item)].skip = (item->checkState() != Qt::Checked);
}

