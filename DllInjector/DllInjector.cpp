// DllInjector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "../BegoneCrashers/framework.hpp"
#include <iostream>
#include <thread>

// Make message boxes less ugly by enabling visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

void findRa3AndfixCrash(HMODULE begoneCrashersDll);
HMODULE getBegoneCrashersDllHandle();

int main()
{
    std::cout << "Begone crashers!\n";
    std::cout << "https://github.com/lanyizi/BegoneCrashers\n";
    std::cout << "Keep this window open, launch your game as usual, \n";
    std::cout << "now crashers cannot crash your RA3 anymore\n\n";
    std::cout << "Start looking for RA3 game process..." << std::endl;
    try
    {
        HMODULE begoneCrashersDll = getBegoneCrashersDllHandle();
        while (true)
        {
            findRa3AndfixCrash(begoneCrashersDll);
            // wait 10 seconds before checking again
            Sleep(10000);
        }
    }
    catch (std::exception const& e)
    {
        MessageBoxA(nullptr, e.what(), "BegoneCrashers Error", MB_OK | MB_ICONERROR);
    }
}

void findRa3AndfixCrash(HMODULE begoneCrashersDll)
{
    // 41DAF790-16F5-4881-8754-59FD8CF3B8D2 is RA3's window class name, defined by EA
    HWND ra3Window = FindWindowW(L"41DAF790-16F5-4881-8754-59FD8CF3B8D2", nullptr);
    if (ra3Window == nullptr)
    {
        // RA3 is not running
        return;
    }
    DWORD ra3ProcessId = 0;
    DWORD ra3ThreadId = GetWindowThreadProcessId(ra3Window, &ra3ProcessId);
    // Open RA3 process
    HANDLE ra3Process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ra3ProcessId);
    if (ra3Process == nullptr)
    {
        int errorCode = GetLastError();
        throw std::system_error
        {
            errorCode,
            std::system_category(),
            "Failed to open RA3 game process, you can try running this tool with administrator priviledge"
        };
    }
    std::cout << "Found RA3 game process with process id " << ra3ProcessId << std::endl;
    // Inject BegoneCrashers dll into RA3 process
    HHOOK hookHandle = SetWindowsHookExW
    (
        WH_CALLWNDPROC,
        WindowsHookEntryPoint,
        begoneCrashersDll,
        ra3ThreadId
    );
    if (hookHandle == nullptr)
    {
        int errorCode = GetLastError();
        std::string errorMessage = "Failed to inject BegoneCrashers dll into RA3 process: ";
        errorMessage += std::system_category().message(errorCode);
        MessageBoxA(nullptr, errorMessage.c_str(), "BegoneCrashers Error", MB_OK | MB_ICONERROR);
    }
    // wait until RA3 process exits
    WaitForSingleObject(ra3Process, INFINITE);
    if (hookHandle != nullptr)
    {
        // Unhook the hook
        UnhookWindowsHookEx(hookHandle);
    }
    // Close RA3 process handle
    CloseHandle(ra3Process);
    std::cout << "RA3 game process exited" << std::endl;
}

HMODULE getBegoneCrashersDllHandle()
{
    // Retrieve BegoneCrashers dll handle
    // https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulehandleexw
    DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    LPCWSTR address = reinterpret_cast<LPCWSTR>(&WindowsHookEntryPoint);
    HMODULE begoneCrashersDll = nullptr;
    BOOL succeeded = GetModuleHandleExW(flags, address, &begoneCrashersDll);
    if (not succeeded)
    {
        int errorCode = GetLastError();
        throw std::system_error{ errorCode, std::system_category(), "Failed to load dll" };
    }
    return begoneCrashersDll;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
