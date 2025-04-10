@echo off
echo Uninstalling WebViewLauncher Password Reset Solution
echo ==================================================

:: Check for administrator privileges
NET SESSION >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo Error: Administrator privileges required!
    echo Please run this script as Administrator.
    pause
    exit /b 1
)

:: Unregister the credential provider DLL
echo Unregistering credential provider...
regsvr32 /u /s "%SystemRoot%\System32\MyCredentialProvider.dll"

:: Remove the DLL
echo Removing credential provider DLL...
del /q "%SystemRoot%\System32\MyCredentialProvider.dll"

:: Remove the WebViewLauncher application
echo Removing WebViewLauncher application...
rmdir /s /q "C:\Program Files\WebViewLauncher"

echo.
echo Uninstallation complete!
echo.
pause 