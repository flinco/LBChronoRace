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

#include "crloader.h"
#include "rankingswizard.h"
#include "rankingswizardselection.h"

RankingsWizardSelection::RankingsWizardSelection(QWidget *parent) :
    QWizardPage(parent),
    layout(parent)
{
    Qt::ItemFlags flags;
    QListWidgetItem *item;
    uint i = 0;

    QVector<Category> categories = CRLoader::getCategories();

    setTitle(tr("Rankings"));
    setSubTitle("You can exclude some categories from the generated rankings.");

    for (auto const &category : categories) {
        categoriesList.addItem(category.getFullDescription());
        item = categoriesList.item(i);

        flags = item->flags();
        flags |= Qt::ItemIsUserCheckable;
        item->setFlags(flags);

        i++;
    }

    layout.addWidget(&categoriesList);

    setLayout(&layout);
}

void RankingsWizardSelection::initializePage()
{
    QList<RankingsWizardSelection::RankingsWizardItem> *rankingsList = (qobject_cast<RankingsWizard *>(wizard()))->getRankingsList();

    for (uint i = 0; i < rankingsList->size(); i++) {
        categoriesList.item(i)->setCheckState((*rankingsList)[i].skip ? Qt::Unchecked : Qt::Checked);
    }

    QObject::connect(&categoriesList, &QListWidget::itemChanged, this, &RankingsWizardSelection::toggleSkipRanking);
}

void RankingsWizardSelection::cleanupPage()
{
    QObject::disconnect(&categoriesList, &QListWidget::itemChanged, this, &RankingsWizardSelection::toggleSkipRanking);
}

void RankingsWizardSelection::toggleSkipRanking(QListWidgetItem const *item) const
{
    (*(qobject_cast<RankingsWizard *>(wizard())->getRankingsList()))[categoriesList.row(item)].skip = (item->checkState() != Qt::Checked);
}

