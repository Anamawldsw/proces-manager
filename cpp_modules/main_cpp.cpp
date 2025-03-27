#include <iostream>
//#include <pybind11/pybind11.h> //Działa

#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <stdio.h>

BOOL GetProcessList();
// BOOL ListProcessModules(Dword dwPID);
// BOOL ListProcessThreads(Dword dwOwnerPID);
void printError(TCHAR const* msg);
//namespace py = pybind11;

int main(void)
{
    GetProcessList();
    return 0;
}

BOOL GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
        printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if(!Process32First(hProcessSnap, &pe32))
    {
        printError(TEXT("Process32First"));
        CloseHandle(hProcessSnap);
        return(FALSE);
    }

    do
    {
        _tprintf(TEXT("\n\n====================================================="));
        _tprintf(TEXT("\nPROCESS NAME: %s"),pe32.szExeFile);
        _tprintf(TEXT("\n\n====================================================="));

    } while(Process32Next(hProcessSnap, &pe32));

    return 0;
}

//----------------------------------------
int dodaj(int a, int b) {
    return a + b;
}

// PYBIND11_MODULE(main_cpp, m) {
//     m.def("dodaj", &dodaj, "Dodaje dwie liczby");
// }