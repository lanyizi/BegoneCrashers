#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

extern "C"
#ifdef BEGONECRASHERS_EXPORTS
__declspec(dllexport)
#else
__declspec(dllimport)
#endif
LRESULT CALLBACK WindowsHookEntryPoint
(
    int    nCode,
    WPARAM wParam,
    LPARAM lParam
);
