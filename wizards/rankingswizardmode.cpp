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

#include <QRadioButton>

#include "wizards/rankingswizardmode.hpp"

RankingsWizardMode::RankingsWizardMode(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Single/multiple files"));
    setSubTitle(tr("How many PDFs do you want?"));

    layout.addWidget(new QRadioButton(tr("One single file containing all rankings")));
    QRadioButton *singleFile = qobject_cast<QRadioButton *>(layout.itemAt(0)->widget());
    singleFile->setChecked(true);

    layout.addWidget(new QRadioButton(tr("A separate file for each ranking")));
    QRadioButton *multiFile = qobject_cast<QRadioButton *>(layout.itemAt(1)->widget());
    multiFile->setChecked(false);
    registerField("mode.multiFile", multiFile);

    setLayout(&layout);
}
