function restoreTargetDir() {
    var targetDir = installer.value("TargetDir");

    // restore any existing Target Directory
    var moveSrc = installer.toNativeSeparators(targetDir + "_backup");
    if (installer.fileExists(moveSrc)) {
        var moveTarget = installer.toNativeSeparators(targetDir);
        console.log("Try restoring " + moveSrc + " to " + moveTarget);
        var result = installer.execute("cmd", [ "/C", "move", "/Y", moveSrc, moveTarget ]);
        console.log("Move returned code: " + result[0] + " output: " + result[1]);
    }
}

function Controller() {
    if (!installer.hasAdminRights()) {
        installer.gainAdminRights();
    }

    installer.statusChanged.connect(function(status) {
        if (status === QInstaller.Canceled) {
            restoreTargetDir();
        }
    });
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var targetDir = installer.value("TargetDir");
    
    // rename any existing Target Directory
    var moveSrc = installer.toNativeSeparators(targetDir);
    if (installer.fileExists(moveSrc)) {
        var moveTarget = installer.toNativeSeparators(targetDir + "_backup");
        console.log("Try moving " + moveSrc + " to " + moveTarget);
        var result = installer.execute("cmd", [ "/C", "move", "/Y", moveSrc, moveTarget ]);
        console.log("Move returned code: " + result[0] + " output: " + result[1]);
    }
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    // restore any existing Target Directory
    restoreTargetDir();
}

Controller.prototype.PerformInstallationPageCallback = function() {
    var targetDir = installer.value("TargetDir");

    var uninstaller = installer.toNativeSeparators(targetDir + "/LBChronoRaceInstaller.exe");
    if (!installer.fileExists(uninstaller)) {
        uninstaller = installer.toNativeSeparators(targetDir + "/maintenancetool.exe");
    }
    if (installer.fileExists(uninstaller)) {
        console.log("Try uninstalling previous version...");
        var results = installer.execute(uninstaller, [ "--confirm-command", "pr" ]);
        console.log("Uninstall returned " + results[0] + "(" + results[1] + ")");
    }
}