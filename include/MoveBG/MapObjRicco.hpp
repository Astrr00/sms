#ifndef MOVE_BG_MAP_OBJ_RICCO_HPP
#define MOVE_BG_MAP_OBJ_RICCO_HPP

#include <MoveBG/MapObjBase.hpp>
#include <MoveBG/MapObjBlock.hpp>
#include <MoveBG/Item.hpp>

// TODO: mark virtual methods as such

class TCraneRotY : public TMapObjBase {
public:
	void calc();
	void control();
	void load(JSUMemoryInputStream&);
	TCraneRotY(const char* name = "Ｙ軸回転クレーン");

public:
	/* 0x138 */ f32 unk138;
	/* 0x13C */ f32 unk13C;
	/* 0x140 */ f32 unk140;
	/* 0x144 */ f32 unk144;
	/* 0x148 */ u32 unk148;
};

class TCraneUpDown : public TMapObjBase {
public:
	void control();
	void initMapObj();
	TCraneUpDown(const char* name = "上下クレーン");
};

class TCraneCargo : public TLeanBlock {
public:
	void control();
	void calc();
	TCraneCargo()
	    : TLeanBlock("クレーン積み荷")
	{
	}
};

class TRiccoWatermill : public TMapObjBase {
public:
	u32 touchWater(THitActor*);
	void control();
	void calc();
	void loadAfter();
	TRiccoWatermill(const char* name = "リコ水車");
};

class TSurfGesoObj : public TItem {
public:
	void initMapObj();
	TSurfGesoObj(const char* name = "イカサーフィン");
};

class TFruitLauncher;

class TFruitSwitch : public TMapObjBase {
public:
	void pullUp();
	void pushDown();
	BOOL receiveMessage(THitActor* sender, u32 message);
	TFruitSwitch(const char* name = "フルーツスイッチ");

public:
	/* 0x138 */ TFruitLauncher* unk138;
};

class TFruitLauncher : public TMapObjBase {
public:
	void appearFruit() const;
	void fireObj();
	void loadAfter();
	TFruitLauncher(const char* name = "フルーツ発射口");
};

#endif
