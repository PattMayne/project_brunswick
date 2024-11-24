/*
* Game state object holds abstract information.
* Singleton.
*/

module;

#include<vector>
#include<cstdlib> // Needed for rand() and srand()
#include<ctime>   // Needed for time()
#include<iostream>
export module GameState;

using namespace std;

import ScreenType;

export class GameState {
	public:
		// Deleted copy constructor and assignment operator to prevent copies
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;

		// static method to get instance of the singleton
		static GameState& getInstance() {
			static GameState instance; // will be destroyed when program exits
			return instance;
		}

		ScreenType getScreenType() { return screenType; }
		void setScreenType(ScreenType incomingScreenType) { screenType = incomingScreenType; }

	private:
		// Constructor is private to prevent outside instantiation
		GameState() {
			cout << "GameState created\n";
			screenType = ScreenType::Menu;
		}

		// private destructor prevents deletion through a pointer to the base class
		~GameState() = default;

		ScreenType screenType;
};