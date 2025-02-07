#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <fstream>
#include <algorithm>

HANDLE hProcess = NULL;

DWORD dwClientState = 0;
DWORD dwLocalPlayer = 0;
DWORD dwEntityList = 0;
DWORD m_hViewModel = 0;
DWORD m_iViewModelIndex = 0;
DWORD m_flFallbackWear = 0;
DWORD m_nFallbackPaintKit = 0;
DWORD m_iItemIDHigh = 0;
DWORD m_iEntityQuality = 0;
DWORD m_iItemDefinitionIndex = 0;
DWORD m_hActiveWeapon = 0;
DWORD m_hMyWeapons = 0;
DWORD m_nModelIndex = 0;
DWORD m_dwModelPrecache = 0;

std::string ReadMemoryAsString(DWORD address, size_t size) {
    char buffer[128] = {0};
    if (ReadMemory(hProcess, address, buffer, size)) {
        return std::string(buffer);
    }
    return "";
}

DWORD GetClientState() {
    return (DWORD)ReadMemory(hProcess, dwClientState, NULL, sizeof(DWORD));
}

UINT GetModelIndexByName(const std::string& modelName) {
    DWORD cstate = GetClientState();
    DWORD nst = (DWORD)ReadMemory(hProcess, cstate + m_dwModelPrecache, NULL, sizeof(DWORD));
    DWORD nsd = (DWORD)ReadMemory(hProcess, nst + 0x40, NULL, sizeof(DWORD));
    DWORD nsdi = (DWORD)ReadMemory(hProcess, nsd + 0xC, NULL, sizeof(DWORD));

    for (UINT i = 0; i < 1024; ++i) {
        DWORD nsdi_i = (DWORD)ReadMemory(hProcess, nsdi + 0xC + i * 0x34, NULL, sizeof(DWORD));
        std::string model = ReadMemoryAsString(nsdi_i, sizeof(model));
        if (_stricmp(model.c_str(), modelName.c_str()) == 0) {
            return i;
        }
    }
    return 0;
}

void LoadSkins(const std::string& filePath, std::vector<std::string>& skinNames, std::vector<UINT>& skinIDs) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[!] Error loading skins from file!\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(": ");
        if (pos == std::string::npos) continue;

        std::string skinName = line.substr(pos + 2);
        UINT skinID = std::stoi(line.substr(0, pos));
        skinNames.push_back(skinName);
        skinIDs.push_back(skinID);
    }

    file.close();
}

void SortSkins(std::vector<std::string>& names, std::vector<UINT>& values) {
    for (size_t i = 0; i < names.size(); ++i) {
        for (size_t j = i + 1; j < names.size(); ++j) {
            if (names[i] > names[j]) {
                std::swap(names[i], names[j]);
                std::swap(values[i], values[j]);
            }
        }
    }
}

void PrintMenu(const std::string& title, const std::vector<std::string>& names, size_t index) {
    std::cout << title << " " << (index > 0 ? "<" : "|") << " " << names[index] << " " << (index < names.size() - 1 ? ">" : "|") << "\r";
    Sleep(names.size() < 20 ? 150 : 35);
}

size_t ItemSelect(const std::string& title, const std::vector<std::string>& names) {
    size_t index = 0;
    PrintMenu(title, names, index);

    while (!GetAsyncKeyState(VK_RETURN)) {
        if (GetAsyncKeyState(VK_RIGHT) && index < names.size() - 1) {
            PrintMenu(title, names, ++index);
        } else if (GetAsyncKeyState(VK_LEFT) && index > 0) {
            PrintMenu(title, names, --index);
        }
    }

    std::cout << title << " " << names[index] << "\n";
    Sleep(50);
    return index;
}

void xSkins(const short knifeIndex, const UINT knifeSkin) {
    const int itemIDHigh = -1;
    const int entityQuality = 3;
    const float fallbackWear = 0.0001f;

    UINT modelIndex = 0;
    DWORD localPlayer = 0;

    while (!GetAsyncKeyState(VK_F6)) {
        DWORD tempPlayer = (DWORD)ReadMemory(hProcess, dwLocalPlayer, NULL, sizeof(DWORD));
        if (!tempPlayer) {
            modelIndex = 0;
            continue;
        } else if (tempPlayer != localPlayer) {
            localPlayer = tempPlayer;
            modelIndex = 0;
        }

        while (!modelIndex) {
            modelIndex = GetModelIndexByName("models/weapons/v_knife_default_ct.mdl");
        }

        for (UINT i = 0; i < 8; ++i) {
            DWORD currentWeapon = (DWORD)ReadMemory(hProcess, localPlayer + m_hMyWeapons + i * 0x4, NULL, sizeof(DWORD)) & 0xfff;
            currentWeapon = (DWORD)ReadMemory(hProcess, dwEntityList + (currentWeapon - 1) * 0x10, NULL, sizeof(DWORD));
            if (!currentWeapon) { continue; }

            short weaponIndex = (short)ReadMemory(hProcess, currentWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
            UINT weaponSkin = knifeSkin;

            if (weaponIndex == WEAPON_KNIFE || weaponIndex == knifeIndex) {
                WriteMemory(hProcess, currentWeapon + m_iItemDefinitionIndex, &knifeIndex, sizeof(short));
                WriteMemory(hProcess, currentWeapon + m_nModelIndex, &modelIndex, sizeof(UINT));
                WriteMemory(hProcess, currentWeapon + m_iViewModelIndex, &modelIndex, sizeof(UINT));
                WriteMemory(hProcess, currentWeapon + m_iEntityQuality, &entityQuality, sizeof(int));
            }

            if (weaponSkin) {
                WriteMemory(hProcess, currentWeapon + m_iItemIDHigh, &itemIDHigh, sizeof(int));
                WriteMemory(hProcess, currentWeapon + m_nFallbackPaintKit, &weaponSkin, sizeof(UINT));
                WriteMemory(hProcess, currentWeapon + m_flFallbackWear, &fallbackWear, sizeof(float));
            }
        }

        DWORD activeWeapon = (DWORD)ReadMemory(hProcess, localPlayer + m_hActiveWeapon, NULL, sizeof(DWORD)) & 0xfff;
        activeWeapon = (DWORD)ReadMemory(hProcess, dwEntityList + (activeWeapon - 1) * 0x10, NULL, sizeof(DWORD));
        if (!activeWeapon) { continue; }

        short weaponIndex = (short)ReadMemory(hProcess, activeWeapon + m_iItemDefinitionIndex, NULL, sizeof(short));
        if (weaponIndex != knifeIndex) { continue; }

        DWORD activeViewModel = (DWORD)ReadMemory(hProcess, localPlayer + m_hViewModel, NULL, sizeof(DWORD)) & 0xfff;
        activeViewModel = (DWORD)ReadMemory(hProcess, dwEntityList + (activeViewModel - 1) * 0x10, NULL, sizeof(DWORD));
        if (!activeViewModel) { continue; }

        WriteMemory(hProcess, activeViewModel + m_nModelIndex, &modelIndex, sizeof(UINT));
    }
}

int main() {
    std::cout << "[xSkins] External Knife & Skin Changer\n";

    std::vector<std::string> skinNames;
    std::vector<UINT> skinIDs;

    LoadSkins("skins.txt", skinNames, skinIDs);
    if (skinNames.empty()) {
        return 1;
    }

    std::cout << "[+] Loaded " << skinNames.size() << " skins from file\n";
    SortSkins(skinNames, skinIDs);

    size_t knifeID = ItemSelect("[+] Knife model:", knifeNames);
    size_t skinID = ItemSelect("[+] Knife skin:", skinNames);

    skinID = skinIDs[skinID];

    DWORD dwProcessId = GetProcessIdByProcessName(_T("csgo.exe"));
    std::cout << "[+] csgo.exe process id: " << dwProcessId << "\n";

    DWORD dwClientBase = GetModuleBaseAddress(dwProcessId, _T("client.dll"));
    std::cout << "[+] client.dll base: 0x" << std::hex << dwClientBase << std::dec << "\n";

    DWORD dwClientSize = GetModuleSize(dwDWORD dwClientSize = GetModuleSize(dwProcessId, _T("client.dll"));
    std::cout << "[+] client.dll size: " << dwClientSize << " bytes\n";

    // Locate the necessary offsets based on the client base address
    dwClientState = dwClientBase + 0x589D38;  // Example offset, adjust as needed
    dwLocalPlayer = dwClientBase + 0xD3C20C;  // Example offset, adjust as needed
    dwEntityList = dwClientBase + 0x4DCE2A4;  // Example offset, adjust as needed

    m_hViewModel = 0x32F8;         // Example offset for m_hViewModel, adjust as needed
    m_iViewModelIndex = 0x32F4;    // Example offset for m_iViewModelIndex, adjust as needed
    m_flFallbackWear = 0x31E0;     // Example offset for m_flFallbackWear, adjust as needed
    m_nFallbackPaintKit = 0x31F8;  // Example offset for m_nFallbackPaintKit, adjust as needed
    m_iItemIDHigh = 0x2C00;        // Example offset for m_iItemIDHigh, adjust as needed
    m_iEntityQuality = 0x2B00;     // Example offset for m_iEntityQuality, adjust as needed
    m_iItemDefinitionIndex = 0x2C00; // Example offset for m_iItemDefinitionIndex, adjust as needed
    m_hActiveWeapon = 0x2E80;      // Example offset for m_hActiveWeapon, adjust as needed
    m_hMyWeapons = 0x2F00;         // Example offset for m_hMyWeapons, adjust as needed
    m_nModelIndex = 0x2500;        // Example offset for m_nModelIndex, adjust as needed
    m_dwModelPrecache = 0x2F4C;    // Example offset for m_dwModelPrecache, adjust as needed

    std::cout << "[+] Skins and Knife Model Initialized\n";

    // Call to modify skins and weapons
    xSkins(knifeID, skinID);

    std::cout << "[+] Exiting... Press any key to close.\n";
    std::cin.get(); // Wait for user input to prevent the program from closing immediately

    return 0;
}
