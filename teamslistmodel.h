#ifndef LBTEAMSLISTMODEL_H
#define LBTEAMSLISTMODEL_H

#include <QObject>
#include <QSet>
#include <QList>
#include <QAbstractTableModel>

class TeamsListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TeamsListModel(QObject *parent = 0)
        : QAbstractTableModel(parent), teamsList() {}

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

public slots:
    void addTeam(const QString& team);

private:
    QList<QString> teamsList;
};

#endif // LBTEAMSLISTMODEL_H
