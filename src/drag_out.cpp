#include "drag_out.h"
#include "shelf_items.h"
#include <windows.h>
#include <shlobj.h>
#include <objidl.h>
#include <ole2.h>

static UINT GetPreferredDropEffectFormat(void) {
    static UINT fmt = 0;
    if (!fmt) fmt = RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT);
    return fmt;
}

static FORMATETC MakeFormat(CLIPFORMAT cf) {
    FORMATETC fe = {};
    fe.cfFormat = cf;
    fe.ptd      = NULL;
    fe.dwAspect = DVASPECT_CONTENT;
    fe.lindex   = -1;
    fe.tymed    = TYMED_HGLOBAL;
    return fe;
}

// ---------- IEnumFORMATETC -------------------------------------------------

class ShelfEnumFormat : public IEnumFORMATETC {
public:
    ShelfEnumFormat() : m_ref(1), m_idx(0) {}
    virtual ~ShelfEnumFormat() {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC) {
            *ppv = static_cast<IEnumFORMATETC*>(this);
            AddRef(); return S_OK;
        }
        *ppv = NULL; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_ref);
    }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return (ULONG)r;
    }

    HRESULT STDMETHODCALLTYPE Next(ULONG celt, FORMATETC* rgelt, ULONG* fetched) override {
        ULONG n = 0;
        while (n < celt && m_idx < 2) {
            CLIPFORMAT cf = (m_idx == 0)
                ? (CLIPFORMAT)CF_HDROP
                : (CLIPFORMAT)GetPreferredDropEffectFormat();
            rgelt[n] = MakeFormat(cf);
            ++m_idx; ++n;
        }
        if (fetched) *fetched = n;
        return (n == celt) ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override {
        m_idx += celt;
        return (m_idx <= 2) ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE Reset() override { m_idx = 0; return S_OK; }
    HRESULT STDMETHODCALLTYPE Clone(IEnumFORMATETC** out) override {
        if (!out) return E_POINTER;
        ShelfEnumFormat* c = new ShelfEnumFormat();
        c->m_idx = m_idx;
        c->AddRef();
        *out = c;
        return S_OK;
    }

private:
    LONG  m_ref;
    ULONG m_idx;
};

// ---------- IDataObject ----------------------------------------------------

class ShelfDataObject : public IDataObject {
public:
    ShelfDataObject() : m_ref(1) {}
    virtual ~ShelfDataObject() {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IDataObject) {
            *ppv = static_cast<IDataObject*>(this);
            AddRef(); return S_OK;
        }
        *ppv = NULL; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_ref);
    }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return (ULONG)r;
    }

    HRESULT STDMETHODCALLTYPE GetData(FORMATETC* fe, STGMEDIUM* sm) override {
        if (!fe || !sm) return E_POINTER;

        if (fe->cfFormat == (CLIPFORMAT)GetPreferredDropEffectFormat()) {
            HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DWORD));
            if (!h) return E_OUTOFMEMORY;
            DWORD* p = (DWORD*)GlobalLock(h);
            *p = DROPEFFECT_COPY;
            GlobalUnlock(h);
            sm->tymed = TYMED_HGLOBAL;
            sm->hGlobal = h;
            sm->pUnkForRelease = NULL;
            return S_OK;
        }

        if (fe->cfFormat != CF_HDROP) return DV_E_FORMATETC;

        int n = items_count();
        if (n == 0) return DV_E_FORMATETC;

        SIZE_T bytes = sizeof(DROPFILES);
        for (int i = 0; i < n; ++i)
            bytes += (wcslen(items_get(i)) + 1) * sizeof(wchar_t);
        bytes += sizeof(wchar_t);

        HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, bytes);
        if (!h) return E_OUTOFMEMORY;

        DROPFILES* df = (DROPFILES*)GlobalLock(h);
        df->pFiles = sizeof(DROPFILES);
        df->fWide  = TRUE;
        wchar_t* p = (wchar_t*)((BYTE*)df + sizeof(DROPFILES));
        for (int i = 0; i < n; ++i) {
            wcscpy(p, items_get(i));
            p += wcslen(items_get(i)) + 1;
        }
        *p = 0;
        GlobalUnlock(h);

        sm->tymed = TYMED_HGLOBAL;
        sm->hGlobal = h;
        sm->pUnkForRelease = NULL;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC*, STGMEDIUM*) override {
        return E_NOTIMPL;
    }
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* fe) override {
        if (!fe) return E_POINTER;
        if (fe->cfFormat == CF_HDROP) return S_OK;
        if (fe->cfFormat == (CLIPFORMAT)GetPreferredDropEffectFormat()) return S_OK;
        return DV_E_FORMATETC;
    }
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC*, FORMATETC* out) override {
        if (out) out->ptd = NULL;
        return E_NOTIMPL;
    }
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC*, STGMEDIUM*, BOOL) override {
        return E_NOTIMPL;
    }
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dir, IEnumFORMATETC** out) override {
        if (!out) return E_POINTER;
        if (dir != DATADIR_GET) return E_NOTIMPL;
        ShelfEnumFormat* e = new ShelfEnumFormat();
        e->AddRef();
        *out = e;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*, DWORD, IAdviseSink*, DWORD*) override {
        return OLE_E_ADVISENOTSUPPORTED;
    }
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA**) override { return OLE_E_ADVISENOTSUPPORTED; }

private:
    LONG m_ref;
};

// ---------- IDropSource ----------------------------------------------------

class ShelfDropSource : public IDropSource {
public:
    ShelfDropSource() : m_ref(1) {}
    virtual ~ShelfDropSource() {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IDropSource) {
            *ppv = static_cast<IDropSource*>(this);
            AddRef(); return S_OK;
        }
        *ppv = NULL; return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_ref);
    }
    ULONG STDMETHODCALLTYPE Release() override {
        LONG r = InterlockedDecrement(&m_ref);
        if (r == 0) delete this;
        return (ULONG)r;
    }

    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL escape, DWORD keys) override {
        if (escape) return DRAGDROP_S_CANCEL;
        if (!(keys & MK_LBUTTON)) return DRAGDROP_S_DROP;
        return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD) override {
        return DRAGDROP_S_USEDEFAULTCURSORS;
    }
private:
    LONG m_ref;
};

// ---------- entry ----------------------------------------------------------

extern "C" HRESULT drag_out_start(HWND hwnd, DWORD ok_effect) {
    (void)hwnd;
    if (items_count() == 0) return E_UNEXPECTED;

    ShelfDataObject* data = new ShelfDataObject();
    ShelfDropSource* src  = new ShelfDropSource();

    DWORD effect = 0;
    HRESULT hr = DoDragDrop(data, src, ok_effect, &effect);

    data->Release();
    src->Release();
    return hr;
}