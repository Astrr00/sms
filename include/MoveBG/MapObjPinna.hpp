#ifndef MOVE_BG_MAP_OBJ_PINNA_HPP
#define MOVE_BG_MAP_OBJ_PINNA_HPP

#include <MoveBG/MapObjBase.hpp>
#include <MoveBG/MapObjGeneral.hpp>
#include <MoveBG/MapObjTown.hpp>

// TODO: mark virtual methods as such

class TCoin;

class TFerrisWheel : public TMapObjBase {
public:
	void becomeCalmlyCallback(u32, u32);
	void control();
	void initMapObj();
	TFerrisWheel(const char* name = "観覧車");
};

class THorizontalViking : public TMapObjBase {
public:
	void updateTrans();
	void moveNormal();
	void control();
	virtual void reset();
	void initMapObj();
	THorizontalViking(const char*);

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ f32 unk148;
};

class TViking : public THorizontalViking {
public:
	void roll();
	void control();
	virtual void reset();
	void loadAfter();
	void initMapObj();
	TViking(const char* name = "バイキング");
};

class TPinnaShell : public THitActor {
public:
	void opened();
	BOOL receiveMessage(THitActor* sender, u32 message);
	void control();
	TPinnaShell(const char*);
	TPinnaShell()
	    : THitActor("シェル")
	    , unk68(0)
	    , unk6C(0.0f)
	    , unk70(0.0f)
	    , unk74(0)
	    , unk78(0)
	    , unk7C(0)
	    , unk80(0)
	    , unk84(0)
	    , unk88(0)
	    , unk8C(0)
	{
		initHitActor(0x4000013A, 1, 0x80000000, 250.0f, 400.0f, 250.0f,
		             200.0f);
	}

public:
	/* 0x68 */ u32 unk68;
	/* 0x6C */ f32 unk6C;
	/* 0x70 */ f32 unk70;
	/* 0x74 */ u32 unk74;
	/* 0x78 */ u32 unk78;
	/* 0x7C */ u32 unk7C;
	/* 0x80 */ u32 unk80;
	/* 0x84 */ u32 unk84;
	/* 0x88 */ u32 unk88;
	/* 0x8C */ u32 unk8C;
};

class TShellCup : public TMapObjBase {
public:
	void control();
	void attachCoin(TCoin*, int);
	void calcAfter();
	void perform(u32 cue, JDrama::TGraphics* graphics);
	void loadAfter();
	void initMapObj();
	TShellCup(const char* name = "シェルカップ");

public:
	/* 0x138 */ TPinnaShell unk138[6];
	/* 0x498 */ void* unk498;
	/* 0x49C */ void* unk49C;
	/* 0x4A0 */ void* unk4A0;
};

class TMerrygoround : public TMapObjBase {
public:
	void control();
	void draw() const;
	void initMapObj();
	TMerrygoround(const char* name = "メリーゴーランド");
};

class TChangeStageMerrygoround : public TMapObjChangeStage {
public:
	void touchPlayer(THitActor*);
	void calc();

	TChangeStageMerrygoround()
	    : TMapObjChangeStage("ステージ切り替え（メリーゴーランド用）")
	    , unk13C(0)
	{
	}

public:
	/* 0x13C */ u8 unk13C;
};

class TBalloonKoopaJr : public TMapObjGeneral {
public:
	void touchActor(THitActor*);
	void kill();
	void load(JSUMemoryInputStream&);
	TBalloonKoopaJr(const char* name = "風船（クッパＪｒ）");
};

class TPinnaEntrance : public TMapObjBase {
public:
	void loadAfter();
	TPinnaEntrance(const char* name = "ピンナ入り口");
};

class TWaterRecoverObj : public TMapObjBase {
public:
	void touchPlayer(THitActor*);
	TWaterRecoverObj(const char* name = "水回復オブジェ");
};

class TAmiKing : public TMapObjBase {
public:
	u32 touchWater(THitActor*) { return 1; }
	void loadAfter();
	void initMapObj();
	void moveObject();
	void calcRootMatrix();
	void bind();
	void touchPlayer(THitActor*);
	TAmiKing(const char* name = "アミキング");
};

class TPinnaCoaster : public TMapObjBase {
public:
	void control();
	void initMapObj();
	TPinnaCoaster(const char* name = "コースター");
};

class TMerryPole : public TMapObjBase {
public:
	virtual Mtx* getRootJointMtx() const { return (Mtx*)unk138.mMtx; }

	TMerryPole()
	    : TMapObjBase("メリーゴーランド用ポール")
	{
		unk138.identity();
	}

public:
	/* 0x138 */ TMtx34f unk138;
};

#endif
