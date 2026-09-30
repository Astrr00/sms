#ifndef MOVE_BG_MAP_OBJ_FENCE_HPP
#define MOVE_BG_MAP_OBJ_FENCE_HPP

#include <MoveBG/MapObjBase.hpp>

class TGraphTracer;

// TODO: mark virtual methods as such

class TFence : public TMapObjBase {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);
	void initMapCollisionData();
	void initMapObj();
	TFence(const char* name = "フェンス")
	    : TMapObjBase(name)
	    , unk138(0)
	{
	}

public:
	/* 0x138 */ u8 unk138;
};

class TRevolvingFenceOuter : public TFence {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);
	void initMapCollisionData();
	TRevolvingFenceOuter(const char* name = "フェンス外側")
	    : TFence(name)
	{
	}

public:
	/* 0x13C */ TMapObjBase* unk13C;
};

class TRevolvingFenceInner : public TFence {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);
	void calcCurrentMtx();
	void controlWall();
	void controlGroundRoof();
	void setGroundCollision();
	void control();
	void initMapCollisionData();
	void initMapObj();

	static f32 mSpeed;

	TRevolvingFenceInner(const char* name = "フェンス内側")
	    : TFence(name)
	    , unk13C(0.0f)
	    , unk140(1)
	{
	}

public:
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ u8 unk140;
};

class TFenceWater : public TFence {
public:
	void draw() const;
	BOOL receiveMessage(THitActor* sender, u32 message);
	virtual void changeStatusToGo();
	virtual void changeStatusToWait();
	void controlRotation();
	void control();
	void initMapCollisionData();
	void initMapObj();
	TFenceWater(const char* name = "水回転フェンス（垂直）")
	    : TFence(name)
	{
	}

	static f32 mWaterAccel;
	static f32 mBackSpeed;
	static int mTurnedWaitTime;

public:
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
};

class TFenceWaterH : public TFenceWater {
public:
	void control();
	void changeStatusToGo();
	void changeStatusToWait();
	TFenceWaterH(const char* name = "水回転フェンス（水平）")
	    : TFenceWater(name)
	{
	}
};

class TRailFence : public TFence {
public:
	BOOL receiveMessage(THitActor* sender, u32 message);
	void falling();
	void goOnRail();
	void control();
	void initMapCollisionData();
	void load(JSUMemoryInputStream&);
	TRailFence(const char* name = "レールフェンス");

	static f32 mFallHeight;
	static int mWaitTime;

public:
	/* 0x13C */ TGraphTracer* unk13C;
	/* 0x140 */ f32 unk140;
};

#endif
