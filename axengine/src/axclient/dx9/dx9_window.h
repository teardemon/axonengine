/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_D3D9WINDOW_H
#define AX_D3D9WINDOW_H

AX_BEGIN_NAMESPACE

class DX9_Window : public IUnknown
{
public:
	DX9_Window();
	DX9_Window(Handle wndId);
	~DX9_Window();

	// methods inherited from ID3DXEffectStateManager
	STDMETHOD(QueryInterface)(THIS_ REFIID iid, LPVOID *ppv)
	{
		if (iid == IID_IUnknown) {
			*ppv = this;
		} else {
			*ppv = NULL;
			return E_NOINTERFACE;
		}

		AddRef();
		return S_OK;
	}
	STDMETHOD_(ULONG, AddRef)(THIS)
	{
		return(ULONG)InterlockedIncrement(&m_ref);
	}

	STDMETHOD_(ULONG, Release)(THIS)
	{
		if (0L == InterlockedDecrement(&m_ref)) {
			delete this;
			return 0L;
		}

		return m_ref;
	}


	// implement RenderTarget
	Rect getRect();
	void bind();
	void unbind();
	bool isWindow() { return true;}

	void setWindowHandle(Handle newId) { m_wndId = handle_cast<HWND>(newId); }
	Handle getWindowHandle() { return Handle(m_wndId); }

	HWND getHandle() const { return m_wndId; }
	void present();

	IDirect3DSurface9 *getSurface() { return m_backbuffer; }

protected:
	void checkSwapChain();

private:
	LONG m_ref;
	HWND m_wndId;
	Point m_swapChainSize;
	IDirect3DSwapChain9 *m_swapChain;
	IDirect3DSurface9 *m_backbuffer;
	HWND m_swapChainWnd;
	DWORD m_presentInterval;

public:
	Handle m_gbuffer;
	Handle m_lightBuffer;
};

AX_END_NAMESPACE

#endif // end guardian
