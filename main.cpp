/*
*	Land of Limbs
* 
* A puzzle/adventure/RPG game where you build a modular character from LIMBs.
*
*/

/*
* NEXT:
* - clicking button opens new screen
* 
*/


#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <vector>
#include <cstdlib>

using namespace std;

//import GameState;
import UI;
import GameState;
import MenuScreen;
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
	GameState& gameState = GameState::getInstance();

	// For now just get the MenuScreen. Later we'll make it dynamic for other screens.
	MenuScreen menuScreen = MenuScreen();

	ScreenToLoadStruct screenToLoadStruct = closingScreenStruct();
	bool running = true;
	SDL_Event e;

	/* Game loop checks gameState for screen to load, then loads that screen.
	* Each screen, once loaded, has its own game loop (where the real game logic happens).
	* When a screen is finished its job (or a user has chosen to move to a new screen),
	* the screen's "run" function returns a NEW screen to load.	*/
	while (running) {

		// Check for closing the app (clicking the (x) button)
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) { running = false; }
		}

		// run the chosen screen and receive the next screen to load
		// if the screenToLoad is Map or Battle, we will send in the ID to the Run function
		screenToLoadStruct =
			gameState.getScreenType() == ScreenType::Menu ? menuScreen.run() :
			closingScreenStruct();

		// check for closingScreen type and close. Otherwise we will load the NEW screen on the next loop
		if (screenToLoadStruct.screenType == ScreenType::NoScreen) { running = false; }

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
