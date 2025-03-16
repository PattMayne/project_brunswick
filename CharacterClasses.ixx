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
import <limits>;
import <unordered_map>;
import <unordered_set>;

import TypeStorage;
import UI;

using namespace std;

export enum CharacterType { Player, Hostile, Friendly, Suit, None }; /* NOT a CLASS because we want to use it as int. */
/* Red beats Green (fire consumes life), Green beats Blue (life consumes water), Blue beats Red (water extinguishes fire) */
export enum class LimbState { Free, Owned, Equipped }; /* If it is OWNED or EQUIPPED, then there must be a character id. Every character should exist in the DB.*/

class Limb;
class Character;
int normalizeAngle(int angle);

/* Where the limb image will be drawn onto the character surface. */
export struct SuitLimbPlacement {
	string slug;
	Point position;
};

export bool compareDrawOrder(Limb& limbA, Limb& limbB);
export bool compareJointsNumber(Limb& limbA, Limb& limbB);

struct AvatarDimensionsStruct {
	AvatarDimensionsStruct() {
		leftmost = 0;
		rightmost = 0;
		topmost = 0;
		bottommost = 0;
		avatarWidth = 0;
		avatarHeight = 0;
		greaterDimension = 0;
	}

	/* Extreme reach of textures. */
	int leftmost;
	int rightmost;
	int topmost;
	int bottommost;

	/* Final dimensions calcualted from extreme reach of textures. */
	int avatarWidth;
	int avatarHeight;

	/* Greater dimension allows us to make a square. */
	int greaterDimension;
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
	BodyPartType type;

	/* CONSTRUCTOR */
	LimbForm(
		string name, string slug,
		int hp, int strength, int speed, int intelligence,
		DominanceNode domNode, string texturePath,
		vector<Point> jointPoints, BodyPartType type)
		:
		name(name), slug(slug),
		hp(hp), strength(strength), speed(speed),
		intelligence(intelligence), domNode(domNode),
		texturePath(texturePath), jointPoints(jointPoints), type(type) {
	}
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
	MapType mapType;
	string name;
	string slug;
	vector<LimbForm> nativeLimbs; /* This is ALL the map's limb FORMs, not just those belonging to a Suit. */
	vector<Character> suits;
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
		id = -1;
	}

	/* When we create a new Joint for a new Limb. */
	Joint(Point pointForm) : pointForm(pointForm), isAnchor(false), connectedLimbId(-1), anchorJointIndex(-1) {
		resetModifiedPoint();
	}

	/* When we load a joint from the database. */
	Joint(
		Point pointForm,
		Point modifiedPoint,
		bool isAnchor,
		int connectedLimbId,
		int anchorJointIndex,
		int id = -1
	) :
		pointForm(pointForm),
		modifiedPoint(modifiedPoint),
		isAnchor(isAnchor),
		connectedLimbId(connectedLimbId),
		anchorJointIndex(anchorJointIndex),
		id(id)
	{ }

	void setId(int id) { this->id = id; }
	void setAnchor(bool makeAnchor = true) { isAnchor = makeAnchor; }
	void connectLimb(int limbId, int jointIndex) {
		connectedLimbId = limbId;
		anchorJointIndex = jointIndex;
	}

	void detachLimb() {
		connectedLimbId = -1;
		anchorJointIndex = -1; }

	bool isFree() { return !isAnchor && connectedLimbId < 0; }

	int getId() { return id; }
	bool getIsAnchor() { return isAnchor; }
	int getConnectedLimbId() { return connectedLimbId; }
	int getChildLimbAnchorJointIndex() { return anchorJointIndex; }
	Point getFormPoint() { return pointForm; }
	Point getPoint() { return modifiedPoint; }
	void resetModifiedPoint() {
		modifiedPoint = { pointForm.x, pointForm.y }; }
	void setModifiedPoint(Point newPoint) { modifiedPoint = newPoint; }

private:
	Point pointForm; /* ORIGINAL point from LimbForm. */
	Point modifiedPoint; /* NOT point MODIFIER. It's the fully modified point. Saved to DB. */
	bool isAnchor;

	/* Data about the CONNECTED limb. */
	int connectedLimbId;
	int anchorJointIndex; /* This is the anchor joint of the CHILD limb. */
	int id;
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
* The core component of a Character.
* Each Limb contains the character's attributes (strength, intelligence, hp, speed).
* 
* Limb should have a LimbForm, and modifiers.
* Only the modifiers are saved to the DB.
* The Form is retrieved fresh from the definition every time the Limb is constructed/instantiated.
* 
* Let a Limb take a Form as its constructor... and a 2nd consrtructor to rebuild existing (or just more specific for any reason) limbs.
* 
* Limb textures will be destroyed by the screens handling them.
*/
export class Limb {
	public:
		/* constructor for NEW Limb. */
		Limb(LimbForm form) : form(form) {
			name = form.name; /* Name can POSSIBLE be changed (no plans for this yet) */
			hpMod = 0;
			strengthMod = 0;
			speedMod = 0;
			intelligenceMod = 0;
			position = Point(50, 95);
			lastPosition = Point(0, 0);
			isAnchor = false;
			rotationPointSDL = SDL_Point(0, 0);

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
			setAnchorJointId();
			drawOrder = -1;
			isUnscrambled = false;
		}

		/* Constructor to rebuild a limb from DB (primarily for roaming limbs).
		*/
		Limb(int id, LimbForm form, Point position, vector<Joint> joints, string mapSlug, int drawOrder = -1) :
			id(id), form(form), position(position), joints(joints), drawOrder(drawOrder), mapSlug(mapSlug)
		{
			lastPosition = Point(0, 0);
			isAnchor = false;
			name = form.name;
			/*
			* Get the TEXTURE.
			* Get the JOINTS.
			*/

			/* get Limb texture */

			UI& ui = UI::getInstance();
			SDL_Surface* limbSurface = IMG_Load(form.texturePath.c_str());
			texture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), limbSurface);
			SDL_FreeSurface(limbSurface);
			/* TO DO: ERROR HANDLING FOR TEXTURE. */

			drawRect = { 0, 0, 0, 0 };
			rotationAngle = 0;
			SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);
			setAnchorJointId();
			isUnscrambled = false;

			hpMod = 0;
			strengthMod = 0;
			speedMod = 0;
			intelligenceMod = 0;

		}

		/* Define equality based on the id. */
		bool operator==(const Limb& other) const {
			return id == other.id;  
		}
		
		void setFlipped(bool flip) { flipped = flip; }
		void flip() { flipped = !flipped; }
		Point getPosition() { return position; }
		Point getLastPosition() { return lastPosition; }
		LimbForm getForm() { return form; }
		int getDrawOrder() { return drawOrder; }

		string getName() { return name; }
		string getTexturePath() { return form.texturePath; }
		string getFormSlug() { return form.slug; }
		int getCharacterId() { return characterId; }
		string getMapSlug() { return mapSlug; }
		int getId() { return id; }

		/* GET the FORM (default) values PLUS the modifiers (which can be negative) */
		int getHP() { return form.hp + hpMod; }
		int getStrength() { return form.strength + strengthMod; }
		int getSpeed() { return form.speed + speedMod; }
		int getIntelligence() { return form.intelligence + intelligenceMod; }
		int getRotationAngle() { return rotationAngle; }

		int getHpMod() { return hpMod; }
		int getStrengthMod() { return strengthMod; }
		int getSpeedMod() { return speedMod; }
		int getIntelligenceMod() { return intelligenceMod; }

		SDL_Rect& getDrawRect() { return drawRect; }
		SDL_Texture* getTexture() { return texture; }
		bool getIsAnchor() { return isAnchor; }
		bool getIsFlipped() { return flipped; }

		BodyPartType getBodyPartType() { return form.type; }
		DominanceNode getDominanceNode() { return form.domNode; }
		
		int modifyStrength(int mod);
		int modifySpeed(int mod);
		int modifyIntelligence(int mod);
		int modifyHP(int mod);

		vector<Joint>& getJoints() { return joints; }
		bool isEquipped();
		Joint& getAnchorJoint();
		int getAnchorJointId();
		LimbButtonData getLimbButtonData();
		
		int rotate(int angleIncrement);
		bool getUnscrambled() { return isUnscrambled; }
		bool hasFreeJoint();
		vector<int> getFreeJointIndexes();
		int getNextFreeJointIndex(int startingIndex);

		bool shiftJointOfLimb(int limbId);
		bool shiftAnchorLimb();

		void setTexture(SDL_Texture* newTexture);
		void destroyTexture();
		void setUnscrambled(bool isUnscrambled) { this->isUnscrambled = isUnscrambled; }
		void setAnchorJointId();
		void setDrawOrder(int newDrawOrder) { this->drawOrder = newDrawOrder; }
		void setId(int id) { this->id = id; }
		void setName(string newName) { name = newName; }
		void setCharacterId(int id) { characterId = id; }
		void setMapSlug(string newSlug) { mapSlug = newSlug; }
		void setPosition(Point newPosition) { position = newPosition; }
		void setLastPosition(Point newPosition) { lastPosition = newPosition; }
		void modifyAttribute(int& attributeMod, int formAttribute, int modMod);
		void setAnchor(bool isAnchor = true) { this->isAnchor = isAnchor; }
		void move(Point newPosition);
		void setRotationPointSDL();
		void draw(UI& ui, bool drawJoints);
		void setDrawRect(SDL_Rect drawRect);
		void resetRotationAngle();
		void unscramble() { isUnscrambled = true; }
		void unEquip();
		void heal();

		SDL_Point getRotationPointSDL() { return rotationPointSDL; }		


	protected:
		LimbForm form;
		string name;
		string mapSlug;
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
		int characterId;
		int id;
		SDL_Point rotationPointSDL;
		int anchorJointId;
		int drawOrder;

		bool isUnscrambled = false; /* Only used by Suit type characters. */
};

/*
* 
*  _     _           _                         
* | |   (_)_ __ ___ | |__                      
* | |   | | '_ ` _ \| '_ \                     
* | |___| | | | | | | |_) |                    
* |_____|_|_| |_| |_|_.__/   _                 
* |  ___|   _ _ __   ___| |_(_) ___  _ __  ___ 
* | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
* |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
* |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
* 
* 
*/




void Limb::heal() {
	if (hpMod < 0) {
		hpMod = 0;
	}
}


/* SET the modifiers. */

int Limb::modifyStrength(int mod) {
	modifyAttribute(strengthMod, form.strength, mod);
	return getStrength();
}

int Limb::modifySpeed(int mod) {
	modifyAttribute(speedMod, form.speed, mod);
	return getSpeed();
}

int Limb::modifyIntelligence(int mod) {
	modifyAttribute(intelligenceMod, form.intelligence, mod);
	return getIntelligence();
}

/* HP is different. It can go down to 0, but can still only be boosted by 1/2. */
int Limb::modifyHP(int mod) {
	hpMod += mod;
	/* Check if updated attributeMod goes beyond limit */
	if (hpMod > form.hp / 2) {
		hpMod = form.hp / 2;
	}
	return getHP();
}



bool Limb::isEquipped() {
	if (getIsAnchor()) {
		return true;
	}
	for (Joint& joint : joints) {
		if (!joint.isFree()) { return true; }
	}
	return false;
}

/* This can be MAP position or DRAW position. */
void Limb::move(Point newPosition) {
	lastPosition = position;
	position = newPosition;
}

void Limb::modifyAttribute(int& attributeMod, int formAttribute, int modMod) {
	attributeMod += modMod;
	/* Check if updated attributeMod goes beyond limit */
	if (attributeMod > formAttribute / 2) {
		attributeMod = formAttribute / 2;
	}
	else if ((attributeMod * -1) > formAttribute / 2) { /* check limit for detriment too */
		attributeMod = formAttribute / (-2);
	}
}


/* TO DO: This must also remove the limb id from the character's drawLimbList.
* DEPRECATED.
* Try to use limb.unEquip() instead, because it detaches child limbs.
*/
void Limb::unEquip() {
	for (Joint& joint : joints) {
		joint.setAnchor(false);
		joint.detachLimb();
		joint.resetModifiedPoint();
	}
	isAnchor = false;
	rotationAngle = 0;
	drawOrder = -1;
}

Joint& Limb::getAnchorJoint() {
	for (Joint& joint : joints) {
		if (joint.getIsAnchor()) {
			return joint;
		}
	}
	/* THIS IS NOT SAFE. */
	return joints[0];
}

void Limb::setAnchorJointId() {
	for (int i = 0; i < joints.size(); ++i) {
		Joint& joint = joints[i];
		if (joint.getIsAnchor()) {
			anchorJointId = i;
			return;
		}
	}
	anchorJointId = -1;
}

int Limb::getAnchorJointId() {
	return anchorJointId;
}

void Limb::setDrawRect(SDL_Rect drawRect) {
	this->drawRect = drawRect;
}

//void setRotationAngle(int rotationAngle) { this->rotationAngle = normalizeAngle(rotationAngle); }
void Limb::resetRotationAngle() {
	rotationAngle = 0;
	for (Joint& joint : joints) {
		joint.resetModifiedPoint();
	}
}


LimbButtonData Limb::getLimbButtonData() {
	return LimbButtonData(
		getTexturePath(), name, id, getHP(), getStrength(),
		getIntelligence(), getSpeed(), getDominanceNode());
}



int Limb::rotate(int angleIncrement) {
	rotationAngle = normalizeAngle(rotationAngle + angleIncrement);

	/* If this LIMB has no ANCHOR JOINT then it's the anchor limb, so rotate on the center instead of on a joint. */
	Point anchorPoint =
		//getAnchorJointId() < 0 ? joint.getPoint() : /* THIS is to test the getRotatedPoint */
		getAnchorJointId() < 0 ? Point(textureWidth / 2, textureHeight / 2) : /* This is the correct one to use. */
		getAnchorJoint().getFormPoint();

	/* Now update all the joint points (except the anchor point). */
	for (Joint& joint : joints) {
		if (!joint.getIsAnchor()) {
			joint.setModifiedPoint(getRotatedPoint(anchorPoint, joint.getFormPoint(), rotationAngle));
		}
	}

	return rotationAngle;
}



bool Limb::hasFreeJoint() {
	for (Joint& joint : joints) {
		if (joint.isFree()) { return true; }
	}
	return false;
}

int Limb::getNextFreeJointIndex(int startingIndex = 0) {
	for (int i = startingIndex; i < joints.size(); ++i) {
		if (joints[i].isFree()) {
			return i;
		}
	}
	return -1;
}

vector<int> Limb::getFreeJointIndexes() {
	vector<int> indexes;
	for (int i = 0; i < joints.size(); ++i) {
		if (joints[i].isFree()) {
			indexes.push_back(i);
		}
	}
	return indexes;
}



/*
* In the character creation screen, when the user wants to change which joint
* the limb uses to anchor itself to its parent limb, this function does that.
* Cycles through joints, and if one is available we do the switch.
* Returns boolean indicating whether a switch took place.
*/
bool Limb::shiftAnchorLimb() {
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


/*
* When the player has loaded a limb which is the CHILD OF (connected to) this limb,
* we call this function on this limb and shift the location of the child (limbId).
*/
bool Limb::shiftJointOfLimb(int limbId) {
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
			break;
		}
	}

	if (oldJointIndex < 0) { return false; }

	/* Find the next available joint index. Start ABOVE the current one. */

	for (int i = oldJointIndex + 1; i < joints.size(); ++i) {
		if (joints[i].isFree()) {
			newJointIndex = i;
			break;
		}
	}

	/* If we didn't find a free joint above the old one, start at the beginning. */
	if (newJointIndex < 0) {
		for (int i = 0; i < oldJointIndex; ++i) {
			if (joints[i].isFree()) {
				newJointIndex = i;
				break;
			}
		}
	}

	if (newJointIndex >= 0 && oldJointIndex >= 0 && anchorJointIndex >= 0) {
		/* make the switch. */
		joints[oldJointIndex].detachLimb();
		joints[newJointIndex].connectLimb(limbId, anchorJointIndex);
		return true;
	}

	return false;
}


void Limb::setTexture(SDL_Texture* newTexture) {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}
	texture = newTexture;
}

void Limb::destroyTexture() {
	if (texture != NULL) {
		SDL_DestroyTexture(texture);
	}
	texture = NULL;
}


void Limb::setRotationPointSDL() {
	setAnchorJointId();
	if (getAnchorJointId() < 0) {
		rotationPointSDL = SDL_Point(textureWidth / 2, textureHeight / 2);
	}
	else {
		Point anchorPoint = getJoints()[getAnchorJointId()].getPoint();
		rotationPointSDL = SDL_Point(anchorPoint.x, anchorPoint.y);
	}
}

void Limb::draw(UI& ui, bool drawJoints = false) {
	/* rotationPoint should already be an SDL_Point in the Joint object.
		* My Point object is redundant.
		* WAIT... MAYBE NOT.
		* There is no "rotation point", there are just joint points or else NULL.
		* Further consideration is necessary.
		* Maybe I do need a rotationPoint member which holds an SDL_Point pointer... GOOD IDEA.
		*/

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		getTexture(),
		NULL, &getDrawRect(),
		getRotationAngle(), &rotationPointSDL, SDL_FLIP_NONE
	);

	//if (drawJoints) { drawJoints(limb, ui); } /* For debugging. */
}




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
	Character() : anchorLimbId(-1) {}
	~Character() { }
	Character(CharacterType characterType, SuitType suitType = SuitType::NoSuit) :
		characterType(characterType), anchorLimbId(-1), suitType(suitType) 
	{
		texture = NULL;
		setLatestLandmarkId(-1);
	}

	/**/
	Character(CharacterType characterType, vector<Limb> limbs, string name, SuitType suitType = SuitType::NoSuit) :
		characterType(characterType), limbs(limbs), name(name), anchorLimbId(-1), suitType(suitType)
	{
		texture = NULL;
		setLatestLandmarkId(-1);
	}

	Character(CharacterType characterType, int x, int y, SuitType suitType = SuitType::NoSuit) :
		characterType(characterType), blockPosition(x, y), lastBlockPosition(x, y), suitType(suitType)
	{
		texture = NULL;
		setLatestLandmarkId(-1);
	}

	/* constructor for when hostile NPC MapCharacter is created. */
	Character(int id, string name, int anchorLimbId, Point position, vector<Limb> limbs, SuitType suitType = SuitType::NoSuit) :
		id(id), name(name), anchorLimbId(anchorLimbId), blockPosition(position),
		lastBlockPosition(position), limbs(limbs), characterType(CharacterType::Hostile), suitType(suitType)
	{
		texture = NULL;
		setLatestLandmarkId(-1);
	}

	void equipLimbsDefault();
	unordered_set<int> getChildLimbIdsRecursively(Limb& parentLimb, unordered_set<int> childLimbIds = {});
	string getName() { return name; }
	string getMapSlug() { return mapSlug; }
	bool shiftChildLimb(int childLimbId);
	bool equipLimb(int limbId);
	vector<Limb> getEquippedLimbs();
	vector<int>& getDrawLimbIDs() { return drawLimbListIDs; }
	vector<tuple<int, int, bool>> getEquippedJointsData(int limbToSkipId);
	vector<Limb>& getLimbs() { return limbs; }
	Limb& getLimbById(int id);
	Limb& getAnchorLimb() { return getLimbById(anchorLimbId); }
	int getType() { return characterType; }
	int getId() { return id; }
	int getParentLimbId(int childLimbId);
	int getNumberOfEquippableLimbs();
	int getAnchorLimbId() { return anchorLimbId; }
	tuple<int, int> getLimbIdAndJointIndexForConnection(int limbIdToSearch, int limbIdToExclude = -1);
	SDL_Texture* createAvatar(bool resetRenderer = true);
	SDL_Texture* getTexture() { return texture; } /* This must move to the parent class. */
	SuitType getSuitType() { return suitType; }
	vector<int> getDrawLimbIndexes() { return drawLimbListIndexes; }
	vector<AttackStruct> getAttacks();
	unordered_set<int> getChildLimbIds(Limb& parentLimb, unordered_set<int> collectedIds = {});

	int getHP();
	int getIntelligence();
	int getSpeed();
	int getStrength();

	int getBlockX() { return blockPosition.x; }
	int getBlockY() { return blockPosition.y; }
	bool hasScrambledLimbs();
	bool limbsContainId(int idQuery);
	bool isGameOver();
	int getNumberOfEquippedLimbs();

	int getLastX() { return lastBlockPosition.x; }
	int getLastY() { return lastBlockPosition.y; }
	int getLatestLandmarkId() { return latestLandmarkId; }

	Point getPosition() { return blockPosition; }
	Point getLastPosition() { return lastBlockPosition; }

	void setTexture(SDL_Texture* incomingTexture);
	void updateLastBlock();
	void moveToPosition(Point newPosition);
	void setBlockPosition(Point blockPosition) { this->blockPosition = blockPosition; }

	void setId(int id) { this->id = id; }
	void addLimb(Limb& newLimb) {
		limbs.emplace_back(newLimb);
	}
	void setName(string newName) { name = newName; }
	void setType(CharacterType type) { characterType = type; }
	void setAnchorLimbId(int newId) { anchorLimbId = newId; }
	void setLimbDrawOrder() { setLimbDrawOrder(drawLimbListIDs); }

	void setChildLimbDrawRects(Limb& parentLimb, UI& ui);
	void getChildLimbsRecursively(Limb& parentLimb, vector<Limb>& childLimbs);

	void setLimbDrawOrder(vector<int> limbIdsInDrawOrder);
	void unEquipLimb(int limbId);
	void clearSuit();
	void setRotationPointsSDL();
	void setAnchorJointIDs();
	void addToDrawLimbList(int limbId);
	void buildDrawLimbList();
	void checkChildLimbsForAvatarBoundaries(Limb& parentLimb, AvatarDimensionsStruct& dimStruct);
	void setLatestLandmarkId(int newId) { latestLandmarkId = newId; }
	void setMapSlug(string mapSlug) { this->mapSlug = mapSlug; }

	DominanceNode getDominanceNode();
	CharStatsData getCharStatsData();

	void sortLimbsByNumberOfJoints() {
		sort(limbs.begin(), limbs.end(), compareJointsNumber); }

	void sortLimbsByDrawOrder() {
		sort(limbs.begin(), limbs.end(), compareDrawOrder); }


protected:
	CharacterType characterType;
	int id;
	int anchorLimbId;
	vector<Limb> limbs;
	string name;
	string mapSlug;
	vector<int> drawLimbListIDs;
	vector<int> drawLimbListIndexes;
	Point blockPosition;
	Point lastBlockPosition;
	SuitType suitType;
	SDL_Texture* texture;
	int latestLandmarkId;
};



/*
* CHARACTER CLASS FUNCTIONS.
*   ____ _                          _
*  / ___| |__   __ _ _ __ __ _  ___| |_ ___ _ __
* | |   | '_ \ / _` | '__/ _` |/ __| __/ _ \ '__|
* | |___| | | | (_| | | | (_| | (__| ||  __/ |
*  \____|_| |_|\__,_|_|  \__,_|\___|\__\___|_|
*  / ___| | __ _ ___ ___
* | |   | |/ _` / __/ __|
* | |___| | (_| \__ \__ \
*  \____|_|\__,_|___/___/_   _
* |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
* | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
* |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
* |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
*/


DominanceNode Character::getDominanceNode() {
	unordered_map<DominanceNode, int> domCounts;
	domCounts[DominanceNode::Green] = 0;
	domCounts[DominanceNode::Red] = 0;
	domCounts[DominanceNode::Blue] = 0;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			++domCounts[limb.getDominanceNode()];
		}
	}

	DominanceNode mostPlentifulDominanceNode = DominanceNode::Blue;

	if (domCounts[DominanceNode::Green] > domCounts[DominanceNode::Blue]) {
		mostPlentifulDominanceNode = DominanceNode::Green;
	}

	if (domCounts[DominanceNode::Red] > domCounts[DominanceNode::Green]) {
		mostPlentifulDominanceNode = DominanceNode::Red;
	}

	return mostPlentifulDominanceNode;
}

void Character::equipLimbsDefault() {
	clearSuit();
	sortLimbsByNumberOfJoints();

	bool keepEquippingLimbs = true;

	for (Limb& limb : limbs) {
		if (limb.getHP() < 1) { continue; }
		if (keepEquippingLimbs) {
			keepEquippingLimbs = equipLimb(limb.getId());
		}
		else { break; }
	}

	buildDrawLimbList();
}

unordered_set<int> Character::getChildLimbIdsRecursively(Limb& parentLimb, unordered_set<int> childLimbIds) {
	/* Protect against infinite recursion by checking for existing limb id, and returning if true. */

	for (Joint& joint : parentLimb.getJoints()) {
		int connectedLimbId = joint.getConnectedLimbId();
		if (connectedLimbId > 0 && limbsContainId(connectedLimbId)) {
			if (childLimbIds.count(connectedLimbId) < 1) {
				childLimbIds.insert(connectedLimbId);
				Limb& childLimb = getLimbById(connectedLimbId);
				childLimbIds = getChildLimbIdsRecursively(childLimb, childLimbIds);
			} else {
				/* We found a double. Bail before it loops.*/
				return childLimbIds;
			}
		}
	}

	return childLimbIds;
}

CharStatsData Character::getCharStatsData() {
	return CharStatsData(
		getName(),
		getHP(),
		getStrength(),
		getSpeed(),
		getIntelligence(),
		getDominanceNode()
	);
}


void Character::setTexture(SDL_Texture* incomingTexture) {
	if (texture && texture != NULL) {
		SDL_DestroyTexture(texture);
	}
	texture = incomingTexture;
}

void Character::updateLastBlock() {
	lastBlockPosition.x = blockPosition.x;
	lastBlockPosition.y = blockPosition.y;
}

/* This can be MAP position or DRAW position. */
void Character::moveToPosition(Point newPosition) {
	lastBlockPosition = blockPosition;
	blockPosition = newPosition;
}

int Character::getNumberOfEquippedLimbs() {
	int count = 0;
	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			++count;
		}
	}

	return count;
}

bool Character::hasScrambledLimbs() {
	int scrambledCount = 0;
	for (Limb& limb : limbs) {
		if (!limb.getUnscrambled()) {
			++scrambledCount;
		}
	}
	return scrambledCount != 0;
}


/* This will always be full HP.
* But it's also misleading, because cutting off a limb with many connections can scatter those limbs.
*/
int Character::getHP() {
	int hp = 0;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			hp += limb.getHP();
		}
	}

	return hp;
}

/* Intelligence is not balanced by anything. Raw number suffices. */
int Character::getIntelligence() {
	int intelligence = 0;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			intelligence += limb.getIntelligence();
		}
	}

	return intelligence;
}

/* This is just raw speed total.
* We might modify this to be balanced by number of joints.
*/
int Character::getSpeed() {
	int speed = 0;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			speed += limb.getSpeed();
		}
	}

	return speed;
}

/* This is just raw speed total.
* We might modify this to be balanced by number of limbs or joints.
*/
int Character::getStrength() {
	int strength = 0;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			strength += limb.getStrength();
		}
	}

	return strength;
}



/* Takes the list of Database IDs, sets the list of vectors which we use to actually draw. */
void Character::setLimbDrawOrder(vector<int> limbIdsInDrawOrder) {
	drawLimbListIDs = limbIdsInDrawOrder;

	drawLimbListIndexes = {};
	drawLimbListIndexes.resize(drawLimbListIDs.size());

	for (int i = 0; i < drawLimbListIDs.size(); ++i) {
		for (int k = 0; k < limbs.size(); ++k) {
			if (limbs[k].getId() == drawLimbListIDs[i]) {
				drawLimbListIndexes[i] = k;
			}
		}
	}
}



/* Takes an ID (from DB) of a limb, adds it to the list of IDs to draw,
* then sets the list of vectors to draw.
*/
void Character::addToDrawLimbList(int limbId) {
	drawLimbListIDs.push_back(limbId);
	setLimbDrawOrder(drawLimbListIDs);
}


/*
*  Ultimately builds the vector of the indexes of the limbs which are equipped,
* and therefore need to be drawn. Builds that vector according to their draw order.
* The order in which they are to be drawn.
* This allows any screen's draw function to get an easy list of indexes to
* quickly grab each limb to draw, during each frame.
*/
void Character::buildDrawLimbList() {
	drawLimbListIDs = {};
	if (anchorLimbId < 1 || !limbsContainId(anchorLimbId)) {
		setLimbDrawOrder(drawLimbListIDs);
		return;
	}

	sortLimbsByDrawOrder();
	int drawOrder = 0;
	unordered_set<int> allConnectedLimbs = getChildLimbIdsRecursively(getAnchorLimb());
	allConnectedLimbs.insert(anchorLimbId);

	for (Limb& limb : limbs) {
		/* Skip unscrambled Suit limbs. */
		if (suitType != SuitType::NoSuit && !limb.getUnscrambled()) { continue; }
		int limbId = limb.getId();

		if (limb.isEquipped() && allConnectedLimbs.count(limbId) > 0) {

			/* Check to make sure its child limbs are still in the vector. */
			for (Joint& joint : limb.getJoints()) {
				if (joint.getConnectedLimbId() > 0 && !limbsContainId(joint.getConnectedLimbId())) {
					joint.detachLimb();
				}
			}

			addToDrawLimbList(limbId);
			limb.setDrawOrder(drawOrder);
			++drawOrder;
		}
		else if (limb.getDrawOrder() >= 0) {
			limb.setDrawOrder(-1);
		}
	}
	setLimbDrawOrder(drawLimbListIDs);
}

/* Only to be used on Player Character. */
bool Character::isGameOver() {
	return latestLandmarkId < 1 && getNumberOfEquippableLimbs() < 1;
}


bool Character::limbsContainId(int idQuery) {
	for (Limb& limb : limbs) {
		if (limb.getId() == idQuery) {
			return true;
		}
	}

	return false;
}

/* 
* When drawing the avatar, we call this function to see how far the limbs' textures reach in each direction.
* AnchorLimb must set its boundaries before calling this recursive function on its child limbs.
* AnchorLimb x,y must be set to 0,0.
*/
void Character::checkChildLimbsForAvatarBoundaries(Limb& parentLimb, AvatarDimensionsStruct& dimStruct) {
	/* Check the dimensions of this limb. */

	SDL_Rect drawRect = parentLimb.getDrawRect();
	int tWidth;
	int tHeight;
	SDL_QueryTexture(parentLimb.getTexture(), NULL, NULL, &tWidth, &tHeight);

	/* Now we actually check the dimensions. */

	if (parentLimb.getRotationAngle() == 0) {
		/* The limb is NOT rotated. Check the normal dimensions. */

		if (drawRect.x < dimStruct.leftmost) {
			dimStruct.leftmost = drawRect.x;
		}
		else if ((drawRect.x + tWidth) > dimStruct.rightmost) {
			dimStruct.rightmost = drawRect.x + tWidth;
		}

		if (drawRect.y < dimStruct.topmost) {
			dimStruct.topmost = drawRect.y;
		}
		else if ((drawRect.y + tHeight) > dimStruct.bottommost) {
			dimStruct.bottommost = drawRect.y + tHeight;
		}

	}
	else {
		/* Limb is rotated. We must rotate each corner and check to see if it extends the bounds. */
		Point anchorPoint = parentLimb.getAnchorJoint().getPoint();
		int rotationAngle = parentLimb.getRotationAngle();

		Point topLeftPoint = Point(drawRect.x, drawRect.y);
		Point topRightPoint = Point(drawRect.x + drawRect.w, drawRect.y);
		Point bottomLeftPoint = Point(drawRect.x, drawRect.y + drawRect.h);
		Point bottomRightPoint = Point(drawRect.x + drawRect.w, drawRect.y + drawRect.h);

		/* Make a vector of the points so we can do a loop to check if they extend the boundaries. */
		vector<Point> rotatedPoints = {
			getRotatedPoint(anchorPoint, topLeftPoint, rotationAngle),
			getRotatedPoint(anchorPoint, topRightPoint, rotationAngle),
			getRotatedPoint(anchorPoint, bottomLeftPoint, rotationAngle),
			getRotatedPoint(anchorPoint, bottomRightPoint, rotationAngle)
		};

		for (Point point : rotatedPoints) {
			if (point.x < dimStruct.leftmost) {
				dimStruct.leftmost = point.x;
			}
			else if (point.x > dimStruct.rightmost) {
				dimStruct.rightmost = point.x;
			}

			if (point.y < dimStruct.topmost) {
				dimStruct.topmost = point.y;
			}
			else if (point.y > dimStruct.bottommost) {
				dimStruct.bottommost = point.y;
			}
		}
	}

	/* Now check each connected limb recursively. */
	for (Joint& joint : parentLimb.getJoints()) {
		int connectedLimbId = joint.getConnectedLimbId();
		if (connectedLimbId >= 0) {
			checkChildLimbsForAvatarBoundaries(getLimbById(connectedLimbId), dimStruct);
		}
	}
}

unordered_set<int> Character::getChildLimbIds(Limb& parentLimb, unordered_set<int> collectedIds) {
	unordered_set<int> childLimbIds = {};

	/* Avoid infintie recursion. */
	if (collectedIds.count(parentLimb.getId()) > 0) {
		return collectedIds;
	}

	for (Joint& joint : parentLimb.getJoints()) {
		int connectedLimbId = joint.getConnectedLimbId();

		if (connectedLimbId > 0 && limbsContainId(connectedLimbId)) {
			unordered_set<int> grandChildLimbIds = getChildLimbIds(getLimbById(connectedLimbId), childLimbIds);
			/* Add this ID after the recursive call to check for looped recursion. */
			childLimbIds.insert(connectedLimbId);

			for (int grandChildId : grandChildLimbIds) {
				childLimbIds.insert(grandChildId);
			}
		}
	}

	return childLimbIds;
}

/*
* Based on what kind of Limbs the character has equipped, generate some Attacks.
* These AttackStructs will be fed to the UI to make buttons.
* But it also gives the opponent their randomly-selected options on Battle screen.
*/
vector<AttackStruct> Character::getAttacks() {
	vector<AttackStruct> attackStructs = {};

	/*
	* All the Attacks should be stored in TypeStorage.
	* 
	* --> 2 heads : Brain Drain (absorb HP and intelligence)
	* --> 1 Torso : Body Slam (attack spread out on ALL limbs)
	* --> 1+ legs : Kick (strong attack, less precision)
	* --> 1+ arms : Punch (less strong, more precise)
	* --> 1+ arms : steal (more arms and more intelligence means more steal)
	* --> 2+ wings : Dive-bomb (Swoop?)
	*/

	int headCount = 0;
	int armCount = 0;
	int legCount = 0;
	int torsoCount = 0;
	int wingCount = 0;
	int otherCount = 0;

	int totalIntelligence = 0;
	int totalSpeed = 0;
	int totalStrength = 0;
	int totalHp = 0;

	int numberOfEquippedLimbs = 0;
	unordered_map<DominanceNode, int> dNodeScores;
	dNodeScores[DominanceNode::Blue] = 0;
	dNodeScores[DominanceNode::Green] = 0;
	dNodeScores[DominanceNode::Red] = 0;

	/* Scoring up some data. */
	for (Limb& limb : limbs) {
		if (!limb.isEquipped()) { continue; }
		++numberOfEquippedLimbs;
		++dNodeScores[limb.getDominanceNode()];

		/* Get the BodyPartType lists. */
		BodyPartType bpType = limb.getBodyPartType();
		if (bpType == BodyPartType::Arm) { ++armCount; }
		else if (bpType == BodyPartType::Leg) { ++legCount; }
		else if (bpType == BodyPartType::Wing) { ++wingCount; }
		else if (bpType == BodyPartType::Head) { ++headCount; }
		else if (bpType == BodyPartType::Torso) { ++torsoCount; }
		else if (bpType == BodyPartType::Other) { ++otherCount; }

		/* Tally the attributes. */
		totalIntelligence += limb.getIntelligence();
		totalSpeed += limb.getSpeed();
		totalStrength += limb.getStrength();
		totalHp += limb.getHP();
	}

	/*
	* Find the dominant dominance score.
	* Use functional, a lambda or whatever here.
	*/

	DominanceNode dominantNode = DominanceNode::Blue;

	if (dNodeScores[DominanceNode::Green] > dNodeScores[dominantNode]) {
		dominantNode = DominanceNode::Green;
	}
	if (dNodeScores[DominanceNode::Red] > dNodeScores[dominantNode]) {
		dominantNode = DominanceNode::Red;
	}

	string theColor = dominantNode == DominanceNode::Blue ? "Blue" : dominantNode == DominanceNode::Red ? "Red" : "Green";

	cout << "Dominant node is " << theColor << " with " << dNodeScores[dominantNode] << " limbs\n";

	if (headCount > 0) {
		/* One head means BRAIN DRAIN. */

		AttributeType attTypeHP = AttributeType::HP;
		AttributeType attTypeIntel = AttributeType::Intelligence;
		vector<AttributeType> attributeTypes = { attTypeHP, attTypeIntel };

		attackStructs.emplace_back(
			"Brain Drain",
			"BRAIN_DRAIN",
			55,
			45,
			dominantNode,
			AttackType::BrainDrain,
			attributeTypes
		);
	}

	if (torsoCount > 0) {
		/* Torso means BODY SLAM. More torsos means more slam. */

		AttributeType attTypeHP = AttributeType::HP;
		vector<AttributeType> attributeTypes = { attTypeHP };

		int intensity = torsoCount == 1 ? 70 : torsoCount == 2 ? 80 : torsoCount == 3 ? 89 : 95;
		int precision = 100 - intensity;

		attackStructs.emplace_back(
			"Body Slam",
			"BODY_SLAM",
			intensity,
			precision,
			dominantNode, 
			AttackType::BodySlam,
			attributeTypes
		);
	}

	if (wingCount > 1) {
		/* Wings mean SWOOP. More wings equals more swoop. */

		AttributeType attTypeHP = AttributeType::HP;
		vector<AttributeType> attributeTypes = { attTypeHP };

		attackStructs.emplace_back(
			"Swoop",
			"SWOOP",
			95,
			5,
			dominantNode, 
			AttackType::Swoop,
			attributeTypes
		);
	}

	if (armCount > 0 && characterType == CharacterType::Player) {

		AttributeType attTypeHP = AttributeType::HP;
		vector<AttributeType> attributeTypes = { attTypeHP };

		/* Only the Player can steal limbs. */
		if (characterType == CharacterType::Player) {
			attackStructs.emplace_back(
				"Steal",
				"STEAL",
				0,
				100,
				dominantNode,
				AttackType::Steal,
				attributeTypes
			);
		}		


		if (armCount == 1) {
			attTypeHP = AttributeType::HP;
			attributeTypes = { attTypeHP };

			attackStructs.emplace_back(
				"Punch",
				"PUNCH",
				30,
				70,
				dominantNode, 
				AttackType::Punch,
				attributeTypes
			);
		}
		else {
			attTypeHP = AttributeType::HP;
			vector<AttributeType> attributeTypes2 = { attTypeHP };

			attackStructs.emplace_back(
				"Double Punch",
				"DOUBLE_PUNCH",
				50,
				50,
				dominantNode, 
				AttackType::DoublePunch,
				attributeTypes2
			);
		}
	}

	if (legCount > 0) {
		AttributeType attTypeHP = AttributeType::HP;
		vector<AttributeType> attributeTypes = { attTypeHP };

		attackStructs.emplace_back(
			"Kick",
			"KICK",
			70,
			30,
			dominantNode, 
			AttackType::Kick,
			attributeTypes
		);
	}
	
	if (attackStructs.size() < 1) {
		/* Generic random attack. (randomize more by selecting a random attribute to attack). */

		AttributeType attTypeHP = AttributeType::HP;
		vector<AttributeType> attributeTypes = { attTypeHP };

		int intensity = (rand() % 80) + 10;
		int precision = 100 - intensity;

		attackStructs.emplace_back(
			"Fast Attack",
			"FAST_ATTACK",
			intensity,
			precision,
			dominantNode, 
			AttackType::Attack,
			attributeTypes
		);
	}

	if (characterType == CharacterType::Player) {
		vector<AttributeType> attributeTypes = { };
		attackStructs.emplace_back(
			"Run Away",
			"RUN_AWAY",
			100,
			0,
			dominantNode,
			AttackType::RunAway,
			attributeTypes
		);
	}
	

	return attackStructs;
}

/*
* This function draws the limbs to an offscreen texture as an avatar, which it returns.
* First we cycle through the equipped limbs and find out how high and wide the drawn avatar will be.
* We take into account the fact that rotating a texture will extend the boundaries.
* We use that information to build a texture, then draw onto it and return it.
*/
SDL_Texture* Character::createAvatar(bool resetRenderer) {
	UI& ui = UI::getInstance();
	Limb& anchorLimb = getAnchorLimb();

	int lWidth, lHeight;
	SDL_QueryTexture(anchorLimb.getTexture(), NULL, NULL, &lWidth, &lHeight);

	/* Set drawRects of all equipped limbs from starting point of 0 for calculating. */
	anchorLimb.setDrawRect({ 0, 0, lWidth, lHeight });
	setChildLimbDrawRects(anchorLimb, ui);

	/* AvatarDimensionsStruct will hold the information about the avatar dimensions as we
	* recursively cycle through the limbs and read the coordinates of their textures. */

	AvatarDimensionsStruct dimStruct = AvatarDimensionsStruct();
	checkChildLimbsForAvatarBoundaries(anchorLimb, dimStruct);

	/* The search is finished. Set the final dimensions based on the boundaries. */
	dimStruct.avatarWidth = dimStruct.rightmost - dimStruct.leftmost;
	dimStruct.avatarHeight = dimStruct.bottommost - dimStruct.topmost;

	dimStruct.greaterDimension = dimStruct.avatarHeight > dimStruct.avatarWidth ?
		dimStruct.avatarHeight : dimStruct.avatarWidth;

	/* Now build the actual avatar.
	* We will draw (render) the limb textures to an off-screen texture,
	* then return that texture as the avatar.
	* Some work is done to ensure transparency (setting the blend mode, using the right pixel format, clearing with 0 alpha).
	*/

	SDL_Renderer* renderer = ui.getMainRenderer();

	/* Create offscreen texture where we will draw the avatar. */
	SDL_Texture* offscreenTexture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_TARGET,
		dimStruct.greaterDimension, dimStruct.greaterDimension);

	if (offscreenTexture == NULL) {
		cout << "TEXTURE ERROR\n";
		SDL_Log("Failed to create offscreen texture: %s", SDL_GetError());
		return NULL;
	}

	/* Set the blend mode of the offscreen texture (to allow transparency). */
	SDL_SetTextureBlendMode(offscreenTexture, SDL_BLENDMODE_BLEND);

	/* Set the render target to the off-screen texture, and clear with transparent color. */
	SDL_SetRenderTarget(renderer, offscreenTexture);
	/* Enable blending for the renderer. */
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); /* 0 alpha means transparent. */
	SDL_RenderClear(renderer);

	/* Reset the drawRects (giving anchorLimb the correct offset) and draw limbs for the avatar    . */
	anchorLimb.setDrawRect({
		0 - dimStruct.leftmost,
		0 - dimStruct.topmost,
		lWidth,
		lHeight
		});

	/* Prepare points, rects, and draw order for drawing. */
	setRotationPointsSDL();
	setChildLimbDrawRects(anchorLimb, ui);
	buildDrawLimbList();

	for (int index : getDrawLimbIndexes()) {
		Limb& limbToDraw = getLimbs()[index];
		/* Ensure limb texture supports blending. */
		SDL_SetTextureBlendMode(limbToDraw.getTexture(), SDL_BLENDMODE_BLEND);
		limbToDraw.draw(ui);
	}

	SDL_RenderPresent(renderer);

	if (resetRenderer) {
		/* Reset the render target back to the default (the window). */
		SDL_SetRenderTarget(renderer, NULL);
	}	

	return offscreenTexture;
}

void Character::clearSuit() {
	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			unEquipLimb(limb.getId());
		}
		limb.unEquip();
	}
	anchorLimbId = -1;
	drawLimbListIDs = {};
	drawLimbListIndexes = {};
}

/*
* This sets the SDL_Points of each limb,
* which is the point around which the texture may be rotated.
* Each limb has a function to decide where this should be.
*/
void Character::setRotationPointsSDL() {
	for (Limb& limb : limbs) {
		limb.setRotationPointSDL();
	}
}

/* Each limb finds its anchor joint and stores the ID. */
void Character::setAnchorJointIDs() {
	for (Limb& limb : limbs) {
		limb.setAnchorJointId();
	}
}

Limb& Character::getLimbById(int limbId) {
	for (Limb& limb : limbs) {
		if (limb.getId() == limbId) {
			return limb;
		}
	}

	cout << "ERROR! LIMB " << limbId << " NOT FOUND!MUST REPLACE THIS WITH DEFAULT LIMB SOMEHOW!\n";
	cout << "Player is " << getName() << " id: " << getId() << endl;
	/* UNSAFE! DO NOT KEEP THIS! */
	return limbs[0];
}

void Character::unEquipLimb(int limbId) {
	if (!limbsContainId(limbId)) { return; }

	/* First remove reference from parent limb. */
	for (Limb& limb : limbs) {
		for (Joint& joint : limb.getJoints()) {
			if (joint.getConnectedLimbId() == limbId) {
				joint.detachLimb();
			}
		}
	}

	if (anchorLimbId == limbId) {
		anchorLimbId = -1;
	}

	/* recursively unequip limbs and child limbs. */
	Limb& baseLimb = getLimbById(limbId);
	baseLimb.setAnchor(false);
	for (int i = 0; i < baseLimb.getJoints().size(); ++i) {
		Joint& joint = baseLimb.getJoints()[i];
		joint.resetModifiedPoint();
		int connectedLimbId = joint.getConnectedLimbId();
		if (connectedLimbId >= 0) {
			unEquipLimb(connectedLimbId);
		}
	}

	baseLimb.unEquip();
	buildDrawLimbList();
}


int Character::getNumberOfEquippableLimbs() {
	int numberOfEquippableLimbs = 0;

	for (Limb& limb : limbs) {
		if (limb.getHP() > 0) {
			++numberOfEquippableLimbs;
		}
	}

	return numberOfEquippableLimbs;
}


int Character::getParentLimbId(int childLimbId) {
	for (int i = 0; i < limbs.size(); ++i) {
		vector<Joint> theseJoints = limbs[i].getJoints();

		for (int k = 0; k < theseJoints.size(); ++k) {
			if (theseJoints[k].getConnectedLimbId() == childLimbId) {
				return limbs[i].getId();
			}
		}
	}

	return -1;
}


/*
* returns vector of: tuple<limbId, jointId, isFree>
*/
vector<tuple<int, int, bool>> Character::getEquippedJointsData(int limbToSkipId = -1) {
	vector<tuple<int, int, bool>> jointsData;

	for (int i = 0; i < limbs.size(); ++i) {
		Limb& thisLimb = limbs[i];
		int thisLimbId = thisLimb.getId();

		if (thisLimbId == limbToSkipId) { continue; }
		if (!thisLimb.isEquipped()) { continue; }

		vector<Joint>& theseJoints = thisLimb.getJoints();

		for (int k = 0; k < theseJoints.size(); ++k) {
			Joint& thisJoint = theseJoints[k];
			tuple<int, int, bool> thisTuple = make_tuple(thisLimbId, k, thisJoint.isFree());
			jointsData.push_back(thisTuple);
		}
	}

	return jointsData;
}

void Character::getChildLimbsRecursively(Limb& parentLimb, vector<Limb>& childLimbs) {
	vector<Joint>& parentJoints = parentLimb.getJoints();

	for (Joint& pJoint : parentJoints) {
		if (!pJoint.isFree() && !pJoint.getIsAnchor()) {
			Limb& childLimb = getLimbById(pJoint.getConnectedLimbId());
			childLimbs.push_back(childLimb);
			getChildLimbsRecursively(childLimb, childLimbs);
		}
	}
}

/* Returns COPIES of the equipped limbs. */
vector<Limb> Character::getEquippedLimbs() {
	vector<Limb> equippedLimbs;

	for (Limb& limb : limbs) {
		if (limb.isEquipped()) {
			equippedLimbs.push_back(limb);
		}
	}

	return equippedLimbs;
}

/*
* Move the loaded limb to the next available joint in the character's equipped limbs.
* Also, cycle back to the beginning of the list once we reach the end.
*/
bool Character::shiftChildLimb(int childLimbId) {

	if (getAnchorLimbId() == childLimbId) {
		/* Don't do it if this is the anchor limb (no parent limb with limbs through which to cycle). */
		return false;
	}
	else {

		/* Try to get the parent limb ID and the parent limb itself. */
		int parentLimbId = getParentLimbId(childLimbId);
		if (parentLimbId < 0) { return false; }
		Limb& parentLimb = getLimbById(parentLimbId);

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
				Limb& childLimb = getLimbById(childLimbId);
				childLimb.setAnchorJointId();
				int loadedLimbAnchorJointId = childLimb.getAnchorJointId();
				newParentJoint.connectLimb(childLimbId, loadedLimbAnchorJointId);
				/* Reset all joints. */
				setAnchorJointIDs();
				setChildLimbDrawRects(getAnchorLimb(), UI::getInstance());
				return true;
			}
		}
	}

	return false;
}

/* 
* Search the given limb (id) for an available joint.
* If none are free, recursively search the connected limbs for free joints.
* 
* Returns a tuple.
* FIRST int is the limb id.
* SECOND int is its free joint index. (will I have to replace joint indexes with DB IDs too?).
*/
tuple<int, int> Character::getLimbIdAndJointIndexForConnection(int limbIdToSearch, int limbIdToExclude) {
	tuple<int, int> failureTuple = make_tuple(-1, -1);
	if (limbIdToSearch < 1) {
		return failureTuple;
	}
	
	Limb& limbToSearch = getLimbById(limbIdToSearch);
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

			Limb& nestedLimbToSearch = getLimbById(connectedLimbId);
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
	UI& ui = UI::getInstance();
	Limb& limbToEquip = getLimbById(limbId);
	if (limbToEquip.isEquipped()) {
		cout << "Limb is already equipped!\n";
		return false;
	}

	/* When there are no limbs equipped, make this limb the FIRST (anchor) limb. */
	if (anchorLimbId < 0) {
		anchorLimbId = limbId;
		limbToEquip.setAnchor(true);
		setChildLimbDrawRects(getAnchorLimb(), ui);
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

	Limb& limbForConnection = getLimbById(limbIdForConnection);
	Joint& jointForConnection = limbForConnection.getJoints()[jointIndexForConnection];

	/* Now connect the actual limb (find a free joint on the limb we're trying to connect). */
	for (int i = 0; i < limbToEquip.getJoints().size(); ++i) {
		Joint& jointToEquip = limbToEquip.getJoints()[i];

		if (jointToEquip.isFree()) {
			jointToEquip.setAnchor(true);
			jointForConnection.connectLimb(limbId, i);
			/* Don't let two identical limbs overlap completely (if: same limb, same angle, same joint). */
			if (limbForConnection.getForm().slug == limbToEquip.getForm().slug &&
				limbForConnection.getRotationAngle() == limbToEquip.getRotationAngle() &&
				jointIndexForConnection == i)
			{
				limbToEquip.rotate(25);
			}
			setChildLimbDrawRects(getAnchorLimb(), ui);
			return true;
		}
	}
	/* Failed to find a free joint on the limb we've been trying to equip (unlikely... should be impossible). */
	return false;
}

/*
* Set the SDL_Rect for child limbds, recursively.
* The SDL_Rect for the anchor limb must be set by the screen FIRST. Then call this function.
* Scale will be baked into the parentRect and the modifiedPoint of each joint.
*/
void Character::setChildLimbDrawRects(Limb& parentLimb, UI& ui) {
	/* Get the "parent" limb's SDL_Rect (for reference) and joints (to access each connected limb). */
	vector<Joint>& parentJoints = parentLimb.getJoints();
	SDL_Rect& parentRect = parentLimb.getDrawRect();

	/* For each joint, get the connected limb and set its rect. Then recursively set ITS child limbs' SDL_Rects. */
	for (int i = 0; i < parentJoints.size(); ++i) {
		Joint& parentJoint = parentJoints[i];
		int connectedLimbId = parentJoint.getConnectedLimbId();
		if (connectedLimbId < 0) { continue; }

		Point parentJointPoint = parentJoint.getPoint();
		Limb& connectedLimb = getLimbById(parentJoint.getConnectedLimbId());

		/* make sure it has an anchor joint (make a function which checks???)... if not, return and stop drawing. */
		Point connectedLimbAnchorJointPoint = connectedLimb.getAnchorJoint().getPoint();

		/* First offset by parent limb location,
		* then by the JOINT to which we are connected.
		* THEN offset by THIS limb's anchor joint
		*/

		connectedLimb.setDrawRect({
			parentRect.x + parentJointPoint.x - connectedLimbAnchorJointPoint.x,
			parentRect.y + parentJointPoint.y - connectedLimbAnchorJointPoint.y,
			parentRect.w,
			parentRect.h
			});

		/* Recursively draw all THIS limb's child limbs. */
		setChildLimbDrawRects(connectedLimb, ui);
	}
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

/* When sorting a vector of Limb objects by drawOrder, we use this function as the third parameter for comparison.
* Ascending.
*/
export bool compareDrawOrder(Limb& limbA, Limb& limbB) {
	return limbA.getDrawOrder() < limbB.getDrawOrder();
}

/* When sorting a vector of Limb objects by number of joints, we use this function as the third parameter for comparison.
* Descending: We want the MOST joints to be first.
*/
export bool compareJointsNumber(Limb& limbA, Limb& limbB) {
	return limbA.getJoints().size() > limbB.getJoints().size();
}