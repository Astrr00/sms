#include <MoveBG/MapObjFlag.hpp>
#include <System/MarDirector.hpp>
#include <MarioUtil/MathUtil.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

TMapObjFlagManager* gpMapObjFlagManager;

f32 TMapObjFlag::mFlutterSpeed = 4.0f;

void TMapObjFlagSail::updateVertex() { }

void TMapObjFlagLower::updateVertex() { }

void TMapObjFlag::draw() { }

void TMapObjFlag::updateVertex()
{
	f32 lo = -180.0f;
	f32 hi = 180.0f;
	for (s32 i = 0; i < unk74; i += unkBC) {
		f32 outer = (f32)i * unk80;
		for (s32 j = 0; j < unk70; j += unkBC) {
			f32 ratio = (f32)j / (f32)unk70;
			f32 angle = unk88 + ((f32)(-j) * unk7C + outer);
			angle = MsWrap(angle, lo, hi);
			unk78[i][j * 3] = unk84 * ratio * MsSin(angle);
		}
	}
}

void TMapObjFlag::update() { }

#pragma dont_inline on
void TMapObjFlag::init(const char*) { }
#pragma dont_inline off

void TMapObjFlag::load(JSUMemoryInputStream& stream)
{
	JDrama::TActor::load(stream);
	char name[0x40];
	stream.readString(name, 0x40);
	init(name);
}

TMapObjFlag::TMapObjFlag(const char* name)
    : THitActor(name)
{
}

void TMapObjFlagManager::initDraw() { }

void TMapObjFlagManager::perform(u32, JDrama::TGraphics*) { }

void TMapObjFlagManager::loadFlag(TMapObjFlagInfo*, TMapObjFlag*, const char*)
{
}

void TMapObjFlagManager::registerObj(TMapObjFlag*, const char*) { }

void TMapObjFlagManager::load(JSUMemoryInputStream& stream)
{
	JDrama::TNameRef::load(stream);
	char name[8];
	stream.readString(name, 8);
	switch (gpMarDirector->getCurrentMap()) {
	case 0:
		TMapObjFlag::mFlutterSpeed = 16.0f;
		break;
	case 2:
		TMapObjFlag::mFlutterSpeed = 16.0f;
		break;
	case 4:
		TMapObjFlag::mFlutterSpeed = 12.0f;
		break;
	default:
		TMapObjFlag::mFlutterSpeed = 8.0f;
		break;
	}

	// Dead slot so the name buffer stays at r1+0x20 (frame -0x30).
	char trash[8];
	trash[0] = 0;
}

TMapObjFlagManager::TMapObjFlagManager(const char* name)
    : JDrama::TViewObj(name)
{
	gpMapObjFlagManager = this;
}
