#include <QPushButton>

#include "chronoracetable.h"
#include "ui_chronoracetable.h"

ChronoRaceTable::ChronoRaceTable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChronoRaceTable)
{
    ui->setupUi(this);
    ui->tableView->setSortingEnabled(true);
    this->connect(ui->tableView->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), ui->tableView, SLOT(sortByColumn(int)));
}

ChronoRaceTable::~ChronoRaceTable()
{
    delete ui;
}

QAbstractTableModel* ChronoRaceTable::getModel() const
{
    return qobject_cast<QAbstractTableModel*>(ui->tableView->model());
}

void ChronoRaceTable::setModel(QAbstractTableModel* model)
{

    ui->tableView->setModel(model);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    //QDialog::show();
    QDialog::exec(); // modal
}

void ChronoRaceTable::on_rowAdd_clicked()
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

void ChronoRaceTable::on_rowDel_clicked()
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

void ChronoRaceTable::on_modelImport_clicked()
{
    emit modelImport();
}

void ChronoRaceTable::on_modelExport_clicked()
{
    emit modelExport();
}

void ChronoRaceTable::on_dialogQuit_clicked()
{
    emit newRowCount(ui->tableView->model()->rowCount());
    this->close();
}
