#include <windows.h>
#include <shellapi.h>
#include <iostream>

int main()
{
    // Use a system path instead of hardcoded user-specific path
    WCHAR szPath[MAX_PATH];
    // Default path or you could read from registry
    wcscpy_s(szPath, MAX_PATH, L"C:\\Program Files\\WebViewLauncher\\WebViewLauncher.exe");

    // Launch the C# application using ShellExecute
    HINSTANCE result = ShellExecute(
        NULL,             // Parent window handle
        L"open",          // Operation to perform
        szPath,           // Path to the application
        NULL,             // Arguments (NULL if none)
        NULL,             // Default directory
        SW_SHOWNORMAL     // Window display option
    );

    // Check if ShellExecute was successful
    if ((int)result <= 32)
    {
        std::wcout << L"Failed to launch the application. Error code: " << (int)result << std::endl;
    }
    else
    {
        std::wcout << L"Application launched successfully!" << std::endl;
    }

    return 0;
}
