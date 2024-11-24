/*
*	Land of Limbs
* 
* A puzzle/adventure/RPG game where you build a modular character from LIMBs.
*
*/

/*
* NEXT:
* - refactor drawing panels, so that the UI object draws the panel (takes a panel as a parameter) (really? I'm not sure... have to think about this)
* - make Screen module
*	- enum of screen types
*	- 
* 
* DESGIN PATTERS TO USE:
*	- Singleton (for GameState and UI)
*	- Factory (for Screens, Panels, and Buttons) (nope, turned out to be a bad idea) (but maybe later for characters, etc)
*	- Observer (allow resizing of screens and propagating the size down to Panels and Buttons)
* 
*/


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

//import GameState;
import UI;
import GameState;
import Screen;
import ScreenType;
using namespace std;
const bool DEBUG = false;



// declarations
void exit(SDL_Surface* surface, SDL_Window* window);

// Main game loop manages particular screens, which do actual game logic.
int main(int argc, char* args[]) {
	cout << "Hello new world\n";

	// instantiate the UI instance, and hold the reference for eventual destruction in this file.
	UI& ui = UI::getInstance();

	// For now just get the MenuScreen. Later we'll make it dynamic for other screens.
	MenuScreen menuScreen = MenuScreen();

	bool running = true;
	SDL_Event e;

	// Game loop
	while (running)
	{

		// Check for closing the app (clicking the (x) button)
		while (SDL_PollEvent(&e) != 0)
		{
			// User pressed X to close
			if (e.type == SDL_QUIT)
			{
				running = false;
			}

			// run the chosen screen
			ParentScreenStruct parentScreenStruct = menuScreen.run();

			// check which screen to load next, or NONE.
			if (parentScreenStruct.id < 0) {
				running = false;
			}
		}

	}

	exit(ui.getWindowSurface(), ui.getMainWindow());
	return 0;
}

// Free SDL resources and quit
void exit(SDL_Surface* surface, SDL_Window* window)
{
	SDL_FreeSurface(surface);
	surface = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}
