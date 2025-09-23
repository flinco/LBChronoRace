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

#ifndef LIVESTARTLIST_HPP
#define LIVESTARTLIST_HPP

#include <QWidget>
#include <QScopedPointer>
#include <QStandardItemModel>
#include <QScreen>
#include <QString>
#include <QDate>
#include <QPixmap>
#include <QModelIndex>

#include "ui_livestartlist.h"

class LiveStartList : public QWidget
{
    Q_OBJECT

public:
    explicit LiveStartList(QWidget *parent = Q_NULLPTR);

    void setLiveScreen(QScreen const *screen);

    void activate();
    void resizeColumns();

    void setHeaderData(QString const &title, QString const &place, QDate const &date, QPixmap const &leftLogo, QPixmap const &rightLogo);

    QStandardItemModel *getModel();

    void setTimeColumns(bool newTimeColumns);

public slots:
    void demoStep();

private:
    QScopedPointer<Ui::liveStartList> ui { new Ui::liveStartList };
    QScopedPointer<QStandardItemModel> model { new QStandardItemModel };

    QScreen const *liveScreen { Q_NULLPTR };

    QModelIndex demoIndex;

    bool timeColumns { false };

    void resizeColumns1(int columns, int fullWidth, int availableWidth);
    void resizeColumns2(int columns, int fullWidth, int availableWidth);
};

#endif // LIVESTARTLIST_HPP
