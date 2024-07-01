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

#include "sexdelegate.hpp"
#include "lbcrexception.hpp"

SexDelegate::SexDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = box.data();
    comboBox->setEditable(false);
    comboBox->setInsertPolicy(QComboBox::NoInsert);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboBox->setDuplicatesEnabled(false);
    comboBox->addItem(QIcon(":/material/icons/unknown_med.svg"), toSexString(Competitor::Sex::UNDEFINED), Competitor::toSexString(Competitor::Sex::UNDEFINED));
    comboBox->addItem(QIcon(":/material/icons/male.svg"), toSexString(Competitor::Sex::MALE), Competitor::toSexString(Competitor::Sex::MALE));
    comboBox->addItem(QIcon(":/material/icons/female.svg"), toSexString(Competitor::Sex::FEMALE), Competitor::toSexString(Competitor::Sex::FEMALE));
}

QWidget *SexDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = box.data();
    comboBox->setParent(parent);

    return comboBox;
}

void SexDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void SexDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(toSexString(Competitor::toSex(index.model()->data(index, Qt::EditRole).toString())));
}

void SexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::EditRole);
}

void SexDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

QString SexDelegate::toSexString(Competitor::Sex const sex)
{
    switch (sex) {
    case Competitor::Sex::MALE:
        return tr("Male");
    case Competitor::Sex::FEMALE:
        return tr("Female");
    case Competitor::Sex::UNDEFINED:
        return tr("Not set");
    default:
        throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}
