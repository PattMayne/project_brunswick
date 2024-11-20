/*
* Basic UI elements.
* This should NOT be specific to any particular Screen.
* All Screen modules should use this UI module.
* The UI class will be a singleton which is shared between all Screen modules.
* 
* So a SCREEN can have PANELS, and PANELS can have BUTTONS (no panels outside of screens, no buttons outside of panels).
* Panel and Button classes will be exported separately from the UI object.
* This module will also include functions to check if a screen's panel's buttons are being hovered over or clicked.
* 
* CLASSES TO CREATE:
* Panel
* Button
* 
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

		SDL_Window* mainWindow;
		SDL_Renderer* mainRenderer;
		SDL_Surface* mainWindowSurface;

		TTF_Font* titleFont;
		TTF_Font* uiFont;
		TTF_Font* bodyFont;
		TTF_Font* dialogFont;


};


