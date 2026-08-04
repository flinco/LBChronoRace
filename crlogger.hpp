/*****************************************************************************
 * Copyright (C) 2026 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#ifndef CRLOGGER_HPP
#define CRLOGGER_HPP

#include <QCoreApplication>
#include <QString>
#include <QTemporaryFile>
#include <QMutex>
#include <QTextStream>
#include <QMessageLogContext>
#include <QPointer>

#include "lbchronorace.hpp"

class CRLogger
{
    Q_DECLARE_TR_FUNCTIONS(CRLogger)

private:
    static QPointer<LBChronoRace> app;
    static QTemporaryFile logFile;
    static QMutex         logMutex;
    static QTextStream    logStream;

    static void messageHandler(QtMsgType type, QMessageLogContext const &context, QString const &msg);

public:
    static void init();
    static void finish();
    static void setApp(LBChronoRace *appPtr);

public slots:
    static void save();
};

#endif // CRLOGGER_HPP
