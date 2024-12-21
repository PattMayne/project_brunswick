/* 
* THERE IS NO ABSTRACT SCREEN. Each Screen object is a VARIATION of each other, but they will NEVER be used interchangably. There's no benefit to that.
*/

/*
* NEXT:
* -- Make Buttons responsive (on hover)
* -- Make window size adjustable
* --	Each Screen will need to capture resize window event from the queue in the loop, and then resize.
* -- Make "New Game" open MAP screen
* --	MAP screen has three buttons (vertical): OTHER MAP, MANU, and BATTLE
* -- Make BATTLE screen with two buttons: BACK TO MAP and MENU
* -- Make "About" open a panel with text.
* -- Make "Settings" open a new menu. (will there really be settings? No... maybe delete!)
* -- Make "Load Game" load different Menu (unless there will only be one game?) (leave alone for now actually)
*/

module;

export module MenuScreen;
import "include/json.hpp";
import "SDL.h";
import "SDL_image.h";
import <stdio.h>;
import <string>;
import <iostream>;
import "SDL_ttf.h";
import <vector>;
import <cstdlib>;
import <time.h>;
import <unordered_map>;

import TypeStorage;
import GameState;
import Resources;
import UI;

using namespace std;

export class MenuScreen {
	public:
		/* constructor */
		MenuScreen() {
			UI& ui = UI::getInstance();
			screenType = ScreenType::Menu;
			screenToLoadStruct = ScreenStruct(ScreenType::NoScreen, -1);
			getBackgroundTexture(ui);
			createTitleTexture(ui);
		}

		/* Destructor */
		~MenuScreen() {
			SDL_DestroyTexture(bgTexture);
			SDL_DestroyTexture(titleTexture);
		}

		void setParentStruct(ScreenStruct incomingParentStruct) {
			screenToLoadStruct = incomingParentStruct;
		}

		ScreenStruct getParentStruct() { return screenToLoadStruct; }

		void getBackgroundTexture(UI& ui);
		void createTitleTexture(UI& ui);
		void run();

	private:
		ScreenType screenType;
		ScreenStruct screenToLoadStruct;
		SDL_Texture* bgTexture = NULL;
		SDL_Rect bgSourceRect;
		SDL_Rect bgDestinationRect;
		SDL_Texture* titleTexture;
		SDL_Rect titleRect;
		void handleEvent(SDL_Event &e, bool& running, Panel& menuPanel, Panel& settingsPanel, GameState& gameState);
		void checkMouseLocation(SDL_Event& e, Panel& menuPanel, Panel& settingsPanel);
		void rebuildDisplay(Panel& menuPanel, Panel& settingsPanel);
		void draw(UI& ui, Panel& menuPanel, Panel& settingsPanel);
		void drawPanel(UI& ui, Panel& panel);
};

/* The main function of the class. Contains the game loop. */
void MenuScreen::run() {
	cout << "Menu Screen loaded\n";
	// Get reference to UI singleton for the loop
	UI& ui = UI::getInstance();
	GameState& gameState = GameState::getInstance();
	Panel menuPanel = ui.createMainMenuPanel();
	Panel settingsPanel = ui.createSettingsPanel();
	menuPanel.setShow(true);
	settingsPanel.setShow(false);

	/* Timeout data */
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
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
			handleEvent(e, running, menuPanel, settingsPanel, gameState);
		}

		/* check mouse location in every frame (so buttons stay "hovered" after click */
		checkMouseLocation(e, menuPanel, settingsPanel);

		draw(ui, menuPanel, settingsPanel);

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

/* Specific Draw functions for each Screen */

void MenuScreen::drawPanel(UI& ui, Panel& panel) {
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

/* 
* All data has been updated. Time to draw a representation of the current state of things.
* We draw one panel at a time.
* 
* TODO: There is a problem here. We should NOT be drawing the background twice.
* CLEAN THIS UP.
* 
* Send in a vector of panels.
*/
void MenuScreen::draw(UI& ui, Panel& menuPanel, Panel& settingsPanel) {

	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 14, 14, 14, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* print the BG image) */
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);

	/* draw the actual panels */
	drawPanel(ui, settingsPanel);
	drawPanel(ui, menuPanel);

	/* draw the logo */
	SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}

void MenuScreen::getBackgroundTexture(UI& ui) {
	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };
	bgTexture = ui.createBackgroundTexture();
}

/* Create the texture with the name of the game */
void MenuScreen::createTitleTexture(UI& ui) {
	Resources& resources = Resources::getInstance();
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture(resources.getTitle());
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}

/* Screen has been resized. Rebuild! */
void MenuScreen::rebuildDisplay(Panel& menuPanel, Panel& settingsPanel) {
	UI& ui = UI::getInstance();
	getBackgroundTexture(ui);
	createTitleTexture(ui);
	ui.rebuildSettingsPanel(settingsPanel);
	ui.rebuildMainMenuPanel(menuPanel);
}

/* Process user input */
void MenuScreen::handleEvent(SDL_Event& e, bool& running, Panel& menuPanel, Panel& settingsPanel, GameState& gameState) {
	/* User pressed X to close */
	if (e.type == SDL_QUIT) { running = false; }
	else {
		// user clicked
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			cout << "user clicked mouse\n";
			// These events might change the value of screenToLoad
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);

			if (menuPanel.getShow() && menuPanel.isInPanel(mouseX, mouseY)) {

				// panel has a function to return which ButtonOption was clicked, and an ID (in the ButtonClickStruct).
				ButtonClickStruct clickStruct = menuPanel.checkButtonClick(mouseX, mouseY);

				// see what button might have been clicked:
				switch (clickStruct.buttonOption) {
				case ButtonOption::About:
					screenToLoadStruct.screenType = ScreenType::CharacterCreation;
					running = false;
					cout << "ABOUT\n";
					break;
				case ButtonOption::NoOption:
					cout << "NO OPTION\n";
					break;
				case ButtonOption::NewGame:
					screenToLoadStruct.screenType = ScreenType::Map;
					running = false;
					cout << "NEW GAME\n";
					break;
				case ButtonOption::LoadGame:
					screenToLoadStruct.screenType = ScreenType::Battle;
					running = false;
					cout << "LOAD GAME (currently battle screen)\n";
					break;
				case ButtonOption::Settings:
					// switch to other panel
					settingsPanel.setShow(true);
					menuPanel.setShow(false);
					cout << "SETTINGS\n";
					break;
				case ButtonOption::Exit:
					gameState.setScreenType(ScreenType::NoScreen);
					running = false;
					break;
				default:
					cout << "ERROR\n";
				}
			} else if (settingsPanel.getShow() && settingsPanel.isInPanel(mouseX, mouseY)) {

				// panel has a function to return which ButtonOption was clicked, and an ID (in the ButtonClickStruct).
				ButtonClickStruct clickStruct = settingsPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				// see what button might have been clicked:
				switch (clickStruct.buttonOption) {
				case ButtonOption::Mobile:
					ui.resizeWindow(WindowResType::Mobile);
					rebuildDisplay(menuPanel, settingsPanel);
					break;
				case ButtonOption::Tablet:
					ui.resizeWindow(WindowResType::Tablet);
					rebuildDisplay(menuPanel, settingsPanel);
					break;
				case ButtonOption::Desktop:
					ui.resizeWindow(WindowResType::Desktop);
					rebuildDisplay(menuPanel, settingsPanel);
					break;
				case ButtonOption::Fullscreen:
					ui.resizeWindow(WindowResType::Fullscreen);
					rebuildDisplay(menuPanel, settingsPanel);
					break;
				case ButtonOption::Back:
					// switch to other panel
					settingsPanel.setShow(false);
					menuPanel.setShow(true);
					break;
				default:
					cout << "ERROR\n";
				}
			}
		}
	}
}

void MenuScreen::checkMouseLocation(SDL_Event& e, Panel& menuPanel, Panel& settingsPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (menuPanel.getShow()) { menuPanel.checkMouseOver(mouseX, mouseY); }
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
}