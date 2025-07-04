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
