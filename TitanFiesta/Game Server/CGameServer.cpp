#include "main.h"
#include "CGameServer.hpp"
#include "CCharacter.h"

bool CGameServer::OnServerReady(){
	CTitanIniReader ini("GameServer.ini");
	db = new CTitanSQL(ini.GetString("Server","MySQL","127.0.0.1"), ini.GetString("Username","MySQL","root"), ini.GetString("Password","MySQL",""), ini.GetString("Database","MySQL","titanfiesta"));
	if(!db->Connect()){
		Log(MSG_ERROR, "Failed to connect to MYSQL Server");
		return false;
	}else{
		Log(MSG_STATUS, "Connected to MYSQL Server");
	}
	
	itemInfo = new CShn();
	if(!itemInfo->Open("ItemInfo.shn", 0)){
		Log(MSG_ERROR, "Could not open 'ItemInfo.shn'");
		return false;
	}
	
	ServerData.flags = 0;
	ServerData.id = ini.GetInt("Id","Game Server",3);
	ServerData.ip = Config.BindIp;
	ServerData.iscid = 99;
	ServerData.name = ini.GetString("Name","Game Server","Game Server");
	ServerData.owner = ini.GetInt("Owner","Game Server",2);
	ServerData.port = Config.BindPort;
	ServerData.status = 9;
	ServerData.type = 3;

	// Identify with ISC
	CISCPacket pakout(TITAN_ISC_IDENTIFY);
	pakout.Add<CServerData*>( &ServerData );
	SendISCPacket( &pakout );

	return true;
}

void CGameServer::OnReceivePacket( CTitanClient* baseclient, CTitanPacket* pak ){
	CGameClient* thisclient = (CGameClient*)baseclient;
	Log(MSG_INFO,"Received packet: Command: %04x Size: %04x", pak->Command(), pak->Size());

	if(thisclient->id == -1){
		if(pak->Command() == 0x1801)
			PACKETRECV(pakUserLogin);
	}else{
		switch(pak->Command()){
			case 0x0817:
				Log(MSG_DEBUG, "0x0817 Hide Players");
			break;
			case 0x1071:
				Log(MSG_DEBUG, "0x1071 Quitting Game...");
			break;
			case 0x1072:
				Log(MSG_DEBUG, "0x1072 Stop Quitting Game! WE WANT MORE PLAY!");
			break;
			case 0x1803: // pakClientReady
				PACKETRECV(pakClientReady);
			break;
			case 0x1c01: // pakRequestEntityInformation
				Log(MSG_DEBUG, "Client wants information about entity %d", pak->Read<word>());
			break;
			case 0x2001:
				PACKETRECV(pakChat);
			break;
			case 0x2012:
				Log(MSG_DEBUG, "0x2012 Stop Moving -> cY: %d cX: %d", pak->Read<dword>(), pak->Read<dword>());
			break;
            case 0x2017:
                Log(MSG_DEBUG, "0x2017 Walk Packet -> nY: %d nX: %d cY: %d cX: %d", pak->Read<dword>(), pak->Read<dword>(), pak->Read<dword>(), pak->Read<dword>());
            break;
			case 0x2019:
				PACKETRECV(pakMove);
			break;
			case 0x201e:
				PACKETRECV(pakShout);
			break;
			case 0x2020:
				PACKETRECV(pakBasicAction);
			break;
			case 0x2024:
				Log(MSG_DEBUG, "0x2024 Jump!");
			break;
			case 0x2027:
				PACKETRECV(pakRest);
			break;
			case 0x202A:
				PACKETRECV(pakEndRest);
			break;
			case 0x300B:
				PACKETRECV(pakMoveInvItem);
			break;
			case 0x300F:
				PACKETRECV(pakEquipInvItem);
			break;
			case 0x3010:
				PACKETRECV(pakEquipInvItemSlot);
			break;
			case 0x3012:
				PACKETRECV(pakUnequipInvItem);
			break;
			case 0x3020: // Request Premium Items
			{
				Log(MSG_DEBUG, "Player requested premium item tab %d", pak->Read<word>());
				CPacket pakout(0x3021);
				pakout.AddFile("3021.pak");
				SendPacket(thisclient, &pakout);
			}
			break;
			case 0x3c02:
			{
				byte btWho = pak->Read<byte>();
				char* who;
				if(btWho == 0)
					who = "ExJam";
				else if(btWho == 1)
					who = "Drakia";
				else if(btWho == 2)
					who = "Brett";

				char buffer[255];
				sprintf_s(buffer, 255, "\"%s\" wants to have sex with %s!", thisclient->charname, who);
				CPacket pakout(0x6402);
				pakout << byte(11);
				pakout.AddStringLen<byte>(buffer);
				pakout << byte(0);
				SendPacket(thisclient, &pakout);
			}
			break;
			case 0x4822:
				{
					word skillId = pak->Read<word>();
					Log(MSG_DEBUG, "0x4822 Learn Production Skill -> id: %d", skillId);
					//[Game]IN 06 23 48 AD 71 01 0B 
					//[Game]IN 05 04 48 AD 71 00 
					{
						CPacket pakout(0x4823);
						pakout << skillId;
						pakout << word(0x0B01);
						SendPacket(thisclient, &pakout);
					}
					{
						CPacket pakout(0x4804);
						pakout << skillId;
						pakout << byte(0x00);
						SendPacket(thisclient, &pakout);
					}
				}
			break;
			case 0x6001:
				PACKETRECV(pakSetTitle);
			break;
			default:
				pak->Pos(0);
				printf("Unhandled Packet, Command: %04x Size: %04x:\n", pak->Command(), pak->Size());
				for(dword i = 0; i < pak->Size(); i++)
					printf("%02x ", pak->Read<byte>());
				printf("\n");
			break;
		}
	}
}

PACKETHANDLER(pakSetTitle){
	dword titleId;
	*pak >> titleId;
	CPacket pakout(0x6002);
	pakout << titleId;
	pakout << word(0x0a01);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakEndRest){
	CPacket pakout(0x202B);
	pakout << word(0x0a81);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakMoveInvItem){
	byte SourceSlot = pak->Read<byte>();
	byte SourceState = pak->Read<byte>();
	byte DestSlot = pak->Read<byte>();
	byte DestState = pak->Read<byte>();
	if (SourceState != DestState) return false; // Unhandles at this time.
	if (SourceState != 0x09 << 2) return false; 
	
	if (!thisclient->Inventory->SwapItems(SourceSlot, DestSlot)) {
		Log(MSG_DEBUG, "Error swapping items. %d -> %d", SourceSlot, DestSlot);
		return false;
	}

	// Technically Source is now Dest, and Dest is now Source.
	SItemBase* Source = thisclient->Inventory->GetItem(SourceSlot);
	SItemBase* Dest = thisclient->Inventory->GetItem(DestSlot);

	CPacket pakout(0x3001);
	pakout.Add<byte>(DestSlot);
	pakout.Add<byte>(DestState);
	pakout.Add<byte>(SourceSlot);
	pakout.Add<byte>(SourceState);
	if (Source != NULL)
		pakout.AddBytes((byte*)Source + 3, Source->Length - 2);
	else 
		pakout.Add<word>(0xffff);
	SendPacket(thisclient, &pakout);

	pakout.Size(pakout.HeaderSize());
	pakout.Pos(pakout.Size());
	pakout.Add<byte>(SourceSlot);
	pakout.Add<byte>(SourceState);
	pakout.Add<byte>(DestSlot);
	pakout.Add<byte>(DestState);
	if (Dest != NULL)
		pakout.AddBytes((byte*)Dest + 3, Dest->Length - 2);
	else 
		pakout.Add<word>(0xffff);
	SendPacket(thisclient, &pakout);

	return true;
}

PACKETHANDLER(pakEquipInvItem){
	byte Slot = pak->Read<byte>();
	SItemBase* Item = thisclient->Inventory->GetItem(Slot);
	if (Item == NULL) return false; // Slot is empty
	
	// Get current item in equipslot
	word EquipSlot = thisclient->Inventory->GetEquipSlot(Item->Id);
	SItemBase* Equip = thisclient->Equipment->GetItem(EquipSlot);

	{
		CPacket pakout(0x3002); // Modify Equipment
		pakout.Add<byte>(Slot);
		pakout.Add<byte>(Item->Type);
		pakout.Add<byte>(EquipSlot);
		pakout.AddBytes((byte*)Item + 3, Item->Length - 2); // Skip size/slot/type
		SendPacket(thisclient, &pakout);
	}
	{
		CPacket pakout(0x3001); // Modify Inventory
		pakout.Add<byte>(EquipSlot);
		pakout.Add<byte>(thisclient->Equipment->ItemType() << 2);
		pakout.Add<byte>(Slot);
		pakout.Add<byte>(Item->Type);
		if (Equip != NULL)
			pakout.AddBytes((byte*)Equip + 3, Equip->Length - 2); // Skip size/slot/type
		else
			pakout.Add<word>(0xffff);
		SendPacket(thisclient, &pakout);
	}

	// Update inventory/equipment
	thisclient->Inventory->RemoveItem(Slot, false);
	if (Equip != NULL) thisclient->Equipment->RemoveItem(EquipSlot, false);
	thisclient->Equipment->SetItem(Item, EquipSlot);
	if (Equip != NULL) thisclient->Inventory->SetItem(Equip, Slot);
	return true;
}
	
PACKETHANDLER(pakEquipInvItemSlot) {
	byte InvSlot = pak->Read<byte>();
	byte EquipSlot = pak->Read<byte>();
	SItemBase* Item = thisclient->Inventory->GetItem(InvSlot);
	if (Item == NULL) return false; // Slot is empty
	
	// Get current item in equipslot
	SItemBase* Equip = thisclient->Equipment->GetItem(EquipSlot);
	
	{
		CPacket pakout(0x3002); // Modify Equipment
		pakout.Add<byte>(InvSlot);
		pakout.Add<byte>(Item->Type);
		pakout.Add<byte>(EquipSlot);
		pakout.AddBytes((byte*)Item + 3, Item->Length - 2); // Skip size/slot/type
		SendPacket(thisclient, &pakout);
	}
	{
		CPacket pakout(0x3001); // Modify Inventory
		pakout.Add<byte>(EquipSlot);
		pakout.Add<byte>(thisclient->Equipment->ItemType() << 2);
		pakout.Add<byte>(InvSlot);
		pakout.Add<byte>(Item->Type);
		if (Equip != NULL)
			pakout.AddBytes((byte*)Equip + 3, Equip->Length - 2); // Skip size/slot/type
	else 
			pakout.Add<word>(0xffff);
		SendPacket(thisclient, &pakout);
	}

	// Update inventory/equipment
	thisclient->Inventory->RemoveItem(InvSlot, false);
	if (Equip != NULL) thisclient->Equipment->RemoveItem(EquipSlot, false);
	thisclient->Equipment->SetItem(Item, EquipSlot);
	if (Equip != NULL) thisclient->Inventory->SetItem(Equip, InvSlot);
	return true;
}

PACKETHANDLER(pakUnequipInvItem) {
	byte SourceSlot = pak->Read<byte>();
	byte DestSlot = pak->Read<byte>();

	SItemBase* Source = thisclient->Equipment->GetItem(SourceSlot);
	SItemBase* Dest = thisclient->Inventory->GetItem(DestSlot);
	if (Source == NULL || Dest != NULL) { // No item equipped or slot taken.
		return false;
	}
	
	{
		CPacket pakout(0x3002); // Clear equipped slot.
		pakout.Add<byte>(DestSlot);
		pakout.Add<byte>(thisclient->Inventory->ItemType() << 2);
		pakout.Add<byte>(SourceSlot);
		pakout.Add<word>(0xffff); // Empty item.
		SendPacket(thisclient, &pakout);
	}
	{
		CPacket pakout(0x3001); // Put item into inventory
		pakout.Add<byte>(SourceSlot);
		pakout.Add<byte>(Source->Type);
		pakout.Add<byte>(DestSlot);
		pakout.Add<byte>(thisclient->Inventory->ItemType() << 2);
		pakout.AddBytes((byte*)Source + 3, Source->Length - 2);
		SendPacket(thisclient, &pakout);
	}

	// Update inventory data.
	thisclient->Equipment->RemoveItem(SourceSlot, false);
	thisclient->Inventory->SetItem(Source, DestSlot);
	return true;
}

PACKETHANDLER(pakRest){
	CPacket pakout(0x2028);
	pakout << word(0x0a81);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakMove) {
	thisclient->newY = pak->Read<dword>();
	thisclient->newX = pak->Read<dword>();
	thisclient->curY = pak->Read<dword>();
	thisclient->curX = pak->Read<dword>();
	Log(MSG_DEBUG, "0x2019 Move Packet -> nY: %d nX: %d cY: %d cX: %d", thisclient->newY, thisclient->newX, thisclient->curY, thisclient->curX);

	CPacket pakout(0x201a);
		pakout.Add<word>(thisclient->clientid);
		pakout.Add<dword>(thisclient->newY);
		pakout.Add<dword>(thisclient->newX);
		pakout.Add<dword>(thisclient->curY);
		pakout.Add<dword>(thisclient->curX);
		pakout.Add<word>(0x68); // Speed?

	for (dword i = 0; i < ClientList.size(); i++)
		SendPacket((CGameClient*)ClientList.at(i), &pakout);

	return true;
}

PACKETHANDLER(pakClientReady) {
	CPacket pakident(0x1c06);
		pakident.Add<word>(thisclient->clientid);
		pakident.AddFixLenStr(thisclient->charname, 0x10);
		pakident.Add<dword>(thisclient->curY);
		pakident.Add<dword>(thisclient->curX);
		pakident.Add<byte>(0);
		pakident.Add<byte>(0x01); // State
		pakident.Add<byte>(thisclient->profession);
		pakident.Add<byte>(thisclient->gender << 7 | thisclient->profession << 2 | 0x01);
		pakident.Add<byte>(thisclient->hairstyle);
		pakident.Add<byte>(thisclient->haircolor);
		pakident.Add<byte>(thisclient->facestyle);
		pakident.Add<word>(thisclient->Equipment->GetItemId(1)); // Helmet
		pakident.Add<word>(thisclient->Equipment->GetItemId(12)); // Weapon
		pakident.Add<word>(thisclient->Equipment->GetItemId(7)); // Armor
		pakident.Add<word>(thisclient->Equipment->GetItemId(10)); // Shield
		pakident.Add<word>(thisclient->Equipment->GetItemId(19)); // Pants
		pakident.Add<word>(thisclient->Equipment->GetItemId(21)); // Boots
		pakident.Fill<byte>(0xff, 0x1A);
		pakident.Add<word>(thisclient->Equipment->GetItemId(28)); // Pet
		pakident.Add<byte>(thisclient->Equipment->GetRefine(12) << 4 | 
						   thisclient->Equipment->GetRefine(10)); // Refine
		pakident.Add<word>(0x00);
		pakident.Add<byte>(0x00);
		pakident.Add<word>(0xffff);
		pakident.Add<byte>(thisclient->emote);
		pakident.Add<word>(0xffff);
		pakident.Add<byte>(0);
		pakident.Add<byte>(0);
		pakident.Add<word>(0);
		pakident.Fill<byte>(0x00, 0x28);
		pakident.Add<dword>(0x00);
		pakident.Add<word>(0x0002);

	// Send all users
	for (dword i = 0; i < ClientList.size(); i++) {
		CGameClient* c = (CGameClient*)ClientList.at(i);
		if (c == NULL) continue;
		CPacket pakout(0x1c06);
		pakout.Add<word>(c->clientid);
		pakout.AddFixLenStr(c->charname, 0x10);
		pakout.Add<dword>(c->curY);
		pakout.Add<dword>(c->curX);
		pakout.Add<byte>(0);
		pakout.Add<byte>(0x01); // State
		pakout.Add<byte>(c->profession);
		pakout.Add<byte>(c->gender << 7 | c->profession << 2 | 0x01);
		pakout.Add<byte>(c->hairstyle);
		pakout.Add<byte>(c->haircolor);
		pakout.Add<byte>(c->facestyle);
		pakout.Add<word>(c->Equipment->GetItemId(1)); // Helmet
		pakout.Add<word>(c->Equipment->GetItemId(12)); // Weapon
		pakout.Add<word>(c->Equipment->GetItemId(7)); // Armor
		pakout.Add<word>(c->Equipment->GetItemId(10)); // Shield
		pakout.Add<word>(c->Equipment->GetItemId(19)); // Pants
		pakout.Add<word>(c->Equipment->GetItemId(21)); // Boots
		pakout.Fill<byte>(0xff, 0x1A);
		pakout.Add<word>(c->Equipment->GetItemId(28)); // Pet
		pakout.Add<byte>(c->Equipment->GetRefine(12) << 4 | 
						 c->Equipment->GetRefine(10)); // Refine
		pakout.Add<byte>(0x00); // Refine
		pakout.Add<word>(0x00);
		pakout.Add<byte>(0x00);
		pakout.Add<word>(0xffff);
		pakout.Add<byte>(c->emote);
		pakout.Add<word>(0xffff);
		pakout.Add<byte>(0);
		pakout.Add<byte>(0);
		pakout.Add<word>(0);
		pakout.Fill<byte>(0x00, 0x28);
		pakout.Add<dword>(0x00);
		pakout.Add<word>(0x0002);

		SendPacket(thisclient, &pakout); // Send client to thisclient
		SendPacket(c, &pakident); // Send thisclient to client
	}

	return true;
}

PACKETHANDLER(pakChat){
	byte chatLen;// = pak->Read<byte>();
	*pak >> chatLen;
	*(byte*)(pak->Buffer() + pak->Pos() + chatLen) = 0;
	if(*(byte*)(pak->Buffer() + pak->Pos()) == '&'){
		char origText[255];
		memcpy(origText, pak->Buffer() + pak->Pos(), chatLen + 1);
		char* command = (char*)(pak->Buffer() + pak->Pos() + 1);
		*(command + chatLen - 1) = 0;
		char* context;
		command = strtok_s(command, " ", &context);
		Log(MSG_DEBUG, "Lul wut GM COMMAND %s", command);
		if(_strcmpi(command, "title") == 0){
			char* titleId = strtok_s(NULL, " ", &context);
			char* titleLevel = strtok_s(NULL, " ", &context);
			if(titleId == NULL || titleLevel == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &title");
				return true;
			}
			CPacket pakout(0x6004);
			pakout << strtobyte(titleId);
			pakout << byte(0x80 | strtobyte(titleLevel));
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "mon") == 0){
			char* monId = strtok_s(NULL, " ", &context);
			if(monId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &mon");
				return true;
			}
			 
			CPacket pakout(0x1C08);
			pakout << word(0x1336);
			pakout << strtoword(monId);
			pakout << dword(0x251e);
			pakout << dword(0x0cb9);
			pakout << dword(1337);
			pakout.Fill<byte>(0, 0x21);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "delsoulja") == 0){
			char* souljaNumber = strtok_s(NULL, " ", &context);
			if(souljaNumber == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &delsoulja <soulja count>");
				return true;
			}

			for(dword i = 0; i < strtoul(souljaNumber, NULL, 0); i++){
				CPacket pakout(0x1C0E);//deletes client id
				pakout << word(0x3000 + i);//ClientID
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "soulja") == 0){
			char* souljaNumber = strtok_s(NULL, " ", &context);
			char* txtRadius = strtok_s(NULL, " ", &context);
			if(souljaNumber == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &soulja <soulja count>");
				return true;
			}

			dword souljaCount = strtoul(souljaNumber, NULL, 0);
			float angleIncrements = ((4.0 * atan( 1.0 )) * 2.0f) / float(souljaCount);
			float rotationIncrements = 180.0f / float(souljaCount);
			float radius = (txtRadius == NULL)?50.0f:float(atoi(txtRadius));
			word clientIdStart = 0x3000;
			dword xStart = 9110;
			dword yStart = 3516;
			char name[0x10];
			CPacket pakout(0x1c07);
			pakout << byte(souljaCount);
			for(dword i = 0; i < souljaCount; i++){
				pakout << word(clientIdStart + i);//ClientID
				sprintf_s(name, 0x10, "Soulja %d", i);
				pakout.AddFixLenStr(name, 0x10);
				pakout << dword(xStart + (cos(angleIncrements * float(i)) * float(radius)));//X
				pakout << dword(yStart + (sin(angleIncrements * float(i)) * float(radius)));//Y
				pakout << byte(180 - byte(rotationIncrements * float(i)));//Starting Rotation
				pakout << byte(0x01);//unk2
				pakout << byte(0x01);//Is Visible
				pakout << byte(0x85);//Profession bollocks
				pakout << byte(0x07);//hair
				pakout << byte(0x01);//hcolour
				pakout << byte(0x00);//face
				pakout << word(0x7d89);//Hat
				pakout << word(0xFFFF);//weapon
				pakout << word(0x8a45);//body
				pakout << word(0xFFFF);//shield
				pakout << word(0xFFFF);//pants
				pakout << word(0xFFFF);//boots
				pakout.Fill<byte>(0xFF, 0x1c);
				pakout << byte(0x99);//Refine weapon << 4 | shield
				pakout << word(0x0000);
				pakout << byte(0x00);
				pakout << word(0xffff);
				pakout << byte(26);
				pakout << word(0xffff);
				pakout << word(0xFFFF);//TitleId/titlelevel
				pakout << word(0x00);//Monster ID For Title
				//0x27 bytes of bit array, oh joys.
				pakout.Fill<byte>(0x00, 0x21);
				pakout << byte(0x40);
				pakout.Fill<byte>(0x00, 0x06);
				pakout << dword(0x00); // GuildID
				pakout << word(0x0200);
			}
			SendPacket(thisclient, &pakout);
			{
				CPacket pakout(0x6402);
				pakout << byte(11);
				pakout.AddStringLen<byte>("CRANK DAT SOULJA BOY");
				pakout << byte(0);
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "ask") == 0){
			CPacket pakout(0x3c01);
			if (SERVERTYPE == EURSERVER)
				pakout.AddFixLenStr("Who would you like to have sex with?", 0x41);
			else
			pakout.AddFixLenStr("Who would you like to have sex with?", 0x40);
			pakout << byte(0x03);//Answer Count

			pakout << byte(0x00);
			pakout.AddFixLenStr("ExJam", 0x20);

			pakout << byte(0x01);
			pakout.AddFixLenStr("Drakia", 0x20);

			pakout << byte(0x02);
			pakout.AddFixLenStr("Brett", 0x20);

			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "drop") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			if(itemId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &drop");
				return true;
			}

			CPacket pakout(0x1c0A);
			pakout << word(0x1335);
			pakout << strtoword(itemId);
			pakout << dword(0x251e);
			pakout << dword(0x0cb9);
			pakout << byte(0x60);
			pakout << byte(0x35);
			pakout << byte(0x08);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "item") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			if(itemId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &item");
				return true;
			}
			SItemBase* Item = thisclient->Inventory->CreateItem(atoi(itemId), 9, 0);
			if (Item == NULL) {
				Log(MSG_ERROR, "Error creating item. Invalid ID or ItemInfo not loaded");
				return false;
			}
			thisclient->Inventory->SetItem(Item, thisclient->Inventory->GetNextSlot());
				CPacket pakout(0x3001);
			pakout.Add<byte>(Item->Slot);
			pakout.Add<byte>(Item->Type);
			pakout.Add<byte>(Item->Slot);
			pakout.Add<byte>(Item->Type);
			pakout.AddBytes((byte*)Item + 3, Item->Length - 2);
				SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "gmsay") == 0){
			char* text = origText + strlen("&gmsay ");
			CPacket pakout(0x6402);
			pakout << byte(11);
			pakout.AddStringLen<byte>(text);
			pakout << byte(0);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "rest") == 0){
			char* first = strtok_s(NULL, " ", &context);
			char* second = strtok_s(NULL, " ", &context);
			if (first == NULL || second == NULL) return true;
			CPacket pakout(0x202B);
			pakout << strtobyte(first);
			pakout << strtobyte(second);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "tele") == 0){
			char* teleX = strtok_s(NULL, " ", &context);
			char* teleY = strtok_s(NULL, " ", &context);
			if(teleX == NULL || teleY == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &tele <teleX> <teleY>");
				return true;
			}
			Log(MSG_DEBUG, "Tele nX: %d nY: %d", strtoul(teleX, NULL, 0), strtoul(teleY, NULL, 0));
			CPacket pakout(0x201b);
			pakout << strtodword(teleY);
			pakout << strtodword(teleX);
			SendPacket(thisclient, &pakout);
		}else if(_strcmpi(command, "equip") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			char* slotId = strtok_s(NULL, " ", &context);
			char* refineId = strtok_s(NULL, " ", &context);
			if(refineId == NULL) refineId = "9";
			if(itemId == NULL || slotId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &equip <item id> <slot id> |<refine id> (9)|");
				return true;
			}
			//34 02 30 0F 90 0C 08 07 03 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 
			{
				CPacket pakout(0x3002);
				pakout << byte(0x0F);
				pakout << byte(0x90);

				pakout << strtobyte(slotId);//'Slot'/'Item Type' 0x0C for Weapon, 0x0A for Shield/Bow
				pakout << strtoword(itemId);//Item
				pakout << strtobyte(refineId);//Refine
				pakout << word(0x01);

				for(dword i = 0; i < 4; i++){
					pakout << word(0xFFFF);
					pakout << dword(0x0);
				}

				pakout.Fill<byte>(0, 0x11);
				pakout << byte(0x01);
				SendPacket(thisclient, &pakout);
			}
			{
				//0C 80 0F 90 FF FF 
				CPacket pakout(0x3001);
				pakout << byte(0x0C);
				pakout << byte(0x80);
				pakout << byte(0x0F);
				pakout << byte(0x90);
				pakout << byte(0xFF);
				pakout << byte(0xFF);
				SendPacket(thisclient, &pakout);
			}
		}else if(_strcmpi(command, "itemname") == 0){
			char* itemId = strtok_s(NULL, " ", &context);
			if(itemId == NULL){
				Log(MSG_DEBUG, "Not enough arguments for &itemname <item id>");
				return true;
			}

			char* itemName = itemInfo->GetStringId(strtoul(itemId, NULL, 0), 2);
			if (itemName == NULL) {
				CPacket pakout(0x2002);
				pakout << thisclient->clientid;
				pakout << byte(strlen("Item Not Found"));
				pakout << ':';
				pakout << "Item Not Found";
				SendPacket(thisclient, &pakout);
			}
			CPacket pakout(0x2002);
			pakout << thisclient->clientid;
			pakout << byte(strlen(itemName));
			pakout << ':';
			pakout << itemName;
			SendPacket(thisclient, &pakout);
		} else if (_strcmpi(command, "sendpak") == 0) {
			char* pakfile = strtok_s(NULL, " ", &context);
			if (pakfile == NULL) {
				Log(MSG_DEBUG, "Not enough params for &sendpak <pakfile>");
				return true;
			}
			CPacket pakout;
			pakout.Pos(1);pakout.Size(1);
			if (!pakout.AddFile(pakfile)) {
				Log(MSG_DEBUG, "Error opening input file %s", pakfile);
				return true;
			}
			pakout.Command((word)pakout.Buffer()[1]);
			SendPacket(thisclient, &pakout);
		}
	}else{
		//leet test kekeke
		CPacket pakout(0x2002);
		pakout.Add<word>(thisclient->clientid);
		pakout.Add<byte>(chatLen);
		pakout.Add<byte>(0x01); // Unknown?
		pakout.AddBytes(pak->Buffer() + pak->Pos(), chatLen);
		for (std::vector<CTitanClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++) {
			CGameClient* c = (CGameClient*)*i;
			if (c == thisclient) pakout.Set<byte>(0x1A, 3);
			else pakout.Set<byte>(0xFE, 3);
			SendPacket((CGameClient*)*i, &pakout);
		}
	}
	return true;
	}

PACKETHANDLER(pakShout) {
	byte length = pak->Read<byte>();
	byte* message = pak->ReadBytes(length, true);
	Log(MSG_DEBUG, "Shout: %s", message);
	CPacket pakout(0x201f);
	pakout.AddFixLenStr(thisclient->charname, 0x10);
	pakout.Add<byte>(0x00);
	pakout.Add<byte>(length);
	pakout.AddBytes(message, length);
	for (std::vector<CTitanClient*>::iterator i = ClientList.begin(); i != ClientList.end(); i++)
		SendPacket((CGameClient*)*i, &pakout);
	delete[] message;
	return true;
}

const static byte packet1802[236] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x72, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 
	0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x25, 0x00, 0x00, 0xb9, 0x0c, 0x00, 0x00 
};

PACKETHANDLER(pakBasicAction) {
	byte actionid = pak->Read<byte>();
	Log(MSG_DEBUG, "0x2020 Do Emote -> id: %d", actionid);
	CPacket pakout(0x2021);
	pakout.Add<word>(thisclient->clientid);
	pakout.Add<byte>(actionid);
	SendPacket(thisclient, &pakout);
	return true;
}

PACKETHANDLER(pakUserLogin){
	MYSQL_RES* result = NULL;
	char charname[0x13];
	charname[0x12] = 0;
	memcpy(charname, pak->Data() + 2, 0x12);

	thisclient->charname = db->MakeSQLSafe(charname);
	thisclient->loginid = pak->Get<word>(0);
	if(_strcmpi(thisclient->charname, charname)){
		Log(MSG_DEBUG, "MySql Safe login %s != %s", thisclient->charname, charname);
		goto authFail;
	}

	Log(MSG_DEBUG, "Charname: %s, Unique Id: %d", thisclient->charname, thisclient->loginid);

	result = db->DoSQL("SELECT u.`id`,u.`username`,u.`loginid`,u.`accesslevel`,u.`lastslot`,c.`id`,c.`level`,c.`profession`,c.`ismale`,c.`map`,c.`hair`,c.`haircolor`,c.`face`, c.`inventory`, c.`equipment` FROM `users` AS `u`, `characters` AS `c` WHERE c.`charname`='%s' AND u.`username`=c.`owner`", thisclient->charname);
	if(!result || mysql_num_rows(result) != 1){
		Log(MSG_DEBUG, "SELECT returned bollocks");
		goto authFail;
	}

	MYSQL_ROW row = mysql_fetch_row(result);

	{	// Check loginid
		if (thisclient->loginid != ((word*)row[2])[0]) {
			Log(MSG_DEBUG, "Incorrect loginid %d != %d", thisclient->loginid, ((word*)row[2])[0]);
		thisclient->id = -1;
		goto authFail;
		}
	}

	// Initialize inventory space.
	thisclient->Inventory = new CItemManager(itemInfo, MAXINVSLOT);
	thisclient->Equipment = new CItemManager(itemInfo, MAXEQSLOT);

	thisclient->id = atoi(row[0]);
	thisclient->username = row[1];
	thisclient->accesslevel = atoi(row[3]);
	thisclient->lastslot = atoi(row[4]);
	thisclient->charid = atoi(row[5]);
	thisclient->clientid = thisclient->charid; // FIXME: Find a better way to do this.
	thisclient->level = atoi(row[6]);
	thisclient->profession = atoi(row[7]);
	thisclient->gender = atoi(row[8]);
	thisclient->hairstyle = atoi(row[10]);
	thisclient->haircolor = atoi(row[11]);
	thisclient->facestyle = atoi(row[12]);

	thisclient->curX = thisclient->newX = 3257;
	thisclient->curY = thisclient->newY = 9502;
	
	thisclient->Inventory->LoadItemDump((byte*)row[13]);
	thisclient->Equipment->LoadItemDump((byte*)row[14]);

	if(thisclient->accesslevel < 1){
		Log(MSG_DEBUG, "thisclient->accesslevel < 1");
		thisclient->id = -1;
		goto authFail;
	}

	{	// Char Info
		CPacket pakout(0x1038);
			pakout << (dword)thisclient->charid;
			pakout << FixLenStr(thisclient->charname, 0x10);
			pakout << thisclient->lastslot; // Slot
			pakout << thisclient->level; // Level
			pakout << qword(0x00); // Total Exp
			pakout << dword(0x00); // Unk - Doesn't appear to change anything
			pakout << word(0x000f); // HP Stones
			pakout << word(0x000b); // SP Stones
			pakout << dword(0x002E); // HP
			pakout << dword(0x0020); // SP
			pakout << dword(0x0000); // Fame
			pakout << qword(0x1d1a); // Money
			pakout.AddFixLenStr(row[9], 0x0C); // Cur map
			pakout << dword(thisclient->curY); // Y
			pakout << dword(thisclient->curX); // X
			pakout << byte(0x5a); // Starting Rotation
			pakout << byte(0x01); // STR+
			pakout << byte(0x02); // END+
			pakout << byte(0x03); // DEX+
			pakout << byte(0x04); // INT+
			pakout << byte(0x05); // SPR+

			//  If you see anything ingame that matches any of these numbers, update this
			pakout << byte(0x06);
			pakout << byte(0x07);
			pakout << dword(0x00); // Kill Points
			pakout << byte(0x0c);
			pakout << byte(0x0d);
			pakout << byte(0x0e);
			pakout << byte(0x0f);
			pakout << byte(0x10);
			pakout << byte(0x11);
			pakout << byte(0x12);
		SendPacket(thisclient, &pakout);
	}

	{	// Look info
		CPacket pakout(0x1039);
			pakout << byte(0x01 | (thisclient->profession << 2) | (atoi(row[8]) << 7)); // Class
			pakout << byte(atoi(row[10])); // Hair
			pakout << byte(atoi(row[11])); // Hair Color
			pakout << byte(atoi(row[12])); // Face
		SendPacket(thisclient, &pakout);
	}

	{	// Quests?
		CPacket pakout(0x103a);
			pakout << (dword)thisclient->charid;
			pakout << byte(0x01);
			pakout << byte(0x00); // Count
		SendPacket(thisclient, &pakout);
	}

	{	// Basic Action? <-- hell no.
		CPacket pakout(0x103b);
			pakout << (dword)thisclient->charid;
			pakout << word(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Active Skill list
		CPacket pakout(0x103d);
		pakout << byte(0x00); // Unk
		pakout << (dword)thisclient->charid; // Char ID
		pakout << word(0x01); // Num of skills
		{ // For num of skills
			pakout << word(0x18D8); // Skill ID [Inferno01]
			pakout << word(0x0000); // unk
			pakout << word(0x0000); // Unk
			pakout << word(0x5432); // Empowerment (dmg | (sp << 4) | (time << 8) | (cool << 12)
			pakout << word(0x000F); // Uses
			pakout << word(0x0000); // Unk
		}
		SendPacket(thisclient, &pakout);
	}

	{	// Passive skill list
		CPacket pakout(0x103e);
			pakout << word(0x00);
		SendPacket(thisclient, &pakout);
	}

	{	// Inventory
		CPacket pakout(0x1047);
		word DumpSize = 0;
		byte* ItemDump = thisclient->Inventory->DumpItems(DumpSize);
		pakout.AddBytes(ItemDump, DumpSize);
		thisclient->Inventory->FreeDump(ItemDump);
		SendPacket(thisclient, &pakout);
	}

	{	// Equips
		
		CPacket pakout(0x1047);
		word DumpSize = 0;
		byte* ItemDump = thisclient->Equipment->DumpItems(DumpSize);
		pakout.AddBytes(ItemDump, DumpSize);
		thisclient->Equipment->FreeDump(ItemDump);
		SendPacket(thisclient, &pakout);
	}

	{	// House
		CPacket pakout(0x1047);
			pakout << byte(0x01); // Count
			pakout << byte(0x0C); // Type
			{ // For Count
				pakout << byte(0x08); // Data Length
				pakout << byte(0x00); // Slot
				pakout << byte(0xC0); // State?
				pakout << word(31170); // ItemID [Liberty House]
				pakout << byte(0x01); // ?
				pakout << byte(0x02); // ?
				pakout << byte(0x03); // ?
				pakout << byte(0x04); // ?
			}
		SendPacket(thisclient, &pakout);
	}

	{	// Titles
		CPacket pakout(0x1049);
			pakout << byte(0x0d);//Current Title
			pakout << byte(0x03);//Current Title#
			pakout << word(0x0000);//u wut
			pakout << word(0x0001); // Count
			pakout << word(0xC30D); //titleID | level << 8 | 0x8000 | 0x4000 (if not visible)
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x104A);
			pakout << word(0x00);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1048);
			pakout << word(0xFF);
		SendPacket(thisclient, &pakout);
	}

	{
		CPacket pakout(0x1802);
			pakout << word(thisclient->clientid);
			pakout.AddBytes((byte*)packet1802, 236);
		SendPacket(thisclient, &pakout);
	}

	db->QFree(result);
	return true;
authFail:
	{
		CPacket pakout(0x1804);
		pakout << word(0x146);
		pakout << byte(0x12);
		SendPacket(thisclient, &pakout);
	}
	if (result != NULL)
	db->QFree(result);
	return true;
}

void CGameServer::OnClientDisconnect(CTitanClient* baseclient) {
	CGameClient* thisclient = (CGameClient*)baseclient;
	Log(MSG_INFO, "Saving Inventory");

	word InvSize = 0;
	word EquipSize = 0;
	byte* InvDump = thisclient->Inventory->DumpItems(InvSize);
	byte* EquipDump = thisclient->Equipment->DumpItems(EquipSize);

	char* InvDump_Safe = db->MakeSQLSafe((string)InvDump, InvSize);
	char* EquipDump_Safe = db->MakeSQLSafe((string)EquipDump, EquipSize);
	db->ExecSQL("UPDATE `characters` SET `inventory` = '%s', `equipment` = '%s' WHERE `id` = %i", InvDump_Safe, EquipDump_Safe, thisclient->charid);

	thisclient->Inventory->FreeDump(InvDump);
	thisclient->Equipment->FreeDump(EquipDump);
	free(InvDump_Safe);
	free(EquipDump_Safe);

	// Hide player entity.
	CPacket pakout(0x1c0e);
	pakout.Add<word>(thisclient->clientid);
	for (word i = 0; i < ClientList.size(); i++)
		SendPacket(ClientList.at(i), &pakout);
	
	// Free memory used by inventory/equipment
	delete thisclient->Inventory;
	delete thisclient->Equipment;
}

void CGameServer::ReceivedISCPacket( CISCPacket* pak ){
	Log(MSG_INFO,"Received ISC Packet: Command: %04x Size: %04x", pak->Command(), pak->Size());
	switch(pak->Command()){
		case TITAN_ISC_SETISCID:
		{
			ServerData.iscid = pak->Read<word>();
		}
		break;
		case TITAN_ISC_IDENTIFY:
		case TITAN_ISC_REMOVE:
		case TITAN_ISC_UPDATEUSERCNT:
		case TITAN_ISC_SERVERLIST:
		break;
	}
}