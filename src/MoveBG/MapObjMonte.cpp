#include <MoveBG/MapObjMonte.hpp>
#include <Map/MapCollisionManager.hpp>
#include <Player/MarioAccess.hpp>
#include <Player/Yoshi.hpp>
#include <System/FlagManager.hpp>
#include <JSystem/J3D/J3DGraphBase/J3DSys.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <MoveBG/MapObjManager.hpp>
#include <System/MarDirector.hpp>
#include <MSound/MSound.hpp>
#include <MSound/SoundEffects.hpp>
#include <MarioUtil/RandomUtil.hpp>

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

// Dead slot so the frame lands at -0x118.
static inline void hangingBoardControlPad()
{
	char trash[0x78];
	trash[0] = 0;
}

static inline void pullBoard(THangingBridgeBoard* board, f32 accel, f32 scale)
{
	f32 vel  = board->mVelocity.y;
	f32 kick = accel * scale;
	board->mVelocity.y = vel - kick;
}

void THangingBridgeBoard::control()
{
	TLeanBlock::control();

	if (marioIsOn()) {
		mVelocity.y -= mMarioAccelY;
		f32 accel = mMarioAccelY;
		if (unk194 != nullptr) {
			pullBoard(unk194, accel, unk1BC->unk3C.y);
			if (unk19C != nullptr)
				pullBoard(unk19C, accel, unk1BC->unk3C.z);
		}
		if (unk198 != nullptr) {
			pullBoard(unk198, accel, unk1BC->unk3C.y);
			if (unk1A0 != nullptr)
				pullBoard(unk1A0, accel, unk1BC->unk3C.z);
		}
	}

	if (marioHipAttack()) {
		mVelocity.y -= mMarioHipDropAccelY;
		f32 accel = mMarioHipDropAccelY;
		if (unk194 != nullptr) {
			pullBoard(unk194, accel, unk1BC->unk3C.y);
			if (unk19C != nullptr)
				pullBoard(unk19C, accel, unk1BC->unk3C.z);
		}
		if (unk198 != nullptr) {
			pullBoard(unk198, accel, unk1BC->unk3C.y);
			if (unk1A0 != nullptr)
				pullBoard(unk1A0, accel, unk1BC->unk3C.z);
		}
	}

	mPosition.y += mVelocity.y;
	mVelocity.y += mReturnAccelRate * (mInitialPosition.y - mPosition.y);
	mVelocity.y *= mSpeedDownRate;

	MtxPtr mtx = getModel()->getAnmMtx(0);
	unk1A4[0].x = mPosition.x - mtx[0][0] * unk1BC->unk3C.x;
	unk1A4[0].y = mPosition.y - mtx[1][0] * unk1BC->unk3C.x + 70.0f;
	unk1A4[0].z = mPosition.z - mtx[2][0] * unk1BC->unk3C.x;
	unk1A4[1].x = mPosition.x + mtx[0][0] * unk1BC->unk3C.x;
	unk1A4[1].y = mPosition.y + mtx[1][0] * unk1BC->unk3C.x + 70.0f;
	unk1A4[1].z = mPosition.z + mtx[2][0] * unk1BC->unk3C.x;
	hangingBoardControlPad();
}

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

void THangingBridge::drawLowerMinus(const JGeometry::TVec3<f32>& start,
                                    const JGeometry::TVec3<f32>& end,
                                    const JGeometry::TVec2<f32>& width,
                                    int count) const
{
	f32 x   = start.x;
	f32 y   = start.y;
	f32 z   = start.z;
	f32 inv = 1.0f / (f32)count;
	f32 dx  = (end.x - start.x) * inv;
	f32 dy  = (end.y - start.y) * inv;
	f32 dz  = (end.z - start.z) * inv;
	for (int i = 0; i < count; ++i) {
		f32 h = y - ((f32*)unk38)[i];
		f32 t = mBetweenBoardsTexPosRate * (x + z);
		GXPosition3f32(x - width.x, h, z - width.y);
		GXTexCoord2f32(0.0f, t);
		GXPosition3f32(x, h - mRopeWidthBetweenBoardsY, z);
		GXTexCoord2f32(1.0f, t);
		x += dx;
		y += dy;
		z += dz;
	}
}

void THangingBridge::drawLowerPlus(const JGeometry::TVec3<f32>& start,
                                   const JGeometry::TVec3<f32>& end,
                                   const JGeometry::TVec2<f32>& width,
                                   int count) const
{
	f32 x   = start.x;
	f32 y   = start.y;
	f32 z   = start.z;
	f32 inv = 1.0f / (f32)count;
	f32 dx  = (end.x - start.x) * inv;
	f32 dy  = (end.y - start.y) * inv;
	f32 dz  = (end.z - start.z) * inv;
	for (int i = 0; i < count; ++i) {
		f32 h = y - ((f32*)unk38)[i];
		f32 t = mBetweenBoardsTexPosRate * (x + z);
		GXPosition3f32(x, h - mRopeWidthBetweenBoardsY, z);
		GXTexCoord2f32(0.0f, t);
		GXPosition3f32(x + width.x, h, z + width.y);
		GXTexCoord2f32(1.0f, t);
		x += dx;
		y += dy;
		z += dz;
	}
}

void THangingBridge::drawUpper(const JGeometry::TVec3<f32>& start,
                               const JGeometry::TVec3<f32>& end,
                               const JGeometry::TVec2<f32>& width,
                               int count) const
{
	f32 x   = start.x;
	f32 y   = start.y;
	f32 z   = start.z;
	f32 inv = 1.0f / (f32)count;
	f32 dx  = (end.x - start.x) * inv;
	f32 dy  = (end.y - start.y) * inv;
	f32 dz  = (end.z - start.z) * inv;
	for (int i = 0; i < count; ++i) {
		f32 h = y - ((f32*)unk38)[i];
		f32 t = mBetweenBoardsTexPosRate * (x + z);
		GXPosition3f32(x + width.x, h, z + width.y);
		GXTexCoord2f32(0.0f, t);
		GXPosition3f32(x - width.x, h, z - width.y);
		GXTexCoord2f32(1.0f, t);
		x += dx;
		y += dy;
		z += dz;
	}
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

// Args are evaluated before the stores, so x/y/z stay in f0/f1/f2.
static inline void storeRopePos(JGeometry::TVec3<f32>& dst, f32 x, f32 y,
                                 f32 z)
{
	dst.x = x;
	dst.y = y;
	dst.z = z;
}

// dont_inline: perform must keep the out-of-line call.
#pragma dont_inline on
void THangingBridge::drawRopeBetweenBoards(f32 height, int count) const
{
	// latX stays in f31 with unk30 still live in f1. latZ is loaded
	// straight into f30 and multiplied in place, so width.y reloads unk34.
	f32 latX = unk30 * unk3C.x;
	f32 latZ = unk34;
	latZ *= unk3C.x;

	// top keeps 8 bytes between width and the saved GPRs (frame 0x108).
	char top[8];
	JGeometry::TVec2<f32> width;
	width.x = unk30;
	width.y = unk34;
	width.scale(mRopeWidthBetweenBoards);
	top[0] = 0;

	// (boards + the two end calls) * 2 verts. Kept as u16 so each
	// GXBegin is a move from r31, not a second mask.
	u16 vtxCount = (u16)((unk10 + 2) * count) * 2;

	// Dead slot under the vecs. end r1+0xAC, start r1+0xB8, width r1+0xC4.
	JGeometry::TVec3<f32> start;
	JGeometry::TVec3<f32> end;
	char pad[0x48];
	pad[0] = 0;

	// Six strips. The second end call is a zero-length segment; the
	// vertex count includes it. The last strip's index lands in r31.
#define DRAW_PASS(op, point, drawFn)                                           \
	do {                                                                       \
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, vtxCount);                       \
		storeRopePos(start, unk18.x op latX, unk18.y + height,                 \
		             unk18.z op latZ);                                         \
		for (int i = 0; i < (int)unk10; ++i) {                                 \
			THangingBridgeBoard* board                                         \
			    = ((THangingBridgeBoard**)unk14)[i];                           \
			*(Vec*)&end = *(Vec*)&board->unk1A4[point];                        \
			end.y += height;                                                   \
			drawFn(start, end, width, count);                                  \
			*(Vec*)&start = *(Vec*)&end;                                       \
		}                                                                      \
		storeRopePos(end, unk24.x op latX, unk24.y + height,                   \
		             unk24.z op latZ);                                         \
		drawFn(start, end, width, count);                                      \
		*(Vec*)&start = *(Vec*)&end;                                           \
		drawFn(start, end, width, count);                                      \
	} while (0)

	DRAW_PASS(+, 0, drawLowerMinus);
	DRAW_PASS(+, 0, drawLowerPlus);
	DRAW_PASS(+, 0, drawUpper);
	DRAW_PASS(-, 1, drawLowerMinus);
	DRAW_PASS(-, 1, drawLowerPlus);
	DRAW_PASS(-, 1, drawUpper);
#undef DRAW_PASS
}
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

f32 THangingBridgeBoard::mMarioAccelY        = 0.15f;
f32 THangingBridgeBoard::mMarioHipDropAccelY = 2.0f;
f32 THangingBridgeBoard::mReturnAccelRate    = 0.005f;
f32 THangingBridgeBoard::mSpeedDownRate      = 0.98f;
f32 THangingBridgeBoard::mRopeWidthX         = 10.0f;
f32 THangingBridgeBoard::mRopeWidthZ = 7.0f;
f32 THangingBridgeBoard::mTexPosRate = 0.01f;

f32 TSwingBoard::mRopeWidthX = 10.0f;
f32 TSwingBoard::mRopeWidthZ = 7.0f;
f32 TSwingBoard::mTexPosRate = 0.01f;

f32 THangingBridge::mRopeWidthBetweenBoards = 10.0f;
f32 THangingBridge::mRopeWidthBetweenBoardsY = 10.0f;
int THangingBridge::mPointNumBetweenBoards = 10;
f32 THangingBridge::mBetweenBoardsTexPosRate = 0.01f;
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

// Dead slot under the velocity copy so the frame lands at -0x70.
static inline void fluffMovePad()
{
	char trash[0x40];
	trash[0] = 0;
}

void TFluff::move()
{
	mPosition.y -= unk13C;
	if (mPosition.y < 0.0f)
		mPosition.y = 5000.0f;

	unk154.x += unk150 * gpMapObjManager->unkD0.x;
	unk154.z += unk150 * gpMapObjManager->unkD0.z;

	Vec vel = mVelocity;
	unk154.x += vel.x;
	unk154.y += vel.y;
	unk154.z += vel.z;

	f32 drag = unk164;
	mVelocity.x *= drag;
	mVelocity.y *= drag;
	mVelocity.z *= drag;

	f32 s   = sinf(3.14f * unk148 / 180.0f);
	f32 amp = unk138 * s;
	mPosition.x = unk154.x + (amp * (unk144 + unk140) + mInitialPosition.x);
	mPosition.y += unk150 * gpMapObjManager->unkD0.y;
	mPosition.z = unk154.z + (amp * (unk140 - unk144) + mInitialPosition.z);

	if (reinterpret_cast<JGeometry::TVec3<f32>&>(gpMapObjManager->unkD0)
	        .isZero()) {
		unk148 += unk14C;
		if (unk148 > 360.0f)
			unk148 -= 360.0f;
	}

	if (mHeldObject != nullptr && mHeldObject->isActorType(0x80000001))
		gpMarioPos->y -= unk13C;
	fluffMovePad();
}

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

static inline f32 loadVol(volatile f32& slot) { return slot; }

static inline f32 distSq(const JGeometry::TVec3<f32>& a,
                         const JGeometry::TVec3<f32>& b)
{
	f32 dx = a.x - b.x;
	f32 dy = a.y - b.y;
	f32 dz = a.z - b.z;
	f32 xx = dx * dx;
	f32 yy = dy * dy;
	f32 zz = dz * dz;
	return zz + (xx + yy);
}

static inline f32 doSqrt(f32 lenSq) { return JGeometry::TUtil<f32>::sqrt(lenSq); }

f32 TFluffManager::mWindMin = 1.0f;

void TFluffManager::control()
{
	char trash[0x38];
	trash[0] = 0;
	switch (mState) {
	case 1:
		if (unk15C == nullptr
		    && unk158->mPosition.y - 100.0f < mPosition.y - unk138.z) {
			{
				for (int i = 3; i < (int)unk164; ++i) {
					TFluff* fluff = unk168[i];
					if (fluff->unk16C != 0)
						continue;
					if (fluff->mHeldObject != nullptr)
						continue;
					const JGeometry::TVec3<f32>& pos = fluff->mPosition;
					const JGeometry::TVec3<f32>& mario = *gpMarioPos;
					if (!(doSqrt(distSq(pos, mario)) > 3000.0f))
						continue;
					unk15C = unk168[i];
					unk168[i]->kill();
					break;
				}
			}
		}
		if (unk158->mPosition.y < mPosition.y - unk138.z) {
			const JGeometry::TVec3<f32>& soundPos = unk158->mPosition;
			gpMSound->startSoundActor(MSD_SE_OBJ_WATAGE_WIND, &soundPos, 0,
			                          nullptr, 0, 4);
			startStateTimer(unk144);
			mState = 2;
		}
		break;
	case 2: {
		TMapObjManager* mgr = gpMapObjManager;
		f32 x               = mgr->unkD0.x;
		f32 ax              = unk148.x;
		f32 ay              = loadVol(unk148.y);
		f32 y               = mgr->unkD0.y;
		x += ax;
		y += ay;
		f32 z  = mgr->unkD0.z;
		f32 az = unk148.z;
		z += az;
		mgr->unkD0.x = x;
		mgr->unkD0.y = y;
		mgr->unkD0.z = z;
		if (!isStateTimerEngaged())
			mState = 3;
		break;
	}
	case 3: {
		Vec& wind = gpMapObjManager->unkD0;
		f32 wx    = wind.x;
		f32 s     = unk154;
		f32 wy    = wind.y;
		f32 wz    = wind.z;
		wx *= s;
		wy *= s;
		wz *= s;
		if (fabsf(wx) < mWindMin && fabsf(wy) < mWindMin
		    && fabsf(wz) < mWindMin) {
			wx     = 0.0f;
			wy     = wx;
			wz     = wx;
			unk158 = unk15C;
			{
				JGeometry::TVec3<f32>& rot = unk158->mRotation;
				rot.x                      = mRotation.x;
				rot.y                      = mRotation.y;
				rot.z                      = mRotation.z;
			}
			*(Vec*)&unk158->mInitialRotation = *(Vec*)&mRotation;
			unk158->appear();
			{
				JGeometry::TVec3<f32>& pos = unk158->mPosition;
				pos.x                      = mPosition.x;
				pos.y                      = mPosition.y;
				pos.z                      = mPosition.z;
			}
			*(Vec*)&unk158->mInitialPosition = *(Vec*)&mPosition;
			{
				f32 zero                       = 0.0f;
				JGeometry::TVec3<f32>& rot = unk158->mRotation;
				rot.x                      = zero;
				rot.y                      = zero;
				rot.z                      = zero;
			}
			*(Vec*)&unk158->mInitialRotation = *(Vec*)&unk158->mRotation;
			unk158->unk148                   = 0.0f;
			unk158->unk150                   = 1.0f;
			unk158->unk16C                   = 1;
			unk15C                           = nullptr;
			mState                           = 1;
		}
		{
			TMapObjManager* mgr = gpMapObjManager;
			mgr->unkD0.x        = wx;
			mgr->unkD0.y        = wy;
			mgr->unkD0.z        = wz;
		}
		break;
	}
	default:
		break;
	}
}

void TFluffManager::registerNextFluff(TFluff*) { }

void TFluffManager::setUpNextFluff() { }

void TFluffManager::newFluff(const char*) { }

void TFluffManager::getRandomX() const { }

void TFluffManager::getRandomZ() const { }

void TFluffManager::loadAfter()
{
	JGeometry::TVec3<f32> initial;
	// Dead slot so MWCC keeps frame -0x78 and the vec at r1+0x30.
	char trash[24];
	trash[0] = 0;
	unk160 = 0;
	unk164 = 32;
	unk168 = new TFluff*[unk164];

	{
		TFluff* fluff = new TFluff("１つ目のわた毛");
		fluff->initAndRegister("Fluff");
		fluff->unk168  = (u32)this;
		unk158         = fluff;
		unk158->unk16C = 1;
		unk158->appear();
		{
			JGeometry::TVec3<f32>& pos = unk158->mPosition;
			pos.x                      = mPosition.x;
			pos.y                      = mPosition.y;
			pos.z                      = mPosition.z;
		}
		{
			JGeometry::TVec3<f32>& rot = unk158->mRotation;
			rot.x                      = mRotation.x;
			rot.y                      = mRotation.y;
			rot.z                      = mRotation.z;
		}
		// Declared y-then-z so the first scatter keeps Z in f30 and Y in f31.
		f32 y;
		f32 z;
		z = unk138.y * (MsRandF() * 2.0f - 1.0f);
		y = mPosition.y * MsRandF();
		f32 x = unk138.x * (MsRandF() * 2.0f - 1.0f);
		initial.x                        = x;
		initial.y                        = y;
		initial.z                        = z;
		*(Vec*)&unk158->mInitialPosition = *(Vec*)&initial;
		unk168[unk160]                   = unk158;
		unk160 += 1;
	}
	{
		TFluff* fluff = new TFluff("２つ目のわた毛");
		fluff->initAndRegister("Fluff");
		fluff->unk168 = (u32)this;
		unk15C        = fluff;
		{
			JGeometry::TVec3<f32>& pos = unk15C->mPosition;
			pos.x                      = mPosition.x;
			pos.y                      = mPosition.y;
			pos.z                      = mPosition.z;
		}
		{
			JGeometry::TVec3<f32>& rot = unk15C->mRotation;
			rot.x                      = mRotation.x;
			rot.y                      = mRotation.y;
			rot.z                      = mRotation.z;
		}
		f32 z = unk138.y * (MsRandF() * 2.0f - 1.0f);
		f32 y = mPosition.y * MsRandF();
		f32 x = unk138.x * (MsRandF() * 2.0f - 1.0f);
		initial.x                        = x;
		initial.y                        = y;
		initial.z                        = z;
		*(Vec*)&unk15C->mInitialPosition = *(Vec*)&initial;
		unk15C->makeObjDead();
		unk168[unk160] = unk15C;
		unk160 += 1;
	}
	for (int i = 2; i < (int)unk164; ++i) {
		TFluff* fluff = new TFluff("わた毛");
		fluff->initAndRegister("Fluff");
		fluff->unk168  = (u32)this;
		unk168[unk160] = fluff;
		unk168[unk160]->appear();
		unk160 += 1;
	}
}

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
