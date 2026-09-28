#include <MoveBG/MapObjMare.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/JParticle/JPAEmitter.hpp>
#include <M3DUtil/MActor.hpp>
#include <Map/MapWireManager.hpp>
#include <MoveBG/ItemManager.hpp>
#include <System/Particles.hpp>
#include <MSound/MSound.hpp>

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

// TODO: retail lfsu of unk158->unk138. `+=` emits lfs + stfs 0x138.
BOOL TCogwheelScale::receiveMessage(THitActor* sender, u32 message)
{
	return TMapObjBase::receiveMessage(sender, message);
}

void TCogwheelScale::touchPlayer(THitActor*) { }

// control is fcmpo + ble. Left as a stub.
void TCogwheelScale::control() { }

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

void TCogwheel::initDraw() const { }

void TCogwheel::draw() const { }

void TCogwheel::rebound() { }

void TCogwheel::calc() { }

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

void TMapObjElasticCode::draw() const { }

void TMapObjElasticCode::control() { }

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

void TWireBell::initDraw() const { }

void TWireBell::draw() const { }

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

void TMuddyBoat::initMapObj() { }

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

void TMareCork::loadAfter() { }

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

BOOL TMareEventPoint::receiveMessage(THitActor*, u32) { return FALSE; }

void TMareEventPoint::load(JSUMemoryInputStream& stream)
{
	JDrama::TActor::load(stream);
	initHitActor(0x40000236, 0, 0, 0.0f, 0.0f, 300.0f, 600.0f);
}
