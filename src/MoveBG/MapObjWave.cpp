#include <MoveBG/MapObjWave.hpp>

#include <System/MarDirector.hpp>
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

f32 TMapObjWave::getHeight(float, float, float) const { return 0.0f; }

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
