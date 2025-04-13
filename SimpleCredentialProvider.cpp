#include <windows.h>
#include <credentialprovider.h>
#include <strsafe.h>
#include <shlguid.h>
#include <propkey.h>

// Class ID for our credential provider
// Use a new GUID for your provider
// {12345678-1234-1234-1234-123456789ABC}
static const GUID CLSID_SimpleCredentialProvider = 
{ 0x12345678, 0x1234, 0x1234, { 0x12, 0x34, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc } };

// Field IDs
enum FIELD_ID
{
    FIELD_ID_LINK = 0,
    FIELD_ID_COUNT
};

// Forward declarations of classes
class SimpleCredential;
class SimpleCredentialProvider;

// SimpleCredential class implements the credential itself
class SimpleCredential : public ICredentialProviderCredential
{
public:
    // Constructor
    SimpleCredential() : _cRef(1), _pCredProvEvents(nullptr)
    {
        ZeroMemory(_rgCredentials, sizeof(_rgCredentials));
        ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
    }

    // Destructor
    ~SimpleCredential()
    {
        _CleanUpAllFields();
        if (_pCredProvEvents)
        {
            _pCredProvEvents->Release();
            _pCredProvEvents = nullptr;
        }
    }

    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (ppv)
        {
            if (IID_IUnknown == riid || IID_ICredentialProviderCredential == riid)
            {
                *ppv = static_cast<ICredentialProviderCredential*>(this);
                reinterpret_cast<IUnknown*>(*ppv)->AddRef();
                hr = S_OK;
            }
            else
            {
                *ppv = nullptr;
                hr = E_NOINTERFACE;
            }
        }
        else
        {
            hr = E_POINTER;
        }
        return hr;
    }

    // ICredentialProviderCredential methods
    STDMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce)
    {
        if (_pCredProvEvents)
        {
            _pCredProvEvents->Release();
        }

        _pCredProvEvents = pcpce;
        if (_pCredProvEvents)
        {
            _pCredProvEvents->AddRef();
        }
        return S_OK;
    }

    STDMETHODIMP UnAdvise()
    {
        if (_pCredProvEvents)
        {
            _pCredProvEvents->Release();
            _pCredProvEvents = nullptr;
        }
        return S_OK;
    }

    STDMETHODIMP SetSelected(BOOL* pbAutoLogon)
    {
        *pbAutoLogon = FALSE;
        return S_OK;
    }

    STDMETHODIMP SetDeselected()
    {
        return S_OK;
    }

    STDMETHODIMP GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
    {
        *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
        *pcpfis = CPFIS_NONE;
        return S_OK;
    }

    STDMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz)
    {
        HRESULT hr;
        
        if (dwFieldID == FIELD_ID_LINK)
        {
            hr = SHStrDupW(L"Reset Password", ppwsz);
        }
        else
        {
            hr = E_INVALIDARG;
        }
        
        return hr;
    }

    STDMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP CommandLinkClicked(DWORD dwFieldID)
    {
        HRESULT hr = E_INVALIDARG;
        if (dwFieldID == FIELD_ID_LINK)
        {
            // Launch the WebViewLauncher application
            ShellExecute(NULL, L"open", L"C:\\Program Files\\WebViewLauncher\\WebViewLauncher.exe", NULL, NULL, SW_SHOW);
            hr = S_OK;
        }
        return hr;
    }

    STDMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
    {
        *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
        return S_OK;
    }

    STDMETHODIMP ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
    {
        return E_NOTIMPL;
    }

private:
    LONG _cRef;
    PWSTR _rgFieldStrings[FIELD_ID_COUNT];
    PWSTR _rgCredentials[FIELD_ID_COUNT];
    ICredentialProviderCredentialEvents* _pCredProvEvents;

    // Clean up string fields
    void _CleanUpAllFields()
    {
        for (int i = 0; i < FIELD_ID_COUNT; i++)
        {
            CoTaskMemFree(_rgFieldStrings[i]);
            CoTaskMemFree(_rgCredentials[i]);
            _rgFieldStrings[i] = nullptr;
            _rgCredentials[i] = nullptr;
        }
    }
};

// SimpleCredentialProvider class implements the credential provider itself
class SimpleCredentialProvider : public ICredentialProvider
{
public:
    // Constructor
    SimpleCredentialProvider() : _cRef(1)
    {
        ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
        _Initialize();
    }

    // Destructor
    ~SimpleCredentialProvider()
    {
        _CleanUpAllFields();
    }

    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (ppv)
        {
            if (IID_IUnknown == riid || IID_ICredentialProvider == riid)
            {
                *ppv = static_cast<ICredentialProvider*>(this);
                reinterpret_cast<IUnknown*>(*ppv)->AddRef();
                hr = S_OK;
            }
            else
            {
                *ppv = nullptr;
                hr = E_NOINTERFACE;
            }
        }
        else
        {
            hr = E_POINTER;
        }
        return hr;
    }

    // ICredentialProvider methods
    STDMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
    {
        HRESULT hr;
        // This provider only works with logon and credential change
        if (cpus == CPUS_LOGON || cpus == CPUS_UNLOCK_WORKSTATION)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_INVALIDARG;
        }
        return hr;
    }

    STDMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
    {
        return E_NOTIMPL;
    }

    STDMETHODIMP Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
    {
        return S_OK;
    }

    STDMETHODIMP UnAdvise()
    {
        return S_OK;
    }

    STDMETHODIMP GetFieldDescriptorCount(DWORD* pdwCount)
    {
        *pdwCount = FIELD_ID_COUNT;
        return S_OK;
    }

    STDMETHODIMP GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
    {
        HRESULT hr;
        if (dwIndex < FIELD_ID_COUNT)
        {
            hr = FieldDescriptorCopy(_rgFieldDescriptors[dwIndex], ppcpfd);
        }
        else
        {
            hr = E_INVALIDARG;
        }
        return hr;
    }

    STDMETHODIMP GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
    {
        *pdwCount = 1;          // We always have just one credential
        *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
        *pbAutoLogonWithDefault = FALSE;
        return S_OK;
    }

    STDMETHODIMP GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
    {
        HRESULT hr = E_INVALIDARG;
        if ((dwIndex == 0) && ppcpc)
        {
            SimpleCredential* pSimpleCred = new SimpleCredential();
            if (pSimpleCred)
            {
                hr = pSimpleCred->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
                pSimpleCred->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        return hr;
    }

private:
    LONG _cRef;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgFieldDescriptors[FIELD_ID_COUNT];

    // Initialize field descriptors
    void _Initialize()
    {
        // Reset Password link
        _rgFieldDescriptors[FIELD_ID_LINK].dwFieldID = FIELD_ID_LINK;
        _rgFieldDescriptors[FIELD_ID_LINK].cpft = CPFT_COMMAND_LINK;
        _rgFieldDescriptors[FIELD_ID_LINK].pszLabel = L"Reset Password";
    }

    // Clean up fields
    void _CleanUpAllFields()
    {
        for (int i = 0; i < FIELD_ID_COUNT; i++)
        {
            if (_rgFieldDescriptors[i].pszLabel)
            {
                CoTaskMemFree((LPVOID)_rgFieldDescriptors[i].pszLabel);
                _rgFieldDescriptors[i].pszLabel = nullptr;
            }
        }
    }

    // Helper to copy field descriptors
    HRESULT FieldDescriptorCopy(const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR& rcpfd, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
    {
        HRESULT hr;
        CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* pcpfd = (CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(sizeof(CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR));
        if (pcpfd)
        {
            pcpfd->dwFieldID = rcpfd.dwFieldID;
            pcpfd->cpft = rcpfd.cpft;
            pcpfd->guidFieldType = rcpfd.guidFieldType;
            
            if (rcpfd.pszLabel)
            {
                hr = SHStrDupW(rcpfd.pszLabel, (PWSTR*)&pcpfd->pszLabel);
            }
            else
            {
                pcpfd->pszLabel = nullptr;
                hr = S_OK;
            }
            
            if (SUCCEEDED(hr))
            {
                *ppcpfd = pcpfd;
            }
            else
            {
                CoTaskMemFree(pcpfd);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        return hr;
    }
};

// Factory class for creating credential provider instances
class SimpleCredentialProviderFactory : public IClassFactory
{
public:
    // Constructor
    SimpleCredentialProviderFactory() : _cRef(1)
    {
    }

    // Destructor
    ~SimpleCredentialProviderFactory()
    {
    }

    // IUnknown methods
    STDMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    STDMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (ppv)
        {
            if (IID_IUnknown == riid || IID_IClassFactory == riid)
            {
                *ppv = static_cast<IClassFactory*>(this);
                reinterpret_cast<IUnknown*>(*ppv)->AddRef();
                hr = S_OK;
            }
            else
            {
                *ppv = nullptr;
                hr = E_NOINTERFACE;
            }
        }
        else
        {
            hr = E_POINTER;
        }
        return hr;
    }

    // IClassFactory methods
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (!pUnkOuter)
        {
            SimpleCredentialProvider* pProvider = new SimpleCredentialProvider();
            if (pProvider)
            {
                hr = pProvider->QueryInterface(riid, ppv);
                pProvider->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            hr = CLASS_E_NOAGGREGATION;
        }
        return hr;
    }

    STDMETHODIMP LockServer(BOOL bLock)
    {
        if (bLock)
        {
            InterlockedIncrement(&g_cRef);
        }
        else
        {
            InterlockedDecrement(&g_cRef);
        }
        return S_OK;
    }

private:
    LONG _cRef;
};

// Global variables
HINSTANCE g_hDll;
LONG g_cRef;

// DLL entry point
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hDll = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

// Standard DLL functions for COM
STDAPI DllCanUnloadNow()
{
    return (g_cRef > 0) ? S_FALSE : S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppv)
{
    HRESULT hr;
    if (CLSID_SimpleCredentialProvider == rclsid)
    {
        SimpleCredentialProviderFactory* pFactory = new SimpleCredentialProviderFactory();
        if (pFactory)
        {
            hr = pFactory->QueryInterface(riid, ppv);
            pFactory->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hr;
}

// Converts a GUID to a string
HRESULT GuidToString(const GUID& guid, wchar_t* guidStr, size_t strSize)
{
    int res = StringCbPrintfW(
        guidStr, 
        strSize, 
        L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1],
        guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5],
        guid.Data4[6], guid.Data4[7]
    );
    
    return (res == S_OK) ? S_OK : E_FAIL;
}

// Register the DLL
STDAPI DllRegisterServer()
{
    HRESULT hr;
    wchar_t guidString[40] = { 0 };
    
    hr = GuidToString(CLSID_SimpleCredentialProvider, guidString, sizeof(guidString));
    
    if (SUCCEEDED(hr))
    {
        wchar_t modulePath[MAX_PATH] = { 0 };
        DWORD pathLen = GetModuleFileNameW(g_hDll, modulePath, ARRAYSIZE(modulePath));
        
        if (pathLen != 0 && pathLen < ARRAYSIZE(modulePath))
        {
            // Create registry keys
            HKEY hKey = NULL;
            HKEY hSubkey = NULL;
            LPCWSTR credProvRegistryPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers";
            
            // Create the credential provider key
            if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, credProvRegistryPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
            {
                if (RegCreateKeyExW(hKey, guidString, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL) == ERROR_SUCCESS)
                {
                    // Set the default value to the friendly name
                    LPCWSTR friendlyName = L"Reset Password Provider";
                    RegSetValueExW(hSubkey, NULL, 0, REG_SZ, (BYTE*)friendlyName, (wcslen(friendlyName) + 1) * sizeof(wchar_t));
                    RegCloseKey(hSubkey);
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
                RegCloseKey(hKey);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            
            // Register COM object for current user
            if (SUCCEEDED(hr))
            {
                // Create entries in HKCR
                wchar_t keyPath[128] = { 0 };
                StringCbPrintfW(keyPath, sizeof(keyPath), L"CLSID\\%s", guidString);
                
                if (RegCreateKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL) == ERROR_SUCCESS)
                {
                    // Set default value to friendly name
                    LPCWSTR friendlyName = L"Reset Password Provider";
                    RegSetValueExW(hKey, NULL, 0, REG_SZ, (BYTE*)friendlyName, (wcslen(friendlyName) + 1) * sizeof(wchar_t));
                    
                    // Create the InProcServer32 key
                    if (RegCreateKeyExW(hKey, L"InprocServer32", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubkey, NULL) == ERROR_SUCCESS)
                    {
                        // Set the default value to the path of the DLL
                        RegSetValueExW(hSubkey, NULL, 0, REG_SZ, (BYTE*)modulePath, (wcslen(modulePath) + 1) * sizeof(wchar_t));
                        
                        // Set the threading model
                        LPCWSTR threadingModel = L"Apartment";
                        RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (BYTE*)threadingModel, (wcslen(threadingModel) + 1) * sizeof(wchar_t));
                        
                        RegCloseKey(hSubkey);
                        hr = S_OK;
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    RegCloseKey(hKey);
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                }
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    
    return hr;
}

// Unregister the DLL
STDAPI DllUnregisterServer()
{
    HRESULT hr;
    wchar_t guidString[40] = { 0 };
    
    hr = GuidToString(CLSID_SimpleCredentialProvider, guidString, sizeof(guidString));
    
    if (SUCCEEDED(hr))
    {
        wchar_t keyPath[128] = { 0 };
        
        // Remove HKLM credential provider registration
        LPCWSTR credProvRegistryPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Authentication\\Credential Providers";
        StringCbPrintfW(keyPath, sizeof(keyPath), L"%s\\%s", credProvRegistryPath, guidString);
        RegDeleteKeyW(HKEY_LOCAL_MACHINE, keyPath);
        
        // Remove HKCR InprocServer32 key
        StringCbPrintfW(keyPath, sizeof(keyPath), L"CLSID\\%s\\InprocServer32", guidString);
        RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath);
        
        // Remove CLSID key
        StringCbPrintfW(keyPath, sizeof(keyPath), L"CLSID\\%s", guidString);
        RegDeleteKeyW(HKEY_CLASSES_ROOT, keyPath);
        
        hr = S_OK;
    }
    
    return hr;
} 