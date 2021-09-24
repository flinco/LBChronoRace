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
    explicit TeamsListModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };

    friend QDataStream &operator<<(QDataStream &out, const TeamsListModel &data);
    friend QDataStream &operator>>(QDataStream &in, TeamsListModel &data);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void reset();

    const QList<QString>& getTeamsList() const;

    uint getTeamNameWidthMax() const;

public slots:
    void addTeam(const QString& team);
    void refreshCounters(int r) override;

private:
    QList<QString> teamsList;
    uint           teamNameWidthMax;
};

#endif // LBTEAMSLISTMODEL_H
