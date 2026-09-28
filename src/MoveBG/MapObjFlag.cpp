#include <MoveBG/MapObjFlag.hpp>
#include <System/MarDirector.hpp>

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

TMapObjFlagManager* gpMapObjFlagManager;

f32 TMapObjFlag::mFlutterSpeed = 4.0f;

void TMapObjFlagSail::updateVertex() { }

void TMapObjFlagLower::updateVertex() { }

void TMapObjFlag::draw() { }

void TMapObjFlag::updateVertex() { }

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
