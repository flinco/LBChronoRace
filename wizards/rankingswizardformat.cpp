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
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

#include "wizards/rankingswizard.hpp"
#include "wizards/rankingswizardformat.hpp"
#include "crloader.hpp"
#include "crhelper.hpp"
#include "languages.hpp"

RankingsWizardFormat::RankingsWizardFormat(QWidget *parent) :
    QWizardPage(parent)
{
    QPointer<QWidget> widgetPtr(Q_NULLPTR);

    widgetPtr = new QComboBox;
    QComboBox *rankingsLanguage = qobject_cast<QComboBox *>(widgetPtr.data());
    rankingsLanguage->addItem(QIcon(QString(":/material/icons/language.svg")), tr("Default"));
    layout.addRow(new QLabel(tr("Rankings language")), rankingsLanguage);
    registerField("format.language", rankingsLanguage);

    widgetPtr = new QComboBox;
    QComboBox *fileFormat = qobject_cast<QComboBox *>(widgetPtr.data());
    using enum CRLoader::Format;
    fileFormat->insertItem(static_cast<int>(PDF), CRHelper::formatToLabel(PDF), static_cast<uint>(PDF));
    fileFormat->insertItem(static_cast<int>(TEXT), CRHelper::formatToLabel(TEXT), static_cast<uint>(TEXT));
    fileFormat->insertItem(static_cast<int>(CSV), CRHelper::formatToLabel(CSV), static_cast<uint>(CSV));
    fileFormat->setCurrentText(CRHelper::formatToLabel(CRLoader::getFormat()));
    layout.addRow(new QLabel(tr("Format")), fileFormat);
    registerField("format.format", fileFormat, "currentData");

    widgetPtr = new QComboBox;
    QComboBox *fileEncoding = qobject_cast<QComboBox *>(widgetPtr.data());
    using enum QStringConverter::Encoding;
    fileEncoding->addItem(CRHelper::encodingToLabel(Utf8), static_cast<uint>(Utf8));
    fileEncoding->addItem(CRHelper::encodingToLabel(Latin1), static_cast<uint>(Latin1));
    fileEncoding->setCurrentText(CRHelper::encodingToLabel(CRLoader::getEncoding()));
    layout.addRow(new QLabel(tr("Encoding")), fileEncoding);
    registerField("format.encoding", fileEncoding, "currentData");

    widgetPtr = new QCheckBox;
    QCheckBox *fileOpen = qobject_cast<QCheckBox *>(widgetPtr.data());
    fileOpen->setCheckState(Qt::CheckState::Checked);
    layout.addRow(new QLabel(tr("Open file after\npublishing")), fileOpen);
    layout.setAlignment(fileOpen, Qt::AlignmentFlag::AlignLeft | Qt::AlignmentFlag::AlignVCenter);
    registerField("format.open", fileOpen);

    widgetPtr = Q_NULLPTR;

    formatChange(fileFormat->currentIndex());
    connect(fileFormat, &QComboBox::currentIndexChanged, this, &RankingsWizardFormat::formatChange);

    setLayout(&layout);
}

void RankingsWizardFormat::initializePage()
{
    RankingsWizard *parentWizard = qobject_cast<RankingsWizard *>(wizard());

    Q_ASSERT(parentWizard);

    if (parentWizard->getTarget() == RankingsWizard::RankingsWizardTarget::StartList)
        setTitle(tr("Start List file format"));
    else
        setTitle(tr("Rankings file format"));

    setSubTitle(tr("Please select a file format and, if required, an encoding type."));

    ChronoRaceData *raceData = parentWizard->getRaceData();
    QComboBox *rankingsLanguage = qobject_cast<QComboBox *>(layout.itemAt(0, QFormLayout::ItemRole::FieldRole)->widget());
    QStringList filter = raceData->getFieldValues(ChronoRaceData::IndexField::LANGUAGE);
    Languages::loadMenu(rankingsLanguage, &filter);
    rankingsLanguage->setCurrentIndex(raceData->getFieldIndex(ChronoRaceData::IndexField::LANGUAGE));
}

int RankingsWizardFormat::nextId() const
{
    int id = -1;

    if (RankingsWizard const *parentWizard = qobject_cast<RankingsWizard *>(wizard());
        !parentWizard || (parentWizard->getTarget() == RankingsWizard::RankingsWizardTarget::Rankings)) {
        QComboBox const *fileFormat = qobject_cast<QComboBox *>(layout.itemAt(1, QFormLayout::ItemRole::FieldRole)->widget());
        id = static_cast<int>((fileFormat->currentIndex() == static_cast<int>(CRLoader::Format::PDF)) ?
                                RankingsWizard::RankingsWizardPage::Page_Mode :
                                RankingsWizard::RankingsWizardPage::Page_Selection);
    }

    return id;
}

void RankingsWizardFormat::formatChange(int index) const
{
    QComboBox *fileEncoding = qobject_cast<QComboBox *>(layout.itemAt(2, QFormLayout::ItemRole::FieldRole)->widget());

    switch (index) {
        using enum CRLoader::Format;

        case static_cast<int>(PDF):
            fileEncoding->setEnabled(false);
            break;
        case static_cast<int>(TEXT):
            [[fallthrough]];
        case static_cast<int>(CSV):
            fileEncoding->setEnabled(true);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }

}
