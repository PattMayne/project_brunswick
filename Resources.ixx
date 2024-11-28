module;

#include <iostream>
#include <fstream>
#include <vector>
#include "include/json.hpp"

export module Resources;

using json = nlohmann::json;
using namespace std;


/*
* Singleton object to fetch string and numeric data stored in JSON file.
* Dialog and other longer texts will be stored here.
* A similar one will access LIMB object definitions. JSON to store LIMB data.
* 
* Each "get" function must be a member function of the singleton. That way we can keep the file object alive.
*/

export class Resources {
    public:
        // Deleted copy constructor and assignment operator to prevent copies
        Resources(const Resources&) = delete;
        Resources& operator=(const Resources&) = delete;

        static Resources& getInstance() {
            // create the actual object
            static Resources instance;
            return instance;
        }
        
        // get data functions
        string getTitle();
        vector<string> getTitleWords();
        string getButtonText(string buttonLabel);

    private:
        // constructors
        Resources() {
            jsonData = loadJsonData();
        }

        ~Resources() = default;

        // private functions
        json loadJsonData();

        // private variables
        json jsonData;
};

json Resources::loadJsonData() {
    // Create an ifstream to read the JSON file
    std::ifstream file("data/resources.json");
    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
    }

    // Create a JSON object
    json jsonData;

    // Parse the JSON file
    try {
        file >> jsonData;
    }
    catch (const json::parse_error& e) {
        cerr << "Parse error: " << e.what() << endl;
        return jsonData;;
    }

    // Close the file
    file.close();
    return jsonData;
}

// Get the whole title of the game as a string
export string Resources::getTitle() {
    string title = "";

    // build string from title words
    for (int i = 0; i < jsonData["title"].size(); ++i) {
        title.append(jsonData["title"][i]);
        if (i < jsonData["title"].size() - 1) {  title.append(" "); }
    }

    cout << title;
    return title;
}

// get a vector of the words in the title for dynamic display
vector<string> Resources::getTitleWords() {
    vector<string> titleWords;

    for (const string word : jsonData["title"]) {
        titleWords.push_back(word);
    }

    return titleWords;
}

/* Get string values for buttons */
string Resources::getButtonText(string buttonLabel) {
    if (jsonData.contains("buttonText") && jsonData["buttonText"].contains(buttonLabel)) {
        return jsonData["buttonText"][buttonLabel];
    }
    else {
        cerr << "Key does not exist";
        return "BUTTON";
    }
}