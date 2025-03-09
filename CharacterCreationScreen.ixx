/*
* 
*       _____ _                          _
*      / ____| |                        | |
*     | |    | |__   __ _ _ __ __ _  ___| |_ ___ _ __
*     | |    | '_ \ / _` | '__/ _` |/ __| __/ _ \ '__|
*     | |____| | | | (_| | | | (_| | (__| ||  __/ |
*      \_____|_| |_|\__,_|_|  \__,_|\___|\__\___|_|
*       _____                _   _
*      / ____|              | | (_)
*     | |     _ __ ___  __ _| |_ _  ___  _ __
*     | |    | '__/ _ \/ _` | __| |/ _ \| '_ \
*     | |____| | |  __/ (_| | |_| | (_) | | | |
*      \_____|_|  \___|\__,_|\__|_|\___/|_| |_|
*      / ____|
*     | (___   ___ _ __ ___  ___ _ __
*      \___ \ / __| '__/ _ \/ _ \ '_ \
*      ____) | (__| | |  __/  __/ | | |
*     |_____/ \___|_|  \___|\___|_| |_|
*
* 
*	Snap together limbs from the Player Character's inventory.
* Limbs connect hierarchically: The first limb is the character's anchor limb. Other limbs branch off from there.
* Each subsequent limb has an anchor joint: the joint which is connected to a joint of its parent limb.
* So the anchor limb (first limb) has no anchor joint, and all its joints are free.
* The character object has an "anchorLimbId" int which stores the id of the anchor limb (id from the database).
* 
* Each joint object has an "isAnchor" boolean.
* When a limb is equipped to the character, its "anchor" joint's "isAnchor" boolean is set to true.
* Its parent limb's joint takes data from the child limb and the child limb's anchor joint.
* This way, we can start with the anchor limb and find all its child limbs, and all their child limbs.
* So we don't need to store a suit. All the info of the equipped character is in the limbs and their joints.
* But it starts with the anchorLimbId int in the character object.
* 
* A limb can rotate on its anchor joint. The limb object contains its angle of rotation.
* The joint objects contain a point object called "modifiedPoint" which has the NEW location of each joint after rotation.
* Rotating a limb means we need to use trigonometry to rotate the location of each joint point.
* 
* TO DO:
* 1) Review Mode should include a 2nd panel of equipped limbs buttons, so we can LOAD limbs which are already equipped.
* ---> This allows us to "unEquip" a limb AND all its child limbs (recursively) without clearing the whole character.
* 2) Make drawOrder separate from branching hierarchy.
* 3) Draw from player inventory instead of hard-coding all the limbs
* 4) Load already-built character for editing.
* 5) Pagination once we have many limbs on the map and in the inventory.
* 
* 
* PROBLEMS: 
* 2) Vector out of bounds error when trying to connect to deer head (SOMETIMES).
* 
*/

module;

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

export module CharacterCreationScreen;
import <string>;
import <iostream>;
import <vector>;
import <unordered_map>;
import <algorithm>;

import CharacterClasses;
import FormFactory;
import TypeStorage;
import GameState;
import Resources;
import UI;
import Database;

using namespace std;

const bool DRAW_JOINTS = false;

enum class CreationMode {
	Review, /* DEFAULT. Show default panel. */
	ChooseLimb, /* Show list of limbs. */
	LimbLoaded /* Limb is loaded. Show LoadedLimb Panel. */
};


/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class CharacterCreationScreen {
public:
	/* constructor */
	CharacterCreationScreen() {
		cout << "\nLoading Character Creation Screen\n\n";
		screenType = ScreenType::CharacterCreation;
		id = 0;
		screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);
		UI& ui = UI::getInstance();
		getBackgroundTexture(ui);
		createTitleTexture(ui);

		playerCharacter = loadPlayerCharacter();

		limbLoaded = false;
		loadedLimbId = -1;
		showTitle = true;
		titleCountdown = 140;
		
		/* create panels */
		reviewModePanel = ui.createReviewModePanel();
		createLimbLoadedPanel(ui);
		createChooseLimbPanel(false);

		changeCreationMode(CreationMode::Review);
		messagePanel = ui.createConfirmationPanel("", ConfirmationButtonType::OkCancel, false);
		messagePanel.setShow(false);

		cout << playerCharacter.getLimbs().size() << " LIMBS\n";
		drawLoadedLimb = true;
		loadedLimbCountdown = 20;
		playerCharacter.setAnchorJointIDs();
		playerCharacter.setRotationPointsSDL();
		loadedLimbIsAlreadyEquipped = false;

		playerStatsPanel = ui.createHud(ScreenType::Battle, playerCharacter.getCharStatsData(), true);
		playerStatsPanel.setShow(true);

	}

	/* Destructor */
	~CharacterCreationScreen() {
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyTexture(titleTexture);
	}

	void createLimbLoadedPanel(UI& ui);
	void createChooseLimbPanel(bool showEquippedLimbs, int page = 1);
	void setAnchorLimbDrawRect(UI& ui);
	void changeCreationMode(CreationMode creationMode);
	ScreenType getScreenType() { return screenType; }
	void run();

private:
	ScreenType screenType;
	int id;
	ScreenStruct screenToLoadStruct;

	SDL_Texture* bgTexture = NULL;
	SDL_Rect bgSourceRect;
	SDL_Rect bgDestinationRect;

	SDL_Texture* titleTexture;
	SDL_Rect titleRect;

	Panel reviewModePanel;
	Panel limbLoadedPanel;
	Panel chooseLimbPanel;
	Panel messagePanel;
	Panel playerStatsPanel;

	void drawCharacter(UI& ui);

	bool limbLoaded;
	int loadedLimbId; /* Currently holds index from vector. Will hold id from DB. */

	void draw(UI& ui);
	void handleEvent(SDL_Event& e, bool& running, GameState& gameState);
	void checkMouseLocation(SDL_Event& e);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay();
	void createTitleTexture(UI& ui);
	void raiseTitleRect() { --titleRect.y; }
	int getTitleBottomPosition() { return titleRect.y + titleRect.h; }

	void showNewMessage(string newMessage, ConfirmationButtonType confirmationType, bool showRefuse, UI& ui = UI::getInstance());
	void loadLimbAttempt(int limbToLoadID);

	bool showTitle;
	bool drawLoadedLimb;
	int titleCountdown;
	int loadedLimbCountdown;
	bool loadedLimbIsAlreadyEquipped; /* When we are RE-LOADING an already-equipped limb, so UNLOADING means going back to REVIEW menu. */

	Character playerCharacter;
	CreationMode creationMode;
};



/* When we load the limb we need a panel with certain buttons to manipulate the loaded limb.
* Some buttons might not be necessary, so check if we have enough joints to move the limb around.
*/
void CharacterCreationScreen::createLimbLoadedPanel(UI& ui = UI::getInstance()) {
	/* Destroy textures if they already exist. */
	if (limbLoadedPanel.getButtons().size() > 0) {
		limbLoadedPanel.destroyTextures(); }

	bool loadedLimbHasExtraJoints = false;
	bool characterHasExtraJoints = false;

	if (loadedLimbId > 0) {
		Limb& loadedLimb = playerCharacter.getLimbById(loadedLimbId);
		int anchorLimbId = playerCharacter.getAnchorLimbId();
		if (loadedLimb.getFreeJointIndexes().size() > 1 && loadedLimbId != anchorLimbId) {
			loadedLimbHasExtraJoints = true;
		}

		if (anchorLimbId > 0 && loadedLimbId != playerCharacter.getAnchorLimbId()) {
			tuple<int, int> limbIdAndJointIndexForConnection = playerCharacter.getLimbIdAndJointIndexForConnection(anchorLimbId, loadedLimbId);
			int possibleConnectorLimbId = get<0>(limbIdAndJointIndexForConnection);
			int possibleConnectorJointId = get<1>(limbIdAndJointIndexForConnection);

			if (possibleConnectorLimbId > 0 && possibleConnectorJointId >= 0 && possibleConnectorLimbId != loadedLimbId) {
				characterHasExtraJoints = true;
			}
		}
	}
	limbLoadedPanel = ui.createLimbLoadedModePanel(loadedLimbHasExtraJoints, characterHasExtraJoints);
}



/* 
* Create the panel which displays a list of buttons, one button for each limb.
* Depending on the context, we will either load the NON-EQUIPPED limbs, or the EQUIPPED limbs.
* In Review Mode we load the equipped limbs (so we can manipulate them).
* In ChooseLimb mode we load the non-equipped limbs.
* Panel must be rebuilt every time we load/equip or unequip a limb.
*/
void CharacterCreationScreen::createChooseLimbPanel(bool showEquippedLimbs, int page) {
	UI& ui = UI::getInstance();
	/* Destroy textures if they already exist. */
	if (chooseLimbPanel.getButtons().size() > 0) {
		chooseLimbPanel.destroyTextures();
	}

	/* Build a vector of data structures so the UI can build the panel of Limb buttons. */
	vector<LimbButtonData> limbBtnDataStructs;
	vector<Limb>& inventoryLimbs = playerCharacter.getLimbs();

	for (int i = 0; i < inventoryLimbs.size(); ++i) {
		Limb& thisLimb = inventoryLimbs[i];
		if (thisLimb.isEquipped() == showEquippedLimbs && !(!showEquippedLimbs && thisLimb.getHP() < 1)) {
			LimbButtonData newStruct = thisLimb.getLimbButtonData();
			limbBtnDataStructs.emplace_back(newStruct);
		}
	}

	string label = showEquippedLimbs ? "EQUIPPED LIMBS" : "NON-EQUIPPED LIMBS (with HP)";

	chooseLimbPanel = ui.createChooseLimbModePanel(limbBtnDataStructs, true, label, page);
}


/*
* CreationMode indicates whether a list of limbs should be shown,
* or a panel to manipulate a loaded limb,
* or the main panel for reviewing the current character.
* Changing the mode changes the panels.
*/
void CharacterCreationScreen::changeCreationMode(CreationMode creationMode) {
	this->creationMode = creationMode;

	switch (creationMode) {
	case CreationMode::ChooseLimb:
		createChooseLimbPanel(false);
		chooseLimbPanel.setShow(true);
		reviewModePanel.setShow(false);
		limbLoadedPanel.setShow(false);
		break;
	case CreationMode::Review:
		createChooseLimbPanel(true);
		chooseLimbPanel.setShow(true);
		reviewModePanel.setShow(true);
		limbLoadedPanel.setShow(false);
		break;
	case CreationMode::LimbLoaded:
		chooseLimbPanel.setShow(false);
		reviewModePanel.setShow(false);
		limbLoadedPanel.setShow(true);
		break;
	}
}

void CharacterCreationScreen::getBackgroundTexture(UI& ui) {
	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };
	bgTexture = getSkyBackgroundTexture();
}

export void CharacterCreationScreen::run() {
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();
	reviewModePanel.setShow(true);

	/* Timeout data */
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; /* milliseconds per frame. */
	Uint32 frameStartTime; /* Tick count when this particular frame began. */
	int frameTimeElapsed; /* how much time has elapsed during this frame. */

	/* loop and event control */
	SDL_Event e;
	bool running = true;
	setAnchorLimbDrawRect(ui);

	if (playerCharacter.getAnchorLimbId() > 0) {
		playerCharacter.setChildLimbDrawRects(playerCharacter.getAnchorLimb(), ui);
	}
	
	playerCharacter.buildDrawLimbList();

	while (running) {
		/* Get the total running time(tick count) at the beginning of the frame, for the frame timeout at the end */
		frameStartTime = SDL_GetTicks();

		/* Check for events in queue, and handle them(really just checking for X close now */
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				handleEvent(e, running, gameState); }
		}

		/* Deal with blinking loaded limb */
		if (loadedLimbId >= 0) {
			--loadedLimbCountdown;
			if (loadedLimbCountdown < 1) {
				drawLoadedLimb = !drawLoadedLimb;
				loadedLimbCountdown = 20;
			}
		}

		/* Deal with showing the title. */
		if (showTitle) {
			if (titleCountdown > 0) {
				--titleCountdown; }
			else {
				raiseTitleRect(); }

			if (getTitleBottomPosition() < -1) {
				showTitle = false; }
		}

		checkMouseLocation(e);
		draw(ui);

		/* Delay so the app doesn't just crash */
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
		/* Delay loop */
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed);
		}
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}


void CharacterCreationScreen::draw(UI& ui) {
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* draw BG for now */
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);

	if (showTitle) {
		/* draw the title */
		SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);
	}	

	limbLoadedPanel.draw(ui);
	reviewModePanel.draw(ui);
	drawCharacter(ui);
	playerStatsPanel.draw();
	chooseLimbPanel.draw(ui);
	messagePanel.draw(ui);

	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}


/* Create the texture with the name of the game */
void CharacterCreationScreen::createTitleTexture(UI& ui) {
	Resources& resources = Resources::getInstance();
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture("Character Creation!");
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}


/* Screen has been resized. Rebuild! */
void CharacterCreationScreen::rebuildDisplay() {
	UI& ui = UI::getInstance();
	/* These panels don't exist in this module anymore, but I'm keeping these function calls as a blueprint for rebuilding displays. */
	//ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	//ui.rebuildGameMenuPanel(gameMenuPanel);
	getBackgroundTexture(ui);
	createTitleTexture(ui);
}


void printAllLimbs(Character character) {
	for (Limb& limb : character.getLimbs()) {
		string isAnchorString = limb.getIsAnchor() ? " IS anchor " : " is NOT anchor";
		cout << limb.getName() << isAnchorString << "\n";
	}
}

void CharacterCreationScreen::showNewMessage(string newMessage, ConfirmationButtonType confirmationType, bool showRefuse, UI& ui) {
	messagePanel = getNewConfirmationMessage(messagePanel, newMessage, confirmationType, showRefuse);
	messagePanel.setShow(true);
}

void CharacterCreationScreen::setAnchorLimbDrawRect(UI& ui = UI::getInstance()) {
	if (playerCharacter.getAnchorLimbId() < 0) { return; }
	int screenWidth = ui.getWindowWidth();
	int screenHeight = ui.getWindowHeight();
	Limb& anchorLimb = playerCharacter.getAnchorLimb();

	/* FOR NOW we will use the natural size of the textures (for different sized screens we might need to adjust this). */
	int limbWidth, limbHeight;
	SDL_QueryTexture(anchorLimb.getTexture(), NULL, NULL, &limbWidth, &limbHeight);

	anchorLimb.setDrawRect({
		(screenWidth / 2) - (limbWidth / 2),
		(screenHeight / 2) - (limbHeight / 2),
		limbWidth,
		limbHeight
		});
}


/*
* When the user clicks on a limb to load it (whether the limb is already loading or freshly equipping it)
* We call this function.
*/
void CharacterCreationScreen::loadLimbAttempt(int limbToLoadID) {
	/* clickStruct.buttonOption stores a limb ID from the database. The ID of the limb to load.*/

	UI& ui = UI::getInstance();
	Limb& clickedLimb = playerCharacter.getLimbById(limbToLoadID);
	bool limbEquipped = false;

	/* Make sure there is somewhere to attach the limb
	* (only if it NEEDS a new attachment because it's NOT already equipped...
	* ...we allow loading an already-equipped limb, but that case does NOT require this check).
	*/
	if (!clickedLimb.isEquipped() &&
		playerCharacter.getAnchorLimbId() > 0 &&
		get<0>(playerCharacter.getLimbIdAndJointIndexForConnection(playerCharacter.getAnchorLimbId(), limbToLoadID)) < 0
	) {
		showNewMessage(
			Resources::getInstance().getMessageText("NO_FREE_CHAR_JOINTS"),
			ConfirmationButtonType::OkCancel, false
		);
		return;
	}

	/*
	* To set the draw order of the limbs, we set the draw order of the clicked limb to the SIZE of the vector of limbs.
	* Then at the end we rebuild the draLimbList which brings everything down to the floor (starts at 0 and increments).
	* 
	*/

	clickedLimb.setDrawOrder(playerCharacter.getEquippedLimbs().size());
	loadedLimbId = limbToLoadID;

	/* Now actually equip the limb if it isn't already equipped. */
	if (!clickedLimb.isEquipped()) {
		limbEquipped = playerCharacter.equipLimb(limbToLoadID);
		if (limbEquipped) {
			playerStatsPanel.destroyTextures();
			playerStatsPanel = ui.createHud(ScreenType::Battle, playerCharacter.getCharStatsData(), true);
			playerStatsPanel.setShow(true);

		}
		else {
			loadedLimbId = -1;
			clickedLimb.setDrawOrder(-1);

			cerr << "LIMB NOT EQUIPPED\n";
			return;
		}
	}

	if (loadedLimbId == playerCharacter.getAnchorLimbId()) {
		setAnchorLimbDrawRect(ui);
	}

	loadedLimbIsAlreadyEquipped = creationMode == CreationMode::Review;
	createLimbLoadedPanel();
	changeCreationMode(CreationMode::LimbLoaded);
	clickedLimb.setRotationPointSDL();
	playerCharacter.buildDrawLimbList();
}

/* Process user input */
void CharacterCreationScreen::handleEvent(SDL_Event& e, bool& running, GameState& gameState) {
	/* User pressed X to close */
	if (e.type == SDL_QUIT) {
		cout << "\nQUIT\n";
		running = false;
		return;
	}
	else {
		/* user clicked */
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			cout << "user clicked mouse\n";
			/* These events might change the value of screenToLoad. */
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (messagePanel.getShow() && messagePanel.isInPanel(mouseX, mouseY)) {
				/* panel has a function to return which ButtonOption was clicked, and an ID(in the ButtonClickStruct). */
				ButtonClickStruct clickStruct = messagePanel.checkButtonClick(mouseX, mouseY);

				switch (clickStruct.buttonOption) {
				case ButtonOption::Agree:
					messagePanel.setShow(false);
					break;
				case ButtonOption::Refuse:
					messagePanel.setShow(false);
					break;
				}
			} else if (reviewModePanel.getShow() && reviewModePanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICKED REVIEW MENU \n\n";
				ButtonClickStruct clickStruct = reviewModePanel.checkButtonClick(mouseX, mouseY);
				string suitSavedString = "Suit Saved.";

				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::MapOptions:
					cout << "\nMAP OPTIONS\n";
					break;
				case ButtonOption::ShowLimbs:
					changeCreationMode(CreationMode::ChooseLimb);
					break;
				case ButtonOption::ClearSuit:
					cout << "CLEARING SUIT\n";
					playerCharacter.clearSuit();
					playerStatsPanel.destroyTextures();
					playerStatsPanel = ui.createHud(ScreenType::Battle, playerCharacter.getCharStatsData(), true);
					playerStatsPanel.setShow(true);
					changeCreationMode(CreationMode::Review);
					break;
				case ButtonOption::SaveSuit:
					updateCharacterLimbs(gameState.getPlayerID(), playerCharacter.getAnchorLimbId(), playerCharacter.getLimbs());
					cout << suitSavedString << endl;
					messagePanel = getNewConfirmationMessage(messagePanel, suitSavedString, ConfirmationButtonType::OkCancel, false);
					messagePanel.setShow(true);
					break;
				case ButtonOption::Continue:

					/* 
					* 1. FIND OUT if player is ready to go (suit is saved, suit has limbs equipped).
					* 2. FIND OUT where player is supposed to go (Map or Battle).
					*/
					if (true) {
						int battleId = getCurrentBattleId(gameState.getPlayerID());
						cout << "Battle id: " << battleId << endl;

						if (battleId < 1) {
							screenToLoadStruct.screenType = ScreenType::Map;
						}
						else {
							screenToLoadStruct.screenType = ScreenType::Battle;
							screenToLoadStruct.id = battleId;
						}
					}					

					running = false;
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else if (chooseLimbPanel.getShow() && chooseLimbPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK LIMB MENU \n\n";
				ButtonClickStruct clickStruct = chooseLimbPanel.checkButtonClick(mouseX, mouseY);
				int limbToLoadID = clickStruct.itemID;

				/* If we sent in a limb id/index: */
				if (clickStruct.buttonOption == ButtonOption::LoadLimb && limbToLoadID > 0) {
					loadLimbAttempt(limbToLoadID);
				}
				else if(clickStruct.buttonOption == ButtonOption::Back) {
					if (creationMode != CreationMode::Review) {
						changeCreationMode(CreationMode::Review);
					}
					else {
						chooseLimbPanel.setShow(false);
					}
					
				}
				else if (clickStruct.buttonOption == ButtonOption::NextPage) {
					bool showEquippedLimbs = creationMode == CreationMode::Review;
					createChooseLimbPanel(showEquippedLimbs, clickStruct.itemID);
					chooseLimbPanel.setShow(true);
				}
			}
			else if (limbLoadedPanel.getShow() && limbLoadedPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK LOADED LIMB MENU \n\n";
				ButtonClickStruct clickStruct = limbLoadedPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();

				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::Equip:
					loadedLimbId = -1;
					changeCreationMode(CreationMode::Review);
					loadedLimbIsAlreadyEquipped = false;
					break;
				case ButtonOption::NextCharJoint:
					/* 
					* Move the loaded limb to the next available joint in the character's equipped limbs.
					* Also, cycle back to the beginning of the list once we reach the end.
					* Maybe put this in a function in the character class.
					*/

					if (loadedLimbId > 0) {
						bool switched = playerCharacter.shiftChildLimb(loadedLimbId);
						/* Reset all joints. */
						playerCharacter.setAnchorJointIDs();
						playerCharacter.setRotationPointsSDL();
						playerCharacter.setChildLimbDrawRects(playerCharacter.getAnchorLimb(), ui);
					}

					break;
				case ButtonOption::NextLimbJoint:
					/* Change the anchor joint of the loaded limb. */
					if (playerCharacter.getAnchorLimbId() == loadedLimbId) {
						break;
					}
					else {
						Limb& loadedLimb = playerCharacter.getLimbById(loadedLimbId);
						cout << "LIMB: " << loadedLimb.getForm().slug << "\n";
						bool anchorShifted = loadedLimb.shiftAnchorLimb();
						if (anchorShifted) {
							playerCharacter.setAnchorJointIDs();
							playerCharacter.setRotationPointsSDL();
							playerCharacter.setChildLimbDrawRects(playerCharacter.getAnchorLimb(), ui);
						}
						break;
					}

				case ButtonOption::RotateClockwise:
					if (loadedLimbId > 0) {
						Limb& loadedLimb = playerCharacter.getLimbById(loadedLimbId);
						loadedLimb.rotate(15);
						/* Make all the child limbs follow the rotation. */
						playerCharacter.setChildLimbDrawRects(loadedLimb, ui);
					}
					break;
				case ButtonOption::RotateCounterClockwise:
					if (loadedLimbId > 0) {
						Limb& loadedLimb = playerCharacter.getLimbById(loadedLimbId);
						loadedLimb.rotate(-15);
						/* Make all the child limbs follow the rotation. */
						playerCharacter.setChildLimbDrawRects(loadedLimb, ui);
					}
					break;
				case ButtonOption::UnloadLimb:
					/*
					* Unequip that limb.
					* Go back to the ChooseLimb mode.
					*/
					if (loadedLimbId >= 0) {
						Limb& loadedLimb = playerCharacter.getLimbById(loadedLimbId);
						if (loadedLimbId == playerCharacter.getAnchorLimbId()) {
							playerCharacter.setAnchorLimbId(-1); }
						playerCharacter.unEquipLimb(loadedLimbId);

						playerStatsPanel.destroyTextures();
						playerStatsPanel = ui.createHud(ScreenType::Battle, playerCharacter.getCharStatsData(), true);
						playerStatsPanel.setShow(true);
					}

					if (loadedLimbIsAlreadyEquipped) {
						changeCreationMode(CreationMode::Review);
						loadedLimbIsAlreadyEquipped = false; }
					else {
						changeCreationMode(CreationMode::ChooseLimb); }
					
					loadedLimbId = -1;
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else {
				/* Clicked outside all panels. */

				if (creationMode == CreationMode::Review && !chooseLimbPanel.getShow()) {
					chooseLimbPanel.setShow(true);
				}
			}
		}
	}
}

void CharacterCreationScreen::checkMouseLocation(SDL_Event& e) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (!messagePanel.getShow()) {
		if (reviewModePanel.getShow()) { reviewModePanel.checkMouseOver(mouseX, mouseY); }
		if (limbLoadedPanel.getShow()) { limbLoadedPanel.checkMouseOver(mouseX, mouseY); }
		if (chooseLimbPanel.getShow()) { chooseLimbPanel.checkMouseOver(mouseX, mouseY); }
	}
	else {
		messagePanel.checkMouseOver(mouseX, mouseY);
	}
}



/* FOR DEBUGGING: Draw a box over each Joint.*/
void drawJoints(Limb& limb, UI& ui) {
		SDL_Color jointColor;
		jointColor.r = 255;
		jointColor.g = 255;
		jointColor.b = 51;
		jointColor.a = 255; /* Alpha (fully opaque). */

		for (Joint& joint : limb.getJoints()) {
			Point point = joint.getPoint();

			SDL_Rect pointRect = {
				limb.getDrawRect().x + point.x - 5,
				limb.getDrawRect().y + point.y - 5,
				10,
				10
			};

			SDL_Surface* pointSurface = createTransparentSurface(10, 10);
			SDL_FillRect(pointSurface, NULL, convertSDL_ColorToUint32(pointSurface->format, jointColor));
			SDL_Texture* pointTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), pointSurface);
			SDL_FreeSurface(pointSurface);

			SDL_RenderCopyEx(
				ui.getMainRenderer(),
				pointTexture,
				NULL, &pointRect,
				NULL, NULL, SDL_FLIP_NONE
			);

			SDL_DestroyTexture(pointTexture);
		}
}



/*
* Draw each limb.
* 
* For now we will start with the anchor limb,
* then cycle through each of its joints and draw their connected limbs,
* then cycle through their joints, etc, recursively.
* 
* THEN, the above sequence will just be to create the rects.
* We need to add drawOrder as an int member of Limb
* and draw based on the order instead of the connection hierarchy.
*/
void CharacterCreationScreen::drawCharacter(UI& ui) {
	int anchorLimbId = playerCharacter.getAnchorLimbId();
	if (anchorLimbId < 0) { return; }

	/*
	* 4. Make LoadedLimb blink.
	* 5. Allow user to change which LOADED LIMB JOINT is the ANCHOR.
	* 6. Allow user to change which EQUIPPED LIMB JOINT it is anchored TO.
	* 7. Draw them at their ANGLES.
	* 8. Allow user to CHANGE the angle of the ORIGINAL (we're using a reference anyway for this reason).
	* 9. SAVE this information to the DATABASE.
	*		---> We won't need a "suit." AnchorLimbId will be in the character, and the other attributes are in the Limb and Joint objects.
	* 10. EVENTUALLY allow user to FLIP limbs horizontally or vertically.
	*/

	vector<Limb>& limbs = playerCharacter.getLimbs();
	Limb& anchorLimb = playerCharacter.getAnchorLimb();

	/* 1.Make a rectangle of the whole screen. (will I use this?? NOPE... unless I draw something on it?? YES. some backdrop... off-white.  */
	int screenWidth = ui.getWindowWidth();
	int screenHeight = ui.getWindowHeight();
	SDL_Rect screenRect = { 0, 0, screenWidth, screenHeight };

	/* 
	* Drawing based on vector index instead of searching for limb with ID each time.
	*/
	for (int index : playerCharacter.getDrawLimbIndexes()) {
		Limb& limbToDraw = playerCharacter.getLimbs()[index];

		/* Skip "loaded" limb when drawLoadedLimb is false (for hot blinking action). */
		if (limbToDraw.getId() != loadedLimbId || drawLoadedLimb) {
			limbToDraw.draw(ui);
		}
	}
}