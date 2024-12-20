/*
* 
*   ____ _                          _
*  / ___| |__   __ _ _ __ __ _  ___| |_ ___ _ __ ___
* | |   | '_ \ / _` | '__/ _` |/ __| __/ _ \ '__/ __|
* | |___| | | | (_| | | | (_| | (__| ||  __/ |  \__ \
*  \____|_| |_|\__,_|_|  \__,_|\___|\__\___|_|  |___/
*   __ _ _ __   __| |
*  / _` | '_ \ / _` |
* | (_| | | | | (_| |
*  \__,_|_| |_|\__,_|_
* | |   (_)_ __ ___ | |__  ___
* | |   | | '_ ` _ \| '_ \/ __|
* | |___| | | | | | | |_) \__ \
* |_____|_|_| |_| |_|_.__/|___/
* 
* CHARACTER and LIMB (abstract) objects to be extended in the modules that use them.
* 
* 
* TO DO:
* -- Create a master list, or some kind of reference, to find out which TYPE (label? slug?) of Limb we're using.
* ---- It should somehow be part of a Map.
* ---- Does a Map own the Limb? Or does the Limb hold a reference to its parent Map?
* ------ Certainly each Limb saved in the DB should reference its parent Map... or not. The Label or Slug
* ---- Should we use enum?
* ---- Should I get rid of the JSON and hard-code it all in a factory in this module?
* 
*/

module;

#include "SDL.h"
#include "SDL_image.h"
#include <string>
#include <vector>

export module CharacterClasses;

using namespace std;

import TypeStorage;
import UI;

export enum CharacterType { Player, Hostile, Friendly }; /* NOT a CLASS because we want to use it as int. */
/* Red beats Green (fire consumes life), Green beats Blue (life consumes water), Blue beats Red (water extinguishes fire) */
export enum class LimbState { Free, Owned, Equipped }; /* If it is OWNED or EQUIPPED, then there must be a character id. Every character should exist in the DB.*/

class Limb;
struct LimbPlacement;



/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~  _____ ___  ____  __  __                  ~
* ~ |  ___/ _ \|  _ \|  \/  |                 ~
* ~ | |_ | | | | |_) | |\/| |                 ~
* ~ |  _|| |_| |  _ <| |  | |                 ~
* ~ |_|   \___/|_| \_\_|  |_|				  ~
* ~  ____ _____ _____ _   _  ____ _____ ____  ~
* ~ / ___|_   _|  _ \| | | |/ ___|_   _/ ___| ~
* ~ \___ \ | | | |_) | | | | |     | | \___ \ ~
* ~  ___) || | |  _ <| |_| | |___  | |  ___) |~
* ~ |____/ |_| |_| \_\\___/ \____| |_| |____/ ~
* ~                                           ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* These are the basic types holding core data of what will be objects.
* Vanilla forms of objects, prior to being saved to (or retrieved from) the database.
*
* Every value for a form struct must be a const.
*/
export struct LimbForm {
	const string name;
	const string slug;
	const  int hp;
	const int strength;
	const int speed;
	const int intelligence;
	const DominanceNode dNode;
	const vector<Point> joints;
	const string texturePath;

	/* CONSTRUCTOR */
	LimbForm(
		string name, string slug,
		int hp, int strength, int speed, int intelligence,
		DominanceNode dNode, string texturePath, vector<Point> joints)
		:
		name(name), slug(slug),
		hp(hp), strength(strength), speed(speed), intelligence(intelligence),
		dNode(dNode), texturePath(texturePath), joints(joints) {
	}
};

/* A suit is abstract. It is NOT a character. It holds information to build an abstract base character. */
export struct SuitForm {
	const string name;
	const string slug;
	const bool unscrambled;
	const vector<LimbPlacement> limbPlacements;
};


export struct LandmarkForm {
	const int blocksWidth;
	const int blocksHeight;
	const vector<Point> blockPositions;
	const SDL_Texture* texture;
	const LandmarkType landmarkType;
};


export struct MapForm {
	MapLevel mapLevel;
	string name;
	string slug;
	vector<LimbForm> nativeLimbs; /* We will need some limbs to be "free" and NOT part of a Suit. So the suits will simply refer to the slugs of the limbs, not contain the limbs. */
	vector<SuitForm> suits;
	int blocksWidth;
	int blocksHeight;
	SDL_Texture* wallTexture;
	SDL_Texture* floorTexture;
};


/**
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* ~   ____ _        _    ____ ____  _____ ____  ~
* ~  / ___| |      / \  / ___/ ___|| ____/ ___| ~
* ~ | |   | |     / _ \ \___ \___ \|  _| \___ \ ~
* ~ | |___| |___ / ___ \ ___) |__) | |___ ___) |~
* ~  \____|_____/_/   \_\____/____/|_____|____/ ~
* ~                                             ~
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


/*
* Very minimal parent class.
* Different Screen modules will extend this to be useful in their environments.
* We can't hold a vector of Limb objects here, because the derived Character classes must hold similarly derived Limb objects.
* 
* Maybe Character should hold a vector of Limbs, and its derived object can have a function to turn those Limbs into derived Limbs.
* 
* REMOVE TEXTURE.
* Only screen-specific derivatives should have textures, created from limbs.
* In fact, only the Map has a texture, since it's displayed as a collection of limbs everywhere else.
* 
* I don't need MAP here, because MAP will only exist on the MapScreen screen, so it doesn't need derived classes.
* The factories will include a Map factory, just to retrieve the basic Map data and send it to the MapScreen screen.
*/
export class Character {
	public:
		Character() {}
		~Character() {}
		Character(CharacterType characterType) :
			characterType(characterType) { }

		int getType() { return characterType; }
		void setId(int id) { this->id = id; }
		virtual void setLimbs(vector<Limb> limbs) {
			/*
			* Derived classes MUST override this.
			* They will take the base Limbs vector provided, and cast them into their own DERIVED Limbs.
			* So the MapCharacter will take a vector of normal Limbs (or derived MapLimbs),
			* and explicitly cast them as MapLimbs into vector<MapLimb> limbs,
			* so the module can programmatically use MapLimbs' specific functions and attributes.
			*/
		}

	protected:
		CharacterType characterType;
		int id;
		/*
		* NO LIMBS VECTOR HERE.
		* Instead each Derived class will
		*/
		// vector<Limb> limbs; /* LIMBS vector should be the BASE. Derived Character classes have functions to convert the base Limbs into derived Limbs. */
};

/*
* Minimalistic class from which useful Limb classes will derive for their objects.
* Every Limb object must be stored in the database. As soon as it exists it must have an ID.
* 
* Maybe "attack" should be "strength".
* "Intelligence" can affect how precisely you hit a limb, vs how much the damage is spread around.
* Certain Low-intelligence limbs can create a special power which spreads damage around intentionally.
* Intelligence raises your chances of hitting at all. Or hitting the correct target.
* Intelligence also raises your chance of being missed (make whole limb twirl around on a joint-pivot or something?)
* Position is where the Limb is located in any Character or Suit that's holding it.
* 
* 
* 
* Limb should have a LimbForm, and modifiers.
* Modifiers cannot be larger than the original.
* 
* Let a Limb take a Form as its constructor... and a 2nd consrtructor which also takes an ID? Or only an ID (and gets the Form based on slug?)?
* 
* Limb object should take a Form as its basic stats, and have modifiers.
* Only the modifiers are saved to the DB.
* The Form is retrieved fresh from the definition every time the Limb is constructed/instantiated.
* 
*/
export class Limb {
	public:
		/* constructor */
		Limb(LimbForm form) : form(form) {
			name = form.name; /* Name can POSSIBLE be changed (no plans for this yet) */
			hpMod = 0;
			strengthMod = 0;
			speedMod = 0;
			intelligenceMod = 0;
			position = Point(0, 0);
			lastPosition = Point(0, 0);

			/* get Limb texture */

			UI& ui = UI::getInstance();
			SDL_Surface* limbSurface = IMG_Load(form.texturePath.c_str());
			texture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), limbSurface);
			SDL_FreeSurface(limbSurface);
			/* TO DO: ERROR HANDLING FOR TEXTURE. */
		}
		/*  */
		void setFlipped(bool flip) { flipped = flip; }
		void flip() { flipped = !flipped; }
		bool save() { /* SAVE this limb to the database. (save the MODIFICATIONS) */ }
		Point getPosition() { return position; }
		void setPosition(Point newPosition) { position = newPosition; }

		/* GET the FORM (default) values PLUS the modifiers (which can be negative) */
		int getHP() { return form.hp + hpMod; }
		int getStrength() { return form.strength + strengthMod; }
		int getSpeed() { return form.speed + speedMod; }
		int getIntelligence() { return form.intelligence + intelligenceMod; }

		/* SET the modifiers. */

		int modifyStrength(int mod) {
			modifyAttribute(strengthMod, form.strength, mod);
			return getStrength(); }

		int modifySpeed(int mod) {
			modifyAttribute(speedMod, form.speed, mod);
			return getSpeed(); }

		int modifyIntelligence(int mod) {
			modifyAttribute(intelligenceMod, form.intelligence, mod);
			return getIntelligence(); }

		/* HP is different. It can go down to 0, but can still only be boosted by 1/2. */
		int modifyHP(int mod) {
			hpMod += mod;
			/* Check if updated attributeMod goes beyond limit */
			if (hpMod > form.hp / 2) {
				hpMod = form.hp / 2; }
			return getHP();
		}

		void modifyAttribute(int& attributeMod, int formAttribute, int modMod) {
			attributeMod += modMod;
			/* Check if updated attributeMod goes beyond limit */
			if (attributeMod > formAttribute / 2) {
				attributeMod = formAttribute / 2; }
			else if ((attributeMod * -1) > formAttribute / 2) { /* check limit for detriment too */
				attributeMod = formAttribute / (-2); }
		}

	protected:
		LimbForm form;
		string name;
		int hpMod;
		int strengthMod;
		int speedMod;
		int intelligenceMod;
		bool flipped = false;
		SDL_Texture* texture = NULL;
		Point position;
		Point lastPosition;
};

export struct LimbPlacement {
	string slug;
	Point position;
};