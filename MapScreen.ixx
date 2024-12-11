/*
* GAME TIME:
*	1.	Create an ENTRANCE and an EXIT.
*			We check landmarks on each CHANGE of user location / animate/move.
*	2.	Paint a "character" on the ENTRANCE at game start.
*	3.	When the character reaches the exit, lead back to the main loop.
* 
* 
* The MAP has a vector of LANDMARK objects.
* We'll create an ENTRANCE and EXIT... but we need some way to access them explicitly
* 
* 
* Buildings will turn their underlying Block objects into WALL objects... EXCEPT for the entrance.
* So when a player is ON a building block, they "enter" the building (load the building map)
* But this will only be possible on the entrance block, because it's the only FLOOR.
* 
* 
* MAP vs MAP SCREEN
* 
* The MAP contains grid data about the map itself, its landmarks, and characters and limbs on the map.
* None of this should be affected by changes to the display (zoom in/out, or which location we're viewing).
* 
* The MAP SCREEN controls the user's interaction with the map, and our view onto the map.
* 
* 
* 
* NEXT:
* -- START the game with the player character on the screen.
* -- "move" buttons actually move the player primarily.
* ---- THEN we re-calculate where the drawStartX/drawStartY should be
* ---- The player SHOULD be in the center... so:
*			--- we WANT to draw (hRes / 2) and (yRes / 2) from the character
*			--- BUT, if the character is less than (hRes / 2) or (yRes / 2) FROM either edge,
*					then we must recalculate...
*/

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

enum class LandmarkType { Entrance, Exit, Building, Shrine };
enum Direction { Up, Down, Left, Right, Total };
enum CharacterType { Player, Hostile, Friendly };

class Character {
	public:
		SDL_Texture* getTexture() { return texture; }
		int getBlockX() { return blockX; }
		int getBlockY() { return blockY; }
		int getType() { return characterType; }

		Character() { }

		Character(CharacterType characterType, SDL_Texture* texture, int x, int y) :
			blockX(x), blockY(y), texture(texture), characterType(characterType) { }

		~Character() { }

		bool move(Direction direction) {
			bool moved = false;
			/* This will become more complicated when we do animations. */
			/* Checking for obstacles must be done by MapScreen object.
			* When this is called, we follow blindly.
			*/

			switch(direction) {
			case Direction:: Up:
				--blockY;
				moved = true;
				break;
			case Direction::Down:
				++blockY;
				moved = true;
				break;
			case Direction::Left:
				--blockX;
				moved = true;
				break;
			case Direction::Right:
				++blockX;
				moved = true;
				break;
			}
			return moved;
		}

	private:
		SDL_Texture* texture;
		int blockX;
		int blockY;
		CharacterType characterType;
};

/*
* SubPath helps draw paths of floors through the blockmap.
* Create smaller sub-paths so we aren't moving around totally randomly.
*/
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

class Landmark {
	public:
		/* constructor */
		Landmark(int iX, int iY, SDL_Texture* iTexture, LandmarkType iLandmarkType) : x(iX), y(iY), texture(iTexture), landmarkType(iLandmarkType) {
			if (landmarkType == LandmarkType::Exit) {
				cout << "creating EXIT LANDMARK\n\n"; }
			else if (landmarkType == LandmarkType::Entrance) {
				cout << "creating ENTRANCE LANDMARK\n\n"; }
		}

		/* destructor */
		~Landmark() { }

		int getX() { return x; }
		int getY() { return y; }
		SDL_Texture* getTexture() { return texture; }

	private:
		/* x and y refer to the block grid, not the pixels */
		int x;
		int y;
		SDL_Texture* texture;
		LandmarkType landmarkType;
};

class Block {
	public:
		/* constructor */
		Block(bool isFloor = true)
			: isFloor(isFloor) { }

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
			* 
			* Maybe we need an isPath boolean, to draw a different kind of floor.
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
		vector<vector<Block>>& getRows() { return rows; }
		vector<Landmark>& getLandmarks() { return landmarks; }
		Character& getPlayerCharacter() { return playerCharacter; }

	private:
		vector<vector<Block>> rows;
		void floorize(int x, int y, int radius);
		vector<Landmark> landmarks;
		void buildMap(int mapWidth);
		vector<Character> NPCs;
		Character playerCharacter;
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
		* mapWidth refers to the number of blocks to CREATE.
		*/
		MapScreen(int mapWidth): map(mapWidth) {
			mapType = MapType::World; /* TODO: once we get the MAP object from the DB (based on the id) we can read its attribute to get its MapType. */
			screenType = ScreenType::Map;
			id = 0;
			screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

			UI& ui = UI::getInstance();
			setViewResAndBlockWidth(ui);
			setMaxDrawBlock();
			setDrawStartBlock();
			buildMapDisplay();
			createTitleTexture(ui);
		}

		/* Destructor */
		~MapScreen() {
			SDL_DestroyTexture(floorTexture);
			SDL_DestroyTexture(wallTexture);
			SDL_DestroyTexture(titleTexture);

			/* Destroy all the textures in the Landmarks */
			for (int i = 0; i < map.getLandmarks().size(); i++) {
				SDL_Texture* textureToDestroy = map.getLandmarks()[i].getTexture();
				if (textureToDestroy) { SDL_DestroyTexture(textureToDestroy); }				
			}

			/* Destroy all the textures in the Characters */

			SDL_DestroyTexture(map.getPlayerCharacter().getTexture());
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
		void drawLandmarks(UI& ui);
		void drawCharacters(UI& ui);
		void draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel);
		void drawPanel(UI& ui, Panel& panel);

		void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState);
		void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel);

		void buildMapDisplay();
		void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);
		void setDrawStartBlock();
		void setMaxDrawBlock();
		void setViewResAndBlockWidth(UI& ui);

		int hViewRes; /* Horizontal Resolution of the screen ( # of blocks displayed across the top) */
		int yViewRes; /* Vertical Resolution of the screen ( # of vertical blocks, depends on hViewRes) */
		int blockWidth; /* actual pixel dimensions of the block. depends on horizontal resolution */
		int vBlocksVisible;

		Map map;

		SDL_Texture* floorTexture = NULL;
		SDL_Texture* wallTexture = NULL;

		SDL_Texture* getWallTexture() { return wallTexture; }
		SDL_Texture* getFloorTexture() { return floorTexture; }

		void createTitleTexture(UI& ui);

		SDL_Texture* titleTexture;
		SDL_Rect titleRect;

		void requestUp();
		void requestDown();
		void requestLeft();
		void requestRight();

		int drawStartX = 0;
		int drawStartY = 0;

		int maxDrawStartX = 0;
		int maxDrawStartY = 0;

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

	setViewResAndBlockWidth(ui);
	setMaxDrawBlock();
	setDrawStartBlock();

	SDL_Surface* mainSurface = ui.getWindowSurface();

	vBlocksVisible = (mainSurface->h / blockWidth) + 1; /* adding one to fill any gap on the bottom. (Will always add 1 to hViewRes when drawing) */

	/* make wall and floor textures (move into function(s) later) */

	unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();

	SDL_Surface* wallSurface = IMG_Load("assets/wall.png");
	SDL_Surface* floorSurface = IMG_Load("assets/floor.png");

	wallTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface);
	floorTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface);

	if (!floorTexture || !wallTexture) { cout << "\n\n ERROR! \n\n"; }

	SDL_FreeSurface(wallSurface);
	SDL_FreeSurface(floorSurface);

	setMaxDrawBlock();
}

/* get the maximum allowed map position of top left block on-screen. */
void MapScreen::setMaxDrawBlock() {
	maxDrawStartX = map.getRows().size() - hViewRes;
	maxDrawStartY = map.getRows().size() - yViewRes;
}


/* set when screen loads or resizes */
void MapScreen::setViewResAndBlockWidth(UI& ui) {
	hViewRes = 20; /* LATER user can update this to zoom in or out. Function to update must also updated yViewRes */

	/* get and set y resolution... must be updated whenever hViewRes is updated. PUT THIS IN FUNCTION LATER. */
	blockWidth = ui.getWindowWidth() / hViewRes;
	yViewRes = (ui.getWindowHeight() / blockWidth) + 1;
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
			if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_KEYDOWN) {
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
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	drawMap(ui);
	drawLandmarks(ui);
	drawCharacters(ui);

	/* draw the title */
	SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);

	drawPanel(ui, settingsPanel);
	drawPanel(ui, gameMenuPanel);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}

void MapScreen::drawLandmarks(UI& ui) {
	/* NOW draw all LANDMARKS */
	/* This is SIMPLISTIC FOR NOW. Real buildings come LATER. (new algorithms for each building TYPE!! */

	SDL_Rect targetRect = { 0, 0, blockWidth, blockWidth };
	vector<Landmark>& landmarks = map.getLandmarks();
	int lX = 0;
	int lY = 0;

	/* Draw any landmarks that are in bounds */
	for (int i = 0; i < landmarks.size(); i++) {
		Landmark& landmark = landmarks[i];
		lX = landmark.getX();
		lY = landmark.getY();

		if (
			lX >= drawStartX &&
			lX <= (drawStartX + hViewRes) &&
			lY >= drawStartY &&
			lY <= (drawStartY + yViewRes)
		) {
			targetRect.x = (landmark.getX() - drawStartX) * blockWidth;
			targetRect.y = (landmark.getY() - drawStartY) * blockWidth;

			SDL_RenderCopyEx(
				ui.getMainRenderer(),
				landmark.getTexture(),
				NULL, &targetRect,
				0, NULL, SDL_FLIP_NONE);
		}
	}
}

void MapScreen::drawCharacters(UI& ui) {

	/* use this rect for all the characters. Just change the X Y each time. */
	SDL_Rect characterRect = { 0, 0, blockWidth, blockWidth };

	/* cycle through the characters list and draw them */

	/* ONLY draw the ones in the screen */

	/* Draw Player Character last */

	Character& playerCharacter = map.getPlayerCharacter();

	characterRect.x = (playerCharacter.getBlockX() - drawStartX) * blockWidth;
	characterRect.y = (playerCharacter.getBlockY() - drawStartY) * blockWidth;

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		playerCharacter.getTexture(),
		NULL, &characterRect,
		0, NULL, SDL_FLIP_NONE
	);
}

void MapScreen::drawMap(UI& ui) {
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

	vector<vector<Block>>& rows = map.getRows();
	SDL_Rect targetRect = { 0, 0, blockWidth, blockWidth };

	/* start with the TOP ROW that we want to draw, then work our way down */
	for (int y = drawStartY; y < drawStartY + yViewRes; ++y) {
		vector<Block>& blocks = rows[y];

		for (int x = drawStartX; x < drawStartX + hViewRes; ++x) {

			Block& block = blocks[x];
			targetRect.x = (x - drawStartX) * blockWidth;
			targetRect.y = (y - drawStartY) * blockWidth;

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


/*
* Sets the top left block for the camera.Cannot be less than 0,0.
* cannot be less than 0,0.
* cannot be more than end-of-list minus resolution.
*/
void MapScreen::setDrawStartBlock() {
	Character playerCharacter = map.getPlayerCharacter();
	int playerX = playerCharacter.getBlockX();
	int playerY = playerCharacter.getBlockY();

	/* get the IDEAL position for the camera (with the player in the center) */
	int idealX = playerX - (hViewRes / 2);
	int idealY = playerY - (yViewRes / 2);

	drawStartX = idealX >= 0 && idealX <= maxDrawStartX ? idealX : idealX > maxDrawStartX ? maxDrawStartX : 0;
	drawStartY = idealY >= 0 && idealY <= maxDrawStartY ? idealY : idealY > maxDrawStartY ? maxDrawStartY : 0;
}


void Map::buildMap(int mapWidth) {
	/* 
	* This WILL get the DB object and build the entire map as data.
	* To draw the map will mean to review this data and draw the local blocks.
	* For now just make a checkerboard grid of blocks.
	* 
	* Might not be a member of the MapScreen object.
	* Might instead be a member of the Map object.
	* 
	* 
	*		DEVELOPMENT STAGES:
	*	1. Build a map with ALL WALLS
	*	2. Draw a PATH through those walls
	*	3. Navigate around the map with arrows (no character)
	*	4. Put a "character" (struct) in the map and navigate around like a maze
	*	5. When you reach the other end you go back to the other screen
	*	6. Save the generated screen to a database.
	*	7. Read map paramaters from a JSON
	*	8. Delete map entirely from database
	*/

	UI& ui = UI::getInstance();
	rows = vector<vector<Block>>(mapWidth);

	/* replace with reading from DB */
	for (int i = 0; i < rows.size(); ++i) {
		vector<Block> blocks(mapWidth);

		for (int k = 0; k < blocks.size(); ++k) {
			blocks[k] = Block(false); }
		rows[i] = blocks;
	}

	/* Now make the PATH */
	/* get a random x starting point, but the y will be map's height - 2 */

	int pathX = (rand() % (mapWidth - 10)) + 5;
	int pathY = static_cast<int>(rows.size()) - 2;

	int playerX = pathX;
	int playerY = pathY;

	Block& startingBlock = rows[pathY][pathX];
	startingBlock.setIsFloor(true);

	/* make the path */

	SubPath subPath = SubPath(
		(rand() % 5) + 2,
		Direction::Up,
		(rand() % 3) + 1);

	/* 
	* getting the textures for entrance / exit
	* Store elsewhere later. Possible store a pointer in each block.
	*/

	SDL_Surface* gateSurface = IMG_Load("assets/ENTRANCE.png");
	/* Entrance landmark */
	landmarks.emplace_back(pathX, pathY, SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface), LandmarkType::Entrance);

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

		floorize(pathX, pathY, subPath.radius);
		--subPath.seed;

		if (subPath.seed < 1) {
			/* refresh seed */
			subPath.direction = static_cast<Direction>(rand() % Direction::Total);
			subPath.seed = (rand() % 12) + 1;
			subPath.radius = rand() % 4;
		}
	}

	/* Exit landmark */
	landmarks.emplace_back(pathX, pathY, SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface), LandmarkType::Exit);
	SDL_FreeSurface(gateSurface);

	/* create Player Character */

	/* get character texture */
	SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
	SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
	playerCharacter = Character(CharacterType::Player, characterTexture, playerX, playerY);
	SDL_FreeSurface(characterSurface);
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
	* 
	* 
	* NAVIGATION PLAN:
	* 
	* -- create a startDraw block to be the top left block.
	* ---- moving around changes both character location AND startDraw block
	* ---- when you reach either edge, player still moves but startDraw does NOT.
	* ---- HOW do we know if the player has moved away from the center?
	* ------ choose a centreBlock (which updates each time)
	* ------ if user is away from center block, don't move map until user is back on center block
	* -------- MORE:::: -->  if userX != centerX act accordingly (same with userY != centerY)... until user is back on centerBlock
	* 
	* 
	* 
	*/
	buildMap(mapWidth);

	// populate characters and limbs after building the map (and its landmarks).
}

/*
* When we create a path we want to clear a radius around each block of the central path.
* There's the opportunity here to draw an actual "path" block (different than a floor block... maybe non-diggable?).
* But only certain paths are *actual* paths... many are just normal floor blocks.
*/
void Map::floorize(int x, int y, int radius) {
	if (rows[y][x].getIsFloor()) { return; }
	rows[y][x].setIsFloor(true);
	if (y < (radius + 2)) { return; } /* don't clear top blocks (except exit block, which is already cleared) */

	/* increment counters (to help reach radius) */
	int upInc = 0;
	int leftInc = 0;
	int downInc = 0;
	int rightInc = 0;

	/* clear ABOVE */
	while (y - upInc > 0 && upInc < radius) {

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

/* NAVIGATION FUNCTIONS
*	We must check:
*		A)	if we are moving out of bounds. DONE
*		B)	if the player character has hit a wall.
*		C)	if the player character has hit a door, or another character.
* 
*		TODO:	Make a "hit a wall" function (to play a sound)
*				Do an animated transition for these moves (maybe).
*/

void MapScreen::requestUp() {
	map.getPlayerCharacter().move(Direction::Up);
	setDrawStartBlock();
}
void MapScreen::requestDown() {
	map.getPlayerCharacter().move(Direction::Down);
	setDrawStartBlock();
}
void MapScreen::requestLeft() {
	map.getPlayerCharacter().move(Direction::Left);
	setDrawStartBlock();
}
void MapScreen::requestRight() {
	map.getPlayerCharacter().move(Direction::Right);
	setDrawStartBlock();
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
		if (e.type == SDL_KEYDOWN) {
			switch (e.key.keysym.sym)
			{
			case SDLK_UP:
			case SDLK_w:
				requestUp();
				break;

			case SDLK_DOWN:
			case SDLK_s:
				requestDown();
				break;

			case SDLK_LEFT:
			case SDLK_a:
				requestLeft();
				break;

			case SDLK_RIGHT:
			case SDLK_d:
				requestRight();
				break;
			default:
				cout << e.key.keysym.sym << "\n";
			}
		}
		/*  user clicked */
		else if (e.type == SDL_MOUSEBUTTONDOWN) {
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
				cout << "\n\n CLICK MAP MENU \n\n";
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