#include <MoveBG/MapObjMonte.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.
// Bodies below the matched function are stubs so the TU's symbols exist.

void TMapObjMonteRoot::initMapObj() { }

BOOL TJumpMushroom::receiveMessage(THitActor*, unsigned long)
{
	startAnim(1);
	return TRUE;
}

void TJumpMushroom::load(JSUMemoryInputStream&) { }

void THangingBridgeBoard::drawOneRope(const JGeometry::TVec3<f32>&) const { }

void THangingBridgeBoard::drawRopes() const { }

void THangingBridgeBoard::push(f32) { }

void THangingBridgeBoard::pushNeighbor(f32) { }

void THangingBridgeBoard::control() { }

void THangingBridgeBoard::calcDefaultMtx() { }

void THangingBridgeBoard::setGroundCollision() { }

void THangingBridgeBoard::initMapObj() { }

THangingBridgeBoard::THangingBridgeBoard(const char* name)
    : TLeanBlock(name)
{
}

void THangingBridge::drawLowerMinus(const JGeometry::TVec3<f32>&,
                                    const JGeometry::TVec3<f32>&,
                                    const JGeometry::TVec2<f32>&, int) const
{
}

void THangingBridge::drawLowerPlus(const JGeometry::TVec3<f32>&,
                                   const JGeometry::TVec3<f32>&,
                                   const JGeometry::TVec2<f32>&, int) const
{
}

void THangingBridge::drawUpper(const JGeometry::TVec3<f32>&,
                               const JGeometry::TVec3<f32>&,
                               const JGeometry::TVec2<f32>&, int) const
{
}

void THangingBridge::setDrawPos(int, f32, JGeometry::TVec3<f32>*) const { }

void THangingBridge::drawRopeBetweenBoards(f32, int) const { }

void THangingBridge::initDraw() const { }

void THangingBridge::perform(unsigned long, JDrama::TGraphics*) { }

void THangingBridge::initMonte() { }

void THangingBridge::loadAfter() { }

THangingBridge::THangingBridge(const char* name)
    : JDrama::TViewObj(name)
{
}

void TSwingBoard::drawOneRope(const JGeometry::TVec3<f32>&,
                              const JGeometry::TVec3<f32>&) const
{
}

void TSwingBoard::initDraw() const { }

void TSwingBoard::draw() const { }

void TSwingBoard::swing() { }

void TSwingBoard::control() { }

void TSwingBoard::load(JSUMemoryInputStream&) { }

TSwingBoard::TSwingBoard(const char* name)
    : TMapObjBase(name)
{
}

void TGoalFlag::touchActor(THitActor*) { }

void TGoalFlag::initMapObj() { TMapObjBase::initMapObj(); }

u32 TFluff::touchWater(THitActor*) { return 0; }

void TFluff::move() { }

void TFluff::kill() { }

void TFluff::control() { }

void TFluff::appear() { }

void TFluff::initMapObj()
{
	TMapObjBase::initMapObj();
	unk138 = 300.0f;
	unk13C = 0.5f;
}

TFluff::TFluff(const char* name)
    : TMapObjBase(name)
{
}

void TFluffManager::findNextFluff() { }

void TFluffManager::control() { }

void TFluffManager::registerNextFluff(TFluff*) { }

void TFluffManager::setUpNextFluff() { }

void TFluffManager::newFluff(const char*) { }

void TFluffManager::getRandomX() const { }

void TFluffManager::getRandomZ() const { }

void TFluffManager::loadAfter() { }

void TFluffManager::load(JSUMemoryInputStream&) { }

TFluffManager::TFluffManager(const char* name)
    : TMapObjBase(name)
{
}
