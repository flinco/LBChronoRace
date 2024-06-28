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

ChronoRaceTable::ChronoRaceTable(QWidget *parent) : QDialog(parent)
{
    ui->setupUi(this);

    ui->tableView->setSortingEnabled(true);

    QObject::connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, ui->tableView, &QTableView::sortByColumn);
    QObject::connect(ui->rowAdd, &QPushButton::clicked, this, &ChronoRaceTable::rowAdd);
    QObject::connect(ui->rowDel, &QPushButton::clicked, this, &ChronoRaceTable::rowDel);
    QObject::connect(ui->modelImport, &QPushButton::clicked, this, &ChronoRaceTable::modelImport);
    QObject::connect(ui->modelExport, &QPushButton::clicked, this, &ChronoRaceTable::modelExport);
    QObject::connect(ui->dialogQuit, &QPushButton::clicked, this, &ChronoRaceTable::dialogQuit);
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
    ui->modelImport->setEnabled(false);
    //ui->modelExport->setEnabled(false); //NOSONAR
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
                                 tr("Are the data you are importing ISO-8859-1 (Latin-1) encoded?\n"
                                    "Choose No to use UTF-8 encoding. If in doubt, choose Yes."),
                                 QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes) == QMessageBox::Yes) {
        emit modelImported(CRLoader::Encoding::LATIN1);
    } else {
        emit modelImported(CRLoader::Encoding::UTF8);
    }
}

void ChronoRaceTable::modelExport()
{
    emit modelExported();
}

void ChronoRaceTable::dialogQuit()
{
    emit newRowCount(ui->tableView->model()->rowCount());
    this->close();
}
