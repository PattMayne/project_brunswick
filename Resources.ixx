module;

export module Resources;

import <iostream>;
import <fstream>;
import <vector>;
import <cstdint>;
import "include/json.hpp";

import TypeStorage;
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
        /* Deleted copy constructor and assignment operator to prevent copies */
        Resources(const Resources&) = delete;
        Resources& operator=(const Resources&) = delete;

        static Resources& getInstance() {
            /* create the actual object */
            static Resources instance;
            return instance;
        }
        
        /* get data functions */
        string getTitle();
        vector<string> getTitleWords();
        string getButtonText(string buttonLabel);
        string getMessageText(string messageLabel);
        string getCardinalDirectionText(string directionKey);
        Resolution getDefaultDimensions(WindowResType restype);
        int getFontSize(FontContext fontContext, int windowWidth);
        int getButtonBorder(int windowWidth);
        unordered_map<string, string> getKeyCommands(ScreenType screenType);

    private:
        /* constructors */
        Resources() {
            jsonData = loadJsonData();
        }

        ~Resources() = default;

        /* private functions */
        json loadJsonData();

        /* private variables  */
        json jsonData;
};

json Resources::loadJsonData() {
    /* Create an ifstream to read the JSON file */
    std::ifstream file("data/resources.json");
    if (!file.is_open()) {
        cerr << "Could not open the file!" << endl;
    }

    /* Create a JSON object */
    json jsonData;

    /* Parse the JSON file */
    try {
        file >> jsonData;
    }
    catch (const json::parse_error& e) {
        cerr << "Parse error: " << e.what() << endl;
        return jsonData;;
    }

    /* Close the file */
    file.close();
    return jsonData;
}

/* Get the whole title of the game as a string */
export string Resources::getTitle() {
    string title = "";

    /* build string from title words */
    for (int i = 0; i < jsonData["TITLE"].size(); ++i) {
        title.append(jsonData["TITLE"][i]);
        if (i < jsonData["TITLE"].size() - 1) {  title.append(" "); }
    }
    return title;
}

/* get a vector of the words in the title for dynamic display */
vector<string> Resources::getTitleWords() {
    vector<string> titleWords;

    for (const string word : jsonData["TITLE"]) {
        titleWords.push_back(word);
    }

    return titleWords;
}

/* Get string values for buttons. */
string Resources::getButtonText(string buttonLabel) {
    if (jsonData.contains("BUTTON_TEXT") && jsonData["BUTTON_TEXT"].contains(buttonLabel)) {
        return jsonData["BUTTON_TEXT"][buttonLabel];
    }
    else {
        cerr << "Key does not exist";
        return "BUTTON";
    }
}

/* Get string values for message box. */
string Resources::getMessageText(string messageLabel) {
    if (jsonData.contains("MESSAGES") && jsonData["MESSAGES"].contains(messageLabel)) {
        return jsonData["MESSAGES"][messageLabel];
    }
    else {
        cerr << "Key does not exist";
        return messageLabel;
    }
}

/* Get string values for message box. */
string Resources::getCardinalDirectionText(string directionKey) {
    if (jsonData.contains("CARDINAL_DIRECTIONS") && jsonData["CARDINAL_DIRECTIONS"].contains(directionKey)) {
        return jsonData["CARDINAL_DIRECTIONS"][directionKey];
    }
    else {
        cerr << "Key does not exist";
        return directionKey;
    }
}

/* Default Dimensions are the width & height of our pre-defined WINDOW size options
    returns a struct containing integers: w, h
*/
Resolution Resources::getDefaultDimensions(WindowResType resType) {
    int width = 200;
    int height = 200;

    if (
        jsonData.contains("WINDOW") &&
        jsonData["WINDOW"].contains("DEFAULTS") &&
        jsonData["WINDOW"]["DEFAULTS"].contains("WIDTH") &&
        jsonData["WINDOW"]["DEFAULTS"].contains("HEIGHT")
    ) {
        json widthData = jsonData["WINDOW"]["DEFAULTS"]["WIDTH"];
        json heightData = jsonData["WINDOW"]["DEFAULTS"]["HEIGHT"];
        /* mobile first */

        if (resType == WindowResType::Mobile) {
            if (widthData.contains("MOBILE")) {
                width = widthData["MOBILE"];
                height = heightData["MOBILE"];
            }
        } else if (resType == WindowResType::Tablet) {
            if (widthData.contains("TABLET")) {
                width = widthData["TABLET"];
                height = heightData["TABLET"];
            }
        }
        else if (resType == WindowResType::Desktop) {
            if (widthData.contains("DESKTOP")) {
                width = widthData["DESKTOP"];
                height = heightData["DESKTOP"];
            }
        }
    }
    return Resolution(width, height);
}

/* Get font size for specified context and screen width */
int Resources::getFontSize(FontContext fontContext, int windowWidth) {
    if (jsonData.contains("FONT_SIZES")) {
        json fontData = jsonData["FONT_SIZES"];

        if (jsonData.contains("MEDIA_MIN_WIDTHS")) {
            json mediaMinWidths = jsonData["MEDIA_MIN_WIDTHS"];

            if (mediaMinWidths.contains("X_SMALL") && mediaMinWidths.contains("SMALL") && mediaMinWidths.contains("MEDIUM") && mediaMinWidths.contains("LARGE")) {
                /* Get the string label under which the fonts are stored in the JSON data,
            based on the chosen context */
                string fontContextLabel = fontContext == FontContext::Title ? "TITLE" :
                    fontContext == FontContext::Body ? "BODY" :
                    fontContext == FontContext::Dialog ? "DIALOG" :
                    fontContext == FontContext::Headline ? "HEADLINE" :
                    fontContext == FontContext::HeadlineLarge ? "HEADLINE_LG" :
                    fontContext == FontContext::Button ? "BUTTON" : "ERROR";

                if (fontContextLabel != "ERROR") {
                    if (fontData.contains(fontContextLabel)) {
                        json fontDataWithContext = fontData[fontContextLabel];
                        /* Now we are in the correct context.Get the correct size label. */
                        string sizeLabel = windowWidth > mediaMinWidths["LARGE"] ? "LARGE" :
                            windowWidth > mediaMinWidths["MEDIUM"] ? "MEDIUM" :
                            windowWidth > mediaMinWidths["SMALL"] ? "SMALL" : "X_SMALL";

                        if (fontDataWithContext.contains(sizeLabel)) {
                            int fontSize = static_cast<int>(fontDataWithContext[sizeLabel]);
                            return fontSize;
                        }
                    }
                }
            }
        }
    }

    /* if we reach here there was an error  */
    cerr << "Could not retrieve font size";

    return 16;
}

/* small or large button border */
int Resources::getButtonBorder(int windowWidth) {
    if (jsonData.contains("BUTTON_BORDER")) {
        json bordersData = jsonData["BUTTON_BORDER"];

        if (jsonData.contains("MEDIA_MIN_WIDTHS")) {
            json mediaMinWidths = jsonData["MEDIA_MIN_WIDTHS"];

            string sizeLabel = windowWidth > mediaMinWidths["MEDIUM"] ? "LARGE" : "SMALL";

            if (bordersData.contains(sizeLabel)) {
                return bordersData[sizeLabel];
            }
        }
    }

    return 15;
}

unordered_map<string, string> Resources::getKeyCommands(ScreenType screenType) {
    unordered_map<string, string> keyCommands = {};

    if (jsonData.contains("KEY_COMMANDS")) {
        json keyCommandsData = jsonData["KEY_COMMANDS"];

        string screenTypeText = screenType == ScreenType::Battle ? "BATTLE_SCREEN" :
            screenType == ScreenType::CharacterCreation ? "CHARACTER_CREATION_SCREEN" :
            screenType == ScreenType::Map ? "MAP_SCREEN" : "MENU_SCREEN";

        cout << screenTypeText << endl;

        if (keyCommandsData.contains(screenTypeText)) {
            json screenCommandsData = keyCommandsData[screenTypeText];

            for (const auto& pair : screenCommandsData.items()) {
                cout << pair.key() << ", " << pair.value() << endl;
                string key = pair.key();
                string value = pair.value();
                keyCommands.insert({ key, value });
            }
        }
    }

    return keyCommands;
}