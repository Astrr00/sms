#include <MoveBG/MapObjFence.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DAnimation.hpp>
#include <M3DUtil/InfectiousStrings.hpp>

#include <Enemy/Conductor.hpp>
#include <Enemy/Graph.hpp>
#include <Map/MapCollisionManager.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>
// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <string.h>

// -inline deferred: source order is the reverse of mario.MAP emission order.

// Usual TU prefix. initMapCollisionData pools string addresses from the
// rodata base, so these bytes have to stay ahead of "fence3x3".
static const char cDirtyFileName[] = "/scene/map/pollution/H_ma_rak.bti";
static const char cDirtyTexName[]  = "H_ma_rak_dummy";
static const char cMessengerName[] = "地形オブジェメッセンジャー";
static const char cObjGroupName[]  = "オブジェクトグループ";

f32 TFenceWater::mWaterAccel     = 2.1f;
f32 TFenceWater::mBackSpeed      = 3.0f;
int TFenceWater::mTurnedWaitTime = 600;
f32 TRailFence::mFallHeight      = 50000.0f;
int TRailFence::mWaitTime        = 240;

BOOL TFence::receiveMessage(THitActor*, u32 message)
{
	if (message == 3) {
		startBck("fence_normal_shake");
		return TRUE;
	}
	return FALSE;
}

void TFence::initMapCollisionData()
{
	mMapCollisionManager = new TMapCollisionManager(1, "mapObj", this);
	if (strcmp(unkF4, "fence3x3") != 0) {
		if (fabsf(mRotation.x) < 1.0f && fabsf(mRotation.z) < 1.0f)
			mMapCollisionManager->init("fence_normal_v_tool", 0, nullptr);
		else
			mMapCollisionManager->init("fence_h_tool", 0, nullptr);
	} else {
		if (fabsf(mRotation.x) < 1.0f && fabsf(mRotation.z) < 1.0f)
			mMapCollisionManager->init("fence_half_v_tool", 0, nullptr);
		else
			mMapCollisionManager->init("fence_half_h_tool", 0, nullptr);
	}

	TMapCollisionManager* mgr = mMapCollisionManager;
	Mtx mtx;
	MsMtxSetTRS(mtx, mPosition, mRotation, mScaling);
	TMapCollisionBase* col = mgr->unk8;
	MTXCopy(mtx, col->unk20);
	col->setUp();
}

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

void TRevolvingFenceOuter::initMapCollisionData()
{
	mMapCollisionManager = new TMapCollisionManager(1, "mapObj", this);
	if (fabsf(mRotation.x) < 1.0f && fabsf(mRotation.z) < 1.0f)
		mMapCollisionManager->init("fence_revolve_outer_v_tool", 0, nullptr);
	else
		mMapCollisionManager->init("fence_revolve_outer_h_tool", 0, nullptr);

	JGeometry::TVec3<f32> scaleBamboo;
	JGeometry::TVec3<f32> scaleInner;
	TMapCollisionManager* mgr = mMapCollisionManager;
	Mtx mtx;
	MsMtxSetTRS(mtx, mPosition, mRotation, mScaling);
	TMapCollisionBase* col = mgr->unk8;
	MTXCopy(mtx, col->unk20);
	col->setUp();

	TMapObjBase* child;
	if (unk138 != 0) {
		// Address in a pointer so it stays in r6 across the stores.
		JGeometry::TVec3<f32>* scale = &scaleBamboo;
		scale->x                     = 1.0f;
		scale->y                     = 1.0f;
		scale->z                     = 1.0f;
		child = TMapObjBaseManager::newAndRegisterObj(
		    "bambooFence_revolve_inner", mPosition, mRotation, *scale);
	} else {
		JGeometry::TVec3<f32>* scale = &scaleInner;
		scale->x                     = 1.0f;
		scale->y                     = 1.0f;
		scale->z                     = 1.0f;
		child = TMapObjBaseManager::newAndRegisterObj(
		    "fence_revolve_inner", mPosition, mRotation, *scale);
	}
	unk13C = child;
	unk13C->appear();
}

BOOL TRevolvingFenceInner::receiveMessage(THitActor*, u32 message)
{
	// TODO: unk140 != 0 still has the hip-drop angle path.
	if (message == 3 && unk140 == 0) {
		if (isState(1)) {
			if (gpMSound->gateCheck(MSD_SE_OBJ_FENCE_REVERSE1))
				MSoundSESystem::MSoundSE::startSoundActor(
				    MSD_SE_OBJ_FENCE_REVERSE1, &mPosition, 0, nullptr, 0, 4);
			setState(3);
			startBck("fence_revolve_inner_roll_down");
			offMapObjFlag(MAP_OBJ_FLAG_UNK100);
			return TRUE;
		}
		if (isState(2)) {
			if (gpMSound->gateCheck(MSD_SE_OBJ_FENCE_REVERSE2))
				MSoundSESystem::MSoundSE::startSoundActor(
				    MSD_SE_OBJ_FENCE_REVERSE2, &mPosition, 0, nullptr, 0, 4);
			setState(4);
			startBck("fence_revolve_inner_roll_up");
			offMapObjFlag(MAP_OBJ_FLAG_UNK100);
			return TRUE;
		}
	}
	return FALSE;
}

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

extern "C" BOOL curAnmEndsNext__6MActorFiPc(MActor*, int, char*);
extern "C" void setFrameRate__6MActorFfi(MActor*, f32, int);
extern "C" J3DFrameCtrl* getFrameCtrl__6MActorFi(MActor*, int);
extern "C" void calc__6MActorFv(MActor*);

#pragma dont_inline on
void TRevolvingFenceInner::controlGroundRoof()
{
	int state = mState;
	if (state == 4)
		goto state46;
	if (state >= 4)
		goto high;
	if (state >= 3)
		goto state35;
	return;
high:
	if (state == 6)
		goto state46;
	if (state >= 6)
		return;
state35:
	if (curAnmEndsNext__6MActorFiPc(getMActor(), 0, nullptr)) {
		setState(2);
		setFrameRate__6MActorFfi(getMActor(), 0.0f, 0);
		getFrameCtrl__6MActorFi(getMActor(), 0)->setFrame(0.0f);
		calc__6MActorFv(getMActor());
		onMapObjFlag(MAP_OBJ_FLAG_UNK100);
	}
	return;
state46:
	if (curAnmEndsNext__6MActorFiPc(getMActor(), 0, nullptr)) {
		setState(1);
		setFrameRate__6MActorFfi(getMActor(), 0.0f, 0);
		getFrameCtrl__6MActorFi(getMActor(), 0)->setFrame(0.0f);
		calc__6MActorFv(getMActor());
		onMapObjFlag(MAP_OBJ_FLAG_UNK100);
	}
	char trash[0x10];
	trash[0] = 0;
}

// The extra inline leaves a dead 4-byte slot so the matrix sits at 0x38.
static inline MtxPtr fenceAnmMtx(TRevolvingFenceInner* self)
{
	return self->getModel()->getAnmMtx(0);
}

void TRevolvingFenceInner::setGroundCollision()
{
	if (SMS_GetYoshi()->isHatched()
	    && mPosition.x - mBodyRadius < SMS_GetYoshi()->getTranslation().x
	    && mPosition.x + mBodyRadius > SMS_GetYoshi()->getTranslation().x
	    && mPosition.z - mBodyRadius < SMS_GetYoshi()->getTranslation().z
	    && mPosition.z + mBodyRadius > SMS_GetYoshi()->getTranslation().z) {
		TMtx34f mtx;
		MtxPtr src = fenceAnmMtx(this);
		mtx.set(src);
		if (mMapCollisionManager->unk8)
			mMapCollisionManager->unk8->moveMtx(mtx);
	}
	TMapObjBase::setGroundCollision();
}

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

// dont_inline keeps the call in control().
#pragma dont_inline on
void TFenceWater::controlRotation()
{
	switch (mState) {
	case 1:
		// Empty. The low half of the switch still compares against 1.
		break;
	case 2:
		unk140 -= unk13C;
		if (unk140 <= -90.0f) {
			unk140       = -90.0f;
			unk13C       = 0.0f;
			mState       = 3;
			mStateTimer  = mTurnedWaitTime;
		}
		break;
	case 3:
		if (isStateTimerEngaged())
			break;
		if (gpMSound->gateCheck(MSD_SE_OBJ_WATER_FENCE_REV))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_WATER_FENCE_REV, &mPosition, 0, nullptr, 0, 4);
		unk13C = mBackSpeed;
		mState = 4;
		break;
	case 4:
		unk140 += unk13C;
		if (unk140 >= 0.0f)
			changeStatusToWait();
		break;
	}

	// Dead slot so the frame stays at -0x28 (r31 at r1+0x24).
	char trash[0x9];
	trash[0] = 0;
}
#pragma dont_inline off

void TFenceWater::control()
{
	TMapObjBase::control();
	controlRotation();

	// 182.04445 is 65536/360. Written out so JMASSin's dont_inline
	// does not turn the lookup into a call.
	f32 scale   = 182.04445f;
	f32 radius  = 500.0f;
	mRotation.y = MsWrap(unk140 + mInitialRotation.y, 0.0f, 360.0f);

	// TMapObjMessenger* at 0x144. void* keeps the second lwz.
	// Not a real member: adding it would change sizeof.
	void* base = reinterpret_cast<u8*>(this) + 0x144;
	int index  = static_cast<u16>(scale * mRotation.y) >> jmaSinShift;
	reinterpret_cast<THitActor**>(base)[0]->mPosition.x
	    = mPosition.x + radius * jmaCosTable[index];

	index = static_cast<u16>(scale * mRotation.y) >> jmaSinShift;
	reinterpret_cast<THitActor**>(base)[0]->mPosition.z
	    = mPosition.z - radius * jmaSinTable[index];

	// Dead slot so the fctiwz spills stay at r1+0x38 and r1+0x30.
	char trash[0x9];
	trash[0] = 0;
}

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

BOOL TRailFence::receiveMessage(THitActor*, u32 message)
{
	if (message == 3) {
		if (gpMSound->gateCheck(MSD_SE_OBJ_MVING_FENCT_PNCH))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_MVING_FENCT_PNCH, &mPosition, 0, nullptr, 0, 4);
		setUpMapCollision(1);
		offMapObjFlag(MAP_OBJ_FLAG_UNK100);
		mState = 2;
		char trash[1];
		trash[0] = 0;
		return TRUE;
	}
	return FALSE;
}

void TRailFence::falling()
{
	JGeometry::TVec3<f32> velocity = mVelocity;
	mPosition.y += velocity.y;
	mVelocity.y -= mGravity;
	if (mVelocity.y < -100.0f)
		mVelocity.y = -100.0f;
	if (mPosition.y < mInitialPosition.y - mFallHeight) {
		mPosition.x = mInitialPosition.x;
		mPosition.y = mInitialPosition.y;
		mPosition.z = mInitialPosition.z;
		setUpMapCollision(0);
		unk13C->setTo(
		    unk13C->unk0->findNearestNodeIndex(mPosition, 0xffffffff));
		makeObjAppeared();
		calcRootMatrix();
		getModel()->calc();
		onMapObjFlag(MAP_OBJ_FLAG_UNK100);
	}
}

// Dead inline, kept only for the stack slots it leaves behind (no code).
// The 0x40 local sits under the indexToPoint return temps; the 0x8 return
// sits above them. trash[0x8] then lands goal at r1+0x70 in a 0x88 frame.
struct TRailFenceBig {
	char c[0x40];
};
struct TRailFenceSmall {
	char c[0x8];
};
static inline TRailFenceSmall railFencePad()
{
	TRailFenceBig big;
	return *(TRailFenceSmall*)(void*)&big;
}

#pragma dont_inline on
void TRailFence::goOnRail()
{
	if (unk13C->unk0 == nullptr)
		return;

	railFencePad();
	JGeometry::TVec3<f32> goal
	    = unk13C->unk0->indexToPoint(unk13C->mCurrIdx);
	char trash[0x8];
	goal.sub(mPosition);
	if (goal.squared() < 50.0f) {
		TGraphTracer* tracer = unk13C;
		TRailNode* rail      = tracer->unk0->unk0[tracer->mCurrIdx].unk0;
		if (rail->mConnectionNum == 0 && (rail->mFlags & 8)) {
			if (gpMSound->gateCheck(MSD_SE_OBJ_MVING_FENCT_SET))
				MSoundSESystem::MSoundSE::startSoundActor(
				    MSD_SE_OBJ_MVING_FENCT_SET, &mPosition, 0, nullptr, 0,
				    4);
			mStateTimer = mWaitTime;
			startAnim(1);
			mState = 3;
			return;
		}
		// void* reload: a typed tracer->unk0 stays in r3 and this lwz disappears.
		// prev is named so its load is scheduled before that reload.
		int prev = tracer->mPrevIdx;
		tracer->moveTo((*(TGraphWeb**)(void*)tracer)
		                   ->getShortestNextIndex(tracer->mCurrIdx, prev,
		                                          0xffffffff));
		goal.set(unk13C->unk0->indexToPoint(unk13C->mCurrIdx));
	}
	if (gpMSound->gateCheck(MSD_SE_OBJ_MVING_FENCE_MOVE))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_MVING_FENCE_MOVE, &mPosition, 0, nullptr, 0, 4);
	VECNormalize(&goal, &goal);
	goal.scale(unk140);
	mLinearVelocity.add(goal);
}
#pragma dont_inline off

void TRailFence::control()
{
	TMapObjBase::control();
	switch (mState) {
	case 1:
		break;
	case 2:
		goOnRail();
		break;
	case 3:
		if (isStateTimerEngaged())
			break;
		removeMapCollision();
		if (gpMSound->gateCheck(MSD_SE_OBJ_SUPERBLOCK_BREAK))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_SUPERBLOCK_BREAK, &mPosition, 0, nullptr, 0, 4);
		mState = 4;
		break;
	case 4: {
		// falling() inlined. The out-of-line copy stays UNUSED.
		JGeometry::TVec3<f32> velocity = mVelocity;
		char trash[0x24];
		mPosition.y += velocity.y;
		mVelocity.y -= mGravity;
		if (mVelocity.y < -100.0f)
			mVelocity.y = -100.0f;
		if (mPosition.y < mInitialPosition.y - mFallHeight) {
			mPosition.x = mInitialPosition.x;
			mPosition.y = mInitialPosition.y;
			mPosition.z = mInitialPosition.z;
			setUpMapCollision(0);
			unk13C->setTo(unk13C->unk0->findNearestNodeIndex(mPosition,
			                                                 0xffffffff));
			makeObjAppeared();
			calcRootMatrix();
			getModel()->calc();
			onMapObjFlag(MAP_OBJ_FLAG_UNK100);
		}
		break;
	}
	}
}

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
