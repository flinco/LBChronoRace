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

#include "crlogger.hpp"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QStandardPaths>
#include <QStringConverter>

QPointer<LBChronoRace> CRLogger::app;
QTemporaryFile         CRLogger::logFile;
QMutex                 CRLogger::logMutex;
QTextStream            CRLogger::logStream;

void CRLogger::messageHandler(QtMsgType type, QMessageLogContext const &context, QString const &msg)
{
    QMutexLocker locker(&logMutex);

    if (!logFile.isOpen())
        return;

    logStream << qFormatLogMessage(type, context, msg) << Qt::endl;
    logStream.flush();

    if (type == QtFatalMsg)
        abort();
}

void CRLogger::init()
{
    if (qEnvironmentVariableIntValue("QTC_RUN") == 1) {
        qDebug() << "Running from QtCreator";
    } else if (!logFile.isOpen() && logFile.open()) {
        qInstallMessageHandler(messageHandler);
        qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] "
                           "[%{type}] "
                           //"%{file}:%{line} "
                           //"%{function} - "
                           "%{message}");
        logStream.setDevice(&logFile);
        logStream.setEncoding(QStringConverter::Utf8);
    }
}

void CRLogger::finish()
{
    if (logFile.isOpen())
        logFile.close();
}

void CRLogger::setApp(LBChronoRace *appPtr)
{
    CRLogger::app = appPtr;
}

void CRLogger::save()
{
    QString logFileName = QDir::toNativeSeparators(
            QFileDialog::getSaveFileName(Q_NULLPTR, tr("Select Log File"),
                                         QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                         tr("ChronoRace Log (*.log)")));

    if (logFileName.isEmpty() || !logFile.isOpen())
        return;

    if (!logFileName.endsWith(".log", Qt::CaseInsensitive))
        logFileName.append(".log");

    QFile dst(logFileName);

    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    QMutexLocker locker(&logMutex);

    logFile.flush();

    qint64 oldPos = logFile.pos();

    logFile.seek(0);
    dst.write(logFile.readAll());

    logFile.seek(oldPos);

    if (app)
        app->appendInfoMessage(tr("Saved Logs: %1").arg(logFileName));
}
