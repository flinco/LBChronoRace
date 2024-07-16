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

#include <QLabel>

#include "crloader.hpp"
#include "rankingswizard.hpp"
#include "rankingswizardformat.hpp"
#include "crhelper.hpp"

RankingsWizardFormat::RankingsWizardFormat(QWidget *parent) :
    QWizardPage(parent),
    fileFormat(parent),
    fileEncoding(parent)
{
    int formatIdx;
    int encodingIdx;

    formatIdx = static_cast<int>(CRLoader::getFormat());
    fileFormat.insertItem(static_cast<int>(CRLoader::Format::PDF), CRHelper::formatToLabel(CRLoader::Format::PDF));
    fileFormat.insertItem(static_cast<int>(CRLoader::Format::TEXT), CRHelper::formatToLabel(CRLoader::Format::TEXT));
    fileFormat.insertItem(static_cast<int>(CRLoader::Format::CSV), CRHelper::formatToLabel(CRLoader::Format::CSV));
    fileFormat.setCurrentIndex(formatIdx);
    layout.addRow(new QLabel(tr("Format")), &fileFormat);

    encodingIdx = static_cast<int>(CRLoader::getEncoding());
    fileEncoding.insertItem(static_cast<int>(CRLoader::Encoding::UTF8), CRHelper::encodingToLabel(CRLoader::Encoding::UTF8));
    fileEncoding.insertItem(static_cast<int>(CRLoader::Encoding::LATIN1), CRHelper::encodingToLabel(CRLoader::Encoding::LATIN1));
    fileEncoding.setCurrentIndex(encodingIdx);
    layout.addRow(new QLabel(tr("Encoding")), &fileEncoding);

    formatChange(formatIdx);
    connect(&fileFormat, &QComboBox::currentIndexChanged, this, &RankingsWizardFormat::formatChange);
    connect(&fileEncoding, &QComboBox::currentIndexChanged, this, &RankingsWizardFormat::encodingChange);

    setLayout(&layout);
}

void RankingsWizardFormat::initializePage()
{
    if (RankingsWizard const *parentWizard = qobject_cast<RankingsWizard *>(wizard());
        parentWizard && (parentWizard->getTarget() == RankingsWizard::RankingsWizardTarget::StartList))
        setTitle(tr("Start List file format"));
    else
        setTitle(tr("Rankings file format"));

    setSubTitle(tr("Please select a file format and, if required, an encoding type."));
}

int RankingsWizardFormat::nextId() const
{
    int id = -1;

    if (RankingsWizard const *parentWizard = qobject_cast<RankingsWizard *>(wizard());
        !parentWizard || (parentWizard->getTarget() == RankingsWizard::RankingsWizardTarget::Rankings)) {
        id = static_cast<int>((fileFormat.currentIndex() == static_cast<int>(CRLoader::Format::PDF)) ?
                                RankingsWizard::RankingsWizardPage::Page_Mode :
                                RankingsWizard::RankingsWizardPage::Page_Selection);
    }

    return id;
}

void RankingsWizardFormat::formatChange(int index)
{
    switch (index) {
    case static_cast<int>(CRLoader::Format::PDF):
        fileEncoding.setEnabled(false);
        CRLoader::setFormat(CRLoader::Format::PDF);
        break;
    case static_cast<int>(CRLoader::Format::TEXT):
        fileEncoding.setEnabled(true);
        CRLoader::setFormat(CRLoader::Format::TEXT);
        break;
    case static_cast<int>(CRLoader::Format::CSV):
        fileEncoding.setEnabled(true);
        CRLoader::setFormat(CRLoader::Format::CSV);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

void RankingsWizardFormat::encodingChange(int index) const
{
    switch (index) {
    case static_cast<int>(CRLoader::Encoding::UTF8):
        CRLoader::setEncoding(CRLoader::Encoding::UTF8);
        break;
    case static_cast<int>(CRLoader::Encoding::LATIN1):
        CRLoader::setEncoding(CRLoader::Encoding::LATIN1);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}
