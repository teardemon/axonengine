/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/



#include "gfx_local.h"

namespace {

	AX_USE_NAMESPACE;

	float frand()
	{
		return rand()/(float)RAND_MAX;
	}

	float randfloat(float lower, float upper)
	{
		return lower + (upper-lower)*(rand()/(float)RAND_MAX);
	}

	int randint(int lower, int upper)
	{
		return lower + (int)((upper+1-lower)*frand());
	}

	//Generates the rotation matrix based on spread
	Matrix4 SpreadMat;
	void CalcSpreadMatrix(float Spread1, float Spread2, float w, float l)
	{
		int i,j;
		float a[2], c[2], s[2];
		Matrix4 Temp;

		Temp.setIdentity();

		a[0] = randfloat(-Spread1, Spread1)/2.0f;
		a[1] = randfloat(-Spread2, Spread2)/2.0f;

		for (i=0; i<2; i++) {		
			c[i]=cos(a[i]);
			s[i]=sin(a[i]);
		}

		Temp.setIdentity();
		Temp.m[1][1]=c[0];
		Temp.m[2][1]=s[0];
		Temp.m[2][2]=c[0];
		Temp.m[1][2]=-s[0];

		SpreadMat=SpreadMat*Temp;

		Temp.setIdentity();
		Temp.m[0][0]=c[1];
		Temp.m[1][0]=s[1];
		Temp.m[1][1]=c[1];
		Temp.m[0][1]=-s[1];

		SpreadMat=SpreadMat*Temp;

		float Size=abs(c[0])*l+abs(s[0])*w;
		for (i=0; i<3; i++)
			for (j=0; j<3; j++)
				SpreadMat.m[i][j]*=Size;
	}

	template<class T>
	T lifeRamp(float life, float mid, const T &a, const T &b, const T &c)
	{
		if (life<=mid) return interpolate<T>(life / mid,a,b);
		else return interpolate<T>((life-mid) / (1.0f-mid),b,c);
	}

} // anonymous namespace

AX_BEGIN_NAMESPACE

static BlockAlloc<Particle> ParticleAllocator;

ParticleEmitter::ParticleEmitter()
{

}

ParticleEmitter::~ParticleEmitter()
{

}

Particle* ParticleEmitter::planeEmit(float width, float length, float speed, float variant, float spread, float spread2)
{
	Particle* p = ParticleAllocator.alloc();

	p->pos = Vector3(randfloat(-length,length), 0, randfloat(-width,width));
	Vector3 dir = Vector3(0,1,0);

	p->dir = dir;//.normalize();
	p->down = Vector3(0,-1.0f,0); // dir * -1.0f;
	p->speed = dir * speed * (1.0f+randfloat(-variant,variant));

	return 0;
}

Particle* ParticleEmitter::sphereEmit(float w, float l, float spd, float var, float spr, float spr2)
{
	return 0;
}

BoundingBox ParticleEmitter::getLocalBoundingBox()
{
	return BoundingBox::UnitBox;
}

BoundingBox ParticleEmitter::getBoundingBox()
{
	return getLocalBoundingBox().getTransformed(m_tm);
}

Primitives ParticleEmitter::getHitTestPrims()
{
	return Primitives();
}

void ParticleEmitter::frameUpdate(QueuedScene *qscene)
{
	float grav = m_Gravity;
	float deaccel = m_Gravity2;

	// spawn new particles
	float frate = m_EmissionRate;
	float flife = m_Lifespan;
	float dt = qscene->camera.getFrameTime() * 0.001f;

	float rem = m_remain;
	float ftospawn = (dt * frate / flife) + rem;
	if (ftospawn < 1.0f) {
		rem = ftospawn;
		if (rem<0) 
			rem = 0;
	} else {
		int tospawn = (int)ftospawn;

		if ((tospawn + m_particles.size()) > MAX_PARTICLES) // Error check to prevent the program from trying to load insane amounts of particles.
			tospawn = (int)m_particles.size() - MAX_PARTICLES;

		rem = ftospawn - (float)tospawn;


		float w = m_EmissionAreaWidth * 0.5f;
		float l = m_EmissionAreaLength * 0.5f;
		float spd = m_EmissionSpeed;
		float var = m_SpeedVariation;
		float spr = m_VerticalRange;
		float spr2 = m_HorizontalRange;
		bool en = m_Enabled != 0;

		if (en) {
			for (int i=0; i<tospawn; i++) {
				Particle* p = planeEmit(w, l, spd, var, spr, spr2);
				m_particles.push_back(p);
			}
		}
	}

	float mspeed = 1.0f;

	for (List<Particle*>::iterator it = m_particles.begin(); it != m_particles.end(); ) {
		Particle &p = **it;
		p.speed += p.down * grav * dt - p.dir * deaccel * dt;

		if (m_slowdown>0) {
			mspeed = expf(-1.0f * m_slowdown * p.life);
		}
		p.pos += p.speed * mspeed * dt;

		p.life += dt;
		float rlife = p.life / p.maxlife;
		// calculate size and color based on lifetime
		p.size = lifeRamp<float>(rlife, m_mid, m_sizes[0], m_sizes[1], m_sizes[2]);
		p.color = lifeRamp<Vector4>(rlife, m_mid, m_colors[0], m_colors[1], m_colors[2]);

		// kill off old particles
		if (rlife >= 1.0f) 
			m_particles.erase(it++);
		else 
			++it;
	}
}

void ParticleEmitter::issueToQueue( QueuedScene *qscene )
{

}

AX_END_NAMESPACE
