#include "CItems.h"

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
bool ItemList::Insert(ItemNode* node)
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

ItemNode *ItemList::Insert(byte flags, ItemBase* i, int length)
{
	ItemNode *node= (ItemNode *)malloc(3+length);
	node->Flags= flags;
	node->Size= length+2;
	memcpy(&node->Item, i, length);

	int pos= 0;
	iterator it= begin();
	for(; it < end(); it++, pos++)
		if ((*it)->Pos > pos) break;
	if (pos > 255) return NULL;

	node->Pos= pos;
	insert(it, node);
	return node;
}

ItemNode *ItemList::Insert(byte pos, byte flags, ItemBase* i, int length)
{
	ItemNode *node= (ItemNode *)malloc(3+length);
	node->Pos= pos;
	node->Flags= flags;
	node->Size= length+2;
	memcpy(&node->Item, i, length);
	return Insert(node) ? node : NULL;
}

bool ItemList::Move(int pos1, int pos2, ItemNode **out1, ItemNode **out2)
{
	if (pos1 > pos2)
	{
		int tmp= pos1;
		pos1= pos2;
		pos2= tmp;
		ItemNode **tmp2= out1;
		out1= out2;
		out2= tmp2;
	}

	unsigned int i= 0;
	for(; i < size(); i++)
		if (at(i)->Pos >= pos1) break;

	unsigned int j= i;
	for(; j < size(); j++)
		if (at(j)->Pos >= pos2) break;

	if (i < size() && at(i)->Pos == pos1) 
	{
		if (j < size() && at(j)->Pos == pos2) { //switch 
			ItemNode *tmp= at(i);
			at(i)= at(j);
			at(j)= tmp;

			at(i)->Pos= pos1;
			at(j)->Pos= pos2;

			if ( out1 != NULL ) *out1= at(j);
			if ( out2 != NULL )	*out2= at(i);
		} else { 
			//pos1 found, pos2 not
			at(i)->Pos= pos2;
			insert(begin() +j, at(i));
			if ( out1 != NULL )	*out1= at(j);
			if ( out2 != NULL )	*out2= (ItemNode *)NULL;
			erase(begin() +i);
		}
	} else if (j < size() && at(j)->Pos == pos2) {
			//pos2 found, pos1 not
			at(j)->Pos= pos1;
			insert(begin() +i, at(j));
			if ( out1 != NULL )	*out1= (ItemNode *)NULL;
			if ( out2 != NULL )	*out2= at(i);
			erase(begin() +j+1);
	} else {
		if ( out1 != NULL ) *out1= 0;
		if ( out2 != NULL ) *out2= 0;
	}
	return true;
}	

bool ItemList::Remove(int pos)
{
	for(iterator it= begin(); it < end(); it++)
	{
		if ((*it)->Pos == pos)
		{
			free (*it);
			erase( it);
			return true;
		}
	}
	return false;
}

void ItemList::CorrectOrder()
{
	if (size() < 3)
		return;
	bool changed;
	do {
		changed= false;
		ItemNode* last= at(0);
		ItemNode* current;
		for(iterator it= begin()+1; it < end(); it++) {
			current= *it;
			if (last->Pos == current->Pos) //2Items are on the same spot
			{
				current->Pos+= 1;
				changed= true;
			}
			else if (last->Pos > current->Pos) //Order isnt correct
			{
				ItemNode* tmp= last;
				last= current;
				current= tmp;
				changed= true;
			}
			last= current;
		}
	} while (changed);
}
