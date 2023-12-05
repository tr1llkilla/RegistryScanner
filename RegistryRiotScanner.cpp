/*
Copyright (C) 2023 tr1llkilla
    This program comes with ABSOLUTELY NO WARRANTY.
    This is free software, and you are welcome to redistribute it
    under legal conditions.
Author:
tr1llkilla

Author's note:
This program is unfinished, but does the trick to get the job done manually
Feel free to credit this work in any future registry forensic or data analysis works!
*/

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>

// Function prototypes
bool ContainsSubstring(const std::wstring& str, const std::wstring& substr);
void SearchRegistryRecursive(HKEY hKey, const std::vector<std::wstring>& searchTerms, std::vector<std::wstring>& foundItems);
void SearchKeys(HKEY hKey, const std::wstring& searchTerm, std::vector<std::wstring>& foundItems);
void SearchValues(HKEY hKey, const std::wstring& searchTerm, std::vector<std::wstring>& foundItems);

// Function to check if a wide string contains a specific substring
bool ContainsSubstring(const std::wstring& str, const std::wstring& substr) {
    return str.find(substr) != std::wstring::npos;
}

// Function to search for a term in key names, value names, and data
void SearchRegistryRecursive(HKEY hKey, const std::vector<std::wstring>& searchTerms, std::vector<std::wstring>& foundItems) {
    // Search in key names
    for (const auto& searchTerm : searchTerms) {
        SearchKeys(hKey, searchTerm, foundItems);
    }

    // Search in value names and data
    for (const auto& searchTerm : searchTerms) {
        SearchValues(hKey, searchTerm, foundItems);
    }

    // Recursively search in subkeys
    DWORD index = 0;
    wchar_t subkeyName[256];
    DWORD subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);

    while (RegEnumKeyExW(hKey, index++, subkeyName, &subkeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        HKEY hSubKey;
        if (RegOpenKeyExW(hKey, subkeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            SearchRegistryRecursive(hSubKey, searchTerms, foundItems);
            RegCloseKey(hSubKey);
        }

        // Reset subkeyNameSize for the next iteration
        subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);
    }
}

// Function to search for a term in key names
void SearchKeys(HKEY hKey, const std::wstring& searchTerm, std::vector<std::wstring>& foundItems) {
    DWORD index = 0;
    wchar_t subkeyName[256];
    DWORD subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);

    while (RegEnumKeyExW(hKey, index++, subkeyName, &subkeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
        if (ContainsSubstring(subkeyName, searchTerm)) {
            std::wcout << L"Found in key name: " << subkeyName << std::endl;
            foundItems.push_back(L"Found in key name: " + std::wstring(subkeyName));
        }

        // Reset subkeyNameSize for the next iteration
        subkeyNameSize = sizeof(subkeyName) / sizeof(subkeyName[0]);
    }
}

// Function to search for a term in value names and data
void SearchValues(HKEY hKey, const std::wstring& searchTerm, std::vector<std::wstring>& foundItems) {
    DWORD index = 0;
    wchar_t valueName[256];
    DWORD dataSize = 0;
    std::vector<BYTE> data(4096);

    while (true) {
        dataSize = static_cast<DWORD>(data.size());

        // Retrieve the value name and data
        if (RegEnumValueW(hKey, index++, valueName, &dataSize, nullptr, nullptr, data.data(), &dataSize) != ERROR_SUCCESS) {
            break; // Break the loop if there are no more values
        }

        // Check if the term is present in the valueName
        if (ContainsSubstring(valueName, searchTerm)) {
            // Convert the raw data to a wide string
            std::wstring valueData(reinterpret_cast<wchar_t*>(data.data()), dataSize / sizeof(wchar_t));

            // Output the found information
            std::wcout << L"Found in value name: " << valueName << std::endl;
            std::wcout << L"Found in value data: " << valueData << std::endl;

            // Store in the vector for later display
            foundItems.push_back(L"Found in value name: " + std::wstring(valueName) + L", value data: " + valueData);
        }
    }
}

int main() {
    // Specify the search terms
    std::vector<std::wstring> searchTerms = { L"Riot", L"RiotClient", L"RiotGames", L"League", L"Legends", L"Valorant", L"Vanguard", L"Runeterra" };

    // Open different root registry keys and perform the search
    HKEY rootKeys[] = { HKEY_CURRENT_USER, HKEY_CLASSES_ROOT, HKEY_CURRENT_CONFIG, HKEY_LOCAL_MACHINE, HKEY_USERS };

    // Vector to store found items
    std::vector<std::wstring> foundItems;

    for (HKEY hRootKey : rootKeys) {
        // Call the function to search for the terms in key names, value names, and data
        SearchRegistryRecursive(hRootKey, searchTerms, foundItems);

        // Convert HKEY to string for display
        std::wstring rootKeyName;
        if (hRootKey == HKEY_CURRENT_USER) rootKeyName = L"HKEY_CURRENT_USER";
        else if (hRootKey == HKEY_CLASSES_ROOT) rootKeyName = L"HKEY_CLASSES_ROOT";
        else if (hRootKey == HKEY_CURRENT_CONFIG) rootKeyName = L"HKEY_CURRENT_CONFIG";
        else if (hRootKey == HKEY_LOCAL_MACHINE) rootKeyName = L"HKEY_LOCAL_MACHINE";
        else if (hRootKey == HKEY_USERS) rootKeyName = L"HKEY_USERS";

        // Display the found items
        std::wcout << L"\nFound items containing the search terms in root key " << rootKeyName << L":\n";
        for (const auto& item : foundItems) {
            std::wcout << item << std::endl;
        }

        // Clear the vector for the next iteration
        foundItems.clear();
    }

    return 0;
}
