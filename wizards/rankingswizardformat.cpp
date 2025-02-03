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
#include "wizards/rankingswizard.hpp"
#include "wizards/rankingswizardformat.hpp"
#include "crhelper.hpp"

RankingsWizardFormat::RankingsWizardFormat(QWidget *parent) :
    QWizardPage(parent),
    fileFormat(parent),
    fileEncoding(parent),
    fileOpen(parent)
{
    using enum CRLoader::Format;

    auto formatIdx = static_cast<int>(CRLoader::getFormat());
    fileFormat.insertItem(static_cast<int>(PDF), CRHelper::formatToLabel(PDF));
    fileFormat.insertItem(static_cast<int>(TEXT), CRHelper::formatToLabel(TEXT));
    fileFormat.insertItem(static_cast<int>(CSV), CRHelper::formatToLabel(CSV));
    fileFormat.setCurrentIndex(formatIdx);
    layout.addRow(new QLabel(tr("Format")), &fileFormat);

    fileEncoding.addItem(CRHelper::encodingToLabel(QStringConverter::Encoding::Utf8), QVariant(QStringConverter::Encoding::Utf8));
    fileEncoding.addItem(CRHelper::encodingToLabel(QStringConverter::Encoding::Latin1), QVariant(QStringConverter::Encoding::Latin1));
    fileEncoding.setCurrentText(CRHelper::encodingToLabel(CRLoader::getEncoding()));
    layout.addRow(new QLabel(tr("Encoding")), &fileEncoding);

    fileOpen.setCheckState(Qt::CheckState::Checked);
    layout.addRow(new QLabel(tr("Open file after\npublishing")), &fileOpen);
    layout.setAlignment(&fileOpen, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);

    formatChange(formatIdx);
    connect(&fileFormat, &QComboBox::currentIndexChanged, this, &RankingsWizardFormat::formatChange);
    connect(&fileEncoding, &QComboBox::currentIndexChanged, this, &RankingsWizardFormat::encodingChange);
    connect(&fileOpen, &QCheckBox::checkStateChanged, this, &RankingsWizardFormat::openChange);

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
        using enum CRLoader::Format;

        case static_cast<int>(PDF):
            fileEncoding.setEnabled(false);
            CRLoader::setFormat(PDF);
            break;
        case static_cast<int>(TEXT):
            fileEncoding.setEnabled(true);
            CRLoader::setFormat(TEXT);
            break;
        case static_cast<int>(CSV):
            fileEncoding.setEnabled(true);
            CRLoader::setFormat(CSV);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

void RankingsWizardFormat::encodingChange(int index) const
{
    CRLoader::setEncoding(fileEncoding.itemData(index, Qt::UserRole).value<QStringConverter::Encoding>());
}

void RankingsWizardFormat::openChange(Qt::CheckState state)
{
    emit notifyOpenChange(state == Qt::CheckState::Checked);
}
