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
#include <QWindow>
#include <QStringBuilder>

#include "chronoracetimings.hpp"
#include "crloader.hpp"
#include "lbcrexception.hpp"

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
    QString startDisplay = "0:00:00";

    ui->setupUi(this);
    qApp->installEventFilter(this);
    this->setWindowFlag(Qt::CustomizeWindowHint, true);
    this->setWindowFlag(Qt::WindowCloseButtonHint, false);

    ui->timer->display(startDisplay);

    ui->dataArea->setSelectionMode(QAbstractItemView::NoSelection);

    QObject::connect(ui->dataArea, &QTableWidget::itemClicked, this, &ChronoRaceTimings::bibClicked);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChronoRaceTimings::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChronoRaceTimings::reject);

    QObject::connect(ui->startButton, &QToolButton::clicked, this, &ChronoRaceTimings::start);
    QObject::connect(ui->stopButton,  &QToolButton::clicked, this, &ChronoRaceTimings::stop );
    QObject::connect(ui->resetButton, &QToolButton::clicked, this, &ChronoRaceTimings::reset);

    QObject::connect(ui->lockToggle, &QSlider::valueChanged, this, &ChronoRaceTimings::lock);
    QObject::connect(ui->liveViewToggle, &QSlider::valueChanged, this, &ChronoRaceTimings::live);

    saveToDiskWorker.moveToThread(&saveToDiskThread);
    QObject::connect(this, &ChronoRaceTimings::saveToDisk, &saveToDiskWorker, &TimingsWorker::writeToDisk);
    QObject::connect(&saveToDiskWorker, &TimingsWorker::writeDone, this, &ChronoRaceTimings::clearDiskBuffer);

    if (QGuiApplication::screens().count() > 1) {
        ui->liveViewLabel->setEnabled(true);
        ui->liveViewToggle->setEnabled(true);
        ui->liveViewOff->setEnabled(true);
        ui->liveViewOn->setEnabled(true);
    }

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
            live(false);
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
    ui->liveViewToggle->setValue(0);

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
    ui->liveViewToggle->setValue(0);

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

void ChronoRaceTimings::setRaceData(ChronoRaceData const *newRaceData)
{
    raceData = newRaceData;
}

void ChronoRaceTimings::updateCurrentBibItem(QTableWidgetItem *newBibItem)
{
    if (this->currentBibItem) {
        if (this->currentBibItem->text().isEmpty()) {
            this->currentBibItem->setBackground(Qt::GlobalColor::red);
        } else {
            this->currentBibItem->setBackground(Qt::GlobalColor::green);
        }
    }

    this->currentBibItem = newBibItem;

    if (this->currentBibItem) {
        this->currentBibItem->setBackground(Qt::GlobalColor::yellow);
    }
}

void ChronoRaceTimings::recordTiming(qint64 milliseconds)
{
    QTableWidgetItem *timingCell;
    Qt::ItemFlags flags;

    auto hr = milliseconds / 3600000;
    milliseconds %= 3600000;
    auto min = milliseconds / 60000;
    milliseconds %= 60000;
    auto sec = milliseconds / 1000;
    milliseconds %= 1000;

    auto newTiming = QString("%1:%2:%3.%4").arg(hr).arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(milliseconds, 3, 10, QChar('0'));

    if (timingRowCount == ui->dataArea->rowCount()) {
        ui->dataArea->setRowCount(timingRowCount + 1);
        ui->dataArea->setItem(timingRowCount, 0, new QTableWidgetItem(""));
        timingCell = ui->dataArea->item(timingRowCount, 0);
        if (!this->currentBibItem) {
            updateCurrentBibItem(timingCell);
        } else {
            timingCell->setBackground(Qt::GlobalColor::red);
        }
        flags = timingCell->flags();
        flags |= Qt::ItemIsEditable;
        flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable);
        timingCell->setFlags(flags);
    }
    ui->dataArea->setItem(timingRowCount, 1, new QTableWidgetItem(newTiming));
    timingCell = ui->dataArea->item(timingRowCount, 1);
    timingCell->setData(Qt::ItemDataRole::UserRole, QVariant(static_cast<uint>(milliseconds)));
    flags = timingCell->flags();
    flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable);
    timingCell->setFlags(flags);
    ui->dataArea->scrollToBottom();
    timingRowCount += 1;
}

void ChronoRaceTimings::deleteTiming()
{
    QTableWidgetItem *bibItem;

    if (timingRowCount <= 0)
        return;

    timingRowCount -= 1;
    bibItem = ui->dataArea->item(timingRowCount, 0);

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
    int r;
    int c;

    QTableWidgetItem const *bib;
    QTableWidgetItem const *time;

    CRLoader::clearTimings();
    for (r = 0, c = 0; r < ui->dataArea->rowCount(); r++) {

        bib = ui->dataArea->item(r, 0);
        time = ui->dataArea->item(r, 1);

        if (!bib && !time) {
            continue;
        } else if (!bib) {
            emit error(tr("Missing bib for time %1").arg(time->text()));
        } else if (!time) {
            emit error(tr("Missing time for bib %1").arg(bib->text()));
        }

        CRLoader::addTiming(bib ? bib->text() : "0", time ? time->text() : "0:00:00.000");
        c++;
    }
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

void ChronoRaceTimings::toggleLiveView()
{
    auto pLiveTable = liveTable.data();

    if (liveScreen != Q_NULLPTR) {
        QTableWidgetItem const *bibItem;
        QTableWidgetItem const *timingItem;
        int rowCount = ui->dataArea->rowCount();

        pLiveTable->show();

//NOSONAR #ifdef Q_OS_WIN
//NOSONAR         pLiveTable->windowHandle()->setScreen(liveScreen);
//NOSONAR #else
        pLiveTable->windowHandle()->setGeometry(liveScreen->geometry());
//NOSONAR #endif

        liveTable->setRaceInfo(this->raceData);

        for (int row = 0; row < rowCount; row++) {
            bibItem = ui->dataArea->item(row, 0);
            if (!bibItem || bibItem->text().isEmpty())
                continue;

            timingItem = ui->dataArea->item(row, 1);
            if (!timingItem || timingItem->text().isEmpty())
                continue;

            liveTable.data()->addEntry(bibItem->text().toUInt(), timingItem->data(Qt::ItemDataRole::UserRole).toUInt());
        }
    } else {
        pLiveTable->hide();
    }

}

void ChronoRaceTimings::toggleLiveControls(bool enable)
{
    ui->liveViewLabel->setEnabled(enable);
    ui->liveViewToggle->setEnabled(enable);
    ui->liveViewOff->setEnabled(enable);
    ui->liveViewOn->setEnabled(enable);
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
        QString startDisplay = "0:00:00";
        ui->timer->display(startDisplay);
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

void ChronoRaceTimings::live(int value)
{
    auto pLiveTable = liveTable.data();

    liveScreen = Q_NULLPTR;

    if (value) {
        auto const *primaryScreen = QApplication::primaryScreen();
        for (auto *screen : QApplication::screens()) {
            if (screen == primaryScreen)
                continue;

            if (auto startList = CRLoader::getStartList(); startList.empty()) {
                emit error(tr("Enter competitors to use the Live View"));
            } else try {
                pLiveTable->setStartList(startList);
                liveScreen = screen;
            } catch (ChronoRaceException &e) {
                emit error(e.getMessage());
            }

            break;
        }
    }

    toggleLiveView();

    if ((liveScreen == Q_NULLPTR) && (value != 0))
        ui->liveViewToggle->setValue(0);
}

void ChronoRaceTimings::bibClicked(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        updateCurrentBibItem(item);
    }
}

void ChronoRaceTimings::screenRemoved(QScreen const *screen)
{
    if ((screen == liveScreen) || (screen == QApplication::primaryScreen())) {

        liveScreen = Q_NULLPTR;
        liveTable.data()->hide();

        ui->liveViewToggle->setValue(0);

        if (QGuiApplication::screens().count() < 2) {
            ui->liveViewLabel->setEnabled(false);
            ui->liveViewToggle->setEnabled(false);
            ui->liveViewOff->setEnabled(false);
            ui->liveViewOn->setEnabled(false);
        }
    }
}

void ChronoRaceTimings::screenAdded(QScreen const *screen)
{
    Q_UNUSED(screen)

    if (QGuiApplication::screens().count() > 1) {
        ui->liveViewLabel->setEnabled(true);
        ui->liveViewToggle->setEnabled(true);
        ui->liveViewOff->setEnabled(true);
        ui->liveViewOn->setEnabled(true);
    }
}
