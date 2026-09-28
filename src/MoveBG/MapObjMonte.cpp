#include <MoveBG/MapObjMonte.hpp>
#include <Map/MapCollisionManager.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>

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

// dont_inline: empty stub would otherwise fold into the rope draw.
#pragma dont_inline on
void THangingBridgeBoard::drawOneRope(const JGeometry::TVec3<f32>&) const { }
#pragma dont_inline off

// Extra inline level so the inlined copy keeps the dead stack slot (frame -0x40).
static inline const JGeometry::TVec3<f32>&
boardRopePoint(const THangingBridgeBoard* board, int index)
{
	return board->unk1A4[index];
}

void THangingBridgeBoard::drawRopes() const
{
	JGeometry::TVec3<f32> pos = boardRopePoint(this, 0);
	drawOneRope(pos);
	pos = boardRopePoint(this, 1);
	drawOneRope(pos);
}

void THangingBridgeBoard::push(f32) { }

void THangingBridgeBoard::pushNeighbor(f32) { }

void THangingBridgeBoard::control() { }

void THangingBridgeBoard::calcDefaultMtx()
{
	Mtx rotX;
	Mtx rotY;
	makeRootMtxRotX(rotX);
	makeRootMtxRotY(rotY);
	MTXConcat(rotY, rotX, rotY);
	mDefaultMtx.set(rotY);
	mVelocity.y    = 0.0f;
	mPosition.y    = mInitialPosition.y;
}

// Extra inline level so MWCC keeps the dead 8-byte temp (frame -0x40).
static inline MtxPtr hangingBoardAnmMtx(THangingBridgeBoard* board)
{
	return board->getModel()->getAnmMtx(0);
}

void THangingBridgeBoard::setGroundCollision()
{
	if (SMS_GetYoshi()->isHatched()
	    && mPosition.x - mBodyRadius < SMS_GetYoshi()->getTranslation().x
	    && mPosition.x + mBodyRadius > SMS_GetYoshi()->getTranslation().x
	    && mPosition.z - mBodyRadius < SMS_GetYoshi()->getTranslation().z
	    && mPosition.z + mBodyRadius > SMS_GetYoshi()->getTranslation().z) {
		MtxPtr mtx = hangingBoardAnmMtx(this);
		if (mMapCollisionManager->unk8)
			mMapCollisionManager->unk8->moveMtx(mtx);
	} else {
		TMapObjBase::setGroundCollision();
	}
}

void THangingBridgeBoard::initMapObj()
{
	TLeanBlock::initMapObj();
	unk140 = 0.01f;
	unk144 = 0.02f;
	unk148 = 0.08f;
}

THangingBridgeBoard::THangingBridgeBoard(const char* name)
    : TLeanBlock(name)
{
	unk1BC = 0;
	unk194 = 0;
	unk198 = 0;
	unk19C = 0;
	unk1A0 = 0;
	unk1A4[0].zero();
	unk1A4[1].zero();
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

// dont_inline: empty stubs would otherwise fold into THangingBridge::perform.
#pragma dont_inline on
void THangingBridge::drawRopeBetweenBoards(f32, int) const { }

void THangingBridge::initDraw() const { }
#pragma dont_inline off

int THangingBridge::mPointNumBetweenBoards = 10;
f32 THangingBridge::mRopeHeight;

void THangingBridge::perform(unsigned long cue, JDrama::TGraphics*)
{
	if (cue & CUE_DRAW) {
		initDraw();
		for (int i = 0; i < (int)unk10; ++i)
			((THangingBridgeBoard**)unk14)[i]->drawRopes();
		drawRopeBetweenBoards(0.0f, mPointNumBetweenBoards);
		drawRopeBetweenBoards(mRopeHeight, 1);
	}
}

void THangingBridge::initMonte() { }

void THangingBridge::loadAfter() { }

THangingBridge::THangingBridge(const char* name)
    : JDrama::TViewObj(name)
    , unk10(0)
    , unk14(0)
    , unk38(0)
    , unk3C(0.0f)
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

u32 TFluff::touchWater(THitActor* actor)
{
	const JGeometry::TVec3<f32>& water = TMapObjBase::getWaterPos(actor);
	JGeometry::TVec3<f32> normal;
	getNormalVecFromTarget(water.x, water.y, water.z, &normal);
	mVelocity.x -= normal.x * unk160;
	mVelocity.y -= normal.y * unk160;
	mVelocity.z -= normal.z * unk160;
	return 1;
}

void TFluff::move() { }

void TFluff::kill()
{
	if (mHeldObject != nullptr) {
		mHeldObject->receiveMessage(this, HIT_MESSAGE_UNK8);
		mHeldObject = nullptr;
	}
	mState = 3;
}

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
    , unk138(0.0f)
    , unk13C(0.0f)
    , unk140(0.0f)
    , unk144(0.0f)
    , unk148(0.0f)
    , unk14C(0.0f)
    , unk150(0.0f)
    , unk160(1.0f)
    , unk164(0.95f)
    , unk168(0)
    , unk16C(0)
{
	unk154.zero();
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
    , unk138(0.0f)
    , unk144(0)
    , unk154(0.0f)
    , unk158(0)
    , unk15C(0)
    , unk160(0)
    , unk164(0)
{
	unk148.setAll(0.0f);
}
