#ifndef NOZZLETRIGGER_HPP
#define NOZZLETRIGGER_HPP

#include <Player/NozzleBase.hpp>

class TNozzleTrigger : public TNozzleBase {
public:
	TNozzleTrigger(const char* name, const char* prm, TWaterGun* fludd);

	virtual void init();
	virtual s32 getNozzleKind() const { return 1; };
	virtual void movement(const TMarioControllerWork&);
	virtual void emit(int);
	virtual void animation(int);

	// Inactive = not holding R, Active = charging R, Dead = R Waiting to be
	// depressed
	enum SPRAYSTATE { INACTIVE = 0, ACTIVE = 1, DEAD = 2 };

	/* 0x384 */ bool unk384; // mRumbleOnCharge
	/* 0x385 */ s8 unk385;   // mSprayState, Current spray state
	/* 0x386 */ s16 unk386;  // Quarter frames left of spray (i think)
	/* 0x388 */ f32 unk388;  // mTriggerFill - How far the trigger has gotten
	/* 0x38C */ u32 unk38C;  // mSoundID - The sound to play when triggering
};

class TNozzleButton {
public:
	TNozzleButton(const char*, const char*, TWaterGun*);
	void init();
	void movement(const TMarioControllerWork&);
	void emit(int);
	void animation(int);
	s32 getNozzleKind() const;
};

class TNozzleTurbo {
public:
	TNozzleTurbo(const char*, const char*, TWaterGun*);
	void animation(int);
	void movement(const TMarioControllerWork&);
	s32 getNozzleKind() const;
};

#endif
