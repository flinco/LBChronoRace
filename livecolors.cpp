/*****************************************************************************
 * Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include <QPushButton>
#include <QColorDialog>
#include <QDialogButtonBox>

#include "livecolors.hpp"
#include "crsettings.hpp"

LiveColors::LiveColors(QWidget *parent) : QDialog(parent)
{
    ui->setupUi(this);

    this->loadConfiguredColors();

    QObject::connect(this->ui->lslTitleButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lslTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lslBMTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lslBFTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lslBackgroundButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lrTitleButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lrTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lrBMTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lrBFTextButton, &QPushButton::clicked, this, &LiveColors::loadColor);
    QObject::connect(this->ui->lrBackgroundButton, &QPushButton::clicked, this, &LiveColors::loadColor);

    QObject::connect(this->ui->buttonBox, &QDialogButtonBox::clicked, this, &LiveColors::restore);

    this->setWindowModality(Qt::ApplicationModal);
}

void LiveColors::accept()
{
    using enum CRSettings::Color;

    CRSettings::setColor(LiveStartListTitleColor, this->liveStartListTitleColor);
    CRSettings::setColor(LiveStartListTextColor, this->liveStartListTextColor);
    CRSettings::setColor(LiveStartListBestMColor, this->liveStartListBestMColor);
    CRSettings::setColor(LiveStartListBestFColor, this->liveStartListBestFColor);
    CRSettings::setColor(LiveStartListBackgroundColor, this->liveStartListBackgroundColor);
    CRSettings::setColor(LiveRankingsTitleColor, this->liveRankingsTitleColor);
    CRSettings::setColor(LiveRankingsTextColor, this->liveRankingsTextColor);
    CRSettings::setColor(LiveRankingsBestMColor, this->liveRankingsBestMColor);
    CRSettings::setColor(LiveRankingsBestFColor, this->liveRankingsBestFColor);
    CRSettings::setColor(LiveRankingsBackgroundColor, this->liveRankingsBackgroundColor);

    QDialog::accept();
}

void LiveColors::reject()
{
    this->loadConfiguredColors();

    QDialog::reject();
}

void LiveColors::restore(QAbstractButton *button)
{
    if (this->ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::ResetRole) {
        using enum CRSettings::Color;

        this->liveStartListTitleColor = CRSettings::getDefaultColor(LiveStartListTitleColor);
        this->liveStartListTextColor = CRSettings::getDefaultColor(LiveStartListTextColor);
        this->liveStartListBestMColor = CRSettings::getDefaultColor(LiveStartListBestMColor);
        this->liveStartListBestFColor = CRSettings::getDefaultColor(LiveStartListBestFColor);
        this->liveStartListBackgroundColor = CRSettings::getDefaultColor(LiveStartListBackgroundColor);
        this->liveRankingsTitleColor = CRSettings::getDefaultColor(LiveRankingsTitleColor);
        this->liveRankingsTextColor = CRSettings::getDefaultColor(LiveRankingsTextColor);
        this->liveRankingsBestMColor = CRSettings::getDefaultColor(LiveRankingsBestMColor);
        this->liveRankingsBestFColor = CRSettings::getDefaultColor(LiveRankingsBestFColor);
        this->liveRankingsBackgroundColor = CRSettings::getDefaultColor(LiveRankingsBackgroundColor);

        this->show();
    }
}

void LiveColors::show()
{
    ui->retranslateUi(this);

    QString style = QStringLiteral("font: bold; color: ") + this->liveStartListTitleColor.name() + QStringLiteral("; background-color: ") + this->liveStartListBackgroundColor.name();
    this->ui->lslTitleColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveStartListTextColor.name() + QStringLiteral("; background-color: ") + this->liveStartListBackgroundColor.name();
    this->ui->lslTextColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveStartListBestMColor.name() + QStringLiteral("; background-color: ") + this->liveStartListBackgroundColor.name();
    this->ui->lslBMTextColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveStartListBestFColor.name() + QStringLiteral("; background-color: ") + this->liveStartListBackgroundColor.name();
    this->ui->lslBFTextColor->setStyleSheet(style);
    style = QStringLiteral("font: bold; color: ") + this->liveRankingsTitleColor.name() + QStringLiteral("; background-color: ") + this->liveRankingsBackgroundColor.name();
    this->ui->lrTitleColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveRankingsTextColor.name() + QStringLiteral("; background-color: ") + this->liveRankingsBackgroundColor.name();
    this->ui->lrTextColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveRankingsBestMColor.name() + QStringLiteral("; background-color: ") + this->liveRankingsBackgroundColor.name();
    this->ui->lrBMTextColor->setStyleSheet(style);
    style = QStringLiteral("color: ") + this->liveRankingsBestFColor.name() + QStringLiteral("; background-color: ") + this->liveRankingsBackgroundColor.name();
    this->ui->lrBFTextColor->setStyleSheet(style);

    QDialog::show();
}

void LiveColors::loadConfiguredColors()
{
    using enum CRSettings::Color;

    this->liveStartListTitleColor = CRSettings::getColor(LiveStartListTitleColor);
    this->liveStartListTextColor = CRSettings::getColor(LiveStartListTextColor);
    this->liveStartListBestMColor = CRSettings::getColor(LiveStartListBestMColor);
    this->liveStartListBestFColor = CRSettings::getColor(LiveStartListBestFColor);
    this->liveStartListBackgroundColor = CRSettings::getColor(LiveStartListBackgroundColor);
    this->liveRankingsTitleColor = CRSettings::getColor(LiveRankingsTitleColor);
    this->liveRankingsTextColor = CRSettings::getColor(LiveRankingsTextColor);
    this->liveRankingsBestMColor = CRSettings::getColor(LiveRankingsBestMColor);
    this->liveRankingsBestFColor = CRSettings::getColor(LiveRankingsBestFColor);
    this->liveRankingsBackgroundColor = CRSettings::getColor(LiveRankingsBackgroundColor);
}

void LiveColors::loadColor()
{
    QColor *oldColor = Q_NULLPTR;
    QString title;

    if (auto const *senderObject = qobject_cast<QPushButton *>(sender()); senderObject == this->ui->lslTitleButton) {
        oldColor = &this->liveStartListTitleColor;
        title = tr("Live Start List title color");
    } else if (senderObject == this->ui->lslTextButton) {
        oldColor = &this->liveStartListTextColor;
        title = tr("Live Start List text color");
    } else if (senderObject == this->ui->lslBMTextButton) {
        oldColor = &this->liveStartListBestMColor;
        title = tr("Live Start List Best Male color");
    } else if (senderObject == this->ui->lslBFTextButton) {
        oldColor = &this->liveStartListBestFColor;
        title = tr("Live Start List Best Female color");
    } else if (senderObject == this->ui->lslBackgroundButton) {
        oldColor = &this->liveStartListBackgroundColor;
        title = tr("Live Start List background color");
    } else if (senderObject == this->ui->lrTitleButton) {
        oldColor = &this->liveRankingsTitleColor;
        title = tr("Live Rankings title color");
    } else if (senderObject == this->ui->lrTextButton) {
        oldColor = &this->liveRankingsTextColor;
        title = tr("Live Rankings text color");
    } else if (senderObject == this->ui->lrBMTextButton) {
        oldColor = &this->liveRankingsBestMColor;
        title = tr("Live Rankings Best Male color");
    } else if (senderObject == this->ui->lrBFTextButton) {
        oldColor = &this->liveRankingsBestFColor;
        title = tr("Live Rankings Best Female color");
    } else if (senderObject == this->ui->lrBackgroundButton) {
        oldColor = &this->liveRankingsBackgroundColor;
        title = tr("Live Rankings background color");
    }

    if (oldColor != Q_NULLPTR) {
        if (auto newColor = QColorDialog::getColor(*oldColor, this, title); newColor.isValid()) {
            *oldColor = newColor;
            this->show();
        }
    }
}
