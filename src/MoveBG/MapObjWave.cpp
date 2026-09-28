#include <MoveBG/MapObjWave.hpp>

#include <JSystem/JUtility/JUTColor.hpp>

TMapObjWave* gpMapObjWave;

static JUtility::TColor sColor;

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TMapObjWave::initDraw() { }

void TMapObjWave::getMoveTexPos1(float) const { }

void TMapObjWave::getMoveTexPos0(float) const { }

void TMapObjWave::getStaticTexPos1(float) const { }

void TMapObjWave::getStaticTexPos0(float) const { }

f32 TMapObjWave::getWaveHeight(float, float) const { return 0.0f; }

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

void TMapObjWave::draw() { }

void TMapObjWave::updateHeightAndAlpha() { }

void TMapObjWave::updateTime() { }

void TMapObjWave::movement() { }

void TMapObjWave::perform(u32, JDrama::TGraphics*) { }

void TMapObjWave::load(JSUMemoryInputStream&) { }

TMapObjWave::TMapObjWave(const char* name)
    : JDrama::TViewObj(name)
{
}
