#include "pch.h"
#define DEBUGGER_BREAK() if(IsDebuggerPresent()) { DebugBreak(); } // Break only if a debugger is present

// Default policy registry key paths that should not return any results
static std::vector<const wchar_t*> DEFAULT_POLICY_PATHS = {
	L"SOFTWARE\\Policies\\Chromium",
	L"SOFTWARE\\Policies\\Google\\Chrome",
	L"SOFTWARE\\Policies\\Microsoft\\Edge",
	L"SOFTWARE\\Policies\\BraveSoftware\\Brave",
	L"SOFTWARE\\Policies\\Vivaldi",
	L"SOFTWARE\\Policies\\YandexBrowser"
};


// Partially taken from https://stackoverflow.com/questions/937044/determine-path-to-registry-key-from-hkey-handle-in-c
typedef DWORD(__stdcall* NtQueryKey_Type)(HANDLE KeyHandle, int KeyInformationClass, PVOID KeyInformation, ULONG Length, PULONG ResultLength);
std::wstring GetKeyPathFromHKEY(HKEY key) {
	std::wstring keyPath;

	if (key != NULL) {
		HMODULE dll = LoadLibrary(L"ntdll.dll"); // Load ntdll or use the existing loaded module

		if (dll != NULL) {
			NtQueryKey_Type func = (NtQueryKey_Type)GetProcAddress(dll, "NtQueryKey"); // Get NtQueryKey

			if (func != NULL) {
				DWORD size = 0;
				DWORD result = 0;
				result = func(key, 3, 0, 0, &size);

				if (result == 0xC0000023L) { // STATUS_BUFFER_TOO_SMALL
					size = size + 2;
					wchar_t* buffer = new (std::nothrow) wchar_t[size / sizeof(wchar_t)]; // Size is in bytes

					if (buffer != NULL) {
						result = func(key, 3, buffer, size, &size); // Run NtQueryKey

						if (result == 0x00000000L) { // STATUS_SUCCESS
							buffer[size / sizeof(wchar_t)] = L'\0';
							keyPath = std::wstring(buffer + 2); // Turn buffer into a great wstring
						}

						delete[] buffer;
					}
				}
			}
		}
	}

	return keyPath;
}


typedef LSTATUS(WINAPI* RegQueryInfoKeyW_Type)(HKEY, LPWSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME);
RegQueryInfoKeyW_Type Orig_RegQueryInfoKeyW;

// This should return an empty value list for the policy registry keys
LSTATUS WINAPI Hook_RegQueryInfoKeyW(HKEY hKey, LPWSTR lpClass, LPDWORD lpcchClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues, LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime) {
	LSTATUS result = Orig_RegQueryInfoKeyW(hKey, lpClass, lpcchClass, lpReserved, lpcSubKeys, lpcbMaxSubKeyLen, lpcbMaxClassLen, lpcValues, lpcbMaxValueNameLen, lpcbMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime); // Only that little arguments. Well done, Microsoft!

	if (hKey != NULL && lpcValues != nullptr) {
		std::wstring keyPath = GetKeyPathFromHKEY(hKey);

		// Compare keyPath with predefined default keys
		for (const wchar_t* defaultPath : DEFAULT_POLICY_PATHS) {
			if (keyPath.find(defaultPath) != std::string::npos) { // If contains defaultPath
				*lpcValues = 0; // Set amount of values to 0, so that Chromium thinks there are no values

				if (lpcbMaxValueNameLen != nullptr) {
					*lpcbMaxValueNameLen = 0;
				}
				if (lpcbMaxValueLen != nullptr) {
					*lpcbMaxValueLen = 0;
				}

				MessageBoxW(NULL, keyPath.c_str(), L"Successfully spoofed:", MB_OK); // Debug
				return result;
			}
		}
	}

	return result;
}

BOOL APIENTRY Main(LPVOID hModule) { // Hook the registry functions
	if (MH_Initialize()) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_CreateHook(&RegQueryInfoKeyW, &Hook_RegQueryInfoKeyW, (LPVOID*)&Orig_RegQueryInfoKeyW)) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_EnableHook(&RegQueryInfoKeyW)) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	return TRUE;
}

BOOL APIENTRY UnHook(LPVOID hModule) {
	if (MH_DisableHook(&RegQueryInfoKeyW)) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	if (MH_Uninitialize()) {
		DEBUGGER_BREAK();
		return FALSE;
	}

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)Main, hModule, NULL, NULL);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)Main, hModule, NULL, NULL);
			break;
	}

	return TRUE;
}
