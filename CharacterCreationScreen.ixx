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
		settingsPanel = ui.createSettingsPanel(ScreenType::Map);
		gameMenuPanel = ui.createGameMenuPanel();
		reviewModePanel = ui.createReviewModePanel();
		createLimbLoadedPanel(ui);
		createChooseLimbPanel(ui);
		creationMode = CreationMode::Review;
		messagePanel = ui.createConfirmationPanel("", ConfirmationButtonType::OkCancel, false);
		messagePanel.setShow(true);

		cout << playerCharacter.getLimbs().size() << " LIMBS\n";
		drawLoadedLimb = true;
		loadedLimbCountdown = 20;
	}

	/* Destructor */
	~CharacterCreationScreen() {
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyTexture(titleTexture);
	}

	/* When we load the limb we need a panel with certain buttons to manipulate the loaded limb.
	* Some buttons might not be necessary, so check if we have enough joints to move the limb around.
	*/
	void createLimbLoadedPanel(UI& ui = UI::getInstance()) {
		/* Destroy textures if they already exist. */
		if (limbLoadedPanel.getButtons().size() > 0) {
			limbLoadedPanel.destroyTextures();
		}

		bool loadedLimbHasExtraJoints = false;
		bool characterHasExtraJoints = false;

		if (loadedLimbId > -1) {
			Limb& loadedLimb = playerCharacter.getLimbs()[loadedLimbId];
			int anchorLimbId = playerCharacter.getAnchorLimbId();
			if (loadedLimb.getFreeJointIndexes().size() > 1 && loadedLimbId != anchorLimbId) {
				loadedLimbHasExtraJoints = true; }

			if (anchorLimbId > -1 && loadedLimbId != playerCharacter.getAnchorLimbId()) {
				tuple<int, int> limbIdAndJointIndexForConnection = playerCharacter.getLimbIdAndJointIndexForConnection(anchorLimbId, loadedLimbId);
				int possibleConnectorLimbId = get<0>(limbIdAndJointIndexForConnection);
				int possibleConnectorJointId = get<1>(limbIdAndJointIndexForConnection);

				if (possibleConnectorLimbId >= 0 && possibleConnectorJointId >= 0 && possibleConnectorLimbId != loadedLimbId) {
					characterHasExtraJoints = true;
				}
			}
		}
		limbLoadedPanel = ui.createLimbLoadedModePanel(loadedLimbHasExtraJoints, characterHasExtraJoints);
	}

	void createChooseLimbPanel(UI& ui = UI::getInstance()) {
		/* Destroy textures if they already exist. */
		if (chooseLimbPanel.getButtons().size() > 0) {
			chooseLimbPanel.destroyTextures();
		}

		/* Build a vector of data structures so the UI can build the panel of Limb buttons. */
		vector<LimbButtonData> limbBtnDataStructs;
		vector<Limb>& inventoryLimbs = playerCharacter.getLimbs();

		for (int i = 0; i < inventoryLimbs.size(); ++i) {
			/* FOR NOW we want the index. But when we bring in the database, we will use the ID. So the LimbButtonData says id instead of index. */
			Limb& thisLimb = inventoryLimbs[i];
			if (!thisLimb.isEquipped()) {
				limbBtnDataStructs.emplace_back(thisLimb.getTexturePath(), thisLimb.getName(), i);
			}
		}

		chooseLimbPanel = ui.createChooseLimbModePanel(limbBtnDataStructs);
	}

	void changeCreationMode(CreationMode creationMode) {
		this->creationMode = creationMode;

		switch(creationMode){
		case CreationMode::ChooseLimb:
			chooseLimbPanel.setShow(true);
			reviewModePanel.setShow(false);
			limbLoadedPanel.setShow(false);
			break;
		case CreationMode::Review:
			chooseLimbPanel.setShow(false);
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

	ScreenType getScreenType() { return screenType; }
	void drawChildLimbs(Limb& parentLimb, UI& ui);
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

	Panel settingsPanel;
	Panel gameMenuPanel;
	Panel reviewModePanel;
	Panel limbLoadedPanel;
	Panel chooseLimbPanel;
	Panel messagePanel;

	void drawCharacter(UI& ui);

	bool limbLoaded;
	int loadedLimbId; /* Currently holds index from vector. Will hold id from DB. */

	void draw(UI& ui);
	void handleEvent(SDL_Event& e, bool& running, GameState& gameState);
	void checkMouseLocation(SDL_Event& e);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);
	void createTitleTexture(UI& ui);
	void raiseTitleRect() { --titleRect.y; }
	int getTitleBottomPosition() { return titleRect.y + titleRect.h; }
	void drawLimb(Limb& limb, UI& ui);

	void showNewMessage(string newMessage, ConfirmationButtonType confirmationType, bool showRefuse, UI& ui = UI::getInstance());

	bool showTitle;
	bool drawLoadedLimb;
	int titleCountdown;
	int loadedLimbCountdown;

	Character playerCharacter;
	CreationMode creationMode;
};

void CharacterCreationScreen::getBackgroundTexture(UI& ui) {
	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };
	bgTexture = ui.createBackgroundTexture();
}

export void CharacterCreationScreen::run() {
	/* singletons */
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();

	settingsPanel.setShow(false);
	gameMenuPanel.setShow(false);
	reviewModePanel.setShow(true);

	/* Timeout data */
	const int TARGET_FPS = 120;
	const int FRAME_DELAY = 1200 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	/* loop and event control */
	SDL_Event e;
	bool running = true;

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
			SDL_Delay(FRAME_DELAY - frameTimeElapsed); }
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

	chooseLimbPanel.draw(ui);
	limbLoadedPanel.draw(ui);
	reviewModePanel.draw(ui);
	settingsPanel.draw(ui);
	//gameMenuPanel.draw(ui);
	messagePanel.draw(ui);
	drawCharacter(ui);

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
void CharacterCreationScreen::rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	ui.rebuildGameMenuPanel(gameMenuPanel);
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
			} else if (settingsPanel.getShow() && settingsPanel.isInPanel(mouseX, mouseY)) {
				ButtonClickStruct clickStruct = settingsPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::Mobile:
					ui.resizeWindow(WindowResType::Mobile);
					rebuildDisplay(settingsPanel, gameMenuPanel);
					break;
				case ButtonOption::Tablet:
					ui.resizeWindow(WindowResType::Tablet);
					rebuildDisplay(settingsPanel, gameMenuPanel);
					break;
				case ButtonOption::Desktop:
					ui.resizeWindow(WindowResType::Desktop);
					rebuildDisplay(settingsPanel, gameMenuPanel);
					break;
				case ButtonOption::Fullscreen:
					ui.resizeWindow(WindowResType::Fullscreen);
					rebuildDisplay(settingsPanel, gameMenuPanel);
					break;
				case ButtonOption::Back:
					// switch to other panel
					settingsPanel.setShow(false);
					reviewModePanel.setShow(true);
					break;
				case ButtonOption::Exit:
					/* back to menu screen */
					running = false;
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else if (reviewModePanel.getShow() && reviewModePanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK REVIEW MENU \n\n";
				ButtonClickStruct clickStruct = reviewModePanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::MapOptions:
					cout << "\nMAP OPTIONS\n";
					break;
				case ButtonOption::Settings:
					settingsPanel.setShow(true);
					reviewModePanel.setShow(false);
					break;
				case ButtonOption::ShowLimbs:
					createChooseLimbPanel();
					chooseLimbPanel.setShow(true);
					reviewModePanel.setShow(false);
					break;
				case ButtonOption::ClearSuit:
					cout << "CLEARING SUIT\n";
					playerCharacter.clearSuit();
					break;
				case ButtonOption::SaveSuit:
					cout << "SAVING SUIT\n";
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else if (chooseLimbPanel.getShow() && chooseLimbPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK LIMB MENU \n\n";
				ButtonClickStruct clickStruct = chooseLimbPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();

				/* If we sent in a limb id/index: */
				if (clickStruct.itemID >= 0) {
					Limb& clickedLimb = playerCharacter.getLimbs()[clickStruct.itemID];
					bool limbEquipped = false;

					/* see what button might have been clicked : */
					switch (clickStruct.buttonOption) {
					case ButtonOption::LoadLimb:
						/*
						* Get the limb index. Where should it be stored? Edit the BUTTON.
						* BUTTON should also optionally take a texture as an argument.
						* ClickStruct has an ITEM ID!!!!!
						*/
						
						/* Make sure there is somewhere to attach the limb. */
						if (
							playerCharacter.getAnchorLimbId() >= 0 &&
							get<0>(playerCharacter.getLimbIdAndJointIndexForConnection(playerCharacter.getAnchorLimbId(), clickStruct.itemID)) < 0
						) {
							showNewMessage(
								Resources::getInstance().getMessageText("NO_FREE_CHAR_JOINTS"),
								ConfirmationButtonType::OkCancel, false
							);
							break;
						}

						/* Now actually equip the limb if it isn't already equipped. */
						if (!clickedLimb.isEquipped()) {
							loadedLimbId = clickStruct.itemID;
							limbEquipped = playerCharacter.equipLimb(clickStruct.itemID);

							if (!limbEquipped) {
								break;
							}

							if (loadedLimbId == playerCharacter.getAnchorLimbId()) {
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

							createLimbLoadedPanel();
							changeCreationMode(CreationMode::LimbLoaded);
						}
						//printAllLimbs(playerCharacter);
						
						break;
					default:
						cout << "ERROR\n";
					}
				}
				else {
					/* see what button might have been clicked : */
					switch (clickStruct.buttonOption) {
					case ButtonOption::Back:
						// switch to other panel
						changeCreationMode(CreationMode::Review);
						break;
					default:
						cout << "ERROR\n";
					}
				}
			}
			else if (limbLoadedPanel.getShow() && limbLoadedPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK REVIEW MENU \n\n";
				ButtonClickStruct clickStruct = limbLoadedPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();

				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::Equip:
					loadedLimbId = -1;
					changeCreationMode(CreationMode::Review);
					cout << "\nEQUIPPING LOADED LIMB\n";
					break;
				case ButtonOption::NextCharJoint:
					/* 
					* Move the loaded limb to the next available joint in the character's equipped limbs.
					* Also, cycle back to the beginning of the list once we reach the end.
					* Maybe put this in a function in the character class.
					*/

					if (loadedLimbId >= 0) {
						bool switched = playerCharacter.shiftChildLimb(loadedLimbId); }

					break;
				case ButtonOption::NextLimbJoint:
					/* Change the anchor joint of the loaded limb. */
					if (playerCharacter.getAnchorLimbId() == loadedLimbId) {
						break;
					}
					else {
						Limb& loadedLimb = playerCharacter.getLimbs()[loadedLimbId];
						bool anchorShifted = loadedLimb.shiftAnchorLimb();
						break;
					}

				case ButtonOption::RotateClockwise:
					if (loadedLimbId >= 0 && loadedLimbId < playerCharacter.getLimbs().size()) {
						Limb& loadedLimb = playerCharacter.getLimbs()[loadedLimbId];
						loadedLimb.rotate(15);
					}
					break;
				case ButtonOption::RotateCounterClockwise:
					if (loadedLimbId >= 0 && loadedLimbId < playerCharacter.getLimbs().size()) {
						Limb& loadedLimb = playerCharacter.getLimbs()[loadedLimbId];
						loadedLimb.rotate(-15);
					}
					break;
				case ButtonOption::UnloadLimb:
					cout << "UNLOADING LIMB #" << loadedLimbId << "\n";
					/*
					* Unequip that limb.
					* Go back to the ChooseLimb mode.
					*/
					if (loadedLimbId >= 0) {
						cout << "UNLOADING LIMB #" << loadedLimbId << "?????\n";
						Limb& loadedLimb = playerCharacter.getLimbs()[loadedLimbId];
						if (loadedLimbId == playerCharacter.getAnchorLimbId()) {
							playerCharacter.setAnchorLimbId(-1); }
						playerCharacter.unEquipLimb(loadedLimbId);
					}
					changeCreationMode(CreationMode::ChooseLimb);
					loadedLimbId = -1;
					break;
				default:
					cout << "ERROR\n";
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
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (gameMenuPanel.getShow()) { gameMenuPanel.checkMouseOver(mouseX, mouseY); }
	if (reviewModePanel.getShow()) { reviewModePanel.checkMouseOver(mouseX, mouseY); }
	if (limbLoadedPanel.getShow()) { limbLoadedPanel.checkMouseOver(mouseX, mouseY); }
	if (chooseLimbPanel.getShow()) { chooseLimbPanel.checkMouseOver(mouseX, mouseY); }
	if (messagePanel.getShow()) { messagePanel.checkMouseOver(mouseX, mouseY); }
}

/* Remember to delete this SDL_Point pointer if it's not null!
*  TO DO: Maybe we should replace ALL Point objects with SDL_Points to avoid using this function so often?
* Or just for the Joint Points.
*/
SDL_Point* getRotationPointSDL(Limb& limb, int anchorJointId) {
	if (anchorJointId < 0) { return NULL; }
	Point anchorPoint = limb.getJoints()[anchorJointId].getPoint();
	return new SDL_Point(anchorPoint.x, anchorPoint.y);
}


/* FOR DEBUGGING: Draw a box over each Joint.*/
void drawJoints(Limb& limb, UI& ui) {
		SDL_Color jointColor;
		jointColor.r = 255;
		jointColor.g = 255;
		jointColor.b = 51;
		jointColor.a = 255; // Alpha (fully opaque)

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
* Drawing limb. This can possible be moved to Limb class (we'll see if the Map and Battle draw functions are different).
*/
void CharacterCreationScreen::drawLimb(Limb& limb, UI& ui) {
	/* rotationPoint should already be an SDL_Point in the Joint object.
	* My Point object is redundant.
	* WAIT... MAYBE NOT.
	* There is no "rotation point", there are just joint points or else NULL.
	* Further consideration is necessary.
	* Maybe I do need a rotationPoint member which holds an SDL_Point pointer... GOOD IDEA.
	*/
	SDL_Point* rotationPoint = getRotationPointSDL(limb, limb.getAnchorJointId());

	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		limb.getTexture(),
		NULL, &limb.getDrawRect(),
		limb.getRotationAngle(), rotationPoint, SDL_FLIP_NONE
	);
	if (rotationPoint != NULL) { delete rotationPoint; }
	if (DRAW_JOINTS) { drawJoints(limb, ui); } /* For debugging. */
}

void CharacterCreationScreen::drawChildLimbs(Limb& parentLimb, UI& ui) {
	vector<Limb>& limbs = playerCharacter.getLimbs();
	vector<Joint>& anchorJoints = parentLimb.getJoints();
	SDL_Rect& parentRect = parentLimb.getDrawRect();

	for (int i = 0; i < anchorJoints.size(); ++i) {
		Joint& anchorJoint = anchorJoints[i];
		int connectedLimbId = anchorJoint.getConnectedLimbId();
		if (connectedLimbId < 0) { continue; }

		Point anchorJointPoint = anchorJoint.getPoint();
		Limb& connectedLimb = limbs[anchorJoint.getConnectedLimbId()];

		/* make sure it has an anchor joint (make a function which checks???)... if not, return and stop drawing. */
		Joint& connectedLimbAnchorJoint = connectedLimb.getAnchorJoint();
		Point connectedLimbAnchorJointPoint = connectedLimbAnchorJoint.getPoint();

		/* First offset by parent limb location,
		* then by the JOINT to which we are connected.
		* THEN offset by THIS limb's anchor joint
		*/

		connectedLimb.setDrawRect({
			parentRect.x + anchorJointPoint.x - connectedLimbAnchorJointPoint.x,
			parentRect.y + anchorJointPoint.y - connectedLimbAnchorJointPoint.y,
			parentRect.w,
			parentRect.h
		});

		/* Skip "loaded" limb when drawLoadedLimb is false (for hot blinking action). */
		if (connectedLimbId != loadedLimbId || drawLoadedLimb) {
			drawLimb(connectedLimb, ui); }

		/* Recursively draw all THIS limb's child limbs. */
		drawChildLimbs(connectedLimb, ui);
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
	* We will want the draw order to eventually be (optionally) different from the hierarchy... so each will need its own SDL_Rect.
	* So this algorithm which currently DRAWS each limb... will eventually need to SET each rect and draw LATER.
	* But we DO NOT want to calculate each rect for each frame of the animation.
	* SO we will instead make an unordered_map of RECTs and LIMBs, and update them...
	* NO... instead it will be a vector of new structs?
	* We'll figure it out later... we need the draw order, the rect, the angle, many things...
	*/


	/* Skip "loaded" limb when drawLoadedLimb is false (for hot blinking action). */
	if (anchorLimbId != loadedLimbId || drawLoadedLimb) {
		drawLimb(anchorLimb, ui); }

	/* Now the recursion... a "draw all limbs" function. */
	drawChildLimbs(anchorLimb, ui);
}