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
* 1.	Build the PANELS.
* ---------- MAIN PANEL:: SHOW LIMBS / HIDE LIMBS / SAVE / CLEAR
* ---------- LOADED LIMB PANEL:: NEXT LIMB JOINT / NEXT CHARACTER JOINT / ROTATE LIMB CLOCKWISE / COUNTER-CLOCKWISE /  REMOVE LIMB
* ---------- INVENTORY PANEL:: Make it a full-screen panel. A button for each limb.
* ---------- LOADED LIMB PANEL:: click a limb to bring it to the top.
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

		showTitle = true;
		titleCountdown = 140;


		cout << playerCharacter.getInventoryLimbs().size() << " LIMBS\n";
	}

	/* Destructor */
	~CharacterCreationScreen() {
		SDL_DestroyTexture(bgTexture);
		SDL_DestroyTexture(titleTexture);
	}

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

	void draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel);
	void drawPanel(UI& ui, Panel& panel);

	void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState);
	void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);

	void createTitleTexture(UI& ui);

	void raiseTitleRect() {
		--titleRect.y; }

	int getTitleBottomPosition() {
		return titleRect.y + titleRect.h; }

	bool showTitle;
	int titleCountdown;

	Character playerCharacter;
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
	/* panels */
	Panel settingsPanel = ui.createSettingsPanel(ScreenType::Map);
	Panel gameMenuPanel = ui.createGameMenuPanel();
	settingsPanel.setShow(false);
	gameMenuPanel.setShow(true);

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
				handleEvent(e, running, settingsPanel, gameMenuPanel, gameState);
			}
		}

		/* Deal with showing the title. */
		if (showTitle) {
			if (titleCountdown > 0) {
				--titleCountdown;
			}
			else {
				raiseTitleRect();
			}

			if (getTitleBottomPosition() < -1) {
				showTitle = false;
			}
		}

		checkMouseLocation(e, settingsPanel, gameMenuPanel);
		draw(ui, settingsPanel, gameMenuPanel);

		/* Delay so the app doesn't just crash */
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
		/* Delay loop */
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed); }
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}


void CharacterCreationScreen::draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel) {
	//unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* draw BG for now */
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);

	/* draw the title */
	SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);

	drawPanel(ui, settingsPanel);
	drawPanel(ui, gameMenuPanel);
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


/* Process user input */
void CharacterCreationScreen::handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState) {
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
					gameMenuPanel.setShow(true);
					break;
				case ButtonOption::Exit:
					/* back to menu screen */
					running = false;
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else if (gameMenuPanel.getShow() && gameMenuPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK MAP MENU \n\n";
				ButtonClickStruct clickStruct = gameMenuPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::MapOptions:
					cout << "\nMAP OPTIONS\n";
					break;
				case ButtonOption::Settings:
					settingsPanel.setShow(true);
					gameMenuPanel.setShow(false);
					break;
				default:
					cout << "ERROR\n";

				}
			}
		}
	}
}

void CharacterCreationScreen::checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (gameMenuPanel.getShow()) { gameMenuPanel.checkMouseOver(mouseX, mouseY); }
}