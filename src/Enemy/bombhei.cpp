#include <Enemy/BombHei.hpp>
#include <M3DUtil/InfectiousStrings.hpp>

static const char* bombhei_bastable[] = {
	"/scene/bombhei/bas/downnejibomb_down1.bas",
	nullptr,
	nullptr,
	"/scene/bombhei/bas/nejibomb_land1.bas",
	nullptr,
	nullptr,
	"/scene/bombhei/bas/nejibomb_stop_down1.bas",
};

TBombHei::TBombHei(const char* name)
    : TWalkerEnemy(name)
    , unk194(0)
    , unk198(0)
    , unk19C(1)
    , unk1A4(0)
{
}

const char** TBombHei::getBasNameTable() const { return bombhei_bastable; }

TBombHeiManager::TBombHeiManager(const char* name)
    : TSmallEnemyManager(name)
    , unk60(0)
{
}

void TBombHeiManager::createModelData()
{
	static TModelDataLoadEntry entry[] = {
		{ "nejibomb_model1.bmd", 0x10230000, 0 },
		{ "downnejibomb_model1.bmd", 0x10210000, 0 },
		{ nullptr, 0, 0 },
	};
	createModelDataArray(entry);
}

TSpineEnemy* TBombHeiManager::createEnemyInstance()
{
	return new TBombHei;
}
