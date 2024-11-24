/*
* Possibly make parent SCREEN a virtual or abstract class.
* Map, Menu, and Battle will all extend the class.
* 
* Apparently with virtual members we are supposed to use pointers to the derived class,
* instead of calling functions directly on the objects.
* 
* BUT FOR NOW just accept the ONE screen for the MENU.
*/

/*
* NEXT:
* -- Load background image
* -- Print title on tope
* -- Make window size adjustable
* -- Make Screen class abstract
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
#include <vector>
//#include <functional>

export module Screen;

using namespace std;

import ScreenType;
import UI;

void draw(UI& ui, Panel& panel);

export class Screen {
	public:
		// constructor
		Screen(ScreenType incomingScreenType, int incomingId) {
			screenType = incomingScreenType;
			id = incomingId;
		}

		int run() {

			bool running = true;

			while (running) {
				// screen logic goes here
				running = false;
			}

			return 0;
		}


		void setParentStruct(ParentScreenStruct incomingParentStruct) {
			parentStruct = incomingParentStruct;
		}

		ParentScreenStruct getParentStruct() { return parentStruct; }


	protected:
		ScreenType screenType;
		int id;
		ParentScreenStruct parentStruct;		
};

export class MapScreen : Screen {
	public:
		MapScreen(ScreenType incomingScreenType, MapType incomingMapType, int incomingId) : Screen(incomingScreenType, incomingId) {
			mapType = incomingMapType;
		}

		ScreenType getScreenType() {
			return screenType;
		}

	protected:
		MapType mapType;
};


export class MenuScreen {
	public:
		// constructor
		MenuScreen() {
			screenType = ScreenType::Menu;
			parentStruct.id = -1;
			parentStruct.screenType = ScreenType::Menu;
		}

		ParentScreenStruct run() {
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
				while (SDL_PollEvent(&e) != 0)
				{
					// User pressed X to close
					if (e.type == SDL_QUIT)
					{
						running = false;
					}

					// check event for mouse or keyboard action
				}

				draw(ui, menuPanel);

				// Delay so the app doesn't just crash
				frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
				// Delay loop
				if (frameTimeElapsed < FRAME_DELAY) {
					SDL_Delay(FRAME_DELAY - frameTimeElapsed);
				}
			}

			// we return the information to load the appropriate screen.
			return parentStruct;
		}

		void setParentStruct(ParentScreenStruct incomingParentStruct) {
			parentStruct = incomingParentStruct;
		}

		ParentScreenStruct getParentStruct() { return parentStruct; }


	private:
		ScreenType screenType;
		ParentScreenStruct parentStruct;
};

// Currently the ONLY draw function...
// ... must re-work so that it's specifically for the MainMenu screen...
// ...overriding the abstract class Screen.
void draw(UI& ui, Panel& panel) {
	// draw panel ( make this a function of the UI object which takes a panel as a parameter )

	SDL_SetRenderDrawColor(ui.getMainRenderer(), 145, 145, 154, 1);
	SDL_RenderClear(ui.getMainRenderer());

	SDL_SetRenderDrawColor(ui.getMainRenderer(), 95, 77, 227, 1);

	vector<Button> buttons = panel.getButtons();

	for (int i = 0; i < buttons.size(); ++i) {
		// get the rect, send it a reference (to be converted to a pointer)
		SDL_Rect rect = buttons[i].getRect();
		SDL_Rect textRect = buttons[i].getTextRect();
		SDL_RenderFillRect(ui.getMainRenderer(), &rect);

		// now draw the text
		SDL_RenderCopyEx(ui.getMainRenderer(), buttons[i].getTextTexture(), NULL, &textRect, 0, NULL, SDL_FLIP_NONE);
	}

	// Update window
	SDL_RenderPresent(ui.getMainRenderer());
}
