#include "pch.h" // Add this line at the very top
#include "MyCredentialProvider.h"
#include <shellapi.h>
#include <iostream>
#include <new>

// MyCredential implementation
MyCredential::MyCredential() : _cRef(1), _pCredProvCredentialEvents(nullptr)
{
}

MyCredential::~MyCredential()
{
    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
        _pCredProvCredentialEvents = nullptr;
    }
}

HRESULT MyCredential::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_POINTER;
    }

    *ppv = nullptr;
    
    if (riid == IID_IUnknown || riid == IID_ICredentialProviderCredential)
    {
        *ppv = static_cast<ICredentialProviderCredential*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG MyCredential::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG MyCredential::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT MyCredential::Advise(ICredentialProviderCredentialEvents* pcpce)
{
    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
    }

    _pCredProvCredentialEvents = pcpce;

    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->AddRef();
    }

    return S_OK;
}

HRESULT MyCredential::UnAdvise()
{
    if (_pCredProvCredentialEvents)
    {
        _pCredProvCredentialEvents->Release();
        _pCredProvCredentialEvents = nullptr;
    }
    return S_OK;
}

HRESULT MyCredential::SetSelected(BOOL* pbAutoLogon)
{
    if (pbAutoLogon)
    {
        *pbAutoLogon = FALSE;
    }
    return S_OK;
}

HRESULT MyCredential::SetDeselected()
{
    return S_OK;
}

HRESULT MyCredential::GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis)
{
    if (pcpfs == nullptr || pcpfis == nullptr)
    {
        return E_POINTER;
    }

    if (dwFieldID >= SFI_NUM_FIELDS)
    {
        return E_INVALIDARG;
    }

    *pcpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
    *pcpfis = CPFIS_ENABLED;

    return S_OK;
}

HRESULT MyCredential::GetStringValue(DWORD dwFieldID, PWSTR* ppwsz)
{
    if (ppwsz == nullptr)
    {
        return E_POINTER;
    }

    if (dwFieldID >= SFI_NUM_FIELDS)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_NOTIMPL;
    if (dwFieldID == SFI_RESET_PASSWORD_LINK)
    {
        hr = SHStrDupW(L"Reset Password", ppwsz);
    }

    return hr;
}

HRESULT MyCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::SetStringValue(DWORD dwFieldID, PCWSTR pwz)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
    return E_NOTIMPL;
}

HRESULT MyCredential::CommandLinkClicked(DWORD dwFieldID)
{
    if (dwFieldID == SFI_RESET_PASSWORD_LINK)
    {
        // Launch the password reset application
        WCHAR szPath[MAX_PATH];
        // Use the correct path to the WebViewLauncher exe
        wcscpy_s(szPath, MAX_PATH, L"C:\\Program Files\\WebViewLauncher\\WebViewLauncher.exe");
        ShellExecute(NULL, L"open", szPath, NULL, NULL, SW_SHOWNORMAL);
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT MyCredential::GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    if (pcpgsr == nullptr || pcpcs == nullptr)
    {
        return E_POINTER;
    }

    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    return S_OK;
}

HRESULT MyCredential::ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon)
{
    return E_NOTIMPL;
}

// MyCredentialProvider implementation
MyCredentialProvider::MyCredentialProvider() : _cRef(1), _cpus(CPUS_INVALID)
{
    // Initialize field descriptors
    ZeroMemory(_rgFieldDescriptors, sizeof(_rgFieldDescriptors));
}

MyCredentialProvider::~MyCredentialProvider()
{
    _CleanupFieldDescriptors();
}

void MyCredentialProvider::_CleanupFieldDescriptors()
{
    for (DWORD i = 0; i < SFI_NUM_FIELDS; i++)
    {
        if (_rgFieldDescriptors[i].pszLabel)
        {
            CoTaskMemFree((LPVOID)_rgFieldDescriptors[i].pszLabel);
            _rgFieldDescriptors[i].pszLabel = nullptr;
        }
    }
}

HRESULT MyCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    _cpus = cpus;

    // We are making this Credential Provider available for logon and unlock scenarios
    if (cpus == CPUS_LOGON || cpus == CPUS_UNLOCK_WORKSTATION)
    {
        // Set up the field descriptors for our credential UI
        _rgFieldDescriptors[SFI_RESET_PASSWORD_LINK].dwFieldID = SFI_RESET_PASSWORD_LINK;
        _rgFieldDescriptors[SFI_RESET_PASSWORD_LINK].cpft = CPFT_COMMAND_LINK;
        _rgFieldDescriptors[SFI_RESET_PASSWORD_LINK].pszLabel = const_cast<LPWSTR>(L"Reset Password");

        return S_OK;
    }
    return E_NOTIMPL;
}

HRESULT MyCredentialProvider::SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs)
{
    return E_NOTIMPL;
}

HRESULT MyCredentialProvider::Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext)
{
    return S_OK;
}

HRESULT MyCredentialProvider::UnAdvise()
{
    return S_OK;
}

HRESULT MyCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount)
{
    if (pdwCount == nullptr)
    {
        return E_POINTER;
    }

    *pdwCount = SFI_NUM_FIELDS;
    return S_OK;
}

HRESULT MyCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    if (ppcpfd == nullptr)
    {
        return E_POINTER;
    }

    if (dwIndex >= SFI_NUM_FIELDS)
    {
        return E_INVALIDARG;
    }

    *ppcpfd = &_rgFieldDescriptors[dwIndex];
    return S_OK;
}

HRESULT MyCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    if (pdwCount == nullptr || pdwDefault == nullptr || pbAutoLogonWithDefault == nullptr)
    {
        return E_POINTER;
    }

    *pdwCount = 1;  // Only one credential (your tile)
    *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

HRESULT MyCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    if (ppcpc == nullptr)
    {
        return E_POINTER;
    }

    if (dwIndex >= 1)
    {
        return E_INVALIDARG;
    }

    // Create a new credential
    MyCredential* pMyCredential = new MyCredential();
    if (pMyCredential == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    // Get the interface pointer to return
    HRESULT hr = pMyCredential->QueryInterface(IID_ICredentialProviderCredential, reinterpret_cast<void**>(ppcpc));
    pMyCredential->Release();
    return hr;
}

HRESULT MyCredentialProvider::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == nullptr)
    {
        return E_POINTER;
    }

    *ppv = nullptr;

    if (riid == IID_IUnknown || riid == IID_ICredentialProvider)
    {
        *ppv = static_cast<ICredentialProvider*>(this);
    }
    else
    {
        return E_NOINTERFACE;
    }

    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG MyCredentialProvider::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG MyCredentialProvider::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}
