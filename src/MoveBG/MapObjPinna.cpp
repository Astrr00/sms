#define PINNA_EMIT_MOVEMTX
#include <MoveBG/MapObjPinna.hpp>
#include <MoveBG/MapObjManager.hpp>

#include <M3DUtil/MActor.hpp>
#include <M3DUtil/MActorUtil.hpp>
#include <JSystem/J3D/J3DGraphLoader/J3DModelLoaderFlags.hpp>
#include <System/Application.hpp>
#include <System/MarDirector.hpp>
#include <System/FlagManager.hpp>
#include <MoveBG/ItemManager.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <Map/MapCollisionEntry.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>
#include <Enemy/Conductor.hpp>
#include <Enemy/EffectObj.hpp>
#include <Map/MapData.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <M3DUtil/InfectiousStrings.hpp>
#include <string.h>
#include <stdlib.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <System/Particles.hpp>
#include <Map/Map.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

extern "C" s32 becomeCalmlyCallback__12TFerrisWheelFUlUl(u32, u32);

// Scheduling off keeps the unk140 load between getFrameCtrl's arg setup
// and the call, in source order.
static inline void stepWheel(TFerrisWheel* self)
{
#pragma scheduling off
	MActor* actor      = self->mMActor;
	f32 speed          = self->unk140;
	J3DFrameCtrl* ctrl = actor->getFrameCtrl(ANM_TYPE_BCK);
	f32 cur            = ctrl->getFrame();
	actor              = self->mMActor;
	actor->getFrameCtrl(ANM_TYPE_BCK)->setFrame(speed + cur);
#pragma scheduling on
}

static inline void wheelSound(TFerrisWheel* self)
{
	MSound* sound = gpMSound;
	if (sound->gateCheck(MSD_SE_OBJ_MAHRE_GATE_LIGHT))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_MAHRE_GATE_LIGHT, &self->mPosition, 0, &sound->unk80, 0,
		    4);
}

// Inlined trash lands under the flags. With the 4-byte pad in the else,
// moveObject's frame is -0xd8 and the spilled vec/flags sit at 0xac/0xa4/0xa0.
static inline void amiKingMovePad()
{
	char trash[0x48];
	(void)&trash;
}

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

void TFerrisWheel::control()
{
	// Holds the frame at -0x60. The store is dropped.
	char tail[0x8];
	tail[0] = 0;

	TMapObjBase::control();
	if (isState(2)) {
		if (!isStateTimerEngaged()) {
			f32 rate = SMSGetAnmFrameRate();
			rate *= 0.25f;
			if (unk140 > rate)
				unk140 -= 0.015f;
			else
				mState = STATE_NORMAL;
		}
	}

	f32 step = SMSGetAnmFrameRate();
	step *= 0.25f;
	if (unk140 > step)
		wheelSound(this);

	stepWheel(this);

	for (s32 i = 0; i < unk138; ++i) {
		TMapObjBase* car = unk13C[i];
		MtxPtr src      = getModel()->getAnmMtx(i + 1);
		MTXCopy(src, car->getModel()->getAnmMtx(0));
		f32 y = src[1][3] + car->mYOffset;
		f32 z = src[2][3];
		f32 x = src[0][3];
		car->mPosition.x = x;
		car->mPosition.y = y;
		car->mPosition.z = z;
	}
}

void TFerrisWheel::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = getModel()->getModelData()->getJointNum() - 1;
	unk13C = new TMapObjBase*[unk138];
	JGeometry::TVec3<f32> pos;
	JGeometry::TVec3<f32> rot;
	JGeometry::TVec3<f32> scale;
	const JGeometry::TVec3<f32>& rs = scale;
	const JGeometry::TVec3<f32>& rr = rot;
	const JGeometry::TVec3<f32>& rp = pos;
	for (u16 i = 0; i < unk138; ++i) {
		scale.x = 1.0f;
		scale.y = 1.0f;
		scale.z = 1.0f;
		rot.x = 0.0f;
		rot.y = 0.0f;
		rot.z = 0.0f;
		pos.x = 0.0f;
		pos.y = 0.0f;
		pos.z = 0.0f;
		unk13C[i] = TMapObjBaseManager::newAndRegisterObj("FerrisGondola", rp,
		                                                  rr, rs);
		unk13C[i]->appear();
	}
	if (gpMarDirector->unk7D == 2)
		unk140 = 10.0f;
	else {
		f32 rate = SMSGetAnmFrameRate();
		rate *= 0.25f;
		unk140 = rate;
	}
	char trash[0xC];
	trash[0] = 0;
}

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
void TViking::roll()
{
	char trash[0x30];
	(void)trash;

	switch (mState) {
	case 1:
		unk144 *= unk154;
		unk144 -= unk13C;
		unk148 += unk144;
		if (unk148 < 0.0f) {
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_PIN_BIKING_WING,
			                                   &mPosition, nullptr,
			                                   fabsf(unk144), 0, 0, nullptr, 0,
			                                   4);
			mState = 2;
		}
		if (unk148 > 180.0f) {
			unk148 -= 360.0f;
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_PIN_BIKING_WING,
			                                   &mPosition, nullptr,
			                                   fabsf(unk144), 0, 0, nullptr, 0,
			                                   4);
			mState = 4;
		}
		break;
	case 2:
		unk144 *= unk154;
		unk144 += unk13C;
		unk148 += unk144;
		if (unk148 > 0.0f) {
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_PIN_BIKING_WING,
			                                   &mPosition, nullptr,
			                                   fabsf(unk144), 0, 0, nullptr, 0,
			                                   4);
			mState = 1;
		}
		if (unk148 < -180.0f) {
			unk148 += 360.0f;
			gpMSound->startSoundActorWithInfo(MSD_SE_OBJ_PIN_BIKING_WING,
			                                   &mPosition, nullptr,
			                                   fabsf(unk144), 0, 0, nullptr, 0,
			                                   4);
			mState = 3;
		}
		break;
	case 3:
		unk144 *= unk158;
		unk144 -= unk13C;
		unk148 += unk144;
		if (unk148 < 0.0f) {
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_PIN_BIKING_WING, &mPosition, nullptr,
			    fabsf(unk144), 0, 0, nullptr, 0, 4);
			if (unk144 > -unk150)
				mState = 2;
			else
				mState = 4;
		}
		break;
	case 4:
		unk144 *= unk158;
		unk144 += unk13C;
		unk148 += unk144;
		if (unk148 > 0.0f) {
			gpMSound->startSoundActorWithInfo(
			    MSD_SE_OBJ_PIN_BIKING_WING, &mPosition, nullptr,
			    fabsf(unk144), 0, 0, nullptr, 0, 4);
			if (unk144 < unk150)
				mState = 1;
			else
				mState = 3;
		}
		break;
	default:
		break;
	}
}
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

#pragma dont_inline on
void TPinnaShell::control()
{
	char gap[8];
	(void)gap;
	Mtx rot;

	if ((s32)unk7C > 0)
		--unk7C;

	switch (unk68) {
	case 0:
		if (unk6C < 0.0f) {
			f32 unit = (f32)rand() * 0.000030517578f;
			unk6C += TShellCup::mCloseAccel * (0.5f + 0.5f * unit);
		} else {
			unk6C = 0.0f;
		}
		break;
	case 1:
		unk6C -= 0.8f;
		if (unk6C < -TShellCup::mOpenRotMax) {
			unk6C = -TShellCup::mOpenRotMax;
			unk7C = 360;
			TLiveActor* coin = (TLiveActor*)unk80;
			if (coin != nullptr && !coin->checkLiveFlag(LIVE_FLAG_DEAD)) {
				if (coin->isActorType(0x20000010))
					gpMSound->startSoundSystemSE(MSD_SE_SY_COLLECT_PRETTY, 0,
					                             nullptr, 0);
				else
					gpMSound->startSoundSystemSE(MSD_SE_SY_COIN_APPEAR, 0,
					                             nullptr, 0);
			} else {
				gpMSound->startSoundSystemSE(MSD_SE_SY_NOT_COLLECT, 0, nullptr,
				                             0);
			}
			unk68 = 2;
		}
		break;
	case 2:
		if ((s32)unk7C <= 0) {
			unk68 = 3;
			gpMSound->startSoundActor(MSD_SE_OBJ_PIN_SHELL_CLOSE, &mPosition,
			                          0, nullptr, 0, 4);
		}
		break;
	case 3:
		unk6C += unk70;
		if (unk6C >= -TShellCup::mShellDamageRot)
			((THitActor*)unk88)->offHitFlag(HIT_FLAG_NO_COLLISION);
		if (unk6C >= 0.0f) {
			unk6C = 0.0f;
			unk68 = 0;
			((THitActor*)unk88)->onHitFlag(HIT_FLAG_NO_COLLISION);
		}
		break;
	default:
		break;
	}

	MtxPtr shellMtx = (MtxPtr)unk74;
	THitActor* base = (THitActor*)unk8C;
	f32 mtxX  = shellMtx[0][3];
	f32 baseZ = base->mPosition.z;
	f32 baseX = base->mPosition.x;
	f32 mtxZ  = shellMtx[2][3];
	f32 mtxY  = shellMtx[1][3];
	f32 outX  = baseX + 0.7f * (mtxX - baseX);
	f32 outZ  = baseZ + 0.7f * (mtxZ - baseZ);
	f32 outY  = mtxY - 100.0f;
	mPosition.x     = outX;
	mPosition.y     = outY;
	mPosition.z     = outZ;

	((THitActor*)unk88)->mPosition.set(mPosition);

	if (mColCount != 0) {
		MsMtxSetRotX(rot, unk6C);
		TMapObjBase::concatOnlyRotFromRight((MtxPtr)unk74, rot, rot);
		((TMapCollisionMove*)unk84)->moveMtx(rot);
	}

	char trash[0x34];
	(void)trash;
}
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

// Loop lives in its own function: inlining it initializes the byte-offset
// induction with `addi` off the index. A for-loop written in perform emits
// `li` instead. The function pointer keeps the weak MsMtxSetRotX copy and
// a real `bl`; a direct call inlines the sine table.
static inline void spinShells(TShellCup* self, MtxPtr rot)
{
	for (int i = 0; i < 6; ++i) {
		TPinnaShell* shell = self->unk138 + i;
		void (*const setRotX)(MtxPtr, f32) = MsMtxSetRotX;
		setRotX(rot, shell->unk6C);
		MtxPtr shellMtx = (MtxPtr)shell->unk74;
		TMapObjBase::concatOnlyRotFromRight(shellMtx, rot, shellMtx);
	}
}

void TShellCup::perform(u32 cue, JDrama::TGraphics* graphics)
{
	Mtx rot;
	TMapObjBase::perform(cue, graphics);
	if (!(cue & CUE_CALC_ANIM))
		return;

	// Talk mode and demo mode are mutually exclusive, but both checks
	// are present: a talk-mode state returns before the shells update.
	if (gpMarDirector->isTalkModeNow()) {
		if (!gpMarDirector->isDemoModeNow())
			return;
	}

	spinShells(this, rot);

	TLiveActor* coin = (TLiveActor*)unk498;
	if (!coin->checkLiveFlag(LIVE_FLAG_DEAD)) {
		coin->mPosition.x = unk138[0].mPosition.x;
		coin->mPosition.y = unk138[0].mPosition.y;
		coin->mPosition.z = unk138[0].mPosition.z;
	}
	coin = (TLiveActor*)unk49C;
	if (!coin->checkLiveFlag(LIVE_FLAG_DEAD)) {
		coin->mPosition.x = unk138[2].mPosition.x;
		coin->mPosition.y = unk138[2].mPosition.y;
		coin->mPosition.z = unk138[2].mPosition.z;
	}
	coin = (TLiveActor*)unk4A0;
	if (!coin->checkLiveFlag(LIVE_FLAG_DEAD)) {
		coin->mPosition.x = unk138[4].mPosition.x;
		coin->mPosition.y = unk138[4].mPosition.y;
		coin->mPosition.z = unk138[4].mPosition.z;
	}
	// Dead slot so the rotation matrix stays at r1+0x28 (frame -0x68).
	char trash[12];
}

// Shifts the newAndRegisterObj temps up to r1+0x30 (frame -0x68).
static inline void reserveShellCupVecSlot()
{
	char pad[0x14];
	pad[0] = 0;
}

// director, then coin: flag check keeps the coin pointer in r4.
static inline bool shellCupBlueCoinGot(TMarDirector* director,
                                       TMapObjBase* coin)
{
	return TFlagManager::smInstance->getBlueCoinFlag(director->getCurrentMap(),
	                                                 coin->mEventId);
}

void TShellCup::loadAfter()
{
	reserveShellCupVecSlot();
	TMapObjBase::loadAfter();
	for (int i = 0; i < 6; ++i)
		TMapObjBase::joinToGroup("オブジェクトグループ",
		                         (THitActor*)unk138[i].unk88);

	unk498 = TMapObjBaseManager::newAndRegisterObj(
	    "coin_blue", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
	    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
	unk49C = gpItemManager->newAndRegisterCoinReal();
	unk4A0 = gpItemManager->newAndRegisterCoinReal();

	((TMapObjBase*)unk498)->mEventId = 2;
	if (!shellCupBlueCoinGot(gpMarDirector, (TMapObjBase*)unk498)) {
		((TMapObjBase*)unk498)->makeObjAppeared();
		unk138[0].unk80 = (u32)unk498;
	}

	((TMapObjBase*)unk49C)->makeObjAppeared();
	((TMapObjBase*)unk49C)->onMapObjFlag(MAP_OBJ_FLAG_UNK10000000);
	((TMapObjBase*)unk4A0)->makeObjAppeared();
	((TMapObjBase*)unk4A0)->onMapObjFlag(MAP_OBJ_FLAG_UNK10000000);
	unk138[2].unk80 = (u32)unk49C;
	unk138[4].unk80 = (u32)unk4A0;
}

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

void TBalloonKoopaJr::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	SMS_LoadParticle("/scene/mapObj/balloonKoopaJr.jpa", 0x5A);
	SMS_LoadParticle("/scene/mapObj/balloonKoopaJrA.jpa", 0x5B);
	SMS_LoadParticle("/scene/mapObj/balloonKoopaJrB.jpa", 0x5C);

	s32 idx = getModel()->getModelData()->getJointName()->getIndex("center");
	MtxPtr mtx = getModel()->getAnmMtx((u16)idx);
	// Declare y,z,x but assign z,y,x so MWCC loads 0x2c into f2
	// before 0x1c into f1.
	f32 y;
	f32 z;
	f32 x;
	z        = mtx[2][3];
	y        = mtx[1][3];
	x        = mtx[0][3];
	unk148.x = x;
	unk148.y = y;
	unk148.z = z;
}

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

void TAmiKing::initMapObj()
{
	TMapObjBase::initMapObj();
	initAnmSound();
	getMActor()->setBck("amiking_sleep1");
	setAnmSound("/scene/mapObj/amiking_sleep1.bas");
	offLiveFlag(LIVE_FLAG_UNK10);
	// Empty trip over the joints. MWCC keeps the counted loop.
	for (u8 i = 0; i < getMActor()->getModel()->getModelData()->getJointNum();
	     ++i)
		;
}

void TAmiKing::moveObject()
{
	amiKingMovePad();
	TLiveActor::moveObject();
	if (unk138 != 0) {
		if (getMActor()->checkCurAnm("amiking_flying1_start", ANM_TYPE_BCK)) {
			if (getMActor()->curAnmEndsNext())
				getMActor()->setBck("amiking_flying1_loop");
		} else if (mGroundPlane->isWaterSurface()) {
			if (!isAirborne()) {
				if (gpMSound->gateCheck(MSD_SE_EN_AMIKING_DIVE))
					MSoundSESystem::MSoundSE::startSoundActor(
					    MSD_SE_EN_AMIKING_DIVE, &mPosition, 0, nullptr, 0, 4);

				JPABaseEmitter* emitter
				    = gpMarioParticleManager->emitAndBindToMtxPtr(
				        0xCA, getMActor()->getModel()->getAnmMtx(0), 0,
				        nullptr);
				if (emitter != nullptr) {
					JGeometry::TVec3<f32> scale(4.0f, 4.0f, 4.0f);
					emitter->setGlobalScale(scale);
				}

				TEffectColumWater* column
				    = (TEffectColumWater*)gpConductor->makeOneEnemyAppear(
				        mPosition, "エフェクト水柱マネージャー", 1);
				if (column != nullptr) {
					JGeometry::TVec3<f32> scale(4.0f, 4.0f, 4.0f);
					column->generate(mPosition, scale);
				}

				gpItemManager->makeShineAppearWithDemo(
				    "シャイン（観覧車シャイン用）", "観覧車シャインカメラ",
				    mPosition.x, mPosition.y, mPosition.z);

				TFerrisWheel* wheel = (TFerrisWheel*)JDrama::TNameRefGen::search(
				    "FerrisWheel");
				SMSGetMarDirector()->fireStartDemoCamera(
				    "観覧車正常化カメラ", &wheel->mPosition, -1, 0.0f, true,
				    becomeCalmlyCallback__12TFerrisWheelFUlUl, (u32)wheel,
				    nullptr, JDrama::TFlagT<u16>(0));
				kill();
			}
		}
	} else {
		TMapObjBase* actor = (TMapObjBase*)mGroundPlane->mActor;
		if (actor != nullptr && actor->mActorType == 0x4000006A) {
			bool wake;
			// Volatile load: keeps the first compare from being reused, so
			// isState(5) reloads mState.
			if ((*(volatile u16*)&actor->mState == 3 ? true : false)
			    || actor->isState(5) || actor->isState(4)
			    || actor->isState(6))
				wake = true;
			else
				wake = false;
			if (wake) {
				unk138 = 1;
				setVelocityAndFlag10(5.0f, 10.0f, -10.0f);
				getMActor()->setBck("amiking_flying1_start");
				setAnmSound(nullptr);
				SMSGetMarDirector()->fireStartDemoCamera(
				    "観覧車ボス撃沈カメラ", &mPosition, -1, 0.0f, true, nullptr,
				    0, nullptr, JDrama::TFlagT<u16>(0));
			}
		}
		char pad[4];
	}
}

void TAmiKing::calcRootMatrix()
{
	TMapObjBase::calcRootMatrix();
	gpMarioParticleManager->emitAndBindToMtxPtr(0x184, getModel()->getAnmMtx(0),
	                                            1, this);
	if (unk138 == 0) {
		char pad[8];
		JGeometry::TVec3<f32> off;
		Mtx rot;
		char trash[0x14];
		MtxPtr joint = getMActor()->getModel()->getAnmMtx(6);
		unk13C.set(joint[0][3], joint[1][3], joint[2][3]);

		off.x = 0.0f;
		off.y = 0.0f;
		off.z = 200.0f;
		MsMtxSetRotRPH(rot, 0.0f, mRotation.y, 0.0f);
		MTXMultVec(rot, &off, &off);
		unk13C += off;

		JPABaseEmitter* emitter = gpMarioParticleManager->emitAndBindToPosPtr(
		    PARTICLE_MS_POI_ZZZ, &unk13C, 1, this);
		if (emitter != nullptr) {
			JGeometry::TVec3<f32> scale(2.0f, 2.0f, 2.0f);
			emitter->setGlobalScale(scale);
		}

		if (gpMSound->gateCheck(MSD_SE_EN_AMIKING_SPARK))
			MSoundSESystem::MSoundSE::startSoundActor(MSD_SE_EN_AMIKING_SPARK,
			                                          &mPosition, 0, nullptr, 0,
			                                          4);
	} else if (gpMSound->gateCheck(MSD_SE_EN_AMIKING_FLY)) {
		MSoundSESystem::MSoundSE::startSoundActor(MSD_SE_EN_AMIKING_FLY,
		                                          &mPosition, 0, nullptr, 0, 4);
	}
}

void TAmiKing::bind()
{
	if (checkLiveFlag(LIVE_FLAG_UNK10))
		gpMap->checkGround(mPosition.x, mPosition.y + mHeadHeight, mPosition.z,
		                   &mGroundPlane);
	else
		TLiveActor::bind();
}

void TAmiKing::touchPlayer(THitActor*) { SMS_SendMessageToMario(this, 9); }

// One inline deep so TUtil::sqrt stays a call. length() expands to frsqrte.
static inline f32 coasterSqrt(f32 lenSq)
{
	return JGeometry::TUtil<f32>::sqrt(lenSq);
}

static s32 switchSnd;

void TPinnaCoaster::control()
{
	// self is declared before the rail pointer so it stays in r31.
	TPinnaCoaster* self = this;
	self->TMapObjBase::control();
	self->unk138->frameUpdate();
	self->unk138->calc();
	MtxPtr rail = self->unk138->getModel()->getAnmMtx(0);
	// Base TR mtx is model+0x20. A folded getBaseTRMtx() is one addi;
	// adding after the pointer copy keeps the retail split.
	J3DModel* model = self->getModel();
	char* base      = (char*)model;
	base += 0x20;
	MTXCopy(rail, (MtxPtr)base);
	self->mMActor->frameUpdate();
	self->mMActor->calc();

	// Load z, y, x so the translation column lands in f2, f1, f0.
	MtxPtr mtx = self->getModel()->getAnmMtx(0);
	f32 y;
	f32 z;
	f32 x;
	z                 = mtx[2][3];
	y                 = mtx[1][3];
	x                 = mtx[0][3];
	self->mPosition.x = x;
	self->mPosition.y = y;
	self->mPosition.z = z;

	// gap is the 4 bytes between the vecs. tail holds frame -0x88.
	JGeometry::TVec3<f32> delta;
	char gap[4];
	JGeometry::TVec3<f32> pos;
	char tail[0x20];
	gap[0]  = 0;
	tail[0] = 0;
	pos = self->mPosition;
	pos.sub(self->unk140);
	delta    = pos;
	f32 dist = coasterSqrt(delta.squared());
	if (switchSnd != 0) {
		if (gpMSound->gateCheck(MSD_SE_OBJ_JET_COASTER))
			MSoundSESystem::MSoundSE::startSoundActorWithInfo(
			    MSD_SE_OBJ_JET_COASTER, &self->mPosition, nullptr, dist, 0, 0,
			    nullptr, 0, 4);
	}
	switchSnd ^= 1;
	self->unk140.x = self->mPosition.x;
	self->unk140.y = self->mPosition.y;
	self->unk140.z = self->mPosition.z;
}

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
