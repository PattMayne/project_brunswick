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

export module Character;

using namespace std;

import TypeStorage;

export enum CharacterType { Player, Hostile, Friendly }; /* NOT a CLASS because we want to use it as int. */
/* Red beats Green (fire consumes life), Green beats Blue (life consumes water), Blue beats Red (water extinguishes fire) */
export enum class DominanceNode { Red, Green, Blue };
export int const dominanceCycleAdvantage = 15;
export enum class LimbState { Free, Owned, Equipped }; /* If it is OWNED or EQUIPPED, then there must be a character id. Every character should exist in the DB. */

/* this POINT should be available everywhere. Where should it be stored? Should there be a Point module? */
export struct Point {
	Point() {}
	Point(int x, int y) : x(x), y(y) {}
	int x;
	int y;
};

class Limb;

/*
* Very minimal parent class.
* Different Screen modules will extend this to be useful in their environments.
* We can't hold a vector of Limb objects here, because the derived Character classes must hold similarly derived Limb objects.
* 
* Maybe Character should hold a vector of Limbs, and its derived object can have a function to turn those Limbs into derived Limbs.
*/
export class Character {
	public:
		Character() {}
		~Character() {}
		Character(SDL_Texture* texture, CharacterType characterType) :
			texture(texture), characterType(characterType) { }

		SDL_Texture* getTexture() { return texture; }
		void setTexture(SDL_Texture* incomingTexture) {
			if (texture) {
				SDL_DestroyTexture(texture);
				texture = incomingTexture; }
		}
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
		SDL_Texture* texture;
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
*/
export class Limb {
	public:
		/* constructor */
		Limb(
			string name,
			int hp,
			int attack,
			int speed,
			int weight,
			DominanceNode dNode,
			bool flipped,
			vector<Point> joints
		) :
			name(name),
			hp(hp),
			attack(attack),
			speed(speed),
			weight(weight),
			dNode(dNode),
			flipped(flipped),
			joints(joints)
		{}
		void setFlipped(bool flip) { flipped = flip; }
		void flip() { flipped = !flipped; }
		bool save() { /* SAVE this limb to the database. */ }

	protected:
		string name;
		int hp;
		int attack;
		int speed;
		int weight;
		DominanceNode dNode;
		bool flipped = false;
		vector<Point> joints;
		SDL_Texture* avatar = NULL;
};