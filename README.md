# Modular Action Puzzle Game

This is a **work in progress.** It's an action-puzzle game with modular characters made from limbs that snap together.

Currently I've implemented basic mechanics for the Map Screen and the Character Creation Screen. Some data is stored to the database. Once the database is more fully implemented, I'll make the Battle Screen.

Old code for the Java version is here: [Limbs Cyberpunk](https://github.com/PattMayne/LimbsCyberpunk)

### Technologies:

**Language:** C++, JSON, SQL

**Libraries:** SDL2, SDL_TTF, SDL_Image, SDL_Mixer, nlohmann (JSON), SQLite3
 
## TO DO:

- [x] TO DO list
- [x] Build battle system
    - [x] Green attacks.
    - [x] Blue attacks.
    - [x] Red attacks.
    - [x] Steal.
- [x] Build Battle Screen
    - [x] Load both character objects.
    - [x] Draw both character objects.
    - [x] Animate limbs.
- [ ] Switch to SDL3
- [ ] Terminology for game objects ("Suits" not sexy enough. Citizens v. monsters? Natives v. monsters? Map = world ? Scrambled v. unscrambled?)
- [x] Access Character Creation screen through Map screen button.
- [x] Access Map screen through Character Creation screen button.
- [x] Redraw Avatar on resize in Map.
- [x] Put DB access in dedicated function.
- [ ] Multiple (3) save files (DBs), option stored in GameState singleton.
- [ ] Finish main menu screen
  - [ ] New vs Continue
  - [ ] "ABOUT" opens an info box (passing message box... or turn Message buttons into prev/next options... or a new class for that)
- [x] Build MAP screen
- [ ] Finish MAP screen
    - [ ] HUD
        - [ ] Name of character
        - [ ] Avatar of character
        - [ ] Distance to chosen scrambled Limb
            - [ ] Requires a new Point member in MapScreen object.
    - [x] Character Creation Screen ("build") button.
    - [ ] Option to throw limbs? (Not in this game).
    - [ ] Put string messages in resource file.
    - [ ] High speed characters can move multiple spaces with mouse-click (or number followed by direction key).
        - [ ] Passing OVER each other means collision.
        - [ ] Works for Player, NPC, and Limb.
        - [ ] Mouseover block can click if in-range?
- [x] Build Character Creation Screen.
- [x] Finish Character Creation Screen.
    - [x] Show stats of character in panel (bottom right).
    - [x] Show stats of loaded limb in panel (top right).
    - [x] Continue button.
- [x] Represent some STATIC limbs
- [x] Incorporate LIMBS and AVATARS into MAP screen
- [ ] When closing program close each font with TTF_CloseFont()
- [ ] Error checks for all Texture & Surface generation.
- [x] incorporate SQLite database (#include <sqlite3.h>)
- [x] Make a Resources module to encapsulate string and int resources (JSON)

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


## Possible Sequel(s)

I was going to have story-quests, including buildings and friendly NPCs, as well as Dungeons (maps within the maps). But that's too ambitious for the first game, in my current position. Instead I'm going to focus on perfecting this game as a minimalist action-puzzle game, barely more than a card game on a map, but to make those basic features mind-blowing. If this game sells enough, then I'll work on a sequel which turns it into more of an RPG, and hire artists to make limbs, whole creatures and worlds custom-made for this game style.

A third game could expand on the RPG elements even further, and be isometric for enhanced weirdness and realism.

An online version would include limb-farms, allow artists to upload their own designs (or even launch their own federated instances) and feature true territorial warfare.