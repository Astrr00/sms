#ifndef MOVE_BG_MAP_OBJ_MARE_HPP
#define MOVE_BG_MAP_OBJ_MARE_HPP

#include <MoveBG/MapObjBase.hpp>

struct TBGWallCheckRecord;

class TCogwheel;

class TCogwheelScale : public TMapObjBase {
public:
	virtual u32 touchWater(THitActor*);
	virtual BOOL receiveMessage(THitActor* sender, u32 message);
	virtual void touchPlayer(THitActor*);
	virtual void control();
	TCogwheelScale(const char*);

	static f32 mWaterLeakSpeed;

	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
	/* 0x14C */ f32 unk14C;
	/* 0x150 */ f32 unk150;
	/* 0x154 */ u8 unk154;
	/* 0x158 */ TCogwheel* unk158;
};

class TCogwheel : public TMapObjBase {
public:
	void initDraw() const;
	virtual void draw() const;
	void rebound();
	virtual void calc();
	virtual void control();
	virtual void initMapObj();
	TCogwheel(const char* name = "天秤");

	static f32 mRopeWidthX;
	static f32 mRopeWidthZ;
	static f32 mTexPosRate;
	static f32 mMinSpeed;

	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
	/* 0x14C */ f32 unk14C;
	/* 0x150 */ u32 unk150;
	/* 0x154 */ JGeometry::TVec3<f32> unk154;
	/* 0x160 */ f32 unk160;
	/* 0x164 */ u32 unk164;
	/* 0x168 */ JGeometry::TVec3<f32> unk168;
	/* 0x174 */ f32 unk174;
};

class TMapObjElasticCode : public TMapObjBase {
public:
	virtual void draw() const;
	virtual void control();
	virtual void initMapObj();
	TMapObjElasticCode(const char* name = "ゴムひも");

	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
};

class TMapObjGrowTree : public TMapObjBase {
public:
	void getGrowHeightFromRate(float) const;
	void updateHeight();
	virtual u32 touchWater(THitActor*);
	virtual void control();
	virtual void loadAfter();
	virtual void initMapObj();
	TMapObjGrowTree(const char* name = "もやしの木");

	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ u32 unk144;
	/* 0x148 */ f32 unk148;
};

class TWireBell : public TMapObjBase {
public:
	void initDraw() const;
	virtual void draw() const;
	virtual void control();
	virtual void loadAfter();
	TWireBell(const char* name = "ワイヤー鈴（紫）");

	/* 0x138 */ s32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
	/* 0x14C */ JGeometry::TVec3<f32> unk14C;
};

class TMapObjPuncher : public TMapObjBase {
public:
	virtual void touchPlayer(THitActor*);
	virtual void control();
	virtual void load(JSUMemoryInputStream&);
	TMapObjPuncher(const char* name = "パンチャー");
};

class TMuddyBoat : public TMapObjBase {
public:
	void moveByWater();
	virtual void calcRootMatrix();
	virtual void kill();
	void touchWall(JGeometry::TVec3<float>*, const TBGWallCheckRecord&);
	void bindToWall(const JGeometry::TVec3<float>&, float,
	                JGeometry::TVec3<float>*);
	virtual void bind();
	virtual void control();
	virtual void calc();
	virtual u32 getSDLModelFlag() const;
	virtual void initMapObj();
	TMuddyBoat(const char* name = "どろの船");

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
	/* 0x160 */ f32 unk160;
	/* 0x164 */ f32 unk164;
	/* 0x168 */ u32 unk168;
	/* 0x16C */ u32 unk16C;
	/* 0x170 */ JGeometry::TVec3<f32> unk170;
	/* 0x17C */ JGeometry::TVec3<f32> unk17C;
};

class TMareFall : public TMapObjBase {
public:
	virtual void calc();
	virtual void load(JSUMemoryInputStream&);
	TMareFall(const char* name = "マーレ滝");
};

class TMareCork : public TMapObjBase {
public:
	virtual void loadAfter();
	virtual void moveObject();
	virtual void calcRootMatrix();
	virtual MtxPtr getTakingMtx();
	virtual void drawObject(JDrama::TGraphics*);
	TMareCork(const char* name = "マーレコルク");
};

class TMareEventPoint : public THitActor {
public:
	virtual BOOL receiveMessage(THitActor* sender, u32 message);
	virtual void load(JSUMemoryInputStream&);
	TMareEventPoint(const char* name = "イベントポイント");
};

#endif
