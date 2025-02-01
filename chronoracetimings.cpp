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

#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStringBuilder>

#include "chronoracetimings.hpp"
#include "timespandialog.hpp"
#include "crloader.hpp"
#include "lbcrexception.hpp"

constexpr char DISPLAY_CHRONO_ZERO[] = "0:00:00";

TimingsWorker::TimingsWorker()
{
    QTemporaryFile outFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) % "/lbchronorace_XXXXXX.csv");
    if (outFile.open()) {
        timingsFilePath.append(outFile.fileName());
    } else {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFile.fileName())));
    }
}

void TimingsWorker::writeToDisk(QString const &buffer)
{
    QFile outFile(timingsFilePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFile.fileName())));
    }
    QTextStream outStream(&outFile);

    outStream.setEncoding(CRLoader::getEncoding());

    outStream << buffer;
    outStream.flush();
    outFile.close();

    emit writeDone();
};

ChronoRaceTimings::ChronoRaceTimings(QWidget *parent) : QDialog(parent)
{
    ui->setupUi(this);
    qApp->installEventFilter(this);
    this->setWindowFlag(Qt::CustomizeWindowHint, true);
    this->setWindowFlag(Qt::WindowCloseButtonHint, false);

    ui->timer->display(DISPLAY_CHRONO_ZERO);

    ui->dataArea->setSelectionMode(QAbstractItemView::NoSelection);

    QObject::connect(ui->dataArea, &QTableWidget::itemClicked, this, &ChronoRaceTimings::bibClicked);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChronoRaceTimings::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChronoRaceTimings::reject);

    QObject::connect(ui->startButton, &QToolButton::clicked, this, &ChronoRaceTimings::start);
    QObject::connect(ui->stopButton,  &QToolButton::clicked, this, &ChronoRaceTimings::stop );
    QObject::connect(ui->resetButton, &QToolButton::clicked, this, &ChronoRaceTimings::reset);

    QObject::connect(ui->lockToggle, &QSlider::valueChanged, this, &ChronoRaceTimings::lock);

    saveToDiskWorker.moveToThread(&saveToDiskThread);
    QObject::connect(this, &ChronoRaceTimings::saveToDisk, &saveToDiskWorker, &TimingsWorker::writeToDisk);
    QObject::connect(&saveToDiskWorker, &TimingsWorker::writeDone, this, &ChronoRaceTimings::clearDiskBuffer);

    ui->stopButton->setEnabled(false);
}

bool ChronoRaceTimings::eventFilter(QObject *watched, QEvent *event)
{
    bool retval = true;
    if (!this->isVisible() || (event->type() != QEvent::Type::KeyPress)) {
        retval = QDialog::eventFilter(watched, event);
    } else {
        auto const keyEvent = static_cast<QKeyEvent *>(event);
        auto const keyModifiers = keyEvent->modifiers();
        switch (keyEvent->key()) {
            case Qt::Key::Key_F10:
                if (!(keyModifiers & Qt::KeyboardModifier::AltModifier)) {
                    retval = QDialog::eventFilter(watched, event);
                    break;
                }
                [[fallthrough]];
            case Qt::Key::Key_Space:
                if (updateTimerId != 0)
                    recordTiming(this->timer.elapsed());
                break;
            case Qt::Key::Key_Return:
                [[fallthrough]];
            case Qt::Key::Key_Enter:
                retval = enterPressed();
                break;
            case Qt::Key::Key_Backspace:
                deleteBib();
                break;
            case Qt::Key::Key_Delete:
                if (keyModifiers & Qt::KeyboardModifier::AltModifier)
                    deleteTiming();
                else
                    deleteBib();
                break;
            case Qt::Key::Key_0:
                [[fallthrough]];
            case Qt::Key::Key_1:
                [[fallthrough]];
            case Qt::Key::Key_2:
                [[fallthrough]];
            case Qt::Key::Key_3:
                [[fallthrough]];
            case Qt::Key::Key_4:
                [[fallthrough]];
            case Qt::Key::Key_5:
                [[fallthrough]];
            case Qt::Key::Key_6:
                [[fallthrough]];
            case Qt::Key::Key_7:
                [[fallthrough]];
            case Qt::Key::Key_8:
                [[fallthrough]];
            case Qt::Key::Key_9:
                retval = digitPressed((static_cast<QKeyEvent *>(event))->text());
                break;
            case Qt::Key::Key_Up:
                retval = upPressed();
                break;
            case Qt::Key::Key_Down:
                retval = downPressed();
                break;
            case Qt::Key::Key_Escape:
                if (ui->lockToggle->value() == 0)
                    retval = QDialog::eventFilter(watched, event);
                break;
            default:
                retval = QDialog::eventFilter(watched, event);
                break;
        }
    }

    return retval;
}

void ChronoRaceTimings::timerEvent(QTimerEvent *event) {

    if (event->timerId() == backupTimerId) {
        QString buffer;
        QTextStream outStream(&buffer);
        QTableWidgetItem const *item0;
        QTableWidgetItem const *item1;
        for (int r = 0; r < ui->dataArea->rowCount(); r++) {
            item0 = ui->dataArea->item(r, 0);
            item1 = ui->dataArea->item(r, 1);
            outStream << (item0 ? item0->text() : "XXXX") << ",0," << (item1 ? item1->text() : "-:--:--") << Qt::endl;
            outStream.flush();
        }

        saveToDiskQueue.append(buffer);

        emit saveToDisk(saveToDiskQueue.constLast());
    } else if (event->timerId() == updateTimerId) {
        qint64 elapsed = this->timer.elapsed() / 1000;

        ui->timer->display(QString("%1:%2:%3").arg(elapsed / 3600).arg((elapsed % 3600) / 60, 2, 10, QChar('0')).arg(elapsed % 60, 2, 10, QChar('0')));
    }
}

void ChronoRaceTimings::accept()
{
    if (saveToDiskThread.isRunning())
        this->stop();

    if (QMessageBox::information(this, tr("Save Timings List"),
                                     tr("The timings list will be replaced by the current data.\n"
                                        "Any previously recorded timing will be lost.\n"
                                        "Do you want to save the recorded timings?"),
                                     QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Save) == QMessageBox::Save) {
        // Save was clicked
        if (updateTimerId != 0) {
            killTimer(updateTimerId);
            updateTimerId = 0;
        }
        if (backupTimerId != 0) {
            killTimer(backupTimerId);
            backupTimerId = 0;
        }
        saveTimings();
        QDialog::accept();
    }
}

void ChronoRaceTimings::reject()
{
    if (saveToDiskThread.isRunning())
        this->stop();

    if (QMessageBox::information(this, tr("Discard Timings List"),
                                 tr("The current data will be discarded.\n"
                                    "The previously recorded timings list will be preserved.\n"
                                    "Do you want to discard the recorded timings?"),
                                 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard) {
        // Discard was clicked
        if (updateTimerId != 0) {
            killTimer(updateTimerId);
            updateTimerId = 0;
        }
        if (backupTimerId != 0) {
            killTimer(backupTimerId);
            backupTimerId = 0;
        }

        ui->timer->display(DISPLAY_CHRONO_ZERO);
        currentBibItem = Q_NULLPTR;
        timingRowCount = 0;
        ui->dataArea->clearContents();
        ui->dataArea->setRowCount(0);

        QDialog::reject();
    }
}

void ChronoRaceTimings::show()
{
    ui->retranslateUi(this);
    this->setWindowModality(Qt::ApplicationModal);

    QDialog::show();
}

void ChronoRaceTimings::clearDiskBuffer() {
    if (!saveToDiskQueue.isEmpty())
        saveToDiskQueue.removeFirst();
}

void ChronoRaceTimings::addTimeSpan()
{
    try {
        TimeSpanDialog addDialog(this, tr("Time span"), tr("Time interval to add up"), false);

        auto const &dialogHandle = QObject::connect(&addDialog, &TimeSpanDialog::applyOffset, this, &ChronoRaceTimings::applyTimeSpan);

        addDialog.setModal(true);
        addDialog.exec();

        QObject::disconnect(dialogHandle);

    } catch (ChronoRaceException &e) {
        emit error(e.getMessage());
    }
}

void ChronoRaceTimings::subtractTimeSpan()
{
    try {
        TimeSpanDialog subDialog(this, tr("Time span"), tr("Time interval to subtract"), true);

        auto const &dialogHandle = QObject::connect(&subDialog, &TimeSpanDialog::applyOffset, this, &ChronoRaceTimings::applyTimeSpan);

        subDialog.setModal(true);
        subDialog.exec();

        QObject::disconnect(dialogHandle);

    } catch (ChronoRaceException &e) {
        emit error(e.getMessage());
    }
}

void ChronoRaceTimings::applyTimeSpan(int offset)
{
    if (offset > 0) {
        QString text = tr("%n second(s) will be added to all the recorded timings.\nAre you sure you want to continue?", "", offset); // turn to milliseconds
        if (QMessageBox::warning(this, QString("Apply Time Span"), text, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No) == QMessageBox::StandardButton::Yes) {
            (dynamic_cast<TimingsModel *>(CRLoader::getTimingsModel()))->addTimeSpan(offset);
        }
    } else if (offset < 0) {
        QString text = tr("%n second(s) will be subtracted from all the recorded timings.\nTimings resulting below 0 will be set to 0:00:00.000.\nAre you sure you want to continue?", "", -offset); // turn to milliseconds
        if (QMessageBox::warning(this, QString("Apply Time Span"), text, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No) == QMessageBox::StandardButton::Yes) {
            (dynamic_cast<TimingsModel *>(CRLoader::getTimingsModel()))->addTimeSpan(offset);
        }
    }
}

void ChronoRaceTimings::setLiveTable(LiveTable *newLiveTable)
{
    liveTable = newLiveTable;
}

void ChronoRaceTimings::updateCurrentBibItem(QTableWidgetItem *newBibItem)
{
    if (this->currentBibItem) {
        Qt::GlobalColor itemColor = this->currentBibItem->text().isEmpty() ? Qt::GlobalColor::red : Qt::GlobalColor::green;

        QTableWidgetItem const *timingItem = ui->dataArea->item(this->currentBibItem->row(), 1);
        quint64 bibLiveValue = this->currentBibItem->data(Qt::ItemDataRole::UserRole).toULongLong();
        quint64 timingLiveValue = timingItem ? timingItem->data(Qt::ItemDataRole::UserRole).toULongLong() : Q_UINT64_C(0);

        if (liveTable != Q_NULLPTR)
            liveTable->removeEntry(timingLiveValue | bibLiveValue);
        bibLiveValue = ((itemColor == Qt::GlobalColor::green) ? static_cast<quint64>(this->currentBibItem->text().toUInt()) : Q_UINT64_C(0)) & Q_UINT64_C(0xffffffff);
        this->currentBibItem->setData(Qt::ItemDataRole::UserRole, QVariant(bibLiveValue));
        if (liveTable != Q_NULLPTR)
            liveTable->addEntry(timingLiveValue | bibLiveValue);

        this->currentBibItem->setBackground(itemColor);
    }

    this->currentBibItem = newBibItem;

    if (this->currentBibItem) {
        this->currentBibItem->setBackground(Qt::GlobalColor::yellow);
    }
}

void ChronoRaceTimings::recordTiming(qint64 milliseconds)
{
    QTableWidgetItem *bibItem;
    QTableWidgetItem *timingItem;
    Qt::ItemFlags flags;

    auto bibLiveValue = Q_UINT64_C(0);
    auto timingLiveValue = (static_cast<quint64>(milliseconds) << 32);

    auto msec = milliseconds;
    auto hr = msec / 3600000;
    msec %= 3600000;
    auto min = msec / 60000;
    msec %= 60000;
    auto sec = msec / 1000;
    msec %= 1000;

    auto newTiming = QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(msec, 3, 10, QChar('0'));

    if (timingRowCount == ui->dataArea->rowCount()) {
        ui->dataArea->setRowCount(timingRowCount + 1);
        ui->dataArea->setItem(timingRowCount, 0, new QTableWidgetItem(""));
        bibItem = ui->dataArea->item(timingRowCount, 0);
        if (!this->currentBibItem) {
            updateCurrentBibItem(bibItem);
        } else {
            bibItem->setBackground(Qt::GlobalColor::red);
        }
        flags = bibItem->flags();
        flags |= Qt::ItemIsEditable;
        flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable);
        bibItem->setFlags(flags);
        bibItem->setData(Qt::ItemDataRole::UserRole, QVariant(bibLiveValue));
    } else {
        bibItem = ui->dataArea->item(timingRowCount, 0);
        bibLiveValue = bibItem->data(Qt::ItemDataRole::UserRole).toULongLong();
    }
    ui->dataArea->setItem(timingRowCount, 1, new QTableWidgetItem(newTiming));
    timingItem = ui->dataArea->item(timingRowCount, 1);
    timingItem->setData(Qt::ItemDataRole::UserRole, QVariant(timingLiveValue));
    flags = timingItem->flags();
    flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable);
    timingItem->setFlags(flags);

    if (liveTable != Q_NULLPTR)
        liveTable->addEntry(timingLiveValue | bibLiveValue);

    ui->dataArea->scrollToBottom();
    timingRowCount += 1;
}

void ChronoRaceTimings::deleteTiming()
{
    QTableWidgetItem *bibItem;
    QTableWidgetItem const *timingItem;

    if (timingRowCount <= 0)
        return;

    timingRowCount -= 1;
    timingItem = ui->dataArea->item(timingRowCount, 1);
    bibItem = ui->dataArea->item(timingRowCount, 0);

    if (liveTable != Q_NULLPTR)
        liveTable->removeEntry(timingItem->data(Qt::ItemDataRole::UserRole).toULongLong() | bibItem->data(Qt::ItemDataRole::UserRole).toULongLong());

    if (((timingRowCount + 1) == ui->dataArea->rowCount()) && bibItem->text().isEmpty()) {
        int currentBibRow = this->currentBibItem ? this->currentBibItem->row() : -1;

        if (bibItem == this->currentBibItem)
            updateCurrentBibItem(Q_NULLPTR);

        ui->dataArea->setRowCount(timingRowCount);

        if (currentBibRow >= timingRowCount) {
            bibItem = (timingRowCount > 0) ? ui->dataArea->item(timingRowCount - 1, 0) : Q_NULLPTR;
            if (bibItem && bibItem->text().isEmpty())
                updateCurrentBibItem(bibItem);
        }

    } else {
        ui->dataArea->item(timingRowCount, 1)->setText("");
    }
}

void ChronoRaceTimings::deleteBib()
{
    if (!this->currentBibItem) {
        stepUp();
    } else {
        int row = this->currentBibItem->row();
        int rowCount = ui->dataArea->rowCount();

        QString buffer = this->currentBibItem->text();

        if (buffer.isEmpty()) {
            stepUp();
        } else {
            buffer.removeLast();
            this->currentBibItem->setText(buffer);
        }

        if (buffer.isEmpty() && (row >= timingRowCount) && ((row + 1) == rowCount)) {
            updateCurrentBibItem(Q_NULLPTR);
            ui->dataArea->setRowCount(rowCount - 1);
        }
    }
}

void ChronoRaceTimings::saveTimings()
{
    QTableWidgetItem const *bib;
    QTableWidgetItem const *time;

    using enum CRLoader::Action;

    CRLoader::addTiming(BEGIN);
    for (int r = 0; r < ui->dataArea->rowCount(); r++) {

        bib = ui->dataArea->item(r, 0);
        time = ui->dataArea->item(r, 1);

        if (!bib && !time) {
            continue;
        } else if (!bib) {
            emit error(tr("Missing bib for time %1").arg(time->text()));
        } else if (!time) {
            emit error(tr("Missing time for bib %1").arg(bib->text()));
        }

        CRLoader::addTiming(ADD, bib ? bib->text() : "0", time ? time->text() : "0:00:00.000");
    }
    CRLoader::addTiming(END);
}

void ChronoRaceTimings::stepUp()
{
    int row;

    if (!this->currentBibItem) {
        row = ui->dataArea->rowCount();
        updateCurrentBibItem((row > 0) ? ui->dataArea->item(row - 1, 0) : Q_NULLPTR);
    } else if ((row = this->currentBibItem->row()) > 0) {
        updateCurrentBibItem(ui->dataArea->item(row - 1, 0));
    }
}

void ChronoRaceTimings::stepDown()
{
    if (this->currentBibItem) {
        int row = this->currentBibItem->row();
        if (row + 1 < ui->dataArea->rowCount()) {
            updateCurrentBibItem(ui->dataArea->item(row + 1, 0));
        } else if (!this->currentBibItem->text().isEmpty()) {
            updateCurrentBibItem(Q_NULLPTR);
        }
    }
}

bool ChronoRaceTimings::enterPressed()
{
    if (this->currentBibItem /*&& !this->currentBibItem->text().isEmpty()*/) {

        int rowCount = ui->dataArea->rowCount();
        int row = this->currentBibItem->row();

        QTableWidgetItem *bibItem;

        updateCurrentBibItem(Q_NULLPTR);
        while (++row < rowCount) {
            bibItem = ui->dataArea->item(row, 0);
            if (bibItem->text().isEmpty()) {
                updateCurrentBibItem(bibItem);
                break;
            }
        }
    }

    return true;
}

bool ChronoRaceTimings::digitPressed(QString const &key)
{
    QString buffer;

    if (!this->currentBibItem) {
        Qt::ItemFlags flags;
        int rowCount = ui->dataArea->rowCount();
        ui->dataArea->setRowCount(rowCount + 1);
        ui->dataArea->setItem(rowCount, 0, new QTableWidgetItem(buffer));
        updateCurrentBibItem(ui->dataArea->item(rowCount, 0));
        flags = this->currentBibItem->flags();
        flags |= Qt::ItemIsEditable;
        flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable);
        this->currentBibItem->setFlags(flags);
        ui->dataArea->scrollToBottom();
    }

    buffer = this->currentBibItem->text();
    buffer.append(key);
    this->currentBibItem->setBackground(Qt::GlobalColor::yellow);
    this->currentBibItem->setText(buffer);

    return true;
}

bool ChronoRaceTimings::upPressed()
{
    stepUp();

    return true;
}

bool ChronoRaceTimings::downPressed()
{
    stepDown();

    return true;
}

void ChronoRaceTimings::start()
{
    saveToDiskThread.start();

    this->timer.start();
    if (updateTimerId == 0)
        updateTimerId = startTimer(100, Qt::TimerType::PreciseTimer);
    if (backupTimerId == 0)
        backupTimerId = startTimer(1000, Qt::TimerType::VeryCoarseTimer);

    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->resetButton->setEnabled(false);
}

void ChronoRaceTimings::stop()
{
    saveToDiskThread.quit();
    saveToDiskThread.wait();

    if (updateTimerId != 0) {
        killTimer(updateTimerId);
        updateTimerId = 0;
    }
    if (backupTimerId != 0) {
        killTimer(backupTimerId);
        backupTimerId = 0;
    }

    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->resetButton->setEnabled(true);
}

void ChronoRaceTimings::reset()
{
    if (QMessageBox::information(this, tr("Reset Timings List"),
                                 tr("The bibs and timings list will be cleared.\n"
                                    "All recorded timings and bibs will be deleted.\n"
                                    "Continue?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        // Yes was clicked
        ui->timer->display(DISPLAY_CHRONO_ZERO);
        currentBibItem = Q_NULLPTR;
        timingRowCount = 0;
        ui->dataArea->clearContents();
        ui->dataArea->setRowCount(0);
    }
}

void ChronoRaceTimings::lock(int value)
{
    if (value) {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->buttonBox->setEnabled(false);
    } else {
        ui->startButton->setEnabled(updateTimerId == 0);
        ui->stopButton->setEnabled(updateTimerId != 0);
        ui->resetButton->setEnabled(updateTimerId == 0);
        ui->buttonBox->setEnabled(true);
    }
}

void ChronoRaceTimings::bibClicked(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        updateCurrentBibItem(item);
    }
}
