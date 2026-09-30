#include <MoveBG/MapObjWave.hpp>

#include <System/MarDirector.hpp>
#include <Map/Map.hpp>
#include <Map/MapData.hpp>
#include <JSystem/JUtility/JUTColor.hpp>
#include <Player/MarioAccess.hpp>
#include <Camera/CubeManagerBase.hpp>
#include <math.h>
#include <stdlib.h>

TMapObjWave* gpMapObjWave;

static JUtility::TColor sColor;

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

// dont_inline: stub body. Without the pragma, MWCC inlines it into perform.
#pragma dont_inline on
void TMapObjWave::initDraw() { }
#pragma dont_inline off

void TMapObjWave::getMoveTexPos1(float) const { }

void TMapObjWave::getMoveTexPos0(float) const { }

void TMapObjWave::getStaticTexPos1(float) const { }

void TMapObjWave::getStaticTexPos0(float) const { }

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

void TMapObjWave::getAlpha(float, float) const { }

// dont_inline: draw is still a stub. updateHeightAndAlpha and updateTime
// must stay out of line so perform keeps its bls.
#pragma dont_inline on
void TMapObjWave::draw() { }

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

void TMapObjWave::load(JSUMemoryInputStream&) { }

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
