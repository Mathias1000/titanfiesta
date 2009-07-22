#include "../Common/Items.h"

int getItemSize(ItemClass c)
{
	switch (c) {
		case icRegular:		return sizeof(ItemRegular);
		//case icCen:		return sizeof(ItemCen);
		//case icQuest:		return sizeof(ItemQuest);
		//case icJewels:		return sizeof(ItemJewels);
		case icWeapon:		return sizeof(ItemWeapon);
		case icArmor:		return sizeof(ItemArmor);
		//case icShield:		return sizeof(ItemShield);
		//case icMinorArmor:	return sizeof(ItemMinorArmor);
		//case icHouseInv:	return sizeof(ItemHouseInv);
		//case icCostume:		return sizeof(ItemCostume);
		//case icSkicase:		return sizeof(ItemSkill);
		//case icTeleport:	return sizeof(ItemTeleport);
		//case icWing:		return sizeof(ItemWing);
		//case icRefineStone:return sizeof(ItemRefineStone);
		//case icPresentBox:	return sizeof(ItemPresentBox);
		//case icLicense:		return sizeof(ItemLicense);
		//case icKey:			return sizeof(ItemKey);
		//case icHouse:		return sizeof(ItemHouse);
		//case icRedEye:		return sizeof(ItemRedEye);
		//case icBlueEye:		return sizeof(ItemBlueEye);
		//case icMoverFood:	return sizeof(ItemMoverFood);
		//case icMover:		return sizeof(ItemMover);
		//case icSuperPot:	return sizeof(ItemSuperPot);
		//case icGoldNine:	return sizeof(ItemGoldNine);
		//case icCostumeWep:	return sizeof(ItemCostumeWep);

		default: return -1;//TODO: add error
	}
}
bool ItemList::Insert(InventoryNode* node)
{
	iterator it= begin();
	for(; it < end(); it++) {
		if ((*it)->Pos == node->Pos)return false;
		else if ((*it)->Pos > node->Pos)
			break;
	}
	insert(it, node);
	return true;
}

bool ItemList::Move(int pos1, int pos2, InventoryNode **out1, InventoryNode **out2)
{
	if (pos1 < pos2)
	{
		int tmp= pos1;
		pos1= pos2;
		pos2= tmp;
	}

	iterator it1= begin();
	for(; it1 < end(); it1++)
		if ((*it1)->Pos > pos1) break;

	iterator it2= it1;
	for(; it2 < end(); it2++)
		if ((*it2)->Pos > pos2) break;

	if (it1 != end() && (*it1)->Pos == pos1) 
	{
		if (it2 != end() && (*it2)->Pos == pos2) { //switch 
			InventoryNode *tmp= *it1;
			*it1= *it2;
			*it2= tmp;

			(*it1)->Pos= pos1;
			(*it2)->Pos= pos2;

			if ( out1 != NULL )
				*out1= *it2;
			if ( out2 != NULL )
				*out2= *it1;
		} else { 
			//pos1 found, pos2 not
			(*it1)->Pos= pos2;
			insert(it2, *it1);
			if ( out1 != NULL )
				*out1= *it1;
			if ( out2 != NULL )
				*out2 = (InventoryNode *)NULL;
			erase(it1);
		}
	} else if (it2 != end() && (*it2)->Pos == pos2) {
			(*it2)->Pos= pos1;
			insert(it1, *it2);
			if ( out1 != NULL )
				*out1= (InventoryNode *)NULL;
			if ( out2 != NULL )
				*out2 = *it2;
			erase(it2);
	} else return false;
	return true;
}	

bool ItemList::Remove(int pos)
{
	for(iterator it= begin(); it < end(); it++)
	{
		if ((*it)->Pos > pos)
		{
			erase(it);
			free (*it);
			return true;
		}
	}
	return false;
}