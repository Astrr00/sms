#include <MoveBG/MapObjRicco.hpp>
#include <MoveBG/MapObjBall.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <M3DUtil/MActor.hpp>
#include <M3DUtil/MActorUtil.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <Map/MapCollisionManager.hpp>
#include <MarioUtil/RandomUtil.hpp>
#include <MSound/SoundEffects.hpp>
#include <stdlib.h>
#include <string.h>

static JGeometry::TVec3<f32> submarineCranePos_forSound(1956.0f, 1000.0f,
                                                        6425.0f);
static JGeometry::TVec3<f32> submarineSetWtPos_forSound(1956.0f, -100.0f,
                                                        6425.0f);

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.
// Unmatched bodies are stubs so every map symbol in this TU is defined.

void TCraneRotY::calc() { setRootMtxRotY(); }

void TCraneRotY::control()
{
	char trash[0x11];
	trash[0] = 0;
	TMapObjBase::control();
	switch (mState) {
	case 0:
		mRotation.y += unk144;
		if (mRotation.y > unk138 + unk140) {
			mStateTimer = mWaitTime;
			mState = 3;
		}
		break;
	case 1:
		if (!isStateTimerEngaged())
			mState = 0;
		break;
	case 2:
		mRotation.y -= unk144;
		if (mRotation.y < unk138 + unk13C) {
			mStateTimer = mWaitTime;
			mState = 1;
		}
		break;
	case 3:
		if (!isStateTimerEngaged())
			mState = 2;
		break;
	}
	if (isState(0) || isState(2)) {
		u32 se = unk148;
		if (gpMSound->gateCheck(se))
			MSoundSESystem::MSoundSE::startSoundActor(se, &mPosition, 0,
			                                          nullptr, 0, 4);
	}
}

s32 TCraneRotY::mWaitTime = 120;

void TCraneRotY::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	stream.read(&unk140, 4);
	unk138 = mRotation.y;
	unk144 = 0.05f + 0.1f * ((f32)rand() * 0.000030517578f);
	if (strcmp(mName, "crane90 0") == 0)
		unk148 = MSD_SE_OBJ_CRANE_SIDEMOVE1;
	else
		unk148 = MSD_SE_OBJ_CRANE_SIDEMOVE2;
	mState = 0;
}

f32 TCraneUpDown::mRotSpeed = 0.1f;
s32 TCraneUpDown::mWaitTime = 120;

f32 TRiccoWatermill::mRotAccel               = 1.0f;
f32 TRiccoWatermill::mRotSpeedMaxUp         = 3.0f;
f32 TRiccoWatermill::mSubmarineMaxTransY    = 750.0f;
f32 TRiccoWatermill::mSubmarineBottomTransY = -950.0f;

void TCraneUpDown::control() { }

// Inlined 4-byte pad. MWCC keeps it in the caller's frame (retail -0x48).
static inline void craneCargoFramePad()
{
	char trash[4];
	trash[0] = 0;
}

void TCraneUpDown::initMapObj()
{
	TMapObjBase::initMapObj();
	mMapCollisionManager->unk8->setAllActor(nullptr);
	unk138 = TMapObjBaseManager::newAndRegisterObj(
	    "craneCargoUpDown", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	unk138->appear();
	craneCargoFramePad();
	if (strcmp(mName, "craneUpDown 0") == 0) {
		unk144 = -25.0f;
		unk140 = 45.0f;
		unk13C = MSD_SE_OBJ_CRANE_UPDOWN1;
	} else {
		unk144 = -25.0f;
		unk140 = 30.0f;
		unk13C = MSD_SE_OBJ_CRANE_UPDOWN2;
	}
	mRotation.x = unk144 + (unk140 - unk144) * MsRandF();
}

void TCraneCargo::control()
{
	unk158.z = 0.0f;
	unk158.y = 0.0f;
	unk158.x = 0.0f;
	TMapObjBase::control();
}

void TCraneCargo::calc()
{
	updateRootMtxTrans();
	calcLeanMtx(getModel()->getAnmMtx(1));
}

u32 TRiccoWatermill::touchWater(THitActor*)
{
	if (isState(5))
		return 1;
	unk140 = 5;
	if (isState(1))
		unk13C->setUpMapCollision(1);
	offMapObjFlag(MAP_OBJ_FLAG_UNK100);
	unk13C->offMapObjFlag(MAP_OBJ_FLAG_UNK100);
	if (unk13C->mPosition.y < mSubmarineMaxTransY) {
		unk138 += mRotAccel;
		if (unk138 > mRotSpeedMaxUp)
			unk138 = mRotSpeedMaxUp;
		mState = 2;
	} else {
		unk138 = 0.0f;
	}
	return 1;
}

void TRiccoWatermill::control() { }

void TRiccoWatermill::calc() { setRootMtxRotZ(); }

// Leading zeroes so fruit names start at rodata 0xF8 and SurfGeso stays at 0x188.
static const char cRiccoWatermillRodataPad[0xE0] = { 0 };

void TRiccoWatermill::loadAfter()
{
	TMapObjBase::loadAfter();
	unk13C = (TMapObjBase*)JDrama::TNameRefGen::search("submarine");
	unk148 = (TMapObjBase*)JDrama::TNameRefGen::search("青コイン（潜水艦用）");
	unk148->makeObjDead();
	unk13C->mPosition.y = mSubmarineBottomTransY;
	unk13C->removeMapCollision();
	unk13C->setUpCurrentMapCollision();
}

TRiccoWatermill::TRiccoWatermill(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0)
    , unk140(0)
    , unk144(0)
    , unk148(0)
    , unk14C(0)
    , unk150(0)
    , unk154(0)
{
	// Dead slot so MWCC keeps frame -0x28 (r31 at r1+0x24).
	char trash[8];
	trash[0] = 0;
}

void TSurfGesoObj::initMapObj()
{
	TMapObjBase::initMapObj();
	if (strcmp(unkF4, "SurfGesoRed") == 0) {
		unk154.r = 0xFF;
		unk154.g = 0xB4;
		unk154.b = 0xFF;
		unk154.a = 0xFF;
	} else if (strcmp(unkF4, "SurfGesoYellow") == 0) {
		unk154.r = 0xFF;
		unk154.g = 0xFF;
		unk154.b = 0x7D;
		unk154.a = 0xFF;
	} else if (strcmp(unkF4, "SurfGesoGreen") == 0) {
		unk154.r = 0xB4;
		unk154.g = 0xFF;
		unk154.b = 0xB4;
		unk154.a = 0xFF;
	}

	TMapObjManager* mgr = gpMapObjManager;
	SDLModelData* data  = mgr->mSurfGessoModelData;
	MActorAnmData* anm  = mgr->getMActorAnmData();
	mMActor             = SMS_MakeMActorFromSDLModelData(data, anm, 3);
	initPacketMatColor(getModel(), GX_TEVREG1, &unk154);
	mMActor->setBck("surfgeso_run1");
	char trash[8];
	trash[0] = 0;
}

void TFruitSwitch::pullUp() { }

void TFruitSwitch::pushDown() { }

BOOL TFruitSwitch::receiveMessage(THitActor*, u32 message)
{
	if (message == HIT_MESSAGE_HIP_DROP) {
		startBck("riccoswitch");
		onHitFlag(HIT_FLAG_NO_COLLISION);
		if (mMapCollisionManager->unk8 != nullptr)
			mMapCollisionManager->unk8->remove();
		unk138->fireObj();
		return TRUE;
	}
	return FALSE;
}

void TFruitLauncher::appearFruit() const { }

#pragma dont_inline on
void TFruitLauncher::fireObj()
{
	// Address-of keeps the weak out-of-line copy.
	// -inline deferred would otherwise inline the call away.
	volatile MActor* (TLiveActor::*p)() const = &TLiveActor::getMActor;
	(void)p;
	// Keeps the retail camera name between riccoswitch and SurfGesoRed.
	// fireObj itself is still unmatched.
	volatile const char* cameraName = "フルーツタンクカメラカメラ";
	(void)cameraName;
}
#pragma dont_inline off

// Inlined pad. MWCC keeps it in the caller's frame (retail -0x108).
static inline void fruitLauncherFramePad()
{
	char trash[4];
	trash[0] = 0;
}

static inline void beginFruitSwitch(TFruitSwitch* sw)
{
	sw->startBck("riccoswitch");
	sw->onHitFlag(HIT_FLAG_NO_COLLISION);
	if (sw->mMapCollisionManager->unk8 != nullptr)
		sw->mMapCollisionManager->unk8->remove();
}

void TFruitLauncher::loadAfter()
{
	fruitLauncherFramePad();
	TMapObjBase::loadAfter();

	TResetFruit* fruit = (TResetFruit*)TMapObjBaseManager::newAndRegisterObj(
	    "FruitCoconut", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	fruit->unk1A4 = 1;
	fruit = (TResetFruit*)TMapObjBaseManager::newAndRegisterObj(
	    "FruitDurian", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	fruit->unk1A4 = 1;
	fruit = (TResetFruit*)TMapObjBaseManager::newAndRegisterObj(
	    "FruitPapaya", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	fruit->unk1A4 = 1;
	fruit = (TResetFruit*)TMapObjBaseManager::newAndRegisterObj(
	    "FruitPine", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	fruit->unk1A4 = 1;
	fruit = (TResetFruit*)TMapObjBaseManager::newAndRegisterObj(
	    "FruitBanana", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	fruit->unk1A4 = 1;

	unk138 = (TFruitSwitch*)JDrama::TNameRefGen::search("タンクスイッチＡ");
	unk138->unk138 = this;
	unk13C = (TFruitSwitch*)JDrama::TNameRefGen::search("タンクスイッチＢ");
	unk13C->unk138 = this;
	unk140         = 1;
	beginFruitSwitch(unk138);
}
