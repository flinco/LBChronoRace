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
#include <QTextStream>
#include <QStandardPaths>
#include <QMessageBox>

#include "chronoracetimings.h"
#include "crloader.h"
#include "lbcrexception.h"

constexpr uint TIMES_PER_SECOND = 5u;

ChronoRaceTimings::ChronoRaceTimings(QWidget *parent) : QDialog(parent) {

    QString startDisplay = "0:00:00";

    ui->setupUi(this);
    qApp->installEventFilter(this);

    ui->timer->display(startDisplay);

    ui->dataArea->setSelectionMode(QAbstractItemView::NoSelection);

    QObject::connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChronoRaceTimings::accept);
    QObject::connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChronoRaceTimings::reject);

    QObject::connect(ui->startButton, &QPushButton::clicked, this, &ChronoRaceTimings::start);
    QObject::connect(ui->stopButton,  &QPushButton::clicked, this, &ChronoRaceTimings::stop );
    QObject::connect(ui->resetButton, &QPushButton::clicked, this, &ChronoRaceTimings::reset);

    QObject::connect(ui->lockBox, &QCheckBox::clicked, this, &ChronoRaceTimings::lock);

    QObject::connect(&clock, &QTimer::timeout, this, &ChronoRaceTimings::update    );
    QObject::connect(&clock, &QTimer::timeout, this, &ChronoRaceTimings::saveToDisk);
    this->clock.setTimerType(Qt::PreciseTimer);

    ui->stopButton->setEnabled(false);
}

bool ChronoRaceTimings::eventFilter(QObject *watched, QEvent *event)
{
    bool retval = true;
    if (this->isVisible() && (event->type() == QEvent::KeyPress)) {
        auto const keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Space:
            if (this->clock.isActive())
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

void ChronoRaceTimings::accept()
{
    if (QMessageBox::information(this, tr("Save Timings List"),
                                     tr("The timings list will be replaced by the current data.\n"
                                        "Any previously recorded timing will be lost.\n"
                                        "Do you want to save the recorded timings?"),
                                     QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Save) == QMessageBox::Save) {
        // Save was clicked
        if (this->clock.isActive())
            this->clock.stop();
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
        if (this->clock.isActive())
            this->clock.stop();
        QDialog::reject();
    }
}

void ChronoRaceTimings::show()
{
    ui->retranslateUi(this);
    this->setWindowModality(Qt::ApplicationModal);
    QDialog::show();
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

    CRLoader::clearTimings();
    for (r = 0; r < ui->dataArea->rowCount(); r++)
        CRLoader::addTiming(ui->dataArea->item(r, 0)->text(), ui->dataArea->item(r, 1)->text());

    emit newTimingsCount(r);
}

void ChronoRaceTimings::start()
{
    this->timer.start();
    this->clock.start(1000 / TIMES_PER_SECOND);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->resetButton->setEnabled(false);
    ui->buttonBox->setEnabled(false);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);
    this->show();
}

void ChronoRaceTimings::stop()
{
    this->clock.stop();
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->resetButton->setEnabled(true);
    ui->buttonBox->setEnabled(true);
    this->setWindowFlags(this->windowFlags() | Qt::WindowCloseButtonHint);
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

void ChronoRaceTimings::update()
{
    qint64 elapsed = this->timer.elapsed() / 1000;

    ui->timer->display(QString("%1:%2:%3").arg(elapsed / 3600).arg(elapsed / 60, 2, 10, QChar('0')).arg(elapsed % 60, 2, 10, QChar('0')));
}

void ChronoRaceTimings::saveToDisk()
{
    if (++saveToDiskFlag == TIMES_PER_SECOND) {
        QString outPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        outPath.append("/lbchronorace.csv");
        QFile outFile(outPath);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            throw(ChronoRaceException(tr("Error: cannot open %1").arg(outFile.fileName())));
        }
        QTextStream outStream(&outFile);

        if (CRLoader::getEncoding() == CRLoader::Encoding::UTF8)
            outStream.setEncoding(QStringConverter::Utf8);
        else //NOSONAR if (CRLoader::getEncoding() == CRLoader::Encoding::LATIN1)
            outStream.setEncoding(QStringConverter::Latin1);

        QTableWidgetItem const *item0;
        QTableWidgetItem const *item1;
        for (int r = 0; r < ui->dataArea->rowCount(); r++) {
            item0 = ui->dataArea->item(r, 0);
            item1 = ui->dataArea->item(r, 1);
            outStream << (item0 ? item0->text() : "XXXX") << ",0," << (item1 ? item1->text() : "-:--:--") << Qt::endl;
            outStream.flush();
        }

        outFile.close();
        saveToDiskFlag = 0u;
    }
}

void ChronoRaceTimings::lock(bool checked)
{
    Qt::WindowFlags flags = this->windowFlags();

    if (checked) {
        ui->startButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->resetButton->setEnabled(false);
        ui->buttonBox->setEnabled(false);
        flags &= ~Qt::WindowCloseButtonHint;
    } else {
        ui->startButton->setEnabled(!this->clock.isActive());
        ui->stopButton->setEnabled(this->clock.isActive());
        ui->resetButton->setEnabled(!this->clock.isActive());
        ui->buttonBox->setEnabled(true);
        flags |= Qt::WindowCloseButtonHint;
    }

    this->setWindowFlags(flags);
    this->show();
}
