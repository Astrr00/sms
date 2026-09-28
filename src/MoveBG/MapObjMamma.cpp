#ifndef JG_UTIL_HPP
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

	// from SMG
	static bool epsilonEquals(f32 param_1, f32 param_2, f32 eps)
	{
		return -eps <= param_2 - param_1 && param_2 - param_1 <= eps;
	}

	// fabricated
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

#endif

#ifndef JG_VEC3_HPP
#define JG_VEC3_HPP

#include <dolphin/types.h>
#include <dolphin/mtx.h>
#include <JSystem/JGeometry/JGUtil.hpp>

namespace JGeometry {

template <class T> class TVec3 { };

template <> struct TVec3<s16> : public S16Vec {
public:
	TVec3() { }

	TVec3(const S16Vec& b) { set(b.x, b.y, b.z); }

	// fabricated
	TVec3(s16 x_, s16 y_, s16 z_) { set(x_, y_, z_); }

	void set(s16 x_, s16 y_, s16 z_)
	{
		x = (s16)x_;
		y = (s16)y_;
		z = (s16)z_;
	}

	void zero() { x = y = z = 0; }

	void add(const TVec3& operand)
	{
		x += operand.x;
		y += operand.y;
		z += operand.z;
	}

	void add(const TVec3& a, const TVec3& b)
	{
		x = a.x + b.x;
		y = a.y + b.y;
		z = a.z + b.z;
	}

	void sub(const TVec3& translate)
	{
		x -= translate.x;
		y -= translate.y;
		z -= translate.z;
	}

	void sub(const TVec3& fst, const TVec3& snd)
	{
		x = fst.x - snd.x;
		y = fst.y - snd.y;
		z = fst.z - snd.z;
	}

	TVec3& operator+=(const TVec3& other)
	{
		add(other);
		return *this;
	}

	TVec3& operator-=(const TVec3& other)
	{
		sub(other);
		return *this;
	}

	// fabricated and fake and UB but it makes things match??
	friend const TVec3& operator-(TVec3 fst, const TVec3& snd)
	{
		fst -= snd;
		return fst;
	}

	// fabricated and fake and UB but it makes things match??
	friend const TVec3& operator+(TVec3 fst, const TVec3& snd)
	{
		fst += snd;
		return fst;
	}
};

template <> class TVec3<f32> : public Vec {
public:
	TVec3() { }

	TVec3(const Vec& b) { set(b); }

	template <class T> TVec3(T x_, T y_, T z_) { set(x_, y_, z_); }

	explicit TVec3(f32 value) { setAll(value); }

#pragma dont_inline on
	TVec3(const TVec3& other)
	{
		// NOTE: yes, this has to use lwz/stw and not lfs/stf.
		// Checked via MarioCollision.cpp where this is not inlined
		*(Vec*)this = *(Vec*)&other;
	}
#pragma dont_inline off

#pragma dont_inline on
	TVec3& operator=(const TVec3& other)
	{
		// NOTE: yes, this has to use lwz/stw and not lfs/stf.
		// Checked via enemy.cpp where this is not inlined
		*(Vec*)this = *(Vec*)&other;
		return *this;
	}
#pragma dont_inline off

	// fabricated
	operator Vec*() const { return (Vec*)&x; }
	operator const Vec*() const { return (Vec*)&x; }

	void zero() { x = y = z = 0.0f; }

#pragma dont_inline on
	void set(const Vec& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
#pragma dont_inline off

#pragma dont_inline on
	template <class TY> void set(TY x_, TY y_, TY z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}
#pragma dont_inline off

	template <class TY> void set(const TVec3<TY>& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
	}

	template <class TY> void setAll(TY value)
	{
		x = value;
		y = value;
		z = value;
	}

	// === arithmetic stuff ===

	void add(const TVec3& operand)
	{
		x += operand.x;
		y += operand.y;
		z += operand.z;
	}

	void add(const TVec3& a, const TVec3& b)
	{
		x = a.x + b.x;
		y = a.y + b.y;
		z = a.z + b.z;
	}

	void sub(const TVec3& translate);

#pragma dont_inline on
	void sub(const TVec3& fst, const TVec3& snd)
	{
		x = fst.x - snd.x;
		y = fst.y - snd.y;
		z = fst.z - snd.z;
	}
#pragma dont_inline off

	void mul(const TVec3& b)
	{
		x *= b.x;
		y *= b.y;
		z *= b.z;
	}

	void mul(const TVec3& fst, const TVec3& snd)
	{
		x = fst.x * snd.x;
		y = fst.y * snd.y;
		z = fst.z * snd.z;
	}

	void div(f32 divisor)
	{
		divisor = 1.0f / divisor;
		scale(divisor);
	}

	TVec3& operator+=(const TVec3& other)
	{
		add(other);
		return *this;
	}
	TVec3& operator-=(const TVec3& other)
	{
		sub(other);
		return *this;
	}
	TVec3& operator*=(const TVec3& other)
	{
		mul(other);
		return *this;
	}
#pragma dont_inline on
	TVec3& operator*=(f32 other)
	{
		scale(other);
		return *this;
	}
#pragma dont_inline off
	TVec3& operator/=(f32 other)
	{
		div(other);
		return *this;
	}

	// fabricated and fake and UB but it makes things match??
	friend const TVec3& operator-(TVec3 fst, const TVec3& snd)
	{
		fst -= snd;
		return fst;
	}

	// fabricated and fake and UB but it makes things match??
	friend const TVec3& operator+(TVec3 fst, const TVec3& snd)
	{
		fst += snd;
		return fst;
	}

	// @fabricated
	friend TVec3 operator*(TVec3 fst, f32 snd)
	{
		fst *= snd;
		return fst;
	}
	friend TVec3 operator/(TVec3 fst, f32 snd)
	{
		fst /= snd;
		return fst;
	}

	f32 dot(const TVec3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	// Incorrect!!!
	void cross(const TVec3& a, const TVec3& b)
	{
		f32 _x = a.y * b.z - a.z * b.y;
		f32 _y = a.z * b.x - a.x * b.z;
		f32 _z = a.x * b.y - a.y * b.x;

		x = _x;
		y = _y;
		z = _z;
	}

	// Incorrect!!!
	void cross2(const TVec3& a, const TVec3& b)
	{
		set(a.y * b.z - a.z * b.y, //
		    a.z * b.x - a.x * b.z, //
		    a.x * b.y - a.y * b.x);
	}

	void negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	void scale(f32 scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
	}

	void scale(f32 scale, const TVec3& b)
	{
		x = b.x * scale;
		y = b.y * scale;
		z = b.z * scale;
	}

#pragma dont_inline on
	void scaleAdd(f32 scale, const TVec3& b, const TVec3& c)
	{
		x = b.x + c.x * scale;
		y = b.y + c.y * scale;
		z = b.z + c.z * scale;
	}
#pragma dont_inline off

	// === length stuff ===

	f32 distance(const TVec3& other) const
	{
		return TUtil<f32>::sqrt((x - other.x) * (x - other.x)
		                        + (y - other.y) * (y - other.y)
		                        + (z - other.z) * (z - other.z));
	}

	f32 squared() const { return dot(*this); }

	f32 squared(const TVec3& other) const
	{
		f32 dx = x - other.x;
		f32 dy = y - other.y;
		f32 dz = z - other.z;
		return dx * dx + dy * dy + dz * dz;
	}

	f32 length() const { return TUtil<f32>::sqrt(squared()); }

	bool isZero() const { return squared() <= TUtil<f32>::epsilon(); }

	void setLength(f32 length) { setLength(*this, length); }

	void normalize() { setLength(*this, TUtil<f32>::one()); }

	void normalize(const TVec3& other) { setLength(other, TUtil<f32>::one()); }

#pragma dont_inline on
	void setLength(const TVec3& v, f32 length)
	{
		f32 lsq = v.squared();
		if (lsq <= TUtil<f32>::epsilon()) {
			zero();
			return;
		}

		scale(length * JGeometry::TUtil<f32>::inv_sqrt(lsq), v);
	}
#pragma dont_inline off

	void setMax(const TVec3& max)
	{
		if (x <= max.x)
			x = max.x;
		if (y <= max.y)
			y = max.y;
		if (z <= max.z)
			z = max.z;
	}

	void setMin(const TVec3& min)
	{
		if (x >= min.x)
			x = min.x;
		if (y >= min.y)
			y = min.y;
		if (z >= min.z)
			z = min.z;
	}

	// from SMG
	bool epsilonEquals(const TVec3& other, f32 eps) const
	{
		return TUtil<f32>::epsilonEquals(x, other.x, eps)
		       && TUtil<f32>::epsilonEquals(y, other.y, eps)
		       && TUtil<f32>::epsilonEquals(z, other.z, eps);
	}

	// fabricated
	bool epsilonEquals(const TVec3& other) const
	{
		return TUtil<f32>::epsilonEquals(x, other.x)
		       && TUtil<f32>::epsilonEquals(y, other.y)
		       && TUtil<f32>::epsilonEquals(z, other.z);
	}

	// TODO: SMG's operator== uses epsilonEquals. Maybe this wasn't operator==
	// but a separate function? Eh, whatever.
	bool operator==(const TVec3& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

} // namespace JGeometry

#endif

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
#include <System/MarDirector.hpp>

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

bool TSandBase::withering()
{
	mScaling.y -= unk13C;
	if (mScaling.y < mScaleMin)
		mScaling.y = mScaleMin;

	// Address is taken before gateCheck so it stays live in r31.
	const JGeometry::TVec3<f32>* pos = &unk144->mPosition;
	if (gpMSound->gateCheck(MSD_SE_OBJ_SANDBUD_NORMAL))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_SANDBUD_NORMAL, pos, 0, nullptr, 0, 4);

	// Dead slot so the frame stays at -0x20 (r31 at r1+0x1c).
	char trash[1];
	trash[0] = 0;

	if (mScaling.y <= mScaleMin)
		return true;
	return false;
}

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

bool TSandCastle::withering() { return false; }

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

void TLeanMirror::loadAfter()
{
	char trash[0xC];
	trash[0] = 0;
	TMapObjBase::loadAfter();
	unk17C = (u32)JDrama::TNameRefGen::search("ShiningStone");
	JGeometry::TVec3<f32> diff(((TShiningStone*)unk17C)->mPosition);
	diff.sub(mPosition);
	unk180 = diff;
	f32 lsq = unk180.squared();
	if (lsq <= JGeometry::TUtil<f32>::epsilon())
		unk180.zero();
	else
		unk180.scale(JGeometry::TUtil<f32>::one()
		             * JGeometry::TUtil<f32>::inv_sqrt(lsq));
}

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

u32 TMammaBlockRotate::touchWater(THitActor*)
{
	if (isState(1)) {
		mRotation.y += mRotSpeed;
		if (mRotation.y > mRotEnd)
			mState = 2;
	}
	return 1;
}

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

void TGoalWatermelon::touchActor(THitActor* actor)
{
	char trash[4];
	trash[0] = 0;
	if (isState(1) && actor->isActorType(0x400000D0)) {
		unk13C = (TMapObjBase*)actor;
		unk13C->getMActor()->setBck("watermelon_shrink");
		unk13C->offMapObjFlag(MAP_OBJ_FLAG_UNK100);
		JGeometry::TVec3<f32> zero(0.0f);
		unk13C->mVelocity = zero;
		TMarDirector* director = gpMarDirector;
		director->fireStartDemoCamera("スイカゴールカメラ", &unk13C->mPosition,
		                              -1, 0.0f, true, nullptr, 0, nullptr,
		                              JDrama::TFlagT<u16>(0));
		mState = 2;
	}
}

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
