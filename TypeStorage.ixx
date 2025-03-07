module;

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

export module TypeStorage;

using namespace std;

export enum class DominancePosition { Dom, Sub, Neutral };

/* Dominance Node is a Limb's node in the Dominance Cycle (think Rock, Paper, Scissors). */
export enum class DominanceNode { Red, Green, Blue };
export int const dominanceCycleAdvantage = 15;

export DominancePosition attackerDominancePosition(DominanceNode attackerNode, DominanceNode defenderNode) {
	DominancePosition dPosition = DominancePosition::Neutral;
	if (attackerNode == defenderNode) { return dPosition; }

	/*
	* Blue beats Red (water douses fire).
	* Red beats Green (fire burns life).
	* Green beats Blue (life consumes water).
	*/

	if (attackerNode == DominanceNode::Blue) {
		if (defenderNode == DominanceNode::Green) {
			dPosition = DominancePosition::Sub;
		}
		else if (defenderNode == DominanceNode::Red) {
			dPosition = DominancePosition::Dom;
		}
	}
	else if (attackerNode == DominanceNode::Green) {
		if (defenderNode == DominanceNode::Blue) {
			dPosition = DominancePosition::Dom;
		}
		else if (defenderNode == DominanceNode::Red) {
			dPosition = DominancePosition::Sub;
		}
	}
	else if (attackerNode == DominanceNode::Red) {
		if (defenderNode == DominanceNode::Blue) {
			dPosition = DominancePosition::Sub;
		}
		else if (defenderNode == DominanceNode::Green) {
			dPosition = DominancePosition::Dom;
		}
	}

	return dPosition;
}

export enum class ScreenType {
	NoScreen, Menu, Map, Battle, CharacterCreation
};


export enum class MapType {
	World, Building, Dungeon
};

export enum class MapLevel {
	Forest, All
};

export enum class FontContext {
	Title, Body, Button, Dialog
};

export enum SuitType {
	NoSuit, /* Everything EXCEPT an actual SUIT must have THIS SuitType (or else their limbs will not be drawn). */

	/* Forest suits. */
	Deer, Bear, Spider, Fairy, Owl,

	/* Total */

	TotalSuit
};

/* values to flag for resolutions. Mostly for development purposes. Final release should always be fullscreen. */
export enum class WindowResType {
	Mobile, Tablet, Desktop, Fullscreen
};


export enum BattleStatus {
	PlayerTurn, NpcTurn, PlayerVictory, PlayerDefeat, RebuildRequired
};


/* width and height data */
export struct Resolution {
	int w;
	int h;

	Resolution(int x, int y) {
		w = x;
		h = y;
	}
};


/* Add dominance cycle? Other attributes (strength etc)? */
export struct LimbButtonData {
	LimbButtonData(string texturePath, string name, int id, int hp,
		int strength, int intelligence, int speed, DominanceNode domNode)
		:
		texturePath(texturePath), name(name), id(id), hp(hp), intelligence(intelligence),
		speed(speed), strength(strength), domNode(domNode) {
	}

	string texturePath;
	string name;
	int id;

	int hp;
	int intelligence;
	int speed;
	int strength;
	DominanceNode domNode;
};



export enum class ConfirmationButtonType {
	YesNo, OkCancel
};



/* to avoid recursive self-referential class design, I'll make a Struct to hold information about parent Screens.
* id refers to the id of the Map or Battle object in the database.
* Screens will have the power to set ScreenStruct in GameState, not relying on main.cpp to make the change.
	*/
export struct ScreenStruct {
	ScreenType screenType;
	int id;
	ScreenStruct(ScreenType incomingType = ScreenType::NoScreen, int incomingId = -1) {
		screenType = incomingType;
		id = incomingId;
	}
};


/* If this struct is returned to the main function, the program will shut down. */
export ScreenStruct closingScreenStruct() {
	ScreenStruct closingScreenStruct(ScreenType::NoScreen, -1);
	return closingScreenStruct;
}

export struct AvatarAndDrawRect {
	AvatarAndDrawRect(SDL_Texture* texture, SDL_Rect rect) : avatar(texture), drawRect(rect) {}
	SDL_Texture* avatar;
	SDL_Rect drawRect;
};

export enum class BodyPartType {
	Head, Torso, Arm, Leg, Wing, Other
};


/* PANEL BUTTON ENUMS
* Each panel has a list of buttons.
* Each button must store one of these enums, and will choose text independently of the screen.
* The screen checks which enum was triggered (when button clicked) and chooses function/action independently of the button.
* One big enum class will store all Button options.
*/

export enum class ButtonOption {
	NoOption,
	Agree, Refuse, /* For confirmation panel (OK/Cancel or YES/NO) */
	Back, /* for any submenu */
	MainMenu, /* quit back to main menu from any screen */
	/* Main Menu panel buttons */
	NewGame, LoadGame, Settings, About, Exit, Continue, Return, BackToGame,
	/* Settings buttons */
	Mobile, Tablet, Desktop, Fullscreen,
	// TO COME: more options for Map, Battle
	MapOptions,

	/* CHARACTER CREATION SCREEN BUTTONS */
	/* Review Mode panel buttons. */
	ShowLimbs, ClearSuit, SaveSuit,
	/* Inventory (ChooseLimb mode) Panel Buttons. */
	LoadLimb, /* Will be re-used dynamically by ALL limb buttons. */
	NextPage, PreviousPage,
	/* Loaded Limb (LimbLoaded mode) panel buttons. */
	NextCharJoint, NextLimbJoint, RotateClockwise, RotateCounterClockwise, UnloadLimb, Equip,
	/* Battle Screen buttons. */
	BattleMove

};

/* When a button is clicked it must send back one of these Structs.
Sometimes the type of button clicked is enough.
Other times we need a 2nd piece of information,
such as the ID of the map to load or NPC to battle. */
export struct ButtonClickStruct {
	ButtonOption buttonOption;
	int itemID;

	/* constructors. */
	ButtonClickStruct(ButtonOption incomingOption, int incomingID) {
		buttonOption = incomingOption;
		itemID = incomingID;
	}
	/* blank. */
	ButtonClickStruct() {
		buttonOption = ButtonOption::NoOption;
		itemID = -1;
	}
};


export struct Point {
	Point() { x = 0; y = 0; }
	Point(int x, int y) : x(x), y(y) {}
	int x;
	int y;

	bool equals(Point point) {
		return point.x == x && point.y == y;
	}
};


export enum LandmarkType { Entrance, Building, Shrine, Exit };

export bool isValidLandmarkType(int value) {
	return value >= Entrance && value <= Exit; }

export bool isValidSuitType(int value) {
	return value >= 0 && value <= TotalSuit;
}


/*
* For battles.
*/


export enum AttributeType {
	HP, Intelligence, Speed, Strength
};

export enum AttackType {
	NoAttack,
	Attack, /* Generic "attack" */
	Punch, DoublePunch, Kick, BodySlam, /* Just damage HP. */
	Swoop, /* steal Speed and damage HP. */
	BrainDrain, /* Steal intelligence and damage HP. */
	Steal, Throw, Heal
};

export struct AttackStruct {
	AttackStruct(string name, string slug, int intensity, int precision, DominanceNode dNode,
		AttackType attackType, vector<AttributeType> attributeTypes)
		:
		slug(slug), name(name), precision(precision), intensity(intensity), dNode(dNode),
		attackType(attackType), attributeTypes(attributeTypes) { }

	/* Empty constructor for null Attack. */
	AttackStruct() {
		name = "none";
		slug = "none";
		precision = 0;
		intensity = 0;
		dNode = DominanceNode::Green;
		attackType = AttackType::NoAttack;
		fired = false;
	}

	/* Copy constructor. */
	AttackStruct(const AttackStruct& other) {
		name = other.name;
		slug = other.slug;
		intensity = other.intensity;
		precision = other.precision;
		dNode = other.dNode;
		attackType = other.attackType;
		fired = other.fired;
		targetLimbId = other.targetLimbId;
		sourceLimbId = other.sourceLimbId;
	}

	string name;
	string slug;
	int intensity; /* of a possible 100 */
	int precision; /* of a possible 100 */
	DominanceNode dNode;
	AttackType attackType;
	vector<AttributeType> attributeTypes;
	bool fired;

	void fire() { fired = true; }

	int targetLimbId;
	int sourceLimbId;
};


export unordered_map<AttackType, string> getAttackTypeTextDictionary() {
	unordered_map<AttackType, string> typeDictionary;
	typeDictionary[AttackType::NoAttack] = "No Attack";
	typeDictionary[AttackType::Attack] = "Attack";
	typeDictionary[AttackType::Punch] = "Punch";
	typeDictionary[AttackType::DoublePunch] = "Double Punch";
	typeDictionary[AttackType::Kick] = "Kick";
	typeDictionary[AttackType::BodySlam] = "Body Slam";
	typeDictionary[AttackType::Swoop] = "Swoop";
	typeDictionary[AttackType::BrainDrain] = "Brain Drain";
	typeDictionary[AttackType::Steal] = "Steal";
	typeDictionary[AttackType::Throw] = "Throw";
	typeDictionary[AttackType::Heal] = "Heal";

	return typeDictionary;
}


export string attackTypeText(AttackType attackType) {
	unordered_map<AttackType, string> typeDictionary = getAttackTypeTextDictionary();
	return typeDictionary[attackType];
};

export unordered_map<DominanceNode, string> getDominanceNodeTextDictionary() {
	unordered_map<DominanceNode, string> typeDictionary;
	typeDictionary[DominanceNode::Green] = "Green";
	typeDictionary[DominanceNode::Red] = "Red";
	typeDictionary[DominanceNode::Blue] = "Blue";

	return typeDictionary;
}

export string dNodeText(DominanceNode dNode) {
	return getDominanceNodeTextDictionary()[dNode];
}