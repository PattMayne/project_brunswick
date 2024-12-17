/*
* Specific maps (levels), limbs, and suits (pure unscrambled characters) will be hardcoded and defined here.
* Later if I want to use JSON instead (to let non-programmers make levels and characters) then I can just plug the JSON (or whatever) into this file.
* The rest of the program won't have to care where this Factory gets its info.
* Hard-coding for now lets me develop quicker. But it also guarantees type safety.
*/

/*
* TO DO:
* 1 - STRUCTs defining data of LIMB, CHARACTER, and MAP. (does this make base classes useless? MAYBE... maybe not... we want the SAVE function... but structs can have that?)
* 2 - DEFINITIONS of specific LIMBs, SUITs, and MAP.
* 3 - Load these into the Character Creation Screen.
*/

module;
#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <unordered_map>

export module CharacterFactory;

using namespace std;

import CharacterClasses;
import TypeStorage;

struct LimbData {
	string name;
	string slug;
	int attack;
	int speed;
	int weight;
	int intelligence;
	DominanceNode dNode;
	vector<Point> joints;
	SDL_Texture* texture;
};

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

/* 
* TAKES a slug identifier and returns a limb object or struct DEFINED IN THE FUNCTION.
* OR : should each MAP be a function which contains all its objects, and can return EITHER the full map OR just its pieces ?
*/

export LimbData baseLimbData(string slug) {

	/*
	* NEW METHOD. Forget defining them here.
	* Make a huge unordered_map instead!
	* The SLUG is the KEY in the UNORDERED_MAP.
	* 
	* Also, maybe each MAP has its own unordered_map of Limb objects?
	* That way we don't need to load EVERY limb whenever we want to find ONE?
	* And this function can cycle through the maps, check each of their lists,
	* and finally deliver the right one?
	* 
	* DEEPAI suggests using an unordered_map. And apparently 500 items is not too many.
	*/

	if (slug == "deer_leg_4") {
		LimbData data;
		data.slug = slug;
		data.name = "Deer Leg 4";
		data.attack = 5;
		data.speed = 10;
		data.weight = 7;
		data.intelligence = 3;
		data.dNode = DominanceNode::Green;
		//data.texture = // get texture from IMG
		data.joints = {
			Point(144, 81)
		};

		return data;
	}

}