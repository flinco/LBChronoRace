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

#include "catsexdelegate.hpp"
#include "lbcrexception.hpp"

CategorySexDelegate::CategorySexDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = box.data();
    comboBox->setEditable(false);
    comboBox->setInsertPolicy(QComboBox::NoInsert);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboBox->setDuplicatesEnabled(false);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox->addItem(QIcon(":/material/icons/agender.svg"), toCatSexString(Competitor::Sex::UNDEFINED), Competitor::toSexString(Competitor::Sex::UNDEFINED));
    comboBox->addItem(QIcon(":/material/icons/male.svg"), toCatSexString(Competitor::Sex::MALE), Competitor::toSexString(Competitor::Sex::MALE));
    comboBox->addItem(QIcon(":/material/icons/female.svg"), toCatSexString(Competitor::Sex::FEMALE), Competitor::toSexString(Competitor::Sex::FEMALE));
    comboBox->addItem(QIcon(":/material/icons/transgender.svg"), toCatSexString(Competitor::Sex::MISC), Competitor::toSexString(Competitor::Sex::MISC));
}

QWidget *CategorySexDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = box.data();
    comboBox->setParent(parent);

    return comboBox;
}

void CategorySexDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void CategorySexDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(toCatSexString(Competitor::toSex(index.model()->data(index, Qt::EditRole).toString())));
}

void CategorySexDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::EditRole);
}

QSize CategorySexDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return this->box.data()->sizeHint();
}

void CategorySexDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}

QString CategorySexDelegate::toCatSexString(Competitor::Sex const sex)
{
    switch (sex) {
    case Competitor::Sex::MALE:
        return tr("Men");
    case Competitor::Sex::FEMALE:
        return tr("Women");
    case Competitor::Sex::MISC:
        return tr("Mixed");
    case Competitor::Sex::UNDEFINED:
        return tr("All");
    default:
        throw(ChronoRaceException(tr("Unexpected Sex enum value '%1'").arg(static_cast<int>(sex))));
    }
}
