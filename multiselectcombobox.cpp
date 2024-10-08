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

#include <QCheckBox>

#include "multiselectcombobox.hpp"

namespace {
    const int scSearchBarIndex = 0;
}

MultiSelectComboBox::MultiSelectComboBox(QWidget *aParent) :
    QComboBox(aParent)
{
    QListWidget *listWidget;
    QLineEdit *searchBar;
    QLineEdit *lineEdit;

    mListWidget.reset(new QListWidget(this));
    QComboBox::setLineEdit(new QLineEdit(this));

    listWidget = mListWidget.data();
    lineEdit = QComboBox::lineEdit();

    listWidget->insertItem(0, new QListWidgetItem);
    auto curItem = listWidget->item(0);
    listWidget->setItemWidget(curItem, new QLineEdit(this));
    searchBar = static_cast<QLineEdit *>(listWidget->itemWidget(curItem));
    searchBar->setPlaceholderText(tr("Search…"));
    searchBar->setClearButtonEnabled(true);
    lineEdit->setReadOnly(true);
    lineEdit->installEventFilter(this);

    QComboBox::setModel(listWidget->model());
    QComboBox::setView(listWidget);
    QComboBox::setSizeAdjustPolicy(QComboBox::AdjustToContents);

    connect(searchBar, &QLineEdit::textChanged, this, &MultiSelectComboBox::onSearch);
    connect(this, &QComboBox::activated, this, &MultiSelectComboBox::itemClicked);
}

void MultiSelectComboBox::hidePopup()
{
    int width = this->width();
    int height = mListWidget.data()->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();
    if ((x >= 0) && (x <= width) && (y >= this->height()) && (y <= height + this->height())) {
        // Item was clicked, do not hide popup
    } else {
        QComboBox::hidePopup();
    }
}

void MultiSelectComboBox::stateChanged(Qt::CheckState aState)
{
    QListWidget const *listWidget = mListWidget.data();
    QListWidgetItem *listItem;
    QLineEdit *lineEdit = QComboBox::lineEdit();
    QCheckBox const *checkBox;
    QStringList selectedData;

    Q_UNUSED(aState)

    int count = listWidget->count();

    for (int i = 1; i < count; i++) {
        listItem = listWidget->item(i);
        checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listItem));

        if (checkBox->isChecked()) {
            selectedData.append(listItem->data(Qt::UserRole).toString());
        }
    }

    if (selectedData.isEmpty()) {
        lineEdit->clear();
    } else {
        lineEdit->setText(selectedData.join(','));
    }

    lineEdit->setToolTip(lineEdit->text());

    emit selectionChanged();
}

QStringList MultiSelectComboBox::currentText() const
{
    return QComboBox::lineEdit()->text().split(',');

}

void MultiSelectComboBox::addItem(QString const &aText, QVariant const &aUserData)
{
    QListWidget *listWidget;
    QListWidgetItem *listWidgetItem;
    QCheckBox const *checkBox;

    listWidget = mListWidget.data();

    auto numItems = listWidget->count();
    listWidget->insertItem(numItems, new QListWidgetItem);
    listWidgetItem = listWidget->item(numItems);
    listWidgetItem->setData(Qt::UserRole, aUserData);
    listWidget->setItemWidget(listWidgetItem, new QCheckBox(aText, this));

    checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listWidgetItem));
    connect(checkBox, &QCheckBox::checkStateChanged, this, &MultiSelectComboBox::stateChanged);
}

void MultiSelectComboBox::addItems(QStringList const &aTexts)
{
    for(auto const &string : aTexts) {
        addItem(string);
    }
}

int MultiSelectComboBox::count() const
{
    int count = mListWidget.data()->count() - 1; // Do not count the search bar

    return (count < 0) ? 0 : count;
}

void MultiSelectComboBox::onSearch(QString const &aSearchString) const
{
    QCheckBox const *checkBox;
    QListWidget const *listWidget = mListWidget.data();
    auto count = listWidget->count();

    for (int i = 1; i < count; i++) {
        checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listWidget->item(i)));

        if (checkBox->text().contains(aSearchString, Qt::CaseInsensitive)) {
            listWidget->item(i)->setHidden(false);
        } else {
            listWidget->item(i)->setHidden(true);
        }
    }
}

void MultiSelectComboBox::itemClicked(int aIndex) const
{
    if (aIndex != scSearchBarIndex) { // 0 means the search bar
        QListWidget const *listWidget = mListWidget.data();
        auto checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listWidget->item(aIndex)));
        checkBox->setChecked(!checkBox->isChecked());
    }
}

void MultiSelectComboBox::setSearchBarPlaceHolderText(QString const &aPlaceHolderText) const
{
    QListWidget const *listWidget = mListWidget.data();
    static_cast<QLineEdit *>(listWidget->itemWidget(listWidget->item(0)))->setPlaceholderText(aPlaceHolderText);
}

void MultiSelectComboBox::setPlaceHolderText(QString const &aPlaceHolderText) const
{
    return QComboBox::lineEdit()->setPlaceholderText(aPlaceHolderText);

}

void MultiSelectComboBox::clear() const
{
    QListWidget *listWidget = mListWidget.data();
    QListWidgetItem *searchBarItem = listWidget->takeItem(0);

    listWidget->clear();

    listWidget->insertItem(0, searchBarItem);
}

void MultiSelectComboBox::wheelEvent(QWheelEvent *aWheelEvent)
{
    // Do not handle the wheel event
    Q_UNUSED(aWheelEvent)
}

bool MultiSelectComboBox::eventFilter(QObject *aObject, QEvent *aEvent)
{
    if (QEvent::Type evType = aEvent->type();
        (aObject == QComboBox::lineEdit()) && ((evType == QEvent::MouseButtonRelease) || (evType == QEvent::KeyRelease))) {
        showPopup();
        return false;
    }
    return false;
}

void MultiSelectComboBox::keyPressEvent(QKeyEvent *aEvent)
{
    if (aEvent->key() == Qt::Key_Down)
        showPopup();
}

void MultiSelectComboBox::setCurrentText(QString const &aText) const
{
    Q_UNUSED(aText)
}

void MultiSelectComboBox::setCurrentText(QStringList const &aText) const
{
    QCheckBox *checkBox;
    QListWidget const *listWidget = mListWidget.data();
    QListWidgetItem *listItem;
    auto count = listWidget->count();

    for (int i = 1; i < count; i++) {
        listItem = listWidget->item(i);
        checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listItem));

        if (aText.contains(listItem->data(Qt::UserRole))) {
            checkBox->setChecked(true);
        }
    }
}

void MultiSelectComboBox::resetSelection() const
{
    QCheckBox *checkBox;
    QListWidget const *listWidget = mListWidget.data();
    int count = listWidget->count();

    for (int i = 1; i < count; i++) {
        checkBox = static_cast<QCheckBox *>(listWidget->itemWidget(listWidget->item(i)));
        checkBox->setChecked(false);
    }
}

QSize MultiSelectComboBox::sizeHint() const
{
    QListWidget const *listWidget = mListWidget.data();
    QLineEdit const *lineEdit = QComboBox::lineEdit();

    QSize hint = QComboBox::sizeHint();
    if (lineEdit->sizeHint().width() > hint.width())
        hint.setWidth(lineEdit->sizeHint().width());
    if (listWidget->sizeHint().width() > hint.width())
        hint.setWidth(listWidget->sizeHint().width());

    return hint;
}
