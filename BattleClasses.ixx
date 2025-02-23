
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
		BattleStatus battleStatus, bool playerTurn)
		:
		id(id), playerCharacter(playerCharacter), npc(npc), mapSlug(mapSlug),
		battleStatus(battleStatus), playerTurn(playerTurn)
	{}

	bool switchTurn();

	Character& getPlayerCharacter() { return playerCharacter; }
	Character& getNpc() { return npc; }

	Point getDrawStartNpc() { return drawStartNpc; }
	Point getDrawStartPlayer() { return drawStartPlayer; }

	void setDrawStartNpc(Point point) { drawStartNpc = point; }
	void setDrawStartPlayer(Point point) { drawStartPlayer = point; }

	void setDrawPoints() {
		//npc.buildDrawLimbList();
		//playerCharacter.buildDrawLimbList();

		//playerCharacter.setTexture(playerCharacter.createAvatar());
		//npc.setTexture(npc.createAvatar());
	}

private:
	int id;
	Character playerCharacter;
	Character npc;
	string mapSlug;

	BattleStatus battleStatus;
	bool playerTurn;

	Point drawStartPlayer;
	Point drawStartNpc;
};



bool Battle::switchTurn() {
	playerTurn = !playerTurn;
	return playerTurn;
}