#include "pch.h" // Add this line at the very top
#include <Windows.h>
#include <Shlobj.h>
#include <strsafe.h>
#include "MyCredentialProvider.h"
#include "MyCredentialProviderFactory.h"

// Define a GUID for the credential provider
// {5a5f9000-69be-4acc-bc63-5f05de2dc0e7}
GUID CLSID_MyCredentialProvider = { 0x5a5f9000, 0x69be, 0x4acc, { 0xbc, 0x63, 0x5f, 0x05, 0xde, 0x2d, 0xc0, 0xe7 } };

HINSTANCE g_hInst = NULL;
LONG g_cRef = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Function to convert a GUID to a string
HRESULT GuidToString(const GUID& guid, LPWSTR guidStr, size_t guidStrSize)
{
    int res = StringCbPrintfW(
        guidStr, 
        guidStrSize,
        L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]
    );
    
    return (res == S_OK) ? S_OK : E_FAIL;
}

// Required COM export functions
STDAPI DllCanUnloadNow()
{
    return (g_cRef > 0) ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_POINTER;
    }
    
    *ppv = nullptr;
    
    if (rclsid != CLSID_MyCredentialProvider)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    MyCredentialProviderFactory* pFactory = new MyCredentialProviderFactory();
    if (pFactory == nullptr)
    {
        return E_OUTOFMEMORY;
    }
    
    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllRegisterServer()
{
    HRESULT hr = S_OK;
    WCHAR guidString[64] = {0};
    
    // Convert the GUID to a string
    hr = GuidToString(CLSID_MyCredentialProvider, guidString, sizeof(guidString));
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Get the path to the DLL
    WCHAR dllPath[MAX_PATH] = {0};
    if (GetModuleFileName(g_hInst, dllPath, ARRAYSIZE(dllPath)) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    // Register the credential provider
    WCHAR registryPath[256] = {0};
    StringCbPrintfW(registryPath, sizeof(registryPath), 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\%s", 
        guidString);
    
    HKEY hKey = NULL;
    DWORD disposition = 0;
    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, registryPath, 0, NULL, 0, 
                      KEY_WRITE, NULL, &hKey, &disposition) != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    // Set the default value
    LPCWSTR name = L"WebView Password Reset Provider";
    DWORD size = (DWORD)((wcslen(name) + 1) * sizeof(WCHAR));
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)name, size);
    RegCloseKey(hKey);
    
    // Register the COM object
    StringCbPrintfW(registryPath, sizeof(registryPath), L"CLSID\\%s", guidString);
    if (RegCreateKeyEx(HKEY_CLASSES_ROOT, registryPath, 0, NULL, 0, 
                      KEY_WRITE, NULL, &hKey, &disposition) != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    // Set description
    RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)name, size);
    
    // Create the InprocServer32 key
    HKEY hSubKey = NULL;
    if (RegCreateKeyEx(hKey, L"InprocServer32", 0, NULL, 0, 
                      KEY_WRITE, NULL, &hSubKey, &disposition) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    // Set the server path
    size = (DWORD)((wcslen(dllPath) + 1) * sizeof(WCHAR));
    RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (BYTE*)dllPath, size);
    
    // Set the threading model
    LPCWSTR threadingModel = L"Apartment";
    size = (DWORD)((wcslen(threadingModel) + 1) * sizeof(WCHAR));
    RegSetValueEx(hSubKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)threadingModel, size);
    
    RegCloseKey(hSubKey);
    RegCloseKey(hKey);
    
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    WCHAR guidString[64] = {0};
    
    // Convert the GUID to a string
    HRESULT hr = GuidToString(CLSID_MyCredentialProvider, guidString, sizeof(guidString));
    if (FAILED(hr))
    {
        return hr;
    }
    
    // Unregister the credential provider
    WCHAR registryPath[256] = {0};
    StringCbPrintfW(registryPath, sizeof(registryPath), 
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers\\%s", 
        guidString);
    
    RegDeleteKey(HKEY_LOCAL_MACHINE, registryPath);
    
    // Unregister the COM object
    StringCbPrintfW(registryPath, sizeof(registryPath), L"CLSID\\%s\\InprocServer32", guidString);
    RegDeleteKey(HKEY_CLASSES_ROOT, registryPath);
    
    StringCbPrintfW(registryPath, sizeof(registryPath), L"CLSID\\%s", guidString);
    RegDeleteKey(HKEY_CLASSES_ROOT, registryPath);
    
    return S_OK;
}
