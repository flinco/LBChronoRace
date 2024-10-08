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

#include <QMessageBox>

#include "chronoracetable.hpp"
#include "crloader.hpp"
#include "crhelper.hpp"

ChronoRaceTable::ChronoRaceTable(QWidget *parent) : QDialog(parent)
{
    ui->setupUi(this);

    ui->tableView->setSortingEnabled(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    QObject::connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, ui->tableView, &QTableView::sortByColumn);
    QObject::connect(ui->rowAdd, &QPushButton::clicked, this, &ChronoRaceTable::rowAdd);
    QObject::connect(ui->rowDel, &QPushButton::clicked, this, &ChronoRaceTable::rowDel);
    QObject::connect(ui->modelImport, &QPushButton::clicked, this, &ChronoRaceTable::modelImport);
    QObject::connect(ui->modelExport, &QPushButton::clicked, this, &ChronoRaceTable::modelExport);
    QObject::connect(ui->dialogSave, &QPushButton::clicked, this, &ChronoRaceTable::saveRaceData);
    QObject::connect(ui->dialogQuit, &QPushButton::clicked, this, &ChronoRaceTable::close);
}

QAbstractTableModel *ChronoRaceTable::getModel() const
{
    return qobject_cast<QAbstractTableModel *>(ui->tableView->model());
}

void ChronoRaceTable::setModel(CRTableModel *model) const
{
    ui->tableView->setModel(model);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QObject::connect(this, &ChronoRaceTable::finished, model, &CRTableModel::refreshCounters);
}

void ChronoRaceTable::disableButtons() const
{
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->rowAdd->setEnabled(false);
    ui->rowDel->setEnabled(false);
    //NOSONAR ui->modelImport->setEnabled(false);
    //NOSONAR ui->modelExport->setEnabled(false);
}

void ChronoRaceTable::setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate)
{
    ui->tableView->setItemDelegateForColumn(column, delegate);
}

void ChronoRaceTable::show()
{
    ui->retranslateUi(this);
    ui->tableView->resizeColumnsToContents();
    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
}

void ChronoRaceTable::rowAdd() const
{
    if (ui->tableView->selectionModel()->hasSelection()) {
        int rowCount = ui->tableView->model()->rowCount();
        for (int r = rowCount - 1; r >= 0; --r) {
            if (ui->tableView->selectionModel()->rowIntersectsSelection(r, QModelIndex())) {
                ui->tableView->model()->insertRow(r + 1, QModelIndex());
            }
        }
    } else {
        ui->tableView->model()->insertRow(ui->tableView->model()->rowCount(), QModelIndex());
    }
}

void ChronoRaceTable::rowDel() const
{
    if (ui->tableView->selectionModel()->hasSelection()) {
        int rowCount = ui->tableView->model()->rowCount();
        for (int r = rowCount - 1; r >= 0; --r) {
            if (ui->tableView->selectionModel()->rowIntersectsSelection(r, QModelIndex())) {
                ui->tableView->model()->removeRow(r, QModelIndex());
            }
        }
    }
}

void ChronoRaceTable::modelImport()
{
    if (QMessageBox::question(this, tr("CSV Encoding"),
                              tr("The data being imported must be %1 encoded.\n"
                                 "Continue?").arg(CRHelper::encodingToLabel(CRLoader::getEncoding())),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        emit modelImported();
    }
}

void ChronoRaceTable::modelExport()
{
    if (QMessageBox::question(this, tr("CSV Encoding"),
                              tr("The data will be exported with %1 encoding.\n"
                                 "Continue?").arg(CRHelper::encodingToLabel(CRLoader::getEncoding())),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        emit modelExported();
    }
}

void ChronoRaceTable::dialogSave()
{
    emit saveRaceData();
}
