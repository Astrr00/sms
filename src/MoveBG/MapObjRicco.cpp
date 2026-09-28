#include <MoveBG/MapObjRicco.hpp>
#include <JSystem/J3D/J3DGraphAnimator/J3DModel.hpp>
#include <Map/MapCollisionManager.hpp>

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

void TCraneCargo::calc()
{
	updateRootMtxTrans();
	calcLeanMtx(getModel()->getAnmMtx(1));
}

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

BOOL TFruitSwitch::receiveMessage(THitActor*, u32 message)
{
	if (message == HIT_MESSAGE_HIP_DROP) {
		startBck("riccoswitch");
		onHitFlag(HIT_FLAG_NO_COLLISION);
		if (mMapCollisionManager->unk8 != nullptr)
			mMapCollisionManager->unk8->remove();
		unk138->fireObj();
		return TRUE;
	}
	return FALSE;
}

void TFruitLauncher::appearFruit() const { }

#pragma dont_inline on
void TFruitLauncher::fireObj()
{
	// Address-of keeps the weak out-of-line copy.
	// -inline deferred would otherwise inline the call away.
	volatile MActor* (TLiveActor::*p)() const = &TLiveActor::getMActor;
	(void)p;
}
#pragma dont_inline off

void TFruitLauncher::loadAfter() { }
