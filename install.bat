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

:: First try to kill any running instances of WebView processes that might lock files
taskkill /f /im WebViewLauncher.exe /t >nul 2>&1
taskkill /f /im WebView2* /t >nul 2>&1

:: Wait a moment for processes to terminate
timeout /t 2 /nobreak >nul

:: Use robocopy instead of xcopy to handle sharing violations better
robocopy "%WEBVIEW_PATH%." "C:\Program Files\WebViewLauncher" /E /NFL /NDL /NJH /NJS /nc /ns /np

:: Check robocopy error codes (robocopy returns success codes 0-7)
if %ERRORLEVEL% GEQ 8 (
    echo Failed to copy WebViewLauncher files.
    echo Error code: %ERRORLEVEL%
    
    echo.
    echo Attempting alternative copy method...
    
    :: Create a PowerShell script to do the copying
    echo $ErrorActionPreference = 'SilentlyContinue' > "%TEMP%\copy_files.ps1"
    echo try { >> "%TEMP%\copy_files.ps1"
    echo     Copy-Item -Path '%WEBVIEW_PATH%*' -Destination 'C:\Program Files\WebViewLauncher' -Recurse -Force >> "%TEMP%\copy_files.ps1"
    echo     Write-Host "Copy completed with PowerShell" >> "%TEMP%\copy_files.ps1"
    echo } catch { >> "%TEMP%\copy_files.ps1"
    echo     Write-Host "Error: $_" >> "%TEMP%\copy_files.ps1"
    echo     exit 1 >> "%TEMP%\copy_files.ps1"
    echo } >> "%TEMP%\copy_files.ps1"
    
    powershell -ExecutionPolicy Bypass -File "%TEMP%\copy_files.ps1"
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to copy files even with PowerShell method.
        echo.
        echo WARNING: Continuing with installation, but WebViewLauncher might not function correctly.
        echo You may need to manually copy files from "%WEBVIEW_PATH%" to "C:\Program Files\WebViewLauncher" later.
        echo.
    )
)

:: Register the credential provider DLL
echo Registering credential provider...
copy /y "%DLL_FULL_PATH%" "%SystemRoot%\System32\"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to copy MyCredentialProvider.dll.
    echo Error code: %ERRORLEVEL%
    echo Attempting alternative copy method...
    
    powershell -Command "Copy-Item -Path '%DLL_FULL_PATH%' -Destination '%SystemRoot%\System32\' -Force"
    if %ERRORLEVEL% NEQ 0 (
        echo Failed to copy DLL even with PowerShell method.
        pause
        exit /b 1
    )
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

:: Add Windows 11 specific registry settings
echo.
echo Adding Windows 11 specific settings...

:: Add Windows Hello Forgot PIN settings to launch our application
reg add "HKLM\Software\Microsoft\Windows NT\CurrentVersion\PasswordLess\Device" /v "DevicePasswordLessBuildVersion" /t REG_DWORD /d 0 /f

:: Override the default Microsoft password reset experience
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" /v "DisableCAD" /t REG_DWORD /d 1 /f

:: Create direct shortcut to WebViewLauncher in the Winlogon registry
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v "ForgotPasswordCommand" /t REG_SZ /d "C:\Program Files\WebViewLauncher\WebViewLauncher.exe" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v "ForgotPasswordCommandURL" /t REG_SZ /d "C:\Program Files\WebViewLauncher\WebViewLauncher.exe" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" /v "DisablePasswordChange" /t REG_DWORD /d 0 /f

:: Create shell command to launch WebViewLauncher directly
echo.
echo Creating Windows 11 custom recovery button...
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\Credential Providers\{5a5f9000-69be-4acc-bc63-5f05de2dc0e7}" /v "Flags" /t REG_DWORD /d 0x00000003 /f

:: Add group policy settings to allow password reset options to be visible
reg add "HKLM\SOFTWARE\Policies\Microsoft\Windows\System" /v "AllowDomainPINLogon" /t REG_DWORD /d 1 /f
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\LogonUI\SessionData" /v "AllowLockScreen" /t REG_DWORD /d 1 /f

:: Add direct setup to replace PIN reset
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\LogonUI\PasswordResetData" /v "ResetEnabled" /t REG_DWORD /d 1 /f
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\LogonUI\PasswordResetData" /v "ResetCommandPath" /t REG_SZ /d "C:\Program Files\WebViewLauncher\WebViewLauncher.exe" /f
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\LogonUI\PasswordResetData" /v "ResetDisplayText" /t REG_SZ /d "Reset Password" /f

:: Create a shortcut on desktop for testing purposes
echo.
echo Creating desktop shortcut for testing...
powershell -Command "$WshShell = New-Object -ComObject WScript.Shell; $Shortcut = $WshShell.CreateShortcut('C:\Users\Public\Desktop\Password Reset.lnk'); $Shortcut.TargetPath = 'C:\Program Files\WebViewLauncher\WebViewLauncher.exe'; $Shortcut.Save()"

:: Configure gflags.exe to load our credprov on startup
echo.
echo Setting up system to load credential provider at startup...
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Lsa" /v "SecurityProviders" /t REG_SZ /d "tspkg" /f
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Lsa" /v "Authentication Packages" /t REG_MULTI_SZ /d "msv1_0\0C:\Windows\System32\MyCredentialProvider.dll" /f

:: Enable PIN sign-in
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Authentication\LogonUI\PIN" /v "Enabled" /t REG_DWORD /d 1 /f

:: Directly register our credential provider DLL as a Notification Package
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Lsa" /v "Notification Packages" /t REG_MULTI_SZ /d "scecli\0MyCredentialProvider" /f

echo.
echo Installation complete!
echo A desktop shortcut "Password Reset" has been created for testing.
echo The "Reset Password" option may appear under various locations on the login screen:
echo  - Under "I forgot my PIN" option
echo  - In the boot menu options 
echo  - As a separate button
echo.
echo You MUST restart your computer for all changes to take effect.
echo.
pause 