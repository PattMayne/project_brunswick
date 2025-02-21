/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~  _____ ___  ____  __  __						  ~
* ~ |  ___/ _ \|  _ \|  \/  |						  ~
* ~ | |_ | | | | |_) | |\/| |						  ~
* ~ |  _|| |_| |  _ <| |  | |						  ~
* ~ |_|   \___/|_| \_\_|  |_|						  ~
* ~  _____ _    ____ _____ ___  ____  ___ _____ ____  ~
* ~ |  ___/ \  / ___|_   _/ _ \|  _ \|_ _| ____/ ___| ~
* ~ | |_ / _ \| |     | || | | | |_) || ||  _| \___ \ ~
* ~ |  _/ ___ \ |___  | || |_| |  _ < | || |___ ___) |~
* ~ |_|/_/   \_\____| |_| \___/|_| \_\___|_____|____/ ~
* ~                                                   ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 
* 
* 
* 
* Specific, basic forms of maps (levels), limbs, and suits (pure unscrambled characters) will be hardcoded and defined here.
* Map includes Blocks and Landmarks.
* Blocks will ALWAYS come from the DB (no need for BlockForm struct).
* Landmarks never change, except for their position in the map.
* Later if I want to use JSON instead (to let non-programmers make levels and characters) then I can just plug the JSON (or whatever) into this file.
* The rest of the program won't have to care where this Factory gets its info.
* Hard-coding for now lets me develop quicker. But it also guarantees type safety.
* 
* Whenever we actually load a Map (with its landmarks, blocks, limbs, and suits) we will FIRST load the BASE/VANILLA version, and then OVERRIDE it with DB data (if exists).
*/

/*
* TO DO:
* 1 - STRUCTs defining data of LIMB, CHARACTER, and MAP. (does this make base classes useless? MAYBE... maybe not... we want the SAVE function... but structs can have that?)
* 2 - DEFINITIONS of specific LIMBs, SUITs, and MAP.
* 3 - Load these into the Character Creation Screen.
* 
*/



export module FormFactory;

import "SDL.h";
import "SDL_image.h";
import <string>;
import <iostream>;
import <vector>;
import <unordered_map>;

import CharacterClasses;
import LimbFormMasterList;
import TypeStorage;
import UI;

using namespace std;

void equipForestSuitLimbs(vector<Character>& forestSuits);

/*

This is the big, major push.
The entire app changes now. I currently am using a default, vanilla, static "map" to just represent basic functionality.
But now I'm setting up Dynamic Maps. I need to pull from basic Forms first, hard-coded yet dynamic (because each map is a level,
each Limb has a type) and then override them with Database information.


FIRST GOAL:

-- make a FOREST MAP form, complete with textures, use it to populate the actual Map Screen map.
^^DONE^^

SECOND GOAL:

-- add two SUITS
-- make SHRINES from those SUITS
-- scatter LIMBS around

For now we won't save anything to any Database.
We'll just make everything a one-off, based on FORM DEFINITIONS.

Right now the maps are all SQUARE.
Expand this later to lat them be RECTANGLES.


*/

/*
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~  _____ ___  ____  __  __                            ~
* ~ |  ___/ _ \|  _ \|  \/  |                           ~
* ~ | |_ | | | | |_) | |\/| |                           ~
* ~ |  _|| |_| |  _ <| |  | |                           ~
* ~ |_|_ _\___/|_|_\_\_|_ |_|                           ~
* ~  ___ _   _ ____ _____  _    _   _  ____ _____ ____  ~
* ~ |_ _| \ | / ___|_   _|/ \  | \ | |/ ___| ____/ ___| ~
* ~  | ||  \| \___ \ | | / _ \ |  \| | |   |  _| \___ \ ~
* ~  | || |\  |___) || |/ ___ \| |\  | |___| |___ ___) |~
* ~ |___|_| \_|____/ |_/_/   \_\_| \_|\____|_____|____/ ~
* ~                                                     ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 
* The functions which create the actual FORM structs for the levels of the game.
* Sub-maps (dungeons and buildings) will not have suits or their own limbs.
* Their native limbs will draw from their parent map (MapLevel).
* Sub-maps CAN have their own NPCs and landmarks though.
* 
*/


/*
*			FOREST MAP STRUCTS
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~  _____ ___  ____  _____ ____ _____        ~
* ~ |  ___/ _ \|  _ \| ____/ ___|_   _|       ~
* ~ | |_ | | | | |_) |  _| \___ \ | |         ~
* ~ |  _|| |_| |  _ <| |___ ___) || |         ~
* ~ |_|  _\___/|_| \_\_____|____/ |_|         ~
* ~ |  \/  |  / \  |  _ \                     ~
* ~ | |\/| | / _ \ | |_) |                    ~
* ~ | |  | |/ ___ \|  __/                     ~
* ~ |_|__|_/_/_ _\_\_|_   _  ____ _____ ____  ~
* ~ / ___|_   _|  _ \| | | |/ ___|_   _/ ___| ~
* ~ \___ \ | | | |_) | | | | |     | | \___ \ ~
* ~  ___) || | |  _ <| |_| | |___  | |  ___) |~
* ~ |____/ |_| |_| \_\\___/ \____| |_| |____/ ~
* ~                                           ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


MapForm forestMap() {
	UI& ui = UI::getInstance();
	MapForm forestMap;
	forestMap.mapLevel = MapLevel::Forest;
	forestMap.name = "Enchanted Forest";
	forestMap.slug = "forest";
	forestMap.blocksWidth = 50;
	forestMap.blocksHeight = 50;
	forestMap.mapType = MapType::World;

	/* create the TEXTURES */

	/* FLOOR surfaces. */
	SDL_Surface* floorSurface001 = IMG_Load("data/maps/forest/floor_001.png");
	SDL_Surface* floorSurface002 = IMG_Load("data/maps/forest/floor_002.png");
	SDL_Surface* floorSurface003 = IMG_Load("data/maps/forest/floor_003.png");

	/* WALL surfaces. */
	SDL_Surface* wallSurface001 = IMG_Load("data/maps/forest/wall_001.png");
	SDL_Surface* wallSurface002 = IMG_Load("data/maps/forest/wall_002.png");
	SDL_Surface* wallSurface003 = IMG_Load("data/maps/forest/wall_003.png");

	/* PATH surfaces. */
	SDL_Surface* pathSurface001 = IMG_Load("data/maps/forest/path_001.png");
	SDL_Surface* pathSurface002 = IMG_Load("data/maps/forest/path_002.png");
	SDL_Surface* pathSurface003 = IMG_Load("data/maps/forest/path_003.png");

	/* DO ERROR CHECKS */

	forestMap.floorTextures = {
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface001),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface002),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), floorSurface003)
	};

	forestMap.wallTextures = {
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface001),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface002),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), wallSurface003)
	};

	forestMap.pathTextures = {
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), pathSurface001),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), pathSurface002),
		SDL_CreateTextureFromSurface(ui.getMainRenderer(), pathSurface003)
	};


	SDL_FreeSurface(floorSurface001);
	SDL_FreeSurface(floorSurface002);
	SDL_FreeSurface(floorSurface003);

	SDL_FreeSurface(wallSurface001);
	SDL_FreeSurface(wallSurface002);
	SDL_FreeSurface(wallSurface003);

	SDL_FreeSurface(pathSurface001);
	SDL_FreeSurface(pathSurface002);
	SDL_FreeSurface(pathSurface003);

	forestMap.nativeLimbs = getMapLimbs(forestMap.mapLevel);

	// SUITS will come later.

	// Make the deer first.
	// Get copies of the deer's limbs from nativeLimbs.

	vector<Limb> deerLimbs;
	vector<Limb> bearLimbs;
	vector<Limb> spiderLimbs;
	vector<Limb> fairyLimbs;
	vector<Limb> owlLimbs;

	for (LimbForm limbForm : forestMap.nativeLimbs) {

		/* DEER LIMBS */

		if (
			limbForm.slug == "deer_leg_4" ||
			limbForm.slug == "deer_leg_3" ||
			limbForm.slug == "deer_leg_2" ||
			limbForm.slug == "deer_leg_1" ||
			limbForm.slug == "deer_head" ||
			limbForm.slug == "deer_body" ||
			limbForm.slug == "deer_antler_1" ||
			limbForm.slug == "deer_antler_2"
			)
		{
			deerLimbs.emplace_back(limbForm);
		}
		else if (
			limbForm.slug == "bear_head" ||
			limbForm.slug == "bear_body" ||
			limbForm.slug == "bear_arm_left" ||
			limbForm.slug == "bear_arm_right" ||
			limbForm.slug == "bear_leg_left" ||
			limbForm.slug == "bear_leg_right"
			)
		{
			bearLimbs.emplace_back(limbForm);
		}
		else if (
			limbForm.slug == "spider_body" ||
			limbForm.slug == "spider_leg_1" ||
			limbForm.slug == "spider_leg_2" ||
			limbForm.slug == "spider_leg_3" ||
			limbForm.slug == "spider_leg_4" ||
			limbForm.slug == "spider_leg_5" ||
			limbForm.slug == "spider_leg_6"
			)
		{
			spiderLimbs.emplace_back(limbForm);
		}
		else if (
			limbForm.slug == "fairy_head" ||
			limbForm.slug == "fairy_body" ||
			limbForm.slug == "fairy_wing_right" ||
			limbForm.slug == "fairy_wing_left" ||
			limbForm.slug == "fairy_leg_right" ||
			limbForm.slug == "fairy_leg_left" ||
			limbForm.slug == "fairy_arm_left" ||
			limbForm.slug == "fairy_arm_right"
			)
		{
			fairyLimbs.emplace_back(limbForm);
		}
		else if (
			limbForm.slug == "owl_body" ||
			limbForm.slug == "owl_head" ||
			limbForm.slug == "owl_wing_left" ||
			limbForm.slug == "owl_wing_right"
			)
		{
			owlLimbs.emplace_back(limbForm);
		}
	}

	forestMap.suits.emplace_back(CharacterType::Suit, deerLimbs, "Deer", SuitType::Deer);
	forestMap.suits.emplace_back(CharacterType::Suit, bearLimbs, "Bear", SuitType::Bear);
	forestMap.suits.emplace_back(CharacterType::Suit, spiderLimbs, "Spider", SuitType::Spider);
	forestMap.suits.emplace_back(CharacterType::Suit, fairyLimbs, "Fairy", SuitType::Fairy);
	forestMap.suits.emplace_back(CharacterType::Suit, owlLimbs, "Owl", SuitType::Owl);




	return forestMap;
}

/* Get the basic Map data struct based on slug ID parameter value. */
export MapForm getMapFormFromSlug(string slug) {
	if (slug == "forest") {
		return forestMap();
	}
	cout << "string check FAILED\n";
	// Temporary DEFAULT map... deal with error (faulty slug) somehow instead...
	return forestMap();
}

export void equipSuitLimbs(MapLevel mapLevel, vector<Character>& suits) {
	if (mapLevel == MapLevel::Forest) {
		equipForestSuitLimbs(suits);
	}
}

export void equipForestSuitLimbs(vector<Character>& forestSuits) {

	/* HERE we must EQUIP in the correct and explicit order. THEN set the DRAW order. */

	/* Equip Deer limbs. */

	Character& deer = forestSuits[0]; /* It's probably this one, but we'll still search and get it by suit type. */
	for (Character& suit : forestSuits) {
		if (suit.getSuitType() == SuitType::Deer) {
			deer = suit;
		}
	}

	/* Equip BODY first. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getBodyPartType() == BodyPartType::Torso) {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(0);
			break;
		}
	}

	/* Equip HEAD second. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getBodyPartType() == BodyPartType::Head) {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(1);
			break;
		}
	}

	/* Equip Leg 4. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_leg_4") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(2);
			break;
		}
	}

	/* Equip Leg 3. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_leg_3") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(3);
			break;
		}
	}

	/* Equip Leg 1. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_leg_1") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(4);
			break;
		}
	}

	/* Equip Leg 2. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_leg_2") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(5);
			break;
		}
	}

	/* Equip deer_antler_1. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_antler_1") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(6);
			break;
		}
	}

	/* Equip deer_antler_2. */
	for (Limb& limb : deer.getLimbs()) {
		if (limb.getFormSlug() == "deer_antler_2") {
			deer.equipLimb(limb.getId());
			limb.setDrawOrder(7);
			break;
		}
	}



	/* Equip Bear limbs. */

	Character& bear = forestSuits[1];
	for (Character& suit : forestSuits) {
		if (suit.getSuitType() == SuitType::Bear) {
			bear = suit;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getBodyPartType() == BodyPartType::Torso) {
			limb.setDrawOrder(0);
			bear.equipLimb(limb.getId());
			break;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getBodyPartType() == BodyPartType::Head) {
			limb.setDrawOrder(5);
			bear.equipLimb(limb.getId());
			break;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getFormSlug() == "bear_arm_left") {
			limb.setDrawOrder(4);
			bear.equipLimb(limb.getId());
			break;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getFormSlug() == "bear_arm_right") {
			limb.setDrawOrder(3);
			bear.equipLimb(limb.getId());
			break;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getFormSlug() == "bear_leg_left") {
			limb.setDrawOrder(2);
			bear.equipLimb(limb.getId());
			break;
		}
	}

	for (Limb& limb : bear.getLimbs()) {
		if (limb.getFormSlug() == "bear_leg_right") {
			limb.setDrawOrder(1);
			bear.equipLimb(limb.getId());
			break;
		}
	}




	/* Equip Spider limbs. */

	Character& spider = forestSuits[2];
	for (Character& suit : forestSuits) {
		if (suit.getSuitType() == SuitType::Spider) {
			spider = suit;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_body") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(4);
			break;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_3") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(0);
			break;
		}
	}
	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_2") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(1);
			break;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_1") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(5);
			break;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_4") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(6);
			break;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_5") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(2);
			break;
		}
	}

	for (Limb& limb : spider.getLimbs()) {
		if (limb.getFormSlug() == "spider_leg_6") {
			spider.equipLimb(limb.getId());
			limb.setDrawOrder(3);
			break;
		}
	}
	

	/* Equip Fairy limbs. */

	Character& fairy = forestSuits[3];
	for (Character& suit : forestSuits) {
		if (suit.getSuitType() == SuitType::Fairy) {
			fairy = suit;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_body") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(4);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_head") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(7);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_arm_left") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(5);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_wing_left") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(0);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_leg_left") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(1);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_leg_right") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(2);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_wing_right") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(3);
			break;
		}
	}

	for (Limb& limb : fairy.getLimbs()) {
		if (limb.getFormSlug() == "fairy_arm_right") {
			fairy.equipLimb(limb.getId());
			limb.setDrawOrder(6);
			break;
		}
	}



	/* Equip Owl limbs. */

	Character& owl = forestSuits[4];
	for (Character& suit : forestSuits) {
		if (suit.getSuitType() == SuitType::Owl) {
			owl = suit;
		}
	}

	for (Limb& limb : owl.getLimbs()) {
		if (limb.getFormSlug() == "owl_body") {
			owl.equipLimb(limb.getId());
			limb.setDrawOrder(2);
			break;
		}
	}

	for (Limb& limb : owl.getLimbs()) {
		if (limb.getFormSlug() == "owl_head") {
			owl.equipLimb(limb.getId());
			limb.setDrawOrder(3);
			break;
		}
	}

	for (Limb& limb : owl.getLimbs()) {
		if (limb.getFormSlug() == "owl_wing_right") {
			owl.equipLimb(limb.getId());
			limb.setDrawOrder(1);
			break;
		}
	}

	for (Limb& limb : owl.getLimbs()) {
		if (limb.getFormSlug() == "owl_wing_left") {
			owl.equipLimb(limb.getId());
			limb.setDrawOrder(0);
			break;
		}
	}


}


/*
* 
FUNCTIONS TO CREATE:

export SDL_Texture buildAvatarFromLimbs() {}

each map definition will have it collection of Suits like:

vector<Character> suits = {
	getSuit(slugName1),
	getSuit(slugName2),
	getSuit(slugName3)
}

*/