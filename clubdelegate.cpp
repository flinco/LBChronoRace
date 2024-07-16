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

#include "clubdelegate.hpp"
#include "crloader.hpp"

ClubDelegate::ClubDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = clubBox.data();
    comboBox->setEditable(true);
    comboBox->setInsertPolicy(QComboBox::InsertAlphabetically);
    comboBox->setDuplicatesEnabled(false);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox->addItems(CRLoader::getClubs());
}

QWidget *ClubDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = clubBox.data();
    comboBox->setParent(parent);
    comboBox->clear();
    comboBox->addItems(CRLoader::getClubs());

    return comboBox;
}

void ClubDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void ClubDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
}

void ClubDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::EditRole), Qt::EditRole);
}

QSize ClubDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return this->clubBox.data()->sizeHint();
}

void ClubDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}
