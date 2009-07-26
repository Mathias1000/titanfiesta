#pragma once
// 4Slots ea 24(6*4);
#define InventorySize 96

#define EquipmentSize 28

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
	icJewel=		 4,
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
	byte amount;
};
struct ItemArmor {
	word id;
	byte refine;
	word unknown1;
	dword ExpireDate;
	char statBonisCount; 
	StatBonis statBonis[0];
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
	StatBonis statBonis[0];
};

struct ItemJewel {
	word id;
	byte unknown1;
	byte unknown2;
	byte unknown3;
	byte unknown4;
	byte unknown5;
};

struct ItemHouse {
	word id;
	word unknown1;
	word unknown2;
};

struct ItemMover {
	word id;
	word fuel;
	word unknown1; //same as in ItemHouse
	word unknown2; //same as in ItemHouse
	word unknown3;
};

struct ItemQuest {
	word id;
	byte amount;
	byte unknown;
};

struct ItemShield {
	word id;
	byte refine;
	word unknown1;
	dword ExpireDate;
	char statBonisCount; 
	StatBonis statBonis[0];
};

#pragma warning( pop )
#pragma pack ( pop )

