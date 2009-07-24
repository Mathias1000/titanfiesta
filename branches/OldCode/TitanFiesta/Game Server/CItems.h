#ifndef _ITEMS_H
#define _ITEMS_H

#pragma pack (push, 1)
#pragma warning( push )
#pragma warning( disable : 4200 )
// 10 47	pmSetItemList			Server: Sets a Item List for client

#include "..\Common\Common.hpp"
#include <vector>

struct ItemBase;
struct ItemRegular;
struct ItemWeapon;
struct ItemArmor;

struct ItemNode;
enum ItemClass;
struct StatBonis;
struct weaponTitle;



struct ItemBase {
	word id;
};

struct ItemNode{
	byte Size;
	byte Pos;
	byte Flags; //Not sure how to call it
	ItemBase Item;
};

enum ItemClass{
	icRegular=		 0,
	icCen=			 2,
	icQuest=		 3,
	icJewels=		 4,
	icWeapon=		 5,
	icArmor=		 6,
	icShield=		 7,
	icMinorArmor=	 8,
	icHouseInv=		 9,
	icCostume=		10,
	icSkill=		11,
	icTeleport=		12,
	icWing=			13,
	icRefineStone=	14, 
	icPresentBox=	15, 
	icLicense=		16,
	icKey=			17,
	icHouse=		18,
	icRedEye=		19,
	icBlueEye=		20,
	icMoverFood=	22,
	icMover=		23,
	icSuperPot=		24,
	icGoldNine=		25,
	icCostumeWep=	26,
};


struct StatBonis{
	//possible types:
	//Str= 0,
	//End= 1,
	//Dex= 2,
	//Int= 3,
	//Spr= 4,
	byte type;
	word value;
} ;

struct weaponTitle {
	word monsterId;
	dword kills;
};


int getItemSize(ItemClass c);

struct ItemRegular {
	word id;
	word amount;
};
struct ItemArmor {
	word id;
	byte refine;
	word unknown1;
	dword ExpireDate;
	char statBonisCount; 
	StatBonis statBonis[5];
};
struct ItemWeapon {
	word id;
	byte refine;
	word unknown1;
	weaponTitle	titles[3];
	word unknown2;
	char creator[17];
	dword ExpireDate;
	char statBonisCount; 
	StatBonis statBonis[5];
};
#pragma warning( pop )
#pragma pack ( pop )


class ItemList: public std::vector<ItemNode *>
{
public:
		std::vector<ItemNode *>::allocator_type Find(int Slot);
		int Size();
		bool Insert(ItemNode* node);
		bool Move(int pos1, int pos2, ItemNode **out1= NULL, ItemNode **out2= NULL);
		bool Remove(int pos);
};

#endif