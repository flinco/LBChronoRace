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

#include <windows.h>
#include <shlobj.h>  // For SHGetFolderPath
#include <string>
#include <fstream>
#include <vector>

#include "launcher.hpp"

static int rv;

bool ExtractInstaller(const std::wstring& outPath) {
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_INSTALLER_BIN), MAKEINTRESOURCEW(RT_RCDATA));
    if (!hRes) return false;

    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData) return false;

    DWORD size = SizeofResource(NULL, hRes);
    const void *data = LockResource(hData);
    if (!data || size == 0) return false;

    // Convert wstring path to narrow string for ofstream
    std::ofstream out(outPath, std::ios::binary);
    if (!out) return false;

    out.write(static_cast<const char *>(data), size);
    return out.good();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);

    std::wstring installerPath = std::wstring(tempPath) + L"LBChronoRaceInstaller.exe";

    if (!ExtractInstaller(installerPath)) {
        MessageBoxW(NULL, L"Cannot extract the installer.", L"Error", MB_ICONERROR);
        return 1;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    std::wstring cmdStr = L"\"" + installerPath + L"\" AllUsers=true";
    std::vector<wchar_t> cmdVec(cmdStr.begin(), cmdStr.end());
    cmdVec.push_back(L'\0');
    LPWSTR cmd = cmdVec.data();

    BOOL success = CreateProcessW(
        NULL,
        cmd,
        NULL,
        NULL,
        FALSE,
        CREATE_UNICODE_ENVIRONMENT,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        DWORD errorCode = GetLastError();
        wchar_t msg[256];
        swprintf(msg, 256, L"Cannot start the installer. Error code: %lu", errorCode);
        MessageBoxW(NULL, msg, L"Error", MB_ICONERROR);
        return 1;
    }

    return 0;
}
