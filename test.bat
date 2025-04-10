@echo off
echo Testing WebViewLauncher Password Reset Solution
echo ============================================

:: Check if WebViewLauncher.exe exists and is accessible
echo Checking WebViewLauncher application...
if not exist "C:\Program Files\WebViewLauncher\WebViewLauncher.exe" (
    echo ERROR: WebViewLauncher.exe not found at the expected location!
    echo Please ensure the application is properly installed.
) else (
    echo WebViewLauncher.exe found at the expected location.
)

:: Check if the DLL is registered
echo Checking credential provider registration...
reg query "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers" /s | findstr "12345678-1234-1234-1234-56789ABCDEF" > nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Credential provider not properly registered in Windows!
    echo Please run the install.bat script as administrator.
) else (
    echo Credential provider appears to be properly registered.
)

:: Check if the DLL exists in System32
echo Checking credential provider DLL...
if not exist "%SystemRoot%\System32\MyCredentialProvider.dll" (
    echo ERROR: MyCredentialProvider.dll not found in System32 directory!
    echo Please run the install.bat script as administrator.
) else (
    echo MyCredentialProvider.dll found in System32 directory.
)

:: Offer to launch the application for testing
echo.
echo Would you like to test launching the WebViewLauncher application? (Y/N)
choice /c YN /m "Launch WebViewLauncher for testing"
if %ERRORLEVEL%==1 (
    echo Launching WebViewLauncher...
    start "" "C:\Program Files\WebViewLauncher\WebViewLauncher.exe"
)

echo.
echo Test complete.
echo.
pause 