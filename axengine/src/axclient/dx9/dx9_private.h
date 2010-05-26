/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#ifndef AX_DIRECT9_PRIVATE_H
#define AX_DIRECT9_PRIVATE_H

#include "../private.h"

#if defined(_DEBUG) || defined(DEBUG)
#define D3D_DEBUG_INFO
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr.h>

#if 1 || defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { HRESULT hr = (x); if (FAILED(hr)) { Errorf("%s(%d): %s", __FILE__, __LINE__, #x); } }
//#define V(x)           { hr = (x); if (FAILED(hr)) { DXTrace(__FILE__, __LINE__, hr, L#x, true); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if (FAILED(hr)) { Errorf("%s(%d): %s in %s", __FILE__, __LINE__, D3DErrorString(hr), #x); return hr; } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if (FAILED(hr)) { return hr; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#if 0
#define D3D9_SCOPELOCK ScopeLock _autoLocker(d3d9Mutex)
#endif

#define BEGIN_PIX(x) D3DPERF_BeginEvent(D3DCOLOR_RGBA(0,0,0,255), L##x)
#define END_PIX() D3DPERF_EndEvent()

#include "dx9_shader.h"
#include "dx9_window.h"
#include "dx9_driver.h"
#include "dx9_state.h"
#include "dx9_api.h"

AX_BEGIN_NAMESPACE

inline const char *D3DErrorString(HRESULT hr)
{
#define D3DERR(x) case x: return #x;
	switch (hr) {
	D3DERR(D3D_OK)
	D3DERR(D3DERR_WRONGTEXTUREFORMAT)
	D3DERR(D3DERR_UNSUPPORTEDCOLOROPERATION)
	D3DERR(D3DERR_UNSUPPORTEDCOLORARG)
	D3DERR(D3DERR_UNSUPPORTEDALPHAOPERATION)
	D3DERR(D3DERR_UNSUPPORTEDALPHAARG)
	D3DERR(D3DERR_TOOMANYOPERATIONS)
	D3DERR(D3DERR_CONFLICTINGTEXTUREFILTER)
	D3DERR(D3DERR_UNSUPPORTEDFACTORVALUE)
	D3DERR(D3DERR_CONFLICTINGRENDERSTATE)
	D3DERR(D3DERR_UNSUPPORTEDTEXTUREFILTER)
	D3DERR(D3DERR_CONFLICTINGTEXTUREPALETTE)
	D3DERR(D3DERR_DRIVERINTERNALERROR)
	D3DERR(D3DERR_NOTFOUND)
	D3DERR(D3DERR_MOREDATA)
	D3DERR(D3DERR_DEVICELOST)
	D3DERR(D3DERR_DEVICENOTRESET)
	D3DERR(D3DERR_NOTAVAILABLE)
	D3DERR(D3DERR_OUTOFVIDEOMEMORY)
	D3DERR(D3DERR_INVALIDDEVICE)
	D3DERR(D3DERR_INVALIDCALL)
	D3DERR(D3DERR_DRIVERINVALIDCALL)
	D3DERR(D3DERR_WASSTILLDRAWING)
	default: return "unknown error";
	}
#undef D3DERR
}

extern DX9_Window *d3d9InternalWindow;
extern DX9_Driver *d3d9Driver;
extern RenderDriverInfo *d3d9DriverInfo;
#if 0
extern RenderQueue *d3d9Queue;
extern SyncMutex d3d9Mutex;
#endif
extern IDirect3D9 *d3d9Api;
extern IDirect3DDevice9 *d3d9Device;

extern D3D9ShaderManager *d3d9ShaderManager;
#if 0
//extern D3D9TargetManager *d3d9TargetManager;
extern D3D9querymanager *d3d9QueryManager;
extern D3D9primitivemanager *d3d9PrimitiveManager;
extern D3D9Thread *d3d9Thread;
extern D3D9VertexBufferManager *d3d9VertexBufferManager;
extern D3D9Postprocess *d3d9Postprocess;
extern D3D9StateManager *d3d9StateManager;
extern D3D9Draw *d3d9Draw;
extern const GUID d3d9ResGuid;

// vendor specified FOURCC caps
extern bool d3d9NVDB;
extern bool d3d9NULL;

extern RenderCamera *d3d9Camera;

// thread state
extern QueuedScene *d3d9Scene;

extern DX9_Window *d3d9FrameWnd;
extern bool d3d9IsReflecting;
extern bool d3d9ForceWireframe;

extern QueuedScene *d3d9WorldScene;
//extern D3D9Target *d3d9WorldTarget;
#if 0
extern D3D9target *d3d9ShadowMaskTarget;
#endif
extern const QueuedEntity *d3d9Actor;
extern Interaction *d3d9Interaction;
extern RenderTarget *d3d9BoundTarget;
#endif

AX_END_NAMESPACE

#endif // end guardian
