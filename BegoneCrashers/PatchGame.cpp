// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

using namespace std::literals;

void patchGame(std::int32_t patchSite, void* newCode);
void patched54EA88();
void patched81D1F6();

extern "C" __declspec(dllexport) void __stdcall NativeInjectionEntryPoint(void*)
{
    try
    {
        patchGame(0x54EA88, patched54EA88);
        patchGame(0x81D1F6, patched81D1F6);
    }
    catch (std::exception const& e)
    {
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
    }
}

void patchGame(std::int32_t patchSite, void* newCode)
{
    constexpr SIZE_T jmpSize = 6;

    // create new x86 assembly instruction
    // jmp rel32
    std::uint8_t instruction[jmpSize] = { 0x0F, 0x85, 0, 0, 0, 0 };
    std::int32_t jmpTarget = reinterpret_cast<std::int32_t>(newCode);
    std::int32_t offset = jmpTarget - (patchSite + jmpSize);
    std::memcpy(instruction + 2, &offset, sizeof(offset));

    // change the memory protection so we can alter game's memory
    DWORD oldProtection = 0;
    void* patchTarget = reinterpret_cast<void*>(patchSite);
    BOOL memoryProtecitonChanged = VirtualProtect
    (
        patchTarget,
        jmpSize,
        PAGE_EXECUTE_READWRITE,
        &oldProtection
    );
    if (not memoryProtecitonChanged)
    {
        int error = static_cast<int>(GetLastError());
        throw std::system_error
        {
            error,
            std::system_category(),
            "VirtualProtect failed"
        };
    }

    std::memcpy(patchTarget, instruction, sizeof(instruction));

    BOOL instructionCacheFlushed = FlushInstructionCache
    (
        GetCurrentProcess(),
        nullptr,
        0
    );
    if (not instructionCacheFlushed)
    {
        int error = static_cast<int>(GetLastError());
        throw std::system_error
        {
            error,
            std::system_category(),
            "VirtualProtect failed"
        };
    }
}

__declspec(naked) void patched54EA88()
{
    //  mov     ecx, [edx + 374h]
    //  mov     edx, [esi + 374h]
    //  mov     eax, [edx + 144h] // This crash!!!
    //  mov     edi, [ecx + 144h] // This crash!!!
    __asm
    {
        // availaible registers: edi, ebp, eax, ecx

        mov     edi, dword ptr[esi + 0x374];
        test    edi, edi; // check if it's null
        jz      failure;

        mov     edi, dword ptr[edx + 0x374];
        test    edi, edi; // check if it's null
        jz      failure;

        mov     edi, dword ptr[ebx];
        mov     ebp, dword ptr[esp + (0x5C - 0x48)];
        mov     eax, 0x54EA8E;
        jmp     eax;
    failure:
        mov     edi, 0x54EB32;
        jmp     edi;
    }
}

__declspec(naked) void patched81D1F6()
{
    __asm
    {
        mov     ecx, dword ptr[ebx + 0x374];
        mov     eax, dword ptr[edi + 0x374];
        // let edx be 0 or [ecx + 0x144], only if ecx is not null
        test    ecx, ecx; // check ecx
        mov     edx, 0;
        jz      afterEdx;
        mov     edx, dword ptr[ecx + 0x144]; // This crash!!!
    afterEdx:
        // let ecx be 0 or [eax + 0x144], only if eax is not null
        test    eax, eax; // check eax
        mov     ecx, 0;
        jz      afterEcx;
        mov     ecx, dword ptr[eax + 0x144]; // This crash!!!
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