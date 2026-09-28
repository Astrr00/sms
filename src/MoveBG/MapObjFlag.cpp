#include <MoveBG/MapObjFlag.hpp>

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

void TMapObjFlagManager::load(JSUMemoryInputStream&) { }

TMapObjFlagManager::TMapObjFlagManager(const char* name)
    : JDrama::TViewObj(name)
{
	gpMapObjFlagManager = this;
}
