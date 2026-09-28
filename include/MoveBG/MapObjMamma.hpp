#ifndef MOVE_BG_MAP_OBJ_MAMMA_HPP
#define MOVE_BG_MAP_OBJ_MAMMA_HPP

#include <MoveBG/MapObjBase.hpp>
#include <MoveBG/MapObjEx.hpp>

class TMapCollisionMove;
class TMapObjFlag;
class TMapObjGeneral;

class TSandLeaf : public TMapObjBase {
public:
	virtual u32 touchWater(THitActor*);
	virtual void control();
	TSandLeaf(const char* name = "すなやまの芽")
	    : TMapObjBase(name)
	    , unk138(0)
	{
	}

public:
	/* 0x138 */ TMapObjGeneral* unk138;
};

class TSandBase : public TMapObjBase {
public:
	// Retail slot is a null pointer. This compiler emits that for a pure virtual.
	virtual void grow() = 0;
	virtual bool withering();
	void isDown() const;
	TSandBase(const char*);

	static u32 mWitherTime;
	static f32 mScaleMin;

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ u32 unk140;
	/* 0x144 */ TMapObjBase* unk144;
};

class TSandLeafBase : public TSandBase {
public:
	virtual void grow();
	virtual void control();
	virtual void initMapObj();
	TSandLeafBase(const char* name = "すなやまの芽の土台");
};

class TSandBomb : public TSandLeaf {
public:
	virtual void makeObjAppeared();
	virtual u32 touchWater(THitActor*);
	virtual u32 getSDLModelFlag() const;
	virtual void initMapObj();

	TSandBomb()
	    : TSandLeaf("すなやま爆弾")
	    , unk13C(0)
	    , unk140(0)
	{
	}

public:
	/* 0x13C */ u32 unk13C;
	/* 0x140 */ u8 unk140;
};

class TSandBombBase : public TSandBase {
public:
	virtual void loadAfter();
	virtual void initMapObj();
	virtual void control();
	virtual void grow();
	virtual void waitBeforeExplode();
	virtual void explode();
	virtual void exploding();
	virtual void expanded();
	virtual void withered();
	virtual TMapObjBase* findTriggerActor();
	TSandBombBase(const char* name = "すなやま爆弾の土台");

	static f32 mFiringFrameSpeed;
	static f32 mFiringFrameDownSpeed;
	static f32 mExplodeFrameSpeed;
	static f32 mMarioJumpRate;
	static u32 mExlodingRumbleTime;

public:
	/* 0x148 */ u32 unk148;
	/* 0x14C */ f32 unk14C;
	/* 0x150 */ f32 unk150;
	/* 0x154 */ f32 unk154;
};

class TSandCastle : public TSandBombBase {
public:
	virtual void loadAfter();
	virtual void initMapObj();
	virtual void calcRootMatrix();
	virtual bool withering();
	virtual void waitBeforeExplode();
	virtual void explode();
	virtual void expanded();
	virtual TMapObjBase* findTriggerActor();
	TSandCastle(const char* name = "砂の城");

	static f32 mCollisionRate;

public:
	/* 0x158 */ TMapObjBase* unk158;
	/* 0x15C */ u8 unk15C;
};

class TLeanMirror : public TMapObjBase {
public:
	virtual BOOL receiveMessage(THitActor* sender, u32 message);
	virtual void control();
	virtual void loadAfter();
	virtual u32 getSDLModelFlag() const;
	virtual void initMapObj();
	virtual void load(JSUMemoryInputStream&);
	virtual void draw() const;
	virtual void touchPlayer(THitActor*);
	virtual void touchEnemy(THitActor*);

	void enemyIsOn() const;
	void updateSpeedVec(const JGeometry::TVec3<f32>&, f32);
	void calcCurrentMtx(MtxPtr);
	void release();
	void controlGoTarget();
	void controlShake();
	TLeanMirror(const char* name = "ぐらぐら鏡");

	static u32 mGoTargetTime;
	static u32 mDemoWaitTime;
	static u32 mDemoLightTime;

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ JGeometry::TVec3<f32> unk140;
	/* 0x14C */ JGeometry::TVec3<f32> unk14C;
	/* 0x158 */ f32 unk158;
	/* 0x15C */ f32 unk15C;
	/* 0x160 */ f32 unk160;
	/* 0x164 */ f32 unk164;
	/* 0x168 */ f32 unk168;
	/* 0x16C */ f32 unk16C;
	/* 0x170 */ f32 unk170;
	/* 0x174 */ f32 unk174;
	/* 0x178 */ f32 unk178;
	/* 0x17C */ u32 unk17C;
	/* 0x180 */ JGeometry::TVec3<f32> unk180;
	/* 0x18C */ JGeometry::TVec3<f32> unk18C;
	/* 0x198 */ f32 unk198;
	/* 0x19C */ u32 unk19C;
	/* 0x1A0 */ JGeometry::TVec3<f32> unk1A0;
	/* 0x1AC */ u8 unk1AC;
	/* 0x1AE */ u16 unk1AE;
};

class TShiningStone : public THitActor {
public:
	virtual void perform(u32 cue, JDrama::TGraphics* graphics);
	virtual void load(JSUMemoryInputStream&);
	void endDemo();
	void putOnLight(TLiveActor*);
	TShiningStone(const char* name = "太陽石");

public:
	/* 0x68 */ void* unk68;
	/* 0x6C */ u32 unk6C;
	/* 0x70 */ u8 unk70;
	/* 0x71 */ u8 unk71;
	/* 0x72 */ u8 unk72;
	/* 0x73 */ u8 unk73;
	/* 0x74 */ u32 unk74;
	/* 0x78 */ u32 unk78;
	/* 0x7C */ f32 unk7C;
};

class TMammaBlockRotate : public TMapObjBase {
public:
	// fcmpo+ble against mRotEnd. Parked (fold family).
	virtual u32 touchWater(THitActor*);
	virtual void control();
	virtual void initMapObj();
	virtual void load(JSUMemoryInputStream&);
	TMammaBlockRotate(const char* name = "太陽の塔ブロック");

	static f32 mRotSpeed;
	static f32 mRotReturnSpeed;
	static f32 mRotEnd;
	static f32 mMapGoSpeed;
	static f32 mMapBackSpeed;
	static u32 mWaitTime;

public:
	/* 0x138 */ u32 unk138;
	/* 0x13C */ u32 unk13C;
	/* 0x140 */ u32 unk140;
	/* 0x144 */ TMapCollisionMove* unk144;
	/* 0x148 */ TMapCollisionMove* unk148;
};

class TMammaYacht : public TMapObjBase {
public:
	virtual void control();
	virtual void initMapObj();
	TMammaYacht(const char* name = "砂の城")
	    : TMapObjBase(name)
	{
	}

public:
	/* 0x138 */ TMapObjFlag* unk138;
};

class TSandBird : public TJointCoin {
public:
	virtual void control();
	virtual void initMapObj();
	virtual TMapObjBase* makeObjFromJointName(const char*, unsigned short);
	virtual bool nameIsObj(const char*);

	TSandBird(const char* name = "おおすな鳥");

public:
	/* 0x148 */ u32 unk148;
	/* 0x14C */ u32 unk14C;
	/* 0x150 */ u8 unk150;
	/* 0x151 */ u8 unk151;
};

class TGoalWatermelon : public TMapObjBase {
public:
	virtual void touchActor(THitActor*);
	virtual void control();
	virtual void loadAfter();
	virtual void load(JSUMemoryInputStream&);
	TGoalWatermelon(const char* name = "スイカゴール");

public:
	/* 0x138 */ TMapObjBase* unk138;
	/* 0x13C */ TMapObjBase* unk13C;
	/* 0x140 */ JGeometry::TVec3<f32> unk140;
};

class TMammaMirrorMapOperator : public JDrama::TViewObj {
public:
	void show(int);
	void hide(int);
	virtual void perform(u32 cue, JDrama::TGraphics* graphics);
	virtual void loadAfter();
	TMammaMirrorMapOperator(const char* name = "鏡内地形操作");
};

class TSandEgg : public TMapObjBase {
public:
	virtual u32 getSDLModelFlag() const;
	TSandEgg(const char* name = "すなのたまご")
	    : TMapObjBase(name)
	{
	}
};

#endif
