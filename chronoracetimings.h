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
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QTimer>

#include "ui_chronoracetimings.h"

class ChronoRaceTimings : public QDialog
{
    Q_OBJECT

public:
    explicit ChronoRaceTimings(QWidget *parent = Q_NULLPTR);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void accept() override;
    void reject() override;
    void show(); //NOSONAR

signals:
    void newTimingsCount(int count);

private:
    QScopedPointer<Ui::ChronoRaceTimings> ui { new Ui::ChronoRaceTimings };

    int           timingRowCount { 0 };
    int           bibRowCount    { 0 };
    QString       bibBuffer      { "" };

    QTimer        clock;
    QElapsedTimer timer;

    uint          saveToDiskFlag { 0u };

    void recordTiming(qint64 seconds);
    void deleteTiming();
    void recordBib(QString const &key);
    void deleteBib();

    void saveTimings();

private slots:
    void start();
    void stop();
    void reset();
    void update();
    void saveToDisk();
    void lock(bool checked);
};

#endif // CHRONORACETIMINGS_H
