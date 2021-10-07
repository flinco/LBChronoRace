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

#ifndef CHRONORACELOGO_H
#define CHRONORACELOGO_H

#include <QCoreApplication>
#include <QPixmap>
#include <QLabel>

class ChronoRaceLogo : public QObject
{
    Q_OBJECT

public:
    QPixmap pixmap;
    QLabel *uiElement;

public slots:
    void loadLogo();
    void removeLogo();

signals:
    void logoLoaded(QLabel *label = Q_NULLPTR);
    void logoRemoved(QLabel *label = Q_NULLPTR);
};

#endif // CHRONORACELOGO_H
