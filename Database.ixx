module;

import <vector>;
import <cstdlib>;
import <iostream>;
import <unordered_map>;
#include "sqlite3.h"

using namespace std;

export module Database;

export void openDB() {
    sqlite3* db;
    char* errorMessage = 0;

    // Open database
    int exit = sqlite3_open("example.db", &db);
    if (exit) {
        cerr << "Error open DB: " << sqlite3_errmsg(db) << endl;
    }
    else {
        cout << "Opened Database Successfully!" << endl;
    }

    // Close database
    sqlite3_close(db);
}