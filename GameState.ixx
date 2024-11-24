/*
* Game state object holds abstract information
*/

module;


#include<vector>
#include<cstdlib> // Needed for rand() and srand()
#include<ctime>   // Needed for time()
#include<cstdlib>
#include<iostream>
export module GameState;

using namespace std;

import ScreenType;


export class GameState {
	public:
		// constructor
		GameState() {
			screenType = ScreenType::Menu;
		}
				

	private:
		ScreenType screenType;

};