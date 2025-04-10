#pragma once

#include <Windows.h>
#include <credentialprovider.h>
#include <string>

// Field indexes for our credential provider UI
enum SAMPLE_FIELD_ID
{
    SFI_RESET_PASSWORD_LINK = 0,
    SFI_NUM_FIELDS
};

// Define a credential class to handle the actual credential
class MyCredential : public ICredentialProviderCredential
{
public:
    MyCredential();
    virtual ~MyCredential();

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ICredentialProviderCredential methods
    STDMETHODIMP Advise(ICredentialProviderCredentialEvents* pcpce);
    STDMETHODIMP UnAdvise();
    STDMETHODIMP SetSelected(BOOL* pbAutoLogon);
    STDMETHODIMP SetDeselected();
    STDMETHODIMP GetFieldState(DWORD dwFieldID, CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);
    STDMETHODIMP GetStringValue(DWORD dwFieldID, PWSTR* ppwsz);
    STDMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP* phbmp);
    STDMETHODIMP GetCheckboxValue(DWORD dwFieldID, BOOL* pbChecked, PWSTR* ppwszLabel);
    STDMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, DWORD* pdwAdjacentTo);
    STDMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, DWORD* pcItems, DWORD* pdwSelectedItem);
    STDMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, PWSTR* ppwszItem);
    STDMETHODIMP SetStringValue(DWORD dwFieldID, PCWSTR pwz);
    STDMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
    STDMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
    STDMETHODIMP CommandLinkClicked(DWORD dwFieldID);
    STDMETHODIMP GetSerialization(CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr, CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    STDMETHODIMP ReportResult(NTSTATUS ntsStatus, NTSTATUS ntsSubstatus, PWSTR* ppwszOptionalStatusText, CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

private:
    LONG _cRef;
    ICredentialProviderCredentialEvents* _pCredProvCredentialEvents;
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

private:
    LONG _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus;
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgFieldDescriptors[SFI_NUM_FIELDS];
    
    void _CleanupFieldDescriptors();
};

// External for COM registration and class factory
extern HINSTANCE g_hInst;
extern GUID CLSID_MyCredentialProvider;
