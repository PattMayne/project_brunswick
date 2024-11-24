# Land of Limbs

 
## TO DO:

- [x] TO DO list
- [ ] BASIC STUFF:
    - [x] FONTS:
        - [x] Logo (decorative)
        - [x] UI (sans serif)
        - [x] Body (storytelling - serif / mono ?)
        - [x] dialog (script / handwriting)
    - [ ] Background for main menu screen
- [ ] Make main menu screen
  - [ ] UI module to process buttons and panels
    - [ ] Panels can be resized based on screen size.
    - [x] Buttons relative to panels.
    - [x] No buttons without panels! Every button is in a panel.
- [ ] Make other screens as DUMMY screens
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

## Long-Range Notes & Plans

* In the Character table, the Player Character is always #1
* A map has a type (in the DB). So when a Screen object loads a Map object, it creates a sub-type depending on the value of its type.