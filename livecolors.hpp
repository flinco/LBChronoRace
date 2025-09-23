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

#ifndef LIVECOLORS_HPP
#define LIVECOLORS_HPP

#include <QDialog>
#include <QScopedPointer>
#include <QColor>
#include <QAbstractButton>

#include "ui_livecolors.h"

class LiveColors : public QDialog
{
    Q_OBJECT

public:
    explicit LiveColors(QWidget *parent = Q_NULLPTR);

public slots:
    void accept() override;
    void reject() override;
    void restore(QAbstractButton *button);
    void show(); //NOSONAR

private:
    QScopedPointer<Ui::LiveColors> ui { new Ui::LiveColors };

    QColor liveStartListTitleColor;
    QColor liveStartListTextColor;
    QColor liveStartListBackgroundColor;
    QColor liveRankingsTitleColor;
    QColor liveRankingsTextColor;
    QColor liveRankingsBackgroundColor;

    void loadConfiguredColors();

private slots:
    void loadColor();
};

#endif // LIVECOLORS_HPP
