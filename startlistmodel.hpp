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

#ifndef LBSTARTLISTMODEL_H
#define LBSTARTLISTMODEL_H

#include <QObject>
#include <QDataStream>

#include "crtablemodel.hpp"
#include "competitor.hpp"

class StartListModel : public CRTableModel
{
    Q_OBJECT

public:
    explicit StartListModel(QObject *parent = Q_NULLPTR) : CRTableModel(parent) { };
    StartListModel(QList<Competitor> const &startList, QObject *parent = Q_NULLPTR) : CRTableModel(parent), startList(startList) { };

    friend QDataStream &operator<<(QDataStream &out, StartListModel const &data);
    friend QDataStream &operator>>(QDataStream &in, StartListModel &data);

    int rowCount(QModelIndex const &parent = QModelIndex()) const override;
    int columnCount(QModelIndex const &parent = QModelIndex()) const override;
    QVariant data(QModelIndex const &index, int role) const override;
    bool setData(QModelIndex const &index, QVariant const &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(QModelIndex const &index) const override;
    bool insertRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    bool removeRows(int position, int rows, QModelIndex const &index = QModelIndex()) override;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    QList<Competitor> const &getStartList() const;

    void reset();

    uint getLegCount() const;
    void setLegCount(uint leg);
    uint getMaxBib() const;
    uint getCompetitorNameMaxWidth() const;

public slots:
    void refreshCounters(int r) override;

signals:
    void newTeam(QString const &team);
    void error(QString const &message);

private:
    QList<Competitor> startList;
    uint              legCount;
    uint              maxBib;
    uint              competitorNameMaxWidth;

    QString const *getTeam(uint bib);
    void setTeam(uint bib, QString const &team);
    int getMaxLeg(uint bib, int skip);
};

#endif // LBSTARTLISTMODEL_H
