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

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "triggerkeydialog.hpp"
#include "crsettings.hpp"

TriggerKeyDialog::TriggerKeyDialog(QWidget *parent) :
    QDialog(parent)
{
    auto *grid = layout.data();
    this->setLayout(grid);

    this->setWindowTitle(tr("External hardware trigger key detection"));
    this->setWindowFlag(Qt::WindowType::Dialog, true);

    layout->addWidget(new QLabel(tr("Press the key on your trigger device…"), this));
    auto *label = static_cast<QLabel *>(grid->itemAt(0)->widget());
    label->setAlignment(Qt::AlignCenter);

    grid->addWidget(new QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel));
    auto const *buttonBox = static_cast<QDialogButtonBox *>(grid->itemAt(1)->widget());
    buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &TriggerKeyDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    qApp->installEventFilter(this);

    this->setMinimumWidth(360);
    this->setMinimumHeight(120);
    this->adjustSize();

    // Populate the list of forbidden keys
    for (int k = Qt::Key::Key_0; k <= Qt::Key::Key_9; ++k) {
        forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, static_cast<Qt::Key>(k)));
        forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, static_cast<Qt::Key>(k)));
    }
    for (int k = Qt::Key_A; k <= Qt::Key_Z; ++k) {
        forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, static_cast<Qt::Key>(k)));
        forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::ShiftModifier, static_cast<Qt::Key>(k)));
    }
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Space));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::AltModifier, Qt::Key::Key_Delete));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Return));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Return));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Enter));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Enter));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Backspace));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Backspace));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Delete));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Delete));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Up));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Up));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Down));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Down));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key_Escape));
    forbiddenKeys.insert(QKeyCombination(Qt::KeyboardModifier::KeypadModifier, Qt::Key_Escape));
}

void TriggerKeyDialog::keyPressEvent(QKeyEvent *event)
{
    if (!this->isVisible())
        return;

    auto const keyCombination = event->keyCombination();
    auto *label = static_cast<QLabel *>(layout->itemAt(0)->widget());

    if (auto key = keyCombination.key();
        (key == Qt::Key_Shift) ||
        (key == Qt::Key_Control) ||
        (key == Qt::Key_Alt) ||
        (key == Qt::Key_Meta)) {
        event->ignore();
        return;
    }

    if (!isKeyAllowed(keyCombination)) {
        label->setText(tr("This trigger key is not allowed.\nReconfigure the trigger device and try again."));
        return;
    }

    switch (state) {
        using enum TriggerKeyDialog::State;

        case WaitingFirst:
            regiteredKey = keyCombination;
            state = WaitingSecond;
            label->setText(tr("Repeat the trigger key to confirm."));
            break;

        case WaitingSecond:
            if (auto const *buttonBox = static_cast<QDialogButtonBox *>(layout->itemAt(1)->widget()); keyCombination == regiteredKey) {
                label->setText(tr("The trigger key has been set to 0x%1.").arg(regiteredKey.toCombined(), 8, 16, '0'));
                buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
                state = Done;
            } else {
                label->setText(tr("Trigger keys do not match. Try again."));
                regiteredKey = QKeyCombination(Qt::KeyboardModifier::NoModifier, Qt::Key::Key_unknown);
                buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
                state = WaitingFirst;
            }
            break;

        case Done:
            break;
    }
}

bool TriggerKeyDialog::isKeyAllowed(QKeyCombination const &key) const
{
    return !forbiddenKeys.contains(key);
}

void TriggerKeyDialog::accept()
{
    this->hide();

    CRSettings::setTriggerKey(regiteredKey);

    QDialog::accept();
}
