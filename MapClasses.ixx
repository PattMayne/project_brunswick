/**
* 
*  __  __
* |  \/  | __ _ _ __
* | |\/| |/ _` | '_ \
* | |  | | (_| | |_) |
* |_|__|_|\__,_| .__/
*  / ___| | __ |_|__ ___  ___  ___
* | |   | |/ _` / __/ __|/ _ \/ __|
* | |___| | (_| \__ \__ \  __/\__ \
*  \____|_|\__,_|___/___/\___||___/
* 
* 
* The MAP class, and the BLOCK class whose objects that compose the map.
* 
*/

module;
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

export module MapClasses;

import <stdio.h>;
import <string>;
import <iostream>;
import <vector>;
import <cstdlib>;
import <time.h>;
import <unordered_map>;

import FormFactory;
import TypeStorage;
import GameState;
import Resources;
import CharacterClasses;
import LimbFormMasterList;
import UI;

using namespace std;


export enum MapDirection { Up, Down, Left, Right, Total }; /* NOT a CLASS because we want to use it as int. */

/* This is what a Landmark's checkCollision function will return. */
export struct LandmarkCollisionInfo {
	bool hasCollided;
	int subject;
	LandmarkType type;
	/* constructor */
	LandmarkCollisionInfo(bool iHasCollided, LandmarkType iType, int iSubject) :
		hasCollided(iHasCollided), subject(iSubject), type(iType) {
	}
};


/*
* SubPath helps draw paths of floors through the blockmap.
* Create smaller sub-paths so we aren't moving around totally randomly.
*/
export struct SubPath {
	int seed;
	MapDirection direction;
	int radius;

	SubPath(int iSeed, MapDirection iDirection, int iRadius) {
		direction = iDirection;
		seed = iSeed;
		radius = iRadius;
	}
};



/*
* Character will eventually go in its own module, along with Limb, for use in Character Creation and Battle screens.
* Some of the attributes are specific to the Map Screen. So we will have a Character Decorator in the Map Screen
* which will add the map-related members and functions to the extended object.
* Battle and Character Creation screens will have their own decorators, adding their own stuff.
*/
export class MapCharacter : public Character {
public:
	MapCharacter() : Character() {}

	MapCharacter(CharacterType characterType, SDL_Texture* texture, int x, int y) :
		Character(characterType), texture(texture), blockPosition(x, y), lastBlockPosition(x, y) {
	}

	~MapCharacter() {}

	int getBlockX() { return blockPosition.x; }
	int getBlockY() { return blockPosition.y; }

	int getLastX() { return lastBlockPosition.x; }
	int getLastY() { return lastBlockPosition.y; }
	Point getPosition() { return blockPosition; }
	Point getLastPosition() { return lastBlockPosition; }

	void setBlockPosition(Point blockPosition) {
		this->blockPosition = blockPosition;
	}

	SDL_Texture* getTexture() { return texture; } /* This must move to the parent class. */
	void setTexture(SDL_Texture* incomingTexture) {
		if (texture) {
			SDL_DestroyTexture(texture);
			texture = incomingTexture;
		}
	}

	void updateLastBlock() {
		lastBlockPosition.x = blockPosition.x;
		lastBlockPosition.y = blockPosition.y;
	}

	bool move(MapDirection direction, int distance = 1) {
		bool moved = false;
		/* This will become more complicated when we do animations. */
		/* Checking for obstacles must be done by MapScreen object.
		* When this is called, we follow blindly. */

		switch (direction) {
		case MapDirection::Up:
			updateLastBlock();
			blockPosition.y -= distance;
			moved = true;
			break;
		case MapDirection::Down:
			updateLastBlock();
			blockPosition.y += distance;
			moved = true;
			break;
		case MapDirection::Left:
			updateLastBlock();
			blockPosition.x -= distance;
			moved = true;
			break;
		case MapDirection::Right:
			updateLastBlock();
			blockPosition.x += distance;
			moved = true;
			break;
		}

		// cout << blockPosition.x << ", " << blockPosition.y << "\n";

		return moved;
	}

private:
	Point blockPosition;
	Point lastBlockPosition;
	SDL_Texture* texture;
};



/*
* How do we add FUNCTIONS to the Landmark objects?
* This may be where we need lambdas
*    [](){}
*		[variables brought in from parent scope](parameters called at runtime){ logic which possibly returns something }
*
* OR do we let MapScreen handle that, and just let the Landmark class detect collisions?
*
*/
export class Landmark {
public:
	/* constructor */
	Landmark(
		Point position,
		SDL_Texture* texture,
		LandmarkType landmarkType,
		int subjectId = -1
	) :
		texture(texture),
		landmarkType(landmarkType),
		subjectId(subjectId),
		position(position)
	{}

	/* destructor */
	~Landmark() { /* Texture is managed by MapScreen and will be destroyed at the end of the run() function. */ }

	int getDrawX() { return position.x; }
	int getDrawY() { return position.y; }
	SDL_Texture* getTexture() { return texture; }

	LandmarkCollisionInfo checkCollision(Point pos) { return checkCollision(pos.x, pos.y); }
	LandmarkCollisionInfo checkCollision(int x, int y) {
		if (x == position.x && y == position.y) {
			return { true, landmarkType, subjectId };
		}

		return { false, landmarkType, -1 };	}

private:
	/* Point refers to the block grid, not the pixels */
	Point position;
	SDL_Texture* texture;
	LandmarkType landmarkType;
	int subjectId; /* This can be either the MAP id or the SUIT slug??? Needs re-thinking! */
};

export Landmark getExitLandmark(Point position) {
	UI& ui = UI::getInstance();
	SDL_Surface* gateSurface = IMG_Load("assets/ENTRANCE.png");
	SDL_Texture* gateTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface);
	SDL_FreeSurface(gateSurface);
	return Landmark(position, gateTexture, LandmarkType::Exit);
}

export Landmark getEntranceLandmark(Point position) {
	UI& ui = UI::getInstance();
	SDL_Surface* gateSurface = IMG_Load("assets/ENTRANCE.png");
	SDL_Texture* gateTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface);
	SDL_FreeSurface(gateSurface);
	return Landmark(position, gateTexture, LandmarkType::Entrance);
}

export class Block {
public:
	/* constructor */
	Block(bool isFloor = true)
		: isFloor(isFloor), floorTextureIndex(0), isPath(false), pathFlipOption(0), pathRotateAngle(0), wallTextureIndex(0), pathTextureIndex(0) {
		loaded = false;
	}

	/* Rebuilding block from DB. */
	Block(int id, bool isFloor, bool isPath, bool isLooted) :
		id(id), isFloor(isFloor), isPath(isPath), isLooted(isLooted)
	{
		floorTextureIndex = 0;
		pathFlipOption = 0;
		wallTextureIndex = 0;
		pathTextureIndex = 0;
		pathRotateAngle = 0;
		loaded = true;
	}

	/* getters */
	bool getIsFloor() { return isFloor; }
	bool getIsLooted() { return isLooted; }
	bool getIsLoaded() { return loaded; }

	int getFloorTextureIndex() { return floorTextureIndex; }
	void setFloorTextureIndex(int index) { floorTextureIndex = index; }
	int getWallTextureIndex() { return wallTextureIndex; }
	bool getWallIsFlipped() { return wallIsFlipped; }
	bool getIsPath() { return isPath; }
	int getPathFlipOption() { return pathFlipOption; }
	int getPathRotateAngle() { return pathRotateAngle; }
	int getPathTextureIndex() { return pathTextureIndex; }
	int getId() { return id; }

	/* setters */
	void setPathRotateAngle(int angle = 0) { pathRotateAngle = angle; }
	void setPathFlipOption(int option = 0) { pathFlipOption = option; }
	void setIsFloor(bool incomingIsFloor) { isFloor = incomingIsFloor; }
	void setIsPath(bool incomingIsPath = true) { isPath = incomingIsPath; }
	void setWallTextureIndex(int index) { wallTextureIndex = index; }
	void setWallIsFlipped(bool flipWall) { wallIsFlipped = flipWall; }
	void setPathTextureIndex(int index) { pathTextureIndex = index; }
	void setId(int id) { this->id = id; }

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
	bool isPath;
	int pathFlipOption;
	int pathRotateAngle;
	int floorTextureIndex;
	int wallTextureIndex;
	int pathTextureIndex;
	bool wallIsFlipped;
	int id;
	bool loaded = false;
};


/* The Map object contains all the blocks from the DB. */
export class Map {
public:
	/* constructors */
	Map() {};
	Map(MapForm mapForm);
	Map(MapForm mapForm, vector<Limb> roamingLimbs, vector<vector<Block>> rows, Point characterPosition);
	vector<vector<Block>>& getRows() { return rows; }
	vector<Landmark>& getLandmarks() { return landmarks; }
	MapCharacter& getPlayerCharacter() { return playerCharacter; }
	SDL_Texture* getFloorTexture(int index) {
		return mapForm.floorTextures[index];
	}
	SDL_Texture* getWallTexture(int index) {
		return mapForm.wallTextures[index];
	}
	SDL_Texture* getPathTexture(int index) { return mapForm.pathTextures[index]; }
	vector<Limb>& getRoamingLimbs() { return roamingLimbs; }
	string getName() { return mapForm.name; }
	string getSlug() { return mapForm.slug; }
	MapLevel getMapLevel() { return mapForm.mapLevel; }

	MapForm getForm() { return mapForm; }
	void randomizePathOptions(Block& block);

private:
	MapForm mapForm;
	vector<vector<Block>> rows;
	void floorize(int x, int y, int radius);
	vector<Point> buildMap(MapForm mapForm);
	vector<MapCharacter> NPCs;
	MapCharacter playerCharacter;

	/* stuff sent in from MapData struct */

	vector<Landmark> landmarks;
	vector<LimbForm> nativeLimbForms;
	vector<Limb> roamingLimbs;

	/* Will need a list of NPCs */
	/* Will need a list of Roaming Limbs */
	/* Will need classes for both NPC and RoamingLimb (probably?) */
};



/*
*
*
*
*
*
*
*
*		Map Functions
*
*
*
*
*
*
*
*/


/* Map class constructor for brand new map. */
Map::Map(MapForm mapForm) : mapForm(mapForm) {
	/*
	* Currently we create a new map every time we load the Map Screen.
	* It is built on a half-complete definition of Map structs.
	* We still need Suits for Shrines, and other landmarks.
	* Then we will need to save the built maps into the database, so we can load instead of building fresh next time.
	*/

	/*
	* On the first draw, we must build the GRID based on the number of SUITS (therefore shrines) and landmarks.
	* But we must scatter the LIMBS across the available FLOOR tiles, which are only known after creating the grid.
	* So SUITS must be calculated before buildMap, and Limbs must be created and distributed AFTER buildMap.
	*
	* After incorporating the database, we will need to differentiate between FIRST TIME (create map) vs. rebuilding
	* from the DB.
	*/

	vector<Point> floorPositions = buildMap(mapForm); /* Build the actual grid for the first time. Receive a list of floor coordinates. */

	/* populate characters and limbs after building the map(and its landmarks). */
	nativeLimbForms = getMapLimbs(mapForm.mapLevel);

	/* FOR NOW I just have ONE copy of each native limb */
	for (LimbForm& limbForm : nativeLimbForms) {
		int numberOfThisLimb = (rand() % 15) + 5;

		for (int n = 0; n < numberOfThisLimb; ++n) {
			Limb& newLimb = roamingLimbs.emplace_back(limbForm);
			Point newPosition = floorPositions[rand() % floorPositions.size()];
			newLimb.setPosition(newPosition);
			newLimb.setLastPosition(newPosition);
		}
	}

	cout << "\n\nThere are " << roamingLimbs.size() << " LIMBS in Roaming Limbs\n\n";
}

/* Map class constructor to rebuild map from DB data.
*		TO DO:
* -----> Add Landmarks
* -----> Add NPCs
*/
Map::Map( MapForm mapForm, vector<Limb> roamingLimbs, vector<vector<Block>> rows, Point characterPosition) :
	mapForm(mapForm), roamingLimbs(roamingLimbs), rows(rows) {

	/* populate characters and limbs after building the map(and its landmarks). */
	nativeLimbForms = getMapLimbs(mapForm.mapLevel);

	UI& ui = UI::getInstance();

	/* create Player Character */
	/* get character texture */
	SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
	SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
	playerCharacter = MapCharacter(CharacterType::Player, characterTexture, characterPosition.x, characterPosition.y);
	SDL_FreeSurface(characterSurface);

	/* Set wall and floor texture indexes. */
	for (int i = 0; i < this->rows.size(); ++i) {
		vector<Block>& blocks = this->rows[i];
		for (int k = 0; k < blocks.size(); ++k) {
			Block& thisBlock = blocks[k];

			thisBlock.setFloorTextureIndex(rand() % mapForm.floorTextures.size());
			/* set texture values for Wall blocks (defaults if they're not wall blocks). */
			if (thisBlock.getIsFloor()) {
				thisBlock.setWallTextureIndex(rand() % mapForm.floorTextures.size());
				thisBlock.setWallIsFlipped(rand() % 2 == 0);

				if (thisBlock.getIsPath()) {
					randomizePathOptions(thisBlock);
				}
			}
			else {
				thisBlock.setWallTextureIndex(rand() % mapForm.wallTextures.size());
				thisBlock.setWallIsFlipped(rand() % 2 == 0);
			}
		}
	}
}

/*
* Build the actual grid of Block objects.
* Returns a vector of Points which are the coordinates for all Floor objects.
*/
vector<Point> Map::buildMap(MapForm mapForm) {
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
	rows = vector<vector<Block>>(mapForm.blocksHeight);

	/* replace with reading from DB */
	for (int i = 0; i < rows.size(); ++i) {
		vector<Block> blocks(mapForm.blocksWidth);

		for (int k = 0; k < blocks.size(); ++k) {
			blocks[k] = Block(false);
		}

		rows[i] = blocks;
	}

	/* Now make the PATH */
	/* get a random x starting point, but the y will be map's height - 2 */

	int pathX = (rand() % (mapForm.blocksWidth - 10)) + 5;
	int pathY = static_cast<int>(rows.size()) - 2;

	int playerX = pathX;
	int playerY = pathY;

	Block& startingBlock = rows[pathY][pathX];
	startingBlock.setIsFloor(true);

	/* make the path */

	SubPath subPath = SubPath(
		(rand() % 5) + 2,
		MapDirection::Up,
		(rand() % 3) + 1);


	/* Entrance landmark */
	landmarks.emplace_back(getEntranceLandmark(Point(pathX, pathY)));

	vector<Point> floorPositions;

	/* while loop makes the path */
	while (pathY > 0) {

		/* choose the next block to floorize */
		switch (subPath.direction) {
		case MapDirection::Up:
			if (pathY > 0) { /* We ARE allowed to hit the ceiling (FOR NOW this ends the pathmaking) */
				--pathY;
			}
			else {
				++pathY;
				subPath.seed = 0;
			}
			break;
		case MapDirection::Down:
			if (pathY < rows.size() - 2) { /* We are NOT allowed to hit the bottom again. */
				++pathY;
			}
			else {
				--pathY;
				subPath.seed = 0;
			}
			break;
		case MapDirection::Left:
			if (pathX > 3) { /* We are NOT allowed to hit the left wall. */
				--pathX;
			}
			else {
				++pathX;
				subPath.seed = 0;
			}
			break;
		case MapDirection::Right:
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
		floorPositions.push_back(Point(pathX, pathY));
		--subPath.seed;

		if (subPath.seed < 1) {
			/* refresh seed */
			subPath.direction = static_cast<MapDirection>(rand() % MapDirection::Total);
			subPath.seed = (rand() % 12) + 1;
			subPath.radius = rand() % 4;
		}
	}

	/* Create Exit landmark. */
	landmarks.emplace_back(getExitLandmark(Point(pathX, pathY)));

	/* Set wall and floor texture indexes. */
	for (int i = 0; i < rows.size(); ++i) {
		vector<Block>& blocks = rows[i];
		for (int k = 0; k < blocks.size(); ++k) {
			Block& thisBlock = blocks[k];
			thisBlock.setFloorTextureIndex(rand() % mapForm.floorTextures.size());
			/* set texture values for Wall blocks (defaults if they're not wall blocks). */
			if (thisBlock.getIsFloor()) {
				thisBlock.setWallTextureIndex(0);
				thisBlock.setWallIsFlipped(false);
			}
			else {
				thisBlock.setWallTextureIndex(rand() % mapForm.wallTextures.size());
				thisBlock.setWallIsFlipped(rand() % 2 == 0);
			}
		}
	}

	/* create Player Character */

	/* get character texture */
	SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
	SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
	playerCharacter = MapCharacter(CharacterType::Player, characterTexture, playerX, playerY);
	SDL_FreeSurface(characterSurface);

	return floorPositions;
}

void Map::randomizePathOptions(Block& block) {
	/* Set up PATH BLOCK info. The path can be flipped vertical, horizontal, or not at all. */
	block.setPathTextureIndex(rand() % mapForm.pathTextures.size());
	block.setPathFlipOption(rand() % 3);
	/* The path can also be rotated 90 degrees, 270 degrees, or not at all. */
	int pathRotateOption = rand() % 3;
	block.setPathRotateAngle(pathRotateOption == 2 ? 270 : pathRotateOption == 1 ? 90 : 0);
}

/*
* When we create a path we want to clear a radius around each block of the central path.
* There's the opportunity here to draw an actual "path" block (different than a floor block... maybe non-diggable?).
* But only certain paths are *actual* paths... many are just normal floor blocks.
*/
void Map::floorize(int x, int y, int radius) {
	/* Incoming coordinates are always a path */
	Block& thisBlock = rows[y][x];
	if (thisBlock.getIsPath()) { return; }

	randomizePathOptions(thisBlock);
	thisBlock.setIsPath(true);

	/* If the block was already a floor, don't bother clearing the surrounding blocks. */
	if (thisBlock.getIsFloor()) { return; }
	thisBlock.setIsFloor(true);
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