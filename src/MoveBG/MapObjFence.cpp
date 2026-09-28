#include <MoveBG/MapObjFence.hpp>

#include <MarioUtil/MathUtil.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <string.h>

// -inline deferred: source order is the reverse of mario.MAP emission order.

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

BOOL TRevolvingFenceOuter::receiveMessage(THitActor*, u32) { return FALSE; }

void TRevolvingFenceOuter::initMapCollisionData() { }

BOOL TRevolvingFenceInner::receiveMessage(THitActor*, u32) { return FALSE; }

void TRevolvingFenceInner::calcCurrentMtx() { }

void TRevolvingFenceInner::controlWall()
{
	// Address-of keeps the out-of-line copies.
	// -inline deferred would otherwise inline both calls away.
	void (*volatile rot)(MtxPtr, f32)   = &MsMtxSetRotY;
	f32 (*volatile wrap)(f32, f32, f32) = &MsWrap<f32>;
	(void)rot;
	(void)wrap;
}

void TRevolvingFenceInner::controlGroundRoof() { }

void TRevolvingFenceInner::setGroundCollision() { }

void TRevolvingFenceInner::control() { }

void TRevolvingFenceInner::initMapCollisionData() { }

void TRevolvingFenceInner::initMapObj() { }

void TFenceWater::draw() const { }

BOOL TFenceWater::receiveMessage(THitActor*, u32) { return FALSE; }

void TFenceWater::changeStatusToGo() { }

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

void TFenceWaterH::changeStatusToGo() { }

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

void TRailFence::load(JSUMemoryInputStream&) { }

TRailFence::TRailFence(const char* name)
    : TFence(name)
{
}
