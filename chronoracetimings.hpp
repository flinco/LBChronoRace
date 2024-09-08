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

#ifndef CHRONORACETIMINGS_H
#define CHRONORACETIMINGS_H

#include <QDialog>
#include <QTimerEvent>
#include <QElapsedTimer>
#include <QThread>

#include "ui_chronoracetimings.h"

class TimingsWorker : public QObject
{
    Q_OBJECT
public:
    explicit TimingsWorker();

public slots:
    void writeToDisk(QString const &buffer);

signals:
    void writeDone();

private:
    QString timingsFilePath { "" };
};

class ChronoRaceTimings : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceTimings(QWidget *parent = Q_NULLPTR);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

public slots:
    void accept() override;
    void reject() override;
    void show(); //NOSONAR

    void clearDiskBuffer();

signals:
    void newTimingsCount(int count);
    void saveToDisk(QString const &buffer);
    void error(QString const &message);

private:
    QScopedPointer<Ui::ChronoRaceTimings> ui { new Ui::ChronoRaceTimings };

    int            timingRowCount { 0 };

    int            updateTimerId  { 0 };
    int            backupTimerId  { 0 };
    QElapsedTimer  timer;

    QList<QString> saveToDiskQueue;
    QThread        saveToDiskThread;
    TimingsWorker  saveToDiskWorker;

    QTableWidgetItem *currentBibItem { Q_NULLPTR };

    void updateCurrentBibItem(QTableWidgetItem *newBibItem);

    void recordTiming(qint64 seconds);
    void deleteTiming();
    void deleteBib();

    void saveTimings();

    void stepUp();
    void stepDown();

    bool enterPressed();
    bool digitPressed(QString const &key);
    bool upPressed();
    bool downPressed();

private slots:
    void start();
    void stop();
    void reset();
    void lock(bool checked);

    void bibClicked(QTableWidgetItem *item);
};

#endif // CHRONORACETIMINGS_H
