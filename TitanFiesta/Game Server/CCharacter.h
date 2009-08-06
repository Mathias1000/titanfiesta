/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#pragma pack(push)
#pragma pack(1)
struct CCharacter
{
	word clientid;
	char charname[0x10];
	struct {
		dword y;
		dword x;
		byte rot;
	} map;
	byte unk1;
	byte isvisible;
	byte unknown : 2;
	byte job : 5;
	byte gender : 1;
	byte hairstyle;
	byte haircolor;
	byte facestyle;
/*
	word helmet;
	word weapon;
	word armor;
	word shield;
	word pants;
	word boots;
	byte unkitems[0x1C];*/
	union {
		struct {
			word helmet;
			word weapon;
			word armor;
			word shield;
			word pants;
			word boots;
		};
		byte items[0x28];
	};
	struct {
		byte shield : 4;
		byte weapon : 4;
	} refine;

	word unk3;
	byte unk4;
	word unk5;
	byte emote;
	word unk6;
	
	struct {
		byte id;
		byte level;
		word monsterid;
	} title;

	byte buff[0x28];
	dword guildid;
	word unk7;



};
#pragma pack(pop)