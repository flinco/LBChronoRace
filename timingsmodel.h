/*****************************************************************************
 * Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program. If not, see <https://www.gnu.org/licenses/>.     *
 *****************************************************************************/

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

    QList<Timing> const &getTimings() const;

    void reset();

public slots:
    void refreshCounters(int r) override;

private:
    QList<Timing> timings;

signals:
    void error(QString const &message);
};


#endif // LBTIMINGSMODEL_H
