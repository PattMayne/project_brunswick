/*
* Load limbs from JSON
* create a surface that's 1000x1000 (or more?)
* --- snap limbs together on this surface, then generate a texture to resize & display on the screen, and free the surface.
* ------- each time we change the limb setup, we will destroy the texture and replace it with a new texture from the new setup.
* 
* 
* ROTATION:
* ----- SDL_RenderCopyEx takes parameters for angle of rotation AND a point for the point around which the image rotates.
* ---------- SO I can save (A) whether the image should be FLIPPED, (B) angle of rotation, and (C) point of rotation to the Limb and the DB.
* ---------- SDL_RenderCopyEx will then render the texture accordingly.
* ---------- When flipping or rotating, we will have to calculate how that affects any limbs that are attached to that limb.
* --------------- We will need to override joint Points in Limb to accomodate flipping and rotating.
* --------------- We will need to store information about which joints are attached to which. This will be complicated, when it comes to the Battle Screen.
* -------------------- Draw Position is not sufficient anymore. We need Anchor Joint... nested branches of Limbs... this will be complicated, but it will be cool in the end.
* 
* 
* TO DO:
* 1. Cycle through loadedLimb joints.
* 2. Cycle through Character joints.
* 3. Rotate loaded limb.
* 4. In panel, only add buttons for Limbs that are NOT equipped (not in this file, in the UI file).
* 5. Re-write to draw the limbs in a different order.
* 6. Click a button (new panel for equipped limbs in Review mode) to bring THAT limb to the top of the draw order.
* 
* 
* LOADING A LIMB:
* -- A "loaded" limb is actually equipped.
* But we "hold" it and manipulate its position according to button presses from the player.
* "EQUIP" button just sets "limbLoaded" to false and "loadedLimbId" back to -1, and changes the MODE (and therefore which panel is shown (review mode panel).
* 
* Loading / equipping limbs should be done with functions in the Character object.
* That way I can reuse them for NPCs being created on the map.
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

using namespace std;

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
		playerCharacter = buildPlayerCharacter();

		limbLoaded = false;
		loadedLimbId = -1;

		showTitle = true;
		titleCountdown = 140;

		/* create panels */
		settingsPanel = ui.createSettingsPanel(ScreenType::Map);
		gameMenuPanel = ui.createGameMenuPanel();
		reviewModePanel = ui.createReviewModePanel();
		limbLoadedPanel = ui.createLimbLoadedModePanel();

		/* Build a vector of data structures so the UI can build the panel of Limb buttons. */
		vector<LimbButtonData> limbBtnDataStructs;
		vector<Limb>& inventoryLimbs = playerCharacter.getLimbs();

		for (int i = 0; i < inventoryLimbs.size(); ++i) {
			/* FOR NOW we want the index. But when we bring in the database, we will use the ID. So the LimbButtonData says id instead of index. */
			Limb& thisLimb = inventoryLimbs[i];
			limbBtnDataStructs.emplace_back(thisLimb.getTexturePath(), thisLimb.getName(), i); }

		chooseLimbPanel = ui.createChooseLimbModePanel(limbBtnDataStructs);
		creationMode = CreationMode::Review;

		cout << playerCharacter.getLimbs().size() << " LIMBS\n";
		drawLoadedLimb = true;
		loadedLimbCountdown = 20;

	}

	/* Destructor */
	~CharacterCreationScreen() {
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyTexture(titleTexture);
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

	void drawCharacter(UI& ui);

	bool limbLoaded;
	int loadedLimbId; /* Currently holds index from vector. Will hold id from DB. */

	void draw(UI& ui);
	void drawPanel(UI& ui, Panel& panel);
	void handleEvent(SDL_Event& e, bool& running, GameState& gameState);
	void checkMouseLocation(SDL_Event& e);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);
	void createTitleTexture(UI& ui);
	void raiseTitleRect() { --titleRect.y; }
	int getTitleBottomPosition() { return titleRect.y + titleRect.h; }
	void drawLimb(Limb& limb, UI& ui);

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

	/* draw the title */
	SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);

	drawPanel(ui, settingsPanel);
	//drawPanel(ui, gameMenuPanel);
	drawPanel(ui, reviewModePanel);
	
	drawPanel(ui, limbLoadedPanel);
	drawPanel(ui, chooseLimbPanel);
	drawCharacter(ui);

	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}


void CharacterCreationScreen::drawPanel(UI& ui, Panel& panel) {
	if (!panel.getShow()) { return; }
	for (Button button : panel.getButtons()) {
		/* get the rect, send it a reference(to be converted to a pointer) */
		SDL_Rect rect = button.getRect();

		/* now draw the button texture */
		SDL_RenderCopyEx(
			ui.getMainRenderer(),
			button.isMouseOver() ? button.getHoverTexture() : button.getNormalTexture(),
			NULL, &rect,
			0, NULL, SDL_FLIP_NONE
		);
	}
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

/* Process user input */
void CharacterCreationScreen::handleEvent(SDL_Event& e, bool& running, GameState& gameState) {
	/* User pressed X to close */
	if (e.type == SDL_QUIT) {
		cout << "\nQUIT\n";
		running = false;
		return;
	}
	else {
		// user clicked
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			cout << "user clicked mouse\n";
			// These events might change the value of screenToLoad
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (settingsPanel.getShow() && settingsPanel.isInPanel(mouseX, mouseY)) {

				/* panel has a function to return which ButtonOption was clicked, and an ID(in the ButtonClickStruct). */
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

						if (!clickedLimb.isEquipped()) {
							loadedLimbId = clickStruct.itemID;
							limbEquipped = playerCharacter.equipLimb(clickStruct.itemID);
							changeCreationMode(CreationMode::LimbLoaded);
							limbLoadedPanel.setShow(true);
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
					cout << "ROTATING CLOCKWISE\n";
					break;
				case ButtonOption::RotateCounterClockwise:
					cout << "ROTATING COUNTER-CLOCKWISE\n";
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
}

void CharacterCreationScreen::drawLimb(Limb& limb, UI& ui) {
	SDL_RenderCopyEx(
		ui.getMainRenderer(),
		limb.getTexture(),
		NULL, &limb.getDrawRect(),
		limb.getRotationAngle(), NULL, SDL_FLIP_NONE
	);
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

	/* FOR NOW we will use the natural size of the textures. */
	int limbWidth, limbHeight;
	SDL_QueryTexture(anchorLimb.getTexture(), NULL, NULL, &limbWidth, &limbHeight);

	/* 
	* We will want the draw order to eventually be (optionally) different from the hierarchy... so each will need its own SDL_Rect.
	* So this algorithm which currently DRAWS each limb... will eventually need to SET each rect and draw LATER.
	* But we DO NOT want to calculate each rect for each frame of the animation.
	* SO we will instead make an unordered_map of RECTs and LIMBs, and update them...
	* NO... instead it will be a vector of new structs?
	* We'll figure it out later... we need the draw order, the rect, the angle, many things...
	*/

	anchorLimb.setDrawRect({
		(screenWidth / 2) - (limbWidth / 2),
		(screenHeight / 2) - (limbHeight / 2),
		limbWidth,
		limbHeight
	});

	/* Skip "loaded" limb when drawLoadedLimb is false (for hot blinking action). */
	if (anchorLimbId != loadedLimbId || drawLoadedLimb) {
		drawLimb(anchorLimb, ui); }

	/* Now the recursion... a "draw all limbs" function. */
	drawChildLimbs(anchorLimb, ui);
}