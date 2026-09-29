#include <MoveBG/MapObjMonte.hpp>
#include <Map/MapCollisionManager.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>
#include <System/FlagManager.hpp>
#include <JSystem/J3D/J3DGraphBase/J3DSys.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <System/MarDirector.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.
// Bodies below the matched function are stubs so the TU's symbols exist.

void TMapObjMonteRoot::initMapObj()
{
	TMapObjBase::initMapObj();
	mDamageHeight = 1400.0f * mScaling.y;
	calcEntryRadius();
	mPosition.y = mInitialPosition.y + mYOffset;

	// Dead slot so MWCC keeps frame -0x20.
	char trash[1];
	trash[0] = 0;
}

BOOL TJumpMushroom::receiveMessage(THitActor*, unsigned long)
{
	startAnim(1);
	return TRUE;
}

void TJumpMushroom::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	s32 value;
	stream.read(&value, 4);
	if (mMapCollisionManager != nullptr)
		mMapCollisionManager->unk8->setAllData(value);

	// Dead slot so MWCC keeps frame -0x28.
	char trash[1];
	trash[0] = 0;
}

// dont_inline: keep the call in drawRopes.
#pragma dont_inline on
void THangingBridgeBoard::drawOneRope(const JGeometry::TVec3<f32>& pos) const
{
	f32 yBot = pos.y;
	f32 x1   = pos.x + mRopeWidthX;
	f32 x0   = pos.x - mRopeWidthX;
	f32 z1   = pos.z + mRopeWidthZ;
	f32 z0   = pos.z - mRopeWidthZ;
	f32 yTop = pos.y + THangingBridge::mRopeHeight;
	f32 tTop = mTexPosRate * (yTop - pos.y);
	f32 tBot = mTexPosRate * (yBot - pos.y);

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
	GXPosition3f32(pos.x, yTop, z1);
	GXTexCoord2f32(0.0f, tTop);
	GXPosition3f32(pos.x, yBot, z1);
	GXTexCoord2f32(0.0f, tBot);
	GXPosition3f32(x0, yTop, z0);
	GXTexCoord2f32(1.0f, tTop);
	GXPosition3f32(x0, yBot, z0);
	GXTexCoord2f32(1.0f, tBot);
	GXPosition3f32(x1, yTop, z0);
	GXTexCoord2f32(2.0f, tTop);
	GXPosition3f32(x1, yBot, z0);
	GXTexCoord2f32(2.0f, tBot);
	GXPosition3f32(pos.x, yTop, z1);
	GXTexCoord2f32(3.0f, tTop);
	GXPosition3f32(pos.x, yBot, z1);
	GXTexCoord2f32(3.0f, tBot);
	// Dead slot so MWCC keeps frame -0x70.
	char trash[4];
	trash[0] = 0;
}
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

// Dead inline: a 0xC local under the mat-color temp, and a 0x4 return
// above it. No instructions.
struct TSwingDrawLow {
	char c[0xC];
};
struct TSwingDrawHigh {
	char c[4];
};
static inline TSwingDrawHigh swingDrawPad()
{
	TSwingDrawLow low;
	return *(TSwingDrawHigh*)(void*)&low;
}

// Extra 0x14 local plus a 4-byte return. The swing pad alone leaves
// THangingBridge's frame at 0xE0 with the texture slots 0x14 too low.
struct THangDrawLow {
	char c[0x14];
};
struct THangDrawHigh {
	char c[4];
};
static inline THangDrawHigh hangDrawPad()
{
	THangDrawLow low;
	return *(THangDrawHigh*)(void*)&low;
}

// dont_inline: empty stubs would otherwise fold into THangingBridge::perform.
#pragma dont_inline on
void THangingBridge::drawRopeBetweenBoards(f32, int) const { }
#pragma dont_inline off

void THangingBridge::initDraw() const
{
	swingDrawPad();
	hangDrawPad();
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
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	if (gpMarDirector->getCurrentMap() == 0xD) {
		JUTTexture tex(gpMapObjManager->unkCC);
		tex.load(GX_TEXMAP0);
	} else {
		JUTTexture tex(gpMapObjManager->unkCC);
		tex.load(GX_TEXMAP0);
	}
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

f32 THangingBridgeBoard::mRopeWidthX = 10.0f;
f32 THangingBridgeBoard::mRopeWidthZ = 7.0f;
f32 THangingBridgeBoard::mTexPosRate = 0.01f;

f32 TSwingBoard::mRopeWidthX = 10.0f;
f32 TSwingBoard::mRopeWidthZ = 7.0f;
f32 TSwingBoard::mTexPosRate = 0.01f;

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

void TSwingBoard::drawOneRope(const JGeometry::TVec3<f32>& from,
                              const JGeometry::TVec3<f32>& to) const
{
	// Second endpoint first so r5 lands in f23.
	// t is computed after the adds so it lands in f25.
	f32 bx1 = to.x + mRopeWidthX;
	f32 bx0 = to.x - mRopeWidthX;
	f32 bz1 = to.z + mRopeWidthZ;
	f32 bz0 = to.z - mRopeWidthZ;
	f32 ax1 = from.x + mRopeWidthX;
	f32 ax0 = from.x - mRopeWidthX;
	f32 az1 = from.z + mRopeWidthZ;
	f32 az0 = from.z - mRopeWidthZ;
	f32 t   = unk138 * mTexPosRate;

	GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 8);
	GXPosition3f32(from.x, from.y, az1);
	GXTexCoord2f32(0.0f, t);
	GXPosition3f32(to.x, to.y, bz1);
	GXTexCoord2f32(0.0f, 0.0f);
	GXPosition3f32(ax1, from.y, az0);
	GXTexCoord2f32(1.0f, t);
	GXPosition3f32(bx1, to.y, bz0);
	GXTexCoord2f32(1.0f, 0.0f);
	GXPosition3f32(ax0, from.y, az0);
	GXTexCoord2f32(2.0f, t);
	GXPosition3f32(bx0, to.y, bz0);
	GXTexCoord2f32(2.0f, 0.0f);
	GXPosition3f32(from.x, from.y, az1);
	GXTexCoord2f32(3.0f, t);
	GXPosition3f32(to.x, to.y, bz1);
	GXTexCoord2f32(3.0f, 0.0f);
}

void TSwingBoard::initDraw() const
{
	swingDrawPad();
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
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	JUTTexture tex(gpMapObjManager->unkCC);
	tex.load(GX_TEXMAP0);
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

void TSwingBoard::draw() const { }

void TSwingBoard::swing() { }

void TSwingBoard::control() { }

void TSwingBoard::load(JSUMemoryInputStream&) { }

TSwingBoard::TSwingBoard(const char* name)
    : TMapObjBase(name)
{
	// Store order is the retail order. MWCC does not reorder these.
	unk138 = 5000.0f;
	unk13C = 0.0f;
	unk140 = 0.0f;
	unk144 = 0.0f;
	unk148 = 0.0f;
	unk188 = 0;
	unk178 = 0.0f;
	unk168 = 0.0f;
	unk158 = 0.0f;
	unk164 = 0.0f;
	unk154 = 0.0f;
	unk170 = 0.0f;
	unk150 = 0.0f;
	unk16C = 0.0f;
	unk15C = 0.0f;
	unk174 = 1.0f;
	unk160 = 1.0f;
	unk14C = 1.0f;
	unk184 = 0.0f;
	unk180 = 0.0f;
	unk17C = 0.0f;

	// Dead slot so MWCC keeps frame -0x48 (r31 at r1+0x44).
	char trash[0x26];
	trash[0] = 0;
}

void TGoalFlag::touchActor(THitActor* actor)
{
	if (actor->isActorType(0x80000001)) {
		if (!TFlagManager::smInstance->getBool(0x50005))
			TFlagManager::smInstance->setBool(true, 0x50005);
		actor->receiveMessage(this, HIT_MESSAGE_ATTACK);
	} else if (actor->isActorType(0x08000002)) {
		actor->receiveMessage(this, HIT_MESSAGE_ATTACK);
	}
	char trash[1];
	trash[0] = 0;
}

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

void TFluffManager::load(JSUMemoryInputStream& stream)
{
	TMapObjBase::load(stream);
	stream.read(&unk138.z, 4);
	f32 scale;
	stream.read(&scale, 4);
	scale *= 0.01f;
	stream.read(&unk144, 4);
	unk138.x = 5000.0f;
	unk138.y = 5000.0f;
	unk154 = 0.998f;

	TPosition3f mtx;
	MsMtxSetXYZRPH(mtx, 0.0f, 0.0f, 0.0f, mRotation.x, mRotation.y,
	               mRotation.z);
	unk148.set(0.0f, 0.0f, 1.0f);
	mtx.mult(unk148, unk148);
	unk148.scale(scale);
}

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
