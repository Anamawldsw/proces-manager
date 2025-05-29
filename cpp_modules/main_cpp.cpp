#include <iostream>
#include <iomanip>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#include <map>

#pragma comment(lib, "Advapi32.lib")

namespace py = pybind11;

ULONGLONG totalCpuTime = 0;
DWORDLONG physMemUsedTotal = 0;
ULONGLONG totalIoRead = 0;
ULONGLONG totalIoWrite = 0;

void printError(const wchar_t* msg) {
    std::wcerr << L"Błąd: " << msg << L" (kod " << GetLastError() << L")" << std::endl;
}

int get_total_cpu_time() {
    return static_cast<int>(totalCpuTime);
}

int get_total_ram_used_mb() {
    return static_cast<int>(physMemUsedTotal / (1024 * 1024));
}

int get_total_io_read_mb() {
    return static_cast<int>(totalIoRead / (1024 * 1024));
}

int get_total_io_write_mb() {
    return static_cast<int>(totalIoWrite / (1024 * 1024));
}

int get_uptime_ms() {
    return GetTickCount64();
}

int get_cpu_count() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return static_cast<int>(sysInfo.dwNumberOfProcessors);
}

int dodaj(int a, int b) {
    return a + b;
}

void init_all_metrics() {
    totalCpuTime = 0;
    physMemUsedTotal = 0;
    totalIoRead = 0;
    totalIoWrite = 0;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return;
    }

    do {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                physMemUsedTotal += pmc.WorkingSetSize;
            }

            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                ULARGE_INTEGER uKernelTime, uUserTime;
                uKernelTime.LowPart = kernelTime.dwLowDateTime;
                uKernelTime.HighPart = kernelTime.dwHighDateTime;
                uUserTime.LowPart = userTime.dwLowDateTime;
                uUserTime.HighPart = userTime.dwHighDateTime;

                ULONGLONG processCpuTime = (uKernelTime.QuadPart + uUserTime.QuadPart) / 10000;
                totalCpuTime += processCpuTime;
            }

            IO_COUNTERS ioCounters;
            if (GetProcessIoCounters(hProcess, &ioCounters)) {
                totalIoRead += ioCounters.ReadTransferCount;
                totalIoWrite += ioCounters.WriteTransferCount;
            }
            CloseHandle(hProcess);
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
}

std::vector<std::map<std::string, py::object>> get_process_list() {
    std::vector<std::map<std::string, py::object>> process_list;

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) return process_list;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return process_list;
    }

    do {
        std::map<std::string, py::object> proc_info;
        proc_info["name"] = py::cast(std::string(pe32.szExeFile));
        proc_info["pid"] = py::cast((int)pe32.th32ProcessID);

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess) {
            PROCESS_MEMORY_COUNTERS pmc;
            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                proc_info["ram_kb"] = py::cast((int)(pmc.WorkingSetSize / 1024));
            }

            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                ULARGE_INTEGER uKernelTime, uUserTime;
                uKernelTime.LowPart = kernelTime.dwLowDateTime;
                uKernelTime.HighPart = kernelTime.dwHighDateTime;
                uUserTime.LowPart = userTime.dwLowDateTime;
                uUserTime.HighPart = userTime.dwHighDateTime;

                ULONGLONG processCpuTime = (uKernelTime.QuadPart + uUserTime.QuadPart) / 10000;
                proc_info["cpu_time_ms"] = py::cast((int)processCpuTime);
            }

            IO_COUNTERS ioCounters;
            if (GetProcessIoCounters(hProcess, &ioCounters)) {
                proc_info["io_read_kb"] = py::cast((int)(ioCounters.ReadTransferCount / 1024));
                proc_info["io_write_kb"] = py::cast((int)(ioCounters.WriteTransferCount / 1024));
            }
            CloseHandle(hProcess);
        }

        process_list.push_back(proc_info);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return process_list;
}

PYBIND11_MODULE(main_cpp, m) {
    m.def("dodaj", &dodaj);
    m.def("init_all_metrics", &init_all_metrics);
    m.def("get_total_cpu_time", &get_total_cpu_time);
    m.def("get_total_ram_used_mb", &get_total_ram_used_mb);
    m.def("get_total_io_read_mb", &get_total_io_read_mb);
    m.def("get_total_io_write_mb", &get_total_io_write_mb);
    m.def("get_uptime_ms", &get_uptime_ms);
    m.def("get_cpu_count", &get_cpu_count);
    m.def("get_process_list", &get_process_list);
}



















// #include <iostream>
// #include <iomanip>
// #include <windows.h>
// #include <tlhelp32.h>
// #include <psapi.h>
// #pragma comment(lib, "Advapi32.lib")


// #include <pybind11/pybind11.h>
// namespace py = pybind11;

// void printError(const wchar_t* msg) {
//     std::wcerr << L"Błąd: " << msg << L" (kod " << GetLastError() << L")" << std::endl;
// }



// ULONGLONG totalCpuTime = 0;

// int dodaj(int a, int b) {
//     return a + b;
// }
// int get_total_cpu_time() {
//     return totalCpuTime;
// }
// void init_all_metrics() {
//     totalCpuTime = 0;

//     HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//     if (hProcessSnap == INVALID_HANDLE_VALUE) return;

//     PROCESSENTRY32 pe32;
//     pe32.dwSize = sizeof(PROCESSENTRY32);
//     if (!Process32First(hProcessSnap, &pe32)) {
//         CloseHandle(hProcessSnap);
//         return;
//     }

//     do {
//         HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
//         if (hProcess) {
//             FILETIME creationTime, exitTime, kernelTime, userTime;
//             if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
//                 ULARGE_INTEGER uKernelTime, uUserTime;
//                 uKernelTime.LowPart = kernelTime.dwLowDateTime;
//                 uKernelTime.HighPart = kernelTime.dwHighDateTime;
//                 uUserTime.LowPart = userTime.dwLowDateTime;
//                 uUserTime.HighPart = userTime.dwHighDateTime;

//                 ULONGLONG processCpuTime = (uKernelTime.QuadPart + uUserTime.QuadPart) / 10000;
//                 totalCpuTime += processCpuTime;
//             }
//             CloseHandle(hProcess);
//         }
//     } while (Process32Next(hProcessSnap, &pe32));

//     CloseHandle(hProcessSnap);
// }



// PYBIND11_MODULE(main_cpp, m) {
//     m.def("dodaj", &dodaj, "Dodaje dwie liczby");
//     m.def("get_total_cpu_time", &get_total_cpu_time, "Zwraca całkowity czas CPU systemu");
//     m.def("init_all_metrics", &init_all_metrics);
// }

    

// int main() {
//     HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//     if (hProcessSnap == INVALID_HANDLE_VALUE) {
//         printError(L"CreateToolhelp32Snapshot (of processes)");
//         return 1;
//     }

//     MEMORYSTATUSEX memInfo;
//     memInfo.dwLength = sizeof(memInfo);
//     GlobalMemoryStatusEx(&memInfo);
//     DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
//     DWORDLONG physMemUsedTotal = 0;

//     //ULONGLONG totalCpuTime = 0;
//     ULONGLONG totalIoRead = 0;
//     ULONGLONG totalIoWrite = 0;

//     DWORD uptimeMs = GetTickCount();
//     SYSTEM_INFO sysInfo;
//     GetSystemInfo(&sysInfo);
//     DWORD numProcessors = sysInfo.dwNumberOfProcessors;

//     PROCESSENTRY32 pe32;
//     pe32.dwSize = sizeof(PROCESSENTRY32);

//     if (!Process32First(hProcessSnap, &pe32)) {
//         printError(L"Process32First");
//         CloseHandle(hProcessSnap);
//         return 1;
//     }

//     do {
//         std::wcout << L">PROCESS NAME: " << pe32.szExeFile << std::endl;
//         HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

//         if (hProcess != NULL) {
//             // RAM usage
//             PROCESS_MEMORY_COUNTERS pmc;
//             if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
//                 DWORD ramKB = pmc.WorkingSetSize / 1024;
//                 physMemUsedTotal += pmc.WorkingSetSize;
//                 std::wcout << L" RAM (WorkingSet)   = " << ramKB << L" KB" << std::endl;
//             }

//             // CPU time
//             FILETIME creationTime, exitTime, kernelTime, userTime;
//             if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
//                 ULARGE_INTEGER uKernelTime, uUserTime;
//                 uKernelTime.LowPart = kernelTime.dwLowDateTime;
//                 uKernelTime.HighPart = kernelTime.dwHighDateTime;
//                 uUserTime.LowPart = userTime.dwLowDateTime;
//                 uUserTime.HighPart = userTime.dwHighDateTime;

//                 ULONGLONG processCpuTime = (uKernelTime.QuadPart + uUserTime.QuadPart) / 10000;
//                 totalCpuTime += processCpuTime;
//                 double cpuPercent = (double)processCpuTime / (uptimeMs * numProcessors) * 100.0;
//                 std::wcout << L" CPU time           = " << processCpuTime << L" ms ("
//                            << std::fixed << std::setprecision(2) << cpuPercent << L"%)" << std::endl;
//             }

//             // Disk I/O
//             IO_COUNTERS ioCounters;
//             if (GetProcessIoCounters(hProcess, &ioCounters)) {
//                 ULONGLONG readKB = ioCounters.ReadTransferCount / 1024;
//                 ULONGLONG writeKB = ioCounters.WriteTransferCount / 1024;
//                 totalIoRead += ioCounters.ReadTransferCount;
//                 totalIoWrite += ioCounters.WriteTransferCount;
//                 std::wcout << L" IO Read            = " << readKB << L" KB" << std::endl;
//                 std::wcout << L" IO Write           = " << writeKB << L" KB" << std::endl;
//             }

//             CloseHandle(hProcess);
//         } else {
//             printError(L"OpenProcess");
//         }

//         std::wcout << std::endl;

//     } while (Process32Next(hProcessSnap, &pe32));

//     CloseHandle(hProcessSnap);
//         // SYSTEM TOTAL
//     std::wcout << L"====================== SYSTEM TOTAL ======================" << std::endl;
//     std::wcout << L" Total RAM used      = " << physMemUsedTotal / (1024 * 1024) << L" MB ("
//                << (100 * physMemUsedTotal / totalPhysMem) << L"%)" << std::endl;
//     std::wcout << L" Total CPU time      = " << totalCpuTime << L" ms ("
//                << std::fixed << std::setprecision(2)
//                << ((double)totalCpuTime / (uptimeMs * numProcessors)) * 100.0 << L"%)" << std::endl;
//     std::wcout << L" Total IO Read       = " << totalIoRead / (1024 * 1024) << L" MB" << std::endl;
//     std::wcout << L" Total IO Write      = " << totalIoWrite / (1024 * 1024) << L" MB" << std::endl;


//     HKEY hKey;
//     LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
//         L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
//         0, KEY_READ, &hKey);
//     if (lRes == ERROR_SUCCESS) {
//         DWORD mhz = 0;
//         DWORD size = sizeof(mhz);
//         RegQueryValueExW(hKey, L"~MHz", NULL, NULL, (LPBYTE)&mhz, &size);
//         std::wcout << L" CPU Clock Speed     = " << mhz << L" MHz" << std::endl;
//         RegCloseKey(hKey);
//     }

//     return 0;
// }



