/*
* Basic UI elements.
* This should NOT be specific to any particular Screen.
* All Screen modules should use this UI module.
* The UI class will be a singleton which is shared between all Screen modules.
* 
* So a SCREEN can have PANELS, and PANELS can have BUTTONS (no panels outside of screens, no buttons outside of panels).
* Panel and Button classes will be exported separately from the UI object.
* 
* This module will also include functions to check if a screen's panel's buttons are being hovered over or clicked.
* -- OR -- the SCREEN class (different module?) will have those functions.
* That might seem odd, to have classes from one module affected by functions from a different class/module.
* 
* That's something to think about. Where do Panel and Button classes belong?
* 
* If this is really basic stuff that gets used everywhere, then yeah maybe Panel and Button should be in the Screen module.
* In fact, I don't even need to export Button or Panel.
* The Screen class will have functions to run through each Panel's Buttons.
* 
* There are some Panels which should be available to multiple Screens.
* 
* CLASSES TO CREATE:
* Panel
* Button
* 
*/


module;

export module UI;

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

using namespace std;

// Make this a SINGLETON
export class UI {
	private:

		const int SCREEN_WIDTH = 720;
		const int SCREEN_HEIGHT = 960;

		int initialized = false;

		SDL_Window* mainWindow = NULL;
		SDL_Renderer* mainRenderer = NULL;
		SDL_Surface* mainWindowSurface = NULL;

		TTF_Font* titleFont = NULL;
		TTF_Font* uiFont = NULL;
		TTF_Font* bodyFont = NULL;
		TTF_Font* dialogFont = NULL;

		bool initialize() {

			// Load main window, surface, and renderer
			mainWindow = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

			if (!mainWindow) {
				SDL_Log("Window failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Window failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

			if (!mainRenderer) {
				SDL_Log("Renderer failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Renderer failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			mainWindowSurface = SDL_GetWindowSurface(mainWindow);

			if (!mainWindowSurface) {
				SDL_Log("Window Surface failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Window Surface failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			// Load the fonts

			titleFont = TTF_OpenFont("assets/ander_hedge.ttf", 42);

			if (!titleFont) {
				SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			uiFont = TTF_OpenFont("assets/alte_haas_bold.ttf", 36);

			if (!uiFont) {
				SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			bodyFont = TTF_OpenFont("assets/alte_haas.ttf", 18);

			if (!bodyFont) {
				SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			dialogFont = TTF_OpenFont("assets/la_belle_aurore.ttf", 18);

			if (!dialogFont) {
				SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			return true;

		}

	public:
		//constructor
		UI() {
			initialized = initialize();
		}

		bool isInitialized() {
			return initialized;
		}

		SDL_Renderer* getMainRenderer() {
			return mainRenderer;
		}

		SDL_Surface* getWindowSurface() {
			return mainWindowSurface;
		}

		SDL_Window* getMainWindow() {
			return mainWindow;
		}
};


