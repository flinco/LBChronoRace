function Component() {
    console.log("Try removing previous settings...");
    var results = installer.execute("reg.exe", ["DELETE", "HKCU\\Software\\LoBuSoft\\LBChronoRace", "/f"]);
    console.log("REG.EXE returned " + results[0] + "(" + results[1] + ")");
}