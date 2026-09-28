#include <MoveBG/ModelGate.hpp>

static const char* gateMActorNames[] = {
	"05_gate01",
	"05_gate02rico",
	"05_gate03manma",
	"05_gate04monte",
	"05_gate05mare",
};

// rogue includes needed for matching sinit & bss
#include <MSound/MSSetSound.hpp>
#include <MSound/MSoundBGM.hpp>

// -inline deferred: source order is the reverse of mario.MAP emission order.

void TModelGate::loadAfter() { }

void TModelGate::startOpen()
{
	unk70 |= 1;
	unkC4 = 0;
	unk78->setBpk(gateMActorNames[unk71]);
	offHitFlag(HIT_FLAG_NO_COLLISION);
	unk70 |= 2;
}

void TModelGate::screenBlur(JDrama::TGraphics*) { }

BOOL TModelGate::receiveMessage(THitActor*, u32) { return FALSE; }

void TModelGate::perform(u32, JDrama::TGraphics*) { }
