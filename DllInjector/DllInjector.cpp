// DllInjector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <easyhook/easyhook.h>
#include <TlHelp32.h>
#include <filesystem>
#include <iostream>

HANDLE findRa3Process(DWORD* ra3ProcessId);

int main()
{
    std::cout << "Hello World!\n";
    std::filesystem::path dllPath = "BegoneCrashers.dll";
    if (!std::filesystem::exists(dllPath))
    {
        std::cout << "Error: Dll does not exist" << std::endl;
        std::cin.get();
    }
    dllPath = std::filesystem::canonical(dllPath);

    std::cout << "Start monitoring ra3 processes" << std::endl;
    while (true)
    {
        DWORD processId = 0;
        HANDLE ra3 = findRa3Process(&processId);
        if (ra3)
        {
            std::cout << "Found ra3 with pid " << processId << std::endl;
            std::wstring pathString = dllPath.native();
            NTSTATUS result = RhInjectLibrary
            (
                processId, 
                0, 
                EASYHOOK_INJECT_DEFAULT, 
                pathString.data(), 
                nullptr, 
                nullptr, 
                0
            );
            if (NT_SUCCESS(result))
            {
                std::cout << "Injection succeeded" << std::endl;
            }
            else
            {
                std::cout << "Injection failed" << std::endl;
                std::string errorMessage;
                std::wstring content = RtlGetLastErrorString();
                // convert wide string to narrow string
                errorMessage.resize(content.size() * 8);
                int charactersConverted = WideCharToMultiByte
                (
                    CP_ACP, 0,
                    content.data(), content.size(),
                    errorMessage.data(), errorMessage.size(),
                    nullptr, nullptr
                );
                if (charactersConverted <= 0)
                {
                    std::cout << "(Failed to retrieve error message)" << std::endl;
                }
                else
                {
                    errorMessage.resize(charactersConverted);
                    std::cout << errorMessage << std::endl;
                }
            }

            // wait until ra3 process exits
            WaitForSingleObject(ra3, INFINITE);
            CloseHandle(ra3);
        }

        // sleep for 10 seconds
        Sleep(10000);
    }
}

HANDLE findRa3Process(DWORD* ra3ProcessId)
{
    const std::wstring ra3Name = L"ra3_1.12.game";
    HANDLE ra3Process = nullptr;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        int error = static_cast<int>(GetLastError());
        std::cout
            << "Failed to create snapshot: "
            << std::system_category().message(error)
            << std::endl;
        return nullptr;
    }

    PROCESSENTRY32W entry = 
    { 
        .dwSize = sizeof(PROCESSENTRY32W) 
    };

    if (Process32FirstW(snapshot, &entry))
    {
        while (Process32NextW(snapshot, &entry))
        {
            const std::wstring_view exeName = entry.szExeFile;
            
            // check if exeName ends with name
            if (exeName.size() < ra3Name.size())
            {
                continue;
            }
            auto const end = exeName.substr(exeName.size() - ra3Name.size());
            auto const equal =
                (_wcsnicmp(end.data(), ra3Name.data(), ra3Name.size()) == 0);

            if (equal)
            {
                *ra3ProcessId = entry.th32ProcessID;
                ra3Process = OpenProcess(SYNCHRONIZE, false, entry.th32ProcessID);
                break;
            }
        }
    }

    CloseHandle(snapshot);
    return ra3Process;
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
