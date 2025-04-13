#include <windows.h>
#include <shellapi.h>
#include <iostream>

int main()
{
    // Path to your C# EXE - Make sure the path is correct
    LPCWSTR appPath = L"C:\\Users\\Sandeep Maheshwari\\source\\repos\\WebViewLauncher\\WebViewLauncher\\bin\\Debug\\net8.0-windows\\WebViewLauncher.exe";

    // Launch the C# application using ShellExecute
    HINSTANCE result = ShellExecute(
        NULL,             // Parent window handle
        L"open",          // Operation to perform
        appPath,          // Path to the application
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
