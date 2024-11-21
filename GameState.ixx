/*
* Game state object holds abstract information.
* Singleton.
*/

module;

#include<vector>
#include<iostream>
export module GameState;

using namespace std;

import ScreenType;

export class GameState {
	public:
		/* Deleted copy constructor and assignment operator to prevent copies */
		GameState(const GameState&) = delete;
		GameState& operator=(const GameState&) = delete;

		/* static method to get instance of the singleton */
		static GameState& getInstance() {
			static GameState instance; /* will be destroyed when program exits */
			return instance;
		}

		/* getters */
		ScreenStruct getScreenStruct() { return screenStruct; }
		ScreenType getScreenType() { return screenStruct.screenType; }
		int getScreenId() { return screenStruct.id;  }

		/* setters */
		void setScreenStruct(ScreenStruct incomingStruct) { screenStruct = incomingStruct; }
		void setScreenId(int incomingId) { screenStruct.id = incomingId; }
		void setScreenType(ScreenType incomingScreenType) { screenStruct.screenType = incomingScreenType; }

	private:
		/* Constructor is private to prevent outside instantiation */
		GameState() {
			cout << "GameState created\n";
			screenStruct.screenType = ScreenType::Menu;
			screenStruct.id = -1;
		}

		/* private destructor prevents deletion through a pointer to the base class */
		~GameState() = default;

		/* current screen data (main.cpp reads this for the screen to load) */
		ScreenStruct screenStruct;
};