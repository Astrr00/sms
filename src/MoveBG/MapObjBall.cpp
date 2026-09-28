#include <MoveBG/MapObjBall.hpp>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

u32 TResetFruit::mFruitLivingTime       = 14400;
f32 TResetFruit::mScaleUpSpeed          = 1.05f;
f32 TResetFruit::mRottingScaleSpeed     = 0.0f; // UNUSED, value not in the DOL
f32 TResetFruit::mBreakingScaleSpeed    = 0.96f;
u32 TResetFruit::mFruitWaitTimeToAppear = 360;
GXColorS10 TResetFruit::mRottenColor    = { 0, 0, 0, 0 }; // UNUSED

void TMapObjBall::touchRoof(JGeometry::TVec3<f32>*) { }

void TMapObjBall::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }

void TMapObjBall::touchPollution() { kill(); }

void TMapObjBall::touchWaterSurface() { kill(); }

void TMapObjBall::rebound(JGeometry::TVec3<f32>*) { }

void TMapObjBall::touchGround(JGeometry::TVec3<f32>*) { }

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

void TMapObjBall::makeObjDefault() { }

void TMapObjBall::makeObjAppeared() { }

void TMapObjBall::control() { }

BOOL TMapObjBall::receiveMessage(THitActor*, u32) { return 0; }

void TMapObjBall::initMapObj() { }

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

void TResetFruit::makeObjLiving() { }

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

void TResetFruit::initMapObj() { }

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

BOOL TCoverFruit::receiveMessage(THitActor*, u32) { return 0; }

void TCoverFruit::loadAfter() { }

void TBigWatermelon::touchWaterSurface() { }

void TBigWatermelon::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }

void TBigWatermelon::rebound(JGeometry::TVec3<f32>*) { }

void TBigWatermelon::touchGround(JGeometry::TVec3<f32>*) { }

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

void TBigWatermelon::loadAfter() { }

void TBigWatermelon::initMapObj() { }

TBigWatermelon::TBigWatermelon(const char* name)
    : TMapObjBall(name)
    , unk198(0)
    , unk19C(0)
    , unk1A0(0.0f)
{
}
