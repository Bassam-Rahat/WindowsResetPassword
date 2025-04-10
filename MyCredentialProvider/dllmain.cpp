#include "pch.h" // Add this line at the very top
#include <Windows.h>
#include "MyCredentialProvider.h"
#include "MyCredentialProviderFactory.h"

HINSTANCE g_hInst = NULL;
static LONG g_cRef = 0;
GUID CLSID_MyCredentialProvider = { /* create a unique GUID */ 0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF} };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// COM exports for registration
STDAPI DllCanUnloadNow()
{
    return (g_cRef == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    *ppv = NULL;

    if (rclsid == CLSID_MyCredentialProvider)
    {
        MyCredentialProviderFactory* pFactory = new MyCredentialProviderFactory();
        if (pFactory)
        {
            HRESULT hr = pFactory->QueryInterface(riid, ppv);
            pFactory->Release();
            return hr;
        }
        return E_OUTOFMEMORY;
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

// Registry key paths
#define CREDENTIAL_PROVIDER_REGKEY L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers"

STDAPI DllRegisterServer()
{
    HKEY hKey = NULL;
    HKEY hSubKey = NULL;
    WCHAR wszGUID[64];
    LPOLESTR lpOleStr = NULL;
    HRESULT hr = StringFromCLSID(CLSID_MyCredentialProvider, &lpOleStr);
    if (SUCCEEDED(hr))
    {
        wcscpy_s(wszGUID, 64, lpOleStr);
        CoTaskMemFree(lpOleStr);

        // Create registry key for provider
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, CREDENTIAL_PROVIDER_REGKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            if (RegCreateKeyEx(hKey, wszGUID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL) == ERROR_SUCCESS)
            {
                DWORD dwValue = 0;
                RegSetValueEx(hSubKey, L"Disabled", 0, REG_DWORD, (BYTE*)&dwValue, sizeof(dwValue));
                RegCloseKey(hSubKey);
            }
            RegCloseKey(hKey);
        }

        // Register COM object
        WCHAR wszCLSID[128];
        swprintf_s(wszCLSID, 128, L"CLSID\\%s", wszGUID);
        if (RegCreateKeyEx(HKEY_CLASSES_ROOT, wszCLSID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
        {
            RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)L"Reset Password Provider", sizeof(L"Reset Password Provider"));
            RegCloseKey(hKey);

            swprintf_s(wszCLSID, 128, L"CLSID\\%s\\InprocServer32", wszGUID);
            if (RegCreateKeyEx(HKEY_CLASSES_ROOT, wszCLSID, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
            {
                WCHAR wszDllPath[MAX_PATH];
                GetModuleFileName(g_hInst, wszDllPath, MAX_PATH);
                RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)wszDllPath, (DWORD)(wcslen(wszDllPath) + 1) * sizeof(WCHAR));
                RegSetValueEx(hKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)L"Apartment", sizeof(L"Apartment"));
                RegCloseKey(hKey);
                return S_OK;
            }
        }
    }
    return E_FAIL;
}

STDAPI DllUnregisterServer()
{
    WCHAR wszGUID[64];
    LPOLESTR lpOleStr = NULL;
    HRESULT hr = StringFromCLSID(CLSID_MyCredentialProvider, &lpOleStr);
    if (SUCCEEDED(hr))
    {
        wcscpy_s(wszGUID, 64, lpOleStr);
        CoTaskMemFree(lpOleStr);

        // Delete credential provider registry key
        WCHAR wszKey[256];
        swprintf_s(wszKey, 256, L"%s\\%s", CREDENTIAL_PROVIDER_REGKEY, wszGUID);
        RegDeleteKey(HKEY_LOCAL_MACHINE, wszKey);
        
        // Delete COM registration
        swprintf_s(wszKey, 256, L"CLSID\\%s\\InprocServer32", wszGUID);
        RegDeleteKey(HKEY_CLASSES_ROOT, wszKey);
        swprintf_s(wszKey, 256, L"CLSID\\%s", wszGUID);
        RegDeleteKey(HKEY_CLASSES_ROOT, wszKey);
        
        return S_OK;
    }
    return E_FAIL;
}
