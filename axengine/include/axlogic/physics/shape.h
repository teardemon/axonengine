/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#ifndef AX_PHYSICS_SHAPE_H
#define AX_PHYSICS_SHAPE_H

namespace Axon { namespace Physics {

	class Shape {
	public:
		Shape();
		virtual ~Shape();

	protected:
		hkpShape* m_rep;
	};


	class Heightfield : public Shape {
	public:
		Heightfield(const ushort_t* data, int size, float tilemeters);
		virtual ~Heightfield();
	};

}} // namespace Axon::Physics

#endif // end guardian

