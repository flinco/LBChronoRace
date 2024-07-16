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

#include "rankingcatsdelegate.hpp"

QWidget *RankingCategoriesDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    Q_ASSERT(categories);

    auto *comboBox = box.data();
    comboBox->setParent(parent);

    for (auto const &item : categories->getCategories()) {
        comboBox->addItem(item.getFullDescription(), item.getShortDescription());
    }

    return comboBox;
}

void RankingCategoriesDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)

    auto const *comboBox = box.data();
    comboBox->clear();
}

void RankingCategoriesDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto const *comboBox = static_cast<MultiSelectComboBox *>(editor);
    comboBox->setCurrentText(index.model()->data(index, Qt::EditRole).toStringList());
}

void RankingCategoriesDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<MultiSelectComboBox *>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

QSize RankingCategoriesDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return this->box.data()->sizeHint();
}

void RankingCategoriesDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

void RankingCategoriesDelegate::setCategories(CategoriesModel const *newCategories)
{
    categories = newCategories;
}
