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
* - Add "EXIT" button to main menu.
* - modular design for buttons (and possibly background... LATER... number of modular rects depend on height & width of rect)
*/

module;

#include "include/json.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

export module UI;

using namespace std;

import Resources;
import ScreenType;
class Panel;
class Button;
struct PreButtonStruct;

Uint32 convertSDL_ColorToUint32(const SDL_PixelFormat* format, SDL_Color color);
bool isInRect(SDL_Rect rect, int mouseX, int mouseY);

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
		TTF_Font* getTitleFont() { return titleFont; }
		unordered_map<string, SDL_Color> getColors() { return colorsByFunction; }
		Panel createMainMenuPanel();
		Panel createSettingsPanel();
		int getWindowHeight() { return windowHeight; }
		int getWindowWidth() { return windowWidth; }


	private:
		// Constructor is private to prevent instantiation from outside the class
		UI() {
			/* DEV BLOCK START. set screen size (for dev purposes) */
			if (windowResType == WindowResType::Mobile) {
				setWindowSize(360, 800);
			}
			else if (windowResType == WindowResType::Tablet) {
				setWindowSize(810, 1080);
			}
			else if (windowResType == WindowResType::Desktop) {
				setWindowSize(1600, 900);
			}
			/* DEV BLOCK OVER. we assume res type is Fullscreen */
			
			// fullscreen will be dealt with in initialize()
			initialized = initialize();
			prepareColors();

			/* set the height and width values for items depending on those values */
			getAndStoreWindowSize();
		}

		// Private destructor to prevent deletion through a pointer to the base class
		~UI() = default;

		const WindowResType windowResType = WindowResType::Desktop;

		const int BUTTON_PADDING = 20;
		const int PANEL_PADDING = 20;

		/* Window will actually be full size.These make the numbers available for the UI to calculate things. */
		int windowWidth = 720;
		int windowHeight = 960;

		int initialized = false;

		SDL_Window* mainWindow = NULL;
		SDL_Renderer* mainRenderer = NULL;
		SDL_Surface* mainWindowSurface = NULL;

		TTF_Font* titleFont = NULL;
		TTF_Font* buttonFont = NULL;
		TTF_Font* bodyFont = NULL;
		TTF_Font* dialogFont = NULL;
		PreButtonStruct buildPreButtonStruct(string text, ButtonOption buttonOption, int optionID = -1);
		SDL_Rect buildVerticalPanelRectFromButtonTextRects(vector<PreButtonStruct> preButtonStructs);
		vector<Button> buildButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect);
		void prepareColors();
		void getAndStoreWindowSize();
		/* when you need to dictate dimensions (for dev purposes) */
		void setWindowSize(int x, int y) {
			windowWidth = x;
			windowHeight = y;
		}


		// color maps
		unordered_map<string, SDL_Color> colorsByFunction; // colors by function, which reference colors by name
		unordered_map<string, SDL_Color> colorsByName; // raw colors

		bool initialize();
};



/* BUTTON CLASS */

/*  Buttons contain a Rect, but also must contain clickStruct data to send to the calling Screen.
	 Buttons make a Rect clickable and allow us to perform other actions on them.
	 text texture should be saved in the button */
export class Button {
	public:
		/* Constructor to make a button the length of its text (must send in padding)
		* Constructor receives the position of the button, text string, and font.
		* Will receive anonymous function too.
		* NOT CURRENTLY USING THIS ONE... WILL HAVE TO UPDATE IF/WHEN USED (for horizontal screen?)
		*/
		Button(int x, int y, string incomingText, TTF_Font* buttonFont, SDL_Color fontColor, SDL_Renderer* mainRenderer) {
			//mouseOver = false;
			///* constants for the creation of the button*/
			//const int buttonPadding = 20;

			//text = incomingText;
			///*setTextRect(rect, text, buttonFont);
			//Somehow we must pass in a function as an action
			//use the Standard library header <functional> for lambdas and anoymous fuctions*/

			//int textRectWidth, textRectHeight;
			///* get height and width of text based on string(set those values into ints)*/
			//TTF_SizeUTF8(buttonFont, text.c_str(), &textRectWidth, &textRectHeight);

			///* make the textRect with the appropriate borders*/
			//textRect = {
			//	x + (buttonPadding / 2),
			//	y + (buttonPadding / 2),
			//	textRectWidth,
			//	textRectHeight
			//};

			///* the actual button rect*/
			//rect = {
			//	x,
			//	y,
			//	textRectWidth + buttonPadding,
			//	textRectHeight + buttonPadding
			//};

			///* make the textTexture*/
			//SDL_Surface* buttonTextSurface = TTF_RenderText_Blended(buttonFont, text.c_str(), fontColor);
			//textTexture = SDL_CreateTextureFromSurface(mainRenderer, buttonTextSurface);
			//SDL_FreeSurface(buttonTextSurface);
		}

		/* Constructor for when we already have both SDL_Rects (used buttons have pre-set sizes) */
		Button(
			SDL_Rect buttonRect,
			SDL_Rect textRect,
			string incomingText,
			TTF_Font* buttonFont,
			unordered_map<string, SDL_Color> colors,
			SDL_Renderer* mainRenderer,
			ButtonClickStruct incomingClickStruct
		){
			clickStruct = incomingClickStruct;
			mouseOver = false;
			rect = buttonRect;
			text = incomingText;

			createButtonTextures(buttonRect, textRect, incomingText, buttonFont, colors, mainRenderer);

			cout << "button made\n";
		}

		// Might turn this private since we should only operate on it internally
		SDL_Rect getRect() { return rect; }
		string getText() { return text; } // turn this completely into char for the printing
		SDL_Texture* getHoverTexture() { return hoverTexture; }
		SDL_Texture* getNormalTexture() { return normalTexture; }
		// check if mouse location has hit the panel
		bool isInButton(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }
		void setMouseOver(bool incomingMouseOver) { mouseOver = incomingMouseOver; }
		bool isMouseOver() { return mouseOver; }
		ButtonClickStruct getClickStruct() { return clickStruct; }


	private:
		// REMOVE textRect and textTexture
		// Replace with hoverTexture and normalTexture
		SDL_Rect rect;
		SDL_Texture* hoverTexture = NULL;
		SDL_Texture* normalTexture = NULL;
		string text = "";
		bool mouseOver;
		ButtonClickStruct clickStruct;
		SDL_Rect createTextRect(SDL_Rect buttonRect, string buttonText, TTF_Font* buttonFont);
		SDL_Surface* createButtonSurfaceBG(SDL_Rect buttonRect, SDL_Color color, SDL_Renderer* mainRenderer);
		void createButtonTextures(SDL_Rect buttonRect, SDL_Rect textRect, string incomingText, TTF_Font* buttonFont, unordered_map<string, SDL_Color> colors, SDL_Renderer* mainRenderer);
};

/* build inner textRect based on other button information */
SDL_Rect Button::createTextRect(SDL_Rect buttonRect, string buttonText, TTF_Font* buttonFont) {
	rect = buttonRect;

	// get height and width of textRect
	int textRectWidth, textRectHeight;
	TTF_SizeUTF8(buttonFont, buttonText.c_str(), &textRectWidth, &textRectHeight);

	int xPadding = (buttonRect.w - textRectWidth) / 2;
	int yPadding = (buttonRect.h - textRectHeight) / 2;

	// finally make the textRect with the appropriate borders
	return {
		buttonRect.x + xPadding,
		buttonRect.y + yPadding,
		textRectWidth,
		textRectHeight
	};
}

/* Button textures must start with a surface. Create and get it here. */
SDL_Surface* Button::createButtonSurfaceBG(
	SDL_Rect buttonRect,
	SDL_Color color,
	SDL_Renderer* mainRenderer
) {
	SDL_Surface* buttonSurface = SDL_CreateRGBSurface(0, buttonRect.w, buttonRect.h, 32, 0, 0, 0, 0xFF000000);
	// fill the rects with beautiful color
	SDL_FillRect(buttonSurface, NULL, convertSDL_ColorToUint32(buttonSurface->format, color));
	// TODO: CAN ADD BORDERS or just load an IMAGE INSTEAD (for later)
	return buttonSurface;
}

/* A button must have both NORMAL and HOVER textures. Create them both here. */
void Button::createButtonTextures(
	SDL_Rect buttonRect,
	SDL_Rect textRect,
	string incomingText,
	TTF_Font* buttonFont,
	unordered_map<string, SDL_Color> colors,
	SDL_Renderer* mainRenderer
) {
	SDL_Surface* hoverButtonTextSurface = TTF_RenderUTF8_Blended(buttonFont, text.c_str(), colors["DARK_TEXT"]);
	SDL_Surface* normalButtonTextSurface = TTF_RenderUTF8_Blended(buttonFont, text.c_str(), colors["BTN_TEXT"]);

	// make two button surfaces with the correct BG colors, to blit the text surfaces onto
	SDL_Surface* hoverButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_HOVER_BG"], mainRenderer);
	SDL_Surface* normalButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_BG"], mainRenderer);

	// do the blitting
	SDL_BlitSurface(hoverButtonTextSurface, NULL, hoverButtonSurface, &textRect);
	SDL_BlitSurface(normalButtonTextSurface, NULL, normalButtonSurface, &textRect);

	hoverTexture = SDL_CreateTextureFromSurface(mainRenderer, hoverButtonSurface);
	normalTexture = SDL_CreateTextureFromSurface(mainRenderer, normalButtonSurface);

	// free the surfaces
	SDL_FreeSurface(hoverButtonTextSurface);
	SDL_FreeSurface(normalButtonTextSurface);
	SDL_FreeSurface(hoverButtonSurface);
	SDL_FreeSurface(normalButtonSurface);
}

/* PANEL CLASS */

/* Panels contain buttons. There can be no buttons without a Panel container. 
* Panels do not belong to a screen. They are created inside the screen's run() function.
*/
export class Panel {
	public:
		// constructor
		Panel(SDL_Rect incomingRect, vector<Button> incomingButtons) {
			rect = incomingRect;
			buttons = incomingButtons;
			mouseOver = false;
			show = false;
		}

		SDL_Rect getRect() { return rect; }
		vector<Button> getButtons() { return buttons; }

		// check if mouse location has hit the panel
		bool isInPanel(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }
		void setMouseOver(bool incomingMouseOver) { mouseOver = incomingMouseOver; }
		bool getShow() { return show; }
		void setShow(bool incomingShow) { show = incomingShow; }
		void checkMouseOver(int mouseX, int mouseY) {		
			if (isInRect(getRect(), mouseX, mouseY)) {
				// Panel mouseOver helps manage button mouseOver
				mouseOver = true;
				for (Button &button : buttons) {
					button.setMouseOver(button.isInButton(mouseX, mouseY));
				}
			}
			else if (mouseOver) {
				// this checks if mouse recently LEFT the panel, and resets all buttons to "false"
				mouseOver = false;
				// change button mouseOver to false.
				for (Button &button : buttons) {
					button.setMouseOver(false);
				}
			}
		}

		ButtonClickStruct checkButtonClick(int mouseX, int mouseY) {
			for (Button &button : buttons) {
				if (button.isInButton(mouseX, mouseY)) {
					cout << "Button clicked!\n";
					return button.getClickStruct();
				}
			}
			return ButtonClickStruct();
		}

	private:
		SDL_Rect rect;
		vector<Button> buttons;
		bool mouseOver;
		bool show;
		// color? bg image? No. If a panel needs a BG image we can do that in the screen. Most will NOT have it.
};


/* Stores initial data to assist construction of the panel.
* Panel uses this data to help construct itself.
* Then buttons use this data AND panel data to construct THEMselves. */
struct PreButtonStruct {
	int textRectWidth;
	int textRectHeight;
	string text;
	ButtonClickStruct clickStruct;

	PreButtonStruct(int iWidth, int iHeight, string iText, ButtonClickStruct iClickStruct) {
		textRectWidth = iWidth;
		textRectHeight = iHeight;
		text = iText;
		clickStruct = iClickStruct;
	}
};


/* Extra UI member functions */

/* Buttons need their parent panel rect before they can be built.
* Yet panel rects need information about their child buttons before they can be built.
* So we build PART of the button first, for the panel rects which let us finish the buttons.*/
PreButtonStruct UI::buildPreButtonStruct(string text, ButtonOption buttonOption, int optionID) {
	int textRectWidth, textRectHeight;
	// get height and width of text based on string (set those values into ints)
	TTF_SizeUTF8(buttonFont, text.c_str(), &textRectWidth, &textRectHeight);
	return PreButtonStruct(textRectWidth, textRectHeight, text, ButtonClickStruct(buttonOption, optionID));
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
		windowHeight - panelHeight,
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

		/* RELATIVE rect to be later painted onto the button itself (when fed into the constructor)*/
		SDL_Rect thisTextRect = {
			// text x position is HALF of the difference b/w button width and text width
			(thisButtonRect.w - preButtonStructs[i].textRectWidth) / 2,
			BUTTON_PADDING,
			preButtonStructs[i].textRectWidth,
			preButtonStructs[i].textRectHeight
		};

		// HERE we need to make the SURFACE and send it in.

		Button thisButton = Button(
			thisButtonRect,
			thisTextRect,
			preButtonStructs[i].text,
			buttonFont,
			colorsByFunction,
			mainRenderer,
			preButtonStructs[i].clickStruct
		);
		buttons.push_back(thisButton);

		// increment heightSoFar
		heightSoFar += thisButtonRect.h + PANEL_PADDING;
	}

	return buttons;
}

/* UI EXTRA FUNCTIONS */


/* Prepare the color scheme for the whole app to use */
void UI::prepareColors() {
	/*	NESTED COLOR SCHEME:
	*		colors are stored internally by name.
	*		colorsByFunction access the colorsByName values.
	*		colorsByFunction are accessible to the outside.
	*		This allows to easily change the colors in this function, without having to change
	*		the accessors at all.
	*/

	colorsByName["GOLD"] = { 255,214, 10 }; // bright yellowish
	colorsByName["ONYX"] = { 14, 14, 14 }; // almost black
	colorsByName["FERN_GREEN"] = { 79, 119, 45 };
	colorsByName["PERIDOT"] = { 242, 222, 6 }; // yellow orangey
	colorsByName["MIKADO_YELLOW"] = { 255, 195, 0 };
	colorsByName["YALE_BLUE"] = { 0, 53, 102 }; // slightly lighter blue
	colorsByName["OXFORD_BLUE"] = { 0, 29, 61 }; // slightly darker blue
	colorsByName["FRENCH_BLUE"] = { 0, 99, 191 }; // almost light (normal) blue-green
	colorsByName["RICH_BLACK"] = { 0, 8, 20 }; // blue tinted dark
	colorsByName["BLACK"] = { 3, 3, 3 }; // almost absolute... not quite
	colorsByName["WHITE"] = { 250, 250, 250 }; // almost white
	colorsByName["PAPAYA_WHIP"] = { 253, 240, 213 }; // beige
	colorsByName["SCARLET"] = { 193, 18, 31 }; // red
	colorsByName["WOODLAND"] = { 97, 89, 30 }; // brown-green
	colorsByName["SMOKEY_GREY"] = { 117, 117, 113 };

	colorsByFunction["BTN_HOVER_BG"] = colorsByName["PERIDOT"];
	colorsByFunction["BTN_BG"] = colorsByName["FRENCH_BLUE"];
	colorsByFunction["BTN_BRDR"] = colorsByName["YALE_BLUE"];
	colorsByFunction["DARK_TEXT"] = colorsByName["BLACK"];
	colorsByFunction["LIGHT_TEXT"] = colorsByName["WHITE"];
	colorsByFunction["WARNING_RED"] = colorsByName["SCARLET"];
	colorsByFunction["LOGO_COLOR"] = colorsByName["PERIDOT"];
	colorsByFunction["OK_GREEN"] = colorsByName["FERN_GREEN"];
	colorsByFunction["BTN_TEXT"] = colorsByName["GOLD"];
}

void UI::getAndStoreWindowSize() {
	// set screen size ints based on new resolution, for the sake of building the UI.
	int w, h;
	SDL_GetWindowSize(mainWindow, &w, &h);
	setWindowSize(w, h);
}

/* Initialize all the SDL */
bool UI::initialize() {
	Resources& resources = Resources::getInstance();

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

	/* Load main window, surface, and renderer
	* GAME will be FULL SCREEN when finished.
	* We will offer WINDOW MODE in the settings, and various resolutions.
	* Keeping two versions of the function here to set pre-defined size OR to make full-screen.
	* PURPOSE: so I can develop for desktop and mobile, faking a mobile resolution.
	* FULL SCREEN is the DEFAULT.
	 */
	if (windowResType == WindowResType::Fullscreen) {
		mainWindow = SDL_CreateWindow(resources.getTitle().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
		getAndStoreWindowSize();
	} else {
		mainWindow = SDL_CreateWindow(resources.getTitle().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	}
	


	if (!mainWindow) {
		SDL_Log("Window failed to load. SDL_Error: %s\n", SDL_GetError());
		cerr << "Window failed to load. SDL_Error: " << SDL_GetError() << std::endl;
		return false;
	}

	// make window resizable
	SDL_SetWindowResizable(mainWindow, SDL_FALSE);

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

	titleFont = TTF_OpenFont("assets/ander_hedge.ttf", 84);

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
	Resources& resources = Resources::getInstance();
	// preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical)
	preButtonStructs = {
		buildPreButtonStruct(resources.getButtonText("NEW_GAME"), ButtonOption::NewGame),
		buildPreButtonStruct(resources.getButtonText("LOAD_GAME"), ButtonOption::LoadGame),
		buildPreButtonStruct(resources.getButtonText("SETTINGS"), ButtonOption::Settings),
		buildPreButtonStruct(resources.getButtonText("ABOUT"), ButtonOption::About),
		buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit)
	};

	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	return Panel(panelRect, buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect));
}

/*
* Settings available in every screen.
*/
Panel UI::createSettingsPanel() {
	vector<PreButtonStruct> preButtonStructs;
	Resources& resources = Resources::getInstance();
	// preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical)
	preButtonStructs = {
		buildPreButtonStruct(resources.getButtonText("MOBILE"), ButtonOption::Mobile),
		buildPreButtonStruct(resources.getButtonText("TABLET"), ButtonOption::Tablet),
		buildPreButtonStruct(resources.getButtonText("DESKTOP"), ButtonOption::Desktop),
		buildPreButtonStruct(resources.getButtonText("FULLSCREEN"), ButtonOption::Fullscreen),
		buildPreButtonStruct(resources.getButtonText("BACK"), ButtonOption::Back)
	};

	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	return Panel(panelRect, buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect));
}



// because SDL2 is very picky about its colors (and there are a trillion color formats)
Uint32 convertSDL_ColorToUint32(const SDL_PixelFormat* format, SDL_Color color) {
	return SDL_MapRGB(
		format,
		color.r,
		color.g,
		color.b);
}

/* Check if an x/y location falls within a rect */
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


/*
* STILL TO COME:
*	Main Menu Sub-Panels : Settings Menu & Load Game Menu
*	Battle Panel (and possibly battle Sub-Panels)
*	Map Panel (no idea what will go in here!)
*	Character Creation Panels
*/