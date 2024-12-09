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
		Map(int mapWidth);

		vector<vector<Block>> getRows() { return rows; }


	private:
		vector<vector<Block>> rows;
		void floorize(int x, int y, int radius);
};


/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class MapScreen {
	public:
		/* 
		* constructor:
		* 
		* For now we are sending in the WIDTH.
		* Later we'll send in the ID of the database object
		* and/or the reference to a JSON file.
		*/
		MapScreen(int mapWidth): map(mapWidth) {
			cout << "\nLoading Map Screen\n\n";
			mapType = MapType::World; /* TODO: once we get the MAP object from the DB (based on the id) we can read its attribute to get its MapType. */
			screenType = ScreenType::Map;
			id = 0;
			screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

			UI& ui = UI::getInstance();

			hResolution = mapWidth; /* LATER user can update this to zoom in or out. */
			buildMapDisplay();
			createTitleTexture(ui);
		}

		/* Destructor */
		~MapScreen() {
			SDL_DestroyTexture(floorTexture);
			SDL_DestroyTexture(wallTexture);
			SDL_DestroyTexture(titleTexture);
		}

		ScreenType getScreenType() { return screenType; }
		MapType getMapType() { return mapType; }
		void run();

	private:
		MapType mapType;
		ScreenType screenType;
		int id;
		ScreenStruct screenToLoadStruct;
		void drawMap(UI& ui);
		void draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel);
		void drawPanel(UI& ui, Panel& panel);

		void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState);
		void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel);

		void buildMapDisplay();
		void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);

		void buildMap();

		int hResolution; /* Horizontal Resolution of the map ( # of blocks across the top) */
		int blockWidth; /* depends on mapResolution */
		int vBlocksVisible;

		Map map;

		SDL_Texture* floorTexture = NULL;
		SDL_Texture* wallTexture = NULL;

		SDL_Texture* getWallTexture() { return wallTexture; }
		SDL_Texture* getFloorTexture() { return floorTexture; }

		void createTitleTexture(UI& ui);

		SDL_Texture* titleTexture;
		SDL_Rect titleRect;

		/* still need looted wall texture, looted floor texture, character texture (this actually will be in character object).
		* The NPCs (in a vactor) will each have their own textures, and x/y locations.
		*/

};

/* Create the texture with the name of the game */
void MapScreen::createTitleTexture(UI& ui) {
	Resources& resources = Resources::getInstance();
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture("Map!");
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}

void MapScreen::buildMapDisplay() {
	UI& ui = UI::getInstance();
	SDL_Surface* mainSurface = ui.getWindowSurface();

	blockWidth = (mainSurface->w / hResolution);
	vBlocksVisible = (mainSurface->h / blockWidth) + 1; /* adding one to fill any gap on the bottom. (Will always add 1 to hResolution when drawing) */

	/* make wall and floor textures (move into function(s) later) */

	unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();

	SDL_Surface* wallSurface = IMG_Load("assets/wall.png");
	SDL_Surface* floorSurface = IMG_Load("assets/floor.png");

	wallTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface);
	floorTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface);

	if (!floorTexture || !wallTexture) {
		cout << "\n\n ERROR! \n\n";
	}

	SDL_FreeSurface(wallSurface);
	SDL_FreeSurface(floorSurface);
}


export void MapScreen::run() {
	/* singletons */
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();
	/* panels */
	Panel settingsPanel = ui.createSettingsPanel(ScreenType::Map);
	Panel gameMenuPanel = ui.createGameMenuPanel();
	settingsPanel.setShow(false);
	gameMenuPanel.setShow(true);

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
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				handleEvent(e, running, settingsPanel, gameMenuPanel, gameState);
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
		*/

		checkMouseLocation(e, settingsPanel, gameMenuPanel);
		draw(ui, settingsPanel, gameMenuPanel);

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

void MapScreen::draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel) {
	unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	//SDL_SetRenderDrawColor(ui.getMainRenderer(), 140, 140, 140, 1);
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	drawMap(ui);

	/* draw the title */
	SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);

	drawPanel(ui, settingsPanel);
	drawPanel(ui, gameMenuPanel);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
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

			Block block = blocks[x];
			targetRect.x = x * blockWidth;
			targetRect.y = y * blockWidth;

			SDL_RenderCopyEx(
				ui.getMainRenderer(),
				block.getIsFloor() ? getFloorTexture() : getWallTexture(),
				NULL, &targetRect,
				0, NULL, SDL_FLIP_NONE);
		}
	}
	
}


void MapScreen::drawPanel(UI& ui, Panel& panel) {
	if (!panel.getShow()) { return; }
	for (Button button : panel.getButtons()) {
		/* get the rect, send it a reference(to be converted to a pointer) */
		SDL_Rect rect = button.getRect();

		/* now draw the button texture */
		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			button.isMouseOver() ? button.getHoverTexture() : button.getNormalTexture(),
			NULL, &rect,
			0, NULL, SDL_FLIP_NONE
		);
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

	/*			DEVELOPMENT STAGES:
	*	1. Build a map with ALL WALLS
	*	2. Draw a PATH through those walls
	*	3. Navigate around the map with arrows (no character)
	*	4. Put a "character" (struct) in the map and navigate around like a maze
	*	5. When you reach the other end you go back to the other screen
	*	6. Save the generated screen to a database.
	*	7. Read map paramaters from a JSON
	*	8. Delete map entirely from database
	*/

	int mapWidth = 100;
	map = Map(mapWidth);
}


/* Map class constructor */
Map::Map(int mapWidth) {
	/*
	* When loading from DB we will not care about MapScreen's resolution.
	* This will be raw map data from the DB.
	* So our numbers of rows and blocks will be from the DB.
	*
	* FOR NOW we want hardcoded numbers for temporary display purposes.
	* 
	* Right now this makes insane maps.
	* We will tweak it to make more sensible maps when we have the JSON ready.
	* 
	* BUT FIRST: Navigation.
	*/

	rows = vector<vector<Block>>(mapWidth);

	/* replace with reading from DB */
	for (int i = 0; i < rows.size(); ++i) {

		vector<Block> blocks(mapWidth);

		for (int k = 0; k < blocks.size(); ++k) {
			Block block = Block(false);
			blocks[k] = block;
		}

		rows[i] = blocks;
	}

	cout << "\n\n Map is made! \n\n";

	// Now make the PATH
	// get a random x starting point, but the y will be mapWidth - 1

	int pathX = (rand() % (mapWidth - 10)) + 5;
	int pathY = rows.size() - 1;

	Block& startingBlock = rows[pathY][pathX];
	startingBlock.setIsFloor(true);

	// make the path

	// MOVE THIS ENUM somewhere (maybe just at the top of the file?)
	enum Direction { Up, Down, Left, Right, Total };

	/* 
	*  create smaller paths so we aren't moving around totally randomly.
* 	*/
	struct SubPath {
		int seed;
		Direction direction;
		int radius;

		SubPath(int iSeed, Direction iDirection, int iRadius) {
			direction = iDirection;
			seed = iSeed;
			radius = iRadius;
		}
	};

	SubPath subPath = SubPath(
		(rand() % 5) + 2,
		Direction::Up,
		(rand() % 3) +1
	);

	int directionInt = Direction::Up;

	/* while loop makes the path */
	while (pathY > 0) {		

		/* choose the next block to floorize */
		switch (subPath.direction) {
		case Direction::Up:
			if (pathY > 0) { /* We ARE allowed to hit the ceiling (FOR NOW this ends the pathmaking) */
				--pathY;
			}
			else {
				++pathY;
				subPath.seed = 0;
			}
			
			break;
		case Direction::Down:
			if (pathY < rows.size() - 2) { /* We are NOT allowed to hit the bottom again. */
				++pathY;
			}
			else {
				--pathY;
				subPath.seed = 0;
			}
			break;
		case Direction::Left:
			if (pathX > 3) { /* We are NOT allowed to hit the left wall. */
				--pathX;
			}
			else {
				++pathX;
				subPath.seed = 0;
			}
			break;
		case Direction::Right:
			if (pathX < rows[pathY].size() - 2) { /* We are NOT allowed to hit the right wall. */
				++pathX;
			}
			else {
				--pathX;
				subPath.seed = 0;
			}
			
			break;
		}

		cout << "pathY is: " << pathY << "\n";

		floorize(pathX, pathY, subPath.radius);

		--subPath.seed;

		if (subPath.seed < 1) {
			subPath.direction = static_cast<Direction>(rand() % Direction::Total);
			subPath.seed = (rand() % 12) + 1;
			subPath.radius = rand() % 4;
		}
	}
}

/*
* When we create a path we want to clear a radius around each block of the central path.
* There's the opportunity here to draw an actual "path" block (different than a floor block... maybe non-diggable?).
* But only certain paths are *actual* paths... many are just normal floor blocks.
*/
void Map::floorize(int x, int y, int radius) {
	if (rows[y][x].getIsFloor()) { return; }
	rows[y][x].setIsFloor(true);

	/* increment counters (to help reach radius) */
	int upInc = 0;
	int leftInc = 0;
	int downInc = 0;
	int rightInc = 0;

	/* clear ABOVE */
	while (y - upInc >= 0 && upInc < radius) {

		/* directly above */
		rows[y - upInc][x].setIsFloor(true);

		/* up and to the left */
		while (x - leftInc > 0 && leftInc < radius) {
			rows[y - upInc][x - leftInc].setIsFloor(true);
			++leftInc;
		}
		leftInc = 0;

		/* up and to the right */
		while (x + rightInc < rows[y - upInc].size() - 2 && rightInc < radius) {
			rows[y - upInc][x + rightInc].setIsFloor(true);
			++rightInc;
		}
		rightInc = 0;

		++upInc;
	}

	/* reset increment counters */
	upInc = 0;
	leftInc = 0;
	downInc = 0;
	rightInc = 0;

	/* Clear BELOW */
	while (y + downInc < rows.size() - 2 && downInc < radius) {

		/* directly below */
		rows[y + downInc][x].setIsFloor(true);

		/* down and to the left */
		while (x - leftInc > 0 && leftInc < radius) {
			rows[y + downInc][x - leftInc].setIsFloor(true);
			++leftInc;
		}
		leftInc = 0;

		/* up and to the right */
		while (x + rightInc < rows[y - upInc].size() - 2 && rightInc < radius) {
			rows[y + downInc][x + rightInc].setIsFloor(true);
			++rightInc;
		}
		rightInc = 0;

		++downInc;
	}

	/* reset increment counters */
	upInc = 0;
	leftInc = 0;

	/* Clear LEFT */
	while (x - leftInc > 0 && leftInc < radius) {
		rows[y][x - leftInc].setIsFloor(true);
		++leftInc;
	}

	/* Clear RIGHT */
	while (x + rightInc < rows[y - upInc].size() - 2 && rightInc < radius) {
		rows[y][x + rightInc].setIsFloor(true);
		++rightInc;
	}
}


/* Screen has been resized. Rebuild! */
void MapScreen::rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	ui.rebuildGameMenuPanel(gameMenuPanel);
	buildMapDisplay();
	createTitleTexture(ui);
}


/* Process user input */
void MapScreen::handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState) {
	/* User pressed X to close */
	if (e.type == SDL_QUIT) {
		cout << "\nQUIT\n";
		running = false;
		return;
	}
	else {
		// user clicked
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			cout << "user clicked mouse\n";
			// These events might change the value of screenToLoad
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (settingsPanel.getShow() && settingsPanel.isInPanel(mouseX, mouseY)) {

				/* panel has a function to return which ButtonOption was clicked, and an ID(in the ButtonClickStruct). */
				ButtonClickStruct clickStruct = settingsPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
					case ButtonOption::Mobile:
						ui.resizeWindow(WindowResType::Mobile);
						rebuildDisplay(settingsPanel, gameMenuPanel);
						break;
					case ButtonOption::Tablet:
						ui.resizeWindow(WindowResType::Tablet);
						rebuildDisplay(settingsPanel, gameMenuPanel);
						break;
					case ButtonOption::Desktop:
						ui.resizeWindow(WindowResType::Desktop);
						rebuildDisplay(settingsPanel, gameMenuPanel);
						break;
					case ButtonOption::Fullscreen:
						ui.resizeWindow(WindowResType::Fullscreen);
						rebuildDisplay(settingsPanel, gameMenuPanel);
						break;
					case ButtonOption::Back:
						// switch to other panel
						settingsPanel.setShow(false);
						gameMenuPanel.setShow(true);
						break;
					case ButtonOption::Exit:
						/* back to menu screen */
						running = false;
						break;
					default:
						cout << "ERROR\n";
				}
			}
			else if (gameMenuPanel.getShow() && gameMenuPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK MAP MENU \n\n";
				ButtonClickStruct clickStruct = gameMenuPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::MapOptions:
					cout << "\nMAP OPTIONS\n";
					break;
				case ButtonOption::Settings:
					settingsPanel.setShow(true);
					gameMenuPanel.setShow(false);
					break;
				default:
					cout << "ERROR\n";

				}
			}
		}
	}
}

void MapScreen::checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (gameMenuPanel.getShow()) { gameMenuPanel.checkMouseOver(mouseX, mouseY); }
}