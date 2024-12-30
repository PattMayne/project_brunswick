/**
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~  _     _           _       _____                    ~
* ~ | |   (_)_ __ ___ | |__   |  ___|__  _ __ _ __ ___  ~
* ~ | |   | | '_ ` _ \| '_ \  | |_ / _ \| '__| '_ ` _ \ ~
* ~ | |___| | | | | | | |_) | |  _| (_) | |  | | | | | |~
* ~ |_____|_|_| |_| |_|_.__/  |_|  \___/|_|_ |_| |_| |_|~
* ~ |  \/  | __ _ ___| |_ ___ _ __  | |   (_)___| |_    ~
* ~ | |\/| |/ _` / __| __/ _ \ '__| | |   | / __| __|   ~
* ~ | |  | | (_| \__ \ ||  __/ |    | |___| \__ \ |_    ~
* ~ |_|  |_|\__,_|___/\__\___|_|    |_____|_|___/\__|   ~
* ~                                                     ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* 
* All Limbs will be defined here.
* No textures will be created here, but the paths to the PNGs will be coded here.
* 
* Limb objects will be created from these LimbForms.
*/

module;

export module LimbFormMasterList;

import <string>;
import <iostream>;
import <vector>;
import <cstdlib>;
import <unordered_map>;
import <functional>;

using namespace std;

import TypeStorage;
import CharacterClasses;


/*
* 
*			FOREST LIMBS
* 
*/

/* Send in an existing unordered_list, we will populate it with functions to create Forest Limb Form structs. */
void addForestLimbFormMasterList(unordered_map<string, function<LimbForm()>>& limbForms) {
	/* FOREST LIMB FORMS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["deer_leg_4"] = []() {
		return LimbForm(
			"Deer Leg 4", "deer_leg_4",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_4.png",
			{ Point(144, 81) });
		};

	limbForms["deer_leg_3"] = []() {
		return LimbForm(
			"Deer Leg 3", "deer_leg_3",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_3.png",
			{ Point(131, 71) });
		};

	limbForms["deer_leg_2"] = []() {
		return LimbForm(
			"Deer Leg 2", "deer_leg_2",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_2.png",
			{ Point(78, 46) });
		};

	limbForms["deer_leg_1"] = []() {
		return LimbForm(
			"Deer Leg 1", "deer_leg_1",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_1.png",
			{ Point(52, 43) });
		};

	limbForms["deer_head"] = []() {
		return LimbForm(
			"Deer Head", "deer_head",
			8, 4, 3, 7,
			DominanceNode::Green, "data/maps/forest/deer_head.png",
			{ Point(93, 129), Point(117, 33), Point(90, 37) });
		};

	limbForms["deer_body"] = []() {
		return LimbForm(
			"Deer Body", "deer_body",
			10, 7, 3, 3,
			DominanceNode::Green, "data/maps/forest/deer_body.png",
			{ Point(184, 86), Point(183, 130), Point(138, 138), Point(44, 112), Point(57, 114) });
		};

	limbForms["deer_antler_1"] = []() {
		return LimbForm(
			"Deer Antler 1", "deer_antler_1",
			9, 6, 6, 3,
			DominanceNode::Green, "data/maps/forest/deer_antler_1.png",
			{ Point(107, 78) });
		};

	limbForms["deer_antler_2"] = []() {
		return LimbForm(
			"Deer Antler 2", "deer_antler_2",
			9, 6, 6, 3,
			DominanceNode::Green, "data/maps/forest/deer_antler_2.png",
			{ Point(99, 86) });
		};
}


/* 
* An unordered_map of functions to create LimbForm structs.
* The structs are only created when their functions are called.
* The string key is the slug.
*/
export unordered_map<string, function<LimbForm()>> getLimbFormList(MapLevel mapLevel) {
	unordered_map<string, function<LimbForm()>> limbFormsMap;

	if (mapLevel == MapLevel::All) {
		/* ADD ALL FORMS */
		addForestLimbFormMasterList(limbFormsMap);
	} else if (mapLevel == MapLevel::Forest) {
		/* ADD FOREST LIMB FORMS */
		addForestLimbFormMasterList(limbFormsMap);
	}

	return limbFormsMap;
}


/*
* 
* 
* FUNCTIONS TO GET LIMB FORMS
* 
* 
*/


/* get a SINGLE LimbFormStruct from a SPECIFIED MapLevel. */
export LimbForm getLimbForm(string limbSlug, MapLevel mapLevel = MapLevel::All) {
	/* TO DO: error check. Make sure that the list contains the slug key. */
	return getLimbFormList(mapLevel)[limbSlug]();
}

/* get ALL the LimbFormStructs from a SPECIFIED MapLevel. */
export vector<LimbForm> getMapLimbs(MapLevel mapLevel) {
	unordered_map<string, function<LimbForm()>> limbFormsMap = getLimbFormList(mapLevel);
	vector<LimbForm> limbForms;

	for (auto kv : limbFormsMap) {
		limbForms.push_back(kv.second()); }

	return limbForms;
}

/* get a VECTOR of LimbFormStructs from ACROSS ALL MapLevels. */
export vector<LimbForm> getLimbForms(vector<string> slugs) {
	unordered_map<string, function<LimbForm()>> limbFormsMap = getLimbFormList(MapLevel::All);
	vector<LimbForm> limbForms;

	for (string slug : slugs) {
		limbForms.push_back(limbFormsMap[slug]()); }

	return limbForms;
}

/*
* 
* We need:
* 
* ONE function to get a SINGLE LimbFormStruct from a SPECIFIED MapLevel DONE
* 
* ONE function to get ALL the LimbFormStructs from a SPECIFIED MapLevel DONE
* 
* ONE function to get a VECTOR of LimbFormStructs from ACROSS ALL MapLevels DONE
* 
*/

/* ALSO add a function which takes a LIST of slugs to getLimbForms PLURAL without creating and destroying the same unordered_list repeatedly */