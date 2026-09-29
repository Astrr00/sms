#include <MoveBG/ModelGate.hpp>
#include <System/Particles.hpp>
#include <stdlib.h>

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

BOOL TModelGate::receiveMessage(THitActor* sender, u32 message)
{
	if (sender->mActorType == 0x80000001 && message == HIT_MESSAGE_ATTACK) {
		unkC8 = 0;
		unkC4 = 2;
		return TRUE;
	}

	if (sender->mActorType == 0x1000001) {
		JGeometry::TVec3<f32> local;
		MTXMultVec(unk7C, &sender->mPosition, &local);
		Mtx gateMtx;
		char trash[0xC];
		MTXCopy(unk78->getModel()->getAnmMtx(unk72), gateMtx);

		if (local.x * local.x + local.y * local.y < 40000.0f && -100.0f < local.z
		    && local.z < unkFC) {
			if (unk70 & 2) {
				unkD0 += unkD4;
				if (unkD0 > 1.0f) {
					unkCA = unkC8;
					unkD0 = 1.0f;
					unk70 &= ~2;
				}
			}

			if ((f32)rand() * 0.000030517578f < unkF8) {
				gpMarioParticleManager->emitWithRotate(
				    0x1DD, &sender->mPosition, 0, unk74, 0, 2, nullptr);
				gpMarioParticleManager->emitWithRotate(
				    0x1DE, &sender->mPosition, 0, unk74, 0, 2, nullptr);
			}
			return TRUE;
		}
	}

	return FALSE;
}

void TModelGate::perform(u32, JDrama::TGraphics*) { }
