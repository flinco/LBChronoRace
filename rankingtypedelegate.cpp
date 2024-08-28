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

#include "rankingtypedelegate.hpp"
#include "crhelper.hpp"

RankingTypeDelegate::RankingTypeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = rankingTypeBox.data();
    comboBox->setEditable(false);
    comboBox->setInsertPolicy(QComboBox::NoInsert);
    comboBox->setDuplicatesEnabled(false);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox->addItem(QIcon(":/material/icons/person.svg"), CRHelper::toRankingTypeString(Ranking::Type::INDIVIDUAL), QVariant(CRHelper::toTypeString(Ranking::Type::INDIVIDUAL)));
    comboBox->addItem(QIcon(":/material/icons/group.svg"), CRHelper::toRankingTypeString(Ranking::Type::CLUB), QVariant(CRHelper::toTypeString(Ranking::Type::CLUB)));
}

QWidget *RankingTypeDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = rankingTypeBox.data();
    comboBox->setParent(parent);

    return comboBox;
}

void RankingTypeDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void RankingTypeDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(CRHelper::toRankingTypeString(CRHelper::toRankingType(index.model()->data(index, Qt::EditRole).toString())));
}

void RankingTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::EditRole);
}

QSize RankingTypeDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return this->rankingTypeBox.data()->sizeHint();
}

void RankingTypeDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}
