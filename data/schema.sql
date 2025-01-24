
/*

TABLES TO CREATE:

* LIMB
* JOINT
* LIMB_JOINT relational table
* CHARACTER
* MAP

START with LIMBS:



*/


/* LIMB */

CREATE TABLE IF NOT EXISTS limb (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    form_slug TEXT NOT NULL,
    name TEXT,
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
    position_x INTEGER DEFAULT 0,
    position_y INTEGER DEFAULT 0,
    is_anchor INTEGER DEFAULT 0,
    conntected_limb_id INTEGER DEFAULT -1,
    anchor_joint_index INTEGER DEFAULT -1
);

CREATE TABLE IF NOT EXISTS limb_joint (    
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    limb_id INTEGER NOT NULL,
    joint_id INTEGER NOT NULL,
    FOREIGN KEY(limb_id) REFERENCES limb(id),
    FOREIGN KEY(joint_id) REFERENCES joint(id)
);

