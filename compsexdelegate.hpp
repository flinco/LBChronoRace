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

#ifndef COMPSEXDELEGATE_HPP
#define COMPSEXDELEGATE_HPP

#include <QObject>
#include <QStyledItemDelegate>
#include <QScopedPointer>
#include <QComboBox>

class CompetitorSexDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CompetitorSexDelegate(QObject *parent = Q_NULLPTR);

    QWidget *createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, QModelIndex const &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const override;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const override;
    void updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const override;

private:
    QScopedPointer<QComboBox> competitorSexBox { new QComboBox };
};

#endif // COMPSEXDELEGATE_HPP
