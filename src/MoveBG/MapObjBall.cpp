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
#include <System/FlagManager.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

u32 TResetFruit::mFruitLivingTime       = 14400;
f32 TResetFruit::mScaleUpSpeed          = 1.05f;
f32 TResetFruit::mRottingScaleSpeed     = 0.0f; // UNUSED, value not in the DOL
f32 TResetFruit::mBreakingScaleSpeed    = 0.96f;
u32 TResetFruit::mFruitWaitTimeToAppear = 360;
GXColorS10 TResetFruit::mRottenColor    = { 0, 0, 0, 0 }; // UNUSED

void TMapObjBall::touchRoof(JGeometry::TVec3<f32>*) { }

// Empty here. dont_inline keeps the qualified calls in TBigWatermelon.
#pragma dont_inline on
void TMapObjBall::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }
#pragma dont_inline off

void TMapObjBall::touchPollution() { kill(); }

void TMapObjBall::touchWaterSurface() { kill(); }

void TMapObjBall::rebound(JGeometry::TVec3<f32>*) { }

#pragma dont_inline on
void TMapObjBall::touchGround(JGeometry::TVec3<f32>*) { }
#pragma dont_inline off

void TMapObjBall::put()
{
	TMapObjGeneral::put();
	calcCurrentMtx();
}

void TMapObjBall::hold(TTakeActor*) { }

void TMapObjBall::kicked() { }

u32 TMapObjBall::touchWater(THitActor*) { return 0; }

void TMapObjBall::boundByActor(THitActor*) { }

void TMapObjBall::touchActor(THitActor*) { }

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
	char trash[1];
	trash[0] = 0;
}

void TMapObjBall::makeObjAppeared() { }

void TMapObjBall::control() { }

BOOL TMapObjBall::receiveMessage(THitActor*, u32) { return 0; }

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

void TResetFruit::waitingToAppear() { }

void TResetFruit::makeObjWaitingToAppear() { }

void TResetFruit::thrown()
{
	TMapObjGeneral::thrown();
	mState = 11;
}

void TResetFruit::hold(TTakeActor*) { }

void TResetFruit::touchPollution() { }

void TResetFruit::touchWaterSurface() { }

u32 TResetFruit::touchWater(THitActor*) { return 0; }

void TResetFruit::touchActor(THitActor*) { }

void TResetFruit::touchGround(JGeometry::TVec3<f32>*) { }

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

void TResetFruit::breaking() { }

void TResetFruit::appearing() { }

void TResetFruit::control() { }

void TResetFruit::perform(u32, JDrama::TGraphics*) { }

void TResetFruit::killByTimer(int param_1)
{
	mStateTimer = param_1;
	onMapObjFlag(MAP_OBJ_FLAG_DISAPPEARING);
	mState = 11;
}

void TResetFruit::makeObjAppeared() { }

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

void TRandomFruit::initMapObj() { }

TRandomFruit::TRandomFruit(const char* name)
    : TResetFruit(name)
{
	memset(unk1A8, 0, sizeof(unk1A8));
}

void TCoverFruit::calcRootMatrix() { }

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

void TBigWatermelon::kill() { }

void TBigWatermelon::appearing() { }

void TBigWatermelon::control() { }

void TBigWatermelon::startEvent() { }

void TBigWatermelon::checkWallCollision(JGeometry::TVec3<f32>* param_1)
{
	TMapObjGeneral::checkWallCollision(param_1);
}

BOOL TBigWatermelon::receiveMessage(THitActor*, u32) { return 0; }

void TBigWatermelon::loadAfter()
{
	TMapObjGeneral::loadAfter();
	JDrama::TActor* actor = (JDrama::TActor*)JDrama::TNameRefGen::search(
	    "シャイン（お化けスイカ用）");
	actor->mPosition.x = -4659.0f;
	actor->mPosition.y = 460.0f;
	actor->mPosition.z = 13620.0f;
}

void TBigWatermelon::initMapObj() { }

TBigWatermelon::TBigWatermelon(const char* name)
    : TMapObjBall(name)
    , unk198(0)
    , unk19C(0)
    , unk1A0(0.0f)
{
}
