#ifndef LBSTARTLISTMODEL_H
#define LBSTARTLISTMODEL_H

#include <QObject>
#include <QAbstractTableModel>

#include "competitor.h"

class StartListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    StartListModel(QObject *parent = nullptr)
        : QAbstractTableModel(parent), startList() {}
    StartListModel(const QList<Competitor>& startList, QObject *parent = nullptr)
        : QAbstractTableModel(parent), startList(startList) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    void reset();

signals:
    void newTeam(const QString& team);

private:
    QList<Competitor> startList;
};

#endif // LBSTARTLISTMODEL_H
