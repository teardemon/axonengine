/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "map_local.h"

namespace Axon { namespace Editor { namespace MapEdit {

	//------------------------------------------------------------------------------
	// class PerspectiveView
	//------------------------------------------------------------------------------

	PerspectiveView::PerspectiveView(MapContext* context)
		: MapView(context)
		, m_tracking(kNone)
		, m_moving(0)
	{
		m_title = "Perspective";
#if 0
		m_eyepos.set(0, 0, 5);
		m_eyeAxis.fromAngles(Angles(30, 90, 0));
#else
		m_eyeMatrix.setOrigin(0, 0, 5);
		m_eyeMatrix.setAxis(Angles(30, 90, 0));
#endif
		m_camera.setClearColor(Rgba::Black);
		m_autoUpdate = true;

		g_gameSystem->attachObserver(this);
	}

	PerspectiveView::~PerspectiveView(){
		g_gameSystem->detachObserver(this);
	}

	void PerspectiveView::updateMove() {
		MapContext* mapContext = static_cast<MapContext*>(m_context);
		MapState* mapState = mapContext->getMapState();

		int accel = m_moving & Accel;
		accel *= 2;
		float dist = mapState->moveSpeed * (accel + 1) * (m_frameTime * 0.001f);

		Vector3 dir(0,0,0);
		if (m_moving & Front) {
			dir += m_eyeMatrix.axis[0];
		}

		if (m_moving & Back) {
			dir -= m_eyeMatrix.axis[0];
		}

		if (m_moving & Left) {
			dir += m_eyeMatrix.axis[1];
		}

		if (m_moving & Right) {
			dir -= m_eyeMatrix.axis[1];
		}

		dir.normalize();
		Vector3 origin = m_eyeMatrix.origin + dir * dist;

		Map::Terrain* terrain = m_context->getTerrain();
		if (mapState->followTerrain && terrain && terrain->isPresented()) {
			float h = terrain->getHeightByPos(m_eyeMatrix.origin) + m_camera.getZnear() * 2.0f;
			if (origin.z < h) origin.z = h;
		}

		m_eyeMatrix.origin = origin;
		m_context->notify(Context::StatusChanged);
	}

	void PerspectiveView::doRender() {
		m_context->doRender(m_camera, true);
//		gRenderSystem->addToScene(m_context->getRenderWorld());
	}

	void PerspectiveView::checkViewOrg() {
	}

	void PerspectiveView::bindFrame(IFrame* container) {
		// pre bind
		if (m_frame) {
			// unbind old first
			m_frame->removeEventSource();
			g_system->removeTickable(Axon::System::TickGame, this);
			((MapContext*)m_context)->getGameWorld()->setWindow(0);
		}

		// bind
		View::bindFrame(container);

		// post bind
		if (m_frame) {
			((MapContext*)m_context)->getGameWorld()->setWindow(m_frame->getRenderTarget());
			g_system->registerTickable(Axon::System::TickGame, this);
			m_frame->registerEventSource();
			m_context->notify(Context::EverythingChanged);
		}
	}

	void PerspectiveView::onKeyDown(Input::Event* e) {
#if 0
		doUpdate();
#endif
		setAutoUpdate(Force);

		if (e->key == 'w') {
			m_moving |= Front;
			return;
		}

		if (e->key == 's') {
			m_moving |= Back;
			return;
		}

		if (e->key == 'a') {
			m_moving |= Left;
			return;
		}

		if (e->key == 'd') {
			m_moving |= Right;
			return;
		}

		if (e->key == Key::Shift) {
			m_moving |= Accel;
			return;
		}
	}

	void PerspectiveView::onKeyUp(Input::Event* e) {
		setAutoUpdate(Default);

		switch (e->key) {
		case 'w':
			m_moving &= ~Front;
			break;
		case 's':
			m_moving &= ~Back;
			break;
		case 'a':
			m_moving &= ~Left;
			break;
		case 'd':
			m_moving &= ~Right;
			break;
		case Key::Shift:
			m_moving &= ~Accel;
			break;
		}
	}

	void PerspectiveView::onMouseDown(Input::Event* e) {
		if (e->key == Key::MouseRight && e->flags & Input::Event::AltModifier) {
			m_tracking = kPan;
			m_trackingPos = e->pos;
			e->accepted = true;
			m_frame->setCursor(CursorType::ViewPan);
			m_isTrackingCenterSet = true;

			if (!m_context->getSelection().empty()) {
				ActorList alist = m_context->getSelection();
				m_trackingCenter = alist.getCenter();
				return;
			}
			if (!traceWorld(m_trackingCenter)) {
				m_isTrackingCenterSet = true;
				m_trackingCenter.clear();
			}
			return;
		}

		if (e->key == Key::MouseRight && e->flags & Input::Event::ControlModifier && !(m_moving&MoveMask)) {
			m_tracking = kHorizonPan;
			m_trackingPos = e->pos;
			e->accepted = true;
			m_frame->setCursor(CursorType::ViewPan);

			return;
		}

		if (e->key == Key::MouseRight) {
			m_tracking = kRotateHead;
			m_trackingPos = e->pos;
			e->accepted = true;
			m_frame->setCursor(CursorType::Blank);

			return;
		}

		if (e->key == Key::MouseLeft && e->flags & Input::Event::AltModifier) {
			m_tracking = kRotate;
			m_trackingPos = e->pos;
			e->accepted = true;
			m_frame->setCursor(CursorType::ViewRotate);

			if (!m_context->getSelection().empty()) {
				ActorList alist = m_context->getSelection();
				m_trackingCenter = alist.getCenter();
				return;
			}
			if (!traceWorld(m_trackingCenter)) {
				m_trackingCenter.clear();
			}
			return;
		}

	}

	void PerspectiveView::onMouseUp(Input::Event* e) {
//		if (e->key == Key::MouseRight) {
			m_tracking = kNone;
			e->accepted = true;
			m_frame->setCursor(CursorType::Default);
			return;
//		}
	}

	void PerspectiveView::onMouseMove(Input::Event* e) {
		MapContext* mapContext = static_cast<MapContext*>(m_context);
		MapState* mapState = mapContext->getMapState();

		if (!m_tracking)
			return;

		if (m_tracking == kRotateHead) {
			Angles angles = m_eyeMatrix.getAngles();

			Point delta = e->pos - m_trackingPos;
			angles.pitch += delta.y * 0.1f;
			angles.yaw -= delta.x * 0.1f;
			angles.roll = 0;

			angles.pitch = Math::clamp(angles.pitch, -90.0f, 90.0f);

			m_eyeMatrix.setAxis(angles);

			m_frame->setCursorPos(m_trackingPos);

			return;
		}

		if (m_tracking == kHorizonPan) {
			Matrix3 axis = m_eyeMatrix.axis;
			axis[2].set(0, 0, 1);
			axis[0] = axis[1] ^ axis[2];
			axis[1] = axis[2] ^ axis[0];
			axis[0].normalize();
			axis[1].normalize();

			Point delta = e->pos - m_trackingPos;
			Vector3 org = m_eyeMatrix.origin;
			float dist = mapState->moveSpeed * (m_frameTime * 0.001f);
			org += ( axis[0] * delta.y + axis[1] * delta.x ) * dist;

			m_eyeMatrix.origin = org;

			m_trackingPos = e->pos;

			return;
		}

		if (m_tracking == kRotate) {
			Point delta = e->pos - m_trackingPos;
			AffineMat mat1, mat2, mat3;
#if 1
			Vector3 offset = m_eyeMatrix.origin - m_trackingCenter;
			Angles angles = m_eyeMatrix.getAngles();

			angles.pitch += delta.y * 0.1f;
			angles.yaw -= 0; //delta.x * 0.1f;
			angles.roll = 0;
			angles.pitch = Math::clamp(angles.pitch, -90.0f, 90.0f);

			Angles deltaangle = angles - m_eyeMatrix.getAngles();

			Matrix3 axisPitch, axisYaw;
			axisPitch.fromAngles(Angles(deltaangle.pitch, 0, 0));
			axisYaw .fromAngles(Angles(0, deltaangle.yaw, 0));

//			angles += m_eyeMatrix.getAngles();

			Vector3 pos = m_trackingCenter + /*axisYaw**/ axisPitch.getInverse() * offset;

			m_eyeMatrix.origin = pos;
			m_eyeMatrix.setAxis(angles);
#endif
			m_trackingPos = e->pos;
			return;
		}

		if (m_tracking == kPan) {
			if (m_isTrackingCenterSet) {
				Vector3 center = m_camera.worldToScreen(m_trackingCenter);
				center.x += e->pos.x - m_trackingPos.x;
				center.y += e->pos.y - m_trackingPos.y;
				Vector3 offseted = m_camera.screenToWorld(center);
				Vector3 offset = offseted - m_trackingCenter;
				m_eyeMatrix.origin = m_eyeMatrix.origin - offset;
				m_trackingPos = e->pos;
				return;
			}
			return;
		}
	}

	void PerspectiveView::onMouseWheel(Input::Event* e) {
		MapContext* mapContext = static_cast<MapContext*>(m_context);
		MapState* mapState = mapContext->getMapState();

		float delta = e->delta;
		
		delta /= 8 * 15;
		delta *= mapState->moveSpeed * 0.1f;

		if (e->flags & Input::Event::ShiftModifier) {
			delta += delta;
		}

		Vector3 origin = m_eyeMatrix.origin;
		origin += m_eyeMatrix.axis[0] * delta;
		m_eyeMatrix.origin = (origin);
	}

	void PerspectiveView::handleEvent(Input::Event* e) {
		View::handleEvent(e);
//		onUpdateView();
	}


#if 0
	// TODO: select part should as parameter
	bool
	PerspectiveView::SelectRegion(const Rect& rect, OUT ActorList& retlist, OUT Vector3& pos, MapSelectPart select_part) {
		retlist.clear();

		Render::Camera select_view = m_camera.createSelectView(rect);

		gRenderSystem->beginSelect(select_view);

		// select terrain
		if (select_part & MSP_TerrainChunks) {
//			mEditorTerrain->OnSelect();
		}

		edEntityHash& entities = m_context->getEntityHash();

		// select map entities
		if (select_part & MSP_MapEntities) {
			edEntityHash::iterator it;

			for (it = entities.begin(); it != entities.end(); ++it) {
				Entity* ent = *it;

				if (ent->GetType() != edEntityType_MapEntity)
					continue;

				ent->testSelect();
			}
		}

		Render::HitRecords records = gRenderSystem->endSelect();

		if (records.empty())
			return false;

		float minz = 1.0f;
		for (size_t i = 0; i < records.size(); i++) {
			Int id = records[i].name;
			Actor* ed = gEditorSystem->getEditable(id);

			AX_ASSERT(ed);

			retlist.push_back(ed);

			if (records[i].minz < minz)
				minz = records[i].minz;
		}

		Point center = rect.getCenter();
		Vector3 v(center.x, center.y, minz);

		pos = m_cursorPos = select_view.screenToWorld(v);

		return true;
	}

#endif

	void PerspectiveView::tick() {
		doUpdate();
	}

	void PerspectiveView::doNotify( IObservable* subject, int arg )
	{
		if (subject != g_gameSystem)
			return;

		if (arg & GameSystem::ObserveFlag_StopRunning) {
			GameWorld* gw = static_cast<MapContext*>(m_context)->getGameWorld();

			m_eyeMatrix = gw->getLastViewMatrix();
		}
	}

}}} // namespace Axon::Editor::MapEdit
