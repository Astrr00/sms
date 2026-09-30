#include <MoveBG/MapObjWave.hpp>

#include <System/MarDirector.hpp>
#include <Map/Map.hpp>
#include <Map/MapData.hpp>
#include <JSystem/JUtility/JUTColor.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <JSystem/J3D/J3DGraphBase/J3DSys.hpp>
#include <Player/MarioAccess.hpp>
#include <Camera/CubeManagerBase.hpp>
#include <dolphin/gx.h>
#include <math.h>
#include <stdlib.h>

TMapObjWave* gpMapObjWave;

static JUtility::TColor sColor;
static u8 sAlphaCompLarge = 0x55;
static u8 sAlphaCompSmall = 0x23;

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

// dont_inline: perform must keep the out-of-line bl.
#pragma dont_inline on
void TMapObjWave::initDraw()
{
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX1, GX_TEX_ST, GX_F32, 0);

	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX1, GX_DIRECT);

	GXLoadPosMtxImm(j3dSys.getViewMtx(), GX_PNMTX0);
	GXSetCurrentMtx(GX_PNMTX0);

	GXSetNumChans(1);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);

	GXSetNumTexGens(2);
	GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GXSetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY);

	JUTTexture tex(static_cast<const ResTIMG*>(unk94));
	tex.load(GX_TEXMAP0);

	GXSetTevColorS10(GX_TEVREG0, *(GXColorS10*)&unk7C);
	GXSetTevColorS10(GX_TEVREG1, *(GXColorS10*)&unk84);
	GXSetTevColorS10(GX_TEVREG2, *(GXColorS10*)&unk8C);

	GXSetNumTevStages(2);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
	                GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
	                GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);

	GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP0, GX_COLOR0A0);
	GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO,
	                GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE,
	                GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV,
	                GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, GX_TRUE,
	                GX_TEVPREV);

	GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_SRCCLR, GX_LO_NOOP);
	GXSetAlphaCompare(GX_GEQUAL, sAlphaCompLarge, GX_AOP_OR, GX_LEQUAL,
	                  sAlphaCompSmall);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
	GXSetCullMode(GX_CULL_NONE);
}
#pragma dont_inline off

f32 TMapObjWave::getMoveTexPos1(float z) const
{
	f32 scaled = z * unk78;
	return unk70 + scaled;
}

f32 TMapObjWave::getMoveTexPos0(float x) const { return x * unk78 * 0.8f; }

f32 TMapObjWave::getStaticTexPos1(float z) const { return z * unk74; }

f32 TMapObjWave::getStaticTexPos0(float x) const { return x * unk74; }

f32 TMapObjWave::getWaveHeight(float x, float z) const
{
	if (!unk94)
		return 0.0f;

	// Retail float bits. 1/(2*pi) from M_PI is a few bits low.
	f32 xWave = unk3C * sinf(unk24 * (0.15915507f * x) + unk64);
	f32 zWave = unk40 * sinf(unk28 * (0.15915507f * z) + unk68);
	return xWave + zWave;
}

// Inlined into getHeight. Not emitted.
static inline bool isWaveSurface(u16 type)
{
	if (type == BG_TYPE_WATER || type == BG_TYPE_DAMAGING_WATER
	    || (u16)(type - BG_TYPE_SEA_WATER) <= 3u
	    || type == BG_TYPE_SHADED_POOL)
		return true;
	return false;
}

// `==` in a condition is a direct bne. The if/return keeps li 1 / li 0.
static inline bool isType700(u16 type)
{
	if (type == 0x700)
		return true;
	return false;
}

// `||` of these two types folds into subi/bgt. The goto keeps both
// cmplwi checks, with li 1 before li 0.
static inline bool isSeaSurface(u16 type)
{
	if (type == BG_TYPE_SEA_WATER)
		goto yes;
	if (type != BG_TYPE_DAMAGING_SEA_WATER)
		goto no;
yes:
	return true;
no:
	return false;
}

f32 TMapObjWave::getHeight(float x, float y, float z) const
{
	const TBGCheckData* data;
	f32 ground = gpMap->checkGroundExactY(x, 50.0f + y, z, &data);
	u16 type   = data->mBGType;

	// unsigned char, not bool: a bool temporary moves the check
	// pointer from r1+0x1c to r1+0x18.
	unsigned char water = isWaveSurface(type);
	if (water) {
		unsigned char sea = isSeaSurface(type);
		if (sea) {
			if (!unk94)
				return 0.0f;

			f32 xWave = unk3C * sinf(unk24 * (0.15915507f * x) + unk64);
			f32 zWave = unk40 * sinf(unk28 * (0.15915507f * z) + unk68);
			return xWave + zWave;
		}
		return ground;
	}
	return y;
}

void TMapObjWave::noWave()
{
	unk34 = 0.0f;
	unk38 = 0.0f;
	unk2C = 0.0f;
	unk30 = 0.0f;
	unk3C = 0.0f;
	unk40 = 0.0f;
}

int TMapObjWave::getAlpha(float x, float z) const
{
	if (fabsf(x) > fabsf(z))
		return (int)(unk54 * (1.0f - unk18 * fabsf(x)));
	return (int)(unk54 * (1.0f - unk18 * fabsf(z)));
}

// dont_inline: updateHeightAndAlpha and updateTime must stay out of line
// so perform keeps its bls. draw is large enough to stay out of line too.
#pragma dont_inline on
void TMapObjWave::draw()
{
	for (f32 z = -unk14; z <= unk14 - unk1C; z += unk1C) {
		f32 z0 = z + gpMarioPos->z;
		f32 z1 = z0 + unk1C;
		GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, unk20 * 2);
		for (f32 x = -unk14; x <= unk14 - unk1C; x += unk1C) {
			f32 wx = x + gpMarioPos->x;
			int a0 = getAlpha(x, z);
			int a1 = getAlpha(x, z + unk1C);
			f32 y0 = getWaveHeight(wx, z0);
			GXPosition3f32(wx, y0, z0);
			GXColor4u8(sColor.r, sColor.g, sColor.b, a0);
			GXTexCoord2f32(unk6C + getStaticTexPos0(wx), getStaticTexPos1(z0));
			GXTexCoord2f32(getMoveTexPos0(wx), getMoveTexPos1(z0));
			f32 y1 = getWaveHeight(wx, z1);
			GXPosition3f32(wx, y1, z1);
			GXColor4u8(sColor.r, sColor.g, sColor.b, a1);
			GXTexCoord2f32(unk6C + getStaticTexPos0(wx),
			               getStaticTexPos1(z1));
			GXTexCoord2f32(getMoveTexPos0(wx), getMoveTexPos1(z1));
		}
		GXEnd();
	}
	char trash[0x8];
	trash[0] = 0;
}

void TMapObjWave::updateHeightAndAlpha()
{
	const TBGCheckData* groundData;
	const TBGCheckData* exactData;
	const JGeometry::TVec3<f32>& mario = *gpMarioPos;
	gpMap->checkGround(mario, &groundData);
	gpMap->checkGroundExactY(gpMarioPos->x, 10.0f, gpMarioPos->z, &exactData);

	if (SMS_CheckMarioFlag(MARIO_FLAG_IN_SHALLOW_WATER)
	    || isWaveSurface(exactData->mBGType)
	    || isWaveSurface(groundData->mBGType)) {
		f32 groundY = gpMap->checkGroundIgnoreWaterSurface(
		    gpMarioPos->x, 0.0f, gpMarioPos->z, &exactData);
		f32 rise = unk4C + groundY;
		if (rise < 0.0f || isType700(exactData->mBGType)) {
			unk3C = unk2C;
			unk40 = unk30;
		} else {
			f32 t = 1.0f - rise / unk4C;
			unk3C = t * (unk2C - unk34) + unk34;
			unk40 = t * (unk30 - unk38) + unk38;
		}

		f32 riseA = unk50 + groundY;
		if (riseA < 0.0f || isType700(exactData->mBGType)) {
			unk54 = unk58;
		} else {
			f32 t = 1.0f - riseA / unk50;
			unk54 = t * (unk58 - unk5C) + unk5C;
		}
	} else {
		unk3C = unk34;
		unk40 = unk38;
		unk54 = unk5C;
	}

	if (gpMarDirector->getCurrentMap() == 4
	    && -4950.0f < gpMarioPos->x && -4340.0f > gpMarioPos->x
	    && 7660.0f < gpMarioPos->z && 8040.0f > gpMarioPos->z) {
		unk3C = unk34;
		unk40 = unk38;
		unk54 = unk5C;
	}

	const JGeometry::TVec3<f32>& marioCube = *gpMarioPos;
	int cube = gpCubeStream->getInCubeNo(marioCube);
	if (cube != -1) {
		TCubeStreamInfo& info
		    = (TCubeStreamInfo&)*gpCubeStream->unk14->begin()[cube];
		if (unk44 < info.unk3C)
			unk44 += unk48;
	} else {
		if (unk44 > 0.0f)
			unk44 -= unk48;
		else
			unk44 = 0.0f;
	}

	if (unk44 > 0.0f) {
		unk3C = unk2C + unk44;
		unk40 = unk30 + unk44;
	}

	// Dead slot so the frame stays at -0x70 (r31 at r1+0x6c).
	char trash[0x28];
	trash[0] = 0;
}

void TMapObjWave::updateTime()
{
	// Retail bits of @2730. 2*pi is a few bits high.
	unk64 += unk24;
	if (unk64 > 6.28318f)
		unk64 -= 6.28318f;

	unk68 += unk28;
	if (unk68 > 6.28318f)
		unk68 -= 6.28318f;

	unk6C += unk60;
	if (unk6C > 1.0f)
		unk6C -= 1.0f;

	unk70 += unk60;
	if (unk70 > 1.0f)
		unk70 -= 1.0f;
}
#pragma dont_inline off

void TMapObjWave::movement() { }

void TMapObjWave::perform(u32 cue, JDrama::TGraphics*)
{
	if (!unk94)
		return;

	if (cue & CUE_MOVE) {
		updateTime();
		u8 map = gpMarDirector->getCurrentMap();
		if (map == 4 || map == 6)
			updateHeightAndAlpha();
	}

	if (cue & CUE_DRAW) {
		initDraw();
		draw();
	}

	// Dead slot so the frame stays at -0x40 (r31 at r1+0x3c).
	char trash[0x18];
	trash[0] = 0;
}

void TMapObjWave::load(JSUMemoryInputStream& stream)
{
	JDrama::TNameRef::load(stream);
	unk10 = 5200.0f;
	unk1C = 200.0f;
	f32 half = 0.5f;
	unk14 = unk10 * half;
	unk18 = 1.0f / unk14;
	unk20 = static_cast<int>(unk10 / unk1C);
	unk94 = JKRFileLoader::getGlbResource("/scene/map/map/wave.bti");
	unk60 = 0.0015f;
	unk74 = 0.0012f;
	unk78 = 0.0015f;
	unk4C = 400.0f;
	unk50 = 150.0f;
	unk24 = 0.02f;
	unk28 = 0.03f;

	switch (gpMarDirector->getCurrentMap()) {
	case 3:
	case 0x1E:
		unk2C = 25.0f;
		unk30 = 20.0f;
		unk34 = 0.0f;
		unk38 = 0.0f;
		unk3C = unk2C;
		unk40 = unk30;
		break;
	case 4:
		unk2C = 40.0f;
		unk30 = 30.0f;
		unk34 = 5.0f;
		unk38 = 0.0f;
		break;
	case 0xD:
		unk2C = 30.0f;
		unk30 = 25.0f;
		unk34 = 5.0f;
		unk38 = 0.0f;
		break;
	case 9:
	case 0x34:
		unk2C = 10.0f;
		unk30 = 15.0f;
		unk34 = 0.0f;
		unk38 = 0.0f;
		break;
	default:
		unk2C = 30.0f;
		unk30 = 25.0f;
		unk34 = 0.0f;
		unk38 = 0.0f;
		break;
	}
	unk3C = unk2C;
	unk40 = unk30;
}

TMapObjWave::TMapObjWave(const char* name)
    : JDrama::TViewObj(name)
{
	unk10 = 0.0f;
	unk14 = 0.0f;
	unk18 = 0.0f;
	unk20 = 0;
	unk24 = 0.0f;
	unk28 = 0.0f;
	unk2C = 0.0f;
	unk30 = 0.0f;
	unk34 = 0.0f;
	unk38 = 0.0f;
	unk3C = 0.0f;
	unk40 = 0.0f;
	unk44 = 0.0f;
	unk48 = 0.1f;
	unk4C = 0.0f;
	unk50 = 0.0f;
	unk54 = 255.0f;
	unk58 = 255.0f;
	unk5C = 0.0f;
	unk60 = 0.0f;
	unk64 = 360.0f * ((f32)rand() * 0.000030517578f);
	unk68 = 360.0f * ((f32)rand() * 0.000030517578f);
	unk6C = (f32)rand() * 0.000030517578f;
	unk70 = (f32)rand() * 0.000030517578f;
	unk74 = 0.0f;
	unk78 = 0.0f;
	unk94 = nullptr;
	unk98 = 0;
	sColor.set(0xC8, 0xC8, 0xFF, 0);
	unk7C = 0xC2;
	unk7E = 0xF2;
	unk80 = 0xBE;
	unk82 = 0;
	unk84 = 0;
	unk86 = 0;
	unk88 = 0;
	unk8A = 0x48;
	unk8C = 0;
	unk8E = 0;
	unk90 = 0;
	unk92 = 0x90;
	gpMapObjWave = this;
}
