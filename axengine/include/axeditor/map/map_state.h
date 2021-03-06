/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#ifndef AX_EDITOR_MAP_STATE_H
#define AX_EDITOR_MAP_STATE_H

namespace Axon { namespace Editor { namespace MapEdit {

	class MapState : public State {
	public:
		MapState();

		float terrainBrushSize;
		float terrainBrushSoftness;
		float terrainBrushStrength;
		int terrainCurLayerId;

		bool followTerrain;
		String staticModelName;
		String treeFilename;
		String entityClass;		// current entity class for creation
	};

}}} // namespace Axon::Editor::MapEdit

#endif // AX_EDITOR_MAP_STATE_H