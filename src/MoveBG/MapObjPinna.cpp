#define PINNA_EMIT_MOVEMTX
#include <MoveBG/MapObjPinna.hpp>
#include <MoveBG/MapObjManager.hpp>

#include <M3DUtil/MActor.hpp>
#include <M3DUtil/MActorUtil.hpp>
#include <JSystem/J3D/J3DGraphLoader/J3DModelLoaderFlags.hpp>
#include <System/Application.hpp>
#include <System/FlagManager.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <Map/MapCollisionEntry.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <System/Particles.hpp>
#include <Map/Map.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

f32 TShellCup::mOpenRotMax      = 90.0f;
f32 TShellCup::mShellDamageRot = 45.0f;
f32 TShellCup::mWaterOpenAccel = 5.0f;
f32 TShellCup::mCloseAccel     = 3.5f;

s32 TFerrisWheel::becomeCalmlyCallback(u32 param_1, u32)
{
	if (param_1 == 0) {
		mState = 2;
		MSound* sound = gpMSound;
		if (sound->unk80 != nullptr) {
			sound->unk80->setVolume(0.0f, 200, 0);
			sound->unk80->setPitch(0.5f, 200, 0);
		}
		mStateTimer = 120;
	}
	return 0;
}

void TFerrisWheel::control() { }

void TFerrisWheel::initMapObj() { }

TFerrisWheel::TFerrisWheel(const char* name)
    : TMapObjBase(name)
    , unk138(0)
    , unk13C(0)
    , unk140(0.0f)
{
}

void THorizontalViking::updateTrans() { }

void THorizontalViking::moveNormal() { }

void THorizontalViking::control()
{
	TMapObjBase::control();
	switch (mState) {
	case 1:
		unk144 -= unk13C;
		unk148 += unk144;
		if (unk148 < 0.0f)
			mState = 2;
		break;
	case 2:
		unk144 += unk13C;
		unk148 += unk144;
		if (unk148 > 0.0f)
			mState = 1;
		break;
	}
	mPosition.x
	    = unk138 * sinf(3.14f * (unk148 / 180.0f)) + mInitialPosition.x;
	f32 yOff = mYOffset;
	mPosition.y
	    = yOff
	      + (unk138 * (1.0f - cosf(3.14f * (unk148 / 180.0f)))
	         + mInitialPosition.y);
	char trash[4];
	trash[0] = 0;
}

void THorizontalViking::reset()
{
	unk144 = unk140;
	unk148 = 0.0f;
	if (unk144 > 0.0f)
		mState = 1;
	else
		mState = 2;
}

void THorizontalViking::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = 2500.0f;
	unk13C = 0.0008f;
	unk140 = 0.23f;
	reset();
}

THorizontalViking::THorizontalViking(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.0f)
    , unk148(0.0f)
{
}

#pragma dont_inline on
void TViking::roll() { }
#pragma dont_inline off

void TViking::control()
{
	switch (unk14C) {
	case 0:
		switch (mState) {
		case 1:
			unk144 -= unk13C;
			unk148 += unk144;
			if (unk148 < 0.0f)
				mState = 2;
			break;
		case 2:
			unk144 += unk13C;
			unk148 += unk144;
			if (unk148 > 0.0f)
				mState = 1;
			break;
		}
		break;
	case 1:
		roll();
		break;
	}
	mPosition.x
	    = unk138 * sinf(3.14f * (unk148 / 180.0f)) + mInitialPosition.x;
	f32 yOff = mYOffset;
	mPosition.y
	    = yOff
	      + (unk138 * (1.0f - cosf(3.14f * (unk148 / 180.0f)))
	         + mInitialPosition.y);
	mRotation.z = unk148;
	updateObjMtx();
	char trash[4];
	trash[0] = 0;
}

void TViking::reset()
{
	unk144 = unk140;
	unk148 = 0.0f;
	if (unk140 > 0.0f)
		mState = 1;
	else
		mState = 2;
}

void TViking::loadAfter()
{
	TMapObjBase::loadAfter();
	reset();
}

void TViking::initMapObj()
{
	unk14C = 1;
	if (strcmp(getName(), "viking 0") == 0) {
		unk138 = 1400.0f;
		unk13C = 0.001f;
		unk154 = 1.001f;
		unk158 = 0.999f;
		unk140 = -0.3f;
		unk150 = 0.3f;
	} else {
		unk138 = 1400.0f;
		unk13C = 0.001f;
		unk154 = 1.001f;
		unk158 = 0.999f;
		unk140 = 0.3f;
		unk150 = 0.3f;
	}
	mPosition.y -= unk138;
	TMapObjBase::initMapObj();
}

TViking::TViking(const char* name)
    : THorizontalViking(name)
    , unk14C(0)
    , unk150(0.0f)
    , unk154(0.0f)
    , unk158(0.0f)
{
}

void TPinnaShell::opened() { }

BOOL TPinnaShell::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_SPRAYED_BY_WATER) {
		gpMarioParticleManager->emit(PARTICLE_MS_ENM_WATHIT, &sender->mPosition,
		                             0, nullptr);
		SMSGetMSound()->startSoundSet(MSD_SE_EN_COMMON_W_HIT_OK, &mPosition, 0,
		                              0.0f, 0, 0, 4);
		if (unk68 == 0) {
			unk6C -= TShellCup::mWaterOpenAccel;
			if (unk6C < -TShellCup::mOpenRotMax)
				unk68 = 1;
		}
		return TRUE;
	}
	return FALSE;
}

// Retail keeps a weak copy in this TU. The shared header stays the virtual
// inline so other matches that call through the vtable do not change.
__declspec(weak) void TMapCollisionMove::moveMtx(MtxPtr mtx)
{
	MTXCopy(mtx, unk20);
	move();
}

// dont_inline: the retail body is large. The empty stub must stay a call
// so TShellCup::control can match.
#pragma dont_inline on
void TPinnaShell::control() { }
#pragma dont_inline off

TPinnaShell::TPinnaShell(const char* name)
    : THitActor(name)
{
}

void TShellCup::control()
{
	getMActor()->calc();
	for (int i = 0; i < 6; ++i)
		unk138[i].control();
}

void TShellCup::attachCoin(TCoin*, int) { }

void TShellCup::calcAfter() { }

void TShellCup::perform(u32, JDrama::TGraphics*)
{
	// Address-of keeps the weak out-of-line copy.
	void (*volatile rot)(MtxPtr, f32) = &MsMtxSetRotX;
	(void)rot;
}

void TShellCup::loadAfter() { }

void TShellCup::initMapObj() { }

TShellCup::TShellCup(const char* name)
    : TMapObjBase(name)
    , unk498(nullptr)
    , unk49C(nullptr)
    , unk4A0(nullptr)
{
}

void TMerrygoround::control() { }

void TMerrygoround::draw() const { }

void TMerrygoround::initMapObj() { }

TMerrygoround::TMerrygoround(const char* name)
    : TMapObjBase(name)
{
	char trash[1];
	trash[0] = 0;
	unk1A0 = 0;
	unk1A4 = 0;
	unk138 = 0;
	unk140 = 0;
	unk13C = 0;
	unk142 = 0;
	for (int i = 0; i < 9; ++i) {
		unk144[i] = 0;
		unk18C[i] = 0;
		unk168[i] = 0;
	}
}

void TChangeStageMerrygoround::touchPlayer(THitActor* actor)
{
	if (isStateTimerEngaged())
		return;

	if (SMS_GetYoshi()->mType == 1) {
		if (gpMSound->gateCheck(MSD_SE_SY_COLLECT_YOSHI))
			MSoundSESystem::MSoundSE::startSoundSystemSE(
			    MSD_SE_SY_COLLECT_YOSHI, 0, nullptr, 0);
		TMapObjChangeStage::touchPlayer(actor);
		unk13C = 1;
	} else if (gpMSound->gateCheck(MSD_SE_SY_NOT_COLLECT_YOSHI)) {
		MSoundSESystem::MSoundSE::startSoundSystemSE(
		    MSD_SE_SY_NOT_COLLECT_YOSHI, 0, nullptr, 0);
	}

	mStateTimer = 0x258;

	// Dead slot so MWCC keeps frame -0x30.
	char trash[0xF];
	trash[0] = 0;
}

void TChangeStageMerrygoround::calc()
{
	if (unk13C != 0) {
		// Local loads gpMarioPos into r5 before the manager.
		JGeometry::TVec3<f32>* pos = gpMarioPos;
		gpMarioParticleManager->emitAndBindToPosPtr(0x100, pos, 1, this);
		pos = gpMarioPos;
		gpMarioParticleManager->emitAndBindToPosPtr(0x101, pos, 1, this);
	}
}

void TBalloonKoopaJr::touchActor(THitActor*) { kill(); }

void TBalloonKoopaJr::kill()
{
	TMapObjGeneral::kill();
	emitAndScale(0x5A, 0, &unk148);
	emitAndScale(0x5B, 0, &unk148);
	emitAndScale(0x5C, 0, &unk148);
	TFlagManager::smInstance->incFlag(0x60001, 1);
	if (gpMSound->gateCheck(MSD_SE_BS_BSPAKU_SLAP))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_BS_BSPAKU_SLAP, &mPosition, 0, nullptr, 0, 4);

	// Dead slot so MWCC keeps frame -0x20.
	char trash[1];
	trash[0] = 0;
}

void TBalloonKoopaJr::load(JSUMemoryInputStream&) { }

void TPinnaEntrance::loadAfter()
{
	TMapObjBase::loadAfter();
	JGeometry::TVec3<f32> rot(90.0f, 0.0f, 0.0f);
	TMapObjBaseManager::newAndRegisterObj("GateManta", mPosition, rot);
}

void TWaterRecoverObj::touchPlayer(THitActor* actor)
{
	if (actor->isActorType(0x80000001) && !isStateTimerEngaged()) {
		actor->receiveMessage(this, HIT_MESSAGE_ATTACK);
		mStateTimer = 0x258;
	}
}

void TAmiKing::loadAfter()
{
	TMapObjBase::loadAfter();
	SMS_LoadParticle("/scene/Mapobj/amiking.jpa", 0x184);
}

void TAmiKing::initMapObj() { }

void TAmiKing::moveObject() { }

void TAmiKing::calcRootMatrix() { }

void TAmiKing::bind()
{
	if (checkLiveFlag(LIVE_FLAG_UNK10))
		gpMap->checkGround(mPosition.x, mPosition.y + mHeadHeight, mPosition.z,
		                   &mGroundPlane);
	else
		TLiveActor::bind();
}

void TAmiKing::touchPlayer(THitActor*) { SMS_SendMessageToMario(this, 9); }

void TPinnaCoaster::control() { }

void TPinnaCoaster::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = SMS_MakeMActorWithAnmData(
	    "/scene/mapObj/CoasterRail.bmd", mManager->getMActorAnmData(), 3,
	    J3DMLF_MaterialPEFull | J3DMLF_UseUniqueMaterials
	        | (1 << J3DMLF_TevStageNumShift));
	unk138->setBck("coasterrail");
	MsMtxSetXYZRPH(unk138->getModel()->getBaseTRMtx(), mPosition.x, mPosition.y,
	               mPosition.z, mRotation.x, mRotation.y, mRotation.z);
	f32 rate = SMSGetAnmFrameRate();
	rate *= 0.25f;
	unk138->getFrameCtrl(ANM_TYPE_BCK)->setRate(rate);
	unk140.x = mPosition.x;
	unk140.y = mPosition.y;
	unk140.z = mPosition.z;
}

TPinnaCoaster::TPinnaCoaster(const char* name)
    : TMapObjBase(name)
    , unk138(0)
{
	unk140.zero();
}
