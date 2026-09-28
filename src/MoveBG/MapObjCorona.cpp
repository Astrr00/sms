#include "MoveBG/MapObjCorona.hpp"
#include "MoveBG/MapObjBase.hpp"
#include <M3DUtil/MActor.hpp>

// Incomplete: only getRootJointMtx is defined here. Not a TLiveActor
// subclass, so this TU does not emit the grip vtable.
class TBathtubGrip {
public:
	Mtx* getRootJointMtx() const;

	/* 0x0 */ u8 pad[0x200];
	/* 0x200 */ s32 unk200[1];
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

Mtx* TBathtubGrip::getRootJointMtx() const
{
	return (Mtx*)reinterpret_cast<const TLiveActor*>(this)
	    ->getModel()
	    ->getBaseTRMtx();
}

void TBathtub::loadAfter() { }

void TBathtub::hipdrop(const JGeometry::TVec3<f32>&) { }

void TBathtub::quake(const JGeometry::TVec3<f32>&) { }

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

void TBathtub::tumble(f32, f32) { }

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

bool TBathtub::allowsTumble() const { return false; }

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

TBathtub::TBathtub(const char* name)
    : TMapObjBase(name)
{
}

void TBathtub::load(JSUMemoryInputStream&) { }

u8 TBathtub::getNumKillerLaunchable() const { return 0; }

bool TBathtub::isKillerAttackable() const { return unk248 <= 0; }

u8 TBathtub::getNumKillerBurstable() const { return 0; }

// Unused
bool TBathtub::isBreaking() const { return false; }

// Unused
bool TBathtub::isKillerLaunchable() const { return false; }

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
