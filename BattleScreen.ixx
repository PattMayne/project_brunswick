module;
#include "include/json.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <vector>
#include <cstdlib>
#include <time.h>
#include <unordered_map>

export module BattleScreen;

using namespace std;

import ScreenType;
import GameState;
import Resources;
import UI;

/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class BattleScreen {
public:
	/* constructor */
	BattleScreen() {
		cout << "\nLoading Battle Screen\n\n";
		screenType = ScreenType::Battle;
		id = 0;
		screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);
		UI& ui = UI::getInstance();
		getBackgroundTexture(ui);
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

	void draw(UI& ui, Panel& settingsPanel, Panel& mapMenuPanel);
	void drawPanel(UI& ui, Panel& panel);

	void handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& mapMenuPanel, GameState& gameState);
	void checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& mapMenuPanel);

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel, Panel& mapMenuPanel);
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
	/* panels */
	Panel settingsPanel = ui.createSettingsPanel(ScreenType::Map);
	Panel mapMenuPanel = ui.createMapMenuPanel();
	settingsPanel.setShow(false);
	mapMenuPanel.setShow(true);

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
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				handleEvent(e, running, settingsPanel, mapMenuPanel, gameState);
			}
		}

		checkMouseLocation(e, settingsPanel, mapMenuPanel);
		draw(ui, settingsPanel, mapMenuPanel);

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


void BattleScreen::draw(UI& ui, Panel& settingsPanel, Panel& mapMenuPanel) {
	//unordered_map<string, SDL_Color> colorsByFunction = ui.getColorsByFunction();
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* draw BG for now */
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);

	drawPanel(ui, settingsPanel);
	drawPanel(ui, mapMenuPanel);
	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}


void BattleScreen::drawPanel(UI& ui, Panel& panel) {
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



/* Screen has been resized. Rebuild! */
void BattleScreen::rebuildDisplay(Panel& settingsPanel, Panel& mapMenuPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	ui.rebuildMapMenuPanel(mapMenuPanel);
	getBackgroundTexture(ui);
}


/* Process user input */
void BattleScreen::handleEvent(SDL_Event& e, bool& running, Panel& settingsPanel, Panel& mapMenuPanel, GameState& gameState) {
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
					rebuildDisplay(settingsPanel, mapMenuPanel);
					break;
				case ButtonOption::Tablet:
					ui.resizeWindow(WindowResType::Tablet);
					rebuildDisplay(settingsPanel, mapMenuPanel);
					break;
				case ButtonOption::Desktop:
					ui.resizeWindow(WindowResType::Desktop);
					rebuildDisplay(settingsPanel, mapMenuPanel);
					break;
				case ButtonOption::Fullscreen:
					ui.resizeWindow(WindowResType::Fullscreen);
					rebuildDisplay(settingsPanel, mapMenuPanel);
					break;
				case ButtonOption::Back:
					// switch to other panel
					settingsPanel.setShow(false);
					mapMenuPanel.setShow(true);
					break;
				case ButtonOption::Exit:
					/* back to menu screen */
					running = false;
					break;
				default:
					cout << "ERROR\n";
				}
			}
			else if (mapMenuPanel.getShow() && mapMenuPanel.isInPanel(mouseX, mouseY)) {
				cout << "\n\nCLICK MAP MENU \n\n";
				ButtonClickStruct clickStruct = mapMenuPanel.checkButtonClick(mouseX, mouseY);
				UI& ui = UI::getInstance();
				/* see what button might have been clicked : */
				switch (clickStruct.buttonOption) {
				case ButtonOption::MapOptions:
					cout << "\nMAP OPTIONS\n";
					break;
				case ButtonOption::Settings:
					settingsPanel.setShow(true);
					mapMenuPanel.setShow(false);
					break;
				default:
					cout << "ERROR\n";

				}
			}
		}
	}
}

void BattleScreen::checkMouseLocation(SDL_Event& e, Panel& settingsPanel, Panel& mapMenuPanel) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	/* send the x and y to the panel and its buttons to change the color */
	if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
	if (mapMenuPanel.getShow()) { mapMenuPanel.checkMouseOver(mouseX, mouseY); }
}