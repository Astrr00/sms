#include <MoveBG/MapObjRicco.hpp>

static JGeometry::TVec3<f32> submarineCranePos_forSound(1956.0f, 1000.0f,
                                                        6425.0f);
static JGeometry::TVec3<f32> submarineSetWtPos_forSound(1956.0f, -100.0f,
                                                        6425.0f);

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.
// Unmatched bodies are stubs so every map symbol in this TU is defined.

void TCraneRotY::calc() { setRootMtxRotY(); }

void TCraneRotY::control() { }

void TCraneRotY::load(JSUMemoryInputStream&) { }

void TCraneUpDown::control() { }

void TCraneUpDown::initMapObj() { }

void TCraneCargo::control()
{
	unk158.z = 0.0f;
	unk158.y = 0.0f;
	unk158.x = 0.0f;
	TMapObjBase::control();
}

void TCraneCargo::calc() { }

u32 TRiccoWatermill::touchWater(THitActor*) { return 0; }

void TRiccoWatermill::control() { }

void TRiccoWatermill::calc() { setRootMtxRotZ(); }

void TRiccoWatermill::loadAfter() { }

TRiccoWatermill::TRiccoWatermill(const char* name)
    : TMapObjBase(name)
{
}

void TSurfGesoObj::initMapObj() { }

void TFruitSwitch::pullUp() { }

void TFruitSwitch::pushDown() { }

BOOL TFruitSwitch::receiveMessage(THitActor*, u32) { return FALSE; }

void TFruitLauncher::appearFruit() const { }

void TFruitLauncher::fireObj()
{
	// Address-of keeps the weak out-of-line copy.
	// -inline deferred would otherwise inline the call away.
	volatile MActor* (TLiveActor::*p)() const = &TLiveActor::getMActor;
	(void)p;
}

void TFruitLauncher::loadAfter() { }
