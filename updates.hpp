/*****************************************************************************
 * Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                   *
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

#ifndef UPDATES_HPP
#define UPDATES_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QProcess>
#include <QScopedPointer>

namespace updates {
class Updates;
}

class Updates : public QObject
{
    Q_OBJECT

public:
    explicit Updates(QObject *parent = nullptr);

public slots:
    void startCheck();

signals:
    void info(QString const &);
    void error(QString const &);

private:
    QScopedPointer<QProcess> process { new QProcess };
    QString maintenanceToolPath;

    void parseXml(QByteArray const &data, QStringList &updatesList);
    void updatesReady(QStringList const &updates);
    void notifyUserInfo(QString const &message);
    void notifyUserWarning(QString const &message);
    void notifyUserError(QString const &message);

private slots:
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // UPDATES_HPP
