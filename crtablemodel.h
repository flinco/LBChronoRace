#ifndef CRTABLEMODEL_H
#define CRTABLEMODEL_H

#include <QAbstractTableModel>

class CRTableModel : public QAbstractTableModel
{
    Q_OBJECT
    using QAbstractTableModel::QAbstractTableModel;

public slots:
    virtual void refreshCounters(int r) = 0;
};

#endif // CRTABLEMODEL_H
