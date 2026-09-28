#ifndef ENEMY_BOMB_HEI_HPP
#define ENEMY_BOMB_HEI_HPP

#include <Enemy/SmallEnemy.hpp>
#include <Enemy/WalkerEnemy.hpp>
class TBombHei : public TWalkerEnemy {
public:
	TBombHei(const char* name = "ボム兵");

	virtual const char** getBasNameTable() const;
	virtual void setAfterDeadEffect();

	/* 0x194 */ u32 unk194;
	/* 0x198 */ u32 unk198;
	/* 0x19C */ u8 unk19C;
	/* 0x19D */ u8 unk19D[7];
	/* 0x1A4 */ u8 unk1A4;
	/* 0x1A5 */ u8 unk1A5[3];
};

class TBombHeiManager : public TSmallEnemyManager {
public:
	TBombHeiManager(const char* name = "ボム兵マネージャ");

	virtual void createModelData();
	virtual TSpineEnemy* createEnemyInstance();

	/* 0x60 */ u32 unk60;
};

#endif
