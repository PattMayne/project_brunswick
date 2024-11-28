module;

#include <iostream>
#include <fstream>
#include <vector>
#include "include/json.hpp"

export module Resources;

import ScreenType;
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
        Resolution getDefaultDimensions(WindowResType restype);

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

Resolution Resources::getDefaultDimensions(WindowResType resType) {
    int width = 200;
    int height = 200;

    if (
        jsonData.contains("window") &&
        jsonData["window"].contains("defaults") &&
        jsonData["window"]["defaults"].contains("width") &&
        jsonData["window"]["defaults"].contains("height")
    ) {

        cout << "\n data exists \n";
        json widthData = jsonData["window"]["defaults"]["width"];
        json heightData = jsonData["window"]["defaults"]["height"];
        /* mobile first */

        if (resType == WindowResType::Mobile) {
            if (widthData.contains("mobile")) {
                width = widthData["mobile"];
                height = heightData["mobile"];
            }
        } else if (resType == WindowResType::Tablet) {
            if (widthData.contains("tablet")) {
                width = widthData["tablet"];
                height = heightData["tablet"];
            }
        }
        else if (resType == WindowResType::Desktop) {
            if (widthData.contains("desktop")) {
                width = widthData["desktop"];
                height = heightData["desktop"];
            }
        }
    }
    return Resolution(width, height);
}