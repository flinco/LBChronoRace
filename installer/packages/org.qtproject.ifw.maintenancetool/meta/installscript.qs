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

function Component() {
    component.ifwVersion = installer.value("FrameworkVersion");
    installer.installationStarted.connect(this, Component.prototype.onInstallationStarted);
    installer.installationFinished.connect(this, Component.prototype.onInstallationFinished);
}

Component.prototype.onInstallationStarted = function() {
    if (component.updateRequested() || component.installationRequested()) {
        var updateResourceFilePath = installer.value("TargetDir");
        if (installer.value("os") == "win") {
            component.installerbaseBinaryPath = "@TargetDir@/installerbase.exe";
        } else if (installer.value("os") == "x11") {
            component.installerbaseBinaryPath = "@TargetDir@/installerbase";
        } else if (installer.value("os") == "mac") {
            // In macOs maintenance tool can be either installerbase from Qt Installer
            // Framework's install folder, or app bundle created by binarycreator
            // with --create-maintenancetool switch. "MaintenanceTool.app" -name
            // may differ depending on what has been defined in config.xml while
            // creating the maintenance tool.
            // Use either of the following (not both):

            // component.installerbaseBinaryPath = "@TargetDir@/installerbase";
            updateResourceFilePath += "/tmpMaintenanceToolApp";
            component.installerbaseBinaryPath = "@TargetDir@/tmpMaintenanceToolApp/MaintenanceTool.app";
        }
        installer.setInstallerBaseBinary(component.installerbaseBinaryPath);

        updateResourceFilePath += "/update.rcc";
        installer.setValue("DefaultResourceReplacement", updateResourceFilePath);
    }

    var userTemp = QDesktopServices.storageLocation(QDesktopServices.TempLocation);
    installer.performOperation("Copy", [":/SetAppUserModelId.exe", installer.toNativeSeparators(userTemp + "/SetAppUserModelId.exe"), "UNDOOPERATION", ""]);
}

Component.prototype.onInstallationFinished = function() {
    var userTemp = QDesktopServices.storageLocation(QDesktopServices.TempLocation);
    installer.performOperation("Delete", [installer.toNativeSeparators(userTemp + "/SetAppUserModelId.exe"), "UNDOOPERATION", ""]);
}

Component.prototype.createOperationsForArchive = function(archive) {
    // IFW versions 4.8.1 onwards supports extracting the maintenance tool to a folder.
    // It is a good practice to extract the maintenance tool to a folder in macOs, so
    // it won't interfere the current running maintenance tool. As the last step of the
    // installation, IFW will move the maintenance tool to the root of the installation.
    // Windows is using deferred update for the maintenance tool, and Linux inode.
    if (installer.value("os") != "mac") {
        component.createOperationsForArchive(archive)
    } else {
        component.addOperation("Extract", archive, "@TargetDir@/tmpMaintenanceToolApp");
    }
}
