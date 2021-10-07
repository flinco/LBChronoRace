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

#ifndef CHRONORACETABLE_H
#define CHRONORACETABLE_H

#include <QObject>
#include <QDialog>
#include <QStandardItemModel>

#include "ui_chronoracetable.h"
#include "crtablemodel.h"

class ChronoRaceTable : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceTable(QWidget *parent = Q_NULLPTR);

    QAbstractTableModel *getModel() const;
    void setModel(CRTableModel *model) const;
    void disableButtons() const;

private slots:
    void rowAdd() const;
    void rowDel() const;
    void modelImport();
    void modelExport();
    void dialogQuit();

public slots:
    void show(); //NOSONAR

signals:
    void newRowCount(int count);
    void modelImported();
    void modelExported();
    void countersRefresh();

private:
    QScopedPointer<Ui::ChronoRaceTable> ui { new Ui::ChronoRaceTable };
};

#endif // CHRONORACETABLE_H
