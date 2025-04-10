# Windows Password Reset Solution

This project provides a Windows credential provider that adds a "Reset Password" option to the Windows login screen. When clicked, it launches a WebView2-based application that navigates to a password reset website.

## Components

The solution consists of three main components:

1. **WebViewLauncher** - A .NET Windows Forms application using WebView2 to display a password reset webpage
2. **MyCredentialProvider** - A Windows credential provider DLL that integrates with the Windows login system
3. **ActonixWindowApplication** - A native C++ application that can be used to launch the WebViewLauncher separately

## Installation

1. Build the solution in Release mode.
2. Run the `install.bat` script **as administrator**.
3. This will:
   - Copy the WebViewLauncher application to `C:\Program Files\WebViewLauncher\`
   - Register the credential provider DLL in the Windows system
   - Set up the necessary registry entries

## Uninstallation

1. Run the `uninstall.bat` script **as administrator**.
2. This will remove the application files and DLL, and unregister the credential provider.

## Troubleshooting

If the password reset option doesn't appear on the login screen, run the `test.bat` script to diagnose the issue.

Common issues:
- Missing WebView2 Runtime: Make sure the WebView2 Runtime is installed on the target system
- DLL registration failure: Ensure you ran the installation script as administrator
- Path issues: The application should be installed to the default location

## Requirements

- Windows 10 or 11
- .NET 8.0 Runtime
- WebView2 Runtime

## Testing

Before deploying to a production environment, test the solution on a non-critical machine. The credential provider integration affects the Windows login experience, so proper testing is essential.

## Customization

To change the URL of the password reset website, modify the `CoreWebView2.Navigate` line in the `BrowserForm.cs` file in the WebViewLauncher project before building. 