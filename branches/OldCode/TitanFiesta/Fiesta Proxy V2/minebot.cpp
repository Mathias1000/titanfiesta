#include "main.hpp"

bool mineBot = false;
int mineId = 5000;

struct PakMonster {
	word clientid;
	word monsterid;
	dword y;
	dword x;
};

std::vector<PakMonster*> oreList;

PakMonster* targetOre = NULL;

void RemoveOreFromList(PakMonster* ore){
	for(std::vector<PakMonster*>::const_iterator itr = oreList.begin(); itr != oreList.end(); ++itr){
		if(*itr == ore){
			oreList.erase(itr);
			delete ore;
			return;
		}
	}
}

PakMonster* FindClosestOre(){
	float closestDistance = 9999.0f;
	PakMonster* closest = NULL;
	for(std::vector<PakMonster*>::const_iterator itr = oreList.begin(); itr != oreList.end(); ++itr){
		PakMonster* curOre = *itr;

		float xDiff = float(curX - curOre->x);
		float yDiff = float(curY - curOre->y);

		float distance = sqrt((xDiff*xDiff) + (yDiff*yDiff));
		if(distance < closestDistance){
			closestDistance = distance;
			closest = curOre;
		}
	}
	return closest;
}

void StartMineClosest(CConnectClient* game){
	mineBot = true;
	targetOre = FindClosestOre();
	if(targetOre != NULL){
		//Move Packet!
		isMoving = true;
		{CPacket pakout(0x2019, outBuffer);
		pakout.Add<dword>(targetOre->y);
		pakout.Add<dword>(targetOre->x);
		pakout.Add<dword>(curY);
		pakout.Add<dword>(curX);
		pakout.SetBuffer();
		game->SendServerPacket(&pakout);}

		{CPacket pakout(0x201B, outBuffer);
		pakout.Add<dword>(targetOre->y);
		pakout.Add<dword>(targetOre->x);
		pakout.SetBuffer();
		game->SendClientPacket(&pakout);}
	}
}

void OnFinishMine(CPacket* pak, CConnectClient* game){
	if(!mineBot) return;
	if(targetOre == NULL) return;
	if(pak->Read<word>() == targetOre->clientid){
		RemoveOreFromList(targetOre);
		StartMineClosest(game);
	}
}

//Start Mining: 04 2d 20 0d 40

void OnStopMoving(CConnectClient* game){
	if(!mineBot) return;
	if(targetOre != NULL){
		CPacket pakout(0x202D, outBuffer);
		pakout.Add<word>(targetOre->clientid);
		pakout.SetBuffer();
		game->SendServerPacket(&pakout);
	}
}

void OnMonsterSpawn(CPacket* pak){
	word clientId = pak->Read<word>();
	word monsterId = pak->Read<word>();
	Log(MSG_DEBUG, "Monster id %d spawned", monsterId);
	if(monsterId == mineId){
		PakMonster* curOre = new PakMonster();
		curOre->clientid = clientId;
		curOre->monsterid = monsterId;
		curOre->y = pak->Read<dword>();
		curOre->x = pak->Read<dword>();
		oreList.push_back(curOre);
	}
}