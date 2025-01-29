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

import CharacterClasses;
import MapClasses;
import TypeStorage;
import LimbFormMasterList;
import FormFactory;

using namespace std;

const char* dbPath() { return "data/limbs_data.db"; }

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

/*
* When a new Limb is created on a Map object, use this function to create the limb in the database.
* The NewRoamingLimbData object should be created first, then sent here to create the object in the DB,
* then the ID is sent back to create the actual Limb.
* 
* NOTE: Do we really need the struct? Maybe we should send a reference to the Limb object and populate it with an id?
* 
* 
*/
export int createRoamingLimb(Limb& limb, string mapSlug, Point position) {
    sqlite3* db;
    int id = -1;

    /* Open database (create if does not exist). */
    int dbFailed = sqlite3_open(dbPath(), &db);
    cout << dbFailed << "\n";
    if (dbFailed != 0) {
        cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl;
        return id;
    }

    /* Create statement for adding new Limb object to the database. */
    const char* insertSQL = "INSERT INTO LIMB (form_slug, map_slug, position_x, position_y) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* statement;

    /* Prepare the statement. */
    int returnCode = sqlite3_prepare_v2(db, insertSQL, -1, &statement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return id;
    }

    string limbFormSlugString = limb.getForm().slug;

    /* Bind the data. */
    sqlite3_bind_text(statement, 1, limbFormSlugString.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(statement, 2, mapSlug.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 3, position.x);
    sqlite3_bind_int(statement, 4, position.y);

    /* Execute the statement. */
    returnCode = sqlite3_step(statement);
    if (returnCode != SQLITE_DONE) { cerr << "Insert failed: " << sqlite3_errmsg(db) << endl;  }
    else {
        /* Get the ID of the saved item. */
        id = static_cast<int>(sqlite3_last_insert_rowid(db));
    }

    /* Finalize statement and close database. */
    sqlite3_finalize(statement);
    sqlite3_close(db);

    return id;
}


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

    /* Now run a loop to save each block.
    * Do a Transaction to reduce time.
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

    /* Next do a transaction to save all the Roaming Limbs to the database. */

    /* Create statement for adding new Limb object to the database. */
    const char* insertLimbSQL = "INSERT INTO limb (form_slug, map_slug, position_x, position_y) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* limbStatement;

    /* Prepare the statement before starting the loop. */
    returnCode = sqlite3_prepare_v2(db, insertLimbSQL, -1, &limbStatement, nullptr);
    if (returnCode != SQLITE_OK) {
        cerr << "Failed to prepare LIMB insert statement: " << sqlite3_errmsg(db) << endl;
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
        }
        else {
            cerr << "Insert failed for LIMB: " << sqlite3_errmsg(db) << endl;
            limbError = true;
            break;
        }

        sqlite3_reset(limbStatement);
    }

    /* Finalize the statement, commit the transaction. */
    sqlite3_finalize(limbStatement);
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);

    if (!limbError) { success = true; }
    else { /* DELETE map and all blocks and all limbs. */ }

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
        count = sqlite3_column_int(statement, 0);
    }

    /* Finalize statement and close DB. */
    sqlite3_finalize(statement);
    sqlite3_close(db);

    return count > 0;
}

export string stringFromUnsignedChar(const unsigned char* c_str) {
    std::string str;
    int i{ 0 };
    while (c_str[i] != '\0') { str += c_str[i++]; }
    return str;
}

export Map loadMap(string mapSlug) {
    Map map;
    MapForm mapForm = getMapFormFromSlug(mapSlug);
    vector<Limb> roamingLimbs;
    vector<vector<Block>> rows(mapForm.blocksHeight);

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
        cerr << "Failed to prepare map id retrieval statement: " << sqlite3_errmsg(db) << endl;
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

    cout << "Point: " << characterPosition.x << ", " << characterPosition.y << "\n";

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

    // Finalize the prepared statement
    sqlite3_finalize(blocksStatement);

    /* The Block objects are populated. Time to get the Limbs. */


    /* GET THE LIMBS FROM THE DB. */

    /* Create statement template for querying Map objects with this slug. */
    const char* queryLimbsSQL = "SELECT id, form_slug, position_x, position_y FROM limb WHERE map_slug = ?;";
    sqlite3_stmt* queryLimbsStatement;
    returnCode = sqlite3_prepare_v2(db, queryLimbsSQL, -1, &queryLimbsStatement, nullptr);

    if (returnCode != SQLITE_OK) {
        std::cerr << "Failed to prepare blocks retrieval statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return map;
    }

    /* Bind the id value. */
    sqlite3_bind_text(queryLimbsStatement, 1, mapSlug.c_str(), -1, SQLITE_STATIC);

    /* Execute and iterate through results. */
    while ((returnCode = sqlite3_step(queryLimbsStatement)) == SQLITE_ROW) {

        roamingLimbs.emplace_back(
            sqlite3_column_int(queryLimbsStatement, 0),
            getLimbForm(stringFromUnsignedChar(sqlite3_column_text(queryLimbsStatement, 1))),
            Point(
                sqlite3_column_int(queryLimbsStatement, 2),
                sqlite3_column_int(queryLimbsStatement, 3)
            )
        );

    }

    if (returnCode != SQLITE_DONE) {
        cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
        return map;
    }

    // Finalize the prepared statement
    sqlite3_finalize(queryLimbsStatement);
    /* Close DB. */
    sqlite3_close(db);

    /* BUILD THE MAP. */

    map = Map(mapForm, roamingLimbs, rows, characterPosition);

    return map;
}