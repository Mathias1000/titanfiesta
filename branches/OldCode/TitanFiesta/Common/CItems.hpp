#pragma once

// 4 Tabs, 6x4 slots each.
#define MAXINVSLOT 96
#define MAXEQSLOT 28

// Make our structs 1-byte packed. This way everything is properly aligned.
#pragma pack (push, 1)
// Disable the warning issued for having var[] at end of struct. Used for dynamic struct size.
#pragma warning( push )
#pragma warning( disable: 4200 )

#include "..\Common\Common.h"
#include "..\Common\CShn.hpp"

// Data Type Definitions
enum ItemClass {
	Regular,		// 0
	Unused1,
	Sen,
	Quest,
	Accessory,
	Weapon,			// 5
	Armor,
	Shield,
	MinorArmor,
	HouseItem,
	Costume,		// 10
	Skill,
	Scroll,
	Wing,
	Stone,
	PresentBox,		// 15
	License,
	Key,
	House,
	RedEye,
	BlueMile,		// 20
	Unused2,
	MoverFood,
	Mover,
	SuperPot,
	GoldNine,		// 25
	CostumeWeapon,
	ItemClassMax
};

struct SItemStat {
	byte Stat; // 0: Str - 1: End - 2: Dex - 3: Int - 4: Spr
	word Value;
};

struct SWeaponTitle {
	word MonsterId;
	dword KillCount;
};

struct SItemBase {
	byte Length;
	byte Slot;
	byte Type;
	word Id;
};

// Item Types

struct SItemRegular : SItemBase {
	byte Count;
};

struct SItemSen : SItemBase {
	// TODO: SItemSen Structure
};

struct SItemQuest : SItemBase {
	word Count;
};

struct SItemAccessory : SItemBase {
	f_date ExpireDate;
	byte Visible : 1;
	byte StatCount : 7;
	SItemStat Stats[];
};

struct SItemWeapon : SItemBase {
	byte Refine;
	word Unknown1;
	SWeaponTitle Title[3];
	word Unknown2;
	byte Titler[0x11];
	f_date ExpireDate;
	byte Visible : 1;
	byte StatCount : 7;
	SItemStat Stats[];
};

struct SItemArmor : SItemBase {
	byte Refine;
	word Unknown1;
	f_date ExpireDate;
	byte Visible : 1;
	byte StatCount : 7;
	SItemStat Stats[];
};

struct SItemShield : SItemBase {
	byte Refine;
	word Unknown1;
	f_date ExpireDate;
	byte Visible : 1;
	byte StatCount : 7;
	SItemStat Stats[];
};

struct SItemMinorArmor : SItemBase {
	byte Refine;
	word Unknown1;
	f_date ExpireDate;
	byte Visible : 1;
	byte StatCount : 7;
	SItemStat Stats[];
};

struct SItemHouseItem : SItemBase {
	// TODO: SItemHouseItem
};

struct SItemCostume : SItemBase {
	// TODO: SItemCostume Structure
};

struct SItemSkill : SItemBase {
	// TODO: SItemSkill Structure
};

struct SItemScroll : SItemBase {
	byte Count;
};

struct SItemWing : SItemBase {
	// TODO: SItemWing
};

struct SItemStone : SItemBase {
	byte Count;
};

struct SItemPresentBox : SItemBase {
	// TODO: SItemPresentBox
};

struct SItemLicense : SItemBase {
	byte Unknown;
};

struct SItemKey : SItemBase {
	// TODO: SItemKey
};

struct SItemHouse : SItemBase {
	word Unknown1; // Matches SItemMover
	word Unknown2; // Matches SItemMover
};

struct SItemRedEye : SItemBase {
	// TODO: SItemRedEye
};

struct SItemBlueMile : SItemBase {
	// TODO: SItemBlueMile
};

struct SItemMoverFood : SItemBase {
	byte Count;
};

struct SItemMover : SItemBase {
	word Fuel;
	word Unknown1; // Matches SItemHouse
	word Unknown2; // Matches SItemHouse
	word Unknown3;
};

struct SItemSuperPot : SItemBase {
	// TODO: SItemSuperPot
};

struct SItemGoldNine : SItemBase {
	// TODO: SItemGoldNine
};

struct SItemCostumeWeapon : SItemBase {
	// TODO: SItemCostumeWeapon
};

#pragma pack ( pop )
#pragma warning ( pop )

// Class
class CItemManager {
public:
	CItemManager(CShn* ItemInfo, byte SlotCount) {
		this->_ItemInfo = ItemInfo;
		this->_SlotCount = SlotCount;
		this->_ItemCount = 0;
		this->_DataLength = 0;

		this->_Items = new SItemBase*[SlotCount];
		memset(this->_Items, 0, sizeof(SItemBase*) * SlotCount);
	};
	~CItemManager() {
		for (int i = 0; i < this->_SlotCount; i++) {
			if (this->_Items[i] != NULL)
				this->FreeItem(this->_Items[i]);
		}
		delete[] this->_Items;
	};

	// Returns how many items were loaded.
	byte LoadItemDump(byte* ItemBin) {
		// Don't load in if there's already items.
		if (this->_ItemCount > 0) return 0;

		byte* Pos = ItemBin;
		this->_ItemCount = *Pos++;
		this->_ItemType = *Pos++;

		for (int i = 0; i < this->_ItemCount; i++) {
			byte ItemSize = *Pos++;
			SItemBase* Item = (SItemBase*)new byte[ItemSize + 1];
			memcpy((byte*)Item + 1, Pos, ItemSize);
			Item->Length = ItemSize;
			this->_Items[Item->Slot] = Item;
			this->_DataLength += ItemSize + 1;
			Pos += ItemSize;
		}
		return this->_ItemCount;
	};

	// Dump item data to a binary blob. Sets &DataLength to size of blob.
	byte* DumpItems(word &DataLength) {
		byte* Buffer = new byte[this->_DataLength + 2];
		byte* Pos = Buffer;
		*Pos++ = this->_ItemCount;
		*Pos++ = this->_ItemType;
		DataLength = this->_DataLength + 2;
		// If there are no items, we still return the count/type
		if (this->_ItemCount == 0) return Buffer;

		for (int i = 0; i < this->_SlotCount; i++) {
			if (this->_Items[i] == NULL) continue;
			memcpy(Pos, this->_Items[i], this->_Items[i]->Length + 1);
			Pos += this->_Items[i]->Length + 1;
		}
		return Buffer;
	};

	void FreeDump(byte* ItemDump) {
		delete[] ItemDump;
	}

	SItemBase* CreateItem(word ItemId, byte ItemType, byte Stats) {
		if (this->_ItemInfo == NULL) return NULL;
		ItemClass Class = this->GetItemClass(ItemId);
		if (Class == ItemClassMax) return NULL;
		// If Class == 0 it could be item not found. Check if ID == 0
		if (!Class && !this->_ItemInfo->GetDwordId(ItemId, 0)) return NULL;
		int Size = GetItemSize(Class);
		if (Size == -1) return NULL;

		// Allocate space for statcount/stats if required.
		if (Stats != 0)
			Size += Stats * sizeof(SItemStat) + 1;
		SItemBase* Item = (SItemBase*)new byte[Size];
		memset(Item, 0, Size);
		Log(MSG_INFO, "Created item: Size: %d  Class: %d  Type: %d  Id: %d", Size, Class, ItemType << 2, ItemId);
		Item->Id = ItemId;
		Item->Length = Size - 1; // -1 to account for Size
		Item->Slot = 0;
		Item->Type = ItemType << 2;

		// Special case for weapons, they have titles that need to be 0xff'd
		if (Class == 5) {
			SItemWeapon* Weapon = (SItemWeapon*)Item;
			memset(Weapon->Titler, 0xff, 0x11);
			Weapon->Title[0].MonsterId = 0xffff;
			Weapon->Title[1].MonsterId = 0xffff;
			Weapon->Title[2].MonsterId = 0xffff;
		}
		return Item;
	};

	void FreeItem(SItemBase*& Item) {
		delete[] (byte*)Item;
		Item = NULL;
	};

	bool SetItem(SItemBase* Item, byte Slot) {
		if (Slot > this->_SlotCount || this->_Items[Slot] != NULL) return false;
		this->_Items[Slot] = Item;
		Item->Slot = Slot;
		Item->Type = this->_ItemType << 2;
		this->_ItemCount++;
		this->_DataLength += Item->Length + 1;
		return true;
	}

	SItemBase* GetItem(byte Slot) {
		if (Slot > this->_SlotCount) return NULL;
		return this->_Items[Slot];
	}

	bool RemoveItem(byte Slot, bool FreeMemory = true) {
		if (Slot > this->_SlotCount || this->_Items[Slot] == NULL) return false;

		this->_DataLength -= this->_Items[Slot]->Length + 1;
		if (FreeMemory) FreeItem(this->_Items[Slot]);
		this->_Items[Slot] = NULL;
		this->_ItemCount--;
		return true;
	}

	bool SwapItems(byte SourceSlot, byte DestSlot) {
		if (SourceSlot > this->_SlotCount || DestSlot > this->_SlotCount) return false;

		SItemBase* tmp = this->_Items[SourceSlot];
		this->_Items[SourceSlot] = this->_Items[DestSlot];
		this->_Items[DestSlot] = tmp;
		if (this->_Items[SourceSlot]) this->_Items[SourceSlot]->Slot = SourceSlot;
		if (this->_Items[DestSlot]) this->_Items[DestSlot]->Slot = DestSlot;
		return true;
	}

	// Function Definitions
	int GetItemSize(ItemClass c)
	{
		byte BaseSize = sizeof(SItemBase);
		switch (c) {
			case Regular:		return sizeof(SItemRegular);
			case Sen:			return sizeof(SItemSen);
			case Quest:			return sizeof(SItemQuest);
			case Accessory:		return sizeof(SItemAccessory);
			case Weapon:		return sizeof(SItemWeapon);
			case Armor:			return sizeof(SItemArmor);
			case Shield:		return sizeof(SItemShield);
			case MinorArmor:	return sizeof(SItemMinorArmor);
			case HouseItem:		return sizeof(SItemHouseItem);
			case Costume:		return sizeof(SItemCostume);
			case Skill:			return sizeof(SItemSkill);
			case Scroll:		return sizeof(SItemScroll);
			case Wing:			return sizeof(SItemWing);
			case Stone:			return sizeof(SItemStone);
			case PresentBox:	return sizeof(SItemPresentBox);
			case License:		return sizeof(SItemLicense);
			case Key:			return sizeof(SItemKey);
			case House:			return sizeof(SItemHouse);
			case RedEye:		return sizeof(SItemRedEye);
			case BlueMile:		return sizeof(SItemBlueMile);
			case MoverFood:		return sizeof(SItemMoverFood);
			case Mover:			return sizeof(SItemMover);
			case SuperPot:		return sizeof(SItemSuperPot);
			case GoldNine:		return sizeof(SItemGoldNine);
			case CostumeWeapon:	return sizeof(SItemCostumeWeapon);
			default: return -1;//TODO: add error
		}
	};

	ItemClass GetItemClass(word ItemId) {
		if (this->_ItemInfo == NULL) return ItemClassMax;
		return (ItemClass)this->_ItemInfo->GetDwordId(ItemId, 4);
	}

	word GetEquipSlot(word ItemId) {
		if (this->_ItemInfo == NULL) return 0;
		return this->_ItemInfo->GetDwordId(ItemId, 6);
	}

	byte GetItemCount() {return this->_ItemCount;};
	byte GetSlotCount() {return this->_SlotCount;};

	byte ItemType() {return this->_ItemType;};
	void ItemType(byte Type) {this->_ItemType = Type;};
	int GetNextSlot() {
		for (int i = 0; i < this->_SlotCount; i++) {
			if (this->_Items[i] == NULL) return i;
		}
		return -1;
	}

private:
	CShn* _ItemInfo;
	byte _SlotCount;
	byte _ItemCount;
	byte _ItemType;
	word _DataLength;

	SItemBase** _Items;
};
