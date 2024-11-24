export module ScreenType;

export enum class ScreenType {
	Menu, Map, Battle, CharacterCreation
};

// This will be stored in the Map module, with a value held in each
export enum class MapType {
	World, Building, Dungeon
};

/* to avoid recursive self-referential class design, I'll make a Struct to hold information about parent Screens */
export struct ParentScreenStruct {
	ScreenType screenType;
	int id;
};