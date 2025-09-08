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

#ifndef CHRONORACETIMINGS_HPP
#define CHRONORACETIMINGS_HPP

#include <QDialog>
#include <QTimerEvent>
#include <QElapsedTimer>
#include <QThread>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QKeyCombination>

#include "liveview.hpp"

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

    void setLiveTables(LiveView *liveView);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

public slots:
    void accept() override;
    void reject() override;
    void show(); //NOSONAR

    void clearDiskBuffer();

    void addTimeSpan();
    void subtractTimeSpan();
    void applyTimeSpan(int offset);

signals:
    void saveToDisk(QString const &);
    void info(QString const &);
    void error(QString const &);

private:
    QScopedPointer<Ui::ChronoRaceTimings> ui { new Ui::ChronoRaceTimings };

    LiveView *liveTables { Q_NULLPTR };

    int            timingRowCount { 0 };

    int            updateTimerId  { 0 };
    int            backupTimerId  { 0 };
    QElapsedTimer  timer;
    qint64         timerOffset    { Q_INT64_C(0) };
    bool           timerPaused    { false };

    QList<QString> saveToDiskQueue;
    QThread        saveToDiskThread;
    TimingsWorker  saveToDiskWorker;

    QTableWidgetItem *currentBibItem { Q_NULLPTR };

    QKeyCombination triggerKey { Qt::KeyboardModifier::NoModifier, Qt::Key::Key_unknown };

    void updateCurrentBibItem(QTableWidgetItem *newBibItem);

    void recordTiming(qint64 milliseconds);
    void deleteTiming();
    void deleteBib();

    void saveTimings();

    void stepUp();
    void stepDown();

    bool enterPressed();
    bool digitPressed(QString const &key);

    void clear();

private slots:
    void start();
    void stop();
    void reset();
    void lock(int value);

    void bibClicked(QTableWidgetItem *item);
};

#endif // CHRONORACETIMINGS_HPP
