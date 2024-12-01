module;
#include "include/json.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <vector>
#include <cstdlib>
#include <time.h>
#include <unordered_map>

export module MapScreen;

using namespace std;

import ScreenType;
import GameState;
import Resources;
import UI;

// This will get its own module. Each screen needs its own module.
export class MapScreen {
	public:
		/* constructor */
		MapScreen() {
			cout << "\nLoading Map Screen\n\n";
			mapType = MapType::World; /* TODO: once we get the MAP object from the DB (based on the id) we can read its attribute to get its MapType. */
			screenType = ScreenType::Map;
			id = 0;
			screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);
		}

		ScreenType getScreenType() {
			return screenType;
		}

		void run();

	private:
		MapType mapType;
		ScreenType screenType;
		int id;
		ScreenStruct screenToLoadStruct;
};

export void MapScreen::run() {
	GameState& gameState = GameState::getInstance();
	bool running = true;

	while (running) {
		cout << "\nLoaded Map Screen for a fraction of a second.\n";

		/* here we will load the actual MAP screen.
		* For now just a big checker board and a settings menu
		* Deal with handleEvents and resizing.
		* 
		* draw
		* create background texture
		* display map title (for now just id??? and maptype???)
		* handleEvent
		* checkMouseLocation
		* rebuildDisplay
		* drawPanel
		*/

		running = false;
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}