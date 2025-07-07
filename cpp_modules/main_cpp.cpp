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





