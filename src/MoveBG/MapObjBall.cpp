#include <MoveBG/MapObjBall.hpp>
#include <Player/MarioAccess.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <MarioUtil/MapUtil.hpp>
#include <MarioUtil/PacketUtil.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <Map/Map.hpp>
#include <Map/MapCollisionData.hpp>
#include <Map/PollutionManager.hpp>
#include <Camera/CubeManagerBase.hpp>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <System/Particles.hpp>
#include <Player/ModelWaterManager.hpp>
#include <M3DUtil/InfectiousStrings.hpp>
#include <System/FlagManager.hpp>
#include <System/MarDirector.hpp>
#include <Map/MapData.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <MoveBG/Item.hpp>
#include <MoveBG/ItemManager.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

u32 TResetFruit::mFruitLivingTime       = 14400;
f32 TResetFruit::mScaleUpSpeed          = 1.05f;
f32 TResetFruit::mRottingScaleSpeed     = 0.0f; // UNUSED, value not in the DOL
f32 TResetFruit::mBreakingScaleSpeed    = 0.96f;
u32 TResetFruit::mFruitWaitTimeToAppear = 360;
GXColorS10 TResetFruit::mRottenColor    = { 0, 0, 0, 0 }; // UNUSED

void TMapObjBall::touchRoof(JGeometry::TVec3<f32>* param_1)
{
	if (param_1->y > unk140)
		param_1->y = unk140;
	calcReflectingVelocity(unk13C, mMapObjData->mPhysical->unk4->unk4,
	                       &mVelocity);
}

// Empty here. dont_inline keeps the qualified calls in TBigWatermelon.
#pragma dont_inline on
void TMapObjBall::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }
#pragma dont_inline off

void TMapObjBall::touchPollution() { kill(); }

void TMapObjBall::touchWaterSurface() { kill(); }

void TMapObjBall::rebound(JGeometry::TVec3<f32>* param_1)
{
	calcReflectingVelocity(mGroundPlane, mMapObjData->mPhysical->unk4->unk4,
	                       &mVelocity);
	param_1->y = mGroundHeight;
	onLiveFlag(LIVE_FLAG_AIRBORNE);
	if (isActorType(0x400000D0)) {
		if (mScaling.y >= 5.0f)
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_WATERMELON_BBUND, &mPosition, nullptr,
			    fabsf(mGroundPlane->mNormal.y), 0, 0, nullptr, 0, 4);
		else
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_WATERMELON_SBUND, &mPosition, nullptr,
			    fabsf(mGroundPlane->mNormal.y), 0, 0, nullptr, 0, 4);
	} else {
		gpMSound->startSoundActorWithInfo(mMapObjData->mSound->unk4->unk0[4],
		                                  &mPosition, &mVelocity, 0.0f, 0, 0,
		                                  nullptr, 0, 4);
	}
	char trash[48];
	trash[47] = 0;
}

// One inline deep so sqrt stays a call. length() would expand it to frsqrte.
inline f32 doSqrt(f32 lenSq) { return JGeometry::TUtil<f32>::sqrt(lenSq); }

#pragma dont_inline on
void TMapObjBall::touchGround(JGeometry::TVec3<f32>* param_1)
{
	int gap;
	gap = 0;
	JGeometry::TVec3<f32> velocity = mVelocity;
	char trash[0x4c];
	trash[0] = 0;
	f32 speed = fabsf(doSqrt(velocity.squared()));
	if (speed > 0.05f && isActorType(0x400000D0)) {
		if (mScaling.y >= 5.0f)
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_WATERMELON_BROLL,
			                                  &mPosition, nullptr, speed, 0, 0,
			                                  nullptr, 0, 4);
		else
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_WATERMELON_SROLL,
			                                  &mPosition, nullptr, speed, 0, 0,
			                                  nullptr, 0, 4);
	}

	if (mGroundPlane->isWaterSurface()) {
		touchWaterSurface();
		param_1->x = mPosition.x;
		param_1->y = mPosition.y;
		param_1->z = mPosition.z;
		return;
	}

	if (gpPollution->isPolluted(param_1->x, param_1->y, param_1->z)) {
		touchPollution();
		param_1->x = mPosition.x;
		param_1->y = mPosition.y;
		param_1->z = mPosition.z;
		return;
	}

	if (mVelocity.y > -unk188) {
		offLiveFlag(LIVE_FLAG_AIRBORNE);
		mVelocity.y = 0.0f;
		param_1->y  = mGroundHeight;
	} else {
		rebound(param_1);
	}

	if (!isAirborne()) {
		mVelocity.x += unk180 * mGroundPlane->mNormal.x;
		mVelocity.z += unk180 * mGroundPlane->mNormal.z;
	}

	mVelocity.x *= mMapObjData->mPhysical->unk4->unk10;
	mVelocity.z *= mMapObjData->mPhysical->unk4->unk10;

	if (isActorType(0x400000D0)) {
		f32 limit = mMapObjData->mPhysical->unk4->unkC;
		if (fabsf(mVelocity.x) > limit || fabsf(mVelocity.z) > limit)
			gpMSound->startSoundActor(MSD_SE_MA_SLIP, &mPosition, 0, nullptr, 0,
			                          4);
	}
}
#pragma dont_inline off

void TMapObjBall::put()
{
	TMapObjGeneral::put();
	calcCurrentMtx();
}

// The extra inline leaves a dead 4-byte slot so the frame stays at -0x38.
static inline const JGeometry::TVec3<f32>& ballVelocity(const TMapObjBall* self)
{
	return self->mVelocity;
}

void TMapObjBall::hold(TTakeActor* actor)
{
	JGeometry::TVec3<f32> velocity = ballVelocity(this);
	if (!(doSqrt(velocity.squared()) > 10.0f)) {
		TMapObjGeneral::hold(actor);
		mVelocity.x = mVelocity.y = mVelocity.z = 0.0f;
	}
}

void TMapObjBall::kicked() { }

u32 TMapObjBall::touchWater(THitActor* actor)
{
	char trash[4];
	trash[0] = 0;
	if (isState(TMapObjGeneral::STATE_HOLDING)
	    || isState(TMapObjGeneral::STATE_APPEARING))
		return 1;

	JGeometry::TVec3<f32> work;
	work.set(JGeometry::TVec3<f32>(mVelocity));
	const JGeometry::TVec3<f32>& water = getWaterSpeed(actor);
	register f32 w = water.x;
	register f32 s = unk17C;
	work.x = w * s + work.x;
	w = water.y;
	work.y = w * s + work.y;
	w = water.z;
	work.z = w * s + work.z;
	mVelocity = work;
	offLiveFlag(LIVE_FLAG_UNK10);
	return 1;
}

#pragma dont_inline on
void TMapObjBall::boundByActor(THitActor*) { }
#pragma dont_inline off

// dont_inline keeps the qualified call in TResetFruit::touchActor.
#pragma dont_inline on
void TMapObjBall::touchActor(THitActor* actor)
{
	if ((s32)unk194 != 0)
		return;
	if (isState(TMapObjGeneral::STATE_HOLDING))
		return;
	if (isHideObj(actor))
		return;
	if (actor->isActorType(0x08000083) || actor->isActorType(0x400000CA)
	    || actor->isActorType(0x400000CC))
		return;
	if (actor->isActorType(0x80000001) && !isActorType(0x400000D0)
	    && SMS_GetMarioSpeedY() != 0.0f) {
		kicked();
		return;
	}
	boundByActor(actor);
}
#pragma dont_inline off

void TMapObjBall::calcCurrentMtx() { }

// Inlined so the wall record is allocated above the position vector.
static inline void checkBallWall(TMapObjBall* self,
                                 JGeometry::TVec3<f32>* param_1)
{
	// Dead slot so the position sits at r1+0x28 and the frame stays at -0x68.
	char pad[0x10];
	f32 radius;
	JGeometry::TVec3<f32> pos;
	pad[0] = 0;
	pos.x  = param_1->x;
	// Radius is captured in the add so Y loads first and f1 keeps it.
	pos.y = param_1->y + (radius = self->mBodyRadius);
	pos.z  = param_1->z;

	TBGWallCheckRecord check(pos, radius, 4,
	                         self->mMapObjData->mPhysical->mWallCheckFlags);
	if (gpMap->isTouchedWallsAndMoveXZ(&check)) {
		self->unk138 = check.mResultWalls[0];
		param_1->x   = pos.x;
		param_1->z   = pos.z;
		self->touchWall(param_1, &check);
	} else {
		self->unk138 = nullptr;
	}
}

void TMapObjBall::checkWallCollision(JGeometry::TVec3<f32>* param_1)
{
	checkBallWall(this, param_1);
}

void TMapObjBall::makeObjDefault()
{
	TMapObjBase::makeObjDefault();
	MtxPtr mtx = getModel()->getAnmMtx(0);
	mtx[0][3] = mPosition.x;
	mtx[1][3] = mPosition.y + mBodyRadius;
	mtx[2][3] = mPosition.z;

	// Dead slot so MWCC keeps frame -0x28.
	char trash[8];
	trash[0] = 0;
}

void TMapObjBall::makeObjAppeared()
{
	TMapObjBase::makeObjAppeared();
	calcCurrentMtx();
	MtxPtr mtx  = getModel()->getAnmMtx(0);
	mtx[0][3]   = mPosition.x;
	mtx[1][3]   = mPosition.y + mBodyRadius;
	mtx[2][3]   = mPosition.z;
	if (isActorType(0x40000394) && mtx[1][1] > 0.0f)
		mtx[1][3] -= 50.0f * mtx[1][1];
	if (isActorType(0x40000392))
		mtx[1][3] -= 10.0f * (1.0f - mtx[1][1]);
	unkE8 = 0;

	char trash[8];
	trash[0] = 0;
}

// Out of line in TResetFruit::control (states 6 and 0xB). States 2/3
// duplicate this body instead, so the call must not be inlined.
#pragma dont_inline on
void TMapObjBall::control()
{
	TMapObjGeneral::control();
	if ((s32)unk194 != 0)
		unk194 -= 1;
	if (isState(TMapObjGeneral::STATE_HOLDING)) {
		Mtx mtx;
		MTXCopy(mHolder->getTakingMtx(), mtx);
		mtx[1][3] += unk190;
		MTXCopy(mtx, getModel()->getAnmMtx(0));
	} else {
		JGeometry::TVec3<f32> vel = mVelocity;
		if (!(vel.squared() <= JGeometry::TUtil<f32>::epsilon()
		      && mGroundPlane->mActor == nullptr))
			calcCurrentMtx();
	}
	// Dead slot so MWCC keeps the frame at -0x70.
	char trash[0x14];
	trash[0] = 0;
}
#pragma dont_inline off

BOOL TMapObjBall::receiveMessage(THitActor* sender, u32 message)
{
	if (TMapObjGeneral::receiveMessage(sender, message))
		return TRUE;
	if (message == HIT_MESSAGE_TAKE
	    && checkMapObjFlag(MAP_OBJ_FLAG_UNK100000)) {
		hold((TTakeActor*)sender);
		return TRUE;
	}
	if (sender->isActorType(0x80000001) && !isActorType(0x400000D0)
	    && message != HIT_MESSAGE_TAKE) {
		kicked();
		return TRUE;
	}
	return FALSE;
}

#pragma dont_inline on
void TMapObjBall::initMapObj()
{
	TMapObjGeneral::initMapObj();
	mInitialScaling.set(mScaling);

	switch (mActorType) {
	case 0x400000D0:
		unk14C = 4.0f;
		unk150 = 0.0f;
		unk154 = 0.0f;
		unk158 = 0.15f;
		unk15C = 0.0f;
		unk160 = 0.9f;
		unk164 = 0.06f;
		unk168 = 1.5f;
		unk16C = 0.5f;
		unk170 = 0.5f;
		unk174 = 0.2f;
		unk178 = 2.5f;
		unk17C = 0.001f;
		unk180 = 0.3f;
		unk184 = 1.5f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = mBodyRadius / 3.0f;
		break;
	case 0x40000064:
		unk148 = 0.6f;
		unk14C = 2.0f;
		unk150 = 0.02f;
		unk154 = 0.0f;
		unk158 = 0.055f;
		unk15C = 0.02f;
		unk160 = 0.83f;
		unk170 = 0.9f;
		unk174 = 0.13f;
		unk178 = 20.0f;
		unk164 = 0.5f;
		unk168 = 0.02f;
		unk16C = 0.5f;
		unk17C = 1.2f;
		unk180 = 0.8f;
		unk184 = 1.0f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = mBodyRadius / 3.0f;
		break;
	case 0x40000393:
		unk148 = 0.6f;
		unk14C = 0.2f;
		unk150 = 1.3f;
		unk154 = 15.0f;
		unk158 = 0.5f;
		unk15C = 1.3f;
		unk160 = 1.0f;
		unk170 = 0.9f;
		unk174 = 0.13f;
		unk178 = 20.0f;
		unk164 = 2.0f;
		unk168 = 0.02f;
		unk16C = 0.3f;
		unk17C = 0.05f;
		unk180 = 0.5f;
		unk184 = 1.0f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = 50.0f;
		break;
	case 0x40000390:
	case 0x40000391:
	case 0x40000392:
		unk148 = 0.4f;
		unk14C = 0.2f;
		unk150 = 1.3f;
		unk154 = 0.0f;
		unk158 = 1.2f;
		unk15C = 0.8f;
		unk160 = 0.5f;
		unk170 = 0.9f;
		unk174 = 0.13f;
		unk178 = 20.0f;
		unk164 = 2.0f;
		unk168 = 0.02f;
		unk16C = 0.3f;
		unk17C = 0.05f;
		unk180 = 0.5f;
		unk184 = 1.0f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = 50.0f;
		break;
	case 0x40000394:
		unk148 = 0.2f;
		unk14C = 0.0f;
		unk150 = 0.0f;
		unk154 = 0.0f;
		unk158 = 0.0f;
		unk15C = 0.0f;
		unk160 = 0.0f;
		unk170 = 0.0f;
		unk174 = 0.0f;
		unk178 = 0.0f;
		unk164 = 0.0f;
		unk168 = 0.0f;
		unk16C = 0.0f;
		unk17C = 0.05f;
		unk180 = 0.5f;
		unk184 = 1.0f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = 50.0f;
		break;
	case 0x40000395:
		unk148 = 0.4f;
		unk14C = 0.2f;
		unk150 = 1.3f;
		unk154 = 0.0f;
		unk158 = 1.2f;
		unk15C = 0.8f;
		unk160 = 0.5f;
		unk170 = 0.9f;
		unk174 = 0.13f;
		unk178 = 20.0f;
		unk164 = 2.0f;
		unk168 = 0.02f;
		unk16C = 0.3f;
		unk17C = 0.05f;
		unk180 = 0.5f;
		unk184 = 1.0f;
		unk188 = 1.5f;
		mBodyRadius = 50.0f * mScaling.y;
		unk18C      = 50.0f;
		break;
	}

	if (isActorType(0x40000393)) {
		mBodyRadius = 45.0f * mScaling.y;
		unk190      = mBodyRadius;
	}
	if (isActorType(0x40000390)) {
		mBodyRadius = 40.0f * mScaling.y;
		unk190      = 20.0f;
	}
	if (isActorType(0x40000391)) {
		mBodyRadius = 40.0f * mScaling.y;
		unk190      = 20.0f;
	}
	if (isActorType(0x40000392))
		unk190 = 10.0f;
}
#pragma dont_inline off

TMapObjBall::TMapObjBall(const char* name)
    : TMapObjGeneral(name)
    , unk148(0.0f)
    , unk14C(0.0f)
    , unk150(0.0f)
    , unk154(0.0f)
    , unk158(0.0f)
    , unk15C(0.0f)
    , unk160(0.0f)
    , unk164(0.0f)
    , unk168(0.0f)
    , unk16C(0.0f)
    , unk170(0.0f)
    , unk174(0.0f)
    , unk178(0.0f)
    , unk17C(0.0f)
    , unk180(0.0f)
    , unk184(0.0f)
    , unk188(0.0f)
    , unk18C(0.0f)
    , unk190(0.0f)
    , unk194(0)
{
	mInitialScaling.zero();
}

void TResetFruit::checkGroundCollision(JGeometry::TVec3<f32>* pos)
{
	char trash[0x30];
	u8 map = gpMarDirector->mMap;
	if (map != 7 && map != 4) {
		TMapObjGeneral::checkGroundCollision(pos);
		return;
	}

	if (map == 4) {
		mGroundHeight = gpMap->checkGround(pos->x, pos->y + 200.0f, pos->z,
		                                    &mGroundPlane);
		mGroundHeight += 1.0f;
		if (pos->y <= mGroundHeight)
			touchGround(pos);
		else
			onLiveFlag(LIVE_FLAG_AIRBORNE);
		return;
	}

	mGroundHeight = gpMap->checkGround(pos->x, pos->y + mHeadHeight, pos->z,
	                                    &mGroundPlane);
	bool phaseThrough;
	if (mGroundPlane->mBGType
	        == BG_TYPE_EVERYTHING_BUT_MAP_OBJECTS_PHASE_THROUGH
	    || mGroundPlane->mBGType == BG_TYPE_MAP_CHANGE_PHASE_THROUGH)
		phaseThrough = true;
	else
		phaseThrough = false;
	if (phaseThrough) {
		mGroundHeight = gpMap->checkGroundExactY(
		    pos->x, mGroundHeight - 200.0f, pos->z, &mGroundPlane);
	}
	mGroundHeight += 1.0f;
	if (pos->y <= mGroundHeight)
		touchGround(pos);
	else
		onLiveFlag(LIVE_FLAG_AIRBORNE);
}

void TResetFruit::waitingToAppear()
{
	if (gpMarDirector->mMap == 3 && unk1A4 != 0)
		makeObjDead();
	if (checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000))
		return;
	if (isStateTimerEngaged())
		return;
	if (mColCount != 0)
		return;

	onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	makeObjAppeared();

	Mtx scaleMtx;
	MTXScale(scaleMtx, 0.2f, 0.2f, 0.2f);
	MtxPtr anmMtx = getModel()->getAnmMtx(0);
	concatOnlyRotFromLeft(scaleMtx, getModel()->getAnmMtx(0), anmMtx);
	mScaling.y = 0.2f;
	onHitFlag(HIT_FLAG_NO_COLLISION);
	mState = TMapObjGeneral::STATE_APPEARING;
	if (gpMSound->gateCheck(MSD_SE_IT_COMMON_APPEAR))
		MSoundSESystem::MSoundSE::startSoundActor(MSD_SE_IT_COMMON_APPEAR,
		                                          &mPosition, 0, nullptr, 0,
		                                          4);
	// Dead slot so MWCC keeps the matrix at r1+0x3C and the frame at -0x78.
	char trash[0x20];
	trash[0] = 0;
}

void TResetFruit::makeObjWaitingToAppear()
{
	char trash[0x10];
	trash[0] = 0;
	mState = 11;
	makeObjDefault();
	makeObjDead();
	calcRootMatrix();
	getModel()->calc();
	mStateTimer = mFruitWaitTimeToAppear;
	offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
	if (gpMarDirector->mMap == 3 && unk1A4 != 0)
		makeObjDead();
}

void TResetFruit::thrown()
{
	TMapObjGeneral::thrown();
	mState = 11;
}

void TResetFruit::hold(TTakeActor* actor)
{
	JGeometry::TVec3<f32> vel = mVelocity;
	// Dead slot so MWCC keeps the frame at -0x38.
	char trash[4];
	trash[0] = 0;
	if (!(doSqrt(vel.squared()) > 10.0f)) {
		TMapObjGeneral::hold(actor);
		mVelocity.x = mVelocity.y = mVelocity.z = 0.0f;
	}
	mVelocity.x = mVelocity.y = mVelocity.z = 0.0f;
	onLiveFlag(LIVE_FLAG_UNK10);
	if (!checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000)) {
		if (!isStateTimerEngaged()) {
			onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
			mStateTimer = getLivingTime();
		}
	}
}

void TResetFruit::touchPollution()
{
	// Dead slot so MWCC keeps the frame at -0x38.
	char trash[0x20];
	trash[0] = 0;
	gpMarioParticleManager->emitAndBindToPosPtr(PARTICLE_MS_MOE_FIRE_OFF,
	                                            &mPosition, 0, nullptr);
	if (gpMSound->gateCheck(MSD_SE_OBJ_AWAY_INTO_GRAF))
		MSoundSESystem::MSoundSE::startSoundActor(MSD_SE_OBJ_AWAY_INTO_GRAF,
		                                          &mPosition, 0, nullptr, 0, 4);
	makeObjDefault();
	mState = 11;
	makeObjDefault();
	makeObjDead();
	calcRootMatrix();
	getModel()->calc();
	mStateTimer = mFruitWaitTimeToAppear;
	offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
	if (gpMarDirector->mMap == 3 && unk1A4 != 0)
		makeObjDead();
}

void TResetFruit::touchWaterSurface()
{
	// Dead slot so MWCC keeps the frame at -0x30.
	char trash[0x18];
	trash[0] = 0;
	emitColumnWater();
	if (gpMSound->gateCheck(MSD_SE_OBJ_DRINA_TO_WATER))
		MSoundSESystem::MSoundSE::startSoundActor(MSD_SE_OBJ_DRINA_TO_WATER,
		                                          &mPosition, 0, nullptr, 0, 4);
	mState = 11;
	makeObjDefault();
	makeObjDead();
	calcRootMatrix();
	getModel()->calc();
	mStateTimer = mFruitWaitTimeToAppear;
	offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
	if (gpMarDirector->mMap == 3 && unk1A4 != 0)
		makeObjDead();
}

u32 TResetFruit::touchWater(THitActor* actor)
{
	char trash[4];
	trash[0] = 0;
	if (!isState(TMapObjGeneral::STATE_HOLDING)
	    && !isState(TMapObjGeneral::STATE_APPEARING)) {
		JGeometry::TVec3<f32> tmp(mVelocity);
		JGeometry::TVec3<f32> work;
		work.set(tmp);
		const JGeometry::TVec3<f32>& water = getWaterSpeed(actor);
		register f32 w                      = water.x;
		register f32 s                      = unk17C;
		work.x = w * s + work.x;
		w      = water.y;
		work.y = w * s + work.y;
		w      = water.z;
		work.z = w * s + work.z;
		mVelocity = work;
		offLiveFlag(LIVE_FLAG_UNK10);
	}
	if (!isStateTimerEngaged()) {
		onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
		mStateTimer = getLivingTime();
	}
	offLiveFlag(LIVE_FLAG_UNK10);
	mState = 11;
	return 1;
}

void TResetFruit::touchActor(THitActor* param_1)
{
	// Dead slot so MWCC keeps the frame at -0x28.
	char trash[0xC];
	trash[0] = 0;
	if (isState(TMapObjGeneral::STATE_APPEARING))
		return;
	if (isState(TMapObjGeneral::STATE_BREAKING))
		return;
	if (isState(0xC))
		return;
	if (!isState(TMapObjGeneral::STATE_WAITING_TO_APPEAR)) {
		TMapObjBall::touchActor(param_1);
		if (checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000))
			return;
		if (!isState(1))
			return;
		if (checkLiveFlag(LIVE_FLAG_UNK10))
			return;
		if (!isStateTimerEngaged()) {
			onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
			mStateTimer = getLivingTime();
		}
		offLiveFlag(LIVE_FLAG_UNK10);
		mState = 11;
	}
}

void TResetFruit::touchGround(JGeometry::TVec3<f32>* param_1)
{
	// Dead slot so MWCC keeps the frame at -0x30.
	char trash[0x10];
	trash[0] = 0;
	if (mGroundPlane->isDeathPlane()) {
		mState = 11;
		makeObjDefault();
		makeObjDead();
		calcRootMatrix();
		getModel()->calc();
		mStateTimer = mFruitWaitTimeToAppear;
		offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
		mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
		if (gpMarDirector->mMap == 3 && unk1A4 != 0)
			makeObjDead();
		param_1->x = mPosition.x;
		param_1->y = mPosition.y;
		param_1->z = mPosition.z;
	} else {
		TMapObjBall::touchGround(param_1);
	}
}

void TResetFruit::makeObjLiving()
{
	if (!isStateTimerEngaged()) {
		onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
		mStateTimer = getLivingTime();
	}
	offLiveFlag(LIVE_FLAG_UNK10);
	mState = 11;
}

// UNUSED
void TResetFruit::pick(THitActor*) { }

void TResetFruit::kicked() { }

// UNUSED
void TResetFruit::living() { }

// UNUSED
void TResetFruit::waitEffect() { }

// UNUSED
void TResetFruit::rotting() { }

void TResetFruit::breaking()
{
	Mtx scaleMtx;
	// Dead slot so MWCC keeps the matrix at r1+0x24 and the frame at -0x60.
	char trash[10];
	trash[0] = 0;
	MTXScale(scaleMtx, 1.0f, mBreakingScaleSpeed, 1.0f);

	MtxPtr anmMtx = getModel()->getAnmMtx(0);
	concatOnlyRotFromLeft(scaleMtx, anmMtx, anmMtx);

	mScaling.y *= mBreakingScaleSpeed;
	anmMtx[1][3] = mBodyRadius * mScaling.y + mPosition.y;

	if (mScaling.y < 0.2f) {
		f32 half = 0.5f;
		mPosition.y += mBodyRadius * half;
		mScaling.x  = mInitialScaling.x;
		mScaling.y  = mInitialScaling.y;
		mScaling.z  = mInitialScaling.z;
		emitAndScale(PARTICLE_MS_ENM_DISAP_A_W, 0, &mPosition);
		if (gpMSound->gateCheck(MSD_SE_SMOKE_EFFECT))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_SMOKE_EFFECT, &mPosition, 0, nullptr, 0, 4);
		mStateTimer = 0xF0;
		sleep();
		mState = 0xD;
	}
}

void TResetFruit::appearing()
{
	Mtx scaleMtx;
	// Dead slot so MWCC keeps the matrix at r1+0x20 and the frame at -0x58.
	char trash[10];
	trash[0] = 0;
	MTXScale(scaleMtx, mScaleUpSpeed, mScaleUpSpeed, mScaleUpSpeed);

	MtxPtr anmMtx = getModel()->getAnmMtx(0);
	concatOnlyRotFromLeft(scaleMtx, anmMtx, anmMtx);

	mScaling.y *= mScaleUpSpeed;
	mScaledBodyRadius = mBodyRadius * mScaling.y;
	anmMtx[1][3]       = mBodyRadius * mScaling.y + mPosition.y;

	if (mScaling.y >= mInitialScaling.y) {
		mScaling.x = mInitialScaling.x;
		mScaling.y = mInitialScaling.y;
		mScaling.z = mInitialScaling.z;
		getModel()->calc();
		offHitFlag(HIT_FLAG_NO_COLLISION);
		makeObjAppeared();
		mState = 1;
	}
}

void TResetFruit::control()
{
	// Dead slots so the matrix lands at r1+0x90, the velocity at r1+0xC0,
	// and the frame stays 0xF8.
	char gap[0x14];
	JGeometry::TVec3<f32> vel;
	Mtx mtx;
	char pad[0x54];
	gap[0] = 0;
	pad[0] = 0;

	switch (mState) {
	case TMapObjBase::STATE_NORMAL:
		offHitFlag(HIT_FLAG_NO_COLLISION);
		for (int i = 0; i < mColCount; ++i) {
			THitActor* col = mCollisions[i];
			if (isState(TMapObjGeneral::STATE_APPEARING))
				continue;
			if (isState(TMapObjGeneral::STATE_BREAKING))
				continue;
			if (isState(0xC))
				continue;
			if (isState(TMapObjGeneral::STATE_WAITING_TO_APPEAR))
				continue;
			TMapObjBall::touchActor(col);
			if (checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000))
				continue;
			if (!isState(1))
				continue;
			if (checkLiveFlag(LIVE_FLAG_UNK10))
				continue;
			if (!isStateTimerEngaged()) {
				onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
				mStateTimer = getLivingTime();
			}
			offLiveFlag(LIVE_FLAG_UNK10);
			mState = 11;
		}
		if (mGroundPlane->getActor() != nullptr)
			calcCurrentMtx();
		break;
	case 0xB:
		offHitFlag(HIT_FLAG_NO_COLLISION);
		if (gpMarDirector->mMap == 4 && checkLiveFlag(LIVE_FLAG_UNK10))
			offLiveFlag(LIVE_FLAG_UNK10);
		if (mGroundPlane->getActor() != nullptr) {
			if (checkLiveFlag(LIVE_FLAG_UNK10))
				offLiveFlag(LIVE_FLAG_UNK10);
			const TLiveActor* actor = mGroundPlane->getActor();
			if (mPosition.y < 200.0f + mGroundHeight
			    && (actor->isActorType(0x400000CD)
			        || actor->isActorType(0x400000CD))) {
				f32 prev = unk198;
				unk198   = SMS_GetSandRiseUpRatio(actor);
				if (unk198 > 0.05f && unk198 > prev)
					mVelocity.y += 20.0f;
			}
		} else {
			unk198 = 0.0f;
		}
		TMapObjBall::control();
		if (!checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000)
		    && !isStateTimerEngaged()) {
			if (mHolder != nullptr) {
				mHolder->receiveMessage(this, HIT_MESSAGE_UNK8);
				mHolder->mHeldObject = nullptr;
				mHolder               = nullptr;
			}
			mVelocity.set(0.0f, 0.0f, 0.0f);
			mState = 0xC;
		}
		break;
	case TMapObjGeneral::STATE_HOLDING:
		TMapObjBall::control();
		if (!checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000)
		    && !isStateTimerEngaged()) {
			if (mHolder != nullptr) {
				mHolder->receiveMessage(this, HIT_MESSAGE_UNK8);
				mHolder->mHeldObject = nullptr;
				mHolder               = nullptr;
			}
			mVelocity.set(0.0f, 0.0f, 0.0f);
			mState = 0xC;
		}
		break;
	case TMapObjGeneral::STATE_APPEARING:
	case TMapObjGeneral::STATE_BREAKING:
		TMapObjGeneral::control();
		if ((s32)unk194 != 0)
			unk194 -= 1;
		if (isState(TMapObjGeneral::STATE_HOLDING)) {
			MTXCopy(mHolder->getTakingMtx(), mtx);
			mtx[1][3] += unk190;
			MTXCopy(mtx, getModel()->getAnmMtx(0));
		} else {
			vel = mVelocity;
			if (!(vel.squared() <= JGeometry::TUtil<f32>::epsilon()
			      && mGroundPlane->mActor == nullptr))
				calcCurrentMtx();
		}
		break;
	case 0xC: {
		f32 half = 0.5f;
		mPosition.y += mBodyRadius * half;
		mScaling.x = mInitialScaling.x;
		mScaling.y = mInitialScaling.y;
		mScaling.z = mInitialScaling.z;
		emitAndScale(PARTICLE_MS_ENM_DISAP_A_W, 0, &mPosition);
		if (gpMSound->gateCheck(MSD_SE_SMOKE_EFFECT))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_SMOKE_EFFECT, &mPosition, 0, nullptr, 0, 4);
		mStateTimer = 0xF0;
		sleep();
		mState = 0xD;
		break;
	}
	case 0xD:
		if (!isStateTimerEngaged()) {
			unk19C.r = 0xFF;
			unk19C.g = 0xFF;
			unk19C.b = 0xFF;
			awake();
			mState = 0xB;
			makeObjDefault();
			makeObjDead();
			calcRootMatrix();
			getModel()->calc();
			mStateTimer = mFruitWaitTimeToAppear;
			offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
			mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
			if (gpMarDirector->mMap == 3 && unk1A4 != 0)
				makeObjDead();
		}
		break;
	}
}

void TResetFruit::perform(u32 cue, JDrama::TGraphics* graphics)
{
	// Dead slot so the velocity copy lands at r1+0x54 and the frame stays 0x70.
	JGeometry::TVec3<f32> vel;
	char pad[0x34];

	if (gpMarDirector->mMap == 7) {
		if (isState(TMapObjGeneral::STATE_HOLDING)
		    || !((vel = mVelocity).isZero())) {
			if (checkLiveFlag(LIVE_FLAG_UNK200))
				offLiveFlag(LIVE_FLAG_UNK200);
		} else if (!gpCubeArea->isInAreaCube(mPosition) && isState(0xB)
		           && (mPosition.x != mInitialPosition.x
		               || mPosition.z != mInitialPosition.z)) {
			mState = 0xB;
			makeObjDefault();
			makeObjDead();
			calcRootMatrix();
			getModel()->calc();
			mStateTimer = mFruitWaitTimeToAppear;
			offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
			mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
			if (gpMarDirector->mMap == 3 && unk1A4 != 0)
				makeObjDead();
			return;
		}
	}

	TMapObjGeneral::perform(cue, graphics);
}

void TResetFruit::killByTimer(int param_1)
{
	mStateTimer = param_1;
	onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	mState = 11;
}

void TResetFruit::makeObjAppeared()
{
	if (checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000))
		makeObjDefault();

	TMapObjBase::makeObjAppeared();
	calcCurrentMtx();
	MtxPtr mtx = getModel()->getAnmMtx(0);
	mtx[0][3]  = mPosition.x;
	mtx[1][3]  = mPosition.y + mBodyRadius;
	mtx[2][3]  = mPosition.z;
	if (isActorType(0x40000394) && mtx[1][1] > 0.0f)
		mtx[1][3] -= 50.0f * mtx[1][1];
	if (isActorType(0x40000392))
		mtx[1][3] -= 10.0f * (1.0f - mtx[1][1]);
	unkE8 = 0;
	if (checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000))
		mState = 11;

	// Dead slot so MWCC keeps the frame at -0x28.
	char trash[8];
	trash[0] = 0;
}

BOOL TResetFruit::receiveMessage(THitActor* sender, u32 message)
{
	char trash[0x30];
	if (message == HIT_MESSAGE_UNKB) {
		if (isState(1) || isState(TMapObjGeneral::STATE_HOLDING)
		    || isState(11)) {
			mState = 11;
			makeObjDefault();
			makeObjDead();
			calcRootMatrix();
			getModel()->calc();
			mStateTimer = mFruitWaitTimeToAppear;
			offMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
			mState = TMapObjGeneral::STATE_WAITING_TO_APPEAR;
			if (gpMarDirector->mMap == 3 && unk1A4 != 0)
				makeObjDead();
			return true;
		}
		return false;
	}

	if (message == HIT_MESSAGE_UNKD) {
		kill();
		return true;
	}

	if (isState(1) || isState(TMapObjGeneral::STATE_HOLDING)
	    || isState(11)) {
		if (!isState(TMapObjGeneral::STATE_APPEARING)
		    && !isState(TMapObjGeneral::STATE_BREAKING) && !isState(0xC)
		    && !isState(TMapObjGeneral::STATE_WAITING_TO_APPEAR)) {
			TMapObjBall::touchActor(sender);
			if (!checkMapObjFlag(MAP_OBJ_FLAG_UNK4000000)
			    && isState(1) && !checkLiveFlag(LIVE_FLAG_UNK10)) {
				if (!isStateTimerEngaged()) {
					onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
					mStateTimer = getLivingTime();
				}
				offLiveFlag(LIVE_FLAG_UNK10);
				mState = 11;
			}
		}

		BOOL ret = TMapObjGeneral::receiveMessage(sender, message);
		if (ret)
			ret = true;
		else if (message == HIT_MESSAGE_TAKE
		         && checkMapObjFlag(MAP_OBJ_FLAG_UNK100000)) {
			hold((TTakeActor*)sender);
			ret = true;
		} else if (sender->isActorType(0x80000001)
		           && !isActorType(0x400000D0)
		           && message != HIT_MESSAGE_TAKE) {
			kicked();
			ret = true;
		} else {
			ret = false;
		}

		if (message == HIT_MESSAGE_PUT && isState(1))
			mState = 11;
		return ret;
	}
	return false;
}

void TResetFruit::initMapObj()
{
	TMapObjBall::initMapObj();
	SMS_InitPacket_OneTevColor(getModel(), 0, GX_TEVREG0,
	                           (const GXColorS10*)&unk19C);
}

TResetFruit::TResetFruit(const char* name)
    : TMapObjBall(name)
    , unk198(0.0f)
    , unk1A4(0)
{
	unk19C.r = 0xFF;
	unk19C.g = 0xFF;
	unk19C.b = 0xFF;
	unk19C.a = 0xFF;
}

void TRandomFruit::initMapObj()
{
	int kind = rand() * (1.0f / (RAND_MAX + 1)) * 5.0f;
	switch (kind) {
	case 0:
		snprintf((char*)unk1A8, 0x20, "FruitCoconut");
		break;
	case 1:
		snprintf((char*)unk1A8, 0x20, "FruitDurian");
		break;
	case 2:
		snprintf((char*)unk1A8, 0x20, "FruitPapaya");
		break;
	case 3:
		snprintf((char*)unk1A8, 0x20, "FruitPine");
		break;
	case 4:
	case 5:
	default:
		snprintf((char*)unk1A8, 0x20, "FruitPine");
		break;
	}
	unkF4 = (char*)unk1A8;
	TMapObjBall::initMapObj();
	SMS_InitPacket_OneTevColor(getModel(), 0, GX_TEVREG0,
	                           (const GXColorS10*)&unk19C);
}

TRandomFruit::TRandomFruit(const char* name)
    : TResetFruit(name)
{
	memset(unk1A8, 0, sizeof(unk1A8));
}

void MsMtxSetXYZRPH(MtxPtr mtx, f32 x, f32 y, f32 z, s16 r, s16 p, s16 h);

static inline void coverSetPos(JGeometry::TVec3<f32>& p, f32 x, f32 y, f32 z)
{
	p.x = x;
	p.y = y;
	p.z = z;
}

void TCoverFruit::calcRootMatrix()
{
	char trash[8];
	trash[0] = 0;
	if (mHolder != nullptr) {
		MtxPtr mtx = mHolder->getTakingMtx();
		getModel()->setBaseTRMtx(mtx);
		coverSetPos(mPosition, mtx[0][3], mtx[1][3], mtx[2][3]);
	} else {
		f32 x, y, z, rx, ry, rz;
		rz = mRotation.z;
		ry = mRotation.y;
		rx = mRotation.x;
		z  = mPosition.z;
		y  = mPosition.y - mYOffset;
		x  = mPosition.x;
		J3DModel* model = getModel();
		MsMtxSetXYZRPH(model->getBaseTRMtx(), x, y, z,
		               (s16)(rx * (65536.0f / 360.0f)),
		               (s16)(ry * (65536.0f / 360.0f)),
		               (s16)(rz * (65536.0f / 360.0f)));
	}
	getModel()->setBaseScale(mScaling);
}

BOOL TCoverFruit::receiveMessage(THitActor* sender, u32 message)
{
	if (sender->isActorType(0x08000083) && message == HIT_MESSAGE_TAKE) {
		onHitFlag(HIT_FLAG_NO_COLLISION);
		mHolder = (TTakeActor*)sender;
		return TRUE;
	}
	if (message == HIT_MESSAGE_UNKB) {
		kill();
		TFlagManager::smInstance->setBool(true, 0x1038B);
		return TRUE;
	}
	return FALSE;
}

void TCoverFruit::loadAfter()
{
	TMapObjBase::loadAfter();
	if (TFlagManager::smInstance->getBool(0x1038B))
		makeObjDead();
}

void TBigWatermelon::touchWaterSurface()
{
	emitColumnWater();
	if (gpMSound->gateCheck(MSD_SE_OBJ_DRINA_TO_WATER))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_DRINA_TO_WATER, &mPosition, 0, nullptr, 0, 4);
	kill();
	char trash[1];
	trash[0] = 0;
}

void TBigWatermelon::touchWall(JGeometry::TVec3<f32>* param_1,
                               TBGWallCheckRecord* param_2)
{
	TMapObjBall::touchWall(param_1, param_2);
}

void TBigWatermelon::rebound(JGeometry::TVec3<f32>* param_1)
{
	// Dead slot so MWCC keeps the frame at -0x68.
	char trash[0x38];
	trash[0] = 0;
	if (isState(0xC)) {
		kill();
		*param_1 = mPosition;
		return;
	}
	calcReflectingVelocity(mGroundPlane, mMapObjData->mPhysical->unk4->unk4,
	                       &mVelocity);
	param_1->y = mGroundHeight;
	onLiveFlag(LIVE_FLAG_AIRBORNE);
	if (isActorType(0x400000D0)) {
		if (mScaling.y >= 5.0f)
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_WATERMELON_BBUND, &mPosition, nullptr,
			    fabsf(mGroundPlane->mNormal.y), 0, 0, nullptr, 0, 4);
		else
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_WATERMELON_SBUND, &mPosition, nullptr,
			    fabsf(mGroundPlane->mNormal.y), 0, 0, nullptr, 0, 4);
	} else {
		gpMSound->startSoundActorWithInfo(mMapObjData->mSound->unk4->unk0[4],
		                                  &mPosition, &mVelocity, 0.0f, 0, 0,
		                                  nullptr, 0, 4);
	}
	if (isState(0xB))
		mState = 0xC;
}

void TBigWatermelon::touchGround(JGeometry::TVec3<f32>* param_1)
{
	TMapObjBall::touchGround(param_1);
}

class TPoiHana {
public:
	bool isMoving();
};

// Separate squares so fp_contract does not fuse the adds into fmadds.
// A class-method inline schedules the subs; a cpp static does not.
struct TMelonSum {
	static f32 calc(const JGeometry::TVec3<f32>& a,
	                const JGeometry::TVec3<f32>& b)
	{
		f32 x2  = (a.x - b.x) * (a.x - b.x);
		f32 y2  = (a.y - b.y) * (a.y - b.y);
		f32 z2  = (a.z - b.z) * (a.z - b.z);
		f32 sum = x2 + y2;
		return z2 + sum;
	}
};

void TBigWatermelon::touchActor(THitActor* actor)
{
	if (isState(TMapObjGeneral::STATE_APPEARING))
		return;

	if (!isState(TMapObjBase::STATE_NORMAL)) {
		// Word copy, same as a Vec assign. The y compare reads the copy.
		Vec vel = *(Vec*)&mVelocity;
		if (vel.y < 0.0f) {
			kill();
			return;
		}
	}

	if (actor->isActorType(0x80000001)) {
		f32 dist = JGeometry::TUtil<f32>::sqrt(
		    TMelonSum::calc(mPosition, actor->mPosition));
		if (dist < 0.6f * mBodyRadius) {
			kill();
			return;
		}
	}

	if (actor->isActorType(0x10000015)
	    && ((TPoiHana*)actor)->isMoving()) {
		if (fabsf(mVelocity.y) < mMapObjData->mPhysical->unk4->unkC) {
			mVelocity.y += 30.0f;
			mState = 0xB;
		}
		return;
	}

	if ((s32)unk194 != 0)
		return;
	if (isState(TMapObjGeneral::STATE_HOLDING))
		return;
	if (isHideObj(actor))
		return;
	if (actor->isActorType(0x08000083))
		return;
	if (actor->isActorType(0x400000CA))
		return;
	if (actor->isActorType(0x400000CC))
		return;
	if (actor->isActorType(0x80000001) && !isActorType(0x400000D0)
	    && SMS_GetMarioSpeedY() != 0.0f) {
		kicked();
		return;
	}
	boundByActor(actor);
}

class TItemManager;
extern TItemManager* gpItemManager;
extern "C" TMapObjBase*
makeObjAppear__18TMapObjBaseManagerFfffUlb(TItemManager*, f32, f32, f32, u32,
                                           bool);

void TBigWatermelon::kill()
{
	emitAndScale(0x5D, 0, &mPosition);
	emitAndScale(0x5E, 0, &mPosition);
	emitAndScale(0x5F, 0, &mPosition);
	JGeometry::TVec3<f32> scale;
	scale.setAll(1.0f);
	emitAndScale(0x6B, 0, &mPosition, scale);
	emitAndScale(0x6C, 0, &mPosition, scale);
	*(Vec*)((u8*)unk198 + 0x70) = *(Vec*)&mPosition;
	gpModelWaterManager->emitRequest(*(TWaterEmitInfo*)unk198);
	if (gpMSound->gateCheck(MSD_SE_OBJ_WATERMELON_BLOCK))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_WATERMELON_BLOCK, &mPosition, 0, nullptr, 0, 4);
	if ((int)unk19C < 10) {
		TMapObjBase* obj = makeObjAppear__18TMapObjBaseManagerFfffUlb(
		    gpItemManager, mPosition.x, mPosition.y, mPosition.z, 0x2000000E,
		    true);
		if (obj != nullptr) {
			obj->mVelocity.x = 0.0f;
			obj->mVelocity.y = 25.0f;
			obj->mVelocity.z = 0.0f;
			obj->offLiveFlag(LIVE_FLAG_UNK10);
			unk19C += 1;
		}
	}
	TMapObjGeneral::kill();
	char trash[0x10];
	trash[0] = 0;
}

void TBigWatermelon::appearing()
{
	// Dead slot so MWCC keeps the frame at -0x38.
	char trash[0x18];
	trash[0] = 0;
	TMapObjGeneral::appearing();
	MtxPtr mtx = getModel()->getAnmMtx(0);
	calcRootMatrix();
	getModel()->calc();
	mtx[1][3] = mBodyRadius * (mScaling.y / mInitialScaling.y) + mPosition.y;
	mScaledBodyRadius = 50.0f * mScaling.x;
	mDamageRadius     = 50.0f * mScaling.x;
	calcEntryRadius();
	if (isState(1)) {
		mActorType    = 0x400000D0;
		mAttackRadius = 50.0f * mScaling.x;
		calcEntryRadius();
	} else {
		mActorType    = 0x400000DB;
		mAttackRadius = 0.0f;
		calcEntryRadius();
	}
}

void TBigWatermelon::control()
{
	// Declare scale, then velocity, then the take matrix so the
	// slots land low-to-high as matrix, velocity, scale.
	JGeometry::TVec3<f32> scale;
	JGeometry::TVec3<f32> vel;
	Mtx mtx;
	// Dead slot so the matrix stays at r1+0x38 and the frame at -0x90.
	char trash[0x18];
	trash[0] = 0;
	TMapObjGeneral::control();
	if ((s32)unk194 != 0)
		unk194 -= 1;
	if (isState(TMapObjGeneral::STATE_HOLDING)) {
		MTXCopy(mHolder->getTakingMtx(), mtx);
		mtx[1][3] += unk190;
		MTXCopy(mtx, getModel()->getAnmMtx(0));
	} else {
		vel = mVelocity;
		if (!(vel.squared() <= JGeometry::TUtil<f32>::epsilon()
		      && mGroundPlane->mActor == nullptr))
			calcCurrentMtx();
	}
	switch (mState) {
	case TMapObjBase::STATE_NORMAL:
		if (checkLiveFlag(LIVE_FLAG_UNK10))
			offLiveFlag(LIVE_FLAG_UNK10);
		{
			const TLiveActor* actor = mGroundPlane->getActor();
			// Second identical type test is in the retail body.
			if (mPosition.y < 200.0f + mGroundHeight && actor != nullptr
			    && (actor->isActorType(0x400000CD)
			        || actor->isActorType(0x400000CD))) {
				f32 prev = unk1A0;
				unk1A0   = SMS_GetSandRiseUpRatio(actor);
				if (unk1A0 > 0.05f && unk1A0 > prev)
					mVelocity.y += 20.0f;
			}
		}
		break;
	case TMapObjGeneral::STATE_APPEARING:
		break;
	// 0xA..0xC are empty. They are what makes the compare split at 0xA.
	case TMapObjGeneral::STATE_WAITING_TO_APPEAR:
	case 0xB:
	case 0xC:
		break;
	case 0xD:
		if (!isStateTimerEngaged()) {
			scale.setAll(1.0f);
			emitAndScale(0x6B, 0, &mPosition, scale);
			emitAndScale(0x6C, 0, &mPosition, scale);
			mStateTimer = 0x1E;
		}
		if (animIsFinished())
			makeObjDead();
		break;
	}
}

static inline void fireMelonCam(const JGeometry::TVec3<f32>* pos)
{
	// Dead copies keep the -0x88 frame and the flag at r1+0x24.
	u32 a = 0;
	u32 b = a;
	u32 c = b;
	u32 d = c;
	(void)d;
	TMarDirector* director = gpMarDirector;
	director->fireStartDemoCamera("スイカゴールカメラ", pos, -1, 0.0f, true,
	                              nullptr, 0, nullptr,
	                              JDrama::TFlagT<u16>(0));
}

void TBigWatermelon::startEvent()
{
	if (strcmp(getName(), "スイカ（大）") == 0) {
		mPosition.x = -4660.0f;
		mPosition.y = 1300.0f;
		mPosition.z = 13600.0f;
		offMapObjFlag(MAP_OBJ_FLAG_UNK100);
		onLiveFlag(LIVE_FLAG_UNK10);
		mVelocity.z = 0.0f;
		mVelocity.y = 0.0f;
		mVelocity.x = 0.0f;
		onLiveFlag(LIVE_FLAG_UNK10);
		startAnim(7);
		fireMelonCam(&mPosition);
		gpItemManager->makeShineAppearWithDemoOffset(
		    "シャイン（お化けスイカ用）", "スイカシャインカメラ", 0.0f, 0.0f,
		    0.0f);
		startStateTimer(0x17C);
		setState(0xD);
	} else {
		for (int i = 0; i < 10; ++i) {
			TMapObjBase* obj = makeObjAppear__18TMapObjBaseManagerFfffUlb(
			    gpItemManager, gpMarioPos->x, gpMarioPos->y, gpMarioPos->z,
			    0x2000000E, true);
			if (obj != nullptr) {
				f32 rz = (f32)rand() * 0.000030517578f;
				f32 ry = (f32)rand() * 0.000030517578f;
				f32 rx = (f32)rand() * 0.000030517578f;
				obj->mVelocity.x = 20.0f * (rx - 0.5f);
				obj->mVelocity.y = 20.0f * ry + 20.0f;
				obj->mVelocity.z = 20.0f * (rz - 0.5f);
				obj->offLiveFlag(LIVE_FLAG_UNK10);
				((TItem*)obj)->unk14C = 0x3C0;
			}
		}
		makeObjDead();
	}
}

void TBigWatermelon::checkWallCollision(JGeometry::TVec3<f32>* param_1)
{
	TMapObjGeneral::checkWallCollision(param_1);
}

BOOL TBigWatermelon::receiveMessage(THitActor* sender, u32 message)
{
	if (sender->isActorType(0x80000001)) {
		boundByActor(sender);
		return true;
	}
	if (TMapObjGeneral::receiveMessage(sender, message))
		return true;
	if (message == 4 && checkMapObjFlag(MAP_OBJ_FLAG_UNK100000)) {
		hold((TTakeActor*)sender);
		return true;
	}
	if (sender->isActorType(0x80000001) && !isActorType(0x400000D0)
	    && message != 4) {
		kicked();
		return true;
	}
	return false;
}

void TBigWatermelon::loadAfter()
{
	TMapObjGeneral::loadAfter();
	JDrama::TActor* actor = (JDrama::TActor*)JDrama::TNameRefGen::search(
	    "シャイン（お化けスイカ用）");
	actor->mPosition.x = -4659.0f;
	actor->mPosition.y = 460.0f;
	actor->mPosition.z = 13620.0f;
}

void TBigWatermelon::initMapObj()
{
	TMapObjBall::initMapObj();
	SMS_LoadParticle("/scene/mapObj/watermelon_bomb.jpa", 0x5D);
	SMS_LoadParticle("/scene/mapObj/watermelon_bomb_a.jpa", 0x5E);
	SMS_LoadParticle("/scene/mapObj/watermelon_bomb_b.jpa", 0x5F);
	SMS_LoadParticle("/scene/mapObj/watermelon_shrink_a.jpa", 0x6B);
	SMS_LoadParticle("/scene/mapObj/watermelon_shrink_b.jpa", 0x6C);
	unk198 = (u32)new TWaterEmitInfo("/watermelon.prm");
}

TBigWatermelon::TBigWatermelon(const char* name)
    : TMapObjBall(name)
    , unk198(0)
    , unk19C(0)
    , unk1A0(0.0f)
{
}
