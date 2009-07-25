#include "..\Common\CItems.h"

int getItemSize(ItemClass c)
{
	switch (c) {
		case icRegular:		return sizeof(ItemRegular);
		//case icCen:		return sizeof(ItemCen);
		case icQuest:		return sizeof(ItemQuest);
		case icJewel:		return sizeof(ItemJewel);
		case icWeapon:		return sizeof(ItemWeapon);
		case icArmor:		return sizeof(ItemArmor);
		case icShield:		return sizeof(ItemShield);
		case icMinorArmor:	return sizeof(ItemArmor);
		//case icHouseInv:	return sizeof(ItemHouseInv);
		//case icCostume:		return sizeof(ItemCostume);
		//case icSkill:		return sizeof(ItemSkill);
		//case icTeleport:	return sizeof(ItemTeleport);
		//case icWing:		return sizeof(ItemWing);
		//case icRefineStone:return sizeof(ItemRefineStone);
		//case icPresentBox:	return sizeof(ItemPresentBox);
		//case icLicense:		return sizeof(ItemLicense);
		//case icKey:			return sizeof(ItemKey);
		case icHouse:		return sizeof(ItemHouse);
		//case icRedEye:		return sizeof(ItemRedEye);
		//case icBlueEye:		return sizeof(ItemBlueEye);
		//case icMoverFood:	return sizeof(ItemMoverFood);
		case icMover:		return sizeof(ItemMover);
		//case icSuperPot:	return sizeof(ItemSuperPot);
		//case icGoldNine:	return sizeof(ItemGoldNine);
		//case icCostumeWep:	return sizeof(ItemCostumeWep);

		default: return -1;//TODO: add error
	}
}