#include <MoveBG/MapObjFlag.hpp>
#include <System/DummyStrings.hpp>
#include <System/MarDirector.hpp>
#include <MarioUtil/MathUtil.hpp>
#include <MarioUtil/RandomUtil.hpp>
#include <JSystem/JKernel/JKRFileLoader.hpp>
#include <JSystem/JKernel/JKRHeap.hpp>
#include <PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/string.h>
#include <dolphin/gx.h>
#include <stdio.h>

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
void TMapObjFlag::init(const char* name)
{
	char trash[8];
	(void)trash;

	unk68 = 100.0f * mScaling.z;
	unk6C = 100.0f * mScaling.y;
	unk7C /= mScaling.z;
	unk80 /= mScaling.y;
	unk84 *= mScaling.z;

	unk70 = (s32)(unk68 / 50.0f);
	unk74 = (s32)(unk6C / 100.0f);
	if (unk70 < 2)
		unk70 = 3;
	if (unk74 < 2)
		unk74 = 3;

	MsMtxSetXYZRPH(unk8C, mPosition.x, mPosition.y, mPosition.z, mRotation.x,
	               mRotation.y, mRotation.z);

	f32 stepZ = unk68 / (f32)unk70;
	f32 stepY = unk6C / (f32)unk74;
	JKRHeap::getCurrentHeap()->getTotalFreeSize();

	unk78 = (f32**)new JGeometry::TVec3<f32>*[unk74];
	for (s32 i = 0; i < unk74; ++i) {
		unk78[i] = (f32*)new JGeometry::TVec3<f32>[unk70];
		f32 y    = (f32)i * stepY;
		for (s32 j = 0; j < unk70; ++j) {
			JGeometry::TVec3<f32>* vtx
			    = &((JGeometry::TVec3<f32>*)unk78[i])[j];
			vtx->x = 0.0f;
			vtx->y = y;
			vtx->z = (f32)j * stepZ;
		}
	}

	static u32 total_use_size = 0;
	JKRHeap::getCurrentHeap()->getTotalFreeSize();
	gpMapObjFlagManager->registerObj(this, name);
	initHitActor(0x4000000D, 1, 0, 0.0f, 0.0f, 0.0f, 0.0f);
}
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
    , unk68(0.0f)
    , unk6C(0.0f)
    , unk70(0)
    , unk74(0)
    , unk78(nullptr)
    , unk7C(125.0f)
    , unk80(130.0f)
    , unk84(20.0f)
    , unk88(MsRandF() * 360.0f)
    , unkBC(1)
{
	unk8C.identity();
	// Dead slot: retail frame is -0x48, the live locals only fill -0x40.
	char trash[8];
}

void TMapObjFlagManager::initDraw()
{
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GXClearVtxDesc();
	GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GXSetCurrentMtx(GX_PNMTX0);
	GXSetNumChans(0);
	GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_NONE,
	              GX_AF_NONE);
	GXSetChanMatColor(GX_COLOR0A0, (GXColor) { 0xff, 0xff, 0xff, 0xff });
	GXSetNumTexGens(1);
	GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, 0x3c, GX_FALSE,
	                  0x7d);
	GXSetNumTevStages(1);
	GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
	GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO,
	                GX_CC_ZERO);
	GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO,
	                GX_CA_ZERO);
	GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE,
	                GX_TEVPREV);
	GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
	GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_GREATER, 0);
	GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
	GXSetZCompLoc(GX_FALSE);
	GXSetCullMode(GX_CULL_NONE);
}

void TMapObjFlagManager::perform(u32, JDrama::TGraphics*) { }

void TMapObjFlagManager::loadFlag(TMapObjFlagInfo*, TMapObjFlag*, const char*)
{
}

#pragma dont_inline on
void TMapObjFlagManager::registerObj(TMapObjFlag* flag, const char* name)
{
	if (strcmp(name, "flagSun") == 0) {
		if (mInfos[0].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[0].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[0].unk4[mInfos[0].unk0] = flag;
		mInfos[0].unk0 += 1;
	} else if (strcmp(name, "flagWhite") == 0) {
		if (mInfos[1].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[1].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[1].unk4[mInfos[1].unk0] = flag;
		mInfos[1].unk0 += 1;
	} else if (strcmp(name, "flagRedsun") == 0) {
		if (mInfos[2].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[2].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[2].unk4[mInfos[2].unk0] = flag;
		mInfos[2].unk0 += 1;
	} else if (strcmp(name, "flagMonte") == 0) {
		if (mInfos[3].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[3].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[3].unk4[mInfos[3].unk0] = flag;
		mInfos[3].unk0 += 1;
	} else if (strcmp(name, "flagBird") == 0) {
		if (mInfos[4].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[4].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[4].unk4[mInfos[4].unk0] = flag;
		mInfos[4].unk0 += 1;
	} else if (strcmp(name, "flagHigekuri") == 0) {
		if (mInfos[5].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[5].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[5].unk4[mInfos[5].unk0] = flag;
		mInfos[5].unk0 += 1;
	} else if (strcmp(name, "flagBenvenuto") == 0) {
		if (mInfos[6].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[6].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[6].unk4[mInfos[6].unk0] = flag;
		mInfos[6].unk0 += 1;
	} else if (strcmp(name, "flagDolpicDolphin") == 0) {
		if (mInfos[7].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[7].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[7].unk4[mInfos[7].unk0] = flag;
		mInfos[7].unk0 += 1;
	} else if (strcmp(name, "flagDolSun") == 0) {
		if (mInfos[8].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[8].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[8].unk4[mInfos[8].unk0] = flag;
		mInfos[8].unk0 += 1;
	} else if (strcmp(name, "flagDolSunWelcome") == 0) {
		if (mInfos[9].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[9].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[9].unk4[mInfos[9].unk0] = flag;
		mInfos[9].unk0 += 1;
	} else if (strcmp(name, "flagBianco") == 0) {
		if (mInfos[10].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[10].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[10].unk4[mInfos[10].unk0] = flag;
		mInfos[10].unk0 += 1;
	} else if (strcmp(name, "flagRiccoBuoy") == 0) {
		if (mInfos[11].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[11].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[11].unk4[mInfos[11].unk0] = flag;
		mInfos[11].unk0 += 1;
	} else if (strcmp(name, "flagSailMonte") == 0) {
		if (mInfos[12].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[12].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[12].unk4[mInfos[12].unk0] = flag;
		mInfos[12].unk0 += 1;
	} else if (strcmp(name, "MammaYacht00") == 0) {
		if (mInfos[13].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[13].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[13].unk4[mInfos[13].unk0] = flag;
		mInfos[13].unk0 += 1;
	} else if (strcmp(name, "flagMare") == 0) {
		if (mInfos[14].unk54 == nullptr) {
			char buf[0x40];
			snprintf(buf, 0x40, "/scene/mapObj/%s.bti", name);
			mInfos[14].unk54 = JKRFileLoader::getGlbResource(buf);
		}
		mInfos[14].unk4[mInfos[14].unk0] = flag;
		mInfos[14].unk0 += 1;
	}
}
#pragma dont_inline off

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
