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

	/* DEER LIMBS */

	limbForms["deer_leg_4"] = []() {
		return LimbForm(
			"Deer Leg 4", "deer_leg_4",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_4.png",
			{ Point(144, 81) },
			BodyPartType::Leg);
		};

	limbForms["deer_leg_3"] = []() {
		return LimbForm(
			"Deer Leg 3", "deer_leg_3",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_3.png",
			{ Point(131, 71) },
			BodyPartType::Leg);
		};

	limbForms["deer_leg_2"] = []() {
		return LimbForm(
			"Deer Leg 2", "deer_leg_2",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_2.png",
			{ Point(78, 46) },
			BodyPartType::Leg);
		};

	limbForms["deer_leg_1"] = []() {
		return LimbForm(
			"Deer Leg 1", "deer_leg_1",
			9, 6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_1.png",
			{ Point(52, 43) },
			BodyPartType::Leg);
		};

	limbForms["deer_head"] = []() {
		return LimbForm(
			"Deer Head", "deer_head",
			8, 4, 3, 7,
			DominanceNode::Green, "data/maps/forest/deer_head.png",
			{ Point(93, 129), Point(117, 33), Point(90, 37) },
			BodyPartType::Head);
		};

	limbForms["deer_body"] = []() {
		return LimbForm(
			"Deer Body", "deer_body",
			10, 7, 3, 3,
			DominanceNode::Green, "data/maps/forest/deer_body.png",
			{ Point(184, 86), Point(183, 130), Point(138, 138), Point(44, 112), Point(57, 114) },
			BodyPartType::Torso);
		};

	limbForms["deer_antler_1"] = []() {
		return LimbForm(
			"Deer Antler 1", "deer_antler_1",
			9, 6, 6, 3,
			DominanceNode::Green, "data/maps/forest/deer_antler_1.png",
			{ Point(107, 78) },
			BodyPartType::Other);
		};

	limbForms["deer_antler_2"] = []() {
		return LimbForm(
			"Deer Antler 2", "deer_antler_2",
			9, 6, 6, 3,
			DominanceNode::Green, "data/maps/forest/deer_antler_2.png",
			{ Point(99, 86) },
			BodyPartType::Other);
		};

	/* BEAR LIMBS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["bear_head"] = []() {
		return LimbForm(
			"Bear Head", "bear_head",
			8, 5, 3, 6,
			DominanceNode::Green, "data/maps/forest/bear_head.png",
			{ Point(101, 115) },
			BodyPartType::Head);
		};

	limbForms["bear_body"] = []() {
		return LimbForm(
			"Bear Body", "bear_body",
			10, 7, 3, 3,
			DominanceNode::Green, "data/maps/forest/bear_body.png",
			{ Point(100, 7), Point(68, 35), Point(134, 27), Point(64, 164), Point(123, 159) },
			BodyPartType::Torso);
		};

	limbForms["bear_arm_left"] = []() {
		return LimbForm(
			"Bear Left Arm", "bear_arm_left",
			9, 9, 3, 2,
			DominanceNode::Green, "data/maps/forest/bear_arm_left.png",
			{ Point(59, 85) },
			BodyPartType::Arm);
		};

	limbForms["bear_arm_right"] = []() {
		return LimbForm(
			"Bear Right Arm", "bear_arm_right",
			9, 9, 3, 2,
			DominanceNode::Green, "data/maps/forest/bear_arm_right.png",
			{ Point(134, 59) },
			BodyPartType::Arm);
		};

	limbForms["bear_leg_left"] = []() {
		return LimbForm(
			"Bear Left Leg", "bear_leg_left",
			9, 9, 4, 1,
			DominanceNode::Green, "data/maps/forest/bear_leg_left.png",
			{ Point(67, 104) },
			BodyPartType::Leg);
		};

	limbForms["bear_leg_right"] = []() {
		return LimbForm(
			"Bear Right Leg", "bear_leg_right",
			9, 9, 4, 1,
			DominanceNode::Green, "data/maps/forest/bear_leg_right.png",
			{ Point(143, 105) },
			BodyPartType::Leg);
		};

	/* SPIDER LIMBS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["spider_body"] = []() {
		return LimbForm(
			"Spider Body", "spider_body",
			7, 8, 6, 4,
			DominanceNode::Green, "data/maps/forest/spider_body.png",
			{ Point(30, 101), Point(41, 139), Point(76, 183), Point(115, 182), Point(153, 155), Point(173, 106) },
			BodyPartType::Torso);
		};

	limbForms["spider_leg_1"] = []() {
		return LimbForm(
			"Spider Leg 1", "spider_leg_1",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_1.png",
			{ Point(161, 49) },
			BodyPartType::Leg);
		};

	limbForms["spider_leg_2"] = []() {
		return LimbForm(
			"Spider Leg 2", "spider_leg_2",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_2.png",
			{ Point(129, 101) },
			BodyPartType::Leg);
		};

	limbForms["spider_leg_3"] = []() {
		return LimbForm(
			"Spider Leg 3", "spider_leg_3",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_3.png",
			{ Point(147, 130) },
			BodyPartType::Leg);
		};

	limbForms["spider_leg_4"] = []() {
		return LimbForm(
			"Spider Leg 4", "spider_leg_4",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_4.png",
			{ Point(50, 63) },
			BodyPartType::Leg);
		};

	limbForms["spider_leg_5"] = []() {
		return LimbForm(
			"Spider Leg 5", "spider_leg_5",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_5.png",
			{ Point(93, 73) },
			BodyPartType::Leg);
		};

	limbForms["spider_leg_6"] = []() {
		return LimbForm(
			"Spider Leg 6", "spider_leg_6",
			4, 6, 10, 4,
			DominanceNode::Green, "data/maps/forest/spider_leg_6.png",
			{ Point(13, 99) },
			BodyPartType::Leg);
		};

	/* FAIRY LIMBS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["fairy_head"] = []() {
		return LimbForm(
			"Fairy Head", "fairy_head",
			5, 4, 7, 10,
			DominanceNode::Green, "data/maps/forest/fairy_head.png",
			{ Point(97, 102) },
			BodyPartType::Head);
		};

	limbForms["fairy_body"] = []() {
		return LimbForm(
			"Fairy Body", "fairy_body",
			6, 4, 7, 7,
			DominanceNode::Green, "data/maps/forest/fairy_body.png",
			{ Point(88, 12), Point(55, 23), Point(66, 39), Point(73, 133),
				Point(100, 126), Point(101, 38), Point(112, 29) },
			BodyPartType::Torso);
		};

	limbForms["fairy_wing_right"] = []() {
		return LimbForm(
			"Fairy Right Wing", "fairy_wing_right",
			2, 2, 10, 7,
			DominanceNode::Green, "data/maps/forest/fairy_wing_right.png",
			{ Point(81, 133) },
			BodyPartType::Other);
		};

	limbForms["fairy_wing_left"] = []() {
		return LimbForm(
			"Fairy Left Wing", "fairy_wing_left",
			2, 2, 10, 7,
			DominanceNode::Green, "data/maps/forest/fairy_wing_left.png",
			{ Point(109, 128) },
			BodyPartType::Other);
		};

	limbForms["fairy_leg_right"] = []() {
		return LimbForm(
			"Fairy Right Leg", "fairy_leg_right",
			4, 3, 8, 6,
			DominanceNode::Green, "data/maps/forest/fairy_leg_right.png",
			{ Point(134, 23) },
			BodyPartType::Leg);
		};

	limbForms["fairy_leg_left"] = []() {
		return LimbForm(
			"Fairy Left Leg", "fairy_leg_left",
			4, 3, 8, 6,
			DominanceNode::Green, "data/maps/forest/fairy_leg_left.png",
			{ Point(25, 31) },
			BodyPartType::Leg);
		};

	limbForms["fairy_arm_left"] = []() {
		return LimbForm(
			"Fairy Left Arm", "fairy_arm_left",
			3, 3, 8, 7,
			DominanceNode::Green, "data/maps/forest/fairy_arm_left.png",
			{ Point(139, 125) },
			BodyPartType::Arm);
		};

	limbForms["fairy_arm_right"] = []() {
		return LimbForm(
			"Fairy Right Arm", "fairy_arm_right",
			3, 3, 8, 7,
			DominanceNode::Green, "data/maps/forest/fairy_arm_right.png",
			{ Point(44, 80) },
			BodyPartType::Arm);
		};

	/* OWL LIMBS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["owl_body"] = []() {
		return LimbForm(
			"Owl Body", "owl_body",
			6, 4, 7, 4,
			DominanceNode::Green, "data/maps/forest/owl_body.png",
			{ Point(90, 37), Point(117, 51), Point(66, 39) },
			BodyPartType::Torso);
		};

	limbForms["owl_head"] = []() {
		return LimbForm(
			"Owl Head", "owl_head",
			5, 6, 6, 8,
			DominanceNode::Green, "data/maps/forest/owl_head.png",
			{ Point(88, 82) },
			BodyPartType::Head);
		};

	limbForms["owl_wing_left"] = []() {
		return LimbForm(
			"Owl Left Wing", "owl_wing_left",
			4, 4, 8, 6,
			DominanceNode::Green, "data/maps/forest/owl_wing_left.png",
			{ Point(128, 104) },
			BodyPartType::Other);
		};

	limbForms["owl_wing_right"] = []() {
		return LimbForm(
			"Owl Right Wing", "owl_wing_right",
			4, 4, 8, 6,
			DominanceNode::Green, "data/maps/forest/owl_wing_right.png",
			{ Point(16, 102) },
			BodyPartType::Other);
		};

	/* OTHER LIMBS */
	/* HP / STRENGTH / SPEED / INTELLIGENCE */

	limbForms["stick_1"] = []() {
		return LimbForm(
			"Stick 1", "stick_1",
			9, 7, 5, 1,
			DominanceNode::Green, "data/maps/forest/stick_1.png",
			{ Point(143, 15), Point(29, 110), Point(100, 182) },
			BodyPartType::Other);
		};

	limbForms["stick_2"] = []() {
		return LimbForm(
			"Stick 2", "stick_2",
			9, 7, 5, 1,
			DominanceNode::Green, "data/maps/forest/stick_2.png",
			{ Point(15, 108), Point(185, 105) },
			BodyPartType::Other);
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