// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.hpp"
#include <atomic>
#include <system_error>

bool checkAddress(std::uintptr_t address);
void patchGame(std::int32_t patchSite, void* newCode);
void patched54EA88();
void patched590048();
void patched81D1F6();

std::atomic_flag alreadyInitialized = {};

extern "C" LRESULT CALLBACK WindowsHookEntryPoint(int code, WPARAM wParam, LPARAM lParam)
{
    // test_and_set() checks the flag, and sets the flag to true
    if (not alreadyInitialized.test_and_set())
    {
        try
        {
            bool isRa3 = false;
            if (checkAddress(0xC5B6C4))
            {
                // Steam version
                patchGame(0x54EA88, &patched54EA88);
                patchGame(0x81D1F6, &patched81D1F6);
                isRa3 = true;
            }
            else if (checkAddress(0xC6262C))
            {
                // Origin version
                patchGame(0x590048, &patched590048);
                patchGame(0x85B386, &patched81D1F6);
                isRa3 = true;
            }
            else
            {
                MessageBoxA(nullptr, "This is not Red Alert 3", "BegoneCrashers", MB_OK);
            }

            if (isRa3)
            {
                // prevent this dll from being unloaded
                HMODULE unused = nullptr;
                GetModuleHandleExW
                (
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                    reinterpret_cast<LPCWSTR>(&WindowsHookEntryPoint),
                    &unused
                );
            }
        }
        catch (std::exception const& e)
        {
            MessageBoxA(nullptr, e.what(), "BegoneCrashers Error", MB_OK);
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

bool checkAddress(std::uintptr_t address)
{
    return std::strncmp(reinterpret_cast<char const*>(address), "RedAlert", 8) == 0;
}

void patchGame(std::int32_t patchSite, void* newCode)
{
    constexpr SIZE_T jmpSize = 5;

    // create new x86 assembly instruction
    // jmp rel32
    std::uint8_t instruction[jmpSize] = { 0xE9, 0, 0, 0, 0 };
    std::int32_t jmpTarget = reinterpret_cast<std::int32_t>(newCode);
    std::int32_t offset = jmpTarget - (patchSite + jmpSize);
    std::memcpy(instruction + 1, &offset, sizeof(offset));

    // change the memory protection so we can alter game's memory
    DWORD oldProtection = 0;
    void* patchTarget = reinterpret_cast<void*>(patchSite);
    BOOL memoryProtectionChanged = VirtualProtect
    (
        patchTarget,
        jmpSize,
        PAGE_EXECUTE_READWRITE,
        &oldProtection
    );
    if (not memoryProtectionChanged)
    {
        int errorCode = GetLastError();
        throw std::system_error
        {
            errorCode,
            std::system_category(),
            "VirtualProtect failed"
        };
    }
    // patch the game by writing new instruction
    // to game's memory
    std::memcpy(patchTarget, instruction, sizeof(instruction));

    // let CPU know that we have altered game's memory
    BOOL instructionCacheFlushed = FlushInstructionCache
    (
        GetCurrentProcess(),
        nullptr,
        0
    );
    if (not instructionCacheFlushed)
    {
        int errorCode = GetLastError();
        throw std::system_error
        {
            errorCode,
            std::system_category(),
            "FlushInstructionCache failed"
        };
    }
}

__declspec(naked) void patched54EA88()
{
    //  mov     ecx, [edx + 374h]
    //  mov     edx, [esi + 374h]
    //  mov     eax, [edx + 144h] // This might crash
    //  mov     edi, [ecx + 144h] // This might crash
    __asm
    {
        // availaible registers: edi, ebp, eax, ecx

        mov     edi, dword ptr[esi + 0x374];
        test    edi, edi; // check if it's null
        // if it's null, jump away to prevent crash
        jz      failure;

        mov     edi, dword ptr[edx + 0x374];
        test    edi, edi; // check if it's null
        // if it's null, jump away to prevent crash
        jz      failure;

        mov     edi, dword ptr[ebx];
        mov     ebp, dword ptr[esp + (0x5C - 0x48)];

        // Now we are sure it won't crash.
        // Go back to game code which loads [edx + 144h] and [ecx + 144h]
        // 0x54EA8E is Steam version address.
        // Origin version address: 0x59004E
        push    0x54EA8E; // go back to game code
        ret;
    failure:
        // We think the game is going crash
        // We jump away to prevent executing the crashing code.
        // 0x54EB32 is Steam version address.
        // Origin version address: 0x5900F2
        push    0x54EB32; // jump away
        ret;
    }
}

__declspec(naked) void patched590048()
{
    // Fix crash for Origin version at 0x590048

    //  mov     ecx, [edx + 374h]
    //  mov     edx, [esi + 374h]
    //  mov     eax, [edx + 144h] // This might crash
    //  mov     edi, [ecx + 144h] // This might crash
    __asm
    {
        // availaible registers: edi, ebp, eax, ecx

        mov     edi, dword ptr[esi + 0x374];
        test    edi, edi; // check if it's null
        // if it's null, jump away to prevent crash
        jz      failure;

        mov     edi, dword ptr[edx + 0x374];
        test    edi, edi; // check if it's null
        // if it's null, jump away to prevent crash
        jz      failure;

        mov     edi, dword ptr[ebx];
        mov     ebp, dword ptr[esp + (0x5C - 0x48)];

        // Now we are sure it won't crash.
        // Go back to game code which loads [edx + 144h] and [ecx + 144h]
        // 0x59004E is Origin version address.
        // Steam version address: 0x54EA8E
        push    0x59004E; // go back to game code
        ret;
    failure:
        // We think the game is going crash
        // We jump away to prevent executing the crashing code.
        // 0x5900F2 is Origin version address.
        // Steam version address: 0x54EB32
        push    0x5900F2; // jump away
        ret;
    }
}

__declspec(naked) void patched81D1F6()
{
    // Steam Address: 0x81D1F6
    // Origin Address: 0x85B386
    __asm
    {
        mov     ecx, dword ptr[ebx + 0x374];
        mov     eax, dword ptr[edi + 0x374];
        // let edx be 0 or [ecx + 0x144], only if ecx is not null
        test    ecx, ecx; // check ecx
        mov     edx, 0;
        jz      afterEdx; // skip next instruction to prevent crash
        mov     edx, dword ptr[ecx + 0x144]; // This might crash
    afterEdx:
        // let ecx be 0 or [eax + 0x144], only if eax is not null
        test    eax, eax; // check eax
        mov     ecx, 0;
        jz      afterEcx; // skip next instruction to prevent crash
        mov     ecx, dword ptr[eax + 0x144]; // This might crash
    afterEcx:
        cmp     ecx, edx;
        pop     edi;
        sbb     eax, eax;
        pop     esi;
        neg     eax;
        pop     ebx;
        add     esp, 8;
        retn    8;
    }
}
