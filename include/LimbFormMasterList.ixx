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

#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <functional>

export module LimbFormMasterList;

using namespace std;


import TypeStorage;
import CharacterClasses;

export struct LimbForm {
	string name;
	string slug;
	int strength;
	int speed;
	int intelligence;
	DominanceNode dNode;
	vector<Point> joints;
	string texturePath;

	/* CONSTRUCTOR */
	LimbForm(string name, string slug, int strength, int speed, int intelligence, DominanceNode dNode, string texturePath, vector<Point> joints) :
		name(name), slug(slug), strength(strength), speed(speed), intelligence(intelligence), dNode(dNode), texturePath(texturePath), joints(joints) {
	}
};

/* 
* An unordered_map of functions to create LimbForm structs.
* The structs are only created when their functions are called.
* The string key is the slug.
*/
export unordered_map<string, function<LimbForm()>> getLimbFormMasterList() {

	unordered_map<string, function<LimbForm()>> limbForms;

	/* FOREST LIMB FORMS */

	limbForms["deer_leg_4"] = []() {
		return LimbForm(
			"Deer Leg 4", "deer_leg_4",
			6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_4.png",
			{ Point(144, 81) } );
		};

	limbForms["deer_leg_3"] = []() {
		return LimbForm(
			"Deer Leg 3", "deer_leg_3",
			6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_3.png",
			{ Point(131, 71) });
		};
	
	limbForms["deer_leg_2"] = []() {
		return LimbForm(
			"Deer Leg 2", "deer_leg_2",
			6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_2.png",
			{ Point(78, 46) });
		};

	limbForms["deer_leg_1"] = []() {
		return LimbForm(
			"Deer Leg 1", "deer_leg_1",
			6, 10, 3,
			DominanceNode::Green, "data/maps/forest/deer_leg_1.png",
			{ Point(52, 43) });
		};

	return limbForms;
}


export LimbForm getLimbForm(string slug) {
	unordered_map<string, function<LimbForm()>> limbFormMasterList = getLimbFormMasterList();

	/*
	* Do a check
	*/

	return limbFormMasterList[slug]();
}

/* ALSO add a function which takes a LIST of slugs to getLimbForms PLURAL without creating and destroying the same unordered_list repeatedly */