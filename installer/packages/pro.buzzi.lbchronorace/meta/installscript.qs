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
}

Component.prototype.createOperations = function() {
    component.createOperations();

    var userTemp = QDesktopServices.storageLocation(QDesktopServices.TempLocation);
    //console.log("Temporary location " + userTemp);

    if (systemInfo.productType === "windows") {
        var check;
        var targetDir = installer.value("TargetDir");
        var startMenuDir = installer.value("StartMenuDir");
        var desktopDir = installer.value("DesktopDir");
        var setAppUserModelId = installer.toNativeSeparators(userTemp + "/SetAppUserModelId.exe");
        var maintTool = installer.toNativeSeparators(targetDir + "/" + installer.value("MaintenanceToolName") + ".exe");
        var lbChronoRace = installer.toNativeSeparators(targetDir + "/LBChronoRace.exe");
        var targetPath = installer.toNativeSeparators(targetDir);
        var linkPath;
        var linkTempPath;

        // Create the Start Menu directory
        component.addOperation("Mkdir", startMenuDir);

        // Create and install Start Menu shortcut
        linkPath = installer.toNativeSeparators(startMenuDir + "/LBChronoRace.lnk");
        linkTempPath = installer.toNativeSeparators(userTemp + "/LBChronoRace.lnk");
        component.addOperation("CreateShortcut",
                               lbChronoRace,
                               linkTempPath,
                               "workingDirectory=" + targetPath,
                               "iconPath=" + lbChronoRace,
                               "iconId=0",
                               "description=\"Start LBChronoRace\"");
        component.addOperation("SimpleMoveFile", linkTempPath, linkPath);
        component.addOperation("Execute", setAppUserModelId, linkPath, "LBChronoRace.Application");

        // Create temporary shortcut target file (if LBChronoRaceInstaller.exe does not exist)
        check = installer.execute("cmd.exe", ["/C", "IF", "EXIST", maintTool, "echo", "YES"]);
        if ((check[1] !== 0) || (check[0].indexOf("YES") === -1))
            component.addOperation("Copy", lbChronoRace, maintTool, "UNDOOPERATION", "");
        // Create and install Updater shortcut
        linkPath = installer.toNativeSeparators(startMenuDir + "/LBMaintenance.lnk");
        linkTempPath = installer.toNativeSeparators(userTemp + "/LBMaintenance.lnk");
        component.addOperation("CreateShortcut",
                               maintTool,
                               linkTempPath,
                               "--start-updater",
                               "workingDirectory=" + targetPath,
                               "iconPath=" + maintTool,
                               "iconId=0",
                               "description=\"Update LBChronoRace\"");
        component.addOperation("SimpleMoveFile", linkTempPath, linkPath);
        component.addOperation("Execute", setAppUserModelId, linkPath, "LBChronoRace.Updater");
        // Create and install Uninstaller shortcut
        linkPath = installer.toNativeSeparators(startMenuDir + "/LBUninstall.lnk");
        linkTempPath = installer.toNativeSeparators(userTemp + "/LBUninstall.lnk");
        component.addOperation("CreateShortcut",
                               maintTool,
                               linkTempPath,
                               "--start-uninstaller",
                               "workingDirectory=" + targetPath,
                               "iconPath=" + maintTool,
                               "iconId=0",
                               "description=\"Uninstall LBChronoRace\"");
        component.addOperation("SimpleMoveFile", linkTempPath, linkPath);
        component.addOperation("Execute", setAppUserModelId, linkPath, "LBChronoRace.Uninstaller");
        // Remove temporary shortcut target file
        if ((check[1] !== 0) || (check[0].indexOf("YES") === -1))
            component.addOperation("Delete", maintTool, "UNDOOPERATION", "");

        // Create and install Desktop shortcut
        linkPath = installer.toNativeSeparators(desktopDir + "/LBChronoRace.lnk");
        linkTempPath = installer.toNativeSeparators(userTemp + "/LBChronoRace.lnk");
        component.addOperation("CreateShortcut",
                               lbChronoRace,
                               linkTempPath,
                               "workingDirectory=" + targetPath,
                               "iconPath=" + lbChronoRace,
                               "iconId=0",
                               "description=\"Start LBChronoRace\"");
        component.addOperation("SimpleMoveFile", linkTempPath, linkPath);

        // Register the .crd file type
        component.addOperation("RegisterFileType",
                               "crd",
                               "\"" + lbChronoRace + "\" \"%1\"",
                               "LBChronoRace Data",
                               "application/x-binary",
                               installer.toNativeSeparators(targetDir + "/LBChronoRaceCRD.ico"),
                               "ProgId=LBChronoRace.exe");
    }
}
