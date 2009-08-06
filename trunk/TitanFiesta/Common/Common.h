/* Copyright (C) 2008, 2009 TitanFiesta Dev Team
 * Licensed under GNU GPL v3
 * For license details, see LICENCE in the root folder. */

#pragma once

#define USASERVER 0
#define EURSERVER 1

const int SERVERTYPE = USASERVER; // 0 = USA Server / 1 = EUR Server

struct f_date {
	unsigned year : 8;
	unsigned month : 5;
	unsigned day : 6;
	unsigned hour : 6;
	unsigned minute : 7;

	f_date(){year=0;month=0;day=0;hour=0;minute=0;};
};

typedef unsigned __int8			byte;
typedef unsigned __int16		word;
typedef unsigned __int32		dword;
typedef unsigned __int64		qword;
typedef char*					string;
