/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

//class CBaseItem {
//public:
//	CBaseItem(){}
typedef unsigned __int8			byte;
typedef unsigned __int16		word;
typedef unsigned __int32		dword;
typedef unsigned __int64		qword;
typedef char*					string;

//	~CBaseItem(){}
//
//	word itemId;
//	virtual void Add(CPacket* pakout) = 0;
//private:
//};

//class CWeaponTitle {
//public:
//	word monster;
//	dword kills;//assumption
//};
//
//class CItemStat {
//public:
//	byte id;
//	word value;
//};
//
//class CWeaponItem : public CBaseItem {
//public:
//	byte refine;
//	word unk1;
//	CWeaponTitle title[3];
//	word unk2;
//	char[0x10] creator;
//	dword unk3;
//	byte unk4;
//	byte statCount;
//	//dynamic here.. if statcount == 1 then random byte, else stats.
//	byte slot;
//	byte seperator;
//
//	void SetDefault(){
//		memset(this, 0, sizeof(CWeaponItem));
//
//		title[0].monster = 0xFFFF;
//		title[1].monster = 0xFFFF;
//		title[2].monster = 0xFFFF;
//		
//		unk2 = 0xFFFF;
//	}
//
//	void Add(CPacket* pakout){
//		pakout->Add<CWeaponItem>(*this);
//	}
//private:
//};
//
//class CUseItem : public CBaseItem {
//public:
//	byte quantity;
//	byte unk1;
//	byte slot;
//	byte seperator;
//private:
//}
/*
Use/Mat Struct
[word - ItemID]
[byte - Quantity]
[byte - Unk]
[byte - Slot]
[byte - Sep = 0x90]

*/