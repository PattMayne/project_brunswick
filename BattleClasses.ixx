
/**
*  ____    _  _____ _____ _     _____
* | __ )  / \|_   _|_   _| |   | ____|
* |  _ \ / _ \ | |   | | | |   |  _|
* | |_) / ___ \| |   | | | |___| |___
* |____/_/   \_\_|_  |_|_|_____|_____|_ ____
*  / ___| |      / \  / ___/ ___|| ____/ ___|
* | |   | |     / _ \ \___ \___ \|  _| \___ \
* | |___| |___ / ___ \ ___) |__) | |___ ___) |
*  \____|_____/_/   \_\____/____/|_____|____/
* 
* 
* Battle classes
* We import Character classes. There's no need for a character object to hold a battle.
* 
*/


module;
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

export module BattleClasses;

import <stdio.h>;
import <string>;
import <iostream>;
import <vector>;
import <cstdlib>;
import <time.h>;
import <unordered_map>;

import FormFactory;
import TypeStorage;
import CharacterClasses;
import LimbFormMasterList;
import UI;

using namespace std;

export class Battle {
public:

	Battle() {
		id = -1;
	}

	/* Always recreate a Battle from the DB (never create one without an ID). */
	Battle(int id, Character playerCharacter, Character npc, string mapSlug,
		BattleStatus battleStatus)
		:
		id(id), playerCharacter(playerCharacter), npc(npc), mapSlug(mapSlug),
		battleStatus(battleStatus)
	{}

	BattleStatus switchTurn();
	BattleStatus& getBattleStatus() { return battleStatus; }

	Character& getPlayerCharacter() { return playerCharacter; }
	Character& getNpc() { return npc; }

	int getId() { return id; }
	Point getDrawStartNpc() { return drawStartNpc; }
	Point getDrawStartPlayer() { return drawStartPlayer; }
	bool isPlayerTurn() { return battleStatus == BattleStatus::PlayerTurn; }
	bool isNpcTurn() { return battleStatus == BattleStatus::NpcTurn; }

	void setDrawStartNpc(Point point) { drawStartNpc = point; }
	void setDrawStartPlayer(Point point) { drawStartPlayer = point; }
	void setBattleStatus(BattleStatus status) { battleStatus = status; }
	void setId(int battleId) { id = battleId; }


private:
	int id;
	Character playerCharacter;
	Character npc;
	string mapSlug;

	BattleStatus battleStatus;

	Point drawStartPlayer;
	Point drawStartNpc;
};



BattleStatus Battle::switchTurn() {

	if (playerCharacter.getNumberOfEquippableLimbs() < 1) {
		battleStatus = BattleStatus::PlayerDefeat;
	} else if (playerCharacter.getNumberOfEquippedLimbs() < 1) {
		battleStatus = BattleStatus::RebuildRequired;
	}
	else {
		if (battleStatus == BattleStatus::PlayerTurn) {
			battleStatus = BattleStatus::NpcTurn;
		}
		else if (battleStatus == BattleStatus::NpcTurn) {
			battleStatus = BattleStatus::PlayerTurn;
		}
	}

	return battleStatus;
}