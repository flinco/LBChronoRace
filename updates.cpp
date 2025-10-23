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

#include <QCoreApplication>
#include <QtSystemDetection>
#include <QOverload>
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <QMessageBox>

#include "updates.hpp"

Updates::Updates(QObject *parent) : QObject(parent)
{
    // Connect the QProcess finished signal to our handler slot
    this->process.data()->setParent(this);
    connect(this->process.data(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Updates::handleProcessFinished);

    // Resolve the path once during construction (assuming
    // the Maintenance Tool is in the application's root directory)
    this->maintenanceToolPath = QStringLiteral(".") + QDir::separator() + QStringLiteral(LBCHRONORACE_MAINTENANCE_TOOL) + QStringLiteral(".exe");
    if (QFile file(this->maintenanceToolPath); !file.exists()) {
        this->maintenanceToolPath.clear();
    }
}

void Updates::startCheck()
{
#ifdef Q_OS_WIN
    if (this->maintenanceToolPath.isEmpty()) {
        // Path resolution failed previously
        this->notifyUserError(tr("Maintenance Tool not found"));
    } else if (this->process->state() == QProcess::Running) {
        emit info(tr("Updates: update check already running"));
    } else {
        QStringList arguments = { "check-updates" };

        emit info(tr("Updates: checking for updates…"));
        this->process->start(this->maintenanceToolPath, arguments);

        // IMPORTANT: No QProcess::waitForFinished() is called here.
        // The application stays responsive.
    }
#else
    this->notifyUserInfo(tr("update checking is not yet available for this operating system"));
#endif
}

void Updates::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QStringList updates = { };

    if (exitStatus != QProcess::ExitStatus::NormalExit) {
        emit error(tr("Updates: Maintenance Tool crashed"));
    } else if (QByteArray errorOutput = this->process->readAllStandardError(); exitCode != 0) {
        emit error(tr("Updates: error executing Maintenance Tool<br>Exit Code: %1<br>Error Output: %2").arg(exitCode).arg(QString(errorOutput).trimmed()));
    } else if (QByteArray output = this->process->readAllStandardOutput(); output.isEmpty()) {
        // Empty stdout and zero exit code means no updates found.
        emit info(tr("Updates: application is up to date"));
    } else {
        // Non-empty stdout indicates that updatable components were found

        // Isolate the XML block
        auto startIndex = output.indexOf(QByteArrayView("<?xml version=\"1.0\"?>"));
        auto endIndex = output.indexOf(QByteArrayView("</updates>"));

        if (startIndex == -1) {
            emit error(tr("Updates:: XML parsing error (start tag not found)"));
        } else if (endIndex == -1) {
            emit error(tr("Updates:: XML parsing error (end tag not found)"));
        } else {
            // Compute the length: (end of tag </updates>) - (start of tag <?xml) + (length of tag </updates>)
            auto length = (endIndex - startIndex) + QStringLiteral("</updates>").length();

            parseXml(output.mid(startIndex, length), updates);
        }
    }

    // Notify the user
    updatesReady(updates);
}

void Updates::parseXml(QByteArray const &data, QStringList &updatesList)
{
    using enum QXmlStreamReader::TokenType;

    QXmlStreamReader updatesXml;
    QXmlStreamAttributes attributes;
    QString name;
    QString version;

    updatesXml.addData(data);
    while (!updatesXml.atEnd()) {
        updatesXml.readNext();

        if (updatesXml.isStartElement() && updatesXml.name() == QLatin1String("update")) {
            attributes = updatesXml.attributes();

            // Extract required attributes: 'name' and 'version'
            if (attributes.hasAttribute(QLatin1String("name"))) {
                name = attributes.value(QLatin1String("name")).toString();
            } else {
                name.clear(); // Ensure it's clear if missing
            }

            if (attributes.hasAttribute(QLatin1String("version"))) {
                version = attributes.value(QLatin1String("version")).toString();
            } else {
                version.clear(); // Ensure it's clear if missing
            }

            // Notify the UI
            emit info(tr("Updates: available %1 %2").arg(name, version));

            // Fill the list
            updatesList << name + " " + version;
        }
    }

    if (updatesXml.hasError()) {
        emit error(tr("Updates:: data errors (%1)").arg(updatesXml.errorString()));
    }
}

void Updates::updatesReady(QStringList const &updates)
{
    using enum QMessageBox::StandardButton;

    if (updates.empty()) {
        this->notifyUserInfo(tr("no updates available"));
        return;
    } else if (QMessageBox::question(qobject_cast<QWidget *>(this->parent()), tr("%1 Updates").arg(QStringLiteral(LBCHRONORACE_NAME)),
                              tr("<p>New updates are available:</p><ul><li>%1</li></ul><p>Do you want to close %2 and launch the Maintenance Tool to install them?</p>").arg(updates.join("</li><li>"), QStringLiteral(LBCHRONORACE_NAME)),
                              QMessageBox::Ok | QMessageBox::Cancel, Cancel) != Ok) {
        return;
    } else if (this->maintenanceToolPath.isEmpty()) {
        // Path resolution failed previously
        this->notifyUserError(tr("Maintenance Tool not found"));
    } else if (QStringList arguments = { "--start-updater" }; QProcess::startDetached(this->maintenanceToolPath, arguments)) {
        // Close the application
        QCoreApplication::quit();
    } else {
        this->notifyUserError(tr("failed to start the update process.<br>Please try running the Maintenance Tool manually"));
    }
}

void Updates::notifyUserInfo(QString const &message)
{
    emit info(tr("Updates: %1").arg(message));
    QMessageBox::information(qobject_cast<QWidget *>(this->parent()), tr("%1 Updates").arg(QStringLiteral(LBCHRONORACE_NAME)), QStringLiteral("<p>") % message.left(1).toUpper() % message.mid(1) % QStringLiteral(".</p>"));
}

void Updates::notifyUserWarning(QString const &message)
{
    emit error(tr("Updates:: %1").arg(message));
    QMessageBox::warning(qobject_cast<QWidget *>(this->parent()), tr("%1 Updates").arg(QStringLiteral(LBCHRONORACE_NAME)), QStringLiteral("<p>") % message.left(1).toUpper() % message.mid(1) % QStringLiteral(".</p>"));
}

void Updates::notifyUserError(QString const &message)
{
    emit error(tr("Updates: %1").arg(message));
    QMessageBox::critical(qobject_cast<QWidget *>(this->parent()), tr("%1 Updates").arg(QStringLiteral(LBCHRONORACE_NAME)), QStringLiteral("<p>") % message.left(1).toUpper() % message.mid(1) % QStringLiteral(".</p>"));
}
