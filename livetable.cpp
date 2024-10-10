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

#include "livetable.hpp"

LiveTable::LiveTable(QWidget *parent) : QWidget(parent)
{
    ui->setupUi(this);

    //QStringList headers;

    //this->setWindowState(Qt::WindowFullScreen);
    // Make our window without panels
    //this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowTransparentForInput);
    // Resize refer to desktop
    //this->resize( QApplication::desktop()->size() );
    //this->setFocusPolicy(Qt::NoFocus);
    //this->setAttribute(Qt::WA_QuitOnClose, true);

    //headers << tr("Bib")
    //        << tr("Competitor/Team")
    //        << tr("Timing");

    //this->setColumnCount(3);
    //this->setHorizontalHeaderLabels(headers);

    //this->insertRow(0);
    //this->insertRow(1);
    //this->insertRow(2);

}
