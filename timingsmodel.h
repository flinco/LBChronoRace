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
    TimingsModel(QList<Timing> const &timingsList, QObject *parent = Q_NULLPTR) : CRTableModel(parent), timings(timingsList) { };

    friend QDataStream &operator<<(QDataStream &out, TimingsModel const &data);
    friend QDataStream &operator>>(QDataStream &in, TimingsModel &data);

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

    QList<Timing> const &getTimings() const;

public slots:
    void refreshCounters(int r) override;

private:
    QList<Timing> timings;

signals:
    void error(QString const &message);
};


#endif // LBTIMINGSMODEL_H
