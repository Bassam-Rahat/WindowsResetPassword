@echo off
echo Installing WebViewLauncher Password Reset Solution
echo ================================================

:: Check for administrator privileges
NET SESSION >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    echo Error: Administrator privileges required!
    echo Please run this script as Administrator.
    pause
    exit /b 1
)

:: Create installation directory
echo Creating installation directories...
if not exist "C:\Program Files\WebViewLauncher" mkdir "C:\Program Files\WebViewLauncher"

:: Copy WebViewLauncher files
echo Copying WebViewLauncher application...
xcopy /s /y "WebViewLauncher\bin\Release\net8.0-windows\*.*" "C:\Program Files\WebViewLauncher\"

:: Register the credential provider DLL
echo Registering credential provider...
copy /y "x64\Release\MyCredentialProvider.dll" "%SystemRoot%\System32\"
regsvr32 /s "%SystemRoot%\System32\MyCredentialProvider.dll"

echo.
echo Installation complete!
echo The "Reset Password" option should now appear on the Windows login screen.
echo.
pause 