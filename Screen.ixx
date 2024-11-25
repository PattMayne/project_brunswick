/* THERE IS NO ABSTRACT SCREEN. Each Screen object is a VARIATION of each other, but they will NEVER be used interchangably. There's no benefit to that.
* 
* 
*/

/*
* NEXT:
* -- Load background image
* --	bg image texture is SAVED.
* --	Now just print it in the drawMainMenu script
* -- Print title on top
* -- Make window size adjustable
* -- Make MenuScreen a child of Screen
* -- Make Buttons responsive (on hover)
* -- Make "New Game" open MAP screen
* -- Make "About" open a panel with text.
* -- Make "Settings" open a new menu. (will there really be settings? No... maybe delete!)
* -- Make "Load Game" load different Menu (unless there will only be one game?) (leave alone for now actually)
*/

module;

#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <time.h>
//#include <functional>

export module Screen;

using namespace std;

import ScreenType;
import UI;

// This will get its own module. Each screen needs its own module.
export class MapScreen {
	public:
		MapScreen(ScreenType incomingScreenType, MapType incomingMapType, int incomingId) {
			mapType = incomingMapType;
			screenType = incomingScreenType;
			id = incomingId;
			screenToLoadStruct.id = 0;
			screenToLoadStruct.screenType = ScreenType::Map;
		}

		ScreenType getScreenType() {
			return screenType;
		}

	protected:
		MapType mapType;
		ScreenType screenType;
		int id;
		ScreenToLoadStruct screenToLoadStruct;
};


export class MenuScreen {
	public:
		// constructor
		MenuScreen() {
			UI& ui = UI::getInstance();

			screenType = ScreenType::Menu;
			screenToLoadStruct.id = -1;
			screenToLoadStruct.screenType = ScreenType::NoScreen;

			// create background texture

			bgImageRaw = IMG_Load("assets/field.png");
			if (!bgImageRaw) {
				std::cerr << "Image failed to load. SDL_image: " << IMG_GetError() << std::endl;
			}
			getBackgroundTexture(ui);
		}

		ScreenToLoadStruct run() {
			cout << "Menu Screen loaded\n";
			// Get reference to UI singleton for the loop
			UI& ui = UI::getInstance();
			Panel menuPanel = ui.createMainMenuPanel();

			// Timeout data
			const int TARGET_FPS = 60;
			const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
			Uint32 frameStartTime; // Tick count when this particular frame began
			int frameTimeElapsed; // how much time has elapsed during this frame

			// loop and event control
			SDL_Event e;
			bool running = true;

			while (running) {
				// Get the total running time (tick count) at the beginning of the frame, for the frame timeout at the end
				frameStartTime = SDL_GetTicks();

				// Check for events in queue, and handle them (really just checking for X close now
				while (SDL_PollEvent(&e) != 0) {
					// User pressed X to close
					if (e.type == SDL_QUIT) { running = false; }

					// check event for mouse or keyboard action
					// These events might change the value of screenToLoad
				}

				draw(ui, menuPanel);

				// Delay so the app doesn't just crash
				frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
				// Delay loop
				if (frameTimeElapsed < FRAME_DELAY) {
					SDL_Delay(FRAME_DELAY - frameTimeElapsed);
				}
			}

			SDL_FreeSurface(bgImageRaw);

			// we return the information to load the appropriate screen.
			return screenToLoadStruct;
		}

		void setParentStruct(ScreenToLoadStruct incomingParentStruct) {
			screenToLoadStruct = incomingParentStruct;
		}

		ScreenToLoadStruct getParentStruct() { return screenToLoadStruct; }

		void draw(UI& ui, Panel& panel);
		void drawMainMenuBackground(UI& ui);
		void getBackgroundTexture(UI& ui);


	private:
		ScreenType screenType;
		ScreenToLoadStruct screenToLoadStruct;
		// bg image stuff
		SDL_Surface* bgImageRaw = NULL;
		SDL_Texture* bgTexture = NULL;
		SDL_Rect bgSourceRect;
		SDL_Rect bgDestinationRect;

};

/* Specific Draw functions for each Screen */

void MenuScreen::draw(UI& ui, Panel& panel) {
	// draw panel ( make this a function of the UI object which takes a panel as a parameter )
	//SDL_SetRenderDrawColor(ui.getMainRenderer(), 14, 14, 14, 1);
	SDL_RenderClear(ui.getMainRenderer());

	// print the BG image (just a section)
	SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);

	// button background color
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 95, 77, 227, 1);

	for (Button button: panel.getButtons()) {
		// get the rect, send it a reference (to be converted to a pointer)
		SDL_Rect rect = button.getRect();
		SDL_Rect textRect = button.getTextRect();
		SDL_RenderFillRect(ui.getMainRenderer(), &rect);

		// now draw the text
		SDL_RenderCopyEx(ui.getMainRenderer(), button.getTextTexture(), NULL, &textRect, 0, NULL, SDL_FLIP_NONE);
	}

	// Update window
	SDL_RenderPresent(ui.getMainRenderer());
}

void MenuScreen::drawMainMenuBackground(UI& ui) {
	// image texture is in the Screen object (we are in that scope)
	// window is in the ui
	// we need a source rect, showing which portion of the image to print.
}


void MenuScreen::getBackgroundTexture(UI& ui) {
	// This is a big texture of the whole image
	bgTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), bgImageRaw);

	// but I need a sourceRect, a destinationRect (whole window).
	// the destinationRect should ALWAYS have the height and width of the WINDOW.
	// That's easy to change. But if we're centering the image, we also need to recalculate the x and y positions on resize.

	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);

	// for now, don't resize or center. Just print the fucker.
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };

}
