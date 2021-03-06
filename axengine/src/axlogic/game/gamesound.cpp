/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/


#include "../private.h"

namespace Axon { namespace Game {



	GameSound::GameSound()
	{

	}

	GameSound::~GameSound()
	{

	}

	void GameSound::doSelectTest() const
	{

	}

	void GameSound::doDebugRender() const
	{

	}

	void GameSound::doThink()
	{

	}

	void GameSound::loadSound( const Variant& v )
	{
		clear();

		if (v.type != Variant::kTable) {
			return;
		}

		LuaTable table = v;

		table.beginRead();
		String name = table.get("sound");
		m_minDist = table.get("minDist");
		m_maxDist = table.get("maxDist");
		m_looping = table.get("looping");
		m_interval = table.get("interval");
		table.endRead();

		if (name.empty())
			return;

		m_sfx << g_soundSystem->createSfx(name);
	}

	void GameSound::activeSound( bool isActive )
	{
		if (isActive && m_sfx) {
			Sound::LoopingMode looping = Sound::Looping_None;
			if (m_looping) {
				looping = Sound::Looping_Forever;
			}
			getSoundEntity()->playSound(SndChannelId_Ambient, m_sfx, looping, m_minDist, m_maxDist);
		}
	}

	void GameSound::clear()
	{
		m_sfx.clear();
	}

}} // namespace Axon::Game

