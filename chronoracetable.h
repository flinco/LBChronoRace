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
