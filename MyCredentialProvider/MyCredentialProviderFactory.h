#pragma once

#include <unknwn.h>  // For IClassFactory

class MyCredentialProviderFactory : public IClassFactory
{
public:
    MyCredentialProviderFactory();
    virtual ~MyCredentialProviderFactory();

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv);
    STDMETHODIMP LockServer(BOOL fLock);

private:
    LONG _cRef;
};
