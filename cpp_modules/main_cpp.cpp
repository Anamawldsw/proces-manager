#include <iostream>
#include <iomanip>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

// #include <pybind11/pybind11.h>
// namespace py = pybind11;

void printError(const wchar_t* msg) {
    std::wcerr << L"Błąd: " << msg << L" (kod " << GetLastError() << L")" << std::endl;
}

int dodaj(int a, int b) {
    return a + b;
}

// PYBIND11_MODULE(main_cpp, m) {
//     m.def("dodaj", &dodaj, "Dodaje dwie liczby");
// }

int main() {
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printError(L"CreateToolhelp32Snapshot (of processes)");
        return 1;
    }

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    DWORDLONG physMemUsedTotal = 0;

    ULONGLONG totalCpuTime = 0;
    ULONGLONG totalIoRead = 0;
    ULONGLONG totalIoWrite = 0;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        printError(L"Process32First");
        CloseHandle(hProcessSnap);
        return 1;
    }

    do {
        std::wcout << L">PROCESS NAME: " << pe32.szExeFile << std::endl;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

        if (hProcess != NULL) {
            // RAM usage
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                DWORD ramKB = pmc.WorkingSetSize / 1024;
                physMemUsedTotal += pmc.WorkingSetSize;
                std::wcout << L" RAM (WorkingSet)   = " << ramKB << L" KB" << std::endl;
            }

            // CPU time
            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                ULARGE_INTEGER uKernelTime, uUserTime;
                uKernelTime.LowPart = kernelTime.dwLowDateTime;
                uKernelTime.HighPart = kernelTime.dwHighDateTime;
                uUserTime.LowPart = userTime.dwLowDateTime;
                uUserTime.HighPart = userTime.dwHighDateTime;

                ULONGLONG processCpuTime = (uKernelTime.QuadPart + uUserTime.QuadPart) / 10000;
                totalCpuTime += processCpuTime;
                std::wcout << L" CPU time           = " << processCpuTime << L" ms" << std::endl;
            }

            // Disk I/O
            IO_COUNTERS ioCounters;
            if (GetProcessIoCounters(hProcess, &ioCounters)) {
                ULONGLONG readKB = ioCounters.ReadTransferCount / 1024;
                ULONGLONG writeKB = ioCounters.WriteTransferCount / 1024;
                totalIoRead += ioCounters.ReadTransferCount;
                totalIoWrite += ioCounters.WriteTransferCount;
                std::wcout << L" IO Read            = " << readKB << L" KB" << std::endl;
                std::wcout << L" IO Write           = " << writeKB << L" KB" << std::endl;
            }

            CloseHandle(hProcess);
        } else {
            printError(L"OpenProcess");
        }

        std::wcout << std::endl;

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    std::wcout << L"====================== SYSTEM TOTAL ======================" << std::endl;
    std::wcout << L" Total RAM used      = " << physMemUsedTotal / (1024 * 1024) << L" MB ("
               << (100 * physMemUsedTotal / totalPhysMem) << L"%)" << std::endl;
    std::wcout << L" Total CPU time      = " << totalCpuTime << L" ms" << std::endl;
    std::wcout << L" Total IO Read       = " << totalIoRead / (1024 * 1024) << L" MB" << std::endl;
    std::wcout << L" Total IO Write      = " << totalIoWrite / (1024 * 1024) << L" MB" << std::endl;

    return 0;
}
