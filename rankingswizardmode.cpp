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

#include "rankingswizard.hpp"
#include "rankingswizardmode.hpp"

RankingsWizardMode::RankingsWizardMode(QWidget *parent) :
    QWizardPage(parent),
    singleFile(parent),
    multiFile(parent)
{
    setTitle(tr("Single/multiple files"));
    setSubTitle(tr("How many PDFs do you want?"));

    singleFile.setText(tr("One single file containing all rankings"));
    singleFile.setChecked(true);
    layout.addWidget(&singleFile);

    multiFile.setText(tr("A separate file for each ranking"));
    multiFile.setChecked(false);
    layout.addWidget(&multiFile);

    setLayout(&layout);

    QObject::connect(&singleFile, &QRadioButton::toggled, this, &RankingsWizardMode::toggleSingleMode);
}

void RankingsWizardMode::toggleSingleMode(bool checked) const
{
    RankingsWizard *parentWizard;

    if ((parentWizard = qobject_cast<RankingsWizard *>(wizard()))) {
        parentWizard->setPdfSingleMode(checked);
    }
}
