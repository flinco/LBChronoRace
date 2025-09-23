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

#ifndef TRIGGERKEYDIALOG_HPP
#define TRIGGERKEYDIALOG_HPP

#include <QWidget>
#include <QDialog>
#include <QScopedPointer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QKeyCombination>

class TriggerKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TriggerKeyDialog(QWidget *parent = Q_NULLPTR);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    enum class State { WaitingFirst, WaitingSecond, Done };

    QScopedPointer<QVBoxLayout> layout { new QVBoxLayout };

    void updateMessage();
    bool isKeyAllowed(QKeyCombination const &key) const;

    QKeyCombination regiteredKey { Qt::KeyboardModifier::NoModifier, Qt::Key::Key_unknown };

    State state = State::WaitingFirst;

    QSet<QKeyCombination> forbiddenKeys;

public slots:
    void accept() override;
};

#endif // TRIGGERKEYDIALOG_HPP
