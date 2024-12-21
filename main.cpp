/*
*	Land of Limbs
*	Copyright 2024 Matthew Payne
*   <https://www.pattmayne.com>
*
*  .--..--..--..--..--..--..--..--..--..--..--..--..--.
* / .. \.. \.. \.. \.. \.. \.. \.. \.. \.. \.. \.. \.. \
* \ \/\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ \/ /
*  \/ /`--'`--'`--'`--'`--'`--'`--'`--'`--'`--'`--'\/ /
*  / /\                                            / /\
* / /\ \    _                    _          __    / /\ \
* \ \/ /   | |    __ _ _ __   __| |   ___  / _|   \ \/ /
*  \/ /    | |   / _` | '_ \ / _` |  / _ \| |_     \/ /
*  / /\    | |__| (_| | | | | (_| | | (_) |  _|    / /\
* / /\ \   |_____\__,_|_| |_|\__,_|  \___/|_|     / /\ \
* \ \/ /   | |   (_)_ __ ___ | |__  ___           \ \/ /
*  \/ /    | |   | | '_ ` _ \| '_ \/ __|           \/ /
*  / /\    | |___| | | | | | | |_) \__ \           / /\
* / /\ \   |_____|_|_| |_| |_|_.__/|___/          / /\ \
* \ \/ /                                          \ \/ /
*  \/ /                                            \/ /
*  / /\.--..--..--..--..--..--..--..--..--..--..--./ /\
* / /\ \.. \.. \.. \.. \.. \.. \.. \.. \.. \.. \.. \/\ \
* \ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `'\ `' /
*  `--'`--'`--'`--'`--'`--'`--'`--'`--'`--'`--'`--'`--'
*
* A puzzle/adventure/RPG game where you build a modular character from LIMBs.
*
*/

#include "include/json.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <vector>
#include <cstdlib>
#include <ctime>

import UI;
import GameState;
import MenuScreen;
import MapScreen;
import BattleScreen;
import CharacterCreationScreen;
import TypeStorage;
import Resources;
using namespace std;
const bool DEBUG = false;


/* declarations */
void exit(SDL_Surface* surface, SDL_Window* window);

/* Main game loop manages particular screens, which do actual game logic. */
int main(int argc, char* args[]) {
	cout << "Hello new world\n";

	/* seed the random number generator now for the whole game */
	srand((unsigned int)time(NULL));

	/* instantiate the UI instance, and hold the reference for eventual destruction in this file. */
	UI& ui = UI::getInstance();
	GameState& gameState = GameState::getInstance();
	
	ScreenStruct screenToLoadStruct = closingScreenStruct();
	bool running = true;
	SDL_Event e;

	/* Game loop checks gameState for screen to load, then loads that screen.
	* Each screen, once loaded, has its own game loop (where the real game logic happens).
	* When a screen is finished its job (or a user has chosen to move to a new screen),
	* the screen's "run" function returns a NEW screen to load.	*/
	while (running) {

		// Check for closing the app (clicking the (x) button)
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) { running = false; }
		}

		/* run the chosen screen and receive the next screen to load
		 * if the screenToLoad is Map or Battle, we will send in the ID to the Run function */

		if (gameState.getScreenType() == ScreenType::Menu) {
			cout << "\n\n MENU \n\n";
			MenuScreen menuScreen = MenuScreen();
			menuScreen.run();
		}
		else if (gameState.getScreenType() == ScreenType::Map) {
			cout << "\nselected MAP\n";
			MapScreen mapScreen = MapScreen("forest");
			mapScreen.run();
		}else if (gameState.getScreenType() == ScreenType::Battle) {
			cout << "\nselected BATTLE\n";
			BattleScreen battleScreen = BattleScreen();
			battleScreen.run();
		}else if (gameState.getScreenType() == ScreenType::CharacterCreation) {
			cout << "\nselected CHARACTER CREATION\n";
			CharacterCreationScreen characterCreationScreen = CharacterCreationScreen();
			characterCreationScreen.run();
		}

		/* check for closingScreen type and close.Otherwise we will load the NEW screen on the next loop */
		if (gameState.getScreenType() == ScreenType::NoScreen) {
			cout << "\n\n WHY NO SCREEN?? \n\n";
			running = false; }
	}

	exit(ui.getWindowSurface(), ui.getMainWindow());
	return 0;
}

/* Free SDL resources and quit */
void exit(SDL_Surface* surface, SDL_Window* window) {
	SDL_FreeSurface(surface);
	surface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}