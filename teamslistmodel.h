#ifndef LBTEAMSLISTMODEL_H
#define LBTEAMSLISTMODEL_H

#include <QObject>
#include <QSet>
#include <QList>
#include <QDataStream>

#include "crtablemodel.h"

class TeamsListModel : public CRTableModel
{
    Q_OBJECT

public:
    TeamsListModel(QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), teamsList() {}

    friend QDataStream &operator<<(QDataStream &out, const TeamsListModel &data);
    friend QDataStream &operator>>(QDataStream &in, TeamsListModel &data);

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

    const QList<QString>& getTeamsList() const;

    uint getTeamNameWidthMax() const;

public slots:
    void addTeam(const QString& team);
    void refreshCounters(int r);

private:
    QList<QString> teamsList;
    uint           teamNameWidthMax;
};

#endif // LBTEAMSLISTMODEL_H
