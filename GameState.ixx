/*
* Game state (and stats) is held in a GameState object.
* Door objects hold information about the doors themselves (winner/dud ... open/closed ... chosen/not-chosen).
* GameState object operates on the doors.
*/

module;
export module GameState;

#include<vector>
#include<cstdlib> // Needed for rand() and srand()
#include<ctime>   // Needed for time()
#include<cstdlib>
#include<iostream>

using namespace std;
using std::vector;

// The game moves through three phases, initiated by user action.
// Game state affects display, user options, and how input is processed.
export enum class GamePhase {
	chooseDoor, chooseSwitch, gameOver
};

// Meta data about the "door." Does not include the images.
export class Door {

private:
	bool isWinner;
	bool isOpen;
	bool isChosen;

public:
	// constructor
	Door() {
		isWinner = false;
		isOpen = false;
		isChosen = false;
	}

	// setters

	void open() {
		isOpen = true;
	}

	void close() {
		isOpen = false;
	}

	void makeWinner() {
		isWinner = true;
	}

	void choose() {
		isChosen = true;
	}

	void unchoose() {
		isChosen = false;
	}

	// getters

	bool getWinner() {
		return isWinner;
	}

	bool getOpen() {
		return isOpen;
	}

	bool getChosen() {
		return isChosen;
	}
};

export class GameState {
private:
	vector<Door> doors;
	GamePhase gamePhase;

	// These stats can ONLY increment. They remain persistent across multiple games.
	// So they'll be encapsulated, with no option of decrement or resetting.
	int yesSwitchWins;
	int yesSwitchLosses;
	int noSwitchWins;
	int noSwitchLosses;

	// randomly choose one door to be the winner
	void chooseWinner() {
		doors[rand() % 3].makeWinner();
	}

	// returns a vector of indexes/indices of Door objects which are NOT winners
	vector<int> getLosingDoorIndices() {
		vector<int> losingDoorIndices;

		for (int i = 0; i < doors.size(); i++) {
			if (!doors[i].getWinner()) {
				losingDoorIndices.push_back(i);
			}
		}
		return losingDoorIndices;
	}

	// Open one losing door. If there is only one losing door among the unopened doors, open that one.
	// Otherwise, choose one at random.
	void openOneLosingDoor() {
		vector<int> losingDoorIndices = getLosingDoorIndices();

		// check if user chose winner on the first selection
		bool userChoseWinner = false;
		for (Door door : doors) {
			if (door.getChosen() && door.getWinner()) {
				userChoseWinner = true;
			}
		}
		// if user chose winner, randomly select among the losing doors to open one.
		// If user chose loser, simply open the OTHER losing door.
		if (userChoseWinner) {
			const int losingDoorIndexToOpen = rand() % losingDoorIndices.size();
			doors[losingDoorIndices[losingDoorIndexToOpen]].open();
		}
		else {
			// find the losing door that's NOT chosen, and open it
			if (!doors[losingDoorIndices[0]].getChosen()) {
				doors[losingDoorIndices[0]].open();
			}
			else {
				doors[losingDoorIndices[1]].open();
			}
		}
	}

	void unchooseAllDoors() {
		for (int i = 0; i < 3; ++i) {
			doors[i].unchoose();
		}
	}

	void closeAllDoors() {
		for (int i = 0; i < 3; ++i) {
			doors[i].close();
		}
	}

	void setupGame() {
		gamePhase = GamePhase::chooseDoor;
		doors = { Door(), Door(), Door() };
		chooseWinner();
	}

	// The "switchable door" is the non-opened, non-chosen door.
	// Only works during "chooseSwitch" phase
	int getSwitchableDoorIndex() {
		if (gamePhase == GamePhase::chooseSwitch) {
			for (int i = 0; i < doors.size(); ++i) {
				if (!doors[i].getChosen() && !doors[i].getOpen()) {
					return i;
				}
			}
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}

		std::cerr << "ERROR: NO adequate door index found";
		return -1;
	}

	// unchoose chosen door, choose "switchable" door.
	void switchDoors() {
		int switchableDoorIndex = getSwitchableDoorIndex();
		unchooseAllDoors();
		doors[switchableDoorIndex].choose();
	}

	// endgame functions

	// update stats

	void switchedAndWon() {
		if (gamePhase == GamePhase::chooseSwitch) {
			++yesSwitchWins;
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}
	}

	void switchedAndLost() {
		if (gamePhase == GamePhase::chooseSwitch) {
			++yesSwitchLosses;
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}
	}

	void heldAndWon() {
		if (gamePhase == GamePhase::chooseSwitch) {
			++noSwitchWins;
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}
	}

	void heldAndLost() {
		if (gamePhase == GamePhase::chooseSwitch) {
			++noSwitchLosses;
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}
	}

	// only open all doors if the game is over
	void openAllDoors() {
		if (gamePhase == GamePhase::gameOver) {
			for (int i = 0; i < doors.size(); ++i) {
				doors[i].open();
			}
		}
		else {
			std::cerr << "ERROR: wrong game phase";
		}
	}


public:
	vector<Door> getDoors() {
		return doors;
	}

	// constructor
	GameState() {
		srand(time(0)); // This guarantees a NEW random number each time the rand() program runs
		setupGame();

		// These don't change during the program's run.
		// So we can play the game multiple times and see the cumulative stats.
		yesSwitchWins = 0;
		yesSwitchLosses = 0;
		noSwitchWins = 0;
		noSwitchLosses = 0;
	}

	int getYesSwitchWins() {
		return yesSwitchWins;
	}

	int getYesSwitchLosses() {
		return yesSwitchLosses;
	}

	int getNoSwitchWins() {
		return noSwitchWins;
	}

	int getNoSwitchLosses() {
		return noSwitchLosses;
	}

	GamePhase getGamePhase() {
		return gamePhase;
	}

	// Not sure yet how we'll increment wins and losses.

	// Door state will be stored in the doors themselves

	int getWinningDoorIndex() {
		for (int i = 0; i < doors.size(); i++) {
			if (doors[i].getWinner()) {
				return i;
			}
		}
		return -1;
	}

	int getChosenDoorIndex() {
		for (int i = 0; i < doors.size(); i++) {
			if (doors[i].getChosen()) {
				return i;
			}
		}
		return -1;
	}

	// User input functions

	// Mark user's "chosen" door as such.
	void userChoosesDoor(int chosenDoorIndex) {
		// First make sure it will be the ONLY "chosen" door
		unchooseAllDoors();
		doors[chosenDoorIndex].choose();
		// open a random OTHER door
		openOneLosingDoor();
		// Game phase is over. Advance the game phase.
		gamePhase = GamePhase::chooseSwitch;
	}

	// User either clicked "YES" or "NO" to switching doors
	bool chooseSwitchAndEndGame(bool userSwitchedDoors) {
		if (userSwitchedDoors) {
			switchDoors();
		}

		// update stats

		bool userIsWinner = getWinningDoorIndex() == getChosenDoorIndex();

		if (userIsWinner) {
			if (userSwitchedDoors) {
				switchedAndWon();
			}
			else {
				heldAndWon();
			}
		}
		else {
			if (userSwitchedDoors) {
				switchedAndLost();
			}
			else {
				heldAndLost();
			}
		}

		// Advance game phase
		gamePhase = GamePhase::gameOver;
		// Display results
		openAllDoors();
		return userIsWinner;
	}

	void resetGame() {
		setupGame();
	}
};
