// This is a hardcoded and mostly copied injector that starts Chromium, whose path is defined in ".\path.txt", and injects the dll located at ".\Ignorer.dll".
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <filesystem>

int main() {
	std::string chromiumPath; // Get the Chromium executable
	std::ifstream pathStream("path.txt");
	pathStream.seekg(0, std::ios::end);
	chromiumPath.resize(pathStream.tellg());
	pathStream.seekg(0);
	pathStream.read(chromiumPath.data(), chromiumPath.size());
	std::cout << "Chromium Path: " << chromiumPath << std::endl;

	std::string dllPath = std::filesystem::canonical("Ignorer.dll").string(); // Get the dll path
	std::cout << "Dll Path: " << dllPath << std::endl;

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	char cmd[] = ""; // Empty command line; cannot be const
	if (CreateProcessA(chromiumPath.c_str(), cmd, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) { // Create the Chromium process
		HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, pi.dwProcessId);
		std::cout << "Process Handle: " << proc << std::endl;

		if (proc != NULL) {
			uintptr_t loadLib = (uintptr_t)GetProcAddress(LoadLibraryA("kernel32.dll"), "LoadLibraryA");
			std::cout << "LoadLib Address: " << proc << std::endl;
			
			if (loadLib != NULL) {
				LPVOID alloc = VirtualAllocEx(proc, NULL, dllPath.size() + 1, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				std::cout << "Allocated Address: " << alloc << std::endl;

				if (alloc != NULL) {
					SIZE_T bytesWritten;

					if (WriteProcessMemory(proc, alloc, dllPath.c_str(), dllPath.size(), &bytesWritten)) {
						DWORD threadId;
						CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)loadLib, alloc, 0, &threadId);
						std::cout << "Thread Id: " << threadId << std::endl;
					}
				}
			}
		} // I don't bother closing the handles, as the program will close now anyway.
	}
	else {
		std::cerr << "Error!" << std::endl;
	}
}
