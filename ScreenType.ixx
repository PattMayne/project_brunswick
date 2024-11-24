export module ScreenType;

export enum class ScreenType {
	NoScreen, Menu, Map, Battle, CharacterCreation
};

// This will be stored in the Map module, with a value held in each
export enum class MapType {
	World, Building, Dungeon
};

/* to avoid recursive self-referential class design, I'll make a Struct to hold information about parent Screens */
/* id refers to the id of the Map or Battle object in the database */
export struct ScreenToLoadStruct {
	ScreenType screenType;
	int id;
};

export ScreenToLoadStruct closingScreenStruct() {
	ScreenToLoadStruct closingScreenStruct;
	closingScreenStruct.screenType = ScreenType::NoScreen;
	closingScreenStruct.id = -1;
	return closingScreenStruct;
}