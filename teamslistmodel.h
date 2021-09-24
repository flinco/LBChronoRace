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

    friend QDataStream &operator<<(QDataStream &out, TeamsListModel const &data);
    friend QDataStream &operator>>(QDataStream &in, TeamsListModel &data);

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void reset();

    QList<QString> const &getTeamsList() const;

    uint getTeamNameWidthMax() const;

public slots:
    void addTeam(QString const &team);
    void refreshCounters(int r) override;

private:
    QList<QString> teamsList;
    uint           teamNameWidthMax;
};

#endif // LBTEAMSLISTMODEL_H
