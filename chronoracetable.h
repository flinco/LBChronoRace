#ifndef CHRONORACETABLE_H
#define CHRONORACETABLE_H

#include <QObject>
#include <QDialog>
#include <QStandardItemModel>

#include "crtablemodel.h"

namespace Ui {
class ChronoRaceTable;
}

class ChronoRaceTable : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceTable(QWidget *parent = Q_NULLPTR);
    ~ChronoRaceTable();

    QAbstractTableModel *getModel() const;
    void setModel(CRTableModel* model);
    void disableButtons();

private slots:
    void rowAdd();
    void rowDel();
    void modelImport();
    void modelExport();
    void dialogQuit();

public slots:
    void show();

signals:
    void newRowCount(int count);
    void modelImported();
    void modelExported();
    void countersRefresh();

private:
    Ui::ChronoRaceTable *ui;
};

#endif // CHRONORACETABLE_H
