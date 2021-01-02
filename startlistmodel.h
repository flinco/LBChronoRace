#ifndef LBSTARTLISTMODEL_H
#define LBSTARTLISTMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.h"
#include "competitor.h"

class StartListModel : public CRTableModel
{
    Q_OBJECT

public:
    StartListModel(QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), startList() {}
    StartListModel(const QList<Competitor>& startList, QObject *parent = Q_NULLPTR)
        : CRTableModel(parent), startList(startList) {}

    friend QDataStream &operator<<(QDataStream &out, const StartListModel &data);
    friend QDataStream &operator>>(QDataStream &in, StartListModel &data);

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

    const QList<Competitor>& getStartList() const;

    uint getLegCount() const;
    void setLegCount(uint leg);
    uint getMaxBib() const;
    uint getCompetitorNameMaxWidth() const;

public slots:
    void refreshCounters(int r);

signals:
    void newTeam(const QString& team);

private:
    QList<Competitor> startList;
    uint              legCount;
    uint              maxBib;
    uint              competitorNameMaxWidth;
};

#endif // LBSTARTLISTMODEL_H
