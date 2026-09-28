#define PINNA_EMIT_MOVEMTX
#include <MoveBG/MapObjPinna.hpp>

#include <Map/MapCollisionEntry.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <Player/MarioAccess.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>
#include <System/Particles.hpp>
#include <Map/Map.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TFerrisWheel::becomeCalmlyCallback(u32, u32) { }

void TFerrisWheel::control() { }

void TFerrisWheel::initMapObj() { }

TFerrisWheel::TFerrisWheel(const char* name)
    : TMapObjBase(name)
{
}

void THorizontalViking::updateTrans() { }

void THorizontalViking::moveNormal() { }

void THorizontalViking::control() { }

// TODO: retail is fcmpo + ble. MWCC emits cror + bne for this <= test.
void THorizontalViking::reset()
{
	unk144 = unk140;
	unk148 = 0.0f;
	if (unk144 <= 0.0f)
		mState = 2;
	else
		mState = 1;
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
{
}

void TViking::roll() { }

void TViking::control() { }

// TODO: same ble/cror mismatch as THorizontalViking::reset.
void TViking::reset()
{
	unk144 = unk140;
	unk148 = 0.0f;
	if (unk140 <= 0.0f)
		mState = 2;
	else
		mState = 1;
}

void TViking::loadAfter()
{
	TMapObjBase::loadAfter();
	reset();
}

void TViking::initMapObj() { }

TViking::TViking(const char* name)
    : THorizontalViking(name)
{
}

void TPinnaShell::opened() { }

BOOL TPinnaShell::receiveMessage(THitActor*, u32) { return FALSE; }

// Retail keeps a weak copy in this TU. The shared header stays the virtual
// inline so other matches that call through the vtable do not change.
__declspec(weak) void TMapCollisionMove::moveMtx(MtxPtr mtx)
{
	MTXCopy(mtx, unk20);
	move();
}

void TPinnaShell::control() { }

TPinnaShell::TPinnaShell(const char* name)
    : THitActor(name)
{
}

void TShellCup::control() { }

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
}

void TChangeStageMerrygoround::touchPlayer(THitActor*) { }

void TChangeStageMerrygoround::calc() { }

void TBalloonKoopaJr::touchActor(THitActor*) { kill(); }

void TBalloonKoopaJr::kill() { }

void TBalloonKoopaJr::load(JSUMemoryInputStream&) { }

void TPinnaEntrance::loadAfter() { }

void TWaterRecoverObj::touchPlayer(THitActor*) { }

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

void TPinnaCoaster::initMapObj() { }

TPinnaCoaster::TPinnaCoaster(const char* name)
    : TMapObjBase(name)
{
}
