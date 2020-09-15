#ifndef LBTIMINGSMODEL_H
#define LBTIMINGSMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QDataStream>

#include "timing.h"

class TimingsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    TimingsModel(QObject *parent = 0)
        : QAbstractTableModel(parent), timings() {}
    TimingsModel(const QList<Timing>& timingsList, QObject *parent = 0)
        : QAbstractTableModel(parent), timings(timingsList) {}

    friend QDataStream &operator<<(QDataStream &out, const TimingsModel &data);
    friend QDataStream &operator>>(QDataStream &in, TimingsModel &data);

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

    const QList<Timing>& getTimings() const;

public slots:
    void refreshCounters(int r);

private:
    QList<Timing> timings;
};


#endif // LBTIMINGSMODEL_H
