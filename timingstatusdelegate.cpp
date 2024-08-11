/*****************************************************************************
 * Copyright (C) 2024 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#include "timingstatusdelegate.hpp"
#include "crhelper.hpp"

TimingStatusDelegate::TimingStatusDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
    auto *comboBox = timingStatusBox.data();
    comboBox->setEditable(false);
    comboBox->setInsertPolicy(QComboBox::NoInsert);
    comboBox->setDuplicatesEnabled(false);
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBox->addItem(QIcon(":/material/icons/classified.svg"), CRHelper::toStatusFullString(Timing::Status::CLASSIFIED), CRHelper::toStatusString(Timing::Status::CLASSIFIED));
    comboBox->addItem(QIcon(":/material/icons/dsq.svg"), CRHelper::toStatusFullString(Timing::Status::DSQ), CRHelper::toStatusString(Timing::Status::DSQ));
    comboBox->addItem(QIcon(":/material/icons/dnf.svg"), CRHelper::toStatusFullString(Timing::Status::DNF), CRHelper::toStatusString(Timing::Status::DNF));
    comboBox->addItem(QIcon(":/material/icons/dns.svg"), CRHelper::toStatusFullString(Timing::Status::DNS), CRHelper::toStatusString(Timing::Status::DNS));
}

QWidget *TimingStatusDelegate::createEditor(QWidget *parent, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto *comboBox = timingStatusBox.data();
    comboBox->setParent(parent);

    return comboBox;
}

void TimingStatusDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor)
    Q_UNUSED(index)
}

void TimingStatusDelegate::setEditorData(QWidget *editor, QModelIndex const &index) const
{
    // Get the value via index of the Model and put it into the ComboBox
    auto *comboBox = static_cast<QComboBox *>(editor);
    comboBox->setCurrentText(CRHelper::toStatusString(CRHelper::toTimingStatus(index.model()->data(index, Qt::EditRole).toString())));
}

void TimingStatusDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, QModelIndex const &index) const
{
    auto const *comboBox = static_cast<QComboBox *>(editor);
    model->setData(index, comboBox->currentData(Qt::UserRole), Qt::EditRole);
}

QSize TimingStatusDelegate::sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return this->timingStatusBox.data()->sizeHint();
}

void TimingStatusDelegate::updateEditorGeometry(QWidget *editor, QStyleOptionViewItem const &option, QModelIndex const &index) const
{
    Q_UNUSED(index)

    editor->setGeometry(option.rect);
}
