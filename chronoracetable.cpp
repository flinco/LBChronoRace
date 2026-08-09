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

#include <QMessageBox>

#include "chronoracetable.hpp"
#include "crloader.hpp"
#include "crhelper.hpp"

QHash<QStringView, QChar> const ChronoRaceTable::graveAccents = {
    { u"A''", u'À' }, { u"E''", u'È' }, { u"I''", u'Ì' }, { u"O''", u'Ò' }, { u"U''", u'Ù' },
    { u"a''", u'à' }, { u"e''", u'è' }, { u"i''", u'ì' }, { u"o''", u'ò' }, { u"u''", u'ù' }
};
QHash<QStringView, QChar> const ChronoRaceTable::acuteAccents = {
    { u"A``", u'Á' }, { u"E``", u'É' }, { u"I``", u'Í' }, { u"O``", u'Ó' }, { u"U``", u'Ú' },
    { u"A''", u'Á' }, { u"E''", u'É' }, { u"I''", u'Í' }, { u"O''", u'Ó' }, { u"U''", u'Ú' },
    { u"a``", u'á' }, { u"e``", u'é' }, { u"i``", u'í' }, { u"o``", u'ó' }, { u"u``", u'ú' },
    { u"a''", u'á' }, { u"e''", u'é' }, { u"i''", u'í' }, { u"o''", u'ó' }, { u"u''", u'ú' }
};

ChronoRaceTable::ChronoRaceTable(QWidget *parent) : QDialog(parent)
{
    ui->setupUi(this);

    ui->tableView->setSortingEnabled(true);
    ui->tableView->installEventFilter(this);

    QObject::connect(ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, ui->tableView, &QTableView::sortByColumn);
    QObject::connect(ui->rowAdd, &QPushButton::clicked, this, &ChronoRaceTable::rowAdd);
    QObject::connect(ui->rowDel, &QPushButton::clicked, this, &ChronoRaceTable::rowDel);
    QObject::connect(ui->modelImport, &QPushButton::clicked, this, &ChronoRaceTable::modelImport);
    QObject::connect(ui->modelExport, &QPushButton::clicked, this, &ChronoRaceTable::modelExport);
    QObject::connect(ui->dialogSave, &QPushButton::clicked, this, &ChronoRaceTable::saveRaceData);
    QObject::connect(ui->dialogQuit, &QPushButton::clicked, this, &ChronoRaceTable::close);
}

bool ChronoRaceTable::eventFilter(QObject *obj, QEvent *event)
{
    static bool alt { false };
    static bool ctrl { false };

    bool handled { false };

    if (obj == ui->tableView)
    {
        switch (event->type())
        {
            case QEvent::Type::KeyPress:
                if (auto const pressedKey = (static_cast<QKeyEvent *>(event))->key();
                    (pressedKey == Qt::Key::Key_Return) || (pressedKey == Qt::Key::Key_Enter)) {
                    handled = returnPressed(ui->tableView->currentIndex());
                } else if (pressedKey == Qt::Key::Key_Control) {
                    ctrl = true;
                } else if (pressedKey == Qt::Key::Key_Alt) {
                    alt = true;
                } else if (pressedKey == Qt::Key::Key_AltGr) {
                    ctrl = true;
                    alt = true;
                }
                break;
            case QEvent::Type::KeyRelease:
                if (auto const pressedKey = (static_cast<QKeyEvent *>(event))->key();
                    pressedKey == Qt::Key::Key_Apostrophe) {
                    handled = accentPressed((ctrl && alt) ? acuteAccents : graveAccents);
                } else if (pressedKey == Qt::Key::Key_QuoteLeft) {
                    handled = accentPressed(acuteAccents);
                } else if (pressedKey == Qt::Key::Key_Control) {
                    ctrl = false;
                } else if (pressedKey == Qt::Key::Key_Alt) {
                    alt = false;
                } else if (pressedKey == Qt::Key::Key_AltGr) {
                    ctrl = false;
                    alt = false;
                }
                break;
            default:
                // do nothing
                break;
        }
    }

    return handled || QDialog::eventFilter(obj, event);
}

bool ChronoRaceTable::returnPressed(QModelIndex const &current)
{
    if (!current.isValid())
        return false;

    if (auto nextColumnIndex = current.column() + 1;
        nextColumnIndex < ui->tableView->model()->columnCount()) {
        auto next = current.siblingAtColumn(nextColumnIndex);

        QMetaObject::invokeMethod(this, [this, next]() {
            ui->tableView->setCurrentIndex(next);
            //NOSONAR ui->tableView->edit(next);
        }, Qt::ConnectionType::QueuedConnection);
    } else {
        auto model = ui->tableView->model();
        //NOSONAR auto const nextRowIndex = model->rowCount();
        auto const nextRowIndex = current.row() + 1;

        QMetaObject::invokeMethod(this, [this, model, nextRowIndex]() {
            model->insertRow(nextRowIndex);
            auto first = model->index(nextRowIndex, 0);
            ui->tableView->setCurrentIndex(first);
            //NOSONAR ui->tableView->edit(first);
        }, Qt::ConnectionType::QueuedConnection);
    }

    return true;
}

bool ChronoRaceTable::accentPressed(QHash<QStringView, QChar> const &accents) const
{
    if (auto *edit = qobject_cast<QLineEdit *>(QApplication::focusWidget()); edit) {
        if (int pos = edit->cursorPosition(); pos >= 3) {
            QString text = edit->text();
            QStringView last = QStringView{ text }.mid(pos - 3, 3);
            if (auto it = accents.find(last); it != accents.end()) {
                text.replace(pos - 3, 3, it.value());
                edit->setText(text);
                edit->setCursorPosition(pos - 2);
                return true;
            }
        }
    }

    return false;
}

QAbstractTableModel *ChronoRaceTable::getModel() const
{
    return qobject_cast<QAbstractTableModel *>(ui->tableView->model());
}

void ChronoRaceTable::setModel(CRTableModel *model) const
{
    ui->tableView->setModel(model);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QObject::connect(this, &ChronoRaceTable::finished, model, &CRTableModel::refreshCounters);
}

void ChronoRaceTable::disableButtons() const
{
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->rowAdd->setEnabled(false);
    ui->rowDel->setEnabled(false);
    //NOSONAR ui->modelImport->setEnabled(false);
    //NOSONAR ui->modelExport->setEnabled(false);
}

void ChronoRaceTable::setItemDelegateForColumn(int column, QAbstractItemDelegate *delegate)
{
    ui->tableView->setItemDelegateForColumn(column, delegate);
}

void ChronoRaceTable::show()
{
    CRTableModel *model = qobject_cast<CRTableModel *>(ui->tableView->model());
    if (model->rowCount() == 0) {
        if (qobject_cast<StartListModel *>(model)) {
            model->insertRows(0,1);
        } else if (qobject_cast<RankingsModel *>(model)) {
            model->insertRows(0,1);
        } else if (qobject_cast<CategoriesModel *>(model)) {
            model->insertRows(0,1);
        } else if (qobject_cast<TimingsModel *>(model)) {
            // nothing to add
        } else if (qobject_cast<TeamsListModel *>(model)) {
            // nothing to add
        } else {
            Q_UNREACHABLE();
        }
    }
    ui->retranslateUi(this);
    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
    if (model->needsResizing())
        model->resizeHeaders(ui->tableView);
}

void ChronoRaceTable::rowAdd() const
{
    if (ui->tableView->selectionModel()->hasSelection()) {
        int rowCount = ui->tableView->model()->rowCount();
        for (int r = rowCount - 1; r >= 0; --r) {
            if (ui->tableView->selectionModel()->rowIntersectsSelection(r, QModelIndex())) {
                ui->tableView->model()->insertRow(r + 1, QModelIndex());
            }
        }
    } else {
        ui->tableView->model()->insertRow(ui->tableView->model()->rowCount(), QModelIndex());
    }
}

void ChronoRaceTable::rowDel() const
{
    if (ui->tableView->selectionModel()->hasSelection()) {
        int rowCount = ui->tableView->model()->rowCount();
        for (int r = rowCount - 1; r >= 0; --r) {
            if (ui->tableView->selectionModel()->rowIntersectsSelection(r, QModelIndex())) {
                ui->tableView->model()->removeRow(r, QModelIndex());
            }
        }
    }
}

void ChronoRaceTable::modelImport()
{
    if (QMessageBox::question(this, tr("CSV Encoding"),
                              tr("The data being imported must be %1 encoded.\n"
                                 "Continue?").arg(CRHelper::encodingToLabel(CRLoader::getEncoding())),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        emit modelImported();
    }
}

void ChronoRaceTable::modelExport()
{
    if (QMessageBox::question(this, tr("CSV Encoding"),
                              tr("The data will be exported with %1 encoding.\n"
                                 "Continue?").arg(CRHelper::encodingToLabel(CRLoader::getEncoding())),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
        emit modelExported();
    }
}

void ChronoRaceTable::dialogSave()
{
    emit saveRaceData();
}
