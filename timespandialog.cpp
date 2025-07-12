/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QRegularExpressionValidator>
#include <QPushButton>

#include "timespandialog.hpp"

TimeSpanDialog::TimeSpanDialog(QWidget *parent, QString const &title, QString const &label, bool sub) :
    QDialog(parent),
    subtract(sub)
{
    auto *grid = layout.data();
    this->setLayout(grid);

    this->setWindowTitle(title);
    this->setWindowFlag(Qt::WindowType::Dialog, true);
    this->setInputMethodHints(Qt::InputMethodHint::ImhTime);

    grid->addWidget(new QLabel(label), 0, 0, 1, 4);
    grid->addWidget(new QSpinBox(), 1, 0);
    grid->addWidget(new QSpinBox(), 1, 1);
    grid->addWidget(new QSpinBox(), 1, 2);
    grid->addWidget(new QSpinBox(), 1, 3);

    auto *spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 0)->widget());
    spinBox->setSuffix(" h");
    spinBox->setRange(0, 9);
    spinBox->setToolTip(tr("Number of hours to add up or subtract"));
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 1)->widget());
    spinBox->setSuffix(" m");
    spinBox->setRange(0, 59);
    spinBox->setToolTip(tr("Number of minutes to add up or subtract"));
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 2)->widget());
    spinBox->setSuffix(" s");
    spinBox->setRange(0, 59);
    spinBox->setToolTip(tr("Number of seconds to add up or subtract"));
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 3)->widget());
    spinBox->setSuffix(" ms");
    spinBox->setRange(0, 999);
    spinBox->setToolTip(tr("Number of milliseconds to add up or subtract"));

    grid->addWidget(new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel), 2, 0, 1, 4);
    auto const *buttonBox = static_cast<QDialogButtonBox *>(grid->itemAtPosition(2, 0)->widget());

    connect(buttonBox, &QDialogButtonBox::accepted, this, &TimeSpanDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->adjustSize();
}

void TimeSpanDialog::accept()
{
    int value = 0;
    auto const *grid = layout.data();

    auto const *spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 0)->widget());
    value += 3600000 * spinBox->value();
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 1)->widget());
    value += 60000 * spinBox->value();
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 2)->widget());
    value += 1000 * spinBox->value();
    spinBox = static_cast<QSpinBox *>(grid->itemAtPosition(1, 3)->widget());
    value += spinBox->value();

    this->hide();

    emit applyOffset(subtract ? -value : value);
    QDialog::accept();
}
