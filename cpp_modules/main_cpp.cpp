#include <iostream>
#include <iomanip>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib, "Advapi32.lib")


#include <pybind11/pybind11.h>
namespace py = pybind11;

void printError(const wchar_t* msg) {
    std::wcerr << L"Błąd: " << msg << L" (kod " << GetLastError() << L")" << std::endl;
}



ULONGLONG totalCpuTime = 0;

int dodaj(int a, int b) {
    return a + b;
}
int get_total_cpu_time() {
    return totalCpuTime;
}

PYBIND11_MODULE(main_cpp, m) {
    m.def("dodaj", &dodaj, "Dodaje dwie liczby");
    m.def("get_total_cpu_time", &get_total_cpu_time, "Zwraca całkowity czas CPU systemu");}



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

    //ULONGLONG totalCpuTime = 0;
    ULONGLONG totalIoRead = 0;
    ULONGLONG totalIoWrite = 0;

    DWORD uptimeMs = GetTickCount();
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    DWORD numProcessors = sysInfo.dwNumberOfProcessors;

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
                double cpuPercent = (double)processCpuTime / (uptimeMs * numProcessors) * 100.0;
                std::wcout << L" CPU time           = " << processCpuTime << L" ms ("
                           << std::fixed << std::setprecision(2) << cpuPercent << L"%)" << std::endl;
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
        // SYSTEM TOTAL
    std::wcout << L"====================== SYSTEM TOTAL ======================" << std::endl;
    std::wcout << L" Total RAM used      = " << physMemUsedTotal / (1024 * 1024) << L" MB ("
               << (100 * physMemUsedTotal / totalPhysMem) << L"%)" << std::endl;
    std::wcout << L" Total CPU time      = " << totalCpuTime << L" ms ("
               << std::fixed << std::setprecision(2)
               << ((double)totalCpuTime / (uptimeMs * numProcessors)) * 100.0 << L"%)" << std::endl;
    std::wcout << L" Total IO Read       = " << totalIoRead / (1024 * 1024) << L" MB" << std::endl;
    std::wcout << L" Total IO Write      = " << totalIoWrite / (1024 * 1024) << L" MB" << std::endl;


    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS) {
        DWORD mhz = 0;
        DWORD size = sizeof(mhz);
        RegQueryValueExW(hKey, L"~MHz", NULL, NULL, (LPBYTE)&mhz, &size);
        std::wcout << L" CPU Clock Speed     = " << mhz << L" MHz" << std::endl;
        RegCloseKey(hKey);
    }

    return 0;
}



