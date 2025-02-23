/**
* 
*  ____    _  _____ _____ _     _____
* | __ )  / \|_   _|_   _| |   | ____|
* |  _ \ / _ \ | |   | | | |   |  _|
* | |_) / ___ \| |   | | | |___| |___
* |____/_/___\_\_|_  |_|_|_____|_____|
* / ___| / ___|  _ \| ____| ____| \ | |
* \___ \| |   | |_) |  _| |  _| |  \| |
*  ___) | |___|  _ <| |___| |___| |\  |
* |____/ \____|_| \_\_____|_____|_| \_|
* 
* Battle Screen.
* 
* -- Move panels into BattleScreen object.
* 
* 
* BATTLE SYSTEM:
* 
* - When attacking a limb, some of the damage inevitably spreads to the connected limbs instead.
* ---- Higher "intelligence" means more accuracy.
* - Type of attack depend on dominance cycle.
* 
* 
* 
* TO DO:
* - Create Battle Class.
* - Save battle object to DB (from MapScreen).
* - Load battle oject from DB (based on battle_id saved to PLAYER character).
* - Display both opponents (full drawn limbs, no avatar).
* - Animate both opponents (constantly rotating one limbs or another... just one at a time?).
* - HUD of player stats.
* - HUD of opponent stats.
* - Menu of UNEQUIPPED player limbs
* ---- Include dominance cycle
* - Select opponent Limb to attack via right-hand button.
* --- selected limb flashes.
* - Player limb to HEAL (by consuming LIMB from unequipped limbs).
* - THROW unequipped Limb (only if we have arm or leg to throw or kick).
* ---- THROW is more accurate but less power.
* ---- KICK is less accurate but more power.
* - PassingMessage panel shows effects of latest attack.
* 
*/



module;
export module BattleScreen;

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
import CharacterClasses;
import BattleClasses;
import Database;

using namespace std;

/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class BattleScreen {
public:
	/* constructor */
	BattleScreen() {
		GameState& gameState = GameState::getInstance();
		UI& ui = UI::getInstance();

		screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

		id = gameState.getScreenStruct().id;
		screenType = ScreenType::Battle;
		battle = loadBattle(id);

		getBackgroundTexture(ui);
		createTitleTexture(ui);

		settingsPanel = ui.createSettingsPanel(ScreenType::Map);
		gameMenuPanel = ui.createGameMenuPanel();
		settingsPanel.setShow(false);
		gameMenuPanel.setShow(true);

		showTitle = true;
		titleCountdown = 140;
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

	void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState);
	void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel);
	void createTitleTexture(UI& ui);

	void raiseTitleRect() { --titleRect.y; }
	int getTitleBottomPosition() { return titleRect.y + titleRect.h; }

	bool showTitle;
	int titleCountdown;

	/* panels */
	Panel settingsPanel;
	Panel gameMenuPanel;
	Battle battle;
};

void BattleScreen::getBackgroundTexture(UI& ui) {
	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };
	bgTexture = ui.createBackgroundTexture();
}

export void BattleScreen::run() {
	/* singletons */
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();


	/* Timeout data */
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	/* loop and event control */
	SDL_Event e;
	bool running = true;

	if (gameState.getScreenStruct().id < 1) {
		/* Invalid battle id. Give a notice to the user on the way out? Or after getting back?
		* Maybe GameState deserves a message string to display.
		*/
		running = false;
	}

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
			SDL_Delay(FRAME_DELAY - frameTimeElapsed);
		}
	}

	/* set the next screen to load */
	gameState.setScreenStruct(screenToLoadStruct);
}


void BattleScreen::draw(UI& ui, Panel& settingsPanel, Panel& gameMenuPanel) {
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* draw BG for now */
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);
	
	/* draw the title */
	if (showTitle) {
		SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);
	}

	settingsPanel.draw(ui);
	gameMenuPanel.draw(ui);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}


/* Create the texture with the name of the game */
void BattleScreen::createTitleTexture(UI& ui) {
	string playerName = battle.getPlayerCharacter().getName();
	string npcName = battle.getNpc().getName();

	bool twoNamesExist = playerName != "" && npcName != "";

	string titleString = "BATTLE!";

	if (twoNamesExist) {
		titleString = playerName + " vs " + npcName;
	}

	Resources& resources = Resources::getInstance();
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture(titleString);
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}


/* Screen has been resized. Rebuild! */
void BattleScreen::rebuildDisplay(Panel& settingsPanel, Panel& gameMenuPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	ui.rebuildGameMenuPanel(gameMenuPanel);
	getBackgroundTexture(ui);
	createTitleTexture(ui);
}


/* Process user input */
void BattleScreen::handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& gameMenuPanel, GameState& gameState) {
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

void BattleScreen::checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& gameMenuPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (gameMenuPanel.getShow()) { gameMenuPanel.checkMouseOver(mouseX, mouseY); }
}