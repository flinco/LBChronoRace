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
#include <shobjidl.h>
#include <propvarutil.h>
#include <propsys.h>
#include <propkey.h>
#include <shlguid.h>
#include <iostream>
#include <string>
#include <filesystem>

void printError(HRESULT hr, std::string const &msg) {
    std::cerr << msg << " (HRESULT: 0x" << std::hex << hr << ")" << std::endl;
}

int wmain(int argc, wchar_t *argv[]) {

    HRESULT hr;
    bool setMode = false;

    switch (argc) {
    case 2:
        // nothing to do
        break;
    case 3:
        setMode = true;
        break;
    default:
        std::wcout << L"Usage: SetAppUserModelId.exe <shortcut.lnk> [AppUserModelID]" << std::endl;
        return 1;
    }

    std::wstring const linkFile = argv[1];

    if (!std::filesystem::exists(linkFile) || linkFile.substr(linkFile.size() - 4) != L".lnk") {
        std::wcerr << L"File does not exist or is not a .lnk: " << linkFile << std::endl;
        return 1;
    }

    if (FAILED(hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        printError(hr, "Failed to initialize COM");
        return 1;
    }

    IShellLink *pShellLink = nullptr;
    if (FAILED(hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pShellLink)))) {
        printError(hr, "Failed to create IShellLink instance");
        CoUninitialize();
        return 1;
    }

    IPersistFile *pPersistFile = nullptr;
    if (FAILED(hr = pShellLink->QueryInterface(IID_PPV_ARGS(&pPersistFile)))) {
        printError(hr, "Failed to get IPersistFile interface");
        pShellLink->Release();
        CoUninitialize();
        return 1;
    }

    if (FAILED(hr = pPersistFile->Load(linkFile.c_str(), setMode ? STGM_READWRITE : STGM_READ))) {
        printError(hr, "Failed to load .lnk file");
        pPersistFile->Release();
        pShellLink->Release();
        CoUninitialize();
        return 1;
    }

    // Obtain IPropertyStore
    IPropertyStore* pPropStore = nullptr;
    if (FAILED(hr = pShellLink->QueryInterface(IID_PPV_ARGS(&pPropStore)))) {
        printError(hr, "Failed to get IPropertyStore");
        pPersistFile->Release();
        pShellLink->Release();
        CoUninitialize();
        return 1;
    }

    PROPVARIANT pv;
    if (setMode) {
        // Write the new value
        if (FAILED(hr = InitPropVariantFromString(argv[2], &pv))) {
            printError(hr, "Failed to initialize PROPVARIANT");
            pPropStore->Release();
            pPersistFile->Release();
            pShellLink->Release();
            CoUninitialize();
            return 1;
        }

        if (FAILED(hr = pPropStore->SetValue(PKEY_AppUserModel_ID, pv))) {
            printError(hr, "Failed to set AppUserModelID");
        } else if (FAILED(hr = pPropStore->Commit())) {
            printError(hr, "Failed to commit property changes");
        } else if (FAILED(hr = pPersistFile->Save(linkFile.c_str(), TRUE))) {
            printError(hr, "Failed to save .lnk file");
        } else {
            std::wcout << L"AppUserModelID set successfully!" << std::endl;
        }
    } else {
        // Read the current value
        PropVariantInit(&pv);
        if (SUCCEEDED(hr = pPropStore->GetValue(PKEY_AppUserModel_ID, &pv)) && (pv.vt == VT_LPWSTR)) {
            std::wcout << L"Current AppUserModelID: " << pv.pwszVal << std::endl;
        } else {
            std::wcout << L"Current AppUserModelID is not set" << std::endl;
        }
    }
    // Release resources
    PropVariantClear(&pv);

    pPropStore->Release();
    pPersistFile->Release();
    pShellLink->Release();
    CoUninitialize();

    return 0;
}
