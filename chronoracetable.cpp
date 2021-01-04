#include "chronoracetable.h"
#include "ui_chronoracetable.h"

ChronoRaceTable::ChronoRaceTable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChronoRaceTable)
{
    ui->setupUi(this);
    ui->tableView->setSortingEnabled(true);
    this->connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, ui->tableView, &QTableView::sortByColumn);
    this->connect(ui->rowAdd, &QPushButton::clicked, this, &ChronoRaceTable::rowAdd);
    this->connect(ui->rowDel, &QPushButton::clicked, this, &ChronoRaceTable::rowDel);
    this->connect(ui->modelImport, &QPushButton::clicked, this, &ChronoRaceTable::modelImport);
    this->connect(ui->modelExport, &QPushButton::clicked, this, &ChronoRaceTable::modelExport);
    this->connect(ui->dialogQuit, &QPushButton::clicked, this, &ChronoRaceTable::dialogQuit);
}

ChronoRaceTable::~ChronoRaceTable()
{
    delete ui;
}

QAbstractTableModel* ChronoRaceTable::getModel() const
{
    return qobject_cast<QAbstractTableModel*>(ui->tableView->model());
}

void ChronoRaceTable::setModel(CRTableModel* model)
{
    ui->tableView->setModel(model);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->connect(this, &ChronoRaceTable::finished, model, &CRTableModel::refreshCounters);
}

void ChronoRaceTable::disableButtons()
{
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->rowAdd->setEnabled(false);
    ui->rowDel->setEnabled(false);
    ui->modelImport->setEnabled(false);
    //ui->modelExport->setEnabled(false);
}

void ChronoRaceTable::show()
{
    ui->retranslateUi(this);
    ui->tableView->resizeColumnsToContents();
    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
}

void ChronoRaceTable::rowAdd()
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

void ChronoRaceTable::rowDel()
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
