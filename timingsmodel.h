#ifndef LBTIMINGSMODEL_H
#define LBTIMINGSMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.h"
#include "timing.h"

class TimingsModel : public CRTableModel
{
    Q_OBJECT

public:
    explicit TimingsModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };
    TimingsModel(const QList<Timing>& timingsList, QObject *parent = Q_NULLPTR) : CRTableModel(parent), timings(timingsList) { };

    friend QDataStream &operator<<(QDataStream &out, const TimingsModel &data);
    friend QDataStream &operator>>(QDataStream &in, TimingsModel &data);

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

    const QList<Timing>& getTimings() const;

public slots:
    void refreshCounters(int r) override;

private:
    QList<Timing> timings;

signals:
    void error(QString const &message);
};


#endif // LBTIMINGSMODEL_H
