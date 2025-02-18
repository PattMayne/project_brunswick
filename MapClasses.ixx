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


export struct AcquiredLimb {
	SDL_Texture* texture;
	int countdown;
	int rotationAngle;
	SDL_Rect diffRect;

	AcquiredLimb(SDL_Texture* texture, int countdown, int rotationAngle, SDL_Rect diffRect) :
		texture(texture), countdown(countdown), rotationAngle(rotationAngle), diffRect(diffRect) {
	}
};

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
	MapCharacter(CharacterType characterType = CharacterType::None) : Character(characterType) {
		texture = NULL;
		/* Set default texture. */
		UI& ui = UI::getInstance();
		SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
		SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
		SDL_FreeSurface(characterSurface);
		setTexture(characterTexture);
		newNpc = false;
	}

	/* There will be no texture. The character will be drawn from their limbs. */
	MapCharacter(CharacterType characterType, int x, int y) :
		Character(characterType, x, y)
	{
		texture = NULL;
		/* Set default texture. */
		UI& ui = UI::getInstance();
		SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
		SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
		SDL_FreeSurface(characterSurface);
		setTexture(characterTexture);
		newNpc = false;
	}

	/* Hostile NPC when loaded from DB. */
	MapCharacter(int id, string name, int anchorLimbId, Point position, vector<Limb> limbs) :
		Character(id, name, anchorLimbId, position, limbs), homePosition(position)
	{
		texture = NULL;
		/* Set default texture. */
		UI& ui = UI::getInstance();
		SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
		SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
		SDL_FreeSurface(characterSurface);
		setTexture(characterTexture);
		newNpc = false;
	}

	~MapCharacter() {} /* destroy texture. */

	int getBlockX() { return blockPosition.x; }
	int getBlockY() { return blockPosition.y; }

	int getLastX() { return lastBlockPosition.x; }
	int getLastY() { return lastBlockPosition.y; }
	Point getPosition() { return blockPosition; }
	Point getLastPosition() { return lastBlockPosition; }
	Point getHomePosition() { return homePosition; }
	vector<AcquiredLimb>& getAcquiredLimbStructs() { return acquiredLimbStructs; }
	bool isNewNpc() { return newNpc; }
	int getNewNpcCountup() { return newNpcCountup; }

	void clearAcquiredLimbStructs() { acquiredLimbStructs = {}; }
	void setHomePosition(Point position) { homePosition = position; }
	void setBlockPosition(Point blockPosition) {
		this->blockPosition = blockPosition;
	}

	/* We count UP so we can multiply the count to get a rotation.
	* When an NPC is created we want to spin their avatar as an animation.
	*/
	void startNewNpcCountup() {
		newNpcCountup = 0;
		newNpc = true;
	}

	/* Get the countup and increment it in the same function so I can do both in a ternary statement. */
	int tickNewNpcCountup() {
		if (newNpcCountup >= 90) {
			newNpc = false;
		}
		else {
			++newNpcCountup;
		}

		return newNpcCountup;
	}

	void updateLastBlock() {
		lastBlockPosition.x = blockPosition.x;
		lastBlockPosition.y = blockPosition.y;
	}

	/* This can be MAP position or DRAW position. */
	void moveToPosition(Point newPosition) {
		lastBlockPosition = blockPosition;
		blockPosition = newPosition;
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
	Point homePosition;
	vector<AcquiredLimb> acquiredLimbStructs;
	bool newNpc;
	int newNpcCountup;
};



/*
* Entrance, Exit, and Shrines are landmarks.
* In sequels this will include buildings (portals to maps with friendly NPCs) and portals to dungeons (maps with more unfriendly NPCs).
*/
export class Landmark {
public:
	/* constructor (primarily for entrance and exit.) */
	Landmark(
		Point position,
		SDL_Texture* texture,
		LandmarkType landmarkType,
		int characterId,
		SuitType suitType = SuitType::NoSuit
	) :
		texture(texture),
		landmarkType(landmarkType),
		characterId(characterId),
		position(position),
		suitType(suitType)
	{ }

	/* destructor */
	~Landmark() { /* Texture is managed by MapScreen and will be destroyed at the end of the run() function. */ }

	int getDrawX() { return position.x; }
	int getDrawY() { return position.y; }
	int getCharacterId() { return characterId; }
	int getId() { return id; }

	Point getPosition() { return position; }
	LandmarkType getType() { return landmarkType; }

	void setId(int id) { this->id = id; }
	void setCharacterId(int characterId) { this->characterId = characterId; }

	SDL_Texture* getTexture() { return texture; }
	SuitType getSuitType() { return suitType; }

	LandmarkCollisionInfo checkCollision(Point pos) { return checkCollision(pos.x, pos.y); }
	LandmarkCollisionInfo checkCollision(int x, int y) {
		if (x == position.x && y == position.y) {
			return { true, landmarkType, characterId };
		}

		return { false, landmarkType, -1 };	}

private:
	/* Point refers to the block grid, not the pixels */
	int id;
	string slug;
	Point position;
	SDL_Texture* texture;
	LandmarkType landmarkType;
	int characterId; /* This can be either the MAP id or the SUIT slug??? Needs re-thinking! */
	SuitType suitType;
};

export Landmark getExitLandmark(Point position) {
	UI& ui = UI::getInstance();
	SDL_Surface* gateSurface = IMG_Load("assets/ENTRANCE.png");
	SDL_Texture* gateTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface);
	SDL_FreeSurface(gateSurface);
	return Landmark(position, gateTexture, LandmarkType::Exit, -1);
}

export Landmark getEntranceLandmark(Point position) {
	UI& ui = UI::getInstance();
	SDL_Surface* gateSurface = IMG_Load("assets/ENTRANCE.png");
	SDL_Texture* gateTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), gateSurface);
	SDL_FreeSurface(gateSurface);
	return Landmark(position, gateTexture, LandmarkType::Entrance, -1);
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
	Map(MapForm mapForm, vector<Limb> roamingLimbs, vector<vector<Block>> rows, Point characterPosition, vector<MapCharacter> hostileNpcs);
	vector<vector<Block>>& getRows() { return rows; }
	vector<Landmark>& getLandmarks() { return landmarks; }
	MapCharacter& getPlayerCharacter() { return playerCharacter; }
	SDL_Texture* getFloorTexture(int index) { return mapForm.floorTextures[index]; }
	SDL_Texture* getWallTexture(int index) { return mapForm.wallTextures[index]; }
	SDL_Texture* getPathTexture(int index) { return mapForm.pathTextures[index]; }
	vector<Limb>& getRoamingLimbs() { return roamingLimbs; }
	string getName() { return mapForm.name; }
	string getSlug() { return mapForm.slug; }
	MapLevel getMapLevel() { return mapForm.mapLevel; }
	vector<MapCharacter>& getNPCs() { return NPCs; }
	vector<Character>& getSuits() { return mapForm.suits; }

	void setLandmarks(vector<Landmark> landmarks) { this->landmarks = landmarks; }
	void setPlayerCharacter(MapCharacter playerCharacter) { this->playerCharacter = playerCharacter; }
	void addNPC(MapCharacter npc) { NPCs.push_back(npc); }

	MapForm& getForm() { return mapForm; }
	void randomizePathOptions(Block& block);



private:
	MapForm mapForm;
	vector<vector<Block>> rows;
	void floorize(int x, int y, int radius, vector<Point>& floorPositions);
	vector<Point> buildMap();
	vector<MapCharacter> NPCs;
	MapCharacter playerCharacter;

	/* stuff sent in from MapData struct */

	vector<Landmark> landmarks;
	vector<LimbForm> nativeLimbForms;
	vector<Limb> roamingLimbs;
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


/* 
* Map class constructor for brand new map.
* For the first time loading the map, which then gets saved to the database.
*/
Map::Map(MapForm mapForm) : mapForm(mapForm) {
	UI& ui = UI::getInstance();
	/*
	* On the first draw, we must build the GRID based on the number of SUITS (therefore shrines) and landmarks.
	* But we must scatter the LIMBS across the available FLOOR tiles, which are only known after creating the grid.
	* So SUITS must be calculated before buildMap, and Limbs must be created and distributed AFTER buildMap.
	*
	* After incorporating the database, we will need to differentiate between FIRST TIME (create map) vs. rebuilding
	* from the DB.
	*/


	/* Create SHRINE landmarks. One for each Suit in the MapForm.
	* They don't have IDs yet, but we will pass these into the database function
	* to save the map and its members, and then we'll populate the IDs.
	* The suit type acts as a placeholder for the character ID for now.
	* The map building function will also add the Point location.
	*/

	/* Use the same shrine image, but we only have to delete it once. */
	SDL_Surface* shrineSurface = IMG_Load("assets/shrine.png");
	SDL_Texture* shrineTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), shrineSurface);
	SDL_FreeSurface(shrineSurface);

	for (Character& suit : mapForm.suits) {

		/* create a landmark */

		Point shrinePoint = Point(0, 0);

		landmarks.emplace_back(
			shrinePoint,
			shrineTexture,
			LandmarkType::Shrine,
			suit.getId(),
			suit.getSuitType()
		);
	}

	/* Build the actual grid for the first time. Receive a list of floor coordinates. */
	vector<Point> floorPositions = buildMap();

	/* populate characters and limbs after building the map(and its landmarks). */
	nativeLimbForms = getMapLimbs(mapForm.mapLevel);

	/* FOR NOW I have random number of copies of each native limb */
	for (LimbForm& limbForm : nativeLimbForms) {
		int numberOfThisLimb = 4;

		for (int n = 0; n < numberOfThisLimb; ++n) {
			Limb& newLimb = roamingLimbs.emplace_back(limbForm);
			Point newPosition = floorPositions[rand() % floorPositions.size()];
			newLimb.setPosition(newPosition);
			newLimb.setLastPosition(newPosition);
		}
	}


	/*
	* 
	* Time to incorporate SUITS.
	* 
	* 1. Fill out the rest of the nativeLimbForms.
	* 
	* 2. Map gets a vector of Characters, called "suits" or "nativeSuits".
	* 3. Create one Shrine for every Suit.
	* --> Maybe there is no vector of Suits, instead a Shrine object which contains a Suit.
	* --> Shrine also holds a vector of structs (or unordered_maps) which connect limbId with true/false for "retrieved".
	* 4. Floor draw pattern goes from entrance, through each shrine, to the exit.
	* --> Sometimes it goes slightly off-course, for variation.
	* --> Also draw other random paths, some with "isPath" paths, sometimes not.
	* 5. Save Suit character to the database.
	* --> Character needs a new column called isSuit.
	* --> Limb needs a new column called isRetrieved.
	* -----> Maybe these should be relational tables, to keep from bogging down these other tables with rarely-used columns.
	* 
	* 
	*/
}

/* Map class constructor to rebuild map from DB data.
*		TO DO:
* -----> Add Landmarks
* -----> Add NPCs
*/
Map::Map(MapForm mapForm, vector<Limb> roamingLimbs, vector<vector<Block>> rows, Point characterPosition, vector<MapCharacter> hostileNpcs) :
	mapForm(mapForm), roamingLimbs(roamingLimbs), rows(rows), NPCs(hostileNpcs) {

	/* populate characters and limbs after building the map(and its landmarks). */
	nativeLimbForms = getMapLimbs(mapForm.mapLevel);

	UI& ui = UI::getInstance();

	/* create Player Character */

	/* get character texture NO LONGER NECESSARY...
	* DELETE (since this is done in the constructor.... and must ALWAYS be done in constructor...
	* texture must never be NULL
	*/
	SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
	SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
	SDL_FreeSurface(characterSurface);

	/* 
	* PROBLEM
	* 
	* How can I load from the database when the database imports this module?
	* 
	* SOLUTION:
	* Map constructor must take pre-built Character object in constructor.
	* The Screen will create the character object (or maybe the Database will).
	* 
	*/
	playerCharacter = MapCharacter(CharacterType::Player, characterPosition.x, characterPosition.y);
	playerCharacter.setTexture(characterTexture); /* Placeholder. */

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
* The Floor Points are used to populate the map with Roaming Limbs.
* 
* This is used the first time a particular Map is loaded.
* It's based on Landmarks.
* We scatter Entrance, Exit, and Shrines.
* We draw a path from Entrance, to each Shrine (one by one),
* and finally to the Exit.
* We also draw random paths to make it a maze.
*/
vector<Point> Map::buildMap() {
	UI& ui = UI::getInstance();
	/* Create a vector of rows (of blocks) of the specified size. */
	rows = vector<vector<Block>>(mapForm.blocksHeight);

	for (int i = 0; i < rows.size(); ++i) {
		/* Create a vector of blocks of the specified size. All are walls. */
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

		floorize(pathX, pathY, subPath.radius, floorPositions);
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
	SDL_FreeSurface(characterSurface);
	playerCharacter = MapCharacter(CharacterType::Player, playerX, playerY);
	playerCharacter.setTexture(characterTexture);

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
void Map::floorize(int x, int y, int radius, vector<Point>& floorPositions) {
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

	/* clear block ABOVE */
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

	/* Clear block BELOW */
	while (y + downInc < rows.size() - 2 && downInc < radius) {

		/* directly below */
		rows[y + downInc][x].setIsFloor(true);
		floorPositions.emplace_back(x, y + downInc);

		/* down and to the left */
		while (x - leftInc > 0 && leftInc < radius) {
			rows[y + downInc][x - leftInc].setIsFloor(true);
			floorPositions.emplace_back(x - leftInc, y + downInc);
			++leftInc;
		}

		leftInc = 0;

		/* up and to the right */
		while (x + rightInc < rows[y - upInc].size() - 2 && rightInc < radius) {
			rows[y + downInc][x + rightInc].setIsFloor(true);
			floorPositions.emplace_back(x + rightInc, y + downInc);
			++rightInc;
		}

		rightInc = 0;
		++downInc;
	}

	/* reset increment counters */
	upInc = 0;
	leftInc = 0;

	/* Clear blocks LEFT */
	while (x - leftInc > 0 && leftInc < radius) {
		rows[y][x - leftInc].setIsFloor(true);
		floorPositions.emplace_back(x - leftInc, y);
		++leftInc;
	}

	/* Clear blocks RIGHT */
	while (x + rightInc < rows[y - upInc].size() - 2 && rightInc < radius) {
		rows[y][x + rightInc].setIsFloor(true);
		floorPositions.emplace_back(x + rightInc, y);
		++rightInc;
	}
}