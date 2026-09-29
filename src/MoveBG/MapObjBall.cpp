#include <MoveBG/MapObjBall.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <MarioUtil/PacketUtil.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
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

#pragma dont_inline on
void TMapObjBall::touchGround(JGeometry::TVec3<f32>*) { }
#pragma dont_inline off

void TMapObjBall::put()
{
	TMapObjGeneral::put();
	calcCurrentMtx();
}

// One inline deep so sqrt stays a call. length() would expand it to frsqrte.
inline f32 doSqrt(f32 lenSq) { return JGeometry::TUtil<f32>::sqrt(lenSq); }

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

// Empty here. dont_inline keeps the qualified call in TResetFruit::touchActor.
#pragma dont_inline on
void TMapObjBall::touchActor(THitActor*) { }
#pragma dont_inline off

void TMapObjBall::calcCurrentMtx() { }

void TMapObjBall::checkWallCollision(JGeometry::TVec3<f32>*) { }

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
void TMapObjBall::initMapObj() { }
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

void TResetFruit::checkGroundCollision(JGeometry::TVec3<f32>*) { }

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

void TResetFruit::control() { }

void TResetFruit::perform(u32, JDrama::TGraphics*) { }

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

BOOL TResetFruit::receiveMessage(THitActor*, u32) { return 0; }

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

void TBigWatermelon::rebound(JGeometry::TVec3<f32>*) { }

void TBigWatermelon::touchGround(JGeometry::TVec3<f32>* param_1)
{
	TMapObjBall::touchGround(param_1);
}

void TBigWatermelon::touchActor(THitActor*) { }

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

void TBigWatermelon::control() { }

void TBigWatermelon::startEvent()
{
	// TODO: real body is 0x220. These names have to be referenced so they
	// land between the shine literal and the random-fruit formats.
	if (strcmp(mName, "スイカ（大）") == 0)
		return;
	if (strcmp(mName, "スイカゴールカメラ") == 0)
		return;
	if (strcmp(mName, "スイカシャインカメラ") == 0)
		return;
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
