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

:: Hard-coded paths for the files
set WEBVIEW_FULL_PATH=D:\Projects\WebViewLauncher\WebViewLauncher\WebViewLauncher\bin\Release\net8.0-windows\WebViewLauncher.exe
set DLL_FULL_PATH=D:\Projects\WebViewLauncher\WebViewLauncher\x64\Debug\MyCredentialProvider.dll
set REG_FILE=register_credprov.reg

:: Extract directory from WebViewLauncher.exe path
for %%F in ("%WEBVIEW_FULL_PATH%") do set WEBVIEW_PATH=%%~dpF

:: Check if the paths exist
if not exist "%WEBVIEW_FULL_PATH%" (
    echo ERROR: WebViewLauncher.exe not found at "%WEBVIEW_FULL_PATH%"
    pause
    exit /b 1
)

if not exist "%DLL_FULL_PATH%" (
    echo ERROR: MyCredentialProvider.dll not found at "%DLL_FULL_PATH%"
    echo Looking for DLL in alternative locations...
    
    if exist "D:\Projects\WebViewLauncher\WebViewLauncher\x64\Release\MyCredentialProvider.dll" (
        set DLL_FULL_PATH=D:\Projects\WebViewLauncher\WebViewLauncher\x64\Release\MyCredentialProvider.dll
        echo Found at: %DLL_FULL_PATH%
    ) else if exist "C:\Users\Dev\Downloads\MyCredentialProvider.dll" (
        set DLL_FULL_PATH=C:\Users\Dev\Downloads\MyCredentialProvider.dll
        echo Found at: %DLL_FULL_PATH%
    ) else (
        echo Could not find MyCredentialProvider.dll in common locations.
        pause
        exit /b 1
    )
)

:: Look for registry file in multiple locations
if not exist "%REG_FILE%" (
    echo Registry file not found in current directory, checking alternatives...
    if exist "C:\register_credprov.reg" (
        set REG_FILE=C:\register_credprov.reg
        echo Found registry file at: %REG_FILE%
    ) else if exist "D:\Projects\WebViewLauncher\WebViewLauncher\register_credprov.reg" (
        set REG_FILE=D:\Projects\WebViewLauncher\WebViewLauncher\register_credprov.reg
        echo Found registry file at: %REG_FILE%
    ) else (
        echo ERROR: Cannot find register_credprov.reg in any location
        pause
        exit /b 1
    )
)

:: Copy WebViewLauncher files
echo Copying WebViewLauncher application...
xcopy /s /y "%WEBVIEW_PATH%*.*" "C:\Program Files\WebViewLauncher\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy WebViewLauncher files.
    echo Error code: %ERRORLEVEL%
    pause
    exit /b 1
)

:: Register the credential provider DLL
echo Registering credential provider...
copy /y "%DLL_FULL_PATH%" "%SystemRoot%\System32\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy MyCredentialProvider.dll.
    echo Error code: %ERRORLEVEL%
    pause
    exit /b 1
)

:: Skip regsvr32 and go directly to registry import
echo.
echo Using registry import for DLL registration...
echo.

:: Create temporary registry file with correct paths
echo Creating temporary registry file with correct paths...
set TEMP_REG_FILE=%TEMP%\temp_credprov.reg
type "%REG_FILE%" > "%TEMP_REG_FILE%"

:: Use PowerShell to replace the path with the correct one (with proper escaping)
powershell -Command "(Get-Content '%TEMP_REG_FILE%') -replace 'C:\\\\Windows\\\\System32\\\\MyCredentialProvider.dll', '%SystemRoot:\\=\\\\%\\\\System32\\\\MyCredentialProvider.dll' | Set-Content '%TEMP_REG_FILE%'"

echo Importing registry settings...
regedit /s "%TEMP_REG_FILE%"

if %ERRORLEVEL% NEQ 0 (
    echo Failed to import registry settings.
    echo Error code: %ERRORLEVEL%
) else (
    echo Manual registry import successful!
)

:: Clean up temporary file
del "%TEMP_REG_FILE%" 2>nul

echo.
echo Installation complete!
echo The "Reset Password" option should now appear on the Windows login screen.
echo You may need to restart your computer to see the changes.
echo.
pause 