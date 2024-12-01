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



class Block {
	public:
		/* constructor */
		Block(bool incomingIsFloor = true) {
			isFloor = incomingIsFloor;
		}

		/* getters */
		bool getIsFloor() { return isFloor; }
		bool getIsLooted() { return isLooted; }

		/* setters */
		void setIsFloor(bool incomingIsFloor) { isFloor = incomingIsFloor; }
		void loot() {
			isLooted = true;
			/*
			* When we have LIMB objects (imported from a different module)
			* this will return a vector of Limb objects.
			*
			* Also... maybe it won't return it... because you can destroy a wall from afar
			* and the Limb objects will scatter nearby.
			*/
		}

	private:
		bool isFloor;
		bool isLooted;
};


/* The Map object contains all the blocks from the DB. */
class Map {
	public:
		/* constructor */
		Map();

		vector<vector<Block>> getRows() { return rows; }


	private:
		vector<vector<Block>> rows;
};


/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class MapScreen {
	public:
		/* constructor */
		MapScreen() {
			cout << "\nLoading Map Screen\n\n";
			mapType = MapType::World; /* TODO: once we get the MAP object from the DB (based on the id) we can read its attribute to get its MapType. */
			screenType = ScreenType::Map;
			id = 0;
			screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

			hResolution = 10; /* LATER user can update this to zoom in or out. */

			UI& ui = UI::getInstance();
			SDL_Surface* mainSurface = ui.getWindowSurface();

			blockWidth = (mainSurface->w / hResolution);
			vBlocksVisible = (mainSurface->h / blockWidth) + 1; /* adding one to fill any gap on the bottom. (Will always add 1 to hResolution when drawing) */

			/* make wall and floor textures (move into function(s) later) */

			unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();

			SDL_Surface* wallSurface = IMG_Load("assets/door_400_400.png");
			SDL_Surface* floorSurface = IMG_Load("assets/open_door_400_400.png");

			wallTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface);
			floorTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface);

			if (!floorTexture || !wallTexture) {
				cout << "\n\n ERROR! \n\n";
			}

			SDL_FreeSurface(wallSurface);
			SDL_FreeSurface(floorSurface);

			map = Map();
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
		void drawMap(UI& ui);
		void draw(UI& ui, Panel& settingsPanel);

		void buildMap();

		int hResolution; /* Horizontal Resolution of the map ( # of blocks across the top) */
		int blockWidth; /* depends on mapResolution */
		int vBlocksVisible;

		Map map;

		SDL_Texture* floorTexture = NULL;
		SDL_Texture* wallTexture = NULL;

		SDL_Texture* getWallTexture() { return wallTexture; }
		SDL_Texture* getFloorTexture() { return floorTexture; }


		/* still need looted wall texture, looted floor texture, character texture (this actually will be in character object).
		* The NPCs (in a vactor) will each have their own textures, and x/y locations.
		*/

};

export void MapScreen::run() {
	/* singletons */
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();
	/* panels */
	Panel settingsPanel = ui.createSettingsPanel();
	settingsPanel.setShow(false);

	/*
	* PANELS TO COME:
	* * navigation panel
	* * small horizontal panel with buttons to toggle settings panel and nav panel
	* * mini-map
	* * stats
	*/

	/* Timeout data */
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	/* loop and event control */
	SDL_Event e;
	bool running = true;

	while (running) {
		/* Get the total running time(tick count) at the beginning of the frame, for the frame timeout at the end */
		frameStartTime = SDL_GetTicks();

		/* Check for events in queue, and handle them(really just checking for X close now */
		while (SDL_PollEvent(&e) != 0) {
			// handleEvent(e, running, menuPanel, settingsPanel, gameState);
			

			if (e.type == SDL_MOUSEBUTTONDOWN) {
				cout << "\n\nevent happened CLICK\n\n";
				running = false;
			}
		}

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

		draw(ui, settingsPanel);

		/* Delay so the app doesn't just crash */
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
		/* Delay loop */
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed);
		}
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}

void MapScreen::draw(UI& ui, Panel& settingsPanel) {
	unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	//SDL_SetRenderDrawColor(ui.getMainRenderer(), 140, 140, 140, 1);
	SDL_SetRenderDrawColor(ui.getMainRenderer(),
		colorsByFunction["BTN_TEXT"].r,
		colorsByFunction["BTN_TEXT"].g,
		colorsByFunction["BTN_TEXT"].b,
		1);


	SDL_RenderClear(ui.getMainRenderer());


	/* TEST DRAW */

	SDL_Rect targetRect = { 0,0, 55, 55 };

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		getWallTexture(),
		NULL, &targetRect,
		0, NULL, SDL_FLIP_NONE);


	SDL_Surface* bgImageRaw = IMG_Load("assets/door_400_400.png"); /* create BG surface*/
	if (bgImageRaw == NULL) {
		printf("Failed to load image: %s\n", IMG_GetError());
		return;
	}
	SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), bgImageRaw);
	SDL_FreeSurface(bgImageRaw);

	if (bgTexture == NULL) {
		printf("Failed to create texture: %s\n", SDL_GetError());
		return;
	}

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		bgTexture,
		NULL, NULL,
		0, NULL, SDL_FLIP_NONE);

	drawMap(ui);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
	SDL_DestroyTexture(bgTexture);
}


void MapScreen::drawMap(UI& ui) {
	// cout << "\n\nDrawing Map\n\n";

	/*
	* FOR EACH FRAME:
	* 
	* Keep textures for each block type.
	* Redraw them all each frame, but only the ones visible on-screen.
	* This holds less data in memory (better than holding the entire map).
	* So there is no TEXTURE of the whole MAP.
	* 
	* During transition animations, I can start drawing the blocks off-screen,
	* and only parts of them will be displayed. This will NOT give errors.
	*/

	/* FOR NOW just draw a checkerboard. */

	vector<vector<Block>> rows = map.getRows();

	//SDL_Surface* mainSurface = ui.getWindowSurface();
	SDL_Rect targetRect = { 0, 0, blockWidth, blockWidth };

	for (int y = 0; y < rows.size(); ++y) {
		vector<Block> blocks = rows[y];

		for (int x = 0; x < blocks.size(); ++x) {
			if (!getFloorTexture() || !getWallTexture()) {
				cout << "\n\n DRAWING ERROR \n\n";

			}else{
				cout << "\n\nDrawing bLOCK\n\n";
				cout << "\n\nBLOCK WIDTH: " << blockWidth << "\n\n";
			}

			Block block = blocks[x];
			
			targetRect.x = (x + 1) * blockWidth;
			targetRect.y = (y + 1) * blockWidth;

			SDL_RenderCopyEx(
				ui.getMainRenderer(),
				block.getIsFloor() ? getFloorTexture() : getWallTexture(),
				NULL, &targetRect,
				0, NULL, SDL_FLIP_NONE);
		}
	}
	
}


void MapScreen::buildMap() {
	/* 
	* This WILL get the DB object and build the entire map as data.
	* To draw the map will mean to review this data and draw the local blocks.
	* For now just make a checkerboard grid of blocks.
	* 
	* Might not be a member of the MapScreen object.
	* Might instead be a member of the Map object.
	*/

}

/* Map class constructor */
Map::Map() {
	/*
	* When loading from DB we will not care about MapScreen's resolution.
	* This will be raw map data from the DB.
	* So our numbers of rows and blocks will be from the DB.
	*
	* FOR NOW we want hardcoded numbers for temporary display purposes.
	*/

	/* replace with reading from DB */
	bool makeFloor = true;
	for (int i = 0; i < 11; ++i) {

		vector<Block> blocks;

		for (int k = 0; k < 11; ++k) {
			Block block = Block(makeFloor);
			blocks.push_back(block);
			makeFloor = !makeFloor;
		}

		rows.push_back(blocks);
	}

	cout << "\n\n Map is made! \n\n";
}