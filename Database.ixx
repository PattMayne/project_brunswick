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

/* Creates the database and tables if they don't exist. */
export void createDB() {
    sqlite3* db;

    /* Open database (create if does not exist). */
    int dbFailed = sqlite3_open("data/limbs_data.db", &db);
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
* We need:
* mapID, form_slug,
*/
export int createLimb(NewRoamingLimbData limbData) {
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