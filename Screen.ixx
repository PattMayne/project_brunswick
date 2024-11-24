/*
* Possibly make parent SCREEN a virtual or abstract class.
* Map, Menu, and Battle will all extend the class.
* 
* Apparently with virtual members we are supposed to use pointers to the derived class,
* instead of calling functions directly on the objects.
* 
* BUT FOR NOW just accept the ONE screen for the MENU.
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


/* to avoid recursive self-referential class design, I'll make a Struct to hold information about parent Screens */
export struct ParentStruct {
	ScreenType screenType;
	int id;
};

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

		void setParentStruct(ParentStruct incomingParentStruct) {
			parentStruct = incomingParentStruct;
		}

		ParentStruct getParentStruct() { return parentStruct; }


	protected:
		ScreenType screenType;
		int id;
		ParentStruct parentStruct;		
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


