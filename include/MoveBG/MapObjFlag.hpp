#ifndef MOVE_BG_MAP_OBJ_FLAG_HPP
#define MOVE_BG_MAP_OBJ_FLAG_HPP

#include <JSystem/JDrama/JDRViewObj.hpp>
#include <Strategic/HitActor.hpp>

class TMapObjFlag;

class TMapObjFlagManager : public JDrama::TViewObj {
public:
	class TMapObjFlagInfo {
	public:
		TMapObjFlagInfo()
		{
			unk0  = nullptr;
			unk54 = 0;
		}

		/* 0x0 */ TMapObjFlag* unk0;
		/* 0x4 */ char unk4[0x50];
		/* 0x54 */ u32 unk54;
	};

	TMapObjFlagManager(const char* name);

	virtual ~TMapObjFlagManager() { }
	virtual void load(JSUMemoryInputStream& stream);
	virtual void perform(u32 cue, JDrama::TGraphics* graphics);

	void registerObj(TMapObjFlag* flag, const char* name);
	void loadFlag(TMapObjFlagInfo* info, TMapObjFlag* flag, const char* name);
	void initDraw();

	/* 0x10 */ TMapObjFlagInfo mInfos[15];
};

extern TMapObjFlagManager* gpMapObjFlagManager;

class TMapObjFlag : public THitActor {
public:
	TMapObjFlag(const char* name);

	virtual ~TMapObjFlag() { }
	virtual void load(JSUMemoryInputStream& stream);
	virtual void updateVertex();

	void init(const char* name);
	void update();
	void draw();

	static f32 mFlutterSpeed;

	// Retail `new TMapObjFlag` is 0xC0. Field init belongs to the parked ctor.
	/* 0x68 */ u8 unk68[0x58];
};

class TMapObjFlagLower : public TMapObjFlag {
public:
	TMapObjFlagLower(const char* name)
	    : TMapObjFlag(name)
	{
	}

	virtual ~TMapObjFlagLower() { }
	virtual void updateVertex();
};

class TMapObjFlagSail : public TMapObjFlag {
public:
	TMapObjFlagSail(const char* name)
	    : TMapObjFlag(name)
	{
	}

	virtual ~TMapObjFlagSail() { }
	virtual void updateVertex();
};

#endif
