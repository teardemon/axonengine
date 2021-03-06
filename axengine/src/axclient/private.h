/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_RENDER_PRIVATE_H
#define AX_RENDER_PRIVATE_H

#ifdef _LIB
#	define AX_CLIENT_API
#else
#	define AX_CLIENT_API AX_DLL_EXPORT
#endif

#include <axcore/public.h>
#include <axclient/public.h>

#include "render/selection.h"

namespace Axon {

	extern Render::FontFactory* g_fontFactory;
	extern Render::IDriver* g_renderDriver;

#ifdef AX_CONFIG_OPTION_USE_SPEEDTREE_40
	extern Render::TreeManager* g_treeManager;
#endif // AX_CONFIG_OPTION_USE_SPEEDTREE_40
} // namespace Axon

#endif // AX_CLIENT_PRIVATE_H

