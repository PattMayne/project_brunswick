
/*

TABLES TO CREATE:

* LIMB
* JOINT
* LIMB_JOINT relational table
* CHARACTER
* MAP
* MAP BLOCK
* LANDMARK
* SUIT

START with LIMBS:



*/


/* LIMB and CHARACTER TABLES */

CREATE TABLE IF NOT EXISTS limb (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    form_slug TEXT NOT NULL,
    character_id INTEGER DEFAULT -1,
    map_slug TEXT,
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

CREATE INDEX IF NOT EXISTS idx_limb_id ON joint (limb_id);
CREATE INDEX IF NOT EXISTS idx_character_id ON limb (character_id);

/* MAP TABLES. */

CREATE TABLE IF NOT EXISTS map (
    slug PRIMARY KEY,
    character_x INTEGER DEFAULT 0,
    character_y INTEGER DEFAULT 0
);


CREATE TABLE IF NOT EXISTS block (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    map_slug TEXT NOT NULL,
    position_x INTEGER NOT NULL,
    position_y INTEGER NOT NULL,
    is_floor INTEGER DEFAULT 0,
    is_path INTEGER DEFAULT 0,
    is_looted INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS landmark (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    map_slug TEXT NOT NULL,
    landmark_type INTEGER NOT NULL,
    slug TEXT NOT NULL,
    position_x INTEGER NOT NULL,
    position_y INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_map_slug ON block (map_slug);
CREATE INDEX IF NOT EXISTS idx_position_y ON block (position_y);