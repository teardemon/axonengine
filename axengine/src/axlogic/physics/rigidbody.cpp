/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#include "../private.h"

namespace Axon { namespace Physics {

	//--------------------------------------------------------------------------
	// class RigidBody
	//--------------------------------------------------------------------------

	RigidBody::RigidBody() {
		m_package = nullptr;
		m_havokRigid = nullptr;
	}

	RigidBody::RigidBody(Package* package, hkpRigidBody* rigid) {
		m_package = package;
		m_havokRigid = rigid;
		m_havokRigid->setUserData(e2n(this));
		setAutoDeactive(true);
	}

	RigidBody::RigidBody(const String& name)
		: m_package(nullptr)
		, m_havokRigid(nullptr)
	{
		m_package = g_physicsPackageManager->findPackage(name);

		if (!m_package) {
			return;
		}

		m_havokRigid = m_package->getRigidBodyHk();
		if (!m_havokRigid) {
			return;
		}

		m_havokRigid->setUserData(e2n(this));

		setAutoDeactive(true);
	}

	RigidBody::~RigidBody() {
		if (m_world) {
			unbind(m_world);
		}

		if (m_havokRigid) {
			m_havokRigid->removeReference();
		}
		if (m_package) {
			m_package->release();
		}
	}

	void RigidBody::setActive(bool b) {
		if (!m_havokRigid->getWorld()) {
			return;
		}

		if (b) {
			m_havokRigid->activate();
		} else {
			m_havokRigid->deactivate();
		}
	}

	bool RigidBody::isActive() const {
		return m_havokRigid->isActive();
	}

	void RigidBody::setMatrix(const AffineMat& matrix) {
		hkTransform hk;
		Matrix3 axis = matrix.axis;
		axis.removeScale();

		AffineMat newmtx(axis, matrix.origin);
		x2h(newmtx, hk);

		if (m_havokRigid) {
			m_havokRigid->setTransform(hk);
		}

		m_matrix = matrix;
	}

	AffineMat RigidBody::getMatrix() const {
		if (m_havokRigid) {
			return h2x(m_havokRigid->getTransform());
		}
		return m_matrix;
	}

	void RigidBody::setMotionType(MotionType motion) {
		if (m_havokRigid)
			m_havokRigid->setMotionType(x2h(motion));

		m_motionType = motion;
	}

	Entity::MotionType RigidBody::getMotionType() const {
		if (m_havokRigid)
			return h2x(m_havokRigid->getMotionType());

		return m_motionType;
	}

	void RigidBody::setAutoDeactive(bool val) {
		if (!m_havokRigid) return;

		if (val) {
			m_havokRigid->setDeactivator(hkpRigidBodyDeactivator::DEACTIVATOR_SPATIAL);
		} else {
			m_havokRigid->setDeactivator(hkpRigidBodyDeactivator::DEACTIVATOR_NEVER);
		}
	}

	void RigidBody::bind(World* world) {
		if (!m_havokRigid) return;

		world->m_havokWorld->addEntity(m_havokRigid);
	}

	void RigidBody::unbind(World* world) {
		if (!m_havokRigid) return;

		world->m_havokWorld->removeEntity(m_havokRigid);
	}

	//--------------------------------------------------------------------------
	// class Terrain
	//--------------------------------------------------------------------------

	class MyHeightFieldShape : public hkpSampledHeightFieldShape {
	public:
		MyHeightFieldShape(const hkpSampledHeightFieldBaseCinfo& ci, const hkUint16* data, int size)
			: hkpSampledHeightFieldShape(ci)
			, m_data(data)
			, m_size(size)
		{}

		// Generate a rough terrain
		HK_FORCE_INLINE hkReal getHeightAtImpl(int x, int z) const {
			// Lookup data and return a float
			// We scale the data artifically by 5 to make it look interesting
			z = m_size - z;
			return hkReal(m_data[z * m_zRes + x]) / hkReal(hkUint16(-1)) * 2048.0f - 1024.0f;
		}

		//	This should return true if the two triangles share the edge p00-p11
		// otherwise it should return false if the triangles share the edge p01-p10
		HK_FORCE_INLINE hkBool getTriangleFlipImpl() const {	
			return false;
		}

		virtual void collideSpheres(const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const {
			hkSampledHeightFieldShape_collideSpheres(*this, input, outputArray);
		}

	private:
		const hkUint16* m_data;
		int m_size;
	};


	Terrain::Terrain(const ushort_t* data, int size, float tilemeters) {
		hkpSampledHeightFieldBaseCinfo shapeCinfo;
		shapeCinfo.m_xRes = size;
		shapeCinfo.m_zRes = size;
		shapeCinfo.m_scale.set(tilemeters, 1, tilemeters);
		shapeCinfo.m_minHeight = -1024.0f;
		shapeCinfo.m_maxHeight = 1024.0f;

		MyHeightFieldShape* shape = new MyHeightFieldShape(shapeCinfo, data, size);

		hkpRigidBodyCinfo ci;
		ci.m_motionType = hkpMotion::MOTION_FIXED;
		ci.m_shape = shape;
		ci.m_position.set(-0.5f * size * tilemeters, 0.5f *  size * tilemeters, 0);

		//
		// NOTE: this heightfield will be oriented such that the Z-axis is its up axis, i.e. heightfield 'plane' will be in (X,Y)
		//
		ci.m_rotation = hkQuaternion(hkVector4(1.0f, 0.0f, 0.0f, 0.0f), HK_REAL_PI / 2);

		m_havokRigid = new hkpRigidBody(ci);
	}

	Terrain::~Terrain() {}

}} // namespace Axon::Physics

