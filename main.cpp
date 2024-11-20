/*
*	Land of Limbs
* 
* A puzzle/adventure/RPG game where you build a modular character from LIMBs.
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

const bool DEBUG = false;

using std::string;
using std::cout;
using std::cin;
using std::vector;
using std::to_string;


// GLOBAL CONSTANTS - many of these will be stored in the UI module

/*
* There will be different sub - map screens.
* These will inherit from the map class... or they will just be internal enums.
* This will be stored in the GameState module.
*/
enum class Screen {
	Menu, Map, Battle, CharacterCreation
};

// This will be stored in the Map module, with a value held in each
enum class MapType {
	World, Building, Dungeon
};

int main(int argc, char* args[]) {
	cout << "Hello new world";
	return 0;
}



