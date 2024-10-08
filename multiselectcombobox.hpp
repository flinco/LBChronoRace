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

#ifndef MULTISELECTCOMBOBOX_H
#define MULTISELECTCOMBOBOX_H

#include <QComboBox>
#include <QListWidget>
#include <QLineEdit>
#include <QEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QScopedPointer>

class MultiSelectComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit MultiSelectComboBox(QWidget* aParent = Q_NULLPTR);

    void addItem(QString const &aText, QVariant const &aUserData = QVariant()); // NOSONAR
    void addItems(QStringList const &aTexts); // NOSONAR
    QStringList currentText() const; // NOSONAR
    int count() const; // NOSONAR
    void hidePopup() override;
    void setSearchBarPlaceHolderText(QString const &aPlaceHolderText) const;
    void setPlaceHolderText(QString const &aPlaceHolderText) const;
    void resetSelection() const;
    QSize sizeHint() const override;

signals:
    void selectionChanged();

public slots:
    void clear() const; // NOSONAR
    void setCurrentText(QString const &aText) const; // NOSONAR
    void setCurrentText(QStringList const &aText) const; // NOSONAR

protected:
    void wheelEvent(QWheelEvent *aWheelEvent) override;
    bool eventFilter(QObject *aObject, QEvent *aEvent) override;
    void keyPressEvent(QKeyEvent *aEvent) override;

private:
    void stateChanged(Qt::CheckState aState);
    void onSearch(QString const &aSearchString) const;
    void itemClicked(int aIndex) const;

    QScopedPointer<QListWidget> mListWidget;
};

#endif // MULTISELECTCOMBOBOX_H
