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

Component.prototype.beginInstallation = function() {
    // call default implementation
    component.beginInstallation();

    console.log("Try removing previous settings for current user...");
    var results = installer.execute("reg.exe", [
        "DELETE",
        "HKCU\\Software\\LoBuSoft\\LBChronoRace",
        "/f"
    ]);
    console.log("REG.EXE returned " + results[1] + " (" + results[0].replace(/[\r\n]+$/, "") + ")");

    // Get @HomeDir@
    var homeDir = installer.value("HomeDir");
    if (!homeDir || (homeDir.length === 0)) {
        homeDir = "@HomeDir@"; // fallback
    }

    // Remove any trailing backslash
    if (homeDir.match(/[\\\/]$/)) {
        homeDir = homeDir.substring(0, homeDir.length - 1);
    }

    // Split
    var parts = homeDir.split(/[/\\]/);

    // Extract the current user's name
    var currentUser = parts.pop();

    // Rebuild the parent path
    var usersRoot = parts.join("/") + "/";
    if (!usersRoot || usersRoot.length === 0) {
        usersRoot = "C:/Users/"; // fallback
    }

    console.log("HomeDir: " + homeDir);
    console.log("Current user: " + currentUser);
    console.log("Parent dir: " + usersRoot);

    console.log("Enumerating profiles under: " + usersRoot);
    results = installer.execute("cmd.exe", ["/C", "dir", "/B", "/A:D", installer.toNativeSeparators(usersRoot)]);
    if (results[1] !== 0) {
        console.log("Failed to list " + usersRoot + ", error " + results[1]);
    } else {
        var users = results[0].split(/\r?\n/);
        for (var i = 0; i < users.length; ++i) {
            var user = users[i];
            if (!user || (user.length === 0) ||
                (user.toLowerCase() === "default") ||
                (user.toLowerCase() === "default user") ||
                (user === currentUser)) {
                console.log("Skipping " + user);
                continue;
            }

            var ntuser = installer.toNativeSeparators(usersRoot + user + "/NTUSER.DAT");

            // Check if NTUSER.DAT exists
            var check = installer.execute("cmd.exe", ["/C", "IF", "EXIST", ntuser, "echo", "YES"]);
            //console.log("cmd.exe returned " + check[1] + " (" + check[0].replace(/[\r\n]+$/, "") + ")");
            if ((check[1] === 0) && (check[0].indexOf("YES") !== -1)) {
                var hive = "TempHive" + i;
                console.log("Found NTUSER.DAT for user: " + user);

                var load = installer.execute("reg.exe", ["LOAD", "HKU\\" + hive, ntuser]);
                console.log("LOAD returned " + load[1] + " (" + load[0].replace(/[\r\n]+$/, "") + ")");
                if (load[1] === 0) {
                    console.log("Executing reg.exe, [\"DELETE\", \"HKU\\\\" + hive + "\\Software\\LoBuSoft\\LBChronoRace\", \"/f\"]");
                    var del = installer.execute("reg.exe", [
                        "DELETE",
                        "HKU\\" + hive + "\\Software\\LoBuSoft\\LBChronoRace",
                        "/f"
                    ]);
                    console.log("DELETE returned " + del[1] + " (" + del[0].replace(/[\r\n]+$/, "") + ")");
                    installer.execute("reg.exe", ["UNLOAD", "HKU\\" + hive]);
                }
            } else {
                console.log("Skipping " + ntuser);
            }
        }
    }

}
