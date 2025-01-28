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
import <tuple>;
import <cmath>;

import TypeStorage;
import UI;

using namespace std;

export enum CharacterType { Player, Hostile, Friendly }; /* NOT a CLASS because we want to use it as int. */
/* Red beats Green (fire consumes life), Green beats Blue (life consumes water), Blue beats Red (water extinguishes fire) */
export enum class LimbState { Free, Owned, Equipped }; /* If it is OWNED or EQUIPPED, then there must be a character id. Every character should exist in the DB.*/

class Limb;
int normalizeAngle(int angle);

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
		pointForm = { 0, 0 };
		isAnchor = false;
		connectedLimbId = -1;
		anchorJointIndex = -1;
	}

	/* When we create a new Joint for a new Limb. */
	Joint(Point point) : pointForm(point), isAnchor(false), connectedLimbId(-1), anchorJointIndex(-1) {
		resetModifiedPoint();
	}

	/* When we load a joint from the database. */
	Joint(Point point, bool isAnchor, int connectedLimbId, int anchorJointIndex, int rotationAngle) :
		pointForm(point), isAnchor(isAnchor), connectedLimbId(connectedLimbId), anchorJointIndex(anchorJointIndex)
	{
		resetModifiedPoint();
	}

	void setAnchor(bool makeAnchor = true) { isAnchor = makeAnchor; }
	void connectLimb(int limbId, int jointIndex) {
		connectedLimbId = limbId;
		anchorJointIndex = jointIndex;
	}

	void detachLimb() {
		connectedLimbId = -1;
		anchorJointIndex = -1;
	}

	bool isFree() {
		return !isAnchor && connectedLimbId < 0;
	}

	bool getIsAnchor() { return isAnchor; }
	int getConnectedLimbId() { return connectedLimbId; }
	int getChildLimbAnchorJointIndex() { return anchorJointIndex; }
	Point getFormPoint() { return pointForm; }
	Point getPoint() { return modifiedPoint; }
	void resetModifiedPoint() {
		modifiedPoint = { pointForm.x, pointForm.y };
	}
	void setModifiedPoint(Point newPoint) { modifiedPoint = newPoint; }

private:
	Point pointForm; /* Not saved to DB. Get fresh from limb form. */
	Point modifiedPoint; /* NOT point MODIFIER. It's the fully modified point. Saved to DB. */
	bool isAnchor;

	/* Data about the CONNECTED limb. */
	int connectedLimbId;
	int anchorJointIndex;
};


Point getRotatedPoint(Point anchorPoint, Point pointToRotate, int rotationAngle) {
	/* Convert to radians. */
	double angleRad = rotationAngle * (M_PI / 180);

	/* Translate point to rotate to origin. */
	double translatedX = pointToRotate.x - anchorPoint.x;
	double translatedY = pointToRotate.y - anchorPoint.y;

	/* Apply rotation. */
	double rotatedX = translatedX * cos(angleRad) - translatedY * sin(angleRad);
	double rotatedY = translatedX * sin(angleRad) + translatedY * cos(angleRad);

	Point newPoint = {
		static_cast<int>(round(rotatedX + anchorPoint.x)),
		static_cast<int>(round(rotatedY + anchorPoint.y))
	};
	return newPoint;
}


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
			isAnchor = false;

			/* get Limb texture */

			UI& ui = UI::getInstance();
			SDL_Surface* limbSurface = IMG_Load(form.texturePath.c_str());
			texture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), limbSurface);
			SDL_FreeSurface(limbSurface);
			/* TO DO: ERROR HANDLING FOR TEXTURE. */

			/* Build actual Joints from the LimbForm. */

			for (Point& jointPoint : form.jointPoints) {
				/* new Limb constructor (must later accomadate existing joints from existing Limbs. */
				joints.emplace_back(jointPoint);
			}

			drawRect = { 0, 0, 0, 0 };
			rotationAngle = 0;
			SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
		}

		/*  */
		void setFlipped(bool flip) { flipped = flip; }
		void flip() { flipped = !flipped; }
		bool save() { /* SAVE this limb to the database. (save the MODIFICATIONS) */ return true; }
		Point getPosition() { return position; }
		Point getLastPosition() { return lastPosition; }
		void setPosition(Point newPosition) { position = newPosition; }
		void setLastPosition(Point newPosition) { lastPosition = newPosition; }

		LimbForm getForm() { return form; }
		string getName() { return name; }
		string getTexturePath() { return form.texturePath; }

		int getId() { return id; }
		void setId(int id) { this->id = id; }

		/* GET the FORM (default) values PLUS the modifiers (which can be negative) */
		int getHP() { return form.hp + hpMod; }
		int getStrength() { return form.strength + strengthMod; }
		int getSpeed() { return form.speed + speedMod; }
		int getIntelligence() { return form.intelligence + intelligenceMod; }
		int getRotationAngle() { return rotationAngle; }

		SDL_Rect& getDrawRect() { return drawRect; }
		SDL_Texture* getTexture() { return texture; }
		bool getIsAnchor() { return isAnchor; }

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
			position = newPosition; }

		vector<Joint>& getJoints() { return joints; }

		bool isEquipped() {
			if (getIsAnchor()) {
				return true;
			}
			for (Joint& joint : joints) {
				if (!joint.isFree()) { return true; }
			}
			return false;
		}

		void setAnchor(bool isAnchor = true) {
			this->isAnchor = isAnchor; }

		void unEquip() {
			for (Joint& joint : joints) {
				joint.setAnchor(false);
				joint.detachLimb();
				joint.resetModifiedPoint();
			}
			isAnchor = false;
			rotationAngle = 0;
		}

		Joint& getAnchorJoint() {
			for (Joint& joint : joints) {
				if (joint.getIsAnchor()) {
					return joint;
				}
			}
			/* THIS IS NOT SAFE. */
			return joints[0];
		}

		int getAnchorJointId() {
			for (int i = 0; i < joints.size(); ++i) {
				Joint& joint = joints[i];
				if (joint.getIsAnchor()) {
					return i;
				}
			}
			return -1;
		}

		void setDrawRect(SDL_Rect drawRect) {
			this->drawRect = drawRect; }

		//void setRotationAngle(int rotationAngle) { this->rotationAngle = normalizeAngle(rotationAngle); }
		void resetRotationAngle() {
			rotationAngle = 0;
			for (Joint& joint : joints) {
				joint.resetModifiedPoint();
			}
		}
		int rotate(int angleIncrement) {
			rotationAngle = normalizeAngle(rotationAngle + angleIncrement);

			/* Now update all the joint points (except the anchor point). */
			for (Joint& joint : joints) {
				if (!joint.getIsAnchor()) {
					/* If this LIMB has no ANCHOR JOINT then it's the anchor limb, so rotate on the center instead of on a joint. */
					Point anchorPoint =
						//getAnchorJointId() < 0 ? joint.getPoint() : /* THIS is to test the getRotatedPoint */
						getAnchorJointId() < 0 ? Point(textureWidth / 2, textureHeight / 2) :
						getAnchorJoint().getFormPoint();

					joint.setModifiedPoint(getRotatedPoint(anchorPoint, joint.getFormPoint(), rotationAngle));
				}
			}

			return rotationAngle;
		}

		bool hasFreeJoint() {
			for (Joint& joint : joints) {
				if (joint.isFree()) { return true; } }
			return false;
		}

		int getNextFreeJointIndex(int startingIndex = 0) {
			for (int i = startingIndex; i < joints.size(); ++i) {
				if (joints[i].isFree()) {
					return i;
				}
			}
			return -1;
		}

		vector<int> getFreeJointIndexes() {
			vector<int> indexes;
			for (int i = 0; i < joints.size(); ++i) {
				if (joints[i].isFree()) {
					indexes.push_back(i);
				}
			}
			return indexes;
		}

		/*
		* When the player has loaded a limb which is the CHILD OF (connected to) this limb,
		* we call this function on this limb and shift the location of the child (limbId).
		*/
		bool shiftJointOfLimb(int limbId) {
			/* again, we're using the index but must replace with actual limbId. */

			/* Make sure there is a free joint. (maybe useless?) */
			if (!hasFreeJoint()) { return false; }

			/* Get the current joint index. */

			int oldJointIndex = -1;
			int newJointIndex = -1;
			int anchorJointIndex = -1;

			for (int i = 0; i < joints.size(); i++) {
				if (joints[i].getConnectedLimbId() == limbId) {
					anchorJointIndex = joints[i].getChildLimbAnchorJointIndex();
					oldJointIndex = i;
					break; } }

			if (oldJointIndex < 0) { return false; }

			/* Find the next available joint index. Start ABOVE the current one. */

			for (int i = oldJointIndex + 1; i < joints.size(); ++i) {
				if (joints[i].isFree()) {
					newJointIndex = i;
					break; } }

			/* If we didn't find a free joint above the old one, start at the beginning. */
			if (newJointIndex < 0) {
				for (int i = 0; i < oldJointIndex; ++i) {
					if (joints[i].isFree()) {
						newJointIndex = i;
						break; } } }

			if (newJointIndex >= 0 && oldJointIndex >= 0 && anchorJointIndex >= 0) {
				/* make the switch. */
				joints[oldJointIndex].detachLimb();
				joints[newJointIndex].connectLimb(limbId, anchorJointIndex);
				return true;
			}

			return false;
		}

		/*
		* In the character creation screen, when the user wants to change which joint
		* the limb uses to anchor itself to its parent limb, this function does that.
		* Cycles through joints, and if one is available we do the switch.
		* Returns boolean indicating whether a switch took place.
		*/
		bool shiftAnchorLimb() {
			if (!isEquipped() || joints.size() == 1) { return false; }
			int oldAnchorId = -1;
			int newAnchorId = -1;

			/* Get the old anchor id. */
			for (int i = 0; i < joints.size(); ++i) {
				if (joints[i].getIsAnchor()) {
					oldAnchorId = i;
					break;
				}
			}

			if (oldAnchorId < 0) { return false; }

			/* Cycle through joints ABOVE the oldAnchorId. */
			if (oldAnchorId < joints.size() - 1) {
				for (int i = oldAnchorId + 1; i < joints.size(); ++i) {
					if (joints[i].isFree()) {
						newAnchorId = i;
						break;
					}
				}
			}

			/* If that didn't work, start from the beginning. */
			if (newAnchorId < 0) {
				for (int i = 0; i < oldAnchorId; ++i) {
					if (joints[i].isFree()) {
						newAnchorId = i;
						break;
					}
				}
			}

			/* If we found both new and old anchor positions, do the switch. */
			if (oldAnchorId >= 0 && newAnchorId >= 0) {
				joints[newAnchorId].setAnchor(true);
				joints[oldAnchorId].setAnchor(false);

				/* Now that we did the switch, update the joint points based on the rotation angle on the NEW anchor joint. */
				if (rotationAngle != 0) {
					int totalAngle = rotationAngle;
					resetRotationAngle();
					rotate(totalAngle);
				}

				return true;
			}

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
		bool isAnchor;
		SDL_Rect drawRect;
		int rotationAngle;
		int textureWidth;
		int textureHeight;
		int id;
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
	void clearSuit() {
		for (Limb& limb : limbs) {
			limb.unEquip(); }
		anchorLimbId = -1;
	}

	void unEquipLimb(int limbId) {
		/* First check if this is the anchor limb. */
		if (anchorLimbId == limbId) {
			anchorLimbId = -1; }		

		/* Next remove reference from parent limb. */
		for (Limb& limb : limbs) {
			for (Joint& joint : limb.getJoints()) {
				if (joint.getConnectedLimbId() == limbId) {
					joint.detachLimb(); } } }

		/* recursively unequip limbs and child limbs. */
		Limb& baseLimb = limbs[limbId];
		for (int i = 0; i < baseLimb.getJoints().size(); ++i) {
			Joint& joint = baseLimb.getJoints()[i];
			int connectedLimbId = joint.getConnectedLimbId();
			if (connectedLimbId >= 0) {
				unEquipLimb(connectedLimbId); } }

		baseLimb.unEquip();
	}

	int getParentLimbId(int childLimbId) {

		for (int i = 0; i < limbs.size(); ++i) {
			vector<Joint> theseJoints = limbs[i].getJoints();

			for (int k = 0; k < theseJoints.size(); ++k) {
				if (theseJoints[k].getConnectedLimbId() == childLimbId) {
					return i;
				}
			}
		}

		return -1;
	}

	Limb& getLimbById(int limbId) {
		/* FOR NOW return limb by vector index.
		* This is where we'll search for the limb with the actual DB ID later.
		*/

		return limbs[limbId];
	}

	/*
	* returns vector of: tuple<limbId, jointId, isFree>
	*/
	vector<tuple<int, int, bool>> getEquippedJointsData(int limbToSkipId = -1) {
		vector<tuple<int, int, bool>> jointsData;

		for (int i = 0; i < limbs.size(); ++i) {
			if (i == limbToSkipId) {
				continue;
			}

			Limb& thisLimb = limbs[i];

			if (!thisLimb.isEquipped()) { continue; }
			vector<Joint>& theseJoints = thisLimb.getJoints();

			for (int k = 0; k < theseJoints.size(); ++k) {
				Joint& thisJoint = theseJoints[k];
				tuple<int, int, bool> thisTuple = make_tuple(i, k, thisJoint.isFree());
				jointsData.push_back(thisTuple);
			}
		}

		return jointsData;
	}


	int getAnchorLimbId() { return anchorLimbId; }
	Limb& getAnchorLimb() { return limbs[anchorLimbId]; }
	tuple<int, int> getLimbIdAndJointIndexForConnection(int limbIdToSearch, int limbIdToExclude = -1);
	void setAnchorLimbId(int newId) { anchorLimbId = newId; }

	/*
	* Move the loaded limb to the next available joint in the character's equipped limbs.
	* Also, cycle back to the beginning of the list once we reach the end.
	*/
	bool shiftChildLimb(int childLimbId) {
		if (getAnchorLimbId() == childLimbId) {
			/* Don't do it if this is the anchor limb (no parent limb with limbs through which to cycle). */
			return false;
		}
		else {
			/* Try to get the parent limb ID and the parent limb itself. */
			int parentLimbId = getParentLimbId(childLimbId);
			if (parentLimbId < 0) { return false; }
			Limb& parentLimb = getLimbs()[parentLimbId];

			/* Try to get the index of the joint (in the parent limb) holding the loaded limb. */
			int parentJointIndex = -1;
			for (int i = 0; i < parentLimb.getJoints().size(); ++i) {
				if (parentLimb.getJoints()[i].getConnectedLimbId() == childLimbId) {
					parentJointIndex = i;
					break;
				}
			}

			if (parentJointIndex < 0) { return false; }

			/* Get a list of ALL the equipped joints in the character (excluding loaded limb and its children). */
			vector<tuple<int, int, bool>> jointsData = getEquippedJointsData(childLimbId);

			/* Find out where the PARENT joint is located in that list. */
			int parentJointIndexInJointsDataVector = -1;

			for (int i = 0; i < jointsData.size(); ++i) {

				int thisLimbId = get<0>(jointsData[i]);
				int thisJointId = get<1>(jointsData[i]);

				if (thisLimbId == parentLimbId && thisJointId == parentJointIndex) {
					parentJointIndexInJointsDataVector = i;
				}
			}

			if (parentJointIndexInJointsDataVector < 0) { return false; }

			/* Now check ABOVE the current location for a free joint. */

			int newParentJointIndex = -1;
			int newParentLimbId = -1;

			if (parentJointIndexInJointsDataVector < jointsData.size() - 1) {
				for (int i = parentJointIndexInJointsDataVector + 1; i < jointsData.size(); ++i) {
					bool thisJointIsFree = get<2>(jointsData[i]);
					if (thisJointIsFree) {
						newParentLimbId = get<0>(jointsData[i]);
						newParentJointIndex = get<1>(jointsData[i]);
						break;
					}
				}
			}

			/* Check IF we got one from above. And if not, get one from below. */

			if (parentJointIndexInJointsDataVector > 0 && (newParentJointIndex < 0 || newParentLimbId < 0)) {
				for (int i = 0; i < parentJointIndexInJointsDataVector; ++i) {
					bool thisJointIsFree = get<2>(jointsData[i]);
					if (thisJointIsFree) {
						newParentLimbId = get<0>(jointsData[i]);
						newParentJointIndex = get<1>(jointsData[i]);
						break;
					}
				}
			}

			/* If we caught something, do the switch. */
			if (newParentJointIndex >= 0 && newParentLimbId >= 0) {
				/* Detach limb from old parent. */
				Limb& newParentLimb = getLimbById(newParentLimbId);

				if (newParentLimb.isEquipped()) {
					parentLimb.getJoints()[parentJointIndex].detachLimb();
					Joint& newParentJoint = newParentLimb.getJoints()[newParentJointIndex];
					int loadedLimbAnchorJointId = getLimbById(childLimbId).getAnchorJointId();
					newParentJoint.connectLimb(childLimbId, loadedLimbAnchorJointId);

					return true;
				}
			}
		}

		return false;
	}

protected:
	CharacterType characterType;
	int id;
	int anchorLimbId; /* Currently using INDEX for dev purposes... will replace with actual DB ID. */
	vector<Limb> limbs;
};

/* 
* Search the given limb (id) for an available joint.
* If none are free, recursively search the connected limbs for free joints.
* 
* Returns a tuple.
* FIRST int is the limb id.
* SECOND int is its free joint ID.
*/
tuple<int, int> Character::getLimbIdAndJointIndexForConnection(int limbIdToSearch, int limbIdToExclude) {
	tuple<int, int> failureTuple = make_tuple(-1, -1);
	if (limbIdToSearch < 0) {
		return failureTuple;
	}
	
	Limb& limbToSearch = limbs[limbIdToSearch];
	/* Search the joints for a free joint. Hopfeully we find something here. */
	for (int i = 0; i < limbToSearch.getJoints().size(); ++i) {
		Joint& limbToSearchJoint = limbToSearch.getJoints()[i];
		if (limbToSearchJoint.isFree()) {
			return make_tuple(limbIdToSearch, i);			
		}
	}

	/* This limb has no free joints.
	* Now we cycle through each joint and get its connected limb, to search ITS joints for a free joint.
	*/
	for (int i = 0; i < limbToSearch.getJoints().size(); ++i) {
		Joint& limbToSearchJoint = limbToSearch.getJoints()[i];

		/* Anchor joints do not hold connections to child limbs. So only search non-anchor joints. */
		if (!limbToSearchJoint.getIsAnchor()) {
			int connectedLimbId = limbToSearchJoint.getConnectedLimbId();
			if (connectedLimbId == limbIdToExclude || connectedLimbId < 0) { continue; }

			Limb& nestedLimbToSearch = limbs[connectedLimbId];
			tuple<int, int> limbIdAndJointIndexForConnection =
				getLimbIdAndJointIndexForConnection(connectedLimbId, limbIdToExclude);

			int limbIdForConnection = get<0>(limbIdAndJointIndexForConnection);
			int jointIndexForConnection = get<1>(limbIdAndJointIndexForConnection);

			/* 
			* If we found a connection (limb ID and its free joint index), return it.
			* Otherwise, do the recursion.
			*/
			if (limbIdForConnection < 0 || jointIndexForConnection < 0) {
				continue;
			}
			else {
				return limbIdAndJointIndexForConnection; }
		}
	}

	/* Recursive search has failed completely. Return indication of failure. */
	return failureTuple;
}

/*
* Equip given limb (id).
* If no other limbs are equipped, make this limb the base/anchor limb.
* If other limbs are equipped, search their joints (or connected limbs recursively)
* for an available joint.
*/
bool Character::equipLimb(int limbId) {
	Limb& limbToEquip = limbs[limbId];// THIS will get the limb WITH the ID later, not by index!
	if (limbToEquip.isEquipped()) {
		cout << "Limb is already equipped!\n";
		return false;
	}

	/* When there are no limbs equipped, make this limb the FIRST (anchor) limb. */
	if (anchorLimbId < 0) {
		anchorLimbId = limbId;
		limbToEquip.setAnchor(true);
		return true;
	}

	/* 
	* Call the recursive function which finds a free joint for a connection.
	*/

	tuple<int, int> limbIdAndJointIndexForConnection = getLimbIdAndJointIndexForConnection(anchorLimbId);
	int limbIdForConnection = get<0>(limbIdAndJointIndexForConnection);
	int jointIndexForConnection = get<1>(limbIdAndJointIndexForConnection);

	if (limbIdForConnection < 0 || jointIndexForConnection < 0) {
		/* Recursive search failed. Return failure. */
		return false;
	}

	Joint& jointToConnect = limbs[limbIdForConnection].getJoints()[jointIndexForConnection];

	/* Now connect the actual limb (find a free joint on the limb we're trying to connect). */
	for (int i = 0; i < limbToEquip.getJoints().size(); ++i) {
		Joint& jointToEquip = limbToEquip.getJoints()[i];

		if (jointToEquip.isFree()) {
			jointToEquip.setAnchor(true);
			jointToConnect.connectLimb(limbId, i);
			return true;
		}
	}

	/* Failed to find a free joint on the limb we've been trying to equip (unlikely... should be impossible). */
	return false;
}

/*
*			EXTRA HELPER FUNCTIONS
*/


int normalizeAngle(int angle) {
	if (angle < 360 && angle >= 0) {
		return angle;
	}
	else {
		if (angle >= 360) {
			angle = angle - 360;
		}
		else if (angle < 0) {
			angle = angle + 360;
		}

		return normalizeAngle(angle);
	}
}