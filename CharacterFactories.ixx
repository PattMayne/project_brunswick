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
	//int intelligence;
	DominanceNode dNode;
	vector<Point> joints;
	SDL_Texture* texture;
};

/*
* 
FUNCTIONS TO CREATE:

TAKES a slug identifier and returns a limb object or struct DEFINED IN THE FUNCTION.
OR: should each MAP be a function which contains all its objects, and can return EITHER the full map OR just its pieces?

export Limb baseLimb(string slug) { }




export SDL_Texture buildAvatarFromLimbs() {}



each map definition will have it collection of Suits like:

vector<Character> suits = {
	getSuit(slugName1),
	getSuit(slugName2),
	getSuit(slugName3)
}

*/