function Controller() {
}

Controller.prototype.TargetDirectoryPageCallback = function() {
    var targetDir = installer.value("TargetDir");
    var uninstaller = installer.toNativeSeparators(targetDir + "/LBChronoRaceInstaller.exe");
    if (!installer.fileExists(uninstaller))
      uninstaller = installer.toNativeSeparators(targetDir + "/maintenancetool.exe");
    if (installer.fileExists(uninstaller)) {
        console.log("Try uninstalling previous version...");
        var isAdmin = installer.hasAdminRights();
        if (isAdmin || installer.gainAdminRights()) {
            var results = installer.execute(uninstaller, [ "--confirm-command", "pr" ]);
            console.log("Uninstall returned " + results[0] + "(" + results[1] + ")");
            if (!isAdmin)
                installer.dropAdminRights();
        }
    }
}