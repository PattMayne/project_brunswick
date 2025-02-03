/**
*  ____        _        _
* |  _ \  __ _| |_ __ _| |__   __ _ ___  ___
* | | | |/ _` | __/ _` | '_ \ / _` / __|/ _ \
* | |_| | (_| | || (_| | |_) | (_| \__ \  __/
* |____/ \__,_|\__\__,_|_.__/ \__,_|___/\___|
* 
* 
* The modules for objects which are saved to the database do not import the database themselves.
* Instead, the screen modules which load those classes also load the database module.
* The database module also loads all the modules whose classes must be saved to the database.
* So Limb does not have a save() function.
* Instead, the database will have a save() function for each class,
* and the screen modules can call THAT function when needed.
* The same process applies for getting data from the database,
* and using that data to create objects.
*/


module;
export module Database;

import <vector>;
import <cstdlib>;
import <iostream>;
import <unordered_map>;
import <fstream>;
import <string>;
#include "sqlite3.h"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"

import CharacterClasses;
import MapClasses;
import TypeStorage;
import LimbFormMasterList;
import FormFactory;
import GameState;
import UI;

using namespace std;

const char* dbPath() { return "data/databases/limbs_data.db"; }

export string stringFromUnsignedChar(const unsigned char* c_str) {
    std::string str;
    int i{ 0 };
    while (c_str[i] != '\0') { str += c_str[i++]; }
    return str;
}

/* Creates the database and tables if they don't exist. */
export void createDB() {
    sqlite3* db;

    /* Open database (create if does not exist). */
    int dbFailed = sqlite3_open(dbPath(), &db);
    cout << dbFailed << "\n";
    if (dbFailed == 0) {
        cout << "Opened Database Successfully." << "\n";

        /* Get the schema file. */
        string schemaFilename = "data/schema.sql";
        ifstream schemaFile(schemaFilename);

        if (schemaFile.is_open()) {
            cout << "Opened schema file.\n";

            /* Get the SQL string from the open file. */
            string sql((istreambuf_iterator<char>(schemaFile)), istreambuf_iterator<char>());
            char* errMsg = nullptr;
            int returnCode = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);

            if (returnCode == SQLITE_OK) {  cout << "Schema executed successfully." << endl; }
            else {
                cerr << "SQL error: " << errMsg << endl;
                sqlite3_free(errMsg); }
        }
        else {
            cerr << "Could not open file: " << schemaFilename << endl;
            return;
        }
    }
    else { cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl; }

    /* Close database. */
    sqlite3_close(db);
}

/**
* 
* 
* 
*  _     _           _           ____      _       _           _
* | |   (_)_ __ ___ | |__       |  _ \ ___| | __ _| |_ ___  __| |
* | |   | | '_ ` _ \| '_ \ _____| |_) / _ \ |/ _` | __/ _ \/ _` |
* | |___| | | | | | | |_) |_____|  _ <  __/ | (_| | ||  __/ (_| |
* |_____|_|_| |_| |_|_.__/   _  |_| \_\___|_|\__,_|\__\___|\__,_|
* |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
* | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
* |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
* |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
* 
* 
* Limb-Related Functions
* 
*/


/* When a limb changes owner. */
export bool updateLimbOwner(int limbID, int newCharacterID) {
    bool success = false;
    sqlite3* db;

    /* Open database. */
    int dbFailed = sqlite3_open(dbPath(), &db);
    cout << dbFailed << "\n";
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return success;
    }

    /* No need to change the map_slug because map only loads non-owned limbs. */
    const char* updateSQL = "UPDATE limb SET character_id = ? WHERE id = ?;";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, updateSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return success;
    }

    /* Bind the values. */
    sqlite3_bind_int(statement, 1, newCharacterID);
    sqlite3_bind_int(statement, 2, limbID);

    cout << "Updated Limb ID: " << limbID << "\n";

    /* Execute the statement. */
    returnCode = sqlite3_step(statement);
    if (returnCode != SQLITE_DONE) { cerr << "Update failed: " << sqlite3_errmsg(db) << endl; }
    else { success = true; }

    /* Finalize statement and close database. */
    sqlite3_finalize(statement);
    sqlite3_close(db);

    return success;
}

/*
* When the list of roamingLimbs in the map have all moved to a new location,
* send them here and update the database.
*/
export void updateLimbsLocation(vector<Limb>& limbs) {
    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return;
    }

    const char* updateSQL = "UPDATE limb SET position_x = ?, position_y = ? WHERE id = ?;";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, updateSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB POSITION UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    /* Begin the transaction. */
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);

    for (int i = 0; i < limbs.size(); ++i) {
        Point position = limbs[i].getPosition();
        sqlite3_bind_int(statement, 1, position.x);
        sqlite3_bind_int(statement, 2, position.y);
        sqlite3_bind_int(statement, 3, limbs[i].getId());

        if (sqlite3_step(statement) != SQLITE_DONE) {
            cerr << "Update failed for LIMB: " << sqlite3_errmsg(db) << endl;
            break;
        }

        sqlite3_reset(statement);
    }

    /* Finalize the statement, commit the transaction. */
    sqlite3_finalize(statement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    sqlite3_close(db);
}

/**
* 
* 
*   ____ _                          _
*  / ___| |__   __ _ _ __ __ _  ___| |_ ___ _ __
* | |   | '_ \ / _` | '__/ _` |/ __| __/ _ \ '__|____
* | |___| | | | (_| | | | (_| | (__| ||  __/ | |_____|
*  \____|_| |_|\__,_|_|  \__,_|\___|\__\___|_|
* |  _ \ ___| | __ _| |_ ___  __| |
* | |_) / _ \ |/ _` | __/ _ \/ _` |
* |  _ <  __/ | (_| | ||  __/ (_| |
* |_|_\_\___|_|\__,_|\__\___|\__,_|
* |  ___|   _ _ __   ___| |_(_) ___  _ __  ___
* | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
* |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
* |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
* 
* 
*/

/*
* Update all attributes of a character's limbs and their joints.
* To be used when SAVING limb setup in Character Creation Screen.
*/
export void updateCharacterLimbs(int characterId, int anchorLimbId, vector<Limb>& limbs) {
    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return;
    }


    /* First update the Character. */

    const char* updateCharacterSQL = "UPDATE character SET anchor_limb_id = ? WHERE id = ?;";
    sqlite3_stmt* updateCharacterstatement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, updateCharacterSQL, -1, &updateCharacterstatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare CHARACTER UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    /* Bind the values. */
    sqlite3_bind_int(updateCharacterstatement, 1, anchorLimbId);
    sqlite3_bind_int(updateCharacterstatement, 2, characterId);

    /* Execute the statement. */
    returnCode = sqlite3_step(updateCharacterstatement);
    if (returnCode != SQLITE_DONE) { cerr << "Update failed: " << sqlite3_errmsg(db) << endl; }

    /* Finalize statement. */
    sqlite3_finalize(updateCharacterstatement);


    /* 
    * 
    * Now do a transaction and update all limbs and their joints.
    * 
    */

    const char* updateLimbSQL = "UPDATE limb SET map_slug = ?, hp_mod = ?, strength_mod = ?, "
        "speed_mod = ?, intelligence_mod = ?, position_x = ?, position_y = ?, rotation_angle = ?, is_anchor = ?, "
        "is_flipped = ? WHERE character_id = ?;";
    sqlite3_stmt* updateLimbStatement;

    /* Prepare the LIMB statement (to be binded and reset for each Limb). */
    returnCode = sqlite3_prepare_v2(db, updateLimbSQL, -1, &updateLimbStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    const char* updateJointSQL = "UPDATE joint SET vector_index = ?, modified_point_x = ?, modified_point_y = ?, "
        "is_anchor = ?, connected_limb_id = ?, anchor_joint_index = ? WHERE limb_id = ?;";
    sqlite3_stmt* updateJointStatement;

    /* Prepare the JOINT statement (to be binded and reset for each joint of each Limb). */
    returnCode = sqlite3_prepare_v2(db, updateJointSQL, -1, &updateJointStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }


    /* Begin the transaction. */
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);

    for (Limb& limb : limbs) {
        /* Bind the values for each limb. */
        int isAnchorInt = limb.getIsAnchor() ? 1 : 0;
        int isFlippedInt = limb.getIsFlipped() ? 1 : 0;
        sqlite3_bind_text(updateLimbStatement, 1, limb.getMapSlug().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(updateLimbStatement, 2, limb.getHpMod());
        sqlite3_bind_int(updateLimbStatement, 3, limb.getStrengthMod());
        sqlite3_bind_int(updateLimbStatement, 4, limb.getSpeedMod());
        sqlite3_bind_int(updateLimbStatement, 5, limb.getIntelligenceMod());
        sqlite3_bind_int(updateLimbStatement, 6, limb.getPosition().x);
        sqlite3_bind_int(updateLimbStatement, 7, limb.getPosition().y);
        sqlite3_bind_int(updateLimbStatement, 8, limb.getRotationAngle());
        sqlite3_bind_int(updateLimbStatement, 9, isAnchorInt);
        sqlite3_bind_int(updateLimbStatement, 10, isFlippedInt);
        sqlite3_bind_int(updateLimbStatement, 11, characterId);

        /* Execute the statement. */
        returnCode = sqlite3_step(updateLimbStatement);
        if (returnCode != SQLITE_DONE) { cerr << "Update Limb failed: " << sqlite3_errmsg(db) << endl; }

        /* NOW update each JOINT for this LIMB. */

        for (int i = 0; i < limb.getJoints().size(); ++i) {
            Joint& joint = limb.getJoints()[i];
            Point modifiedPoint = joint.getPoint();
            int isAnchorInt = joint.getIsAnchor() ? 1 : 0;

            sqlite3_bind_int(updateJointStatement, 1, i);
            sqlite3_bind_int(updateJointStatement, 2, modifiedPoint.x);
            sqlite3_bind_int(updateJointStatement, 3, modifiedPoint.y);
            sqlite3_bind_int(updateJointStatement, 4, isAnchorInt);
            sqlite3_bind_int(updateJointStatement, 5, joint.getConnectedLimbId());
            sqlite3_bind_int(updateJointStatement, 6, joint.getChildLimbAnchorJointIndex());
            sqlite3_bind_int(updateJointStatement, 7, limb.getId());

            /* Execute the statement. */
            returnCode = sqlite3_step(updateJointStatement);
            if (returnCode != SQLITE_DONE) { cerr << "Update Joint failed: " << sqlite3_errmsg(db) << endl; }

            /* Reset the JOINT statement for the next joint. */
            sqlite3_reset(updateJointStatement);
        }

        /* Reset the LIMB statement for the next limb. */
        sqlite3_reset(updateLimbStatement);
    }

    sqlite3_finalize(updateLimbStatement);
    sqlite3_finalize(updateJointStatement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);

    sqlite3_close(db);
}


/* Update the MAP_SLUG of the Character player. */
export bool updatePlayerMap(string newMapSlug) {
    bool success = false;
    GameState& gameState = GameState::getInstance();
    int playerID = gameState.getPlayerID();

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return success;
    }

    const char* updateSQL = "UPDATE character SET map_slug = ? WHERE id = ?;";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, updateSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare CHARACTER MAP_SLUG UPDATE statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return success;
    }

    /* Bind the values. */
    sqlite3_bind_text(statement, 1, newMapSlug.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, playerID);

    /* Execute the statement. */
    returnCode = sqlite3_step(statement);
    if (returnCode != SQLITE_DONE) { cerr << "Update failed: " << sqlite3_errmsg(db) << endl; }
    else { success = true; }

    /* Finalize statement and close database. */
    sqlite3_finalize(statement);
    sqlite3_close(db);

    return success;
}

/* 
* Same as loadPlayerMapCharacter but without map-related stuff.
* Should extract most of this to make a struct that serves them both!
* May use same system to create loadPlayerBattleCharacter too.
*/
export Character loadPlayerCharacter() {
    GameState& gameState = GameState::getInstance();
    int characterID = gameState.getPlayerID();
    string name;
    string mapSlug;
    int anchorLimbID; /* Get from the limbs. */
    int battleID; /* For later, when battles actually exist. */
    bool isPlayer = true;
    Character character = Character(CharacterType::None);

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return character;
    }


    /* GET THE CHARACTER OBJECT. */

    /* Create statement template for getting the character. */
    const char* queryCharacterSQL = "SELECT * FROM character WHERE id = ?;";
    sqlite3_stmt* characterStatement;
    int returnCode = sqlite3_prepare_v2(db, queryCharacterSQL, -1, &characterStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare character retrieval statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return character;
    }

    /* Bind the id value. */
    sqlite3_bind_int(characterStatement, 1, characterID);

    /* Execute binded statement. */
    if (sqlite3_step(characterStatement) != SQLITE_ROW) {
        cerr << "Failed to retrieve MAP: " << sqlite3_errmsg(db) << endl;
        return character;
    }

    name = stringFromUnsignedChar(sqlite3_column_text(characterStatement, 1));
    anchorLimbID = sqlite3_column_int(characterStatement, 2);
    mapSlug = stringFromUnsignedChar(sqlite3_column_text(characterStatement, 4));
    battleID = sqlite3_column_int(characterStatement, 5);

    /* Finalize statement. */
    sqlite3_finalize(characterStatement);


    /* GET THE LIMBS. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryLimbsSQL = "SELECT * FROM limb WHERE character_id = ?;";
    sqlite3_stmt* queryLimbsStatement;
    returnCode = sqlite3_prepare_v2(db, queryLimbsSQL, -1, &queryLimbsStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare limbs retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return character;
    }

    /* Bind the slug value. */
    sqlite3_bind_int(queryLimbsStatement, 1, characterID);

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(queryLimbsStatement)) == SQLITE_ROW) {

        int limbID = sqlite3_column_int(queryLimbsStatement, 0);
        LimbForm limbForm = getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1)));
        string mapSlug = stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 3));
        int hpMod = sqlite3_column_int(queryLimbsStatement, 4);
        int strengthMod = sqlite3_column_int(queryLimbsStatement, 5);
        int speedMod = sqlite3_column_int(queryLimbsStatement, 6);
        int intelligenceMod = sqlite3_column_int(queryLimbsStatement, 7);
        int posX = sqlite3_column_int(queryLimbsStatement, 8);
        int posY = sqlite3_column_int(queryLimbsStatement, 9);
        int rotationAngle = sqlite3_column_int(queryLimbsStatement, 10);
        bool isAnchor = sqlite3_column_int(queryLimbsStatement, 11) == 1;
        bool isFlipped = sqlite3_column_int(queryLimbsStatement, 12) == 1;
        string limbName = stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 13));

        Point position = Point(posX, posY);


        /* Get the JOINTS for this Limb. */
        /* Start by querying the count to see how big the vector should be. */
        const char* queryCountJointsSQL = "SELECT COUNT(*) FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryCountJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryCountJointsSQL, -1, &queryCountJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare JOINTS retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return character;
        }

        sqlite3_bind_int(queryCountJointsStatement, 1, limbID);

        /* Execute count query. */
        int jointCount = 0;
        if (sqlite3_step(queryCountJointsStatement) == SQLITE_ROW) {
            jointCount = sqlite3_column_int(queryCountJointsStatement, 0);
        }
        sqlite3_finalize(queryCountJointsStatement);

        vector<Joint> joints(jointCount);


        /* Now get the JOINTS themselves. */
        const char* queryJointsSQL = "SELECT * FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryJointsSQL, -1, &queryJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare blocks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return character;
        }

        /* Bind the ID value. */
        sqlite3_bind_int(queryJointsStatement, 1, limbID);
        int jointsReturnCode;

        while ((jointsReturnCode = sqlite3_step(queryJointsStatement)) == SQLITE_ROW) {
            int jointID = sqlite3_column_int(queryJointsStatement, 0);
            int vectorIndex = sqlite3_column_int(queryJointsStatement, 1);
            Point pointForm = Point(
                sqlite3_column_int(queryJointsStatement, 3),
                sqlite3_column_int(queryJointsStatement, 4));
            Point modifiedPoint = Point(
                sqlite3_column_int(queryJointsStatement, 5),
                sqlite3_column_int(queryJointsStatement, 6));
            bool isAnchor = sqlite3_column_int(queryJointsStatement, 7) == 1;
            int connectedLimbID = sqlite3_column_int(queryJointsStatement, 8);
            int anchorJointIndex = sqlite3_column_int(queryJointsStatement, 9);
            int rotationAngle = sqlite3_column_int(queryJointsStatement, 10);

            Joint joint = Joint(pointForm, modifiedPoint, isAnchor, connectedLimbID, anchorJointIndex, rotationAngle, jointID);
            if (vectorIndex < jointCount) { joints[vectorIndex] = joint; }
        }

        Limb limb = Limb(sqlite3_column_int(queryLimbsStatement, 0),
            getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1))),
            position,
            joints
        );

        limb.setName(limbName);
        limb.modifyHP(hpMod);
        limb.modifyStrength(strengthMod);
        limb.modifySpeed(speedMod);
        limb.modifyIntelligence(intelligenceMod);
        limb.rotate(rotationAngle);
        limb.setFlipped(isFlipped);
        limb.setAnchor(isAnchor);
        limb.setMapSlug(mapSlug);
        limb.setCharacterId(characterID);
        limb.setId(limbID);

        character.addLimb(limb);
        cout << "added limb " << limbName << "\n";
    }

    cout << "Player has " << character.getLimbs().size() << " limbs (in DATABASE module)\n";

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
        return character;
    }

    /* Finalize prepared statement. */
    sqlite3_finalize(queryLimbsStatement);

    character.setType(CharacterType::Player);
    character.setName(name);
    character.setId(characterID);
    character.setAnchorLimbId(anchorLimbID);

    cout << "Player has " << character.getLimbs().size() << " limbs in DB module\n";

    return character;
}

/* Same as loadPlayerCharacter, but adding the map-related stuff. */
export MapCharacter loadPlayerMapCharacter() {
    GameState& gameState = GameState::getInstance();
    int characterID = gameState.getPlayerID();
    string name;
    string mapSlug;
    int anchorLimbID; /* Get from the limbs. */
    int battleID; /* For later, when battles actually exist. */
    bool isPlayer = true;
    MapCharacter character = MapCharacter(CharacterType::None);

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return character;
    }


    /* GET THE CHARACTER OBJECT. */

    /* Create statement template for getting the character. */
    const char* queryCharacterSQL = "SELECT * FROM character WHERE id = ?;";
    sqlite3_stmt* characterStatement;
    int returnCode = sqlite3_prepare_v2(db, queryCharacterSQL, -1, &characterStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare character retrieval statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return character;
    }

    /* Bind the id value. */
    sqlite3_bind_int(characterStatement, 1, characterID);

    /* Execute binded statement. */
    if (sqlite3_step(characterStatement) != SQLITE_ROW) {
        cerr << "Failed to retrieve MAP: " << sqlite3_errmsg(db) << endl;
        return character;
    }

    name = stringFromUnsignedChar(sqlite3_column_text(characterStatement, 1));
    anchorLimbID = sqlite3_column_int(characterStatement, 2);
    mapSlug = stringFromUnsignedChar(sqlite3_column_text(characterStatement, 4));
    battleID = sqlite3_column_int(characterStatement, 5);

    /* Finalize statement. */
    sqlite3_finalize(characterStatement);


    /* GET THE LIMBS. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryLimbsSQL = "SELECT * FROM limb WHERE character_id = ?;";
    sqlite3_stmt* queryLimbsStatement;
    returnCode = sqlite3_prepare_v2(db, queryLimbsSQL, -1, &queryLimbsStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare limbs retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return character;
    }

    /* Bind the slug value. */
    sqlite3_bind_int(queryLimbsStatement, 1, characterID);

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(queryLimbsStatement)) == SQLITE_ROW) {

        int limbID = sqlite3_column_int(queryLimbsStatement, 0);
        LimbForm limbForm = getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1)));
        string mapSlug = stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 3));
        int hpMod = sqlite3_column_int(queryLimbsStatement, 4);
        int strengthMod = sqlite3_column_int(queryLimbsStatement, 5);
        int speedMod = sqlite3_column_int(queryLimbsStatement, 6);
        int intelligenceMod = sqlite3_column_int(queryLimbsStatement, 7);
        int posX = sqlite3_column_int(queryLimbsStatement, 8);
        int posY = sqlite3_column_int(queryLimbsStatement, 9);
        int rotationAngle = sqlite3_column_int(queryLimbsStatement, 10);
        bool isAnchor = sqlite3_column_int(queryLimbsStatement, 11) == 1;
        bool isFlipped = sqlite3_column_int(queryLimbsStatement, 12) == 1;
        string limbName = stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 13));

        Point limbPosition = Point(posX, posY);

        /* Get the JOINTS for this Limb. */
        /* Start by querying the count to see how big the vector should be. */
        const char* queryCountJointsSQL = "SELECT COUNT(*) FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryCountJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryCountJointsSQL, -1, &queryCountJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare JOINTS retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return character;
        }

        sqlite3_bind_int(queryCountJointsStatement, 1, limbID);

        /* Execute count query. */
        int jointCount = 0;
        if (sqlite3_step(queryCountJointsStatement) == SQLITE_ROW) {
            jointCount = sqlite3_column_int(queryCountJointsStatement, 0);
        }
        sqlite3_finalize(queryCountJointsStatement);

        vector<Joint> joints(jointCount);


        /* Now get the JOINTS themselves. */
        const char* queryJointsSQL = "SELECT * FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryJointsSQL, -1, &queryJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare blocks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return character;
        }

        /* Bind the ID value. */
        sqlite3_bind_int(queryJointsStatement, 1, limbID);
        int jointsReturnCode;

        while ((jointsReturnCode = sqlite3_step(queryJointsStatement)) == SQLITE_ROW) {
            int jointID = sqlite3_column_int(queryJointsStatement, 0);
            int vectorIndex = sqlite3_column_int(queryJointsStatement, 1);
            Point pointForm = Point(
                sqlite3_column_int(queryJointsStatement, 3),
                sqlite3_column_int(queryJointsStatement, 4));
            Point modifiedPoint = Point(
                sqlite3_column_int(queryJointsStatement, 5),
                sqlite3_column_int(queryJointsStatement, 6));
            bool isAnchor = sqlite3_column_int(queryJointsStatement, 7) == 1;
            int connectedLimbID = sqlite3_column_int(queryJointsStatement, 8);
            int anchorJointIndex = sqlite3_column_int(queryJointsStatement, 9);
            int rotationAngle = sqlite3_column_int(queryJointsStatement, 10);

            Joint joint = Joint(pointForm, modifiedPoint, isAnchor, connectedLimbID, anchorJointIndex, rotationAngle, jointID);
            if (vectorIndex < jointCount) { joints[vectorIndex] = joint; }
        }


        Limb limb = Limb(sqlite3_column_int(queryLimbsStatement, 0),
            getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1))),
            limbPosition,
            joints
        );

        limb.setName(limbName);
        limb.modifyHP(hpMod);
        limb.modifyStrength(strengthMod);
        limb.modifySpeed(speedMod);
        limb.modifyIntelligence(intelligenceMod);
        limb.rotate(rotationAngle);
        limb.setFlipped(isFlipped);
        limb.setAnchor(isAnchor);
        limb.setMapSlug(mapSlug);
        limb.setCharacterId(characterID);
        limb.setId(limbID);

        character.addLimb(limb);
        cout << "added limb " << limbName << "\n";
    }

    cout << "Player has " << character.getLimbs().size() << " limbs (in DATABASE module)\n";

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
        return character;
    }

    /* Finalize prepared statement. */
    sqlite3_finalize(queryLimbsStatement);

    character.setType(CharacterType::Player);
    character.setName(name);
    character.setId(characterID);
    character.setAnchorLimbId(anchorLimbID);




    /*
    *  NOW get the blockPosition from the Map table.
    * ( also, temporarily get the texture... will replace with uilding from limbs of course. )
    *
    * THE ABOVE CODE was all normal Character code.
    * That code must be put into a struct which can be handed to any derivative class,
    * to avoid repeating code.
    * THAT FUNCTION will take a db object as a parameter.
    */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryMapSQL = "SELECT character_x, character_y FROM map WHERE slug = ?;";
    sqlite3_stmt* statement;
    returnCode = sqlite3_prepare_v2(db, queryMapSQL, -1, &statement, nullptr);

    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare map retrieval statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return character;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(statement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute binded statement. */
    if (sqlite3_step(statement) != SQLITE_ROW) {
        cerr << "Failed to retrieve MAP: " << sqlite3_errmsg(db) << endl;
        return character;
    }

    Point characterPosition = Point(
        sqlite3_column_int(statement, 0),
        sqlite3_column_int(statement, 1)
    );

    /* Finally actually set the block position. */
    character.setBlockPosition(characterPosition);

    /* Finalize map ID retrieval statement. */
    sqlite3_finalize(statement);
    sqlite3_close(db);
    cout << "We really reached the end of the character loading, so what's wrong?\n";

    cout << "Player has " << character.getLimbs().size() << " limbs in DB module\n";


    /* get character texture (MUST DELETE AFTER WE START DRAWING RAW LIMBS ONTO THE MAP INSTEAD.) */
    UI& ui = UI::getInstance();
    SDL_Surface* characterSurface = IMG_Load("assets/player_character.png");
    SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(ui.getMainRenderer(), characterSurface);
    SDL_FreeSurface(characterSurface);
    character.setTexture(characterTexture);

    return character;
}


export int createPlayerCharacterOrGetID() {

    /*
    * First check if player character already exists.
    * If it does, get the ID and return the ID.
    * If it does not exist, create the player character and return the ID.
    */

    int count = 0;
    int playerID = -1;

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    /* Create statement template for querying the count. */
    const char* queryCountSQL = "SELECT COUNT(*) FROM character WHERE is_player = 1;";
    sqlite3_stmt* statement;
    int returnCode = sqlite3_prepare_v2(db, queryCountSQL, -1, &statement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    /* Execute statement. */
    if (sqlite3_step(statement) == SQLITE_ROW) {
        count = sqlite3_column_int(statement, 0);
    }

    /* Finalize statement. */
    sqlite3_finalize(statement);

    if (count > 0) {
        /* Player character exists. Get the ID. */

        /* Create statement template for querying the id. */
        const char* queryIDSQL = "SELECT id FROM character WHERE is_player = 1;";
        sqlite3_stmt* idStatement;
        returnCode = sqlite3_prepare_v2(db, queryIDSQL, -1, &idStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }

        /* Execute statement. */
        if (sqlite3_step(idStatement) == SQLITE_ROW) {
            playerID = sqlite3_column_int(idStatement, 0);
            sqlite3_finalize(idStatement);
            sqlite3_close(db);
            cout << "Player ID retrieved: " << playerID << "\n";
            return playerID;
        }

        /* Finalize statement. */
        sqlite3_finalize(idStatement);
    }


    /* Player character does not exist. Create it. */

    const char* newCharacterSQL = "INSERT INTO character (name, is_player) VALUES (?, ?);";
    sqlite3_stmt* newCharStatement;
    returnCode = sqlite3_prepare_v2(db, newCharacterSQL, -1, &newCharStatement, nullptr);

    /* Bind values. */
    const char* name = "Player";
    int isPlayerBoolInt = 1;
    sqlite3_bind_text(newCharStatement, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(newCharStatement, 2, isPlayerBoolInt);

    /* Execute the statement. */
    returnCode = sqlite3_step(newCharStatement);
    if (returnCode != SQLITE_DONE) { cerr << "Insert Player Character failed: " << sqlite3_errmsg(db) << endl; }
    else {
        /* Get the ID of the saved item. */
        playerID = static_cast<int>(sqlite3_last_insert_rowid(db));
    }

    /* Close DB. */
    sqlite3_finalize(newCharStatement);
    sqlite3_close(db);

    return playerID;
}


/**
* 
* 
*  __  __                   ____      _       _           _
* |  \/  | __ _ _ __       |  _ \ ___| | __ _| |_ ___  __| |
* | |\/| |/ _` | '_ \ _____| |_) / _ \ |/ _` | __/ _ \/ _` |
* | |  | | (_| | |_) |_____|  _ <  __/ | (_| | ||  __/ (_| |
* |_|__|_|\__,_| .__/    _ |_| \_\___|_|\__,_|\__\___|\__,_|
* |  ___|   _ _|_|   ___| |_(_) ___  _ __  ___
* | |_ | | | | '_ \ / __| __| |/ _ \| '_ \/ __|
* |  _|| |_| | | | | (__| |_| | (_) | | | \__ \
* |_|   \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
* 
* 
*/


export void updatePlayerMapLocation(string slugString, Point position) {
    sqlite3* db;
    const char* mapSlug = slugString.c_str();

    /* Open database. */
    int dbFailed = sqlite3_open(dbPath(), &db);
    cout << dbFailed << "\n";
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return;
    }

    /* Create statement for updating player position in a Map table entry in the DB. */
    const char* insertSQL = "UPDATE map SET character_x = ?, character_y = ? WHERE slug = ?;";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, insertSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare MAP update statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return;
    }

    /* Bind the data and execute the statement. */
    sqlite3_bind_int(statement, 1, position.x);
    sqlite3_bind_int(statement, 2, position.y);
    sqlite3_bind_text(statement, 3, mapSlug, -1, SQLITE_STATIC);
    returnCode = sqlite3_step(statement);

    if (returnCode != SQLITE_DONE) {
        cerr << "Insert failed for MAP: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(statement);
        return;
    }

    /* Finalize statement. */
    sqlite3_finalize(statement);
    sqlite3_close(db);
}

/*
* When the player first enters a particular map, so the map is built for the first time,
* we send the built map here to be saved to the database.
* So we save the map itself, the blocks, the limbs, and the limbs' joints.
*/
export bool createNewMap(Map& map) {
    bool success = false;
    sqlite3* db;
    string slugString = map.getForm().slug;
    const char* mapSlug = slugString.c_str();
    int playerX = map.getPlayerCharacter().getBlockX();
    int playerY = map.getPlayerCharacter().getBlockY();

    /* Open database. */
    int dbFailed = sqlite3_open(dbPath(), &db);
    cout << dbFailed << "\n";
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return success;
    }

    /* First save the Map object to the DB. */

    /* Create statement for adding new Map object to the database. */
    const char* insertSQL = "INSERT INTO map (slug, character_x, character_y) VALUES (?, ?, ?);";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, insertSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare MAP insert statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return success;
    }

    /* Bind the data and execute the statement. */
    sqlite3_bind_text(statement, 1, mapSlug, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, playerX);
    sqlite3_bind_int(statement, 3, playerY);
    returnCode = sqlite3_step(statement);

    if (returnCode != SQLITE_DONE) {
        cerr << "Insert failed for MAP: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(statement);
        sqlite3_close(db);
        return success;
    }

    /* Finalize statement. */
    sqlite3_finalize(statement);


    /* 
    * Now run a loop to save each block.
    * Do a Transaction to reduce time.
    * 
    */

    /* Create statement for adding new Block object to the database. */
    const char* insertBlockSQL = "INSERT INTO block (map_slug, position_x, position_y, is_floor, is_path) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* blockStatement;

    /* Prepare the statement before starting the loop. */
    returnCode = sqlite3_prepare_v2(db, insertBlockSQL, -1, &blockStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare BLOCK insert statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    char* errMsg;

    /* Begin the transaction. */
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    bool blockError = false;
    int isFloor = 0;
    int isPath = 0;

    for (int y = 0; y < map.getRows().size(); ++y) {
        vector<Block>& row = map.getRows()[y];
        for (int x = 0; x < row.size(); ++x) {
            Block& block = row[x];

            /* save this particular block to the database. */

            isFloor = block.getIsFloor() ? 1 : 0;
            isPath = block.getIsPath() ? 1 : 0;
            /* Bind the data and execute the statement. */
            sqlite3_bind_text(blockStatement, 1, mapSlug, -1, SQLITE_STATIC);
            sqlite3_bind_int(blockStatement, 2, x);
            sqlite3_bind_int(blockStatement, 3, y);
            sqlite3_bind_int(blockStatement, 4, isFloor);
            sqlite3_bind_int(blockStatement, 5, isPath);

            if (sqlite3_step(blockStatement) == SQLITE_DONE) {
                /* Get the ID of the saved item. */
                int blockID = static_cast<int>(sqlite3_last_insert_rowid(db));
                block.setId(blockID);
            }
            else {
                cerr << "Insert failed for BLOCK: " << sqlite3_errmsg(db) << endl;
                blockError = true;
                break;
            }

            sqlite3_reset(blockStatement);
        }
        if (blockError) {
            cout << "ERROR\n";
            break;
        }
    }

    /* Finalize the statement, commit the transaction. */
    sqlite3_finalize(blockStatement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);

    if (blockError) { 
        /* DELETE map and all blocks */
        sqlite3_close(db);
        return success;
    }

    cout << map.getRoamingLimbs().size() << " roaming limbs.\n";

    /* 
    * Next do a transaction to save all the Roaming Limbs to the database.
    * Also save their Joint objects to the database in the same transaction.
    */

    /* Create statements for adding new Limb and Joint objects to the database. */
    const char* insertLimbSQL = "INSERT INTO limb (form_slug, map_slug, position_x, position_y) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* limbStatement;

    /* Prepare the statements before starting the loop.
    * Start with LIMB statement
    */
    returnCode = sqlite3_prepare_v2(db, insertLimbSQL, -1, &limbStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB insert statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    /* Create statement for adding new JOINT object to the database. */
    const char* insertJointSQL = "INSERT INTO joint (vector_index, limb_id, point_form_x, point_form_y, "
        "modified_point_x, modified_point_y) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* jointStatement;

    /* Prepare the JOINT statement before starting the loop. */
    returnCode = sqlite3_prepare_v2(db, insertJointSQL, -1, &jointStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare JOINT insert statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    /* Begin the transaction. */
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    bool limbError = false;

    /* Loop through the Limbs and save each one. */
    string limbFormSlugString;

    for (Limb& limb : map.getRoamingLimbs()) {
        limbFormSlugString = limb.getForm().slug;

        sqlite3_bind_text(limbStatement, 1, limbFormSlugString.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(limbStatement, 2, mapSlug, -1, SQLITE_STATIC);
        sqlite3_bind_int(limbStatement, 3, limb.getPosition().x);
        sqlite3_bind_int(limbStatement, 4, limb.getPosition().y);

        if (sqlite3_step(limbStatement) == SQLITE_DONE) {
            /* Get the ID of the saved item. */
            int limbID = static_cast<int>(sqlite3_last_insert_rowid(db));
            limb.setId(limbID);

            /* Loop through this limb's JOINT objects and save each of them to the DB.
            * Use the statement created just beneath the LIMB statement.
            */
            bool jointError = false;
            for (int i = 0; i < limb.getJoints().size(); ++i) {
                Joint& joint = limb.getJoints()[i];

                sqlite3_bind_int(jointStatement, 1, i); /* vector_index */
                sqlite3_bind_int(jointStatement, 2, limbID); /* limb ID */
                sqlite3_bind_int(jointStatement, 3, joint.getFormPoint().x); /* point form x */
                sqlite3_bind_int(jointStatement, 4, joint.getFormPoint().y); /* point form y */
                sqlite3_bind_int(jointStatement, 5, joint.getPoint().x); /* modified point x */
                sqlite3_bind_int(jointStatement, 6, joint.getPoint().y); /* modified point y */

                if (sqlite3_step(jointStatement) == SQLITE_DONE) {
                    joint.setId(static_cast<int>(sqlite3_last_insert_rowid(db)));
                }
                else {
                    cerr << "Insert failed for JOINT: " << sqlite3_errmsg(db) << endl;
                    jointError = true;
                    break;
                }

                /* Reset the JOINT statement for the next loop. */
                sqlite3_reset(jointStatement);
            }

            if (jointError) { /* TO DO: DELETE map and all blocks and all limbs. */ }
        }
        else {
            cerr << "Insert failed for LIMB: " << sqlite3_errmsg(db) << endl;
            limbError = true;
            break;
        }
        
        /* Reset the LIMB statement for the next loop. */
        sqlite3_reset(limbStatement);
    }

    /* Finalize the statements, commit the transaction. */
    sqlite3_finalize(jointStatement);
    sqlite3_finalize(limbStatement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);

    if (limbError) { /* TO DO: DELETE map and all blocks and all limbs. */ }


    /*
    * 
    * 
    * Now do a loop to save each Landmark.
    * 
    * 
    */

    /* Create statement for adding new Landmark object to the database. */
    const char* insertLandmarkSQL = "INSERT INTO landmark (map_slug, landmark_type, slug, position_x, position_y) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* landmarkStatement;

    /* Prepare the statement before starting the loop. */
    returnCode = sqlite3_prepare_v2(db, insertLandmarkSQL, -1, &landmarkStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LANDMARK insert statement: " << sqlite3_errmsg(db) << endl;
        return false;
    }


    /* Begin the transaction. */
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    bool landmarkError = false;

    for (Landmark& landmark : map.getLandmarks()) {

        int landmarkType = landmark.getType();
        string slug = landmark.getSlug();

        /* Bind the data and execute the statement. */
        sqlite3_bind_text(landmarkStatement, 1, mapSlug, -1, SQLITE_STATIC);
        sqlite3_bind_int(landmarkStatement, 2, landmarkType);
        sqlite3_bind_text(landmarkStatement, 3, slug.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(landmarkStatement, 4, landmark.getPosition().x);
        sqlite3_bind_int(landmarkStatement, 5, landmark.getPosition().y);

        if (sqlite3_step(landmarkStatement) == SQLITE_DONE) {
            /* Get the ID of the saved item. */
            int landmarkID = static_cast<int>(sqlite3_last_insert_rowid(db));
            landmark.setId(landmarkID);
        }
        else {
            cerr << "Insert failed for LANDMARK: " << sqlite3_errmsg(db) << endl;
            landmarkError = true;
            break;
        }

        sqlite3_reset(landmarkStatement);
    }

    /* Finalize the statement, commit the transaction. */
    sqlite3_finalize(landmarkStatement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);

    if (!landmarkError) { success = true; }
    else { /* DELETE map and all blocks and all limbs. */ }



    /* STILL TO COME: Characters (NPCs). */



    /* Close the database. */
    sqlite3_close(db);
    return success;
}

export bool mapObjectExists(string mapSlug) {
    /*
    * 1. Make an SQL query string to check for map object with the slug from the parameters.
    * 2. open DB, execute query, get true or false into a variable, close DB.
    * 3. Return true or false.
    */

    int count = 0;

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return false;
    }

    /* Create statement template for querying the count. */
    const char* queryCountSQL = "SELECT COUNT(*) FROM map WHERE slug = ?;";
    sqlite3_stmt* statement;
    int returnCode = sqlite3_prepare_v2(db, queryCountSQL, -1, &statement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(statement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute binded statement. */
    if (sqlite3_step(statement) == SQLITE_ROW) {
        count = sqlite3_column_int(statement, 0); }

    /* Finalize statement and close DB. */
    sqlite3_finalize(statement);
    sqlite3_close(db);

    return count > 0;
}


export Map loadMap(string mapSlug) {
    Map map;
    MapForm mapForm = getMapFormFromSlug(mapSlug);
    vector<Limb> roamingLimbs;
    vector<vector<Block>> rows(mapForm.blocksHeight);
    GameState& gameState = GameState::getInstance();

    /* Give each vector of blocks a size. */
    for (vector<Block>& vecOfBlocks : rows) {
        vecOfBlocks = vector<Block>(mapForm.blocksWidth);
    }

    /* Open database. */
    sqlite3* db;
    char* errMsg = nullptr;
    int dbFailed = sqlite3_open(dbPath(), &db);
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return map;
    }


    /* GET THE MAP OBJECT. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryMapSQL = "SELECT * FROM map WHERE slug = ?;";
    sqlite3_stmt* statement;
    int returnCode = sqlite3_prepare_v2(db, queryMapSQL, -1, &statement, nullptr);

    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare map retrieval statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return map;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(statement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute binded statement. */
    if (sqlite3_step(statement) != SQLITE_ROW) {
        cerr << "Failed to retrieve MAP: " << sqlite3_errmsg(db) << endl;
        return map;
    }
    
    Point characterPosition = Point(
        sqlite3_column_int(statement, 1),
        sqlite3_column_int(statement, 2)
    );

    /* Finalize map ID retrieval statement. */
    sqlite3_finalize(statement);


    /* GET THE BLOCKS FROM THE DB. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryBlocksSQL = "SELECT * FROM block WHERE map_slug = ?;";
    sqlite3_stmt* blocksStatement;
    returnCode = sqlite3_prepare_v2(db, queryBlocksSQL, -1, &blocksStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare blocks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return map;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(blocksStatement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(blocksStatement)) == SQLITE_ROW) {
        /* Position is not saved in the block. They dictate where in the rows and row the block is set. */
        int positionX = sqlite3_column_int(blocksStatement, 2);
        int positionY = sqlite3_column_int(blocksStatement, 3);

        int id = sqlite3_column_int(blocksStatement, 0);
        bool isFloor = sqlite3_column_int(blocksStatement, 4) == 1;
        bool isPath = sqlite3_column_int(blocksStatement, 5) == 1;
        bool isLooted = sqlite3_column_int(blocksStatement, 6) == 1;

        if (positionY < rows.size() && positionX < rows[positionY].size()) {
            rows[positionY][positionX] = Block(id, isFloor, isPath, isLooted);
        }
        else {
            cout << "Saved position out of bounds of map." << endl;
            break;
        }
    }

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
    }

    /* Finalize prepared statement. */
    sqlite3_finalize(blocksStatement);

    /* The Block objects are populated. Time to get the Limbs. */


    /*
    * 
    * 
    * 
    * GET THE ROAMING LIMBS FROM THE DB.
    * 
    * 
    * 
    */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryLimbsSQL = "SELECT id, form_slug, position_x, position_y, "
        "character_id FROM limb WHERE map_slug = ? AND character_id < 1;";
    sqlite3_stmt* queryLimbsStatement;
    returnCode = sqlite3_prepare_v2(db, queryLimbsSQL, -1, &queryLimbsStatement, nullptr);
        
    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare LIMBS retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return map;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(queryLimbsStatement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(queryLimbsStatement)) == SQLITE_ROW) {
        int limbID = sqlite3_column_int(queryLimbsStatement, 0);

        /* Get the JOINTS for this Limb. */
        /* Start by querying the count to see how big the vector should be. */
        const char* queryCountJointsSQL = "SELECT COUNT(*) FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryCountJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryCountJointsSQL, -1, &queryCountJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare JOINTS retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return map;
        }

        sqlite3_bind_int(queryCountJointsStatement, 1, limbID);

        /* Execute count query. */
        int rowCount = 0;
        if (sqlite3_step(queryCountJointsStatement) == SQLITE_ROW) {
            rowCount = sqlite3_column_int(queryCountJointsStatement, 0); }
        sqlite3_finalize(queryCountJointsStatement);


        /* Now Get the JOINT itself */
        const char* queryJointsSQL = "SELECT * FROM joint WHERE limb_id = ?;";
        sqlite3_stmt* queryJointsStatement;
        returnCode = sqlite3_prepare_v2(db, queryJointsSQL, -1, &queryJointsStatement, nullptr);

        if (returnCode != SQLITE_OK) {
            std::cerr << "Failed to prepare blocks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return map;
        }

        /* Bind the ID value. */
        sqlite3_bind_int(queryJointsStatement, 1, limbID);
        int jointsReturnCode;
        vector<Joint> joints(rowCount);

        while ((jointsReturnCode = sqlite3_step(queryJointsStatement)) == SQLITE_ROW) {
            int jointID = sqlite3_column_int(queryJointsStatement, 0);
            int vectorIndex = sqlite3_column_int(queryJointsStatement, 1);
            Point pointForm = Point(
                sqlite3_column_int(queryJointsStatement, 3),
                sqlite3_column_int(queryJointsStatement, 4));
            Point modifiedPoint = Point(
                sqlite3_column_int(queryJointsStatement, 5),
                sqlite3_column_int(queryJointsStatement, 6));
            bool isAnchor = sqlite3_column_int(queryJointsStatement, 7) == 1;
            int connectedLimbID = sqlite3_column_int(queryJointsStatement, 8);
            int anchorJointIndex = sqlite3_column_int(queryJointsStatement, 9);
            int rotationAngle = sqlite3_column_int(queryJointsStatement, 10);

            Joint joint = Joint( pointForm, modifiedPoint, isAnchor, connectedLimbID, anchorJointIndex, rotationAngle, jointID);
            if (vectorIndex < rowCount) { joints[vectorIndex] = joint; }
        }

        /* Create the actual Limb (with its joints) and add to RoamingLimbs vector. */
        roamingLimbs.emplace_back(
            limbID,
            getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1))),
            Point(
                sqlite3_column_int(queryLimbsStatement, 2),
                sqlite3_column_int(queryLimbsStatement, 3)
            ),
            joints
        );
    }

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
        return map;
    }

    /* Finalize prepared statement. */
    sqlite3_finalize(queryLimbsStatement);




    /* Get the LANDMARK objects. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryLandmarksSQL = "SELECT id, landmark_type, slug, position_x, position_y FROM landmark WHERE map_slug = ?;";
    sqlite3_stmt* queryLandmarksStatement;
    returnCode = sqlite3_prepare_v2(db, queryLandmarksSQL, -1, &queryLandmarksStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare landmarks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return map;
    }

    /* Bind the slug value. */
    sqlite3_bind_text(queryLandmarksStatement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    vector<Landmark> landmarks;

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(queryLandmarksStatement)) == SQLITE_ROW) {

        int landmarkID = sqlite3_column_int(queryLandmarksStatement, 0);
        int landmarkTypeInt = sqlite3_column_int(queryLandmarksStatement, 1);
        LandmarkType landmarkType = isValidLandmarkType(landmarkTypeInt) ? static_cast<LandmarkType>(landmarkTypeInt) : Entrance;
        string landmarkSlug = stringFromUnsignedChar(sqlite3_column_text(queryLandmarksStatement, 2));
        int positionX = sqlite3_column_int(queryLandmarksStatement, 3);
        int positionY = sqlite3_column_int(queryLandmarksStatement, 4);

        Point position = Point(positionX, positionY);

        /*
        * WHEN we start making SHRINE landmarks, we will need to use more of this data and to call a builder
        * or else call a FORM which will get the TEXTURE.
        * 
        * FOR NOW we will just check for entrance and exit, and add them.
        */

        if (landmarkType == Entrance) {
            landmarks.emplace_back(getEntranceLandmark(position));
        } else if (landmarkType == Exit) {
            landmarks.emplace_back(getExitLandmark(position)); }
    }

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed (retrieving landmarks): " << sqlite3_errmsg(db) << endl;
        return map;
    }

    /* Finalize prepared statement. */
    sqlite3_finalize(queryLandmarksStatement);


    /* Close DB. */
    sqlite3_close(db);

    /* BUILD THE MAP. */
    map = Map(mapForm, roamingLimbs, rows, characterPosition);
    map.setLandmarks(landmarks);

    return map;
}
