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

function Controller() {
    console.log("AllUsers=" + installer.value("AllUsers"));

    installer.installationStarted.connect(this, extractEmbeddedResources);
    installer.installationFinished.connect(this, cleanEmbeddedResources);

    if (!installer.hasAdminRights()) {
        installer.gainAdminRights();
    }
}

extractEmbeddedResources = function() {
    var userTemp = QDesktopServices.storageLocation(QDesktopServices.TempLocation);
    installer.performOperation("Copy", [":/SetAppUserModelId.exe", installer.toNativeSeparators(userTemp + "/SetAppUserModelId.exe"), "UNDOOPERATION", ""]);
}

cleanEmbeddedResources = function() {
    var userTemp = QDesktopServices.storageLocation(QDesktopServices.TempLocation);
    installer.performOperation("Delete", [installer.toNativeSeparators(userTemp + "/SetAppUserModelId.exe"), "UNDOOPERATION", ""]);
}
