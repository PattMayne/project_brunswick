
/*

TABLES TO CREATE:

* SUIT (native NPC) ... actually this will probably be a CharacterType instead. But we need to track whether it's been delivered or not.

*/


/* LIMB and CHARACTER TABLES */

CREATE TABLE IF NOT EXISTS limb (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    form_slug TEXT NOT NULL,
    character_id INTEGER DEFAULT -1,
    map_slug TEXT DEFAULT 'no_map',
    hp_mod INTEGER DEFAULT 0,
    strength_mod INTEGER DEFAULT 0,
    speed_mod INTEGER DEFAULT 0,
    intelligence_mod INTEGER DEFAULT 0,
    position_x INTEGER DEFAULT 0,
    position_y INTEGER DEFAULT 0,
    rotation_angle INTEGER DEFAULT 0,
    is_anchor INTEGER DEFAULT 0,
    is_flipped INTEGER DEFAULT 0,
    name TEXT DEFAULT 'Limb',
    draw_order INTEGER DEFAULT -1,
    is_unscrambled INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS joint (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    vector_index INTEGER NOT NULL,
    limb_id INTEGER NOT NULL,
    point_form_x INTEGER DEFAULT 0,
    point_form_y INTEGER DEFAULT 0,
    modified_point_x INTEGER DEFAULT 0,
    modified_point_y INTEGER DEFAULT 0,
    is_anchor INTEGER DEFAULT 0,
    connected_limb_id INTEGER DEFAULT -1,
    anchor_joint_index INTEGER DEFAULT -1
);

CREATE TABLE IF NOT EXISTS character (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    anchor_limb_id INTEGER DEFAULT -1,
    map_slug TEXT DEFAULT 'no_map',
    battle_id INTEGER NOT NULL DEFAULT -1,
    position_x INTEGER DEFAULT 0,
    position_y INTEGER DEFAULT 0,
    character_type INTEGER NOT NULL,
    suit_type INTEGER DEFAULT 0,
    latest_landmark_id INTEGER DEFAULT -1
);

CREATE TABLE IF NOT EXISTS battle (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    map_slug TEXT NOT NULL,
    player_id INTEGER NOT NULL,
    npc_id INTEGER NOT NULL,
    battle_status INTEGER NOT NULL
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
    is_landmark_area INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS landmark (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    map_slug TEXT NOT NULL,
    landmark_type INTEGER NOT NULL,
    position_x INTEGER NOT NULL,
    position_y INTEGER NOT NULL,
    character_id INTEGER DEFAULT -1,
    suit_type INTEGER DEFAULT -1
);

CREATE INDEX IF NOT EXISTS idx_map_slug ON block (map_slug);
CREATE INDEX IF NOT EXISTS idx_position_y ON block (position_y);