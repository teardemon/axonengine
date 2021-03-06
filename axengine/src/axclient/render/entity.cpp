/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#include "../private.h"

namespace Axon { namespace Render {

	//--------------------------------------------------------------------------
	// class Actor
	//--------------------------------------------------------------------------

	Entity::Entity(Kind k)
		: m_kind(k)
		, m_flags(0)
	{
		m_world = 0;
		m_distance = 0;
		m_lodRatio = 1.0f;
		m_lod = 0;
		m_affineMat.setIdentity();
		m_instanceParam.set(1,1,1,1);

		m_visQuery = g_queryManager->allocQuery();
		m_visQuery->setType(Query::QueryType_Vis);
		m_shadowQuery = g_queryManager->allocQuery();
		m_shadowQuery->setType(Query::QueryType_Shadow);

		m_viewDistCulled = false;
		m_queryCulled = false;
		m_visFrameId = 0;

		m_nodeLink.setOwner(this);
	}

	Entity::~Entity() {
		g_queryManager->freeQuery(m_shadowQuery);
		g_queryManager->freeQuery(m_visQuery);

		if (m_world) {
			m_world->removeActor(this);
		}
	}

	const Vector3& Entity::getOrigin() const {
		return m_affineMat.origin;
	}

	void Entity::setOrigin(const Vector3& origin) {
		m_affineMat.origin = origin;
	}

	const Matrix3& Entity::getAxis() const {
		return m_affineMat.axis;
	}

	void Entity::setAxis(const Angles& angles) {
		m_affineMat.setAxis(angles);
		m_instanceParam.w = 1.0f;
	}

	void Entity::setAxis(const Angles& angles, float scale) {
		m_affineMat.setAxis(angles, scale);
		m_instanceParam.w = scale;
	}

	const AffineMat& Entity::getMatrix() const {
		return m_affineMat;
	}

	void Entity::setMatrix(const AffineMat& mat) {
		m_affineMat = mat;
		m_instanceParam.w = m_affineMat.axis[0].getLength();
	}


	void Entity::updateToWorld() {
		if (!isPresented()) {
			return;
		}

		m_world->addActor(this);
	}

	int Entity::getFlags() const {
		return m_flags;
	}

	void Entity::setFlags(int flags) {
		m_flags = flags;
	}

	void Entity::addFlags(int flags) {
		m_flags |= flags;
	}

	bool Entity::isFlagSet(Flag flag) {
		return(m_flags & flag) != 0;
	}

	void Entity::setQueued(QueuedEntity* queued) {
		m_queued = queued;
#if 0
		m_queued->modelMatrix = getModelMatrix();
#endif
		m_queued->m_matrix = m_affineMat;
		m_queued->instanceParam = getInstanceParam();
		m_queued->flags = m_flags;
		m_queued->distance = m_distance;
	}

	QueuedEntity* Entity::getQueued() const {
		return m_queued;
	}


	// read only
	Matrix4 Entity::getModelMatrix() const {
		return m_affineMat.toMatrix4();
	}

	Vector4 Entity::getInstanceParam() const {
		return m_instanceParam;
	}

	void Entity::update(QueuedScene* qscene, Plane::Side side) {
		m_cullSide = side;
		doUpdate(qscene);
		doCalculateLod(qscene);

		if (!m_viewDistCulled && !m_queryCulled && m_world)
			m_visFrameId = m_world->getVisFrameId();
	}

	void Entity::doUpdate(QueuedScene* qscene) {
	}

	void Entity::doCalculateLod(QueuedScene* qscene) {
		if (!m_world)
			return;

		const Vector3& org = qscene->camera.getOrigin();
		m_distance = m_linkedBbox.pointDistance(org);

		if (m_distance < 1) {
			m_viewDistCulled = false;
			return;
		}
		
		float n = r_nearLod->getFloat();
		float f = r_farLod->getFloat();

		m_lod = 1.0f - (m_distance-n) / (f - n);
		m_lod *= m_lodRatio;
		m_lod = Math::saturate(m_lod);

		float forcelod = r_forceLod->getFloat();
		if (forcelod >= 0.0f && forcelod <= 1.0f) {
			m_lod = forcelod;
		}

		float extands = m_linkedExtends;
		float ratio = extands / m_distance / qscene->camera.getFovX() * 90 * 1024;

		if (ratio < r_viewDistCull->getFloat())
			m_viewDistCulled = true;
		else
			m_viewDistCulled = false;

		int hardwareQuery = r_hardwareQuery->getInteger();

		if (hardwareQuery == 1) {
			return;
		}

		m_queryCulled = false;
		if (!hardwareQuery) {
			return;
		}

		if (m_viewDistCulled)
			return;

		if (m_distance < 5) {
			return;
		}

		int updateframe = 1024 / ratio;

		int worldframe = m_world->getVisFrameId();

		if (m_visQuery->m_resultFrame < 0) {
			m_visQuery->issueQuery(worldframe, m_linkedBbox);
			return;
		}

		if (worldframe - m_visQuery->m_resultFrame < 20) {
			if (m_visQuery->m_result < 10) {
				// if is occluded, query every frame to avoid flick
				m_queryCulled = true;
				updateframe /= 2;
			}
		}

		updateframe = Math::clamp(updateframe, 0, 20);

		if (worldframe - m_visQuery->m_resultFrame >= updateframe) {
			m_visQuery->issueQuery(worldframe, m_linkedBbox);
		}
	}

	bool Entity::isVisable() const
	{
		if (!m_world)
			return true;

		return m_visFrameId == m_world->getVisFrameId();
	}

	void Entity::updateCsm( QueuedScene* qscene, Plane::Side side )
	{
		if (!r_csmCull->getBool())
			return;

		if (m_shadowQuery->m_resultFrame == m_world->getShadowFrameId()) {
			return;
		}

		if (side != Plane::Front)
			return;

		m_shadowQuery->issueQuery(m_world->getShadowFrameId(), m_linkedBbox);
	}

	bool Entity::isCsmCulled() const
	{
		if (!r_csmCull->getBool())
			return false;

		if (m_shadowQuery->m_resultFrame < 0)
			return false;

		if (m_shadowQuery->m_resultFrame != m_world->getShadowFrameId())
			return false;

		return m_shadowQuery->m_result == 0;
	}

	void Entity::setInstanceColor( const Vector3& color )
	{
		m_instanceParam.x = color.x;
		m_instanceParam.y = color.y;
		m_instanceParam.z = color.z;
	}

	Vector3 Entity::getInstanceColor() const
	{
		return m_instanceParam.xyz();
	}

}} // namespace Axon::Render


