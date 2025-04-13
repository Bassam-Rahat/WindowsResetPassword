#pragma once

#include <Windows.h>
#include <credentialprovider.h>
#include <string>

// Add the enum definition for field IDs
enum SAMPLE_FIELD_ID
{
    SFI_RESET_PASSWORD_LINK = 0,
    SFI_NUM_FIELDS
};

class MyCredentialProvider : public ICredentialProvider
{
public:
    // Constructor and Destructor
    MyCredentialProvider();
    virtual ~MyCredentialProvider();

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ICredentialProvider methods
    STDMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    STDMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);
    STDMETHODIMP Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    STDMETHODIMP UnAdvise();
    STDMETHODIMP GetFieldDescriptorCount(DWORD* pdwCount);
    STDMETHODIMP GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);
    STDMETHODIMP GetCredentialCount(DWORD* pdwCount, DWORD* pdwDefault, BOOL* pbAutoLogonWithDefault);
    STDMETHODIMP GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc);
    // Add CommandLinkClicked method to match the implementation
    STDMETHODIMP CommandLinkClicked(DWORD dwFieldID);

private:
    LONG _cRef;
};
