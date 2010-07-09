/*
Copyright (c) 2009 AxonEngine Team

This file is part of the AxonEngine project, and may only be used, modified, 
and distributed under the terms of the AxonEngine project license, license.txt.  
By continuing to use, modify, or distribute this file you indicate that you have
read the license and understand and accept it fully.
*/

#ifndef AX_RENDER_QUEUE_H
#define AX_RENDER_QUEUE_H

AX_BEGIN_NAMESPACE

#if 0
struct RenderScene {
	RenderCamera camera;
	RenderWorld *world;

	Primitives primitives;

	// overlay
	Primitives overlays;
};

typedef shared_ptr<RenderScene> ScenePtr;
typedef Sequence<ScenePtr> SceneSeq;
#endif

struct AX_API RenderScene
{
	enum {
		MAX_DEBUG_INTERACTIONS = 1024,
		MAX_SUB_SCENES = 32,
		MAX_LIGHTS = 512,
		MAX_ENTITIES = 2048*4,
		MAX_INTERACTIONS = 2048*16,
		MAX_PRIMITIVES = 1024,
		MAX_OVERLAY_PRIMITIVES = 1024
	};

	enum SceneType {
		Default, WorldMain, ShadowGen, Reflection, RenderToTexture
	};

	SceneType sceneType;

	int worldFrameId;

	RenderLight *sourceLight; // if is shadowGen scene, this is light cast this shadow
	int splitIndex; // if is csm split, this is order index
	bool rendered;

	// some global shader uniform parameters
	Matrix4 windMatrices[RenderWind::NUM_WIND_MATRIXES];
	Vector4 leafAngles[RenderWind::NUM_LEAF_ANGLES];

	int m_histogramIndex; // which histogram need to query
	int m_histogramQueryId;

	float exposure;

	int numSubScenes;
	RenderScene *subScenes[MAX_SUB_SCENES];

	RenderScene *source;
	RenderCamera camera;
	RenderTarget *target; // main scene maybe have a render target
	RenderWorld *world;

	int numLights;
	RenderLight *lights[MAX_LIGHTS];
	RenderLight *globalLight;

	RenderFog *globalFog;
	RenderFog *waterFog;
	Vector4 clearColor; // clear color
	bool isEyeInWater;

	int numEntities;
	RenderEntity *queuedEntities[MAX_ENTITIES];

	int numPrimitives;
	Primitive *primtives[MAX_PRIMITIVES];

	int numInteractions;
	Interaction *interactions[MAX_INTERACTIONS];

	int numDebugInteractions;
	Interaction *debugInteractions[MAX_DEBUG_INTERACTIONS];

	int numOverlayPrimitives;
	Primitive *overlayPrimitives[MAX_OVERLAY_PRIMITIVES];

	RenderScene *addSubScene();
	void addLight(RenderLight *light);
	void addEntity(RenderEntity *entity);
	Interaction *addInteraction(RenderEntity *qentity, Primitive *prim, bool chain = true);
	void addHelperInteraction(RenderEntity *qentity, Primitive *prim);

	void finalProcess();

	bool isLastCsmSplits() const;

	void addPrimitive(Primitive *prim);
	void addOverlayPrimitive(Primitive *prim);

protected:
	void checkLights();
	bool addInteraction(Interaction *ia);
	void addHelperPrims(RenderEntity *entity);

	void findInstance();
	void sortInteractions();
};

inline bool RenderScene::isLastCsmSplits() const
{
	if (sceneType != ShadowGen)
		return false;

	if (!sourceLight)
		return false;

	if (sourceLight->getLightType() != RenderLight::kGlobal)
		return false;

	if (splitIndex != sourceLight->shadowInfo->numSplitCamera-1)
		return false;

	return true;
}

class SyncMethod;
class RenderData;

class AX_API RenderFrame
{
public:
	enum {
		QUERY_FRAME_STACK_DEPTH = 2,
		MAX_QUERIES = 4096,
		MAX_VIEW = 16
	};

	RenderFrame();
	~RenderFrame();

	RenderTarget *getTarget();

	// for providing thread
#if 0
	void beginProviding();
#endif
	MemoryStack *getMemoryStack();
	void setTarget(RenderTarget *target);
	RenderScene *allocScene();
	void addScene(RenderScene *scene);
	Interaction *allocInteraction();
	Interaction **allocInteractionPointer(int num);
#if 0
	QueuedLight *allocQueuedLight();
	QueuedEntity *allocQueuedActor(int num = 1);
	int *allocPrimitives(int num);
	void addDeferredDeleteResource(RenderData *rfr) { m_deferredDeleteResources.push_back(rfr); }
	void addDeferredCommand(SyncMethod *cmd) { m_deferredCommand.push_back(cmd); }
	void endProviding();
#endif

	template <class T>
	T *allocType(int num = 1);

	// for consuming thread
#if 0
	void beginConsuming();
	void endSync();
#endif
	int getSceneCount() { return m_sceneCount; }
	RenderScene *getScene(int index);
	void clear();
#if 0
	void endConsuming();
#endif
private:
	MemoryStack *m_stack;
	RenderTarget *m_target;
	int m_sceneCount;
	RenderScene *m_queuedScenes[MAX_VIEW];
	int m_sourceSceneCount;
#if 0
	RenderScene *m_sourceScenes[MAX_VIEW];
	SyncEvent *m_providingEvent;
	SyncEvent *m_consumingEvent;
	SyncEvent *m_cacheEndEvent;

	List<SyncMethod*> m_deferredCommand;
	List<RenderData*> m_deferredDeleteResources;
#endif
};

template <class T>
T *RenderFrame::allocType(int num)
{
#if 0
	if (num == 1) {
		return new(m_stack) T;
	} else {
		return new(m_stack) T[num];
	}
#else // don't call constructor
	return (T *)m_stack->alloc(sizeof(T) * num);
#endif
}

inline RenderTarget *RenderFrame::getTarget()
{
	return m_target;
}


AX_END_NAMESPACE

#endif // AX_RENDER_QUEUE_H
