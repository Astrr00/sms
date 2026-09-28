#include <MoveBG/MapObjFence.hpp>

#include <Enemy/Conductor.hpp>
#include <Enemy/Graph.hpp>
#include <Map/MapCollisionManager.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <string.h>

// -inline deferred: source order is the reverse of mario.MAP emission order.

f32 TFenceWater::mWaterAccel = 2.1f;

BOOL TFence::receiveMessage(THitActor*, u32 message)
{
	if (message == 3) {
		startBck("fence_normal_shake");
		return TRUE;
	}
	return FALSE;
}

void TFence::initMapCollisionData() { }

void TFence::initMapObj()
{
	if (strstr(unkF4, "bamboo") != nullptr)
		unk138 = 1;
	TMapObjBase::initMapObj();
}

BOOL TRevolvingFenceOuter::receiveMessage(THitActor*, u32 message)
{
	if (message == HIT_MESSAGE_SUPER_HIP_DROP) {
		startBck("fence_revolve_outer_shake");
		unk13C->startBck("fence_revolve_inner_shake");
		return TRUE;
	}
	return FALSE;
}

void TRevolvingFenceOuter::initMapCollisionData() { }

BOOL TRevolvingFenceInner::receiveMessage(THitActor*, u32) { return FALSE; }

void TRevolvingFenceInner::calcCurrentMtx() { }

#pragma dont_inline on
void TRevolvingFenceInner::controlWall()
{
	// Address-of keeps the out-of-line copies.
	// -inline deferred would otherwise inline both calls away.
	void (*volatile rot)(MtxPtr, f32)   = &MsMtxSetRotY;
	f32 (*volatile wrap)(f32, f32, f32) = &MsWrap<f32>;
	(void)rot;
	(void)wrap;
}
#pragma dont_inline off

// Empty here. dont_inline keeps the call in control().
#pragma dont_inline on
void TRevolvingFenceInner::controlGroundRoof() { }
#pragma dont_inline off

void TRevolvingFenceInner::setGroundCollision() { }

void TRevolvingFenceInner::control()
{
	TMapObjBase::control();
	if (unk140 != 0)
		controlWall();
	else
		controlGroundRoof();
}

void TRevolvingFenceInner::initMapCollisionData()
{
	mMapCollisionManager = new TMapCollisionManager(1, "mapObj", this);
	if (fabsf(mRotation.x) < 80.0f && fabsf(mRotation.z) < 80.0f)
		mMapCollisionManager->init("fence_revolve_inner_v_tool", 1, nullptr);
	else
		mMapCollisionManager->init("fence_revolve_inner_h_tool", 1, nullptr);
}

void TRevolvingFenceInner::initMapObj()
{
	if (strstr(unkF4, "bamboo") != nullptr)
		unk138 = 1;

	TMapObjBase::initMapObj();

	if (fabsf(mRotation.x) < 1.0f && fabsf(mRotation.z) < 1.0f)
		unk140 = 1;
	else
		unk140 = 0;

	TMapCollisionManager* mgr = mMapCollisionManager;
	Mtx mtx;
	MsMtxSetTRS(mtx, mPosition, mRotation, mScaling);
	TMapCollisionBase* col = mgr->unk8;
	MTXCopy(mtx, col->unk20);
	col->setUp();
}

void TFenceWater::draw() const { }

BOOL TFenceWater::receiveMessage(THitActor*, u32 message)
{
	if (!isState(3) && message == HIT_MESSAGE_SPRAYED_BY_WATER) {
		unk13C = mWaterAccel;
		if (unk13C > 0.0f)
			changeStatusToGo();
		return TRUE;
	}
	return FALSE;
}

void TFenceWater::changeStatusToGo()
{
	// Local keeps gpMSound in r0 across the this-save (addi r31).
	MSound* sound = gpMSound;
	if (sound->gateCheck(MSD_SE_OBJ_WATER_FENCE_FW))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_WATER_FENCE_FW, &mPosition, 0, nullptr, 0, 4);
	mState = 2;

	// Dead slot so MWCC keeps frame -0x20.
	char trash[1];
	trash[0] = 0;
}

void TFenceWater::changeStatusToWait()
{
	unk140 = 0.0f;
	unk13C = 0.0f;
	mState  = 1;
}

void TFenceWater::controlRotation() { }

void TFenceWater::control() { }

void TFenceWater::initMapCollisionData() { TMapObjBase::initMapCollisionData(); }

void TFenceWater::initMapObj() { }

void TFenceWaterH::control() { }

void TFenceWaterH::changeStatusToGo()
{
	// Local keeps gpMSound in r0 across the this-save (addi r31).
	MSound* sound = gpMSound;
	if (sound->gateCheck(MSD_SE_OBJ_WATER_FENCE_FW))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_WATER_FENCE_FW, &mPosition, 0, nullptr, 0, 4);
	mState = 2;
	setUpMapCollision(1);

	// Dead slot so MWCC keeps frame -0x20.
	char trash[1];
	trash[0] = 0;
}

void TFenceWaterH::changeStatusToWait()
{
	unk140 = 0.0f;
	unk13C = 0.0f;
	mState  = 1;
	setUpMapCollision(0);
}

BOOL TRailFence::receiveMessage(THitActor*, u32) { return FALSE; }

void TRailFence::falling() { }

void TRailFence::goOnRail() { }

void TRailFence::control() { }

void TRailFence::initMapCollisionData() { TMapObjBase::initMapCollisionData(); }

void TRailFence::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	char name[0x40];
	stream.readString(name, 0x40);
	TGraphWeb* graph = gpConductor->getGraphByName(name);
	if (graph != nullptr && graph->isDummy() == 0) {
		unk13C->setGraph(graph);
		unk13C->setTo(graph->findNearestNodeIndex(mPosition, 0xffffffff));
	}
	unk140   = 8.0f;
	mGravity = 0.3f;
}

TRailFence::TRailFence(const char* name)
    : TFence(name)
    , unk13C(new TGraphTracer)
    , unk140(0.0f)
{
}
