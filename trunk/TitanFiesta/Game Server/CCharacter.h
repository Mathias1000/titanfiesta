/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */
#pragma once

#pragma pack(push)
#pragma pack(1)
class CCharacter
{
public:
	dword Id;
	word ClientId;
	char Name[0x10];

	// Player Stats
	word Level;
	qword Exp;
	dword MaxHp;
	dword MaxSp;
	dword Hp;
	dword Sp;
	dword Fame;
	qword Money;
	byte StrBonus;
	byte EndBonus;
	byte DexBonus;
	byte IntBonus;
	byte SprBonus;
	dword KillPoints;

	// Map Information
	char Map[0x0D];
	struct {
		dword Y;
		dword X;
		byte Rotation;
	} Pos;
	struct {
		dword Y;
		dword X;
		byte Rotation;
	} NewPos;

	// View Information
	byte State; // 0x01 = Normal / 0x03 = Dead / 0x04 = Resting / 0x05 = Shop / 0x06 = Mount
	byte Icon; // Has to be same as Job?
	union {
		struct {
	byte Unknown2 : 2;
	byte Job : 5;
	byte Gender : 1;
		};
		byte Profession;
	};
	struct {
		byte Style;
		byte Color;
	} Hair;
	struct {
		byte Style;
	} Face;
	union {
		struct {
			word Helmet;
			word Weapon;
			word Body;
			word Shield;
			word Pants;
			word Boots;
		};
		word Items[0x14];
	};
	struct {
		byte Shield : 4;
		byte Weapon : 4;
	} Refine;

	word Unknown3;
	byte Unknown4;
	word Unknown5;
	byte Emote;
	word Unknown6;
	
	struct {
		byte Id;
		byte Level;
		word MonsterId;
	} Title;

	byte Buff[0x28];
	dword GuildId;
	word Unknown7;

	CCharacter() {
		// Initialize the entire struct as 0, except for items.
		memset(this, 0, sizeof(CCharacter));
		memset(Items, 0xff, sizeof(Items));
		Unknown2 = 1; // Random job thing.
	}
};
#pragma pack(pop)