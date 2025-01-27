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
*/
export int createRoamingLimb(NewRoamingLimbData limbData) {
    sqlite3* db;

    /* Open database (create if does not exist). */
    int dbFailed = sqlite3_open("data/limbs_data.db", &db);
    cout << dbFailed << "\n";
    if (dbFailed == 0) {
        cout << "Opened Database Successfully." << "\n";

        

    }
    else { cerr << "Error opening DB: " << sqlite3_errmsg(db) << endl; }

    /* Close database. */
    sqlite3_close(db);

    return -1;
}


export int createMap() {

    return -1;
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