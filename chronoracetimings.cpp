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

#include "chronoracetimings.hpp"
#include "crloader.hpp"
#include "lbcrexception.hpp"

TimingsWorker::TimingsWorker() {

    QString outPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    outPath.append("/lbchronorace_XXXXXX.csv");

    QTemporaryFile outFile(outPath);
    if (outFile.open()) {
        timingsFilePath.append(outFile.fileName());
    } else {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFile.fileName())));
    }
}

void TimingsWorker::writeToDisk(QString const &buffer) {
    QFile outFile(timingsFilePath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFile.fileName())));
    }
    QTextStream outStream(&outFile);

    if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
        outStream.setEncoding(QStringConverter::Utf8);
    else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
        outStream.setEncoding(QStringConverter::Latin1);

    outStream << buffer;
    outStream.flush();
    outFile.close();

    emit writeDone();
};


ChronoRaceTimings::ChronoRaceTimings(QWidget *parent) : QDialog(parent) {

    QString startDisplay = "0:00:00";

    ui->setupUi(this);
    qApp->installEventFilter(this);

    ui->timer->display(startDisplay);

    ui->dataArea->setSelectionMode(QAbstractItemView::NoSelection);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChronoRaceTimings::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChronoRaceTimings::reject);

    QObject::connect(ui->startButton, &QToolButton::clicked, this, &ChronoRaceTimings::start);
    QObject::connect(ui->stopButton,  &QToolButton::clicked, this, &ChronoRaceTimings::stop );
    QObject::connect(ui->resetButton, &QToolButton::clicked, this, &ChronoRaceTimings::reset);

    QObject::connect(ui->lockBox, &QCheckBox::clicked, this, &ChronoRaceTimings::lock);
    QObject::connect(ui->unlockBox, &QCheckBox::clicked, this, &ChronoRaceTimings::lock);

    saveToDiskWorker.moveToThread(&saveToDiskThread);
    connect(this, &ChronoRaceTimings::saveToDisk, &saveToDiskWorker, &TimingsWorker::writeToDisk);
    connect(&saveToDiskWorker, &TimingsWorker::writeDone, this, &ChronoRaceTimings::clearDiskBuffer);

    ui->unlockBox->hide();
    ui->stopButton->setEnabled(false);
}

bool ChronoRaceTimings::eventFilter(QObject *watched, QEvent *event)
{
    bool retval = true;
    if (this->isVisible() && (event->type() == QEvent::KeyPress)) {
        auto const keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Space:
            if (updateTimerId != 0)
                recordTiming(this->timer.elapsed() / 1000);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            bibBuffer.clear();
            break;
        case Qt::Key_Backspace:
            deleteBib();
            break;
        case Qt::Key_Delete:
            deleteTiming();
            break;
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            recordBib(keyEvent->text());
            break;
        default:
            retval = QDialog::eventFilter(watched, event);
            break;
        }
    } else {
        retval = QDialog::eventFilter(watched, event);
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

        ui->timer->display(QString("%1:%2:%3").arg(elapsed / 3600).arg(elapsed / 60, 2, 10, QChar('0')).arg(elapsed % 60, 2, 10, QChar('0')));
    }
}

void ChronoRaceTimings::accept()
{
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

void ChronoRaceTimings::recordTiming(qint64 seconds)
{
    auto newTiming = QString("%1:%2:%3").arg(seconds / 3600).arg(seconds / 60, 2, 10, QChar('0')).arg(seconds % 60, 2, 10, QChar('0'));

    if (timingRowCount == ui->dataArea->rowCount())
        ui->dataArea->setRowCount(timingRowCount + 1);
    ui->dataArea->setItem(timingRowCount, 1, new QTableWidgetItem(newTiming));
    timingRowCount += 1;
}

void ChronoRaceTimings::deleteTiming()
{
    if (timingRowCount > 0) {
        ui->dataArea->item(timingRowCount - 1, 1)->setText(QString());
        if (timingRowCount-- > bibRowCount)
            ui->dataArea->setRowCount(timingRowCount);
    }
}

void ChronoRaceTimings::recordBib(QString const &key)
{
    if (bibBuffer.isEmpty()) {
        if (bibRowCount == ui->dataArea->rowCount())
            ui->dataArea->setRowCount(bibRowCount + 1);
        ui->dataArea->setItem(bibRowCount, 0, new QTableWidgetItem(bibBuffer));
        bibRowCount += 1;
    }

    bibBuffer.append(key);
    ui->dataArea->item(bibRowCount - 1, 0)->setText(bibBuffer);
}

void ChronoRaceTimings::deleteBib()
{
    bibBuffer.removeLast();
    if (bibBuffer.isEmpty()) {
        if (bibRowCount > 0) {
            if (bibRowCount-- > timingRowCount)
                ui->dataArea->setRowCount(bibRowCount);
            else
                ui->dataArea->item(bibRowCount, 0)->setText(bibBuffer);
            if (bibRowCount)
                bibBuffer = ui->dataArea->item(bibRowCount - 1, 0)->text();
        }
    } else {
        ui->dataArea->item(bibRowCount - 1, 0)->setText(bibBuffer);
    }
}

void ChronoRaceTimings::saveTimings()
{
    int r;
    int c;

    CRLoader::clearTimings();
    for (r = 0, c = 0; r < ui->dataArea->rowCount(); r++)
        if (ui->dataArea->item(r, 0) && ui->dataArea->item(r, 1)) {
            CRLoader::addTiming(ui->dataArea->item(r, 0)->text(), ui->dataArea->item(r, 1)->text());
            c++;
        } else if (ui->dataArea->item(r, 0)) {
            emit error(tr("Droped bib %1 due to missing time").arg(ui->dataArea->item(r, 0)->text()));
        } else if (ui->dataArea->item(r, 1)) {
            emit error(tr("Dropped time %1 due to missing bib").arg(ui->dataArea->item(r, 1)->text()));
        }

    emit newTimingsCount(c);
}

void ChronoRaceTimings::start()
{
    saveToDiskThread.start();

    this->timer.start();
    if (updateTimerId == 0)
        updateTimerId = startTimer(100);
    if (backupTimerId == 0)
        backupTimerId = startTimer(1000);

    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->resetButton->setEnabled(false);
    ui->buttonBox->setEnabled(false);
    this->setWindowFlag(Qt::WindowCloseButtonHint, false);
    this->show();
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
    ui->buttonBox->setEnabled(true);
    this->setWindowFlag(Qt::WindowCloseButtonHint, true);
    this->show();
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
        bibBuffer.clear();
        bibRowCount = 0;
        timingRowCount = 0;
        ui->dataArea->clearContents();
        ui->dataArea->setRowCount(0);
    }
}

void ChronoRaceTimings::lock(bool checked)
{
    if (checked) {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        ui->unlockBox->show();
        ui->lockBox->hide();
        ui->lockBox->setChecked(false);
    } else {
        ui->startButton->setEnabled(updateTimerId == 0);
        ui->stopButton->setEnabled(updateTimerId != 0);
        ui->resetButton->setEnabled(updateTimerId == 0);
        ui->buttonBox->setEnabled(true);
        ui->lockBox->show();
        ui->unlockBox->hide();
        ui->unlockBox->setChecked(true);
    }
    this->setWindowFlag(Qt::WindowCloseButtonHint, !checked);
    this->show();
}
