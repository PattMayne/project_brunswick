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

export module CharacterClasses;

import "SDL.h";
import "SDL_image.h";
import <string>;
import <vector>;

import TypeStorage;
import UI;

using namespace std;

export enum CharacterType { Player, Hostile, Friendly }; /* NOT a CLASS because we want to use it as int. */
/* Red beats Green (fire consumes life), Green beats Blue (life consumes water), Blue beats Red (water extinguishes fire) */
export enum class LimbState { Free, Owned, Equipped }; /* If it is OWNED or EQUIPPED, then there must be a character id. Every character should exist in the DB.*/

class Limb;

/* Where the limb image will be drawn onto the character surface. */
export struct SuitLimbPlacement {
	string slug;
	Point position;
};




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
	string name;
	string slug;
	int hp;
	int strength;
	int speed;
	int intelligence;
	DominanceNode domNode;
	vector<Point> jointPoints; /* Limb CLASS will have full Joint objects which hold references to other limbs. */
	string texturePath;

	/* CONSTRUCTOR */
	LimbForm(
		string name, string slug,
		int hp, int strength, int speed, int intelligence,
		DominanceNode domNode, string texturePath, vector<Point> jointPoints)
		:
		name(name), slug(slug),
		hp(hp), strength(strength), speed(speed), intelligence(intelligence),
		domNode(domNode), texturePath(texturePath), jointPoints(jointPoints) {
	}
};

/* A suit is abstract. It is NOT a character. It holds information to build an abstract base character. */
export struct SuitForm {
	const string name;
	const string slug;
	const vector<SuitLimbPlacement> limbPlacements;
	bool unscrambled;

	SuitForm(string name, string slug, vector<SuitLimbPlacement> limbPlacements, bool unscrambled = false) :
		name(name), slug(slug), limbPlacements(limbPlacements), unscrambled(unscrambled) { }
};


export struct LandmarkForm {
	const string name;
	const int blocksWidth;
	const int blocksHeight;
	const vector<Point> blockPositions;
	const SDL_Texture* texture;
	const LandmarkType landmarkType;

	LandmarkForm(
		string name, int width, int height, vector<Point> blockPositions,
		SDL_Texture* texture, LandmarkType landmarkType ) :
		name(name), blocksWidth(width), blocksHeight(height), blockPositions(blockPositions),
		texture(texture), landmarkType(landmarkType) { }
};


export struct MapForm {
	MapLevel mapLevel;
	string name;
	string slug;
	vector<LimbForm> nativeLimbs; /* We will need some limbs to be "free" and NOT part of a Suit. So the suits will simply refer to the slugs of the limbs, not contain the limbs. */
	vector<SuitForm> suits;
	int blocksWidth;
	int blocksHeight;
	vector<SDL_Texture*> floorTextures;
	vector<SDL_Texture*> wallTextures;
	vector<SDL_Texture*> pathTextures;
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

/* TO DO: add id (from database) */
export class Joint {
public:
	/* Vanilla empty Limb. Possibly never use. */
	Joint() {
		point = { 0, 0 };
		isAnchor = false;
		connectedLimbId = -1;
		anchorJointIndex = -1;
		rotationAngle = 0;
	}

	/* When we create a new Joint for a new Limb. */
	Joint(Point point) : point(point), isAnchor(false), connectedLimbId(-1), anchorJointIndex(-1), rotationAngle(0) {}

	/* When we load a joint from the database. */
	Joint(Point point, bool isAnchor, int connectedLimbId, int anchorJointIndex, int rotationAngle) :
		point(point), isAnchor(isAnchor), connectedLimbId(connectedLimbId), anchorJointIndex(anchorJointIndex), rotationAngle(rotationAngle) {}

	void setAnchor(bool makeAnchor = true) { isAnchor = makeAnchor; }
	void connectLimb(int limbId, int jointIndex) {
		connectedLimbId = limbId;
		anchorJointIndex = jointIndex;
	}
	//void setRotationAngle(int newAngle) { rotationAngle = newAngle; }
	int rotateAnchoredLimb(int angleIncrement) {
		rotationAngle += angleIncrement;
		return rotationAngle;
	}
	void resetRotationAngle() { rotationAngle = 0; }
	void detachLimb() {
		rotationAngle = 0;
		connectedLimbId = -1;
		anchorJointIndex = -1;
	}

	bool isFree() {
		return !isAnchor && connectedLimbId < 0;
	}

private:
	Point point;
	bool isAnchor;

	/* Data about the CONNECTED limb. */
	int connectedLimbId;
	int anchorJointIndex;
	int rotationAngle;
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
* Limb textures will be destroyed by the screens handling them.
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
			position = Point(50, 95);
			lastPosition = Point(0, 0);

			/* get Limb texture */

			UI& ui = UI::getInstance();
			SDL_Surface* limbSurface = IMG_Load(form.texturePath.c_str());
			texture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), limbSurface);
			SDL_FreeSurface(limbSurface);
			/* TO DO: ERROR HANDLING FOR TEXTURE. */

			/* Build actual Joints from the LimbForm. */

			for (Point jointPoint : form.jointPoints) {
				/* new Limb constructor (must later accomadate existing joints from existing Limbs. */
				joints.emplace_back(jointPoint);
			}
		}

		/*  */
		void setFlipped(bool flip) { flipped = flip; }
		void flip() { flipped = !flipped; }
		bool save() { /* SAVE this limb to the database. (save the MODIFICATIONS) */ return true; }
		Point getPosition() { return position; }
		Point getLastPosition() { return lastPosition; }
		void setPosition(Point newPosition) { position = newPosition; }
		void setLastPosition(Point newPosition) { lastPosition = newPosition; }
		string getName() { return name; }
		string getTexturePath() { return form.texturePath; }

		/* GET the FORM (default) values PLUS the modifiers (which can be negative) */
		int getHP() { return form.hp + hpMod; }
		int getStrength() { return form.strength + strengthMod; }
		int getSpeed() { return form.speed + speedMod; }
		int getIntelligence() { return form.intelligence + intelligenceMod; }

		SDL_Texture* getTexture() { return texture; }

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

		/* This can be MAP position or DRAW position. */
		void move(Point newPosition) {
			lastPosition = position;
			position = newPosition;
		}

		vector<Joint>& getJoints() { return joints; }

		bool isEquipped() {
			for (Joint& joint : joints) {
				if (!joint.isFree()) { return true; } }
			return false;
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
		vector<Joint> joints;
};



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
* 
* Maybe the limbs vector(s) must be smart pointers.
*/
export class Character {
public:
	Character() {
		anchorLimbId = -1;
	}
	~Character() {}
	Character(CharacterType characterType) :
		characterType(characterType), anchorLimbId(-1) {}

	int getType() { return characterType; }
	void setId(int id) { this->id = id; }
	vector<Limb>& getLimbs() { return limbs; }
	bool equipLimb(int limbId);


protected:
	CharacterType characterType;
	int id;
	int anchorLimbId; /* Currently using INDEX for dev purposes... will replace with actual DB ID. */
	vector<Limb> limbs;
};


bool Character::equipLimb(int limbId) {
	Limb& limbToEquip = limbs[limbId];// THIS will get the limb WITH the ID later, not by index!
	if (limbToEquip.isEquipped()) {
		cout << "Limb is already equipped!\n";
		return false;
	}

	if (anchorLimbId < 0) {
		anchorLimbId = limbId;
		cout << limbToEquip.getName() << " is now the ANCHOR limb\n";
		return true;
	}
	
	cout << "Another limb is already anchored. Trying to attach...\n";
	/* Recursively search through limb joints (and their limbs and THEIR joints) to find a FREE joint.
	* This FREE joint will become the ANCHOR joint.
	*
	* THEN we need to attach this ANCHOR (free) joint to a free joint on the character's anchor LIMB.
	*
	* Fow now we're doing ONE level of "recursion" (ie no recursion) and getting the base functionality.
	* Will abstract these as functions to make recursion possible.
	*/
	// start with just searching on THIS level.
	// YOU FOOL... we must search the ANCHOR LIMB for free joints first!

	Limb& anchorLimb = limbs[anchorLimbId];

	cout << "Searching anchored limb (" << anchorLimb.getName() << ") for free joints\n";

	for (int k = 0; k < anchorLimb.getJoints().size(); ++k) {
		Joint& anchoredLimbJoint = anchorLimb.getJoints()[k];

		if (anchoredLimbJoint.isFree()) {

			cout << "Found a free joint in the anchored limb. Now searching the LIMB TO EQUIP for a free joint.\n";

			for (int i = 0; i < limbToEquip.getJoints().size(); ++i) {
				Joint& jointToEquip = limbToEquip.getJoints()[i];

				if (jointToEquip.isFree()) {
					/* Now find a joint on which to anchor it. */
					cout << "FOund a free joint in the limb to equip.\n";

					/* First set the limbToEquip joint as the anchor for that limb. */
					jointToEquip.setAnchor(true);

					/* Then anchor the actual limb onto the free joint. */
					anchoredLimbJoint.connectLimb(limbId, i);

					cout << "Seems like we connected the limb?\n";

					return true;
				}
			}

			

			
		}
	}




	

	return false;
}