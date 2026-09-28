#include <MoveBG/MapObjBianco.hpp>
#include <MoveBG/MapObjMessenger.hpp>
#include <Map/MapCollisionManager.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <MarioUtil/DrawUtil.hpp>
#include <MarioUtil/PacketUtil.hpp>
#include <M3DUtil/MActor.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <Player/MarioAccess.hpp>
#include <System/Application.hpp>
#include <stdlib.h>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TBigWindmill::control() { }

void TBigWindmill::load(JSUMemoryInputStream&) { }

void TMapObjRootPakkun::drawObject(JDrama::TGraphics*) { }

void TMapObjRootPakkun::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = new TTrembleModelEffect;
	unk138->init(mMActor->getModel());
	unk138->tremble(100.0f, 1.0f, 1.0f, 0x2EE0);
}

void TBiancoWatermill::turnByEnemy(THitActor*, const TBGCheckData*) { }

// UNUSED
void TBiancoWatermill::turn(const JGeometry::TVec3<f32>&, const TBGCheckData*,
                            f32)
{
}

u32 TBiancoWatermill::touchWater(THitActor*) { return 0; }

void TBiancoWatermill::control()
{
	mRotation.z -= unk138;
	SMSGetMSound()->startSoundActorWithInfo(
	    MSD_SE_OBJ_BI_BIGMILL, &mPosition, nullptr, fabsf(unk138), 0, 0,
	    (JAISoundHandle*)&unk13C, 0, 4);
}

void TBiancoWatermill::initMapObj()
{
	TMapObjBase::initMapObj();
	if (strcmp(unkF4, "BiaWatermill01") == 0)
		mBodyRadius = 1200.0f;
	else if (strcmp(unkF4, "BiaWatermill00") == 0)
		mBodyRadius = 1200.0f;
}

TBiancoWatermill::TBiancoWatermill(const char* name)
    : TMapObjBase(name)
    , unk138(0.3f)
    , unk13C(0)
{
}

u32 TBiancoWatermillVertical::touchWater(THitActor*) { return 0; }

void TBiancoWatermillVertical::setGroundCollision()
{
	if (unk144 != 0 || mColCount != 0) {
		J3DModel* model = getModel();
		MtxPtr mtx     = model->getAnmMtx(0);
		if (mMapCollisionManager->unk8 != nullptr)
			mMapCollisionManager->unk8->moveMtx(mtx);
		unk144 = 0;
	}

	// Dead slot so MWCC keeps frame -0x28.
	char trash[8];
	trash[0] = 0;
}

void TBiancoWatermillVertical::control() { }

void TBiancoWatermillVertical::loadAfter() { }

void TBiancoWatermillVertical::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	stream >> unk13C;
	unk13C /= 1000.0f;
	unk138 = unk13C;
}

TBiancoWatermillVertical::TBiancoWatermillVertical(const char* name)
    : TMapObjBase(name)
{
}

u32 TBiancoMiniWindmill::touchWater(THitActor*) { return 0; }

void TBiancoMiniWindmill::calc() { }

f32 TBiancoMiniWindmill::mFriction = 0.01f;

void TBiancoMiniWindmill::control()
{
	if (unk154 > unk158)
		unk154 -= mFriction;
	else
		unk154 = unk158;
	unk150 += unk154;
	unk150 = MsWrap(unk150, 0.0f, 360.0f);
}

static f32 sMessengerPosZ = 200.0f;
static f32 sMessengerPosY = 6400.0f;

void TBiancoMiniWindmill::initMapObj()
{
	TMapObjBase::initMapObj();
	mAppearSpeed = 0.0f;
	unk15C       = new TMapObjMessenger("地形オブジェメッセンジャー");
	unk15C->initHitActor(0, 1, 0, 0.0f, 0.0f, 300.0f, 500.0f);
	unk15C->mPosition.x
	    = mPosition.x + sMessengerPosZ * MsSin(mRotation.y);
	unk15C->mPosition.y = mPosition.y + sMessengerPosY;
	unk15C->mPosition.z
	    = mPosition.z + sMessengerPosZ * MsCos(mRotation.y);
}

TBiancoMiniWindmill::TBiancoMiniWindmill(const char* name)
    : THideObjBase(name)
{
	unk150 = 360.0f * ((f32)rand() * 0.000030517578f);
	unk154 = 0.0f;
	f32 randScale = (f32)rand() * 0.000030517578f;
	unk158       = 1.0f + randScale;
	unk15C = 0;
	unk160 = 0;
}

void TLeafBoat::touchActor(THitActor*) { }

void TLeafBoat::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }

void TLeafBoat::bind() { }

void TLeafBoat::control() { }

void TLeafBoat::calc() { }

void TLeafBoat::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = 1.0f;
	unk13C = 0.5f;
	unk140 = 0.5f;
	unk148 = 0.998f;
}

TLeafBoat::TLeafBoat(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.03f)
    , unk148(0.0f)
    , unk14C(1.2f)
    , unk150(0.03f)
    , unk154(2.0f)
    , unk158(0.005f)
	, unk15C(0.98f)
	, unk160(0)
{
	unk164.zero();
}

void TLeafBoatRotten::control() { }

void TLeafBoatRotten::perform(u32 cue, JDrama::TGraphics* graphics)
{
	TMapObjBase::perform(cue, graphics);
}

void TLeafBoatRotten::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	stream >> unk170;
	unk170 *= 10;
	SMS_InitPacket_OneTevColor(getModel(), 0, GX_TEVREG0,
	                           (const GXColorS10*)&unk178);
}

TLeafBoatRotten::TLeafBoatRotten(const char* name)
    : TLeafBoat(name)
    , unk170(0)
    , unk178(0xFF)
    , unk17A(0xFF)
    , unk17C(0xFF)
    , unk17E(0xFF)
{
}

void TLampSeesaw::touchPlayer(THitActor*)
{
	if (marioIsOn())
		unk138->pushDown(-unk140);
}

void TLampSeesaw::load(JSUMemoryInputStream&) { }

TLampSeesaw::TLampSeesaw(const char* name)
    : TMapObjBase(name)
    , unk138(nullptr)
    , unk140(0.01f)
{
}

void TLampSeesawMain::pushDown(f32 param_1)
{
	mState = 2;
	unk144 -= param_1;
}

// UNUSED
void TLampSeesawMain::move() { }

void TLampSeesawMain::touchPlayer(THitActor*)
{
	if (marioIsOn())
		pushDown(unk140);
}

void TLampSeesawMain::control() { }

void TLampSeesawMain::loadAfter() { }

TLampSeesawMain::TLampSeesawMain(const char* name)
    : TLampSeesaw(name)
    , unk144(0.0f)
    , unk148(0.998f)
    , unk14C(0.8f)
    , unk150(0.5f)
{
}

// UNUSED
void TBiancoBell::stopToRing() { }

// UNUSED
void TBiancoBell::ring() { }

// UNUSED
void TBiancoBell::ringSingle() { }

// Fabricated. Deferred inlines make MWCC reload mMActor between the field loads.
static inline f32 biancoBellRate(MActor* actor)
{
	return actor->getFrameCtrl(0)->getRate();
}

static inline f32 biancoBellFrame(MActor* actor)
{
	return actor->getFrameCtrl(0)->getFrame();
}

static inline s16 biancoBellEnd(MActor* actor)
{
	return actor->getFrameCtrl(0)->getEnd();
}

u32 TBiancoBell::touchWater(THitActor*)
{
	if (mMActor->getFrameCtrl(0)->getFrame() == 0.0f
	    || biancoBellFrame(mMActor) + biancoBellRate(mMActor)
	           >= (f32)biancoBellEnd(mMActor) - 1.0f) {
		startAnim(4);
		mMActor->getFrameCtrl(0)->setRate(SMSGetAnmFrameRate());
		if (gpMSound->gateCheck(MSD_SE_OBJ_BI_BELL))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_BI_BELL, &mPosition, 0, nullptr, 0, 4);
	}

	// Dead slot so MWCC keeps frame -0x60.
	char trash[1];
	trash[0] = 0;
	return 1;
}

void TBiancoBell::touchPlayer(THitActor*)
{
	if (mMActor->getFrameCtrl(0)->getFrame() == 0.0f
	    || biancoBellFrame(mMActor) + biancoBellRate(mMActor)
	           >= (f32)biancoBellEnd(mMActor) - 1.0f) {
		startAnim(4);
		mMActor->getFrameCtrl(0)->setRate(SMSGetAnmFrameRate());
		if (gpMSound->gateCheck(MSD_SE_OBJ_BI_BELL))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_BI_BELL, &mPosition, 0, nullptr, 0, 4);
	}

	// Dead slot so MWCC keeps frame -0x60.
	char trash[1];
	trash[0] = 0;
}

void TBiancoBell::initMapObj()
{
	TMapObjBase::initMapObj();
	if (strcmp(getName(), "BiaBell 0") == 0) {
		unk138 = 1;
		unk13A = 0;
	} else if (strcmp(getName(), "BiaBell 1") == 0) {
		unk138 = 2;
		unk13A = 1;
	} else {
		unk138 = 3;
		unk13A = 0;
	}
}

TBiancoBell::TBiancoBell(const char* name)
    : TMapObjBase(name)
    , unk138(0)
    , unk13A(0)
{
}

u32 TBellWatermill::touchWater(THitActor*)
{
	unk190 = 1;
	if (fabsf(unk158) > unk16C) {
		unk178 += unk180;
		unk158 += unk15C;
	} else {
		unk158 += unk15C;
	}
	if (unk158 > unk164)
		unk158 = unk164;
	return 1;
}

void TBellWatermill::control() { }

void TBellWatermill::loadAfter() { }

TBellWatermill::TBellWatermill(const char* name)
    : TMapObjTurn(name)
    , unk16C(0.0f)
    , unk170(0.0f)
    , unk174(0.0f)
    , unk178(0.0f)
    , unk17C(0.0f)
    , unk180(0.0f)
    , unk184(0.0f)
    , unk188(0.0f)
    , unk18C(0.0f)
    , unk190(0)
    , unk1A0(0)
    , unk1A4(0)
{
}

void TWoodLog::control()
{
	TMapObjFloatOnSea::control();

	Mtx inv;
	JGeometry::TVec3<f32> marioPos;
	JGeometry::TVec3<f32> localPos;
	JGeometry::TVec3<f32> requestPos;

	MTXInverse(getModel()->getAnmMtx(0), inv);
	marioPos.set(*gpMarioPos);
	MTXMultVec(inv, (Vec*)&marioPos, (Vec*)&localPos);

	if (SMS_IsMarioStatusTypeSwimming() && -232.0f < localPos.y
	    && -141.0f < localPos.x && localPos.x < 141.0f && -441.0f < localPos.z
	    && localPos.z < 441.0f) {
		if (localPos.x > 0.0f)
			localPos.x = 141.0f;
		else
			localPos.x = -141.0f;
		MTXMultVec(getModel()->getAnmMtx(0), (Vec*)&localPos,
		           (Vec*)&requestPos);
		SMS_MarioMoveRequest(requestPos);
	}

	// Dead slot so MWCC keeps frame -0x90.
	char trash[0x14];
	trash[0] = 0;
}
