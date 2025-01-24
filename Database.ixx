module;

import <vector>;
import <cstdlib>;
import <iostream>;
import <unordered_map>;
import <fstream>;
import <string>;
#include "sqlite3.h"

using namespace std;

export module Database;

export void openDB() {
    sqlite3* db;

    /* Open database (create if does not exist). */
    int dbFailed = sqlite3_open("data/limbs_data.db", &db);
    cout << dbFailed << "\n";
    if (dbFailed == 0) {
        cout << "Opened Database Successfully!" << "\n";

        /* Get the schema file. */
        string schemaFilename = "data/schema.sql";
        ifstream schemaFile(schemaFilename);

        if (schemaFile.is_open()) {
            cout << "Opened schema file\n";

            /* Get the SQL string from the open file. */
            string sql((istreambuf_iterator<char>(schemaFile)), istreambuf_iterator<char>());

            char* errMsg = nullptr;

            int returnCode = sqlite3_exec(db, sql.c_str(), NULL, NULL, &errMsg);

            if (returnCode == SQLITE_OK) {
                std::cout << "Schema executed successfully." << std::endl;
            }
            else {
                std::cerr << "SQL error: " << errMsg << std::endl;
                sqlite3_free(errMsg);
            }

        }
        else {
            std::cerr << "Could not open file: " << schemaFilename << std::endl;
            return;
        }

    }
    else {
        cerr << "Error open DB: " << sqlite3_errmsg(db) << endl;
    }

    /* Close database. */
    sqlite3_close(db);
}