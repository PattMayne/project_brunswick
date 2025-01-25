
/*

TABLES TO CREATE:

* LIMB
* JOINT
* LIMB_JOINT relational table
* CHARACTER
* MAP
* MAP BLOCK

START with LIMBS:



*/


/* LIMB and CHARACTER TABLES */

CREATE TABLE IF NOT EXISTS limb (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    form_slug TEXT NOT NULL,
    name TEXT,
    character_id INTEGER DEFAULT -1,
    map_id INTEGER DEFAULT -1,
    hp_mod INTEGER DEFAULT 0,
    strength_mod INTEGER DEFAULT 0,
    speed_mod INTEGER DEFAULT 0,
    intelligence_mod INTEGER DEFAULT 0,
    position_x INTEGER DEFAULT 0,
    position_y INTEGER DEFAULT 0,
    rotation_angle INTEGER DEFAULT 0,
    is_anchor INTEGER DEFAULT 0,
    is_flipped INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS joint (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    limb_id INTEGER NOT NULL,
    position_x INTEGER DEFAULT 0,
    position_y INTEGER DEFAULT 0,
    is_anchor INTEGER DEFAULT 0,
    conntected_limb_id INTEGER DEFAULT -1,
    anchor_joint_index INTEGER DEFAULT -1
);

CREATE TABLE IF NOT EXISTS character (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    anchor_limb_id INTEGER DEFAULT -1,
    is_player INTEGER DEFAULT 0,
    map_slug TEXT
);

CREATE INDEX idx_limb_id ON joint (limb_id);
CREATE INDEX idx_character_id ON limb (character_id);

/* MAP TABLES. */

CREATE TABLE IF NOT EXISTS map (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    slug TEXT NOT NULL,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS row (
    id INTEGER PRIMARY KEY,
    map_slug TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS block (
    id INTEGER PRIMARY KEY,
    row_id INTEGER NOT NULL,
    map_slug TEXT NOT NULL,
    is_floor INTEGER DEFAULT 0,
    is_path INTEGER DEFAULT 0,
    is_looted INTEGER DEFAULT 0
);

CREATE INDEX idx_map_slug ON row (map_slug);
CREATE INDEX idx_row_id ON block (row_id);