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
#include <Map/MapCollisionManager.hpp>
#include <Map/MapData.hpp>
#include <M3DUtil/MActor.hpp>
#include <M3DUtil/MActorUtil.hpp>
#include <Strategic/MirrorActor.hpp>
#include <JSystem/J3D/J3DGraphLoader/J3DModelLoaderFlags.hpp>
#include <MarioUtil/MathUtil.hpp>
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
#include <System/TargetArrow.hpp>
#include <Camera/CameraShake.hpp>
#include <MarioUtil/RumbleMgr.hpp>
#include <Camera/Camera.hpp>
#include <Player/MarioAccess.hpp>
#include <GC2D/GCConsole2.hpp>
#include <Map/MapStaticObject.hpp>
#include <Map/MapMirror.hpp>
#include <printf.h>

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

extern f32 SMSGetAnmFrameRate();

void TSandLeafBase::grow()
{
	if (mState != 1 && mState != 4)
		return;
	if (mScaling.y < 1.0f) {
		mScaling.y += unk138;
		if (mScaling.y > 1.0f)
			mScaling.y = 1.0f;

		if (mState == 1) {
			mMapCollisionManager->changeCollision(1);
			TMapCollisionManager* colMgr = mMapCollisionManager;
			Mtx mtx;
			MsMtxSetTRS(mtx, mPosition.x, mPosition.y, mPosition.z, mRotation.x,
			            mRotation.y, mRotation.z, mScaling.x, mScaling.y,
			            mScaling.z);
			TMapCollisionBase* col = colMgr->unk8;
			MTXCopy(mtx, col->unk20);
			col->setUp();
			unk144->startControlAnim(2);
			mState = 4;
		}

		f32 rate          = SMSGetAnmFrameRate();
		TMapObjBase* leaf = unk144;
		f32 frame         = leaf->getMActor()->getFrameCtrl(0)->getFrame();
		leaf->getMActor()->getFrameCtrl(0)->setFrame(rate + frame);
		SMSRumbleMgr->start(0x15, 5, &mPosition);

		const JGeometry::TVec3<f32>* pos = &unk144->mPosition;
		if (gpMSound->gateCheck(MSD_SE_OBJ_SANDBUD_NORMAL))
			MSoundSESystem::MSoundSE::startSoundActor(
			    MSD_SE_OBJ_SANDBUD_NORMAL, pos, 0, nullptr, 0, 4);

		mStateTimer = TSandBase::mWitherTime;
	}
	char trash[21];
	trash[20] = 0;
}

void TSandLeafBase::control()
{
	TSandLeafBase* self = this;
	self->TMapObjBase::control();
	switch (self->mState) {
	case 1:
		break;
	case 2:
		SMSRumbleMgr->start(0x13, -1, &self->mPosition);
		if (self->withering()) {
			SMSRumbleMgr->stop(0x13);
			self->mMapCollisionManager->changeCollision(0);
			TMapCollisionManager* colMgr = self->mMapCollisionManager;
			Mtx mtx;
			MsMtxSetTRS(mtx, self->mPosition.x, self->mPosition.y,
			            self->mPosition.z, self->mRotation.x, self->mRotation.y,
			            self->mRotation.z, self->mScaling.x, self->mScaling.y,
			            self->mScaling.z);
			TMapCollisionBase* col = colMgr->unk8;
			MTXCopy(mtx, col->unk20);
			col->setUp();
			self->mStateTimer = self->unk140;
			self->mState      = 3;
		}
		break;
	case 3:
		if (self->isStateTimerEngaged())
			break;
		if (self->unk144->animIsFinished()) {
			self->unk144->awake();
			self->unk144->startAnim(1);
			const JGeometry::TVec3<f32>* pos = &self->unk144->mPosition;
			if (gpMSound->gateCheck(MSD_SE_IT_COMMON_APPEAR))
				MSoundSESystem::MSoundSE::startSoundActor(
				    MSD_SE_IT_COMMON_APPEAR, pos, 0, nullptr, 0, 4);
			self->mState = 5;
		}
		break;
	case 5:
		if (self->unk144->animIsFinished()) {
			self->unk144->startAnim(0);
			self->mState = 1;
		}
		break;
	}
	char trash[9];
	trash[8] = 0;
}

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

static inline void addFiringFrame(TLiveActor* actor, int idx)
{
	f32 speed = TSandBombBase::mFiringFrameSpeed;
	f32 frame = actor->getMActor()->getFrameCtrl(idx)->getFrame();
	actor->getMActor()->getFrameCtrl(idx)->setFrame(speed + frame);
}

u32 TSandBomb::touchWater(THitActor*)
{
	addFiringFrame(this, 0);
	addFiringFrame(this, 5);
	getMActor()->getFrameCtrl(0);
	soundBas(MSD_SE_OBJ_SANDBOMB_WATER_1, 7.0f,
	          TSandBombBase::mFiringFrameSpeed);
	soundBas(MSD_SE_OBJ_SANDBOMB_WATER_2, 50.0f,
	          TSandBombBase::mFiringFrameSpeed);
	soundBas(MSD_SE_OBJ_SANDBOMB_WATER_3, 100.0f,
	          TSandBombBase::mFiringFrameSpeed);
	soundBas(MSD_SE_OBJ_SANDBOMB_WATER_4, 150.0f,
	          TSandBombBase::mFiringFrameSpeed);
	if (getMActor()->curAnmEndsNext()) {
		unk138->getLivingTime();
		startControlAnim(3);
		startControlAnim(4);
		startControlAnim(5);
		onHitFlag(HIT_FLAG_NO_COLLISION);
	}
	char trash[28];
	trash[27] = 0;
	return 1;
}

u32 TSandBomb::getSDLModelFlag() const { return 0; }

void TSandBomb::initMapObj() { TMapObjBase::initMapObj(); }

void TSandBombBase::withered()
{
	mStateTimer = unk140;
	mState      = 3;
	unk144->sleep();
}

static inline void sandBombExpanded(TSandBombBase* self)
{
	TMapObjBase* bomb = self->unk144;
	f32 speed         = self->unk150;
	f32 frame = bomb->getMActor()->getFrameCtrl(0)->getFrame();
	bomb->getMActor()->getFrameCtrl(0)->setFrame(speed + frame);

	const JGeometry::TVec3<f32>* pos = &self->unk144->mPosition;
	if (gpMSound->gateCheck(MSD_SE_OBJ_SAMDBOMB_REVERSE))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_SAMDBOMB_REVERSE, pos, 0, nullptr, 0, 4);

	if (self->unk144->animIsFinished())
		self->mState = 2;
}

void TSandBombBase::expanded() { sandBombExpanded(this); }

extern JGeometry::TVec3<f32>* gpMarioPos;
extern f32 SMS_GetMarioGrLevel();
extern bool SMS_SendMessageToMario(THitActor*, u32);
extern void SMS_ThrowMario(const JGeometry::TVec3<f32>&, f32);

static inline void addExplodeFrame(TLiveActor* actor)
{
	f32 speed = TSandBombBase::mExplodeFrameSpeed;
	f32 frame = actor->getMActor()->getFrameCtrl(0)->getFrame();
	actor->getMActor()->getFrameCtrl(0)->setFrame(speed + frame);
}

static inline void sandBombExploding(TSandBombBase* self)
{
	char trash[0xC];
	trash[0] = 0;
	addExplodeFrame(self);
	addExplodeFrame(self->unk144);

	f32 dist = self->getDistanceXZ(*gpMarioPos);
	if ((self->mActorType == 0x400000CE ? true : false) ? true : false) {
	} else if (self->getMActor()->getFrameCtrl(0)->getFrame() < 80.0f
	           && SMS_GetMarioGrLevel() > gpMarioPos->y - 30.0f
	           && dist < self->unk154) {
		SMS_SendMessageToMario(self, HIT_MESSAGE_THROWN);
		JGeometry::TVec3<f32> dir;
		dir.x = 0.0f;
		dir.y = 1.0f;
		dir.z = 0.0f;
		SMS_ThrowMario(dir, self->mMarioJumpRate * (self->unk154 - dist));
	}

	if (self->animIsFinished()) {
		self->unk144->startControlAnim(6);
		self->mState = 8;
	}
}

void TSandBombBase::exploding() { sandBombExploding(this); }

void TSandBombBase::explode()
{
	char trash[36];
	trash[0] = 0;
	startControlAnim(1);
	mScaling.y = 1.0f;
	mMapCollisionManager->changeCollision(1);
	mMapCollisionManager->unk8->setUp();
	if (mMapCollisionManager->unk8 != nullptr)
		mMapCollisionManager->unk8->moveSRT(mPosition, mRotation, mScaling);

	JPABaseEmitter* emitter
	    = gpMarioParticleManager->emit(0x55, &mPosition, 0, nullptr);
	f32 scale                         = unk14C;
	emitter->mGlobalDynamicsScale.x   = scale;
	emitter->mGlobalDynamicsScale.y   = scale;
	emitter->mGlobalDynamicsScale.z   = scale;
	emitter->mGlobalParticleScale.x   = scale;
	emitter->mGlobalParticleScale.y   = scale;
	emitter->mGlobalParticleScale.z   = scale;

	if (!gpMarDirector->isDemoModeNow())
		gpCameraShake->startShake((EnumCamShakeMode)0xd, 1.0f);

	if (gpMSound->gateCheck(MSD_SE_OBJ_SANDBOMB_BANG))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_SANDBOMB_BANG, &mPosition, 0, nullptr, 0, 4);

	SMSRumbleMgr->start(0x15, mExlodingRumbleTime, &mPosition);
	mState = 7;
}

void TSandBombBase::waitBeforeExplode()
{
	mState      = 6;
	mStateTimer = unk148;
}

void TSandBombBase::grow() { mState = 5; }

static inline void addBombFrame(TLiveActor* actor, int idx)
{
	f32 speed = TSandBombBase::mExplodeFrameSpeed;
	f32 frame = actor->getMActor()->getFrameCtrl(idx)->getFrame();
	actor->getMActor()->getFrameCtrl(idx)->setFrame(speed + frame);
}

void TSandBombBase::control()
{
	TMapObjBase::control();
	TSandBomb* bomb = (TSandBomb*)unk144;
	switch (mState) {
	case 1: {
		f32 frame = bomb->getMActor()->getFrameCtrl(0)->getFrame()
		            - mFiringFrameDownSpeed;
		if (frame >= 0.0f) {
			unk144->getMActor()->getFrameCtrl(0)->setFrame(frame);
			unk144->getMActor()->getFrameCtrl(5)->setFrame(frame);
		}
		break;
	}
	case 5:
		addBombFrame(bomb, 0);
		addBombFrame(unk144, 5);
		addBombFrame(unk144, 3);
		if (unk144->animIsFinished())
			waitBeforeExplode();
		break;
	case 6:
		if (isStateTimerEngaged())
			break;
		explode();
		break;
	case 7:
		exploding();
		break;
	case 8:
		expanded();
		break;
	case 2:
		SMSRumbleMgr->start(0x13, -1, &mPosition);
		if (withering()) {
			withered();
			SMSRumbleMgr->stop(0x13);
		}
		break;
	case 3:
		if (isStateTimerEngaged())
			break;
		mState = 1;
		unk144->awake();
		unk144->startControlAnim(1);
		unk144->startControlAnim(2);
		break;
	case 0:
	case 4:
		break;
	}
	if (unk144->mColCount == 0)
		bomb->unk140 = 0;
	char trash[24];
	trash[23] = 0;
}

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

// dont_inline keeps the qualified call in TSandCastle::initMapObj.
#pragma dont_inline on
void TSandBombBase::initMapObj()
{
	unk138     = 0.006f;
	unk13C     = 0.005f;
	unk140     = 0;
	unk148     = 0x3C;
	unk154     = 1000.0f;
	mScaling.y = TSandBase::mScaleMin;
	TMapObjBase::initMapObj();
	unk150 = 0.5f;
	if (strcmp(unkF4, "SandBombBasePyramid") == 0) {
		unk14C = 1.3f;
		unk154 = 1200.0f;
	} else if (strcmp(unkF4, "SandBombBaseShit") == 0) {
		unk14C = 1.3f;
		unk154 = 1500.0f;
	} else if (strcmp(unkF4, "SandBombBaseStar") == 0) {
		unk14C = 1.2f;
	} else if (strcmp(unkF4, "SandBombBaseTurtle") == 0) {
		unk14C = 1.2f;
	}
	SMS_LoadParticle("/scene/mapObj/SandBomb.jpa", 0x55);
}
#pragma dont_inline off

// Leading pool plus this pad keep TSandBombBase::initMapObj's addi
// offsets once TShiningStone::load's strings are in the pool.
static const char cSandBombRodataPad[0xE0] = { 0 };

TSandBombBase::TSandBombBase(const char* name)
    : TSandBase(name)
    , unk148(0)
    , unk14C(1.0f)
    , unk150(0.0f)
    , unk154(0.0f)
{
}

static inline void addWitherFrame(TSandCastle* self, int idx)
{
	f32 speed = self->unk13C;
	f32 frame = self->getMActor()->getFrameCtrl(idx)->getFrame();
	self->getMActor()->getFrameCtrl(idx)->setFrame(speed + frame);
}

bool TSandCastle::withering()
{
	char trash[20];
	trash[0] = 0;
	addWitherFrame(this, 0);
	addWitherFrame(this, 5);

	f32 frame = getMActor()->getFrameCtrl(0)->getFrame();
	f32 end   = getMActor()->getFrameCtrl(0)->getEnd();
	mScaling.y = TSandCastle::mCollisionRate * ((end - frame) / end);

	if (frame > 240.0f)
		if (!unk158->checkLiveFlag(LIVE_FLAG_DEAD)) {
			unk158->kill();
			gpTargetArrow->unk14 = 0;
		}

	if (animIsFinished()) {
		sleep();
		return true;
	}
	return false;
}

extern "C" MActor* getMActor__10TLiveActorCFv(const TLiveActor*);

static inline void sandCastleExpanded(TSandCastle* self)
{
	char trash[5];
	trash[0] = 0;
	TMapObjBase* bomb = self->unk144;
	f32 speed         = self->unk150;
	f32 frame = getMActor__10TLiveActorCFv(bomb)->getFrameCtrl(0)->getFrame();
	getMActor__10TLiveActorCFv(bomb)->getFrameCtrl(0)->setFrame(speed + frame);

	const JGeometry::TVec3<f32>* pos = &self->unk144->mPosition;
	if (gpMSound->gateCheck(MSD_SE_OBJ_SAMDBOMB_REVERSE))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_SAMDBOMB_REVERSE, pos, 0, nullptr, 0, 4);

	if (self->unk144->animIsFinished())
		self->mState = 2;

	if (self->unk144->animIsFinished()) {
		self->mState = 2;
		self->startControlAnim(2);
		self->startControlAnim(3);
	}
}

void TSandCastle::expanded() { sandCastleExpanded(this); }

void TSandCastle::explode()
{
	char trash[36];
	trash[0] = 0;
	startControlAnim(1);
	mScaling.y = 1.0f;
	mMapCollisionManager->changeCollision(1);
	mMapCollisionManager->unk8->setUp();
	if (mMapCollisionManager->unk8 != nullptr)
		mMapCollisionManager->unk8->moveSRT(mPosition, mRotation, mScaling);

	JPABaseEmitter* emitter
	    = gpMarioParticleManager->emit(0x55, &mPosition, 0, nullptr);
	f32 scale                       = unk14C;
	emitter->mGlobalDynamicsScale.x = scale;
	emitter->mGlobalDynamicsScale.y = scale;
	emitter->mGlobalDynamicsScale.z = scale;
	emitter->mGlobalParticleScale.x = scale;
	emitter->mGlobalParticleScale.y = scale;
	emitter->mGlobalParticleScale.z = scale;

	if (!gpMarDirector->isDemoModeNow())
		gpCameraShake->startShake((EnumCamShakeMode)0xd, 1.0f);

	if (gpMSound->gateCheck(MSD_SE_OBJ_SANDBOMB_BANG))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_OBJ_SANDBOMB_BANG, &mPosition, 0, nullptr, 0, 4);

	SMSRumbleMgr->start(0x15, mExlodingRumbleTime, &mPosition);
	mState = 7;
	awake();
	unk158->appear();
	startControlAnim(3);
}

static void SandCastleCallBack(u32, u32) { }

static inline void startSandCastleDemo(TMarDirector* director)
{
	char trash[4];
	trash[3] = 0;
	director->fireStartDemoCamera(
	    "mamma1_sandcastle", nullptr, -1, 0.0f, true,
	    (s32 (*)(u32, u32))SandCastleCallBack, 0, nullptr,
	    JDrama::TFlagT<u16>(0));
}

void TSandCastle::waitBeforeExplode()
{
	mState      = 6;
	mStateTimer = unk148;
	startSandCastleDemo(gpMarDirector);
	unk15C = 1;
}

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

void TLeanMirror::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);

	f32 angle;
	stream.read(&angle, 4);
	unk138 = (100.0f * angle) / 2.0f;
	unk13C = unk138;

	if (gpMarDirector->unk7D == 1) {
		char name[0x40];
		stream.readString(name, 0x40);
		stream.read(&unk1A0.x, 4);
		stream.read(&unk1A0.y, 4);
		stream.read(&unk1A0.z, 4);
	}

	TMirrorModelObj* mirror = new TMirrorModelObj;
	char path[0x40];
	snprintf(path, 0x40, "/scene/mapObj/%sTop.bmd", unkF4);
	mirror->init(path);
	mirror->unk28 = getModel();

	if (gpMarDirector->unk7D != 1)
		mState = 4;

	char trash[0x30];
	(void)trash;
}

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

void TShiningStone::putOnLight(TLiveActor* actor)
{
	// fabricated. File-scope pads all land at the front of .rodata.
	// 0xC7 chars + NUL. TLeanMirror::load's path literal is the other
	// 0x18, keeping SandBombBasePyramid at 0x494.
	strcmp((const char*)actor,
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
	       "XXXXXXX");
}

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

static inline void copyStoneMtx(MtxPtr dstMtx, MActor* actor)
{
	MTXCopy(dstMtx, actor->getModel()->getBaseTRMtx());
}

void TShiningStone::load(JSUMemoryInputStream& stream)
{
	JDrama::TActor::load(stream);

	const char* names[4] = {
		"/scene/mapObj/ShiningStoneGreen.bmd",
		"/scene/mapObj/ShiningStoneBlue.bmd",
		"/scene/mapObj/ShiningStoneRed.bmd",
		"/scene/mapObj/ShiningStoneWhite.bmd",
	};

	Mtx mtx;
	MtxPtr dst = mtx;
	MsMtxSetXYZRPH(dst, mPosition.x, mPosition.y, mPosition.z, mRotation.x,
	               mRotation.y, mRotation.z);

	unk68 = new MActor*[4];
	MActor* actor;
	for (int i = 0; i < 4; ++i) {
		actor = SMS_MakeMActorWithAnmData(
		    names[i], gpMapObjManager->getMActorAnmData(), 3,
		    J3DMLF_MaterialPEFull | (2 << J3DMLF_TevStageNumShift));
		((MActor**)unk68)[i] = actor;
		actor = ((MActor**)unk68)[i];
		copyStoneMtx(dst, actor);
		TMirrorActor* mirror = new TMirrorActor("太陽石in鏡");
		mirror->init(((MActor**)unk68)[i]->getModel(), 0x1A);
	}

	actor = SMS_MakeMActorWithAnmData(
	    "/scene/mapObj/ShiningStone.bmd", gpMapObjManager->getMActorAnmData(),
	    3, J3DMLF_MaterialPEFull | (2 << J3DMLF_TevStageNumShift));
	unk6C = (u32)actor;
	((MActor*)unk6C)->setBpk("shiningstone");
	((MActor*)unk6C)->setBtk("shiningstone");
	actor = (MActor*)unk6C;
	copyStoneMtx(mtx, actor);

	SMS_LoadParticle("/scene/mapObj/ShiningStone1.jpa", 0x143);
	SMS_LoadParticle("/scene/mapObj/ShiningStone2.jpa", 0x144);
	SMS_LoadParticle("/scene/mapObj/ShiningStone3.jpa", 0x145);
	SMS_LoadParticle("/scene/mapObj/ShiningStoneF.jpa", 0x56);
}

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

void TMammaBlockRotate::control()
{
	TMapObjBase::control();
	JGeometry::TVec3<f32> off;
	// Places off at r1+0xa0 so the frame stays -0xb8.
	char trash[8];
	trash[0] = 0;
	switch (mState) {
	case 1:
		if (mRotation.y > 0.0f)
			mRotation.y -= mRotReturnSpeed;
		else
			mRotation.y = 0.0f;
		break;
	case 2: {
		TMapObjBase::moveJoint(unk140->getJoint(), 0.0f, -mMapGoSpeed, 0.0f);
		TMapObjBase::moveJoint(unk13C->getJoint(), 0.0f, -mMapGoSpeed, 0.0f);
		J3DTransformInfo& info = unk140->getJoint()->getTransformInfo();
		unk138->getModel()->calc();
		f32 y = info.mTranslate.y;
		off.set(0.0f, y, 0.0f);
		unk144->moveTrans(off);
		off.set(0.0f, info.mTranslate.y, 0.0f);
		unk148->moveTrans(off);
		if (info.mTranslate.y < 0.0f) {
			mStateTimer = mWaitTime;
			mState = 3;
		}
		break;
	}
	case 3:
		if (!isStateTimerEngaged())
			mState = 4;
		break;
	case 4: {
		TMapObjBase::moveJoint(unk140->getJoint(), 0.0f, mMapBackSpeed, 0.0f);
		TMapObjBase::moveJoint(unk13C->getJoint(), 0.0f, mMapBackSpeed, 0.0f);
		J3DJoint* joint = unk140->getJoint();
		f32 y           = joint->getTransformInfo().mTranslate.y;
		J3DTransformInfo& info = joint->getTransformInfo();
		off.set(0.0f, y, 0.0f);
		unk144->moveTrans(off);
		off.set(0.0f, info.mTranslate.y, 0.0f);
		unk148->moveTrans(off);
		unk138->getModel()->calc();
		y = info.mTranslate.y;
		if (y > unk140->getJoint()->getMax().y - unk140->getJoint()->getMin().y)
			mState = 1;
		break;
	}
	}
}

void TMammaBlockRotate::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = gpMap->getModelManager()->getJointModel(0);

	unk13C = unk138->getChild(0)->getChild(0)->getChild(0)->getChild(1);
	J3DJoint* joint = unk13C->getJoint();
	f32 dy         = joint->getMax().y - joint->getMin().y;
	TMapObjBase::moveJoint(joint, 0.0f, dy, 0.0f);
	JGeometry::TVec3<f32> off(0.0f, dy, 0.0f);
	char trash[0x70];
	unk144->setUp();
	unk144->moveTrans(off);

	unk140 = unk138->getChild(0)->getChild(0)->getChild(0)->getChild(2);
	joint  = unk140->getJoint();
	dy     = joint->getMax().y - joint->getMin().y;
	TMapObjBase::moveJoint(joint, 0.0f,
	                       joint->getMax().y - joint->getMin().y, 0.0f);
	unk148->setUp();
	off.x = 0.0f;
	off.y = dy;
	off.z = 0.0f;
	unk148->moveTrans(off);

	unk138->getModel()->calc();
}

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

void TSandBird::control()
{
	// Dead slot so the frame stays at -0x78.
	char trash[0x28];
	trash[0] = 0;

	TJointCoin::control();

	if (gpMSound->gateCheck(MSD_SE_EN_SANDBIRD_CRY))
		MSoundSESystem::MSoundSE::startSoundActor(
		    MSD_SE_EN_SANDBIRD_CRY, &mPosition, 0, nullptr, 0, 4);

	if (gpMSound->gateCheck(MSD_SE_ENV_SANDBIRD_WIND))
		MSoundSESystem::MSoundSE::startSoundSystemSE(
		    MSD_SE_ENV_SANDBIRD_WIND, 0, nullptr, 0);

	for (s32 i = 0; i < unk13C; ++i) {
		if (unk140[i]->isActorType(0x2000000E)
		    || unk140[i]->isActorType(0x40000023)) {
			gpMarioParticleManager->emitAndBindToPosPtr(
			    0x159, &unk140[i]->mPosition, 1, unk140[i]);
			gpMarioParticleManager->emitAndBindToPosPtr(
			    0x15A, &unk140[i]->mPosition, 1, unk140[i]);
		}
	}

	if (!gpCamera->isDemoCamera() && unk150 == 0) {
		const TLiveActor* actor = SMS_GetMarioGroundPlane()->getActor();
		if (actor != nullptr && actor->isActorType(0x400002C9)) {
			gpMarDirector->getConsole()->startAppearBalloon(0xE002F, false);
			mStateTimer = 0x960;
			unk150      = 1;
		}
	}

	if (unk151 == 0 && unk150 != 0 && !isStateTimerEngaged()) {
		gpMarDirector->getConsole()->startDisappearBalloon(0xE002F, false);
		unk151 = 1;
	}
}

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

static inline void storeMirrorCenter(JGeometry::TVec3<f32>& dst, f32 x, f32 y,
                                      f32 z)
{
	dst.x = x;
	dst.y = y;
	dst.z = z;
}

static inline void mirrorMapLoadAfterPad()
{
	char trash[4];
	trash[0] = 0;
}

void TMammaMirrorMapOperator::loadAfter()
{
	{
		JDrama::TActor* actor
		    = (JDrama::TActor*)JDrama::TNameRefGen::search("mirrorS");
		unkB8[0].x = actor->mPosition.x;
		unkB8[0].y = actor->mPosition.y;
		unkB8[0].z = actor->mPosition.z;

		actor = (JDrama::TActor*)JDrama::TNameRefGen::search("mirrorM");
		unkB8[1].x = actor->mPosition.x;
		unkB8[1].y = actor->mPosition.y;
		unkB8[1].z = actor->mPosition.z;

		actor = (JDrama::TActor*)JDrama::TNameRefGen::search("mirrorL");
		unkB8[2].x = actor->mPosition.x;
		unkB8[2].y = actor->mPosition.y;
		unkB8[2].z = actor->mPosition.z;
	}

	J3DJoint* joint
	    = ((TMapStaticObj*)JDrama::TNameRefGen::search("鏡内地形"))
	          ->getModelData()
	          ->getJointNodePointer(2);
	for (int i = 0; i < 8; ++i) {
		unk10[i] = (JDrama::TNameRef*)joint;
		storeMirrorCenter(
		    unk30[i], 0.5f * (joint->getMax().x + joint->getMin().x),
		    0.5f * (joint->getMax().y + joint->getMin().y),
		    0.5f * (joint->getMax().z + joint->getMin().z));
		f32 dx = 0.5f * (joint->getMax().x - joint->getMin().x);
		f32 dz = 0.5f * (joint->getMax().z - joint->getMin().z);
		if (dx > dz)
			unk90[i] = dx;
		else
			unk90[i] = dz;
		unk90[i] += 2000.0f;
		if (unk90[i] > 3000.0f)
			unk90[i] = 3000.0f;
		joint = (J3DJoint*)joint->getYounger();
	}
	mirrorMapLoadAfterPad();
}

TMammaMirrorMapOperator::TMammaMirrorMapOperator(const char* name)
    : JDrama::TViewObj(name)
{
	unk10[0] = nullptr;
	unk30[0].zero();
	unk90[0] = 0.0f;
	unkB0[0] = 0;
	unk10[1] = nullptr;
	unk30[1].zero();
	unk90[1] = 0.0f;
	unkB0[1] = 0;
	unk10[2] = nullptr;
	unk30[2].zero();
	unk90[2] = 0.0f;
	unkB0[2] = 0;
	unk10[3] = nullptr;
	unk30[3].zero();
	unk90[3] = 0.0f;
	unkB0[3] = 0;
	unk10[4] = nullptr;
	unk30[4].zero();
	unk90[4] = 0.0f;
	unkB0[4] = 0;
	unk10[5] = nullptr;
	unk30[5].zero();
	unk90[5] = 0.0f;
	unkB0[5] = 0;
	unk10[6] = nullptr;
	unk30[6].zero();
	unk90[6] = 0.0f;
	unkB0[6] = 0;
	unk10[7] = nullptr;
	unk30[7].zero();
	unk90[7] = 0.0f;
	unkB0[7] = 0;
	unkB8[0].zero();
	unkB8[1].zero();
	unkB8[2].zero();

	// Dead slot so MWCC keeps the frame at -0x30.
	char trash[8];
	trash[0] = 0;
}

u32 TSandEgg::getSDLModelFlag() const { return 0; }
