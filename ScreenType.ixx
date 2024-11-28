export module ScreenType;

export enum class ScreenType {
	NoScreen, Menu, Map, Battle, CharacterCreation
};

// This will be stored in the Map module, with a value held in each
export enum class MapType {
	World, Building, Dungeon
};


/* values to flag for resolutions. Mostly for development purposes. Final release should always be fullscreen. */
export enum class WindowResType {
	Mobile, Tablet, Desktop, Fullscreen
};

/* width and height data */
export struct Resolution {
	int w;
	int h;

	Resolution(int x, int y) {
		w = x;
		h = y;
	}
};


/* to avoid recursive self-referential class design, I'll make a Struct to hold information about parent Screens.
	id refers to the id of the Map or Battle object in the database */
export struct ScreenToLoadStruct {
	ScreenType screenType = ScreenType::NoScreen;
	int id = -1;
};


/* If this struct is returned to the main function, the program will shut down. */
export ScreenToLoadStruct closingScreenStruct() {
	ScreenToLoadStruct closingScreenStruct;
	closingScreenStruct.screenType = ScreenType::NoScreen;
	closingScreenStruct.id = -1;
	return closingScreenStruct;
}

/* PANEL BUTTON ENUMS
* Each panel has a list of buttons.
* Each button must store one of these enums, and will choose text independently of the screen.
* The screen checks which enum was triggered (when button clicked) and chooses function/action independently of the button.
* One big enum class will store all Button options.
*/

export enum class ButtonOption {
	NoOption,
	Back, /* for any submenu */
	MainMenu, /* quit back to main menu from any screen */
	/* Main Menu panel buttons */
	NewGame, LoadGame, Settings, About, Exit,
	/* Settings buttons */
	Mobile, Tablet, Desktop, Fullscreen,
	// TO COME: more options for Map, Battle, and Character Creation screens
};

/* When a button is clicked it must send back one of these Structs.
Sometimes the type of button clicked is enough.
Other times we need a 2nd piece of information,
such as the ID of the map to load or NPC to battle. */
export struct ButtonClickStruct {
	ButtonOption buttonOption;
	int itemID;

	// constructors
	ButtonClickStruct(ButtonOption incomingOption, int incomingID) {
		buttonOption = incomingOption;
		itemID = incomingID;
	}
	// blank
	ButtonClickStruct() {
		buttonOption = ButtonOption::NoOption;
		itemID = -1;
	}
};