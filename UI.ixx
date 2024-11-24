/*
* Basic UI elements.
* The UI class will be a singleton which is shared between all Screen modules.
* Main script creates it and hands it to the active Screen.
* 
* A SCREEN can have PANELS, and PANELS can have BUTTONS (no panels outside of screens, no buttons outside of panels).
* This module will also include functions to check if a screen's panel's buttons are being hovered over or clicked.
* But the onclick action function will be in the SCREEN object (or at least will originate there and be fed in).
* There are some Panels which should be available to multiple Screens.
* 
* UI holds no state. This object can be created and destroyed for each Screen, within the run function.
* It goes out of scope and dies when the run function is finished.
*
* How do we apply functionality to the buttons?
* Maybe we make our own event queue?
* 
* TO DO:
* - make window resizable.
* - minimum limit for size (set as consts).
* - panel resizes based on window.
*/

module;

#include "SDL.h"
#include "SDL_image.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include "SDL_ttf.h"
#include <cmath>
#include <vector>
#include <cstdlib>
#include <time.h>
#include <vector>
//#include <functional>

export module UI;
using namespace std;

class Panel;
class Button;
struct PreButtonStruct;

/* UI (singleton) class holds basic reusable SDL objects.
* It's also responsible for creating panels and buttons,
* but does not HOLD panels and buttons.
* Screen objects instead will hold panels. */
export class UI {

	public:
		// Deleted copy constructor and assignment operator to prevent copies
		UI(const UI&) = delete;
		UI& operator=(const UI&) = delete;

		// Static method to get the instance of the singleton (so we don't call the constructor... we call getInstance())
		static UI& getInstance() {
			// THIS is the key moment where we create the actual object
			static UI instance; // will be destroyed when program exits
			return instance;
		}


		bool isInitialized() { return initialized; }
		SDL_Renderer* getMainRenderer() { return mainRenderer; }
		SDL_Surface* getWindowSurface() { return mainWindowSurface; }
		SDL_Window* getMainWindow() { return mainWindow; }
		TTF_Font* getButtonFont() { return buttonFont; }
		SDL_Color getTextColor() { return textColor; }
		Panel createMainMenuPanel();


	private:
		// Constructor is PRIVATE to prevent instantiation from outside the class
		UI() {
			cout << "UI created\n";
			initialized = initialize();
		}

		// Private destructor to prevent deletion through a pointer to the base class
		~UI() = default;

		const int SCREEN_WIDTH = 720;
		const int SCREEN_HEIGHT = 960;

		const int BUTTON_PADDING = 20;
		const int PANEL_PADDING = 20;

		int initialized = false;

		SDL_Window* mainWindow = NULL;
		SDL_Renderer* mainRenderer = NULL;
		SDL_Surface* mainWindowSurface = NULL;

		TTF_Font* titleFont = NULL;
		TTF_Font* buttonFont = NULL;
		TTF_Font* bodyFont = NULL;
		TTF_Font* dialogFont = NULL;

		SDL_Color textColor = { 25, 25, 25 };

		PreButtonStruct buildPreButtonStruct(string text);
		SDL_Rect buildVerticalPanelRectFromButtonTextRects(vector<PreButtonStruct> preButtonStructs);
		vector<Button> buildButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect);

		bool initialize() {

			// Initialize SDL
			if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				SDL_Log("SDL failed to initialize. SDL_Error: %s\n", SDL_GetError());
				std::cerr << "SDL failed to initialize. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			// Initialize Image library (though we're not using any images yet)

			//Initialize PNG loading
			int imgFlags = IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags))
			{
				SDL_Log("SDL_image could not initialize! %s\n", IMG_GetError());
				return false;
			}

			// Load main window, surface, and renderer
			mainWindow = SDL_CreateWindow("Main Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

			if (!mainWindow) {
				SDL_Log("Window failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Window failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			mainRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED);

			if (!mainRenderer) {
				SDL_Log("Renderer failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Renderer failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			mainWindowSurface = SDL_GetWindowSurface(mainWindow);

			if (!mainWindowSurface) {
				SDL_Log("Window Surface failed to load. SDL_Error: %s\n", SDL_GetError());
				cerr << "Window Surface failed to load. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

			// Initialize TTF font library
			if (TTF_Init() == -1) {
				SDL_Log("WTTF failed to initialize. TTF_Error: %s\n", TTF_GetError());
				std::cerr << "TTF failed to initialize. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}


			// Load the fonts

			titleFont = TTF_OpenFont("assets/ander_hedge.ttf", 42);

			if (!titleFont) {
				SDL_Log("Font (ander_hedge) failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font (ander_hedge) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}
			
			buttonFont = TTF_OpenFont("assets/alte_haas_bold.ttf", 36);

			if (!buttonFont) {
				SDL_Log("Font (alte_haas_bold) failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font (alte_haas_bold) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			bodyFont = TTF_OpenFont("assets/alte_haas.ttf", 18);

			if (!bodyFont) {
				SDL_Log("Font (alte_haas) failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font (alte_haas) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}

			dialogFont = TTF_OpenFont("assets/la_belle_aurore.ttf", 18);

			if (!dialogFont) {
				SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
				cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
				return false;
			}
			return true;
		}
};


export bool isInRect(SDL_Rect rect, int mouseX, int mouseY) {
	// check horizontal
	if (mouseX >= rect.x && mouseX <= rect.x + rect.w) {
		// check vertical
		if (mouseY >= rect.y && mouseY <= rect.y + rect.h) {
			return true;
		}
	}
	return false;
}

/*  Buttons contain a Rect, but also must contain their (anonymous) action function.
	 Buttons make a Rect clickable and allow us to perform other actions on them.
	 text texture should be saved in the button */
export class Button {
	public:
		/* Constructor to make a button the length of its text (must send in padding)
		* Constructor receives the position of the button, text string, and font.
		* Will receive anonymous function too.
		*/
		Button(int x, int y, string incomingText, TTF_Font* buttonFont, SDL_Color fontColor, SDL_Renderer* mainRenderer) {
			// constants for the creation of the button
			const int buttonPadding = 20;

			text = incomingText;
			//setTextRect(rect, text, buttonFont);
			// Somehow we must pass in a function as an action
			// use the Standard library header <functional> for lambdas and anoymous fuctions

			int textRectWidth, textRectHeight;
			// get height and width of text based on string (set those values into ints)
			TTF_SizeUTF8(buttonFont, text.c_str(), &textRectWidth, &textRectHeight);

			// make the textRect with the appropriate borders
			textRect = {
				x + (buttonPadding / 2),
				y + (buttonPadding / 2),
				textRectWidth,
				textRectHeight
			};

			// the actual button rect
			rect = {
				x,
				y,
				textRectWidth + buttonPadding,
				textRectHeight + buttonPadding
			};

			// make the textTexture
			SDL_Surface* buttonTextSurface = TTF_RenderText_Blended(buttonFont, text.c_str(), fontColor);
			textTexture = SDL_CreateTextureFromSurface(mainRenderer, buttonTextSurface);
			SDL_FreeSurface(buttonTextSurface);
		}

		/* Constructor for when we already have both SDL_Rects */
		Button(SDL_Rect buttonRect, SDL_Rect incomingTextRect, string incomingText, TTF_Font* buttonFont, SDL_Color fontColor, SDL_Renderer* mainRenderer) {
			rect = buttonRect;
			textRect = incomingTextRect;
			text = incomingText;

			// make the textTexture
			SDL_Surface* buttonTextSurface = TTF_RenderText_Blended(buttonFont, text.c_str(), fontColor);
			textTexture = SDL_CreateTextureFromSurface(mainRenderer, buttonTextSurface);
			SDL_FreeSurface(buttonTextSurface);
		}

		// Might turn this private since we should only operate on it internally
		SDL_Rect getRect() { return rect; }
		SDL_Rect getTextRect() { return textRect; }
		string getText() { return text; } // turn this completely into char for the printing
		SDL_Texture* getTextTexture() { return textTexture; }

		// check if mouse location has hit the panel
		bool isInButton(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }


	private:
		SDL_Rect rect;
		SDL_Rect textRect;
		SDL_Texture* textTexture = NULL;
		string text = "";

		/* build inner textRect based on other button information */
		void setTextRect(SDL_Rect buttonRect, string buttonText, TTF_Font* buttonFont) {
			rect = buttonRect;

			// get height and width of textRect
			int textRectWidth, textRectHeight;
			TTF_SizeUTF8(buttonFont, buttonText.c_str(), &textRectWidth, &textRectHeight);

			int xPadding = (buttonRect.w - textRectWidth) / 2;
			int yPadding = (buttonRect.h - textRectHeight) / 2;

			// finally make the textRect with the appropriate borders
			textRect = {
				buttonRect.x + xPadding,
				buttonRect.y + yPadding,
				textRectWidth,
				textRectHeight
			};
		}
};

/* Panels contain buttons. There can be no buttons without a Panel container. */
export class Panel {
	public:
		// constructor
		Panel(SDL_Rect incomingRect, vector<Button> incomingButtons) {
			rect = incomingRect;
			buttons = incomingButtons;
		}

		SDL_Rect getRect() { return rect; }
		vector<Button> getButtons() { return buttons; }

		// check if mouse location has hit the panel
		bool isInPanel(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }

	private:
		SDL_Rect rect;
		vector<Button> buttons;
		// color? bg image?
};

/* Stores initial data to assist construction of the panel.
* Panel uses this data to help construct itself.
* Then buttons use this data AND panel data to construct THEMselves. */
struct PreButtonStruct {
	int textRectWidth;
	int textRectHeight;
	string text;
};


/* Extra member functions */

/* Buttons need their parent panel rect before they can be built.
* Yet panel rects need information about their child buttons before they can be built.
* So we build PART of the button first, for the panel rects which let us finish the buttons.*/
PreButtonStruct UI::buildPreButtonStruct(string text) {
	int textRectWidth, textRectHeight;
	// get height and width of text based on string (set those values into ints)
	TTF_SizeUTF8(buttonFont, text.c_str(), &textRectWidth, &textRectHeight);

	PreButtonStruct preButtonStruct;
	preButtonStruct.textRectWidth = textRectWidth;
	preButtonStruct.textRectHeight = textRectHeight;
	preButtonStruct.text = text;

	return preButtonStruct;
}

/* Now that we have some information about the buttons (via struct), we can build the Panel's RECT. */
SDL_Rect UI::buildVerticalPanelRectFromButtonTextRects(vector<PreButtonStruct> preButtonStructs) {
	int panelHeight = PANEL_PADDING; // start with one padding
	int longestButtonTextLength = 0;
	for (PreButtonStruct preButtonStruct : preButtonStructs) {
		// add up the heights of the buttons plus padding
		panelHeight += preButtonStruct.textRectHeight + (BUTTON_PADDING * 2) + PANEL_PADDING;

		// set the longest text length
		if (preButtonStruct.textRectWidth > longestButtonTextLength) {
			longestButtonTextLength = preButtonStruct.textRectWidth;
		}
	}

	return {
		/* x is always 0 */
		0,
		SCREEN_HEIGHT - panelHeight,
		/* panel is just wide enough to accomodate the longest button */
		longestButtonTextLength + (BUTTON_PADDING * 2) + (PANEL_PADDING * 2),
		panelHeight
	};
}

/* NOW that we have both some info about the buttons, plus the panel rect, make the actual buttons. */
vector<Button> UI::buildButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect) {
	vector<Button> buttons;
	const int xForAll = PANEL_PADDING;
	int widthForAll = panelRect.w - (PANEL_PADDING * 2);
	int heightSoFar = panelRect.y + PANEL_PADDING;


	for (int i = 0; i < preButtonStructs.size(); ++i) {
		// PreButtonStruct preButtonStruct : preButtonStructs
		// start at the top of the panelRect PLUS panelPadding

		SDL_Rect thisButtonRect = {
			xForAll,
			heightSoFar,
			widthForAll,
			preButtonStructs[i].textRectHeight + (BUTTON_PADDING * 2)
		};

		SDL_Rect thisTextRect = {
			// text x position is button position plus HALF of the difference b/w button width and text width
			thisButtonRect.x + ((thisButtonRect.w - preButtonStructs[i].textRectWidth) / 2), // centered text
			thisButtonRect.y + BUTTON_PADDING, // this is correct
			preButtonStructs[i].textRectWidth,
			preButtonStructs[i].textRectHeight
		};

		Button thisButton = Button(thisButtonRect, thisTextRect, preButtonStructs[i].text, buttonFont, textColor, mainRenderer);
		buttons.push_back(thisButton);

		// increment heightSoFar
		heightSoFar += thisButtonRect.h + PANEL_PADDING;
	}

	return buttons;
}

/* 
* PANEL CREATION FUNCTIONS
* These are member functions of the UI class.
* Defining them in the class definition would be too bulky.
* 
* We can have VERTICAL panels (going all up the side)
* and HORIZONTAL panels (2x buttons stacked up from the bottom)
*/


/*
* Panel for the Main Menu Screen.
*/
Panel UI::createMainMenuPanel() {
	vector<PreButtonStruct> preButtonStructs;

	// preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical)
	preButtonStructs = {
		buildPreButtonStruct("NEW GAME"),
		buildPreButtonStruct("LOAD GAME"),
		buildPreButtonStruct("SETTINGS"),
		buildPreButtonStruct("ABOUT")
	};

	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	return Panel(panelRect, buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect));
}

/*
* STILL TO COME:
*	Main Menu Sub-Panels : Settings Menu & Load Game Menu
*	Battle Panel (and possibly battle Sub-Panels)
*	Map Panel (no idea what will go in here!)
*/

