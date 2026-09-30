#include <MoveBG/MapObjMare.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <M3DUtil/MActor.hpp>
#include <Map/MapWireManager.hpp>
#include <MoveBG/ItemManager.hpp>
#include <System/Particles.hpp>
#include <MSound/MSound.hpp>
#include <System/MarDirector.hpp>
#include <Player/ModelWaterManager.hpp>
#include <Map/MapData.hpp>
#include <Map/MapEventMare.hpp>
#include <dolphin/gx.h>
#include <JSystem/J3D/J3DGraphBase/J3DSys.hpp>

extern void MsMtxSetTRS(MtxPtr result, f32 x, f32 y, f32 z, f32 r, f32 p, f32 h,
                        f32 sx, f32 sy, f32 sz);

class TCannon {
public:
	bool isObject();
	void startChorobeiShout();
};

static JGeometry::TVec3<f32> fall_upper_pos(2827.0f, 8604.0f, 7202.0f);

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <M3DUtil/InfectiousStrings.hpp>

// rogue rodata so @2690/@2692 sit ahead of the shine strings
static const char rogueRodata2690[0xc] = { 0 };
static const f32 rogueRodata2692[3]    = { 1.0f, 1.0f, 1.0f };

// -inline deferred: source order is the reverse of mario.MAP emission order.

f32 TCogwheelScale::mWaterLeakSpeed = 0.01f;
static f32 sRadius                  = 800.0f;
f32 TCogwheel::mRopeWidthX          = 10.0f;
f32 TCogwheel::mRopeWidthZ          = 7.0f;
f32 TCogwheel::mTexPosRate          = 0.01f;
f32 TCogwheel::mMinSpeed            = 3.0f;
static f32 mGrowStartFrame          = 90.0f;
static f32 mGrowEndFrame            = 175.0f;

u32 TCogwheelScale::touchWater(THitActor*)
{
	if (unk140 < unk144)
		unk140 += 1.0f;
	return 1;
}

// Inline return lands in f1 and the reference is lfsu.
// A direct += is lfs/stfs and swaps the fadds operands.
static inline f32 takeScale(f32& slot) { return slot; }

BOOL TCogwheelScale::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_HIP_DROP) {
		f32 inc = takeScale(unk158->unk138);
		f32 base = unk150;
		unk158->unk138 = base + inc;
		return TRUE;
	}
	return TMapObjBase::receiveMessage(sender, message);
}

void TCogwheelScale::touchPlayer(THitActor*) { }

void TCogwheelScale::control()
{
	unk148 = 0.0f;
	TMapObjBase::control();
	if (unk140 > 0.0f) {
		unk140 -= mWaterLeakSpeed;
		gpMSound->startSoundActorWithInfo(
		    MSD_SE_OBJ_MR_TSUBO_WATER, &mPosition, nullptr, fabsf(unk140), 0,
		    0, nullptr, 0, 4);
		if (unk140 < 0.0f)
			unk140 = 0.0f;
	}
	char trash[4];
	trash[0] = 0;
}

TCogwheelScale::TCogwheelScale(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.0f)
    , unk148(0.0f)
    , unk14C(0.01f)
    , unk150(5.0f)
    , unk154(0)
    , unk158(nullptr)
{
}

// Dead inline: a 0xC local under the mat-color temp, 0x4 return above it.
// No instructions. Cogwheel::initDraw frame is -0x80; elastic draw is 0x38.
struct TElasticLow {
	char c[0xC];
};
struct TElasticHigh {
	char c[4];
};
static inline TElasticHigh elasticPad()
{
	TElasticLow low;
	return *(TElasticHigh*)(void*)&low;
}

void TCogwheel::initDraw() const
{
	elasticPad();
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXLoadPosMtxImm(j3dSys.getViewMtx(), GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, (GXColor) { 0x00, 0x00, 0x64, 0xff });
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3c, GX_FALSE,
	                  0x7d);
	JUTTexture texture(gpMapObjManager->unkC8);
	texture.load(GX_TEXMAP0);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO,
	                GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO,
	                GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GXSetCullMode(GX_CULL_BACK);
}

void TCogwheel::draw() const { }

void TCogwheel::rebound() { }

void TCogwheel::calc()
{
	Mtx mtxZ;
	Mtx mtxY;
	mRotation.z = 360.0f * (-unk13C / (3.14f * (2.0f * sRadius)));
	makeRootMtxRotZ(mtxZ);
	mtxZ[0][3] = 0.0f;
	mtxZ[1][3] = 0.0f;
	mtxZ[2][3] = 0.0f;
	makeRootMtxRotY(mtxY);
	mtxY[0][3] = 0.0f;
	mtxY[1][3] = 0.0f;
	mtxY[2][3] = 0.0f;
	MtxPtr anm = getModel()->getAnmMtx(0);
	MTXConcat(mtxY, mtxZ, anm);
	anm[0][3] = mPosition.x;
	anm[1][3] = mPosition.y;
	anm[2][3] = mPosition.z;
	char trash[4];
	trash[0] = 0;
}

void TCogwheel::control() { }

void TCogwheel::initMapObj() { }

TCogwheel::TCogwheel(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.0f)
    , unk148(0.0f)
    , unk14C(0.0f)
    , unk150(0)
    , unk160(0.0f)
    , unk164(0)
    , unk174(0.0f)
{
	unk154.zero();
	unk168.zero();
}

void TMapObjElasticCode::draw() const
{
	elasticPad();
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXLoadPosMtxImm(j3dSys.getViewMtx(), GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, (GXColor) { 0x00, 0x00, 0x64, 0xff });
	GXSetNumTexGens(0);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO,
	                GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO,
	                GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
	GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
	GXSetCullMode(GX_CULL_NONE);
	GXSetLineWidth(0x18, GX_TO_ZERO);
	GXBegin(GX_LINES, GX_VTXFMT0, 2);
	GXPosition3f32(mInitialPosition.x, mInitialPosition.y + 1000.0f,
	               mInitialPosition.z);
	GXPosition3f32(mPosition.x, mPosition.y, mPosition.z);
}

void TMapObjElasticCode::control()
{
	TMapObjBase::control();
	mVelocity.y *= unk140;
	mVelocity.y += unk13C * (mInitialPosition.y - mPosition.y) - getGravityY();
	if (mHeldObject) {
		mVelocity.y -= unk138;
		JGeometry::TVec3<f32> pos = mHeldObject->mPosition;
		JGeometry::TVec3<f32> vel = mVelocity;
		pos.y += vel.y;
		mHeldObject->moveRequest(pos);
	}
	JGeometry::TVec3<f32> vel2 = mVelocity;
	mPosition.y += vel2.y;
	char trash[0x18];
	trash[0] = 0;
}

void TMapObjElasticCode::initMapObj()
{
	TMapObjBase::initMapObj();
	unk140   = 0.997f;
	mGravity = 0.01f;
	unk138   = 2.0f;
	unk13C   = 0.0005f;
}

void TMapObjGrowTree::getGrowHeightFromRate(float) const { }

void TMapObjGrowTree::updateHeight() { }

u32 TMapObjGrowTree::touchWater(THitActor*) { return 0; }

void TMapObjGrowTree::control() { }

void TMapObjGrowTree::loadAfter()
{
	TMapObjBase::loadAfter();
	removeMapCollision();
}

void TMapObjGrowTree::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = 1000.0f;
	unk13C = 0.5f;
	unk140 = 0.1f;
	unk144 = 360;
	unk148 = mDamageHeight;
	mMActor->setBtp("moyasi_wink");
}

TMapObjGrowTree::TMapObjGrowTree(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0)
    , unk148(0.0f)
{
}

// Retail body is large; the stub must stay a call from draw.
#pragma dont_inline on
void TWireBell::initDraw() const { }
#pragma dont_inline off

void TWireBell::draw() const
{
	initDraw();

	// Declaration order is the float-reg order (f31 down).
	f32 yBot = mPosition.y;
	f32 x1   = unk14C.x + unk140;
	f32 x0   = unk14C.x - unk140;
	f32 z1   = unk14C.z + unk144;
	f32 z0   = unk14C.z - unk144;
	f32 yTop = unk14C.y;
	f32 tTop = unk148 * (yTop - mPosition.y);
	f32 tBot = unk148 * (yBot - mPosition.y);

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
	GXPosition3f32(x0, yTop, z0);
	GXTexCoord2f32(0.0f, tTop);
	GXPosition3f32(x0, yBot, z0);
	GXTexCoord2f32(0.0f, tBot);
	GXPosition3f32(x1, yTop, z1);
	GXTexCoord2f32(1.0f, tTop);
	GXPosition3f32(x1, yBot, z1);
	GXTexCoord2f32(1.0f, tBot);
	GXPosition3f32(x1, yTop, z0);
	GXTexCoord2f32(0.0f, tTop);
	GXPosition3f32(x1, yBot, z0);
	GXTexCoord2f32(0.0f, tBot);
	GXPosition3f32(x0, yTop, z1);
	GXTexCoord2f32(1.0f, tTop);
	GXPosition3f32(x0, yBot, z1);
	GXTexCoord2f32(1.0f, tBot);
}

void TWireBell::control()
{
	gpMapWireManager->getPointPosInNthWire(unk138, mPosition, &unk14C);
	mPosition.x = unk14C.x;
	mPosition.y = unk14C.y - unk13C;
	mPosition.z = unk14C.z;

	Mtx mtx;
	MsMtxSetTRS(mtx, mPosition.x, mPosition.y, mPosition.z, mRotation.x,
	            mRotation.y, mRotation.z, mScaling.x, mScaling.y, mScaling.z);
	getModel()->setAnmMtx(0, mtx);
}

void TWireBell::loadAfter()
{
	TMapObjBase::loadAfter();
	unk138 = gpMapWireManager->getWireNo(mPosition);
}

TWireBell::TWireBell(const char* name)
    : TMapObjBase(name)
    , unk138(-1)
    , unk13C(200.0f)
    , unk140(10.0f)
    , unk144(5.0f)
    , unk148(0.01f)
{
	unk14C.zero();
}

void TMapObjPuncher::touchPlayer(THitActor*) { }

void TMapObjPuncher::control()
{
	// gap sits above the scale vec; trash below it.
	// Together they keep frame -0x38 and the vec at r1+0x20.
	char gap[4];
	gap[0] = 0;
	TMapObjBase::control();
	// Empty case 1 keeps the bge/b pair. A lone case 2 folds it away.
	switch (mState) {
	case STATE_NORMAL:
		break;
	case 2: {
		J3DFrameCtrl* ctrl = mMActor->getFrameCtrl(ANM_TYPE_BCK);
		soundBas(MSD_SE_OBJ_PUNCHER_RETURN, 101.0f, ctrl->getRate());
		if (animIsFinished()) {
			JGeometry::TVec3<f32> scale(2.0f);
			emitAndScale(PARTICLE_MS_ENM_DISAP_A_W, 0, &mPosition, scale);
			emitAndScale(PARTICLE_MS_ENM_DISAP_B, 0, &mPosition, scale);
			if (gpMSound->gateCheck(MSD_SE_SMOKE_EFFECT))
				MSoundSESystem::MSoundSE::startSoundActor(
				    MSD_SE_SMOKE_EFFECT, &mPosition, 0, nullptr, 0, 4);
			kill();
		}
		break;
	}
	}
	char trash[0x10];
	trash[0] = 0;
}

void TMapObjPuncher::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	s32 value;
	stream.read(&value, 4);
	unk138 = value;
	sleep();
	offHitFlag(HIT_FLAG_NO_COLLISION);
}

void TMuddyBoat::moveByWater() { }

void TMuddyBoat::calcRootMatrix() { }

void TMuddyBoat::kill() { }

void TMuddyBoat::touchWall(JGeometry::TVec3<float>*,
                           const TBGWallCheckRecord&)
{
}

void TMuddyBoat::bindToWall(const JGeometry::TVec3<float>&, float,
                            JGeometry::TVec3<float>*)
{
}

void TMuddyBoat::bind()
{
	// Retail bind calls the weak out-of-line copy. Address-of keeps that
	// symbol in this TU; the header body is empty, so the copy stays off.
	f32 (TMapObjBase::*fn)() const = &TMapObjBase::getObjCollisionHeightOffset;
	(this->*fn)();
}

void TMuddyBoat::control() { }

void TMuddyBoat::calc() { }

u32 TMuddyBoat::getSDLModelFlag() const { return 0; }

void TMuddyBoat::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = 0.04f;
	unk144 = 0.998f;
	unk148 = 0.002f;
	unk150 = 0.997f;
	unk13C = 0.01f;
	unk168 = 0x258;
	if (gpMarDirector->mMap == 0x34) {
		unk158 = 126.0f;
		unk154 = 185.0f;
		unk15C = 150.0f;
		unk160 = 170.0f;
		unk164 = 185.0f;
	} else {
		unk158 = 100.0f;
		unk154 = 170.0f;
		unk15C = 150.0f;
		unk160 = 180.0f;
		unk164 = 100.0f;
	}
	unk17C.x = 3.0f;
	unk17C.y = 2.0f;
	unk17C.z = 5.0f;
	char trash[0xC];
	trash[0] = 0;
}

TMuddyBoat::TMuddyBoat(const char* name)
    : TMapObjBase(name)
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.0f)
    , unk148(0.0f)
    , unk14C(0.0f)
    , unk150(0.0f)
    , unk154(0.0f)
    , unk158(0.0f)
    , unk15C(0.0f)
    , unk160(0.0f)
    , unk164(0.0f)
    , unk168(0)
    , unk16C(0)
{
	unk170.zero();
	unk17C.zero();
}

void TMareFall::calc()
{
	MSound* sound = gpMSound;
	if (sound->gateCheck(MSD_SE_GE_FALL))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_GE_FALL, &mPosition, 0, nullptr, 0, 4);
	if (gpMSound->gateCheck(MSD_SE_GE_FALL_UPPER))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_GE_FALL_UPPER, &fall_upper_pos, 0, nullptr, 0, 4);
	gpMarioParticleManager->emit(0x149, &mPosition, 1, this);
	gpMarioParticleManager->emit(0x14A, &mPosition, 1, this);
	char trash[0xC];
	trash[0] = 0;
}

void TMareFall::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	SMS_LoadParticle("/scene/mapObj/mareFallSplash.jpa", 0x149);
	SMS_LoadParticle("/scene/mapObj/mareFallSmoke.jpa", 0x14A);
}

void TMareCork::loadAfter()
{
	unk138 = (TCannon*)JDrama::TNameRefGen::search("砲台");
	if (((THitActor*)unk138)->receiveMessage(this, HIT_MESSAGE_TAKE))
		mHeldObject = (TTakeActor*)unk138;
	SMS_LoadParticle("/scene/map/map/ms_mare_gunwat_a.jpa", 0x14C);
	SMS_LoadParticle("/scene/map/map/ms_mare_gunwat_b.jpa", 0x14D);
	SMS_LoadParticle("/scene/map/map/ms_mare_gunwat_c.jpa", 0x14E);
	TMapObjBase::loadAfter();
	unk13C.x = 0.0f;
	unk13C.y = 0.0f;
	unk13C.z = 0.0f;
	initAnmSound();
}

void TMareCork::moveObject()
{
	if (unk138->isObject() && unk154 == 0) {
		mMActor->setBck("marecork");
		setAnmSound("/scene/mapObj/marecork.bas");
		removeMapCollision();
		unk154 = 1;
	}
}

void TMareCork::calcRootMatrix()
{
	if (unk154) {
		mMActor->getFrameCtrl(0)->checkPass(350.0f);
		if (mMActor->getFrameCtrl(0)->checkPass(250.0f)) {
			unk138->startChorobeiShout();
			gpItemManager->makeShineAppearWithDemo(
			    "シャイン（ボス用）", "ボスシャインカメラ", mPosition.x,
			    mPosition.y, mPosition.z);
			unk148.x = 2773.0f;
			unk148.y = 8618.0f;
			unk148.z = 7006.0f;
			JPABaseEmitter* emitter
			    = gpMarioParticleManager->emitWithRotate(
			        0x44, &unk148, 0x4000, 0x0D82, 0, 0, nullptr);
			if (emitter != nullptr) {
				emitter->mGlobalDynamicsScale.x = 2.5f;
				emitter->mGlobalDynamicsScale.y = 2.5f;
				emitter->mGlobalDynamicsScale.z = 2.5f;
				emitter->mGlobalParticleScale.x = 2.5f;
				emitter->mGlobalParticleScale.y = 2.5f;
				emitter->mGlobalParticleScale.z = 2.5f;
			}
		}
	}

	TMapObjBase::calcRootMatrix();

	// Dead slot so MWCC keeps frame -0x30.
	char trash[0x18];
	trash[0] = 0;
}

MtxPtr TMareCork::getTakingMtx()
{
	return mMActor->getModel()->getAnmMtx(2);
}

// Extra inline level so MWCC keeps the dead 8-byte temp (frame -0x28).
static inline f32 mareCorkFrame(MActor* actor)
{
	return actor->getFrameCtrl(0)->getFrame();
}

void TMareCork::drawObject(JDrama::TGraphics* graphics)
{
	TLiveActor::drawObject(graphics);
	if (unk154 != 0 && mareCorkFrame(mMActor) > 250.0f) {
		unk148.x = 2773.0f;
		unk148.y = 8618.0f;
		unk148.z = 7006.0f;
		if (gpMSound->gateCheck(MSD_SE_ENV_FALL_JET_LEVEL))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_ENV_FALL_JET_LEVEL, &unk148, 0, nullptr, 0, 4);
		gpMarioParticleManager->emitAndBindToPosPtr(0x14C, &unk13C, 1, this);
		gpMarioParticleManager->emitAndBindToPosPtr(0x14D, &unk13C, 1, this);
		gpMarioParticleManager->emitAndBindToPosPtr(0x14E, &unk13C, 1, this);
	}
}

// One inline level: the dead 4-byte temp rounds the frame from -0x28 to -0x30.
static inline TModelWaterManager* mareEventWater()
{
	return gpModelWaterManager;
}

BOOL TMareEventPoint::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_SPRAYED_BY_WATER
	    && !mareEventWater()->checkFlagBottom4Bits(
	        TMapObjBase::getWaterID(sender), 1)
	    && TMapObjBase::getWaterPlane(sender) != nullptr
	    && TMapObjBase::getWaterPlane(sender)->getNormal().y < 0.1f) {
		if (unk68->startEvent()) {
			gpMarioParticleManager->emit(PARTICLE_MS_ENM_WATHIT,
			                             &sender->mPosition, 0, nullptr);
			gpMSound->startSoundSet(MSD_SE_EN_COMMON_W_HIT_OK, &mPosition, 0,
			                        0.0f, 0, 0, 4);
		}
		return TRUE;
	}
	return FALSE;
}

void TMareEventPoint::load(JSUMemoryInputStream& stream)
{
	JDrama::TActor::load(stream);
	initHitActor(0x40000236, 0, 0, 0.0f, 0.0f, 300.0f, 600.0f);
}
