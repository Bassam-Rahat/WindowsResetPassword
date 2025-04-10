#include "pch.h" // Add this line at the very top
#include "MyCredentialProviderFactory.h"
#include "MyCredentialProvider.h"

MyCredentialProviderFactory::MyCredentialProviderFactory() : _cRef(1) {}
MyCredentialProviderFactory::~MyCredentialProviderFactory() {}

HRESULT MyCredentialProviderFactory::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IClassFactory)
    {
        *ppv = this;
        AddRef();
        return S_OK;
    }
    *ppv = NULL;
    return E_NOINTERFACE;
}

ULONG MyCredentialProviderFactory::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG MyCredentialProviderFactory::Release()
{
    LONG cRef = InterlockedDecrement(&_cRef);
    if (!cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT MyCredentialProviderFactory::CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv)
{
    if (pUnkOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    MyCredentialProvider* pProvider = new MyCredentialProvider();
    if (!pProvider)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pProvider->QueryInterface(riid, ppv);
    pProvider->Release();
    return hr;
}

HRESULT MyCredentialProviderFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        AddRef();
    }
    else
    {
        Release();
    }
    return S_OK;
}
