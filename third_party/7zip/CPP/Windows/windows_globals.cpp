// windows_globals.cpp - Global variables for Windows compatibility layer

#include <windows.h>

// This global variable tracks whether we're running on Windows NT (vs Windows 9x)
// On modern systems, this is always true, but 7-Zip's code still checks it
bool g_IsNT = true;  // We only support NT-based systems (Windows 2000+)

// Initialize the global on startup
struct WindowsGlobalsInitializer {
    WindowsGlobalsInitializer() {
        // On modern Windows, we're always NT
        OSVERSIONINFOA osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

#pragma warning(push)
#pragma warning(disable : 4996)  // GetVersionExA is deprecated but still used by 7-Zip
        if (GetVersionExA(&osvi)) {
            g_IsNT = (osvi.dwPlatformId >= VER_PLATFORM_WIN32_NT);
        }
#pragma warning(pop)
    }
} g_windowsGlobalsInit;
