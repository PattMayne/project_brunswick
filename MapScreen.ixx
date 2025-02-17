/*
* 
*  __  __               ____
* |  \/  | __ _ _ __   / ___|  ___ _ __ ___  ___ _ __
* | |\/| |/ _` | '_ \  \___ \ / __| '__/ _ \/ _ \ '_ \
* | |  | | (_| | |_) |  ___) | (__| | |  __/  __/ | | |
* |_|  |_|\__,_| .__/  |____/ \___|_|  \___|\___|_| |_|
*              |_|
* 
* Where the character roams the open world (or a dungeon or building) seeking limbs, fighting NPCs, and meeting friendly NPCs.
* 
* 
* 
* The MAP has a vector of LANDMARK objects.
* We'll create an ENTRANCE and EXIT... but we need some way to access them explicitly
* The other landmarks will be SHRINES (no buildings in this game... not enough time).
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
* ANIMATING MOVES:
* 
* -- Player Move Animation is different from NPC Move Animation
* -----	Animation Lock
* -----	enum AnimationTurnType { Player, NPC }
* -----	Poll_Event removes the events from the que... so we should STILL POLL, but don't handle the event during animation
* -----	set a countdown of 10 (10 frames) which also moves player's DRAW position and each block's DRAW position 10% per tick
* ---------- At the END of the countdown, THEN we move the player
* ---------- LIMBS and CHARACTERS have "previousBlock" X and Y and we do "increment percent" * countdown FROM the PREVIOUS to the current XY
* 
* -----	I only need to animate the CHARACTER when they're close to the edge. Otherwise I simply animate the map.
* -----	When animating the map, I need to add extra blocks and rows.
* 
* -- Extra SPEED will allow SOME NPCs to move multiple blocks at a time!
* -- This is made possible by storing PREVIOUS BLOCK (instead of just letting the draw functions increment by one block automatically)
* -- The PLAYER CHARACTER can ALSO move in multiple blocks IF they have enough speed.
* -----	They can SET how many moves they WANT to make just by pressing that number... and it stays until they change it!
* ----- They can also click on the block they want to go to, and they'll move as far in that direction as their speed allows.
* 
* --- When the PLAYER has that speed, they just get MULTIPLE TURNS before the NPCs get to move.
*			THAT WAY we only have to animate ONE BLOCK at a time.
* 
* 
* NEXT:
* 
* ----- 
*
* Every map should have three walls and three floors, plus a path (floor) and a border (wall).
* 
* 
*		COLLISION ANIMATIONS:
* 
*			--- When NPCs are created make them spin around and grow larger then normal size, similar to the Limb acquisition.
*			--- When NPCs pick up a new Limb, do the normal Limb Acquisition animation (spin, grow/shrink).
*			--- When player hits NPC the NPC should spin and grow so large they consume the whole screen before we load battle screen.
* 
* 
* Destroy all textures for PANELS and LIMBS (on every screen).
*/

module;

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

export module MapScreen;

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
import MapClasses;
import Database;

using namespace std;


enum class AnimationType { Player, NPC, Collision, None }; /* This is about whose TURN it is. */



/* Map Screen class: where we navigate worlds, dungeons, and buildings.
* (Actually only worlds. Dungeons and buildings happen in sequel).
* 
* We either create a new map (based on the map_slug) and save it to the database,
* or we load it from the database. map_slug is the primary key in the DB table,
* so there is only ONE of each map.
* 
* The run() function contains the game loop.
* Movement is turn-based. Player moves up/down/left/right,
* then the map (limbs, NPCs) also move.
* 
* If player collides with Limb, player collects Limb.
* If player collides with NPC, go to Battle Screen.
* If limbs collide they form a hostile NPC.
* NPCs can pick up new limbs, which are either equipped or stored in inventory.
* 
* Movements are animated, during which time the game is paused and input is ignored.
* Limb-acquisition and NPC-formation are also animated, but this does not pause the game.
*/
export class MapScreen {
	public:
		/* 
		* constructor:
		* 
		* Only needs the slug.
		* We check the DB to see if this mapSlug already has a map (map_slug is primary key).
		* If so, load that map. Otherwise, create a new one and save it to the DB.
		*/
		MapScreen(string mapSlug) {
			UI& ui = UI::getInstance();

			/* TEMPORARY MAP-CREATION SCHEME 
			* 
			* PHASE 1 (DONE):
			* 1) Get vanilla Forest Map every time.
			* 2) Save the map immediately after creation.
			* 
			* PHASE 2 (PRESENT PHASE IMPLEMENTED):
			* 1) Check for EXISTING Forest Map.
			* 2) LOAD that map from the database if it exists.
			* 3) If it does NOT exist, do PHASE 1 steps.
			* 
			* PHASE 3:
			* 1) Load shrines for each native NPC.
			* 2) Path creation is built on connecting those shrines to the entrance and exit.
			* 3) Hostile NPC-creation on limb collision.
			* 4) Collision with hostile NPC goes to Battle screen.
			*/

			mapForm = getMapFormFromSlug(mapSlug);

			if (mapObjectExists(mapSlug)) {
				/* Load existing map. */
				map = loadMap(mapSlug);

				for (MapCharacter& npc : map.getNPCs()) {
					npc.buildDrawLimbList();
					npc.setTexture(npc.createAvatar());
				}

				map.setPlayerCharacter(loadPlayerMapCharacter());
				if (map.getPlayerCharacter().getEquippedLimbs().size() > 0) {
					map.getPlayerCharacter().setTexture(map.getPlayerCharacter().createAvatar());
				}

				updatePlayerMap(mapSlug);
			}
			else {
				/* Create new map. */
				map = Map(mapForm);
				createNewMap(map);
				updatePlayerMap(mapSlug);


				if (map.getPlayerCharacter().getEquippedLimbs().size() > 0 ) {
					map.getPlayerCharacter().setTexture(map.getPlayerCharacter().createAvatar());
				}
				else {
					/* Must replace with DEFAULT LIMBS. */
					SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
					SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
					SDL_FreeSurface(characterSurface);
					map.getPlayerCharacter().setTexture(characterTexture);
				}
			}

			screenType = ScreenType::Map;
			screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

			hBlocksTotal = mapForm.blocksWidth;
			vBlocksTotal = mapForm.blocksHeight;
			animationType = AnimationType::None;

			showTitle = true;
			titleCountdown = 140;

			setViewResAndBlockWidth(ui);
			setMaxDrawBlock();
			setDrawStartBlock();
			buildMapDisplay();
			createTitleTexture(ui);
			limbAngle = 0;
			npcHeight = 0;
			homeBaseRange = 5;
			waitSpin = false;
		}

		/* Destructor */
		~MapScreen() {
			SDL_DestroyTexture(floorTexture);
			SDL_DestroyTexture(wallTexture);
			SDL_DestroyTexture(titleTexture);

			/* Destroy all the textures in the Landmarks */
			for (int i = 0; i < map.getLandmarks().size(); i++) {
				SDL_Texture* textureToDestroy = map.getLandmarks()[i].getTexture();
				if (textureToDestroy) { SDL_DestroyTexture(textureToDestroy); } }

			/* Destroy all the textures in the roamingLimbs */
			vector<Limb>& roamingLimbs = map.getRoamingLimbs();
			for (int i = 0; i < roamingLimbs.size(); i++) {
				SDL_Texture* textureToDestroy = roamingLimbs[i].getTexture();
				if (textureToDestroy) { SDL_DestroyTexture(textureToDestroy); } }

			/* Destroy all the textures in the Characters */
			SDL_DestroyTexture(map.getPlayerCharacter().getTexture());
			SDL_DestroyTexture(wallTexture);
			SDL_DestroyTexture(floorTexture);
		}

		int getAnimationIncrementPercent() { return animationIncrementFraction; }
		int getAnimationCountdown() { return animationCountdown; }
		void startAnimationCountdown(AnimationType iType);
		void decrementCountdown();

		ScreenType getScreenType() { return screenType; }
		MapType getMapType() { return mapForm.mapType; }
		MapForm mapForm;
		void run();

		bool checkPlayerNpcCollision();
		bool checkPlayerLimbCollision(); /* Limb-on-player collision. */
		bool checkLimbOnLimbCollision(); /* Limb collides with Limb to make NPC. */
		bool checkNpcOnLimbCollision();

	private:
		ScreenType screenType;
		ScreenStruct screenToLoadStruct;
		void drawMap(UI& ui);
		void drawBlock(UI& ui, Block& block, SDL_Rect targetRect);
		void drawLandmarks(UI& ui);
		void drawCharacters(UI& ui);
		void drawPlayerCharacter(UI& ui);
		void drawRoamingLimbs(UI& ui);
		void drawNpcs(UI& ui);
		void drawAcquiredLimbs(UI& ui, MapCharacter& character, int drawX, int drawY);
		void draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel);

		bool moveLimb(Limb& roamingLimb);
		bool moveNPC(MapCharacter& npc);
		void animateMapBlockDuringPlayerMove(SDL_Rect& rect, int blockPositionX, int blockPositionY);
		void animateMovingObject(SDL_Rect& rect, int blockPositionX, int blockPositionY, Point lastPosition);

		void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState);
		void handleMousedown(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel);
		void handleKeydown(SDL_Event& e);
		void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel);

		void buildMapDisplay();
		void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);
		void setDrawStartBlock();
		void setMaxDrawBlock();
		void setViewResAndBlockWidth(UI& ui);
		void setScrollLimits();

		int xViewRes; /* Horizontal Resolution of the screen ( # of blocks displayed across the top) */
		int yViewRes; /* Vertical Resolution of the screen ( # of vertical blocks, depends on xViewRes) */
		int blockWidth; /* actual pixel dimensions of the block. depends on horizontal resolution */
		int vBlocksVisible;

		int rightLimit;
		int leftLimit;
		int topLimit;
		int bottomLimit;

		int hBlocksTotal;
		int vBlocksTotal;

		bool animate;
		int animationIncrementFraction = 20;
		int animationCountdown;
		int limbCollisionCountdown = 70;
		int blockAnimationIncrement;
		AnimationType animationType;

		bool showTitle;
		int titleCountdown;

		Map map;

		SDL_Texture* floorTexture = NULL;
		SDL_Texture* wallTexture = NULL;

		SDL_Texture* getWallTexture(int index) { return map.getWallTexture(index); }
		SDL_Texture* getFloorTexture(int index) { return map.getFloorTexture(index); }

		void createTitleTexture(UI& ui);

		SDL_Texture* titleTexture;
		SDL_Rect titleRect;

		void raiseTitleRect() {
			--titleRect.y; }

		int getTitleBottomPosition() {
			return titleRect.y + titleRect.h; }

		void requestUp();
		void requestDown();
		void requestLeft();
		void requestRight();
		void waitTurn();
		void moveCharacter(MapDirection direction);

		int drawStartX = 0;
		int drawStartY = 0;
		int lastDrawStartX = 0;
		int lastDrawStartY = 0;
		int maxDrawStartX = 0;
		int maxDrawStartY = 0;

		int limbAngle;
		int npcHeight;

		int homeBaseRange;
		bool blockIsDrawable(Point position);
		bool waitSpin;

		/* still need looted wall texture, looted floor texture, character texture (this actually will be in character object).
		* The NPCs (in a vactor) will each have their own textures, and x/y locations.
		*/
};


/*
* 
* 
* 
* 
*		MapScreen Functions
* 
* 
* 
* 
*/


/* Create the texture with the name of the game */
void MapScreen::createTitleTexture(UI& ui) {
	Resources& resources = Resources::getInstance();
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture(map.getName());
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}

void MapScreen::buildMapDisplay() {
	UI& ui = UI::getInstance();
	SDL_Surface* mainSurface = ui.getWindowSurface();

	setViewResAndBlockWidth(ui);
	setMaxDrawBlock();
	setDrawStartBlock();	

	vBlocksVisible = (mainSurface->h / blockWidth) + 1; /* adding one to fill any gap on the bottom. (Will always add 1 to xViewRes when drawing) */

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
	setScrollLimits();
}

/* get the maximum allowed map position of top left block on-screen. */
void MapScreen::setMaxDrawBlock() {
	maxDrawStartX = static_cast<int>(map.getRows().size()) - xViewRes;
	maxDrawStartY = static_cast<int>(map.getRows().size()) - yViewRes;
}


/* set when screen loads or resizes */
void MapScreen::setViewResAndBlockWidth(UI& ui) {
	xViewRes = 18; /* LATER user can update this to zoom in or out. Function to update must also updated yViewRes */

	/* get and set y resolution... must be updated whenever xViewRes is updated. PUT THIS IN FUNCTION LATER. */
	blockWidth = ui.getWindowWidth() / xViewRes;
	yViewRes = (ui.getWindowHeight() / blockWidth) + 1;
	++xViewRes; /* give xViewRes an extra block so there's never blank space on the side of the screen (when half blocks get cut off. */

	/* make animationIncrementFraction work based on blockWidth (preferably something fully divisible) */
	if (animationIncrementFraction > blockWidth) {
		animationIncrementFraction = blockWidth; }
	else if (blockWidth % animationIncrementFraction > 3) {
		/* do an algorithm to find something ALMOST divisible (it would be nice to prefer zero remainder) */
		int fractionFinderIncrement = 1;

		while (fractionFinderIncrement < blockWidth) {
			int divisibleFractionFinder = animationIncrementFraction + fractionFinderIncrement;
			if (blockWidth % (animationIncrementFraction + fractionFinderIncrement) <= 3) {
				animationIncrementFraction = animationIncrementFraction + fractionFinderIncrement;
				break;
			} else if (blockWidth % (animationIncrementFraction - fractionFinderIncrement) <= 3) {
				animationIncrementFraction = animationIncrementFraction - fractionFinderIncrement;
				break;
			}
			++fractionFinderIncrement;
		}
	}
}

/*
* Beyond these limits of the map, the character scrolls across the screen, not the map.
*/
void MapScreen::setScrollLimits() {
	rightLimit = maxDrawStartX + (xViewRes / 2);
	leftLimit = xViewRes / 2;
	topLimit = yViewRes / 2;
	bottomLimit = maxDrawStartY + (yViewRes / 2);
}


/* Screen has been resized. Rebuild! */
void MapScreen::rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	ui.rebuildGameMenuPanel(gameMenuPanel);
	buildMapDisplay();
	createTitleTexture(ui);
}


/* Decide if the Point is within the portion of the map that's being drawn. */
bool MapScreen::blockIsDrawable(Point position) {
	int posX = position.x;
	int posY = position.y;

	return posX > drawStartX - 5 &&
		posX < drawStartX + xViewRes + 5 &&
		posY > drawStartY - 5 &&
		posY < drawStartY + yViewRes + 5;
}


/* The main function of the module, containing the game loop.
* Multiple kinds of animations happen during game loop.
* MOVE animation is controlled by the animate boolean.
* Whenever the player moves, the map and/or characters are shifted, and this is called an animation.
* When the player has finished moving, the NPCs and LIMBs also move.
This is also controlled by the animate boolean, but differentiated by the AnimationType enum value.
* 
* SPRITE ANIM is different. It ALWAYS happens.
* Limbs rotate back and forth.
* NPCs bounce up and down.
* These are controlled by spriteAnim ints, which go up and down.
*/
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
	const int TARGET_FPS = 120;
	const int FRAME_DELAY = 1200 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	/* loop and event control */
	SDL_Event e;
	bool running = true;
	animate = false;

	/* Making the Limbs rotate. */
	int spriteAnimMax = 15;
	bool reverseSpriteAnimation = false;

	/* Making the NPCs bounce. */
	int npcAnimMax = 5;
	bool reverseNpcAnimation = false;
	bool animateNpcThisFrame = false;

	vector<int> collidedLimbIDs; /* Contains the database IDs, not the vector indexes. */

	/* SCREEN LOOP */
	while (running) {
		/* Get the total running time(tick count) at the beginning of the frame, for the frame timeout at the end */
		frameStartTime = SDL_GetTicks();
		bool startNpcAnimation = false;


		
		if (animate && animationCountdown < 1) {
			/* Animation counter has run out but animate is still true (final frame of animation).
			* We check collisions on the final frame.
			*/

			bool landmarkCollided = false;
			bool npcCollided = false;
			bool limbCollided = false;
			waitSpin = false;
			
			if (animationType == AnimationType::Player) {
				/*
				* Player animation has finished.
				* Reset lastDrawStart values and check for collisions.
				* If no collisions happened, start NPC animation.
				*/
				animate = false;
				animationType = AnimationType::None;
				lastDrawStartX = drawStartX;
				lastDrawStartY = drawStartY;
				
				/* check for collisions (animation is done, player is ON new block and/or NPCs have moved ONTO new blocks */

				/* collisions with LANDMARK: */

				MapCharacter& playerCharacter = map.getPlayerCharacter();
				
				for (Landmark landmark : map.getLandmarks()) {
					LandmarkCollisionInfo collisionInfo = landmark.checkCollision(playerCharacter.getPosition());
					
					if (collisionInfo.hasCollided) {
						cout << "HIT LANDMARK\n";
						landmarkCollided = true;

						if (collisionInfo.type == LandmarkType::Exit) {
							cout << "EXITING\n";
							running = false;
						}
						else if (collisionInfo.type == LandmarkType::Entrance) {
							cout << "YOU CANNOT LEAVE THIS WAY\n";
							/* TO DO: animate PUSHING the character OFF the entrance??? */
						}
					}					
				}

				checkPlayerLimbCollision(); /* Player collects limb. */

				 /* Collisions with NPCs */
				 checkPlayerNpcCollision();

				if (!landmarkCollided && !npcCollided) {
					/* start the NPC animation. */
					startNpcAnimation = true;
				}
			}
			else if (animationType == AnimationType::NPC) {
				/* Deal with wrapping up the NPC-moved animation. */
				animate = false;
				animationType = AnimationType::None;

				/*
				* Check for collisions again.
				* But also check for Limbs or NPCs who moved onto the Player's block.
				* The order matters.
				*/

				/* Check LIMBs colliding with each other. IF they form an NPC on the player's block, the player fights the NPC. */
				checkNpcOnLimbCollision(); /* NPC collects new limb. It's the NPC's move, so they gather the limb instead of Player (if on same block). */
				checkLimbOnLimbCollision(); /* Limbs combine to form new NPC. */
				checkPlayerLimbCollision(); /* Player collects new limb. */
				checkPlayerNpcCollision(); /* Go to battle screen. */
			}
		}

		/* Check for events in queue, and handle them(really just checking for X close now */
		while (SDL_PollEvent(&e) != 0) {
			if (
				!animate && !startNpcAnimation && /* Drop any events during the animations. */
				(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_KEYDOWN) /* Actual events to respond to. */
			) {
				handleEvent(e, running, settingsPanel, gameMenuPanel, gameState);
			}
		}

		/* Deal with showing the title. */
		if (showTitle) {
			if (titleCountdown > 0) {
				--titleCountdown; }
			else {
				raiseTitleRect(); }

			if (getTitleBottomPosition() < -1) {
				showTitle = false; }
		}

		checkMouseLocation(e, settingsPanel, gameMenuPanel);
		draw(ui, settingsPanel, gameMenuPanel);

		/* Delay so the app doesn't just crash */
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; /* Calculate how long the frame took to process. */
		/* Delay loop */
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed); }

		if (animate) { decrementCountdown(); }

		if (startNpcAnimation) {
			/* First move the NPCs and the Limbs */

			/*
			* 
			* MOVE NPCs
			* 
			*/

			for (MapCharacter& npc : map.getNPCs()) {
				Point npcPosition = npc.getPosition();
				int posX = npcPosition.x;
				int posY = npcPosition.y;

				/* skip limbs that are too far outside of the frame. (still animate them if they're nearby). */
				if (posX < drawStartX - 9 || posX > drawStartX + xViewRes + 9 ||
					posY < drawStartY - 9 || posY > drawStartY + yViewRes + 9
					) { continue; }

				moveNPC(npc);
			}

			// SAVE NPC LOCATIONS (one big transaction in the DB)

			/*
			* 
			*		MOVE LIMBS
			* 
			*/

			for (Limb& limb : map.getRoamingLimbs()) {
				Point limbPosition = limb.getPosition();
				int posX = limbPosition.x;
				int posY = limbPosition.y;

				/* skip limbs that are too far outside of the frame. (still animate them if they're nearby). */
				if (posX < drawStartX - 9 || posX > drawStartX + xViewRes + 9 ||
					posY < drawStartY - 9 || posY > drawStartY + yViewRes + 9
					) { continue; }

				moveLimb(limb); }

			/* Save the new locations. */
			updateLimbsLocation(map.getRoamingLimbs());

			/* Then start the Animation of the movement. */
			startAnimationCountdown(AnimationType::NPC);
			startNpcAnimation = false;
		}

		/* check the sprite anim situation (wiggling Roaming Limbs, bouncing NPCs.) */

		if (reverseSpriteAnimation) {
			if (limbAngle > spriteAnimMax * -1) {
				--limbAngle;
			}
			else { reverseSpriteAnimation = !reverseSpriteAnimation; }
		}
		else {
			if (limbAngle < spriteAnimMax) {
				++limbAngle;
			}
			else { reverseSpriteAnimation = !reverseSpriteAnimation; }
		}


		if (animateNpcThisFrame) {
			if (reverseNpcAnimation) {
				if (npcHeight > npcAnimMax * -1) {
					--npcHeight;
				}
				else { reverseNpcAnimation = !reverseNpcAnimation; }
			}
			else {
				if (npcHeight < npcAnimMax) {
					++npcHeight;
				}
				else { reverseNpcAnimation = !reverseNpcAnimation; }
			}
		}
		animateNpcThisFrame = !animateNpcThisFrame;
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}

int getDiff(int a, int b) {
	if (a > b) {
		return a - b;
	}
	else {
		return b - a;
	}
}

/*
* After every player turn, each NPC must move to an adjacent block.
* 
* TO DO: We must give NPCs a "home" position which they PROTECT and stay near.
* For now we just make them move at random.
*/
bool MapScreen::moveNPC(MapCharacter& npc) {
	bool moved = false;
	Point currentPosition = npc.getPosition();
	int pointX = currentPosition.x;
	int pointY = currentPosition.y;

	int xDistanceFromHome = getDiff(pointX, npc.getHomePosition().x);
	int yDistanceFromHome = getDiff(pointY, npc.getHomePosition().y);

	/* Get list of available blocks */
	vector<Point> availablePositions;
	vector<vector<Block>>& rows = map.getRows();
	Point homePosition = npc.getHomePosition();

	/* Check up. */
	if (pointY > 0) {
		int newY = pointY - 1;
		bool isInRange = getDiff(newY, homePosition.y) <= homeBaseRange;

		if (isInRange && rows[newY][pointX].getIsFloor()) {
			availablePositions.push_back(Point(pointX, newY));
		}
	}

	/* Check down. */
	if (pointY < vBlocksTotal) {
		int newY = pointY + 1;
		bool isInRange = getDiff(newY, homePosition.y) <= homeBaseRange;

		if (isInRange && rows[newY][pointX].getIsFloor()) {
			availablePositions.push_back(Point(pointX, newY));
		}
	}

	/* Check left. */
	if (pointX > 0) {
		int newX = pointX - 1;
		bool isInRange = getDiff(newX, homePosition.x) <= homeBaseRange;

		if (isInRange && rows[pointY][newX].getIsFloor()) {
			availablePositions.push_back(Point(newX, pointY));
		}
	}

	if (pointX < hBlocksTotal) {
		int newX = pointX + 1;
		bool isInRange = getDiff(newX, homePosition.x) <= homeBaseRange;

		if (isInRange && rows[pointY][newX].getIsFloor()) {
			availablePositions.push_back(Point(newX, pointY));
		}
	}

	if (availablePositions.size() > 0) {
		Point& newPoint = availablePositions[rand() % availablePositions.size()];
		npc.moveToPosition(newPoint);
		moved = true;
	}
	else {
		cout << "NPC has no place to go?\n";
	}

	return moved;
}

/*
* After every player turn, each Roaming Limb must move to an adjacent block.
*/
bool MapScreen::moveLimb(Limb& roamingLimb) {
	bool moved = false;
	Point currentPosition = roamingLimb.getPosition();
	int pointX = currentPosition.x;
	int pointY = currentPosition.y;

	/* Get list of available blocks */
	vector<Point> availablePositions;
	vector<vector<Block>>& rows = map.getRows();

	/* Check up. */
	if (pointY > 0) {
		if (rows[pointY - 1][pointX].getIsFloor()) {
			availablePositions.push_back(Point(pointX, pointY - 1)); } }

	/* Check down. */
	if ( pointY < vBlocksTotal) {
		if (rows[pointY + 1][pointX].getIsFloor()) {
			availablePositions.push_back(Point(pointX, pointY + 1)); } }

	/* Check left. */
	if (pointX > 0) {
		if (rows[pointY][pointX - 1].getIsFloor()) {
			availablePositions.push_back(Point(pointX - 1, pointY)); } }

	if (pointX < hBlocksTotal) {
		if (rows[pointY][pointX + 1].getIsFloor()) {
			availablePositions.push_back(Point(pointX + 1, pointY)); } }

	if (availablePositions.size() > 0) {
		Point& newPoint = availablePositions[rand() % availablePositions.size()];
		roamingLimb.move(newPoint);
		moved = true;
	}

	return moved;
}

/* The main DRAW function, which calls the more-specific Draw functions. */
void MapScreen::draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel) {
	unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	if (!animate) { blockAnimationIncrement = 0; }
	else {
		/* Get the distance for the camera to travel in this iteration of the animation sequence. (used in multiple draw functions) */
		int baseIncrement = (blockWidth / animationIncrementFraction);
		blockAnimationIncrement = (baseIncrement * (animationIncrementFraction - animationCountdown)) + baseIncrement; }
	drawMap(ui);
	drawLandmarks(ui);
	drawCharacters(ui);
	if (showTitle) {
		SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE); }

	gameMenuPanel.draw(ui);
	settingsPanel.draw(ui);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}

/* Draw buildings, exits, shrines, and loot boxes. */
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
		lX = landmark.getDrawX();
		lY = landmark.getDrawY();
		targetRect.w = blockWidth;
		targetRect.h = blockWidth;

		if (
			lX >= drawStartX &&
			lX <= (drawStartX + xViewRes) &&
			lY >= drawStartY &&
			lY <= (drawStartY + yViewRes)
		) {
			targetRect.x = (lX - drawStartX) * blockWidth;
			targetRect.y = (lY - drawStartY) * blockWidth;

			if (animate && animationType == AnimationType::Player) {
				animateMapBlockDuringPlayerMove(targetRect, lX, lY); }

			SDL_RenderCopyEx(
				ui.getMainRenderer(),
				landmark.getTexture(),
				NULL, &targetRect,
				0, NULL, SDL_FLIP_NONE);
		}
	}
}

/* Draw NPCs and player character */
void MapScreen::drawCharacters(UI& ui) {
	/* cycle through the characters list and draw them */

	/* ONLY draw the ones in the screen */

	/* 
	* Draw Player Character last
	* --- ACTUALLY... if the NPC moves onto YOU then they should be drawn OVER the player.
	*					(figure that out later)
	*/
	drawRoamingLimbs(ui);
	drawNpcs(ui);
	drawPlayerCharacter(ui);
}

void MapScreen::drawPlayerCharacter(UI& ui) {
	SDL_Rect characterRect = { 0, 0, blockWidth, blockWidth }; /* This should be a member of MapScreen or Character. */
	MapCharacter& playerCharacter = map.getPlayerCharacter();

	int blockX = playerCharacter.getBlockX();
	int blockY = playerCharacter.getBlockY();
	
	characterRect.x = (blockX - drawStartX) * blockWidth;
	characterRect.y = ((blockY - drawStartY) * blockWidth) - npcHeight;

	/* Check if we are animating AND close to an edge.
	* If close to a vertical edge, and moving vertically, animate the character.
	* If close to a horizontal edge, and moving horizontally, animate the character.
	* Player has different animation dynamics from the map blocks.
	*/
	
	if (animate && animationType == AnimationType::Player) {
		animateMovingObject(characterRect, blockX, blockY, playerCharacter.getLastPosition()); }

	double playerRotation = !waitSpin ? 0 : animationCountdown * 20;

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		playerCharacter.getTexture(),
		NULL, &characterRect,
		playerRotation, NULL, SDL_FLIP_NONE
	);

	/*
	* We can animate the player character here.
	* Don't go overboard, but every now and then do a sequence to wiggle a limb.
	*/

	if (false) {
		/* THIS IS JUST AN EXAMPLE
		* But it works. It turns the anchor limb around in a constant circle.
		*/
		playerCharacter.getAnchorLimb().rotate(1);
		playerCharacter.setTexture(playerCharacter.createAvatar());
	}	

	drawAcquiredLimbs(ui, playerCharacter, characterRect.x, characterRect.y);
}

/*
* Call this within the drawPlayerCharacter function so we don't have to calculate the location again.
* Also, the acquired limbs can be considered part of the player character.
* 
*/
void MapScreen::drawAcquiredLimbs(UI& ui, MapCharacter& character, int charDrawX, int charDrawY) {
	
	vector<AcquiredLimb>& acquiredLimbStructs = character.getAcquiredLimbStructs();

	for (int i = acquiredLimbStructs.size() - 1; i >= 0; --i) {
		AcquiredLimb& aLimbStruct = acquiredLimbStructs[i];

		/* If the countdown is finished, or if the draw rect will be too small, remove this limb from the queue. */
		if (aLimbStruct.countdown < 1 || aLimbStruct.diffRect.w < ((blockWidth - 3) * -1)) {
			acquiredLimbStructs.erase(acquiredLimbStructs.begin() + i);
			continue;
		}

		/*
		* Increment the angle so it spins during the whole animation.
		* Make the limb grow large quickly for the first part of the animation countdown.
		* Then make the limb shrink more quickly.
		* Size differentials are stored in the acquired limb struct.
		* The actual rect is based on the character's current location block, altered by those differentials.
		*/

		aLimbStruct.rotationAngle += 7;

		if (aLimbStruct.countdown > 25) {
			/* Growing BIGGER slowly. */
			aLimbStruct.diffRect.x -= 1;
			aLimbStruct.diffRect.y -= 1;
			aLimbStruct.diffRect.w += 2;
			aLimbStruct.diffRect.h += 2;
		}
		else {
			/* Growing SMALLER quickly. */
			aLimbStruct.diffRect.x += 3;
			aLimbStruct.diffRect.y += 3;
			aLimbStruct.diffRect.w -= 6;
			aLimbStruct.diffRect.h -= 6;
		}

		SDL_Rect limbRect = {
			charDrawX + aLimbStruct.diffRect.x,
			charDrawY + aLimbStruct.diffRect.y,
			blockWidth + aLimbStruct.diffRect.w,
			blockWidth + aLimbStruct.diffRect.h
		};

		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			aLimbStruct.texture,
			NULL, &limbRect,
			aLimbStruct.rotationAngle, NULL, SDL_FLIP_NONE
		);

		--aLimbStruct.countdown;
	}
}

/*
* During the animation of the Player Character moving from one block to another,
* if the map has to move, it must move in increments defined by blockAnimationIncrement,
* which is set during every frame of the animation.
* This function accepts a reference to the rect where we will draw the block (or limb, or NPC, or landmark texture)
* onto the screen, and the positions of the block, use those positions to tell the rect where it must draw the block (or &etc).
*/
void MapScreen::animateMapBlockDuringPlayerMove(SDL_Rect& rect, int blockPositionX, int blockPositionY) {
	/* Shifting DOWN or UP. */
	if (drawStartY > lastDrawStartY) {
		rect.y = ((blockPositionY - lastDrawStartY) * blockWidth) - blockAnimationIncrement; }
	else if (drawStartY < lastDrawStartY) {
		rect.y = ((blockPositionY - lastDrawStartY) * blockWidth) + blockAnimationIncrement; }

	/* Shifting RIGHT or LEFT. */
	if (drawStartX > lastDrawStartX) {
		rect.x = ((blockPositionX - lastDrawStartX) * blockWidth) - blockAnimationIncrement; }
	else if (drawStartX < lastDrawStartX) {
		rect.x = ((blockPositionX - lastDrawStartX) * blockWidth) + blockAnimationIncrement; }
}

/*
*  When a character or limb moves while the map stays still, use this function to set the incremented position
* of the rect for each frame of the animation.
*/
void MapScreen::animateMovingObject(SDL_Rect& rect, int blockPositionX, int blockPositionY, Point lastPosition) {
	int lastBlockX = lastPosition.x;
	int lastBlockY = lastPosition.y;

	/* are we close to a horizontal edge and moving horizontally? */
	if (
		blockPositionX > rightLimit ||
		animationType == AnimationType::NPC || /* only check limits if the Player is being animated. */
		(lastBlockX > rightLimit || blockPositionX < leftLimit || lastBlockX < leftLimit)
	) {
		/* are we moving left or moving right? */
		if (blockPositionX > lastBlockX) {
			/* we are moving right */
			rect.x = ((lastBlockX - drawStartX) * blockWidth) + blockAnimationIncrement; }
		else if (blockPositionX < lastBlockX) {
			/* we are moving left */
			rect.x = ((lastBlockX - drawStartX) * blockWidth) - blockAnimationIncrement; }
	}

	/* are we close to a vertical edge and moving vertically? */
	if (
		blockPositionY > bottomLimit ||
		animationType == AnimationType::NPC || /* only check limits if the Player is being animated. */
		(lastBlockY > bottomLimit || blockPositionY < topLimit || lastBlockY < topLimit)
	) {
		/* are we moving up or down? */
		if (blockPositionY > lastBlockY) {
			/* we are moving down */
			rect.y = ((lastBlockY - drawStartY) * blockWidth) + blockAnimationIncrement; }
		else if (blockPositionY < lastBlockY) {
			/* we're moving up */
			rect.y = ((lastBlockY - drawStartY) * blockWidth) - blockAnimationIncrement; }
	}
}

/*
* Cycle through the map's list of hostile NPCs and draw them all.
*/
void MapScreen::drawNpcs(UI& ui) {
	SDL_Rect npcRect = { 0, 0, blockWidth, blockWidth };

	for (MapCharacter& npc : map.getNPCs()) {
		Point position = npc.getPosition();

		/* skip NPCs that are too far outside of the frame. (still draw them if they might fly onto the frame.) */
		if (!blockIsDrawable(position)) {
			npc.clearAcquiredLimbStructs();			
			continue; }

		int posX = position.x;
		int posY = position.y;

		npcRect.x = (posX - drawStartX) * blockWidth;
		npcRect.y = ((posY - drawStartY) * blockWidth) + npcHeight;

		/* Synchronize with map during movement animations. */
		if (animate) {
			if (animationType == AnimationType::Player) {
				animateMapBlockDuringPlayerMove(npcRect, posX, posY);
			}
			else if (animationType == AnimationType::NPC) {
				animateMovingObject(npcRect, posX, posY, npc.getLastPosition());
			}
		}

		int angleToRotate = !npc.isNewNpc() ? 0 : npc.tickNewNpcCountup() * 4;

		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			npc.getTexture(),
			NULL, &npcRect,
			angleToRotate,
			NULL, SDL_FLIP_NONE
		);

		if (npc.getAcquiredLimbStructs().size() > 0) {
			drawAcquiredLimbs(ui, npc, npcRect.x, npcRect.y);
		}
	}
}

/* Make the limbs move after every Player turn. */
void MapScreen::drawRoamingLimbs(UI& ui) {
	SDL_Rect limbRect = { 0, 0, blockWidth, blockWidth };

	for (Limb& limb : map.getRoamingLimbs()) {
		Point position = limb.getPosition();
		int posX = position.x;
		int posY = position.y;

		/* skip limbs that are too far outside of the frame. (still draw them if they might fly onto the frame). */
		if (posX < drawStartX - 5 || posX > drawStartX + xViewRes + 5 ||
			posY < drawStartY - 5 || posY > drawStartY + yViewRes + 5
		) { continue; }

		limbRect.x = (posX - drawStartX) * blockWidth;
		limbRect.y = (posY - drawStartY) * blockWidth;

		if (animate) {
			if (animationType == AnimationType::Player) {
				animateMapBlockDuringPlayerMove(limbRect, posX, posY); }
			else if (animationType == AnimationType::NPC) {
				animateMovingObject(limbRect, posX, posY, limb.getLastPosition()); } }		

		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			limb.getTexture(),
			NULL, &limbRect,
			limbAngle, NULL, SDL_FLIP_NONE
		);
	}
}

/* Draw one individual tile of the map.
* Calculation of the target SDL_Rect is done in the function that calls this function.
*/
void MapScreen::drawBlock(UI& ui, Block& block, SDL_Rect targetRect) {
	/* always draw floor. */
	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		getFloorTexture(block.getFloorTextureIndex()),
		NULL, &targetRect,
		0, NULL, SDL_FLIP_NONE);
	
	/* Draw wall if it's not a floor. */
	if (!block.getIsFloor()) {
		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			getWallTexture(block.getWallTextureIndex()),
			NULL, &targetRect,
			0, NULL,
			block.getWallIsFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
		);
	}
	else if (block.getIsPath()) {
		int pathFlipOption = block.getPathFlipOption();
		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			map.getPathTexture(block.getPathTextureIndex()),
			NULL, &targetRect,
			block.getPathRotateAngle(),
			NULL,
			pathFlipOption == 2 ? SDL_FLIP_VERTICAL : pathFlipOption == 1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE
		);
	}

}

/* No need to check the location of each block,
* since we can select for blocks within range based on their vector index.
*/
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

	/* start with the TOP ROW that we want to draw, then work our way down. */
	for (int y = drawStartY; y < drawStartY + yViewRes; ++y) {
		vector<Block>& blocks = rows[y];
		/* Start with the left-most BLOCK that we want to draw, then work our way across. */
		for (int x = drawStartX; x < drawStartX + xViewRes; ++x) {
			if (animate && animationType == AnimationType::Player) {

				/* Shifting DOWN or UP. */
				if (drawStartY > lastDrawStartY) {
					targetRect.y = ((y - lastDrawStartY) * blockWidth) - blockAnimationIncrement; }
				else if (drawStartY < lastDrawStartY) {
					targetRect.y = ((y - lastDrawStartY) * blockWidth) + blockAnimationIncrement; }
				else {
					/* No vertical shift. */
					targetRect.y = (y - drawStartY) * blockWidth; }

				/* Shifting RIGHT or LEFT. */
				if (drawStartX > lastDrawStartX) {
					targetRect.x = ((x - lastDrawStartX) * blockWidth) - blockAnimationIncrement; }
				else if (drawStartX < lastDrawStartX) {
					targetRect.x = ((x - lastDrawStartX) * blockWidth) + blockAnimationIncrement; }
				else {
					/* No horiztonal shift. */
					targetRect.x = (x - drawStartX) * blockWidth; }
			}
			else {
				targetRect.x = (x - drawStartX) * blockWidth;
				targetRect.y = (y - drawStartY) * blockWidth; }

			drawBlock(ui, blocks[x], targetRect);
		}
	}

	/*
	* During animations, we need to add extra blocks to the side that's about to get cut off.
	*/
	if (animate) {
		/* Add blocks at the TOP */
		if (drawStartY > lastDrawStartY) {
			vector<Block>& blocks = rows[lastDrawStartY];

			for (int x = 0; x < blocks.size(); ++x) {
				Block& block = blocks[x];
				targetRect.x = (x - drawStartX) * blockWidth;
				targetRect.y = 0 - blockAnimationIncrement;
				drawBlock(ui, blocks[x], targetRect);
			}
		} else if (drawStartY < lastDrawStartY && lastDrawStartY <= vBlocksTotal - yViewRes) {
			/* Add blocks at the BOTTOM */
			int bottomRowIndex = drawStartY + yViewRes;
			vector<Block>& blocks = rows[bottomRowIndex];

			for (int x = 0; x < blocks.size(); ++x) {
				Block& block = blocks[x];
				targetRect.x = (x - drawStartX) * blockWidth;
				targetRect.y = ((bottomRowIndex - lastDrawStartY) * blockWidth) + blockAnimationIncrement;
				drawBlock(ui, blocks[x], targetRect);
			}
		}
		else if (drawStartX > lastDrawStartX) {
			/* moving character to the right,  shifting map to the left, add a column on the left */
			for (int y = drawStartY; y < drawStartY + yViewRes; ++y) {
				Block& block = rows[y][lastDrawStartX];
				targetRect.x = 0 - blockAnimationIncrement;
				targetRect.y = (y - drawStartY) * blockWidth;
				drawBlock(ui, block, targetRect);
			}
		}
		else if (drawStartX < lastDrawStartX) {
			/* shifting map to the right, add a column on the right */
			int rightColumnIndex = lastDrawStartX + xViewRes - 1;
			for (int y = drawStartY; y < drawStartY + yViewRes; ++y) {
				Block& block = rows[y][rightColumnIndex];
				targetRect.x = ((xViewRes * blockWidth) - blockWidth) + blockAnimationIncrement;
				targetRect.y = (y - drawStartY) * blockWidth;
				drawBlock(ui, block, targetRect);
			}
		}
	}
}

/*
* Sets the top left block for the camera.Cannot be less than 0,0.
* cannot be less than 0,0.
* cannot be more than end-of-list minus resolution.
*/
void MapScreen::setDrawStartBlock() {
	MapCharacter& playerCharacter = map.getPlayerCharacter();
	int playerX = playerCharacter.getBlockX();
	int playerY = playerCharacter.getBlockY();

	/* get the IDEAL position for the camera (with the player in the center) */
	int idealX = playerX - (xViewRes / 2);
	int idealY = playerY - (yViewRes / 2);

	/* save the camera's recent position (in case this function is called because it changed) */
	lastDrawStartX = drawStartX;
	lastDrawStartY = drawStartY;

	/* New drawStart location should be half the screen away from character (to keep character in center)
	* unless the character is close to either edge, in which case we use maxDrawStart or 0. */
	drawStartX = idealX >= 0 && idealX <= maxDrawStartX ? idealX : idealX > maxDrawStartX ? maxDrawStartX : 0;
	drawStartY = idealY >= 0 && idealY <= maxDrawStartY ? idealY : idealY > maxDrawStartY ? maxDrawStartY : 0;
}


void MapScreen::startAnimationCountdown(AnimationType iType) {
	animate = true;
	animationCountdown = waitSpin ? 18 : animationIncrementFraction;
	animationType = iType;
}

void MapScreen::decrementCountdown() {
	if (animationCountdown > 0) {
		--animationCountdown; }
	else {
		animationCountdown = 0;}
}


bool MapScreen::checkNpcOnLimbCollision() {
	bool collisionFound = false;
	vector<Limb>& roamingLimbs = map.getRoamingLimbs();

	for (MapCharacter& npc : map.getNPCs()) {
		int npcID = npc.getId();
		for (int i = roamingLimbs.size() - 1; i >= 0; --i) {
			Limb& roamingLimb = roamingLimbs[i];
			Point npcPosition = npc.getPosition();

			if (npcPosition.equals(roamingLimb.getPosition())) {
				collisionFound = true;

				roamingLimb.setCharacterId(npcID);
				npc.addLimb(roamingLimb);
				updateLimbOwner(roamingLimb.getId(), npcID);

				if (blockIsDrawable(npc.getPosition())) {
					/* Make object for limb collision animation. */
					SDL_Rect diffRect = { 0, 0, 0, 0 };
					npc.getAcquiredLimbStructs().emplace_back(
						roamingLimb.getTexture(),
						limbCollisionCountdown,
						roamingLimb.getRotationAngle(),
						diffRect
					);
				}
				

				roamingLimbs.erase(roamingLimbs.begin() + i);

				/* NPC has the limb. Now rebuild the NPC. */
				npc.clearSuit();
				npc.sortLimbsByNumberOfJoints();
				vector<Limb>& npcLimbs = npc.getLimbs();
				bool keepEquippingLimbs = true;

				for (Limb& limb : npcLimbs) {
					if (keepEquippingLimbs) {
						keepEquippingLimbs = npc.equipLimb(limb.getId());
					}
					else { break; }
				}

				npc.buildDrawLimbList();
				updateCharacterLimbs(npc.getId(), npc.getAnchorLimbId(), npcLimbs);
				npc.setTexture(npc.createAvatar());
				npc.setHomePosition(npcPosition);
				updateNpcHomePosition(npcID, npc.getHomePosition());
			}
		}
	}


	return collisionFound;
}


/* Limb-on-Limb Collision Functions (NPC creation). */

/*
* We always create a CollidedLimbsStruct with two limbs, because the check is always 1-on-1.
* Once the CollidedLimbsStruct exists, and we keep checking against the rest of the limbs,
* we can add more limbs to the limbIDs vector. So ultimately this object may contain multiple limbs.
*/
struct CollidedLimbsStruct {
	CollidedLimbsStruct(Point point, int limbID_1, int limbID_2) : point(point), limbIDs({ limbID_1, limbID_2 }) { }

	Point point;
	vector<int> limbIDs;
};

/*
* Compare each Limb's location with each other Limb's location to see if they're on the same block.
* Any on the same block form a new NPC.
*/
bool MapScreen::checkLimbOnLimbCollision() {
	vector<CollidedLimbsStruct> collidedLimbsStructs;
	vector<int> collidedIDs; /* Only add the COMPARED ids, not Base Limbs. To skip comparisons for already-collided limbs. */
	bool collisionFound = false;

	/* 
	* PROCESS:
	* Search roamingLimbs vector.
	* Start at [0] (and skip last item) and check each limb of HIGHER index for a match (with a nested loop).
	* If a match is found, create a CollidedLimbsStruct with both ids (and their Point), and add the HIGHER id to the alreadyCheckedIds vec to skip later.
	*/

	vector<Limb>& roamingLimbs = map.getRoamingLimbs();

	for (int i = 0; i < roamingLimbs.size() - 1; ++i) {
		Limb baseLimb = roamingLimbs[i];
		Point basePoint = baseLimb.getPosition();

		for (int k = i + 1; k < roamingLimbs.size(); ++k) {
			Limb comparedLimb = roamingLimbs[k];

			/* Make sure we haven't already added this limb from a previous comparison. */
			bool comparedLimbAlreadyChecked = false;
			int comparedLimbId = comparedLimb.getId();

			for (int checkedID : collidedIDs) {
				if (checkedID == comparedLimbId) {
					comparedLimbAlreadyChecked = true;
					break; } }

			if (comparedLimbAlreadyChecked) { continue; }

			/* This limb has not collided yet. Let's check for collisions. */
			Point comparedPoint = comparedLimb.getPosition();
			bool comparedLimbFoundMatch = false;

			if (comparedPoint.x == basePoint.x && comparedPoint.y == basePoint.y) {
				/* We found a match. Add it to the limbs to skip (for next time). */
				collidedIDs.push_back(comparedLimbId);

				if (!comparedLimbFoundMatch) {
					/* FIRST match for this Point (tile). */
					collidedLimbsStructs.emplace_back(
						Point(comparedPoint.x, comparedPoint.y),
						baseLimb.getId(),
						comparedLimbId);
				}
				else {
					/* 
					* This baseLimb already found a match during the i loop, and now we add comparedLimb ID to that struct.
					* Check the existing collidedLimbStructs for the existing Point record.
					*/
					for (int m = collidedLimbsStructs.size() - 1; m >= 0; --m) {
						CollidedLimbsStruct collidedLimbsStruct = collidedLimbsStructs[m];
						if (collidedLimbsStruct.point.x == comparedPoint.x && collidedLimbsStruct.point.y == comparedPoint.y) {
							collidedLimbsStruct.limbIDs.push_back(comparedLimbId);
							break;
						}
					}
				}

				comparedLimbFoundMatch = true;
				collisionFound = true;
			}
		}
	}

	/* We have collected data on how many NPCs need to be made.
	* Now we must make the NPCs, add the limbs to the NPCs, and remove the limbs from the roamingLimbs vector.
	*/

	for (CollidedLimbsStruct collidedLimbsStruct : collidedLimbsStructs) {
		/* 
		* First create the NPC as an object,
		* then save to the DB to get the ID,
		* then add the limbs to the object AND save their ownership to the NPC,
		* then remove the limbs from the Roaming Limbs list (maybe OUTSIDE of this loop).
		*/

		MapCharacter npc = MapCharacter(CharacterType::Hostile);
		string npcName = "Forest Creature"; /* TO DO: Create a system for giving them names based on their limbs or map. */
		
		npc.setBlockPosition(collidedLimbsStruct.point);
		npc.updateLastBlock();
		npc.setHomePosition(collidedLimbsStruct.point);
		npc.startNewNpcCountup();

		int npcID = createNpcOnMap(map.getSlug(), npcName, collidedLimbsStruct.point);
		npc.setId(npcID);
		bool npcIsDrawable = blockIsDrawable(npc.getPosition());

		for (int limbID : collidedLimbsStruct.limbIDs) {
			for (int i = roamingLimbs.size() - 1; i >= 0; --i) {
				Limb& limb = roamingLimbs[i];
				if (limb.getId() == limbID) {
					limb.setCharacterId(npcID);
					npc.addLimb(limb);
					updateLimbOwner(limbID, npcID);

					/* The acquired limbs should animate. */
					if (npcIsDrawable) {
						/* Make object for limb collision animation. */
						SDL_Rect diffRect = { 0, 0, 0, 0 };
						npc.getAcquiredLimbStructs().emplace_back(
							limb.getTexture(),
							limbCollisionCountdown,
							limb.getRotationAngle(),
							diffRect
						);
					}

					/* Character has limb. Now remove limb from map. */
					roamingLimbs.erase(roamingLimbs.begin() + i);
				}
			}
		}

		/* NPC has all their limbs.
			* Now snap them together. DONE
			* --- sort NPCs limbs by number of joints. DONE
			* Then make a texture. DONE
			* Then draw the texture on each frame.
			* Then load the NPCs when map loads.
			*/
		npc.sortLimbsByNumberOfJoints();
		vector<Limb>& npcLimbs = npc.getLimbs();
		bool keepEquippingLimbs = true;

		for (Limb& limb : npcLimbs) {
			if (keepEquippingLimbs) {
				keepEquippingLimbs = npc.equipLimb(limb.getId());
				
			} else { break; }
		}

		npc.buildDrawLimbList();
		updateCharacterLimbs(npcID, npc.getAnchorLimbId(), npcLimbs);
		npc.setTexture(npc.createAvatar());

		map.addNPC(npc);
	}

	return collisionFound;
}

/* Limb Collision Animation Functions. */


/*
*
* WHEN the player lands on a LIMB (or a limb lands on a player):
* --> Add limb to player inventory (map.getPlayerCharacter().addLimb(limb);
* --> Remove limb from roamingLimbs (  roamingLimbs.erase(roamingLimbs.begin() + index);  )
* --> Animate the limb (grows BIGGER while spinning in a circle, then grows VERY SMALL (still spinning) and disappears)
* --> THEN either let the LIMBS/NPCs move, or it's the player's turn (MUST add new ENUM for whose TURN it is, or find some other way to indicate turn).
*
* Do we need a collision animation queue?
* We need to check both NPC and Limb collisions BEFORE starting either animation.
*
* ANIMATION:
* -- We will NOT hold up gameplay for these animations.
* --- Instead, make a vector of structs which hold Limb texture, rect, rotationAngle, and Countdown.
* --- The rotationAngle of the Limb will start wherever it was, then increment by 15 degrees each iteration.
* --- A function will decrement the countdown, increment (and then decrement) the size of the limb's rect,
* --- and remove items after they reach zero.
*
*/
bool MapScreen::checkPlayerLimbCollision() {
	bool collisionDetected = false;
	Point playerPosition = map.getPlayerCharacter().getPosition();

	for (int i = map.getRoamingLimbs().size() - 1; i >= 0; --i) {
		if (map.getRoamingLimbs()[i].getPosition().equals(playerPosition)) {
			collisionDetected = true;
			Limb& thisLimb = map.getRoamingLimbs()[i];
			GameState& gameState = GameState::getInstance();
			int playerCharID = gameState.getPlayerID();

			/* Move limb from map to character. */
			MapCharacter& playerCharacter = map.getPlayerCharacter();
			vector<Limb>& roamingLimbs = map.getRoamingLimbs();
			playerCharacter.addLimb(thisLimb);
			updateLimbOwner(thisLimb.getId(), playerCharID);
			thisLimb.setCharacterId(playerCharID);

			/* Make object for limb collision animation. */
			SDL_Rect diffRect = { 0, 0, 0, 0 };
			playerCharacter.getAcquiredLimbStructs().emplace_back(
				thisLimb.getTexture(),
				limbCollisionCountdown,
				thisLimb.getRotationAngle(),
				diffRect
			);

			/* Remove from list at the end to avoid changing the item referenced by i. */
			roamingLimbs.erase(roamingLimbs.begin() + i);
		}
	}
	return collisionDetected;
}


bool MapScreen::checkPlayerNpcCollision() {
	bool isCollided = false;

	for (MapCharacter& npc : map.getNPCs()) {
		if (npc.getPosition().equals(map.getPlayerCharacter().getPosition())) {
			cout << "Player collided with " << npc.getName() << "\n";

			/*
			* In Database create a Battle with player id and opponent id.
			* Update player and opponent table with new battle id.
			* Change the screen type in the GameState and end this loop.
			*/

			return true;
		}
	}

	return false;
}

/*
*  MAP SCREEN NAVIGATION FUNCTIONS
*	We must check:
*		A)	if we are moving out of bounds. DONE
*		B)	if the player character has hit a wall.
*		C)	if the player character has hit a door, or another character.
* 
*		TODO:	Make a "hit a wall" function (to play a sound)
*				Do an animated transition for these moves (maybe).
*/

void MapScreen::moveCharacter(MapDirection direction) {
	/* Change the character's position */
	bool moved = map.getPlayerCharacter().move(direction);
	if (moved) {
		/* Change the block to draw based on the character's new position. */
		setDrawStartBlock();
		/* Instead of immediately displaying the move, we start a move animation. */
		startAnimationCountdown(AnimationType::Player);
		updatePlayerMapLocation(map.getSlug(), map.getPlayerCharacter().getPosition());
	}
}

/* destailed documentation for the first function. Other functions follow same process. */
void MapScreen::requestUp() {
	if (animate || animationCountdown > 0) { return; }
	/* Get player character */
	MapCharacter& playerCharacter = map.getPlayerCharacter();
	/* get the NEW index of the block they want to move to (along the dimension of change) */
	int destinationBlockY = playerCharacter.getBlockY() - 1;
	/* Make sure they're not going to move out of bounds */
	if (destinationBlockY >= 0) {
		/* get the new block they want to move to */
		Block& destinationBlock = map.getRows()[destinationBlockY][playerCharacter.getBlockX()];
		/* If the new block is a floor, move there */
		if (destinationBlock.getIsFloor()) {
			moveCharacter(MapDirection::Up);
		}
	}
}

void MapScreen::requestDown() {
	if (animate || animationCountdown > 0) { return; }
	MapCharacter& playerCharacter = map.getPlayerCharacter();
	int destinationBlockY = playerCharacter.getBlockY() + 1;
	if (destinationBlockY < vBlocksTotal) {
		Block& destinationBlock = map.getRows()[destinationBlockY][playerCharacter.getBlockX()];
		if (destinationBlock.getIsFloor()) {
			moveCharacter(MapDirection::Down);
		}
	}
}

void MapScreen::requestLeft() {
	if (animate || animationCountdown > 0) { return; }
	MapCharacter& playerCharacter = map.getPlayerCharacter();
	int destinationBlockX = playerCharacter.getBlockX() - 1;
	if (destinationBlockX < hBlocksTotal) {
		Block& destinationBlock = map.getRows()[playerCharacter.getBlockY()][destinationBlockX];
		if (destinationBlock.getIsFloor()) {
			moveCharacter(MapDirection::Left);
		}
	}
}

void MapScreen::requestRight() {
	if (animate || animationCountdown > 0) { return; }
	MapCharacter& playerCharacter = map.getPlayerCharacter();
	int destinationBlockX = playerCharacter.getBlockX() + 1;
	if (destinationBlockX < hBlocksTotal) {
		Block& destinationBlock = map.getRows()[playerCharacter.getBlockY()][destinationBlockX];
		if (destinationBlock.getIsFloor()) {
			moveCharacter(MapDirection::Right);
		}
	}
}

/* Player wants to let the limbs and NPCs move one turn. */
void MapScreen::waitTurn() {
	/* Set spin in motion. */
	waitSpin = true;
	map.getPlayerCharacter().updateLastBlock();
	startAnimationCountdown(AnimationType::Player);
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
			handleKeydown(e);
		}
		else if (e.type == SDL_MOUSEBUTTONDOWN) {
			handleMousedown(e, running, settingsPanel, gameMenuPanel);
		}
	}
}

/* User pressed a keyboard button. */
void MapScreen::handleKeydown(SDL_Event& e) {
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
	case SDLK_SPACE:
		waitTurn();
		break;
	default:
		cout << e.key.keysym.sym << " KEY PRESSED" << "\n";
	}

	// cout << "Block Width: " << blockWidth << "\n";

}


/* User clicked the mouse. */
void MapScreen::handleMousedown(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel) {
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
			/* Switch to other panel. */
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
		ButtonClickStruct clickStruct = gameMenuPanel.checkButtonClick(mouseX, mouseY);
		UI& ui = UI::getInstance();
		/* see what button might have been clicked : */
		switch (clickStruct.buttonOption) {
		case ButtonOption::MapOptions:
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

void MapScreen::checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (gameMenuPanel.getShow()) { gameMenuPanel.checkMouseOver(mouseX, mouseY); }
}