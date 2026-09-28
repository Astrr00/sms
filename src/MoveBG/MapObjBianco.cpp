#include <MoveBG/MapObjBianco.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TBigWindmill::control() { }

void TBigWindmill::load(JSUMemoryInputStream&) { }

void TMapObjRootPakkun::drawObject(JDrama::TGraphics*) { }

void TMapObjRootPakkun::initMapObj() { }

void TBiancoWatermill::turnByEnemy(THitActor*, const TBGCheckData*) { }

// UNUSED
void TBiancoWatermill::turn(const JGeometry::TVec3<f32>&, const TBGCheckData*,
                            f32)
{
}

u32 TBiancoWatermill::touchWater(THitActor*) { return 0; }

void TBiancoWatermill::control() { }

void TBiancoWatermill::initMapObj() { }

TBiancoWatermill::TBiancoWatermill(const char* name)
    : TMapObjBase(name)
    , unk138(0.3f)
    , unk13C(0)
{
}

u32 TBiancoWatermillVertical::touchWater(THitActor*) { return 0; }

void TBiancoWatermillVertical::setGroundCollision() { }

void TBiancoWatermillVertical::control() { }

void TBiancoWatermillVertical::loadAfter() { }

void TBiancoWatermillVertical::load(JSUMemoryInputStream&) { }

TBiancoWatermillVertical::TBiancoWatermillVertical(const char* name)
    : TMapObjBase(name)
{
}

u32 TBiancoMiniWindmill::touchWater(THitActor*) { return 0; }

void TBiancoMiniWindmill::calc() { }

void TBiancoMiniWindmill::control() { }

void TBiancoMiniWindmill::initMapObj() { }

TBiancoMiniWindmill::TBiancoMiniWindmill(const char* name)
    : THideObjBase(name)
{
}

void TLeafBoat::touchActor(THitActor*) { }

void TLeafBoat::touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*) { }

void TLeafBoat::bind() { }

void TLeafBoat::control() { }

void TLeafBoat::calc() { }

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

void TLeafBoatRotten::load(JSUMemoryInputStream&) { }

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

void TLampSeesaw::load(JSUMemoryInputStream&) { }

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

void TLampSeesawMain::loadAfter() { }

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

u32 TBiancoBell::touchWater(THitActor*) { return 0; }

void TBiancoBell::touchPlayer(THitActor*) { }

void TBiancoBell::initMapObj() { }

TBiancoBell::TBiancoBell(const char* name)
    : TMapObjBase(name)
    , unk138(0)
    , unk13A(0)
{
}

u32 TBellWatermill::touchWater(THitActor*) { return 0; }

void TBellWatermill::control() { }

void TBellWatermill::loadAfter() { }

TBellWatermill::TBellWatermill(const char* name)
    : TMapObjTurn(name)
{
}

void TWoodLog::control() { }
