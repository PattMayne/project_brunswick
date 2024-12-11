# Land of Limbs

This is a very early **work in progress.**

Currently I'm setting up the absolute basic UI framework.

I already worked out the basic game mechanics with Java for Android. I'll rewrite those behaviors for C++ for this desktop app.

Old code for the Java version is here: [Limbs Cyberpunk](https://github.com/PattMayne/LimbsCyberpunk)

### Technologies:

**Language:** C++

**Libraries:** SDL2, SDL_TTF, SDL_Image
 
## TO DO:

- [x] TO DO list
- [x] BASIC STUFF:
    - [x] FONTS:
        - [x] Logo (decorative)
        - [x] UI (sans serif)
        - [x] Body (storytelling - serif / mono ?)
        - [x] dialog (script / handwriting)
    - [x] Background for main menu screen
- [x] Make main menu screen
  - [x] UI module to process buttons and panels
    - [x] Buttons light up on-hover
    - [x] Panels can be resized based on screen size.
    - [x] Buttons relative to panels.
    - [x] No buttons without panels! Every button is in a panel.
- [x] Make other screens as DUMMY screens
  - [ ] Map screen
    - [ ] outside/main (world / non-safe by default, until solved) screen
    - [ ] inside (building / safe) screen
    - [ ] inside (dungeon / unsafe by default, until solved) screen
  - [ ] Character Creation screen
  - [ ] Battle screen
- [ ] Build actual MAP screen
  - [ ] Represent STATIC maps
    - [ ] Decide b/w auto-generated maps vs pre-designed maps
    - [ ] Map designer screen?
    - [ ] saves to SQLite? saves to JSON?
    - [ ] store maps in SQLite???? store in json file???
- [ ] Build Character Creation Screen
- [ ] Represent STATIC limbs (instantiated limb objects will be saved to the DB)
  - [ ] JSON?
- [ ] Incorporate LIMBS and AVATARS into MAP screen
- [ ] Build Battle Screen
- [ ] When closing program close each font with TTF_CloseFont()
- [ ] incorporate SQLite database
    - [ ] must download the sqlite3.h header file (#include <sqlite3.h>)
- [ ] MOBILE proportions are wack. Must unwack. Maybe this will be about SCALING.
    - [ ] Button size and Font size are the main issue. Button size is based on font size. So **font size should adjust automatically based on screen size.**
- [ ]  Possibly install conanfile or vcpkg to handle libraries (SDL2 or SDL3, json)
- [x] Make a Resources module to encapsulate string and int resources
    - [x] Make a JSON file
    - [x] Put all strings in JSON file
    - [x] Make a singleton for accessing the JSON file
        - [ ] Singleton will read gamestate (or something) to know which variations to access (window size affects font size, etc)

### Long-Range Notes & Plans

* A map has a type (in the DB). So when a Screen object loads a Map object, it creates a sub-type depending on the value of its type.
* Title Screen: have random limbs snapping onto each other and coming apart in an animation.
*   As one snaps on, another comes off and flies off-screen, while yet another flies on-screen to snap on... forever!


## How The Software Works

There is a main game loop (in main.cpp), and then each "screen" has its own game loop (which we will call "screen loops") nested in the "main loop".
The main loop checks the GameState, which holds information about which screen loop should run next. The screen loop is contained in a Screen object's "run" function. The screen loop will keep running until something tells it to quit (player action, or some event in the game). When the screen loop ends, the Screen's "run" function will also end, and will return information about which screen loop should run next.

The Map screen takes an id for which Map object to load from the database. The Map object contains a collection of Row objects, which each contain a collection of Block objects. The Row's index in its collection is the row number (y position in the grid of the 2D map). The Block object's index in its collection is the column number (x position in the 2D map). A Block can either be a Floor or a Wall. In previous versions of this game the map would be infinite, the Block objects were Floors by default, and I would randomly generate clusters of walls, whose perimeters would never meet. For this version of the Map, I will instead make Block objects *Wall* by default, and place landmarks (buildings, shrines, etc) at (possibly randomized) locations within a finite map, and procedurally/semi-randomly draw paths of Floor to connect them. Once generated, these Maps are saved to the database. So each one will be unique, but persistent once the player starts their game.

There will be World Maps (the main maps), Building Maps, and Dungeon Maps. Buildings and dungeons can be accessed within world maps.

The Battle screen will load the Player character and the NPC they encountered on the map. You beat your opponent by attacking their Limbs until they have no limbs left. (Actually, getting down to one limb may be a losing condition).

The Character Creation screen allows you to take Limbs from your inventory, snap them together into a character, and save that character to the databse. The image of your completed character will be saved as a SDL_Texture file (or something similar) for use in the Map screen.

### Characters & Limbs

A character (whether Player character or NPC) will have at least two limbs. There is no meta-data such as skill points or XP. Any data is contained in the limbs (HP, attack, defense, etc). But the Character is more than the sum of its parts. New abilities will arise based on the relationships between the Limb objects. [More details on this to follow]

Limb definitions will be hard-coded in some kind of properties file. I'm not sure about the format yet. Possibly JSON. One big master file with a list of Limb objects. More research is needed. Particular limbs will be saved to the database. The Limb table will need to store data about which attributes have changed (boosts or detriments). Images will be stored in a folder, or collection of folders, within the same directory as the Limb definitions.

### Winning the Game

Each world map will have "suits" which are native to that map. In each world, the suits have been scrambled and their Limbs scattered across the map. The player must collect one of each of their limbs and put them back together on a shrine/pedestal. Then order is restored and the player can move on to the next world map.