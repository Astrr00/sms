#ifndef MOVE_BG_MAP_OBJ_BIANCO_HPP
#define MOVE_BG_MAP_OBJ_BIANCO_HPP

#include <MoveBG/MapObjFloat.hpp>
#include <MoveBG/MapObjTurn.hpp>

// TODO: mark remaining virtual methods as such

struct TBGWallCheckRecord;

class TWoodLog : public TMapObjFloatOnSea {
public:
	virtual void control();
};

class TBellWatermill : public TMapObjTurn {
public:
	virtual void loadAfter();
	virtual void control();
	virtual u32 touchWater(THitActor*);
	TBellWatermill(const char* name = "ベル水車");

public:
	/* 0x16C */ f32 unk16C;
	/* 0x170 */ f32 unk170;
	/* 0x174 */ f32 unk174;
	/* 0x178 */ f32 unk178;
	/* 0x17C */ f32 unk17C;
	/* 0x180 */ f32 unk180;
	/* 0x184 */ f32 unk184;
	/* 0x188 */ f32 unk188;
	/* 0x18C */ f32 unk18C;
	/* 0x190 */ u8 unk190;
	/* 0x191 */ u8 unk191[0xF];
	/* 0x1A0 */ u8 unk1A0;
	/* 0x1A1 */ u8 unk1A1[3];
	/* 0x1A4 */ u32 unk1A4;
};

class TBiancoBell : public TMapObjBase {
public:
	virtual void initMapObj();
	virtual void touchPlayer(THitActor*);
	virtual u32 touchWater(THitActor*);
	void ringSingle();
	void ring();
	void stopToRing();
	TBiancoBell(const char* name = "ベル水車");

public:
	/* 0x138 */ u16 unk138;
	/* 0x13A */ u8 unk13A;
};

class TLampSeesaw : public TMapObjBase {
public:
	virtual void load(JSUMemoryInputStream&);
	virtual void touchPlayer(THitActor*);
	virtual void pushDown(f32) { }
	TLampSeesaw(const char* name = "ランプシーソー（従）");

public:
	/* 0x138 */ TLampSeesaw* unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
};

class TLampSeesawMain : public TLampSeesaw {
public:
	virtual void loadAfter();
	virtual void control();
	virtual void touchPlayer(THitActor*);
	void move();
	virtual void pushDown(f32);
	TLampSeesawMain(const char* name = "ランプシーソー");

public:
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
	/* 0x14C */ f32 unk14C;
	/* 0x150 */ f32 unk150;
};

class TLeafBoat : public TMapObjBase {
public:
	virtual void initMapObj();
	virtual void calc();
	virtual void control();
	virtual void bind();
	void touchWall(JGeometry::TVec3<f32>*, TBGWallCheckRecord*);
	virtual void touchActor(THitActor*);
	TLeafBoat(const char* name = "リーフボート");

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
	/* 0x14C */ f32 unk14C;
	/* 0x150 */ f32 unk150;
	/* 0x154 */ f32 unk154;
	/* 0x158 */ f32 unk158;
	/* 0x15C */ f32 unk15C;
	/* 0x160 */ u32 unk160;
	/* 0x164 */ JGeometry::TVec3<f32> unk164;
};

class TLeafBoatRotten : public TLeafBoat {
public:
	virtual void load(JSUMemoryInputStream&);
	virtual void perform(u32 cue, JDrama::TGraphics* graphics);
	virtual void control();
	TLeafBoatRotten(const char* name = "腐ったリーフボート");

public:
	/* 0x170 */ u32 unk170;
	/* 0x174 */ u32 unk174;
	/* 0x178 */ u16 unk178;
	/* 0x17A */ u16 unk17A;
	/* 0x17C */ u16 unk17C;
	/* 0x17E */ u16 unk17E;
};

class TBiancoMiniWindmill : public THideObjBase {
public:
	virtual void initMapObj();
	virtual void control();
	virtual void calc();
	virtual u32 touchWater(THitActor*);
	TBiancoMiniWindmill(const char* name = "風車（ビアンコ小）");

	static f32 mFriction;

public:
	/* 0x150 */ f32 unk150;
	/* 0x154 */ f32 unk154;
	/* 0x158 */ f32 unk158;
	/* 0x15C */ u32 unk15C;
	/* 0x160 */ u32 unk160;
};

class TBiancoWatermillVertical : public TMapObjBase {
public:
	virtual void load(JSUMemoryInputStream&);
	virtual void loadAfter();
	virtual void control();
	virtual void setGroundCollision();
	virtual u32 touchWater(THitActor*);
	TBiancoWatermillVertical(const char* name = "水車（ビアンコ垂直）");

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
};

class TBiancoWatermill : public TMapObjBase {
public:
	virtual void initMapObj();
	virtual void control();
	virtual u32 touchWater(THitActor*);
	void turn(const JGeometry::TVec3<f32>&, const TBGCheckData*, f32);
	void turnByEnemy(THitActor*, const TBGCheckData*);
	TBiancoWatermill(const char* name = "水車（ビアンコ大）");

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ u32 unk13C;
};

class TTrembleModelEffect;

class TMapObjRootPakkun : public TMapObjBase {
public:
	virtual void initMapObj();
	virtual void drawObject(JDrama::TGraphics*);

public:
	/* 0x138 */ TTrembleModelEffect* unk138;
};

class TBigWindmill : public TMapObjBase {
public:
	virtual void load(JSUMemoryInputStream&);
	virtual void control();
};

#endif
