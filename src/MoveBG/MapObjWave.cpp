#include <MoveBG/MapObjWave.hpp>

#include <System/MarDirector.hpp>
#include <Map/Map.hpp>
#include <Map/MapData.hpp>
#include <JSystem/JUtility/JUTColor.hpp>
#include <math.h>

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

// dont_inline: stub bodies. Without the pragma, MWCC inlines them into perform.
#pragma dont_inline on
void TMapObjWave::draw() { }

void TMapObjWave::updateHeightAndAlpha() { }

void TMapObjWave::updateTime() { }
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
}
