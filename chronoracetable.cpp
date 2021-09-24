#include "chronoracetable.h"

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

QAbstractTableModel* ChronoRaceTable::getModel() const
{
    return qobject_cast<QAbstractTableModel*>(ui->tableView->model());
}

void ChronoRaceTable::setModel(CRTableModel* model) const
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
    emit modelImported();
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
