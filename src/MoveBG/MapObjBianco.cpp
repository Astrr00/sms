#include <MoveBG/MapObjBianco.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
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
#include <Player/WaterGun.hpp>
#include <Camera/CubeManagerBase.hpp>
#include <System/Application.hpp>
#include <System/Particles.hpp>
#include <stdlib.h>
#include <string.h>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <M3DUtil/InfectiousStrings.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TBigWindmill::control() { }

void TBigWindmill::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	for (int i = 0; i < 4; ++i) {
		unk138[i] = TMapObjBaseManager::newAndRegisterObj(
		    "bigWindmillBlock", JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
		    JGeometry::TVec3<f32>(0.0f, 0.0f, 0.0f),
		    JGeometry::TVec3<f32>(1.0f, 1.0f, 1.0f));
		unk138[i]->appear();
		unk138[i]->getModel()->calc();
	}
}

void TMapObjRootPakkun::drawObject(JDrama::TGraphics* graphics)
{
	TLiveActor::drawObject(graphics);
	if (fabsf(gpMarioPos->z - mPosition.z) < 10000.0f) {
		unk138->movement();
		if (!isStateTimerEngaged()) {
			unk138->tremble(mTremblePower, mTrembleAccel, mTrembleBrake,
			                mTrembleTime);
			mStateTimer = mTrembleTime;
		}
	}
	char trash[1];
	trash[0] = 0;
}

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

u32 TBiancoWatermillVertical::touchWater(THitActor* actor)
{
	if (getWaterPlane(actor) == nullptr) {
		unk144 = 1;
		return 0;
	}
	if ((u8)waterHitPlane(actor) == 0)
		return 0;

	const JGeometry::TVec3<f32>& waterPos = getWaterPos(actor);
	JGeometry::TVec3<f32> speed;
	JGeometry::TVec3<f32> dir;
	const JGeometry::TVec3<f32>& waterSpeed = getWaterSpeed(actor);
	f32 wz                                  = waterSpeed.z;
	f32 wx                                  = waterSpeed.x;
	speed.x                                 = wx;
	speed.y                                 = 0.0f;
	speed.z                                 = wz;
	if (speed.x != 0.0f || speed.z != 0.0f)
		MsVECNormalize(&speed, &speed);

	getVerticalVecToTargetXZ(waterPos.x, waterPos.z, &dir);
	MsVECNormalize(&dir, &dir);

	f32 radius = mBodyRadius;
	f32 dist   = getDistanceXZ(waterPos);
	f32 ratio  = (radius - dist) / radius;
	f32 dot = speed.z * dir.z + (speed.x * dir.x + speed.y * dir.y);
	if (dot > 0.0f) {
		if (unk138 < mRotSpeedMax)
			unk138 = mRotAccel * ratio + unk138;
	} else if (unk138 > -mRotSpeedMax) {
		unk138 = unk138 - mRotAccel * ratio;
	}
	return 1;
}

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

void TBiancoWatermillVertical::control()
{
	char trash[1];
	trash[0] = 0;
	if (unk138 != unk13C) {
		if (unk138 > unk13C) {
			unk138 -= mRotSpeedDownRate;
			if (unk138 < unk13C)
				unk138 = unk13C;
		} else {
			unk138 += mRotSpeedDownRate;
			if (unk138 > unk13C)
				unk138 = unk13C;
		}
	}

	mRotation.y += unk138;
	mRotation.y = MsWrap(mRotation.y, 0.0f, 360.0f);

	f32 delta = unk138 * mBridgeRotRate;
	((TMapObjBase*)unk140)->mRotation.y += delta;
	f32* rotY = &((TMapObjBase*)unk140)->mRotation.y;
	*rotY     = MsWrap(*rotY, 0.0f, 360.0f);

	SMSGetMSound()->startSoundActorWithInfo(
	    MSD_SE_OBJ_BI_STEPMILL_WIND, &mPosition, nullptr, fabsf(unk138), 0, 0,
	    (JAISoundHandle*)&unk148, 0, 4);
	SMSGetMSound()->startSoundActorWithInfo(
	    MSD_SE_OBJ_BI_STEPMILL_MOVE, &((TMapObjBase*)unk140)->mPosition,
	    nullptr, fabsf(delta), 0, 0, (JAISoundHandle*)&unk14C, 0, 4);
}

void TBiancoWatermillVertical::loadAfter()
{
	TMapObjBase::loadAfter();
	if (strcmp(mName, "BiaWatermillVertical 0") == 0)
		unk140 = (u32)JDrama::TNameRefGen::search("BiaTurnBridge 0");
	else
		unk140 = (u32)JDrama::TNameRefGen::search("BiaTurnBridge 1");
	mBodyRadius = 1000.0f;
}

void TBiancoWatermillVertical::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	stream >> unk13C;
	unk13C /= 1000.0f;
	unk138 = unk13C;
}

TBiancoWatermillVertical::TBiancoWatermillVertical(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0)
    , unk144(0)
    , unk148(0)
    , unk14C(0)
{
	// Dead slot so MWCC keeps frame -0x28 (r31 at r1+0x24).
	char trash[8];
	trash[0] = 0;
}

static f32 sMessengerPosZ = 200.0f;
static f32 sMessengerPosY = 6400.0f;

u32 TBiancoMiniWindmill::touchWater(THitActor* actor)
{
	char pad[8];
	pad[0] = 0;
	const JGeometry::TVec3<f32>& waterPos = getWaterPos(actor);
	if (waterPos.y < mPosition.y + sMessengerPosY - 300.0f)
		return 1;

	const JGeometry::TVec3<f32>& water = getWaterSpeed(actor);
	MtxPtr mtx = getModel()->getAnmMtx(0);
	if (water.z * mtx[2][2] + (water.x * mtx[0][2] + water.y * mtx[1][2])
	    > 0.0f)
		return 0;

	unk154 += mRotWaterAccel;
	if (unk154 > mRotSpeedMax) {
		unk154 = mRotSpeedMax;
		JGeometry::TVec3<f32> point(mPosition.x,
		                            550.0f + unk15C->mPosition.y, mPosition.z);
		mAppearSpeed = 0.0f;
		appearObjFromPoint(point);
	}
	char trash[0x20];
	trash[0] = 0;
	return 1;
}

void TBiancoMiniWindmill::calc() { }

f32 TMapObjRootPakkun::mTremblePower = 15.0f;
f32 TMapObjRootPakkun::mTrembleAccel = 0.95f;
f32 TMapObjRootPakkun::mTrembleBrake = 0.98f;
int TMapObjRootPakkun::mTrembleTime  = 0x168;

f32 TBiancoWatermillVertical::mRotAccel         = 0.15f;
f32 TBiancoWatermillVertical::mRotSpeedDownRate = 0.005f;
f32 TBiancoWatermillVertical::mRotSpeedMax      = 3.0f;
f32 TBiancoWatermillVertical::mBridgeRotRate    = 0.03f;

f32 TBiancoMiniWindmill::mRotWaterAccel = 0.01f;
f32 TBiancoMiniWindmill::mFriction      = 0.01f;
f32 TBiancoMiniWindmill::mRotSpeedMax   = 10.0f;

void TBiancoMiniWindmill::control()
{
	if (unk154 > unk158)
		unk154 -= mFriction;
	else
		unk154 = unk158;
	unk150 += unk154;
	unk150 = MsWrap(unk150, 0.0f, 360.0f);
}

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

void TLeafBoat::control()
{
	TMapObjBase::control();
	if (marioHipAttack())
		mVelocity.y -= unk154;
	if (marioIsOn()) {
		mVelocity.y -= unk150;
		s32 emitting = SMS_GetMarioWaterGun()->mIsEmitWater;
		if (emitting > 0) {
			MtxPtr emitMtx = SMS_GetMarioWaterGun()->getEmitMtx(0);
			mVelocity.x -= emitMtx[0][0] * unk144;
			mVelocity.z -= emitMtx[2][0] * unk144;
		}
	}

	int cube = gpCubeStream->getInCubeNo(mPosition);
	if (cube != -1) {
		TCubeStreamInfo& info
		    = (TCubeStreamInfo&)*gpCubeStream->unk14->begin()[cube];
		Mtx mtx;
		MsMtxSetXYZRPH(mtx, 0.0f, 0.0f, 0.0f, info.unk18.x, info.unk18.y,
		               info.unk18.z);
		f32 scale = 0.0001f * info.unk40;
		mVelocity.x += mtx[0][2] * scale;
		mVelocity.z += mtx[2][2] * scale;
	}

	mPosition.y += mVelocity.y;
	mVelocity.y
	    += unk158 * (mInitialPosition.y - (mPosition.y - mYOffset));
	mVelocity.y *= unk15C;
	mVelocity.x *= unk148;
	mVelocity.z *= unk148;

	// Dead slot so MWCC keeps frame -0xa0 (mtx at r1+0x4c).
	char trash[0x2c];
	trash[0] = 0;
}

void TLeafBoat::calc()
{
	if (unk144 != 0.0f) {
		if (unk160 > 8) {
			if (fabsf(mVelocity.x) + fabsf(mVelocity.z) > 0.1f) {
				f32 py   = mPosition.y - mYOffset;
				f32 pz   = mPosition.z;
				f32 px   = mPosition.x;
				unk164.x = px;
				unk164.y = py;
				unk164.z = pz;
				JGeometry::TVec3<f32> scale;
				scale.setAll(2.0f);
				emitAndBindScale(PARTICLE_MS_M_HAMON_B, 3, &unk164, scale);
				emitAndBindScale(PARTICLE_MS_M_HAMON_A, 1, &unk164, scale);
				// Dead slot so MWCC keeps frame -0x38 (scale at r1+0x20).
				char trash[8];
				trash[0] = 0;
			}
			unk160 = 0;
		} else {
			unk160 += 1;
		}
	}
}

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

void TLampSeesaw::load(JSUMemoryInputStream& stream)
{
	f32 height;
	s32 pad;
	TMapObjBase::load(stream);
	stream.read(&height, 4);
	unk13C = mInitialPosition.y - height;
	stream.read(&unk140, 4);
	unk140 *= 0.0001f;
	// Dead s32 so the height spill stays at r1+0x14 (frame stays -0x20).
	pad = 0;
}

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

void TLampSeesawMain::loadAfter()
{
	char trash[1];
	char nameBuf[0x40];
	trash[0]     = 0;
	size_t len   = strlen("ランプシーソーＡ");
	char suffix0 = mName[len];
	char suffix1 = mName[len + 1];
	char suffix2 = mName[len + 2];
	char suffix3 = mName[len + 3];
	snprintf(nameBuf, 0x40, "ランプシーソーＢ００");
	nameBuf[len]     = suffix0;
	nameBuf[len + 1] = suffix1;
	nameBuf[len + 2] = suffix2;
	nameBuf[len + 3] = suffix3;

	unk138 = (TLampSeesaw*)JDrama::TNameRefGen::search(nameBuf);
	unk138->unk138 = this;
}

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

void TBellWatermill::loadAfter()
{
	TMapObjTurn::loadAfter();
	unk150 = 2;
	unk15C = -0.02f;
	unk160 = -0.008f;
	unk164 = 10.0f;
	unk18C = 10.0f;
	unk174 = 1000.0f;
	unk180 = 0.15f;
	unk184 = 0.1f;
	unk16C = 4.0f;
	unk188 = 0.5f;
	unk17C = 1.0f;
	unk194 = (TBiancoBell*)JDrama::TNameRefGen::search("BiaBell 0");
	unk198 = (TBiancoBell*)JDrama::TNameRefGen::search("BiaBell 1");
	unk19C = (TBiancoBell*)JDrama::TNameRefGen::search("BiaBell 2");
	unk1A0 = 1;
}

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
