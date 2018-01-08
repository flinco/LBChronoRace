#ifndef CHRONORACETABLE_H
#define CHRONORACETABLE_H

#include <QObject>
#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class ChronoRaceTable;
}

class ChronoRaceTable : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceTable(QWidget *parent = 0);
    ~ChronoRaceTable();

    QAbstractTableModel *getModel() const;
    void setModel(QAbstractTableModel* model);
    void disableButtons();

private slots:

    void on_rowAdd_clicked();
    void on_rowDel_clicked();
    void on_modelSave_clicked();

    void on_modelQuit_clicked();

public slots:
    void show();

signals:
    void newRowCount(int count);
    void modelSave();

private:
    Ui::ChronoRaceTable *ui;
};

#endif // CHRONORACETABLE_H
