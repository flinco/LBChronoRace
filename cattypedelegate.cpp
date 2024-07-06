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

#include "cattypedelegate.hpp"
#include "lbcrexception.hpp"

CategoryTypeDelegate::CategoryTypeDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = box.data();
    comboBox->setEditable(false);
    comboBox->setInsertPolicy(QComboBox::NoInsert);
    comboBox->setDuplicatesEnabled(false);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox->addItem(QIcon(":/material/icons/person.svg"), toCatTypeString(Category::Type::INDIVIDUAL), QVariant(Category::toTypeString(Category::Type::INDIVIDUAL)));
    comboBox->addItem(QIcon(":/material/icons/group.svg"), toCatTypeString(Category::Type::CLUB), QVariant(Category::toTypeString(Category::Type::CLUB)));
}

QWidget *CategoryTypeDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = box.data();
    comboBox->setParent(parent);

    return comboBox;
}

void CategoryTypeDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void CategoryTypeDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(toCatTypeString(Category::toType(index.model()->data(index, Qt::EditRole).toString())));
}

void CategoryTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::EditRole);
}

QSize CategoryTypeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return this->box.data()->sizeHint();
}

void CategoryTypeDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

QString CategoryTypeDelegate::toCatTypeString(Category::Type type)
{
    switch (type) {
    case Category::Type::INDIVIDUAL:
        return tr("Individual");
    case Category::Type::CLUB:
        return tr("Club");
    default:
        throw(ChronoRaceException(tr("Unexpected Type enum value '%1'").arg(static_cast<int>(type))));
    }
}
