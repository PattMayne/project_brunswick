/*
* Specific maps (levels), limbs, and suits (pure unscrambled characters) will be hardcoded and defined here.
* Map includes Blocks and Landmarks.
* Blocks will ALWAYS come from the DB (no need for BlockData struct).
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

module;
#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <unordered_map>
#include <functional>

export module CharacterFactory;

using namespace std;

import CharacterClasses;
import TypeStorage;


export struct LimbData {
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


export struct SuitData {
	string name;
	string slug;
	bool unscrambled;
	vector<LimbPlacement> limbPlacements; /* A suit is abstract. It is NOT a character. It holds information to build an abstract base character. */
};


export struct MapData {
	string name;
	string slug;
	vector<LimbData> limbs; /* We will need some limbs to be "free" and NOT part of a Suit. So the suits will simply refer to the slugs of the limbs, not contain the limbs. */
	vector<SuitData> suits;
	// WILL NEED A VECTOR OF BLOCKS. This will require moving Block class from MapScreen into this module.
	// Then we will extend the block class (or include a struct.
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
	* 
	* 
	* BETTER IDEA:
	* 
	* The unordered_map will hold FUNCTIONS instead of objects.
	* These functions will instantiate an object only when the key is accessed:
				unordered_map<string, function<Limb()> baseLimbMap;
	* 
	* We can always check for the existence of a key like this:
	

		if (baseLimbMap.find("dolly") != baseLimbMap.end()) {
			MyObject dolly = baseLimbMap["dolly"](); // Instantiate when accessing
			dolly.display();
		} else {
			std::cout << "'dolly' not found.\n";
		}


	* 
	* ALSO.... I don't want "magic strings" for slug names.
	* There should be a master list of unordered_map<string, string>: "SLUG_NAME", "slug_name"
	* That way they're always SET by something in the master list (though they can be accessed without accessing the slug list)
	* 
	* 
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
		//data.texture = // get texture from IMG // NO... this should only hold the path to the file.
		data.joints = {
			Point(144, 81)
		};

		return data;
	}

	unordered_map<string, Limb> baseLimbMap;

	string name = "name";
	vector<Point> joints = {
		Point(3,3)
	};

	baseLimbMap["dolly"] = Limb(name, 5, 5, 5, 5, DominanceNode::Green, true, joints);

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

