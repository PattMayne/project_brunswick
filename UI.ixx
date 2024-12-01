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

#include <cmath>
#include <time.h>
#include<cstdlib> /* Needed for rand() and srand() */
#include<ctime>   /* Needed for time() */

export module UI;

using namespace std;

import Resources;
import ScreenType;
class Panel;
class Button;
struct PreButtonStruct;

Uint32 convertSDL_ColorToUint32(const SDL_PixelFormat* format, SDL_Color color);
bool isInRect(SDL_Rect rect, int mouseX, int mouseY);
SDL_Surface* flipSurface(SDL_Surface* surface, bool horizontal);


/*
*
*
*
*
*						UI CLASS
*
*
*
*
*
*/

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
		void rebuildMainMenuPanel(Panel& mainMenuPanel);

		Panel createSettingsPanel();
		void rebuildSettingsPanel(Panel& settingsPanel);

		int getWindowHeight() { return windowHeight; }
		int getWindowWidth() { return windowWidth; }
		void resizeWindow(WindowResType newResType);
		void refreshFonts(Resources& resources);

		unordered_map<string, SDL_Color> getColorsByFunction() { return colorsByFunction; }
		


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

		int buttonPadding = 10;
		const int PANEL_PADDING = 10;

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

		// settings panel building functions
		tuple<SDL_Rect, vector<Button>> createSettingsPanelComponents();
		vector<PreButtonStruct> getSettingsPreButtonStructs();

		// main menu panel building functions
		tuple<SDL_Rect, vector<Button>> createMainMenuPanelComponents();
		vector<PreButtonStruct> getMainMenuPreButtonStructs();

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
		bool initializeFonts(Resources& resources);
};



/*
*
*
*
*
*						BUTTON CLASS
*
*
*
*
*
*/

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
		SDL_Surface* createButtonSurfaceBG(SDL_Rect buttonRect, SDL_Color color, SDL_Color overlayColor, SDL_Renderer* mainRenderer, vector<SDL_Rect> overlayRects);
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

//SDL_Rect getRandomRect(int xBlock, int yBlock) {
//	int maxWidth = 7 * xBlock;
//	int maxHeight = 3 * yBlock;
//	int width = (rand() * maxWidth) + xBlock;
//	int height = (rand() * maxHeight) + yBlock;
//	return { 0, 0, xBlock, yBlock };
//}


/* Generate a randomized overlay for button backgrounds */
vector<SDL_Rect> createButtonSurfaceOverlay(SDL_Rect bgRect) {
	/*
	* TODO: Rework this to be useful for larger objects, including the background of the main menu.	
	*/

	/* two to five rects */
	int numberOfRects = (rand() % 5) + 2;

	/* we will store all the rects here and later draw them */
	vector<SDL_Rect> rects;

	// Pick a random starting point
	enum StartingRect { WholeSide, Corner, Top };
	int startingRectNumber = (rand() % 3) + 1;
	StartingRect startingRect = startingRectNumber == 3 ? WholeSide :
		startingRectNumber == 2 ? Corner : Top;

	/* not totally random dimensions. Always in increments, defined as blocks (of pixels) based on resolution. */
	int xRes = 10;
	int yRes = 4;

	/* resolution */

	/* Minimum sizes */
	int blockWidth = bgRect.w / xRes;
	int blockHeight = bgRect.h / yRes;

	/* create a map of false bools corresponding to a grid over the buttons */
	vector<vector<bool>> rows(yRes);

	for (int i = 0; i < yRes; ++i) {
		vector<bool> blocks(xRes);
		for (int k = 0; k < xRes; ++k) {
			blocks[k] = false;
		}
		rows[i] = blocks;
	}

	/* pick a max number of blocks to color */
	int totalBlocks = xRes * yRes;
	int minBlocks = totalBlocks / 2;
	int maxBlocks = (totalBlocks / 4) * 3;
	int seeds = (rand() % (totalBlocks - maxBlocks)) + minBlocks + 1;

	/* get any remaining height & width to tack onto edge blocks */
	int widthOfAllBlocks = xRes + blockWidth;
	int heightOfAllBlocks = yRes * blockHeight;
	int remainderX = bgRect.w - widthOfAllBlocks;
	int remainderY = bgRect.h - heightOfAllBlocks;

	/* NON-RANDOM START: Fill a few blocks on the left. */
	int columnsToFill = 3;
	for (int i = 0; i < rows.size(); ++i) {
		for (int k = 0; k < columnsToFill; ++k) {
			/* add the block and decrement the seeds */
			rows[i][k] = true;
			SDL_Rect newRect = {
				k * blockWidth,
				i * blockHeight,
				blockWidth,
				blockHeight
			};

			/* If it's the last block, make sure it reaches the edge */
			if (i == yRes - 1) { newRect.h += remainderY; }
			if (k == xRes - 1) { newRect.w += remainderX; }

			rects.push_back(newRect);
			--seeds;
		}
	}

	/* Start the path on the top left. */
	int currX = columnsToFill;
	int currY = 0;

	rows[currY][currX] = true;
	SDL_Rect firstRect = {
		currX * blockWidth,
		currY * blockHeight,
		blockWidth,
		blockHeight
	};
	rects.push_back(firstRect);
	--seeds;

	/* create the rest of the rects by crawling around and turning blocks true */

	enum Direction { Up, Down, Left, Right, NoDirection};

	while (seeds > 0) {

		/* check how many directions are available */
		vector<Direction> availableDirections;
		bool nowhereToGo = true;

		/* look UP */
		if (currY > 0 && rows[currY -1][currX] == false) {
			availableDirections.push_back(Direction::Up);
			nowhereToGo = false;
		}

		/* look DOWN */
		if (currY < (yRes - 2) && rows[currY + 1][currX] == false) {
			availableDirections.push_back(Direction::Down);
			nowhereToGo = false;
		}

		/* look LEFT */
		if (currX > 0 && rows[currY][currX - 1] == false) {
			availableDirections.push_back(Direction::Left);
			nowhereToGo = false;
		}

		/* look RIGHT */
		if (currX < (xRes - 1) && rows[currY][currX + 1] == false) {
			availableDirections.push_back(Direction::Right);
			nowhereToGo = false;
		}

		/* If we can't move, break the loop */
		if (nowhereToGo) {
			seeds = 0;
			break;
		}

		int numberOfAvailableDirections = (unsigned int)availableDirections.size();

		Direction newDirection = Direction::NoDirection;

		if (numberOfAvailableDirections == 1) {
			newDirection = availableDirections[0];
		}
		else {
			/* Pick a direction from those available */
			int directionIndex = rand() % numberOfAvailableDirections;
			newDirection = availableDirections[directionIndex];
		}

		/* If we can't move, break the loop */
		if (newDirection == Direction::NoDirection) {
			seeds = 0;
			break;
		}		

		/* move in that direction (change current X and Y) */
		switch (newDirection) {
			case Direction::Up:
				--currY;
				break;
			case Direction::Down:
				++currY;
				break;
			case Direction::Right:
				++currX;
				break;
			case Direction::Left:
				--currX;
				break;
			default:
				currY = -1;
				currX = -1;
				break;
		}

		/* just double check that we really moved */
		if (currY < 0 || currX < 0) {
			seeds = 0;
			break;
		}

		/* add the block and decrement the seeds */
		rows[currY][currX] = true;
		SDL_Rect newRect = {
			currX * blockWidth,
			currY * blockHeight,
			blockWidth,
			blockHeight
		};

		/* If it's the last block, make sure it reaches the edge */
		if (currY == yRes - 1) { newRect.h += remainderY; }
		if (currX == xRes - 1) { newRect.w += remainderX; }

		rects.push_back(newRect);
		--seeds;
	}

	return rects;
}

/* Button textures must start with a surface. Create and get it here. */
SDL_Surface* Button::createButtonSurfaceBG(
	SDL_Rect buttonRect,
	SDL_Color color,
	SDL_Color overlayColor,
	SDL_Renderer* mainRenderer,
	vector<SDL_Rect> overlayRects
) {
	UI& ui = UI::getInstance();
	SDL_Surface* buttonSurface = SDL_CreateRGBSurface(0, buttonRect.w, buttonRect.h, 32, 0, 0, 0, 0xFF000000);
	// fill the rects with beautiful color
	SDL_FillRect(buttonSurface, NULL, convertSDL_ColorToUint32(buttonSurface->format, color));

	/* draw the overlay */
	if (overlayRects.size() > 0) {
		for (SDL_Rect rect : overlayRects) {
			SDL_FillRect(buttonSurface, &rect, convertSDL_ColorToUint32(buttonSurface->format, overlayColor));
		}
	}	

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

	/* make the overlay which normal and hover buttons will share */
	vector<SDL_Rect> overlayRects = createButtonSurfaceOverlay(buttonRect);

	/* make two button surfaces with the correct BG colors, to blit the text surfaces onto */
	SDL_Surface* hoverButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_HOVER_BG"], colors["BTN_HOVER_BRDR"], mainRenderer, overlayRects);
	SDL_Surface* normalButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_BG"], colors["BTN_BRDR"], mainRenderer, overlayRects);

	/* possibility of flipping */
	int hFlipInt = rand() % 2;
	int vFlipInt = rand() % 2;

	if (hFlipInt == 0) {
		hoverButtonSurface = flipSurface(hoverButtonSurface, true);
		normalButtonSurface = flipSurface(normalButtonSurface, true);
	}

	if (vFlipInt == 0) {
		hoverButtonSurface = flipSurface(hoverButtonSurface, false);
		normalButtonSurface = flipSurface(normalButtonSurface, false);
	}

	/* blit the text */
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

/* 
* 
* 
* 
* 
*						PANEL CLASS
* 
* 
* 
* 
* 
*/

/* Panels contain buttons. There can be no buttons without a Panel container. 
* Panels do not belong to a screen. They are created inside the screen's run() function.
*/
export class Panel {
	public:
		/* constructor */
		Panel(SDL_Rect incomingRect, vector<Button> incomingButtons) {
			rect = incomingRect;
			buttons = incomingButtons;
			mouseOver = false;
			show = false;
		}

		SDL_Rect getRect() { return rect; }
		vector<Button> getButtons() { return buttons; }

		/* check if mouse location has hit the panel */
		bool isInPanel(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }
		void setMouseOver(bool incomingMouseOver) { mouseOver = incomingMouseOver; }
		bool getShow() { return show; }
		void setShow(bool incomingShow) { show = incomingShow; }
		void checkMouseOver(int mouseX, int mouseY) {		
			if (isInRect(getRect(), mouseX, mouseY)) {
				/* Panel mouseOver helps manage button mouseOver */
				mouseOver = true;
				for (Button &button : buttons) {
					button.setMouseOver(button.isInButton(mouseX, mouseY));
				}
			}
			else if (mouseOver) {
				/* this checks if mouse recently LEFT the panel, and resets all buttons to "false" */
				mouseOver = false;
				/* change button mouseOver to false. */
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

		void rebuildSelf(SDL_Rect incomingRect, vector<Button> incomingButtons) {
			rect = incomingRect;
			buttons = incomingButtons;
		}

	private:
		SDL_Rect rect;
		vector<Button> buttons;
		bool mouseOver;
		bool show;
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


/*
*
*
*
*
*						EXTRA MEMBER FUNCTIONS FOR UI CLASS
*
*
*
*
*
*/

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
		panelHeight += preButtonStruct.textRectHeight + (buttonPadding * 2) + PANEL_PADDING;

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
		longestButtonTextLength + (buttonPadding * 2) + (PANEL_PADDING * 2),
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
		/* start at the top of the panelRect PLUS panelPadding */

		SDL_Rect thisButtonRect = {
			xForAll,
			heightSoFar,
			widthForAll,
			preButtonStructs[i].textRectHeight + (buttonPadding * 2)
		};

		/* RELATIVE rect to be later painted onto the button itself (when fed into the constructor)*/
		SDL_Rect thisTextRect = {
			// text x position is HALF of the difference b/w button width and text width
			(thisButtonRect.w - preButtonStructs[i].textRectWidth) / 2,
			buttonPadding,
			preButtonStructs[i].textRectWidth,
			preButtonStructs[i].textRectHeight
		};

		buttons.push_back(
			Button(
				thisButtonRect,
				thisTextRect,
				preButtonStructs[i].text,
				buttonFont,
				colorsByFunction,
				mainRenderer,
				preButtonStructs[i].clickStruct));

		/* increment heightSoFar */
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

	/* THEME COLORS */

	/*  Three yellows might seem crazy but it's good, actually.
	* I will probably also add a third, lighter blue as well */
	colorsByName["PERIDOT"] = { 242, 222, 6 };  /* THEME */ /* yellow orangey */
	colorsByName["VIVID_YELLOW"] = { 217, 168, 6 }; /* THEME */ /* brighter yellow */
	colorsByName["GOLD"] = { 255,214, 10 }; /* bright yellowish */
	colorsByName["YALE_BLUE"] = { 0, 53, 102 }; /* slightly lighter blue */
	colorsByName["OXFORD_BLUE"] = { 0, 29, 61 }; /* slightly darker blue */

	/* NOT SURE YET */
	colorsByName["ONYX"] = { 14, 14, 14 }; // almost black
	colorsByName["FERN_GREEN"] = { 79, 119, 45 };
	colorsByName["MIKADO_YELLOW"] = { 255, 195, 0 };
	colorsByName["FRENCH_BLUE"] = { 0, 99, 191 }; // almost light (normal) blue-green
	colorsByName["RICH_BLACK"] = { 0, 8, 20 }; // blue tinted dark
	colorsByName["BLACK"] = { 3, 3, 3 }; // almost absolute... not quite
	colorsByName["WHITE"] = { 250, 250, 250 }; // almost white
	colorsByName["PAPAYA_WHIP"] = { 253, 240, 213 }; // beige
	colorsByName["SCARLET"] = { 193, 18, 31 }; // red
	colorsByName["WOODLAND"] = { 97, 89, 30 }; // brown-green
	colorsByName["SMOKEY_GREY"] = { 117, 117, 113 };

	/* COLORS BY FUNCTION */
	colorsByFunction["BTN_HOVER_BG"] = colorsByName["PERIDOT"];
	colorsByFunction["BTN_BG"] = colorsByName["OXFORD_BLUE"];
	colorsByFunction["BTN_HOVER_BRDR"] = colorsByName["VIVID_YELLOW"];
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
		mainWindow = SDL_CreateWindow(
			resources.getTitle().c_str(),
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			windowWidth,
			windowHeight,
			SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
		getAndStoreWindowSize();
	} else {
		mainWindow = SDL_CreateWindow(resources.getTitle().c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
	}
	
	/* Window cannot be arbitrarily resized by user. Only by clicking buttons in settings menu. */
	SDL_SetWindowResizable(mainWindow, SDL_FALSE);

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
		
	return initializeFonts(resources);
}

/* load the fonts in the correct sizes for their various contexts */
bool UI::initializeFonts(Resources& resources) {
	buttonPadding = resources.getButtonBorder(mainWindowSurface->w);

	int titleFontSize = resources.getFontSize(FontContext::Title, mainWindowSurface->w);
	int bodyFontSize = resources.getFontSize(FontContext::Body, mainWindowSurface->w);
	int buttonFontSize = resources.getFontSize(FontContext::Button, mainWindowSurface->w);
	int dialogFontSize = resources.getFontSize(FontContext::Dialog, mainWindowSurface->w);

	/* Initialize TTF font library */
	if (TTF_Init() == -1) {
		SDL_Log("WTTF failed to initialize. TTF_Error: %s\n", TTF_GetError());
		std::cerr << "TTF failed to initialize. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	/* Load the fonts */

	titleFont = TTF_OpenFont("assets/ander_hedge.ttf", titleFontSize);

	if (!titleFont) {
		SDL_Log("Font (ander_hedge) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (ander_hedge) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	buttonFont = TTF_OpenFont("assets/alte_haas_bold.ttf", buttonFontSize);

	if (!buttonFont) {
		SDL_Log("Font (alte_haas_bold) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (alte_haas_bold) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	bodyFont = TTF_OpenFont("assets/alte_haas.ttf", bodyFontSize);

	if (!bodyFont) {
		SDL_Log("Font (alte_haas) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (alte_haas) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	dialogFont = TTF_OpenFont("assets/la_belle_aurore.ttf", dialogFontSize);

	if (!dialogFont) {
		SDL_Log("Font failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	return true;
}

/* when screen is resized, reload the fonts with new sizes */
void UI::refreshFonts(Resources& resources) {
	/* close existing fonts */
	TTF_CloseFont(titleFont);
	TTF_CloseFont(buttonFont);
	TTF_CloseFont(bodyFont);
	TTF_CloseFont(dialogFont);
	/* re-initialize based on new screen size */
	initializeFonts(resources);
}

/* Prevent arbitrary resizing by selecting from a list of possible resolutions.
	Fullscreen is always an arbitrary option, but won't be done randomly all the time.
	Screen object will take care of recalculating objects on the screen.
	This is only about resizing the window itself.
*/
void UI::resizeWindow(WindowResType newResType) {
	Resources& resources = Resources::getInstance();
	if (newResType == WindowResType::Fullscreen) {
		SDL_SetWindowFullscreen(mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);	}
	else {
		/* turn OFF fullscreen in case it's already set (0 means windowed mode) */
		SDL_SetWindowFullscreen(mainWindow, 0);

		/* get new requested dimensions */
		Resolution newRes = resources.getDefaultDimensions(newResType);

		/* set new requested dimensions to the window.
		Make window resizable, then resize, then make it NOT resizable */
		SDL_SetWindowResizable(mainWindow, SDL_TRUE);
		SDL_SetWindowSize(mainWindow, newRes.w, newRes.h);
		SDL_SetWindowResizable(mainWindow, SDL_FALSE);
	}
	/* destroy old surface and get new one with correct size */
	SDL_FreeSurface(mainWindowSurface);
	mainWindowSurface = SDL_GetWindowSurface(mainWindow);
	/* update dimension data for rebuilding the interface */
	getAndStoreWindowSize();
	refreshFonts(resources);
}

/* 
* PANEL CREATION FUNCTIONS
* 
* These are member functions of the UI class.
* Defining them in the class definition would be too bulky.
* 
* We can have VERTICAL panels (going all up the side)
* and HORIZONTAL panels (spread across the bottom)
*/



/*
* 
* 
* 
* 
*								MENU SCREEN PANELS CREATION AND RE-BUILDING 
* 
* 
* 
* 
* 
* Panel for the Main Menu Screen.
*/

/* build and deliver basic info for main menu panel buttons */
vector<PreButtonStruct> UI::getMainMenuPreButtonStructs() {
	Resources& resources = Resources::getInstance();
	/* preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	return {
		buildPreButtonStruct(resources.getButtonText("NEW_GAME"), ButtonOption::NewGame),
		buildPreButtonStruct(resources.getButtonText("LOAD_GAME"), ButtonOption::LoadGame),
		buildPreButtonStruct(resources.getButtonText("SETTINGS"), ButtonOption::Settings),
		buildPreButtonStruct(resources.getButtonText("ABOUT"), ButtonOption::About),
		buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit)
	};
}

/* create all the components for the settings panel */
tuple<SDL_Rect, vector<Button>> UI::createMainMenuPanelComponents() {
	vector<PreButtonStruct> preButtonStructs = getMainMenuPreButtonStructs();
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}

/*
* Main screen, main menu
*/
Panel UI::createMainMenuPanel() {
	auto [panelRect, buttons] = createMainMenuPanelComponents();
	return Panel(panelRect, buttons);
}


/* rebuild the main menu panel after resize */
void UI::rebuildMainMenuPanel(Panel& mainMenuPanel) {
	auto [panelRect, buttons] = createMainMenuPanelComponents();
	mainMenuPanel.rebuildSelf(panelRect, buttons);
}

/* SETTINGS PANEL CREATION AND RE-BUILDING */


/* build and deliver basic info for settings panel buttons */
vector<PreButtonStruct> UI::getSettingsPreButtonStructs() {
	Resources& resources = Resources::getInstance();
	/* preButonStructs don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	vector<PreButtonStruct> preButtonStructs = {
		buildPreButtonStruct(resources.getButtonText("MOBILE"), ButtonOption::Mobile),
		buildPreButtonStruct(resources.getButtonText("TABLET"), ButtonOption::Tablet),
		buildPreButtonStruct(resources.getButtonText("DESKTOP"), ButtonOption::Desktop),
		buildPreButtonStruct(resources.getButtonText("FULLSCREEN"), ButtonOption::Fullscreen),
		buildPreButtonStruct(resources.getButtonText("BACK"), ButtonOption::Back)
	};

	return preButtonStructs;
}

/* create all the components for the settings panel */
tuple<SDL_Rect, vector<Button>> UI::createSettingsPanelComponents() {
	vector<PreButtonStruct> preButtonStructs = getSettingsPreButtonStructs();
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons};
}

/*
* Settings available in every screen.
*/
Panel UI::createSettingsPanel() {
	auto [panelRect, buttons] = createSettingsPanelComponents();
	return Panel(panelRect, buttons);
}


/* rebuild the settings panel after resize */
void UI::rebuildSettingsPanel(Panel& settingsPanel) {
	auto [panelRect, buttons] = createSettingsPanelComponents();
	settingsPanel.rebuildSelf(panelRect, buttons);
}




/*
* 
* 
*			GENERAL HELPER FUNCTIONS
* 
* 
* 
* 
*/



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


/* whenever we want to flip a surface */
SDL_Surface* flipSurface(SDL_Surface* surface, bool horizontal) {
	/* new surface onto which we'll flip the original surface */
	SDL_Surface* flippedSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0xFF000000);

	/* error check */
	if (!flippedSurface) {
		cout << "\n\n\nERROR!!!\n\n\n";
		cerr << "Failed to create surface: " << SDL_GetError() << endl;
		return surface;
	}

	/* Pointer to the pixel data of the original and flipped surfaces */
	Uint32* originalPixels = static_cast<Uint32*>(surface->pixels);
	Uint32* flippedPixels = static_cast<Uint32*>(flippedSurface->pixels);

	/* Loop through the original surface's pixels */
	for (int y = 0; y < surface->h; ++y) { /* every row */
		for (int x = 0; x < surface->w; ++x) { /* every column */
			int srcX = horizontal ? (surface->w - 1 - x) : x;
			int srcY = horizontal ? y : (surface->h - 1 - y);
			flippedPixels[y * flippedSurface->w + x] = originalPixels[srcY * surface->w + srcX];
		}
	}

	SDL_FreeSurface(surface);
	return flippedSurface;
}

/*
* STILL TO COME:
*	Main Menu Sub-Panels : Load Game Menu & ABOUT panel (BACK button and TEXT display)
*	Battle Panel (and possibly battle Sub-Panels)
*	Map Panel (no idea what will go in here!)
*	Character Creation Panels
*/