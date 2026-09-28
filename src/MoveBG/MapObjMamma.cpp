#include <MoveBG/MapObjMamma.hpp>
#include <MoveBG/MapObjBall.hpp>
#include <MoveBG/MapObjFlag.hpp>
#include <MoveBG/MapObjWave.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <Map/Map.hpp>
#include <Map/MapCollisionEntry.hpp>
#include <Map/MapData.hpp>
#include <M3DUtil/MActor.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <MoveBG/ItemManager.hpp>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <System/Particles.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

u32 TSandBase::mWitherTime                 = 800;
f32 TSandBase::mScaleMin                   = 0.00001f;
f32 TSandBombBase::mFiringFrameSpeed       = 3.0f;
f32 TSandBombBase::mFiringFrameDownSpeed   = 0.2f;
f32 TSandBombBase::mExplodeFrameSpeed      = 1.0f;
f32 TSandBombBase::mMarioJumpRate          = 0.12f;
u32 TSandBombBase::mExlodingRumbleTime     = 0x14;
f32 TSandCastle::mCollisionRate            = 1.7f;
u32 TLeanMirror::mGoTargetTime             = 600;
u32 TLeanMirror::mDemoWaitTime             = 0xFFFFFFFF;
u32 TLeanMirror::mDemoLightTime            = 360;
f32 TMammaBlockRotate::mRotSpeed           = 0.1f;
f32 TMammaBlockRotate::mRotReturnSpeed     = 0.01f;
f32 TMammaBlockRotate::mRotEnd             = 130.0f;
f32 TMammaBlockRotate::mMapGoSpeed         = 1.0f;
f32 TMammaBlockRotate::mMapBackSpeed       = 0.1f;
u32 TMammaBlockRotate::mWaitTime           = 600;

// Vtable is the same size as TMapObjBall. Only control is in this TU.
class TWatermelon : public TMapObjBall {
public:
	virtual void control();
};

u32 TSandLeaf::touchWater(THitActor*)
{
	unk138->getLivingTime();
	return 1;
}

void TSandLeaf::control()
{
	TMapObjBase::control();
	mGroundHeight = gpMap->checkGround(mPosition.x, mPosition.y + 200.0f,
	                                   mPosition.z, &mGroundPlane);
	mPosition.y   = mGroundHeight;
}

void TSandBase::isDown() const { }

void TSandBase::withering() { }

TSandBase::TSandBase(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk144(nullptr)
{
}

void TSandLeafBase::grow() { }

void TSandLeafBase::control() { }

void TSandLeafBase::initMapObj()
{
	unk138 = 0.003f;
	unk13C = 0.001f;
	unk140 = 0;
	mScaling.y = TSandBase::mScaleMin;
	TMapObjBase::initMapObj();
	unk144 = TMapObjBaseManager::newAndRegisterObj("SandLeaf", mPosition,
	                                               mRotation);
	((TSandLeaf*)unk144)->unk138 = (TMapObjGeneral*)this;
	unk144->appear();

	// Dead slot so MWCC keeps frame -0x28 and the scale temp at r1+0x10.
	char trash[1];
	trash[0] = 0;
}

void TSandBomb::makeObjAppeared()
{
	TMapObjBase::makeObjAppeared();
	startControlAnim(1);
	startControlAnim(2);
}

u32 TSandBomb::touchWater(THitActor*) { return 0; }

u32 TSandBomb::getSDLModelFlag() const { return 0; }

void TSandBomb::initMapObj() { TMapObjBase::initMapObj(); }

void TSandBombBase::withered()
{
	mStateTimer = unk140;
	mState      = 3;
	unk144->sleep();
}

void TSandBombBase::expanded() { }

void TSandBombBase::exploding() { }

void TSandBombBase::explode() { }

void TSandBombBase::waitBeforeExplode()
{
	mState      = 6;
	mStateTimer = unk148;
}

void TSandBombBase::grow() { mState = 5; }

void TSandBombBase::control() { }

TMapObjBase* TSandBombBase::findTriggerActor()
{
	JGeometry::TVec3<f32> scale(1.0f);
	return TMapObjBaseManager::newAndRegisterObj("SandBomb", mPosition,
	                                             mRotation, scale);
}

void TSandBombBase::loadAfter()
{
	unk144 = findTriggerActor();
	((TSandLeaf*)unk144)->unk138 = (TMapObjGeneral*)this;
	unk144->appear();
}

// Empty in this TU. dont_inline keeps the qualified call in TSandCastle::initMapObj.
#pragma dont_inline on
void TSandBombBase::initMapObj() { }
#pragma dont_inline off

TSandBombBase::TSandBombBase(const char* name)
    : TSandBase(name)
    , unk148(0)
    , unk14C(1.0f)
    , unk150(0.0f)
    , unk154(0.0f)
{
}

void TSandCastle::withering() { }

void TSandCastle::expanded() { }

void TSandCastle::explode() { }

static void SandCastleCallBack(u32, u32) { }

void TSandCastle::waitBeforeExplode() { }

void TSandCastle::calcRootMatrix()
{
	if (!isState(2))
		TMapObjBase::calcRootMatrix();
}

TMapObjBase* TSandCastle::findTriggerActor()
{
	return (TMapObjBase*)JDrama::TNameRefGen::search("砂の城爆発の芽");
}

void TSandCastle::loadAfter()
{
	unk144                       = findTriggerActor();
	((TSandLeaf*)unk144)->unk138 = (TMapObjGeneral*)this;
	unk144->appear();
	unk158 = (TMapObjBase*)JDrama::TNameRefGen::search(
	    "ステージ切替（砂の城）");
	unk158->makeObjDead();
}

void TSandCastle::initMapObj()
{
	TSandBombBase::initMapObj();
	unk13C = 0.11f;
	unk148 = 0x78;
	sleep();
}

TSandCastle::TSandCastle(const char* name)
    : TSandBombBase(name)
    , unk158(0)
    , unk15C(0)
{
}

void TLeanMirror::enemyIsOn() const { }

void TLeanMirror::draw() const { }

void TLeanMirror::updateSpeedVec(const JGeometry::TVec3<f32>&, f32) { }

BOOL TLeanMirror::receiveMessage(THitActor*, u32) { return FALSE; }

void TLeanMirror::touchPlayer(THitActor*) { }

void TLeanMirror::touchEnemy(THitActor*) { }

void TLeanMirror::calcCurrentMtx(MtxPtr) { }

void TLeanMirror::release() { }

static s32 startCameraShakeSE(u32 pos, u32 param_2)
{
	if (param_2 == 0) {
		// gpMSound stays in r0; the position is saved after that load.
		MSound* sound = gpMSound;
		const Vec* position = (const Vec*)pos;
		if (sound->gateCheck(MSD_SE_OBJ_QUAKE))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_QUAKE, position, 0, nullptr, 0, 4);
	}
	// Dead slot so MWCC keeps frame -0x20.
	char trash[1];
	trash[0] = 0;
	return 0;
}

void TLeanMirror::controlGoTarget() { }

void TLeanMirror::controlShake() { }

void TLeanMirror::control() { }

void TLeanMirror::loadAfter() { }

u32 TLeanMirror::getSDLModelFlag() const { return 0; }

void TLeanMirror::initMapObj()
{
	TMapObjBase::initMapObj();
	unk158 = 0.03f;
	unk15C = 0.999f;
	unk160 = 0.0001f;
	unk168 = 1.0f;
	unk16C = 0.0002f;
	unk170 = 0.0001f;
	unk174 = 0.865f;
	unk178 = 0.5f;
	if (strcmp(unkF4, "mirrorS") == 0) {
		unk164 = 0.002f;
		unk168 = 1.0f;
		unk174 = 0.87f;
		unk19C = 1;
	} else if (strcmp(unkF4, "mirrorM") == 0) {
		unk164 = 0.004f;
		unk19C = 2;
	} else {
		unk164 = 0.006f;
		unk19C = 3;
	}
}

void TLeanMirror::load(JSUMemoryInputStream&) { }

TLeanMirror::TLeanMirror(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk158(0.0f)
    , unk15C(0.0f)
    , unk160(0.0f)
    , unk164(0.0f)
    , unk168(0.0f)
    , unk16C(0.0f)
    , unk170(0.0f)
    , unk17C(0)
    , unk198(0.0f)
    , unk19C(0)
    , unk1AC(0)
    , unk1AE(0)
{
	unk140.zero();
	unk14C.zero();
	unk180.zero();
	unk18C.zero();
	unk1A0.zero();
}

void TShiningStone::endDemo() { }

void TShiningStone::putOnLight(TLiveActor*) { }

void TShiningStone::perform(u32 cue, JDrama::TGraphics* graphics)
{
	for (int i = 0; i < 4; ++i) {
		((MActor**)unk68)[i]->perform(cue, graphics);
		if ((int)unk74 > 0)
			gpMarioParticleManager->emit(0x143, &mPosition, 1, this);
		if ((int)unk74 > 1)
			gpMarioParticleManager->emit(0x144, &mPosition, 1, this);
		if ((int)unk74 > 2)
			gpMarioParticleManager->emit(0x145, &mPosition, 1, this);
	}
	((MActor*)unk6C)->perform(cue, graphics);
}

void TShiningStone::load(JSUMemoryInputStream&) { }

TShiningStone::TShiningStone(const char* name)
    : THitActor(name)
{
	unk74 = 0;
	unk78 = 0;
	unk7C = 0.0f;
	unk70 = 0;
	unk71 = 0;
	unk72 = 0;
	unk73 = 0;
}

u32 TMammaBlockRotate::touchWater(THitActor*) { return 0; }

void TMammaBlockRotate::control() { }

void TMammaBlockRotate::initMapObj() { }

void TMammaBlockRotate::load(JSUMemoryInputStream& stream)
{
	unk144 = new TMapCollisionMove;
	unk144->init("/scene/mapObj/MammaBlockDown.col", 0, this);
	unk148 = new TMapCollisionMove;
	unk148->init("/scene/mapObj/MammaBlockUp.col", 0, this);
	TMapObjBase::load(stream);
}

TMammaBlockRotate::TMammaBlockRotate(const char* name)
    : TMapObjBase(name)
    , unk13C(0)
    , unk140(0)
    , unk144(0)
    , unk148(0)
{
}

// Groups the three stores so MWCC emits stfsu. TVec3::set(f32, f32, f32) is dont_inline.
static inline void setYachtVec(JGeometry::TVec3<f32>& v, f32 x, f32 y, f32 z)
{
	v.x = x;
	v.y = y;
	v.z = z;
}

void TMammaYacht::control()
{
	TMapObjBase::control();
	if (mGroundPlane->isWaterSurface()) {
		mPosition.y = mInitialPosition.y
		              + gpMapObjWave->getWaveHeight(mPosition.x, mPosition.z);
		unk138->mPosition.y = mPosition.y - 50.0f;
	}
}

void TMammaYacht::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = new TMapObjFlag("旗");

	setYachtVec(unk138->mPosition, 2.0f + mPosition.x,
	            (1315.0f + mPosition.y) - 190.0f, mPosition.z - 15.0f);
	setYachtVec(unk138->mRotation, 0.0f, 180.0f, 0.0f);
	setYachtVec(unk138->mScaling, 1.0f, 2.5f, 3.8f);
	unk138->init("MammaYacht00");
}

void TSandBird::control() { }

TMapObjBase* TSandBird::makeObjFromJointName(const char* name, unsigned short param)
{
	TMapObjBase* obj = TJointCoin::makeObjFromJointName(name, param);
	if (obj != nullptr)
		return obj;
	if (strstr(name, "none") == nullptr)
		return makeObj("SandBirdBlock", param);
	return nullptr;
}

bool TSandBird::nameIsObj(const char* name)
{
	return strstr(name, "none") == nullptr ? true : false;
}

void TSandBird::initMapObj()
{
	TJointCoin::initMapObj();
	SMS_LoadParticle("/scene/map/map/ms_sunadori_a.jpa", 0x159);
	SMS_LoadParticle("/scene/map/map/ms_sunadori_b.jpa", 0x15A);
}

TSandBird::TSandBird(const char* name)
    : TJointCoin(name)
    , unk150(0)
    , unk151(0)
{
}

void TWatermelon::control() { }

void TGoalWatermelon::touchActor(THitActor*) { }

void TGoalWatermelon::control()
{
	TMapObjBase::control();

	// Empty 0/1/3 keep the dispatch: cmp 2, beq, bge, b, b.
	switch (mState) {
	case 0:
	case 1:
		break;
	case 2:
		if (unk13C->animIsFinished()) {
			gpItemManager->makeShineAppearWithDemoOffset(
			    "シャイン（お化けスイカ用）", "スイカシャインカメラ", 0.0f,
			    0.0f, 0.0f);
			mState = 3;
		}
		break;
	case 3:
		break;
	}
}

void TGoalWatermelon::loadAfter()
{
	TMapObjBase::loadAfter();
	onHitFlag(HIT_FLAG_CANNOT_GET_HIT);
	unk138 = (TMapObjBase*)JDrama::TNameRefGen::search(
	    "シャイン（お化けスイカ用）");
	unk138->mPosition.set(unk140);
	unk138->appear();
}

void TGoalWatermelon::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	char name[0x20];
	stream.readString(name, 0x20);
	stream.read(&unk140.x, 4);
	stream.read(&unk140.y, 4);
	stream.read(&unk140.z, 4);

	// Dead slot so MWCC keeps frame -0x48.
	char trash[0xC];
	trash[0] = 0;
}

TGoalWatermelon::TGoalWatermelon(const char* name)
    : TMapObjBase(name)
    , unk138(nullptr)
    , unk13C(nullptr)
{
	unk140.zero();
}

void TMammaMirrorMapOperator::show(int) { }

void TMammaMirrorMapOperator::hide(int) { }

void TMammaMirrorMapOperator::perform(u32, JDrama::TGraphics*) { }

void TMammaMirrorMapOperator::loadAfter() { }

TMammaMirrorMapOperator::TMammaMirrorMapOperator(const char* name)
    : JDrama::TViewObj(name)
{
}

u32 TSandEgg::getSDLModelFlag() const { return 0; }
