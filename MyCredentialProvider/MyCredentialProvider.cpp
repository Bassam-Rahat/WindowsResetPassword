#include "pch.h" // Add this line at the very top
#include "MyCredentialProvider.h"
#include <shellapi.h>
#include <iostream>

MyCredentialProvider::MyCredentialProvider() : _cRef(1)
{
}

MyCredentialProvider::~MyCredentialProvider()
{
}

HRESULT MyCredentialProvider::SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags)
{
    // We are making this Credential Provider available for logon and unlock scenarios
    if (cpus == CPUS_LOGON || cpus == CPUS_UNLOCK_WORKSTATION)
    {
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
    *pdwCount = 1; // One tile
    return S_OK;
}

HRESULT MyCredentialProvider::GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd)
{
    return E_NOTIMPL;
}

HRESULT MyCredentialProvider::GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault)
{
    *pdwCount = 1;  // Only one credential (your tile)
    *pdwDefault = 0;
    *pbAutoLogonWithDefault = FALSE;
    return S_OK;
}

HRESULT MyCredentialProvider::GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc)
{
    if (dwIndex != 0)
        return E_INVALIDARG;

    // Return a credential placeholder but don't launch the app yet
    // The app should launch when the user clicks on the reset password option
    *ppcpc = NULL;
    
    // We'll launch the app when CommandLinkClicked is called instead
    return S_OK;
}

HRESULT MyCredentialProvider::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_ICredentialProvider)
    {
        *ppv = static_cast<ICredentialProvider*>(this);
        AddRef();
        return S_OK;
    }

    *ppv = NULL;
    return E_NOINTERFACE;
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

HRESULT MyCredentialProvider::CommandLinkClicked(DWORD dwFieldID)
{
    if (dwFieldID == SFI_RESET_PASSWORD_LINK)
    {
        // Launch the password reset application
        // Use the exact path where we installed the application
        ShellExecute(NULL, L"open", L"C:\\Program Files\\WebViewLauncher\\WebViewLauncher.exe", NULL, NULL, SW_SHOWNORMAL);
        return S_OK;
    }
    return E_INVALIDARG;
}
