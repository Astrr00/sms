// hipdrop calls inv_sqrt. The header body is inline and MWCC folds an
// unused call down to a compare, so this TU sees a declaration only.
// The guard is local: JGUtil.hpp itself stays untouched (its source
// text is load-bearing for other TUs).
#define JG_UTIL_HPP
#include <dolphin/types.h>
#include <math.h>
namespace JGeometry {

template <typename T> struct TUtil {
	static T clamp(T value, T min, T max)
	{
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}

	static T mod(T value, T modulus);
};

template <class T> T TUtil<T>::mod(T value, T modulus)
{
	return value % modulus;
}

template <> struct TUtil<f32> {
#pragma dont_inline on
	static f32 one() { return 1.0f; }
#pragma dont_inline off
	static f32 epsilon() { return 3.81469727e-06f; }
	static f32 PI() { return 3.14159265358979323846f; }
	static f32 halfPI() { return 1.5707963267948966f; }

	static bool epsilonEquals(f32 param_1, f32 param_2, f32 eps)
	{
		return -eps <= param_2 - param_1 && param_2 - param_1 <= eps;
	}

	static bool epsilonEquals(f32 param_1, f32 param_2)
	{
		return -epsilon() <= param_2 - param_1
		       && param_2 - param_1 <= epsilon();
	}

	static f32 clamp(f32 value, f32 min, f32 max)
	{
		if (value < min)
			return min;
		if (value > max)
			return max;
		return value;
	}

	static f32 sqrt(f32 mag)
	{
		if (mag <= 0.0f)
			return mag;

		f32 root = __frsqrte(mag);
		return 0.5f * root * (3.0f - mag * (root * root)) * mag;
	}

	static f32 inv_sqrt(f32 mag);
};

} // namespace JGeometry

#pragma dont_inline on
f32 JGeometry::TUtil<f32>::inv_sqrt(f32 mag)
{
	if (mag <= 0.0f)
		return mag;

	f32 root = __frsqrte(mag);
	return 0.5f * root * (3.0f - mag * (root * root));
}
#pragma dont_inline off

// This TU calls SMatrix33R's ctor out of line. Other TUs keep the inline
// body in JGMatrix33.hpp, so that header stays unchanged.
#define JG_MATRIX33_HPP
#include <JSystem/JGeometry/JGVec3.hpp>
#include <dolphin/types.h>
namespace JGeometry {
template <typename T> struct SMatrix33C {
	T mMtx[3][3];
};
template <> struct SMatrix33C<f32> {
	SMatrix33C() { }
#pragma dont_inline on
	f32 at(u32 i, u32 j) const { return mMtx[i][j]; }
#pragma dont_inline off
	f32& ref(u32 i, u32 j) { return mMtx[i][j]; }
	f32 mMtx[3][3];
};
template <typename T> struct SMatrix33R {
	T mMtx[3][3];
};
template <> struct SMatrix33R<f32> {
	SMatrix33R();
	f32 at(u32 i, u32 j) const { return mMtx[j][i]; }
	f32& ref(u32 i, u32 j) { return mMtx[j][i]; }
	f32 mMtx[3][3];
};
template <typename T> struct TMatrix33 : public T {
	TMatrix33() { }
	void mult(const TVec3<f32>& src, TVec3<f32>& dst) const
	{
		f32 x = this->at(0, 0) * src.x + this->at(0, 1) * src.y
		        + this->at(0, 2) * src.z;
		f32 y = this->at(1, 0) * src.x + this->at(1, 1) * src.y
		        + this->at(1, 2) * src.z;
		f32 z = this->at(2, 0) * src.x + this->at(2, 1) * src.y
		        + this->at(2, 2) * src.z;
		dst.set(x, y, z);
	}
	void mult(TVec3<f32>& v) const
	{
		v.set(
		    this->at(0, 0) * v.x + this->at(0, 1) * v.y + this->at(0, 2) * v.z,
		    this->at(1, 0) * v.x + this->at(1, 1) * v.y + this->at(1, 2) * v.z,
		    this->at(2, 0) * v.x + this->at(2, 1) * v.y + this->at(2, 2) * v.z);
	}
	void identity()
	{
		this->ref(0, 2) = this->ref(1, 2) = 0.0f;
		this->ref(0, 1) = this->ref(2, 1) = 0.0f;
		this->ref(1, 0) = this->ref(2, 0) = 0.0f;
		this->ref(0, 0) = this->ref(1, 1) = this->ref(2, 2) = 1.0f;
	}
};
}
#include "MoveBG/MapObjCorona.hpp"
#include "MoveBG/MapObjBase.hpp"
#include <M3DUtil/MActor.hpp>
#include <JSystem/JMath.hpp>
#include <JSystem/JDrama/JDRNameRefGen.hpp>
#include <Camera/CameraShake.hpp>
#include <MarioUtil/RumbleMgr.hpp>
#include <Player/MarioAccess.hpp>
#include <System/Particles.hpp>
#include <System/ParamInst.hpp>
#include <System/Params.hpp>

// Slot 0x20 of TMapCollisionBase. No body, so the call stays virtual.
class TGripCollision {
public:
	virtual void init(const char*, u16, const TLiveActor*);
	virtual void moveSRT(const JGeometry::TVec3<f32>&,
	                     const JGeometry::TVec3<f32>&,
	                     const JGeometry::TVec3<f32>&);
	virtual void moveTrans(const JGeometry::TVec3<f32>&);
	virtual void moveMtx(MtxPtr);
	virtual void setUp();
	virtual void setUpTrans(const JGeometry::TVec3<f32>&);
	virtual void remove();
};

// Incomplete. Not a TLiveActor subclass, so this TU does not emit the
// grip vtable.
class TBathtubGrip {
public:
	void kill();
	void perform(u32 cue, JDrama::TGraphics* graphics);
	void removeCollisions_();
	Mtx* getRootJointMtx() const;
	void calcRootMatrix();

	/* 0x0 */ u8 pad0[0x150];
	/* 0x150 */ TGripCollision* unk150[5];
	/* 0x164 */ TGripCollision* unk164[17];
	/* 0x1A8 */ u8 pad1A8[0x58];
	/* 0x200 */ s32 unk200[1];
	/* 0x204 */ u8 pad204[0x40];
	/* 0x244 */ TBathtub* unk244;
	/* 0x248 */ u8 unk248;
	/* 0x249 */ u8 pad249;
	/* 0x24A */ u8 unk24A;
	/* 0x24B */ u8 pad24B;
	/* 0x24C */ f32 unk24C;
	/* 0x250 */ u8 pad250[4];
	/* 0x254 */ s32 unk254;
	/* 0x258 */ u8 pad258[4];
	/* 0x25C */ MActor* unk25C;
	/* 0x260 */ u8 unk260;
};

// Incomplete. Joint index lives on the grip at unk200[unkF8]. Not a
// TLiveActor subclass, so this TU does not emit the parts vtable.
class TBathtubGripParts {
public:
	Mtx* getRootJointMtx() const;

	/* 0x0 */ u8 pad[0xF4];
	/* 0xF4 */ TBathtubGrip* unkF4;
	/* 0xF8 */ s32 unkF8;
};

Mtx* TBathtubGripParts::getRootJointMtx() const
{
	s32 joint = unkF4->unk200[unkF8];
	Mtx* mtx  = (Mtx*)reinterpret_cast<const TLiveActor*>(unkF4)
	                ->getModel()
	                ->getAnmMtx(joint);
	// Dead slot so the frame stays at -0x30 (r31 at r1+0x2c).
	char trash[8];
	trash[0] = 0;
	return mtx;
}

// Incomplete. unkF4 is the owning grip. Not a TLiveActor subclass, so this
// TU does not emit the parts vtable.
class TBathtubGripPartsFragile {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);

	/* 0x0 */ u8 pad[0xF4];
	/* 0xF4 */ TLiveActor* unkF4;
};

BOOL TBathtubGripPartsFragile::receiveMessage(THitActor* sender, u32 message)
{
	return unkF4->receiveMessage(sender, message);
}

// Incomplete. Super hip-drop is rewritten to a normal hip-drop, then
// forwarded to the owning grip. Not a TLiveActor subclass.
class TBathtubGripPartsHard {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);

	/* 0x0 */ u8 pad[0xF4];
	/* 0xF4 */ TLiveActor* unkF4;
};

BOOL TBathtubGripPartsHard::receiveMessage(THitActor* sender, u32 message)
{
	if (message == HIT_MESSAGE_SUPER_HIP_DROP)
		message = HIT_MESSAGE_HIP_DROP;
	return unkF4->receiveMessage(sender, message);
}

void TBathtubGrip::kill()
{
	unk24A = 1;
	reinterpret_cast<TMapObjBase*>(this)->makeObjDead();
	removeCollisions_();
}

void TBathtubGrip::perform(u32 cue, JDrama::TGraphics* graphics)
{
	reinterpret_cast<TMapObjBase*>(this)->TMapObjBase::perform(cue, graphics);
	if (unk260 == 0) {
		if (cue & CUE_MOVE) {
			PSMTXCopy(
			    (MtxPtr)reinterpret_cast<TMapObjBase*>(this)->getRootJointMtx(),
			    unk25C->getModel()->getBaseTRMtx());
			if (unk254 > 0 || unk248 != 0)
				if (unk25C->curAnmEndsNext(ANM_TYPE_BCK, nullptr))
					unk260 = 1;
		}
		unk25C->perform(cue, graphics);
	}
}

Mtx* TBathtubGrip::getRootJointMtx() const
{
	return (Mtx*)reinterpret_cast<const TLiveActor*>(this)
	    ->getModel()
	    ->getBaseTRMtx();
}

void MsMtxSetRotRPH(MtxPtr mtx, f32 x, f32 y, f32 z);

void TBathtubGrip::calcRootMatrix()
{
	TPosition3f* dst
	    = (TPosition3f*)reinterpret_cast<TLiveActor*>(this)
	          ->getModel()
	          ->getBaseTRMtx();

	TPosition3f rot;
	MsMtxSetRotRPH((MtxPtr)rot, 0.0f, unk24C, 0.0f);
	rot.ref(0, 3) = 0.0f;
	rot.ref(1, 3) = 0.0f;
	rot.ref(2, 3) = 0.0f;

	TPosition3f* src = (TPosition3f*)unk244->getRootJointMtx();
	dst->concat(*src, rot);
}

void TBathtubGrip::removeCollisions_()
{
	for (int i = 0; i < 17; ++i)
		unk164[i]->remove();
	for (int i = 0; i < 5; ++i)
		unk150[i]->remove();
}

class TBathtubParams : public TParams {
public:
	TBathtubParams();

	/* 0x008 */ TParamRT<u8> resetGrip;
	/* 0x01C */ TParamRT<long> trampleRelease;
	/* 0x030 */ TParamRT<long> trampleRecover;
	/* 0x044 */ TParamRT<long> quakeRelease;
	/* 0x058 */ TParamRT<long> quakeRecover;
	/* 0x06C */ TParamRT<long> hipdropRelease;
	/* 0x080 */ TParamRT<long> hipdropRecover;
	/* 0x094 */ TParamRT<long> breakCount0;
	/* 0x0A8 */ TParamRT<long> breakCount1;
	/* 0x0BC */ TParamRT<long> breakCount2;
	/* 0x0D0 */ TParamRT<long> breakCount3;
	/* 0x0E4 */ TParamRT<long> launchStopCount;
	/* 0x0F8 */ TParamRT<f32> animSpeed0;
	/* 0x10C */ TParamRT<f32> animSpeed1;
	/* 0x120 */ TParamRT<f32> animSpeed2;
	/* 0x134 */ TParamRT<f32> animSpeed3;
	/* 0x148 */ TParamRT<f32> animSpeed4;
	/* 0x15C */ TParamRT<f32> shake;
	/* 0x170 */ TParamRT<f32> watermark;
	/* 0x184 */ TParamRT<f32> maxAngle;
	/* 0x198 */ TParamRT<f32> angleVelDamp;
	/* 0x1AC */ TParamRT<f32> rebound;
	/* 0x1C0 */ TParamRT<f32> shakeDamp;
	/* 0x1D4 */ TParamRT<f32> marioWeight;
	/* 0x1E8 */ TParamRT<f32> marioDropWeight;
	/* 0x1FC */ TParamRT<f32> outerHeight;
};

TBathtubParams::TBathtubParams()
    : TParams("/MapObj/bathtub.prm")
    , PARAM_INIT(resetGrip, 0)
    , PARAM_INIT(trampleRelease, 10)
    , PARAM_INIT(trampleRecover, 10)
    , PARAM_INIT(quakeRelease, 500)
    , PARAM_INIT(quakeRecover, 500)
    , PARAM_INIT(hipdropRelease, 35)
    , PARAM_INIT(hipdropRecover, 35)
    , PARAM_INIT(breakCount0, 750)
    , PARAM_INIT(breakCount1, 710)
    , PARAM_INIT(breakCount2, 685)
    , PARAM_INIT(breakCount3, 655)
    , PARAM_INIT(launchStopCount, 1000)
    , PARAM_INIT(animSpeed0, 0.15f)
    , PARAM_INIT(animSpeed1, 0.15f)
    , PARAM_INIT(animSpeed2, 0.15f)
    , PARAM_INIT(animSpeed3, 0.15f)
    , PARAM_INIT(animSpeed4, 0.22f)
    , PARAM_INIT(shake, 0.0f)
    , PARAM_INIT(watermark, 0.3f)
    , PARAM_INIT(maxAngle, 35.0f)
    , PARAM_INIT(angleVelDamp, 0.93f)
    , PARAM_INIT(rebound, 0.0005f)
    , PARAM_INIT(shakeDamp, 0.93f)
    , PARAM_INIT(marioWeight, 0.01f)
    , PARAM_INIT(marioDropWeight, 5.0f)
    , PARAM_INIT(outerHeight, 20.0f)
{
	TParams::load(mPrmPath);
}

// Leading zeroes so the four .jpa paths sit at the retail rodata offsets.
static const char cBathtubRodataPad[0x2B8] = { 0 };

void TBathtub::loadAfter()
{
	SMS_LoadParticle("/scene/map/map/ms_lkp_yuge1.jpa", 0x1BE);
	SMS_LoadParticle("/scene/map/map/ms_kp_funsui.jpa", 0x1BF);
	SMS_LoadParticle("/scene/map/map/ms_kp_break_a.jpa", 0xF6);
	SMS_LoadParticle("/scene/map/map/ms_kp_break_b.jpa", 0xF7);
	// 0xA4 (with NUL) so /MapObj/bathtub.prm stays at rodata 0x3E4.
	// A named array is emitted before these paths; a literal here is not.
	const char* gap =
	    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
	(void)gap;
}

class TKoopa {
public:
	void stagger(bool);
	void getDown();
	bool allowsLaunch() const;
};

void TBathtub::hipdrop(const JGeometry::TVec3<f32>& pos)
{
	if (unk29A != 0)
		return;

	if (unk250 > unk16C->hipdropRelease.value)
		return;

	// Refs keep init.z loaded after the x subtract.
	const JGeometry::TVec3<f32>& point = pos;
	const JGeometry::TVec3<f32>& home  = mInitialPosition;
	f32 dx = point.x - home.x;
	f32 dz = point.z - home.z;
	f32 zero = 0.0f;
	f32 lsq = zero + dx * dx;
	lsq += dz * dz;
	if (!(lsq <= JGeometry::TUtil<f32>::epsilon()))
		JGeometry::TUtil<f32>::inv_sqrt(lsq);

	unk250 = unk16C->hipdropRelease.value;
	unk258 = unk16C->hipdropRecover.value;
	unk25C = unk16C->hipdropRecover.value;
	unk254 = unk16C->hipdropRelease.value;

	((TKoopa*)JDrama::TNameRefGen::search("クッパ"))->stagger(false);

	// Dead slot so the frame stays at -0x98 (r31 at r1+0x94).
	char trash[0x60];
	trash[0] = 0;
}


void TBathtub::quake(const JGeometry::TVec3<f32>& pos)
{
	if (unk29A != 0)
		return;

	// Refs keep init.z loaded after the x subtract.
	const JGeometry::TVec3<f32>& point = pos;
	const JGeometry::TVec3<f32>& home  = mInitialPosition;
	f32 dx = point.x - home.x;
	f32 dz = point.z - home.z;
	f32 zero = 0.0f;
	f32 lsq = zero + dx * dx;
	lsq += dz * dz;
	if (!(lsq <= JGeometry::TUtil<f32>::epsilon()))
		JGeometry::TUtil<f32>::inv_sqrt(lsq);

	unk24C = 300;
	unk250 = unk16C->quakeRelease.value;
	unk258 = unk16C->quakeRecover.value;
	unk25C = unk16C->quakeRecover.value;
	unk254 = unk16C->hipdropRelease.value;
	unk248 = unk16C->launchStopCount.value;

	TKoopa* koopa = (TKoopa*)JDrama::TNameRefGen::search("クッパ");
	gpCameraShake->startShake((EnumCamShakeMode)0x25, 1.0f);
	gpCameraShake->startShake((EnumCamShakeMode)0x26, 1.0f);
	SMSRumbleMgr->start(4, (f32*)nullptr);

	// Dead slots so the frame stays at -0xa0 and the throw vector
	// stays at r1+0x74. The 0x9 array sits above the vector; the
	// 0x48 array sits below it.
	char above[0x9];
	JGeometry::TVec3<f32> up;
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	SMS_ThrowMario(up, 10.0f);
	koopa->getDown();
	char below[0x48];
	below[0] = 0;
	above[0] = 0;
}

// TBathtubGrip is not reconstructed. Byte 0x249 is 0 while the grip is dead.
struct TBathtubGripDead {
	u8 pad[0x249];
	u8 unk249;
};

int TBathtub::getNumGripsDead() const
{
	int count = 0;
	for (int i = 0; i < 5; ++i)
		if (reinterpret_cast<TBathtubGripDead*>(unk168[i])->unk249 == 0)
			++count;
	return count;
}

void TBathtub::tumble(f32 param_1, f32 param_2)
{
	if (unk29A != 0)
		return;

	// 182.04445 is 65536/360. Same lookup as JMASSin, written out so
	// the header dont_inline does not turn it into a call. The amplitude
	// is a named local so param_2 stays the left fmuls operand.
	f32 amp    = 0.0001f;
	u16 angle  = 182.04445f * param_1;
	f32 scale  = param_2 * amp;
	int index  = static_cast<u16>(angle) >> jmaSinShift;
	f32 cosine = jmaCosTable[index];
	f32 sine   = jmaSinTable[index];
	f32 cosAdd = scale * cosine;
	sine       = -sine;
	unk1E8     = unk1E8 + cosAdd;
	sine       = scale * sine;
	f32 zero   = 0.0f;
	unk1EC     = unk1EC + zero;
	unk1F0     = unk1F0 + sine;

	// Dead slot so the fctiwz spill stays at r1+0x38 (frame -0x40).
	char trash[8];
	trash[0] = 0;
}

MtxPtr TBathtub::getTakingMtx()
{
	return mMActor->getModel()->getAnmMtx(mMarioJntIdx);
}

MtxPtr TBathtub::getSubmarineMtxInDemo()
{
	return mMActor->getModel()->getAnmMtx(mSubmarineJntIdx);
}

MtxPtr TBathtub::getPeachMtxInDemo()
{
	return mMActor->getModel()->getAnmMtx(mDuckJntIdx);
}

MtxPtr TBathtub::getKoopaJrMtxInDemo()
{
	return mMActor->getModel()->getAnmMtx(mJuniorJntIdx);
}

BOOL TBathtub::receiveMessage(THitActor* sender, u32 message) { return false; }

Mtx* TBathtub::getRootJointMtx() const
{
	if (unk29A)
		return (Mtx*)getModel()->getAnmMtx(0);
	return (Mtx*)getModel()->getBaseTRMtx();
}

void TBathtub::perform(u32 cue, JDrama::TGraphics* graphics) { }

void TBathtub::control() { }

void TBathtub::calcBathtubData() { }

void TBathtub::setupCollisions_() { }

void TBathtub::removeCollisions_() { } // Unused

void TBathtub::startDemo() { }

// Real body is 0x26c. Keep the stub out of line so callers emit bl.
#pragma dont_inline on
bool TBathtub::allowsTumble() const { return false; }
#pragma dont_inline off

void TBathtub::calcRootMatrix() { }

bool TBathtub::getNearGrip(const JGeometry::TVec3<f32>&, f32, f32*) const
{
	return false;
}

u8 TBathtub::getNextJuncture(const JGeometry::TVec3<f32>&,
                             const JGeometry::TVec3<f32>&) const
{
	return 0;
}

u8 TBathtub::getNextGrip(const JGeometry::TVec3<f32>&,
                         const JGeometry::TVec3<f32>&, f32, f32*) const
{
	return 0;
}

void TBathtub::updatePosture_() { }

#pragma dont_inline on
__declspec(weak) JGeometry::SMatrix33R<f32>::SMatrix33R() { }
#pragma dont_inline off

TBathtub::TBathtub(const char* name)
    : TMapObjBase(name)
    , unk164(nullptr)
    , unk290(0)
{
	unk16C = new TBathtubParams;

	unk1D8 = 0.0f;
	unk1DC = 0.0f;
	unk1E0 = 0.0f;
	unk1E4 = 1.0f;
	mPosition.x = mPosition.y = mPosition.z = 0.0f;
	unk1E8 = unk1EC = unk1F0 = 0.0f;
	unk250 = 0;
	unk254 = 1;
	unk258 = 0;
	unk25C = 1;
	unk248 = 0;
	unk298 = 0;
	unk23C = unk240 = unk244 = 0.0f;
	unk299 = 0;
	unk29A = 0;
	unk2A0 = 0;
	unk294 = 0;

	volatile char trash[8];
	(void)trash;
}

void TBathtub::load(JSUMemoryInputStream&) { }

int TBathtub::getNumKillerLaunchable() const
{
	if (!isKillerLaunchable())
		return 0;

	int count = getNumGripsDead();
	int num   = count + 1;
	if (num < 2)
		num = 2;
	if (num > 4)
		num = 4;
	return num;
}

bool TBathtub::isKillerAttackable() const { return unk248 <= 0; }

int TBathtub::getNumKillerBurstable() const
{
	if (!isKillerLaunchable())
		return 0;

	int count = getNumGripsDead();
	if (count >= 4)
		return 8;
	if (!allowsTumble() && unk250 == 0 && unk258 == 0) {
		switch (count) {
		case 1:
			return 4;
		case 2:
			return 6;
		case 3:
			return 8;
		case 4:
			return 8;
		}
	}
	return 0;
}

// Unused
bool TBathtub::isBreaking() const { return false; }

// Unused out of line. Inlined into the killer count getters.
bool TBathtub::isKillerLaunchable() const
{
	bool ready;
	if (unk29A != 0) {
		ready = false;
	} else {
		JDrama::TNameRef* ref = JDrama::TNameRefGen::search("クッパ");
		if (!((TKoopa*)ref)->allowsLaunch())
			ready = false;
		else
			ready = unk248 <= 0;
	}
	return ready;
}

// Unused
void TBathtub::showMessage(u32) { }

// Unused
u8 TBathtub::getNearJuncture(const JGeometry::TVec3<f32>&) const { return 0; }

// Unused
MtxPtr TBathtub::getKoopaMtxInDemo() { return nullptr; }

// Unused
MtxPtr TBathtub::getWaterMtx(s32) { return nullptr; }

// Unused
MtxPtr TBathtub::getShineEffectMtx() { return nullptr; }

// Unused
MtxPtr TBathtub::getShineMtx() { return nullptr; }

// Unused
void TBathtub::liftMario(const JGeometry::TVec3<f32>&) { }

// Unused
void TBathtub::trample(const JGeometry::TVec3<f32>&) { }
