/*
* Basic UI elements.
* This should NOT be specific to any particular Screen.
* All Screen modules should use this UI module.
* The UI class will be a singleton which is shared between all Screen modules.
* 
* So a SCREEN can have PANELS, and PANELS can have BUTTONS (no panels outside of screens, no buttons outside of panels).
* Panel and Button classes will be exported separately from the UI object.
* 
* This module will also include functions to check if a screen's panel's buttons are being hovered over or clicked.
* -- OR -- the SCREEN class (different module?) will have those functions.
* That might seem odd, to have classes from one module affected by functions from a different class/module.
* 
* That's something to think about. Where do Panel and Button classes belong?
* 
* If this is really basic stuff that gets used everywhere, then yeah maybe Panel and Button should be in the Screen module.
* In fact, I don't even need to export Button or Panel.
* The Screen class will have functions to run through each Panel's Buttons.
* 
* There are some Panels which should be available to multiple Screens.
* 
* A Screen object will have a collection of Panels.
* A Panel object will have a collection of Buttons.
* 
* So do we need a factory in the UI module?
* And does it have the functions for EACH Panel (with its buttons?)
* YES.
* Now... how do we apply functionality to the buttons?
* Maybe we make our own event queue?
* 
* 
* ACtual PANELS will be created in a PanelFactory module
* 
* CLASSES TO CREATE:
* Panel
* Button
* 
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

// Make this a SINGLETON
export class UI {

	public:
		//constructor
		UI() {
			initialized = initialize();
		}

		bool isInitialized() { return initialized; }
		SDL_Renderer* getMainRenderer() { return mainRenderer; }
		SDL_Surface* getWindowSurface() { return mainWindowSurface; }
		SDL_Window* getMainWindow() { return mainWindow; }
		TTF_Font* getButtonFont() { return buttonFont; }
		SDL_Color getTextColor() { return textColor; }

	private:

		const int SCREEN_WIDTH = 720;
		const int SCREEN_HEIGHT = 960;

		int initialized = false;

		SDL_Window* mainWindow = NULL;
		SDL_Renderer* mainRenderer = NULL;
		SDL_Surface* mainWindowSurface = NULL;

		TTF_Font* titleFont = NULL;
		TTF_Font* buttonFont = NULL;
		TTF_Font* bodyFont = NULL;
		TTF_Font* dialogFont = NULL;

		SDL_Color textColor = { 25, 25, 25 };

		bool initialize() {

			// Initialize SDL
			if (SDL_Init(SDL_INIT_VIDEO) < 0) {
				SDL_Log("SDL failed to initialize. SDL_Error: %s\n", SDL_GetError());
				std::cerr << "SDL failed to initialize. SDL_Error: " << SDL_GetError() << std::endl;
				return false;
			}

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

			// Initialize Image library (though we're not using any images yet)

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

// Buttons contain a Rect, but also must contain their (anonymous) action function.
// Buttons make a Rect clickable and allow us to perform other actions on them.
// We need a TEXT rect for the text.
// -- the textRect should have a constant border on top and bottom, but the width should be based on the length of the text string.
// I'm sending in strings to convert to char... we should START with char instead.
export class Button {
	public:
		// constructor
		Button(int x, int y, int w, int h, string incomingText, TTF_Font* buttonFont) {
			rect = { x, y, w, h };
			text = incomingText;
			setTextRect(rect, text, buttonFont);
			// Somehow we must pass in a function as an action
			// use the Standard library header <functional> for lambdas and anoymous fuctions
		}

		// Might turn this private since we should only operate on it internally
		SDL_Rect getRect() { return rect; }
		SDL_Rect getTextRect() { return textRect; }
		string getText() { return text; } // turn this completely into char for the printing

		// check if mouse location has hit the panel
		bool isInButton(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }


	private:
		SDL_Rect rect;
		SDL_Rect textRect;
		string text = "";

		void setTextRect(SDL_Rect buttonRect, string buttonText, TTF_Font* buttonFont) {
			rect = buttonRect;
			
			// I need to set size and position of the textRect within the buttonRect
			// I can use     int TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h);
			// This fills w & h with the width and height
			// and I can use THAT to position the thing.
			// But I need the actual font from the UI object.

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


export class Panel {
	public:
		// constructor
		Panel(int x, int y, int w, int h, vector<Button> incomingButtons) {
			rect = { x, y, w, h };
			buttons = incomingButtons;
		}

		// Might turn this private since we should only operate on it internally
		SDL_Rect getRect() { return rect; }

		vector<Button> getButtons() { return buttons; }

		// check if mouse location has hit the panel
		bool isInPanel(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }

	private:
		SDL_Rect rect;
		vector<Button> buttons;
		// color? bg image?
};

// Instead of factories, I'm just creating a function to deliver particular panels
// maybe factories will prove useful once I see that these are repeated code?

// Main Menu
// send in anonymous functions as arguments / demand anon. funcs as params
// Possibly a panel will need a parent rect? so we know how large it can be?
// YES this WILL need a parent rect, so we can know the X,Y.
// Also need COLORS. But not yet.
export Panel createMainMenuPanel(TTF_Font* buttonFont) {
	vector<Button> buttons;

	int x = 0;
	int y = 250;
	int w = 200;
	int h = 84;

	// now make the BUTTONS

	int newGameButtonX = x;
	int newGameButtonY = y;
	int newGameButtonW = w;
	int newGameButtonH = h;
	string newGameButtonText = "New Game";

	Button newGameButton = Button(
		newGameButtonX,
		newGameButtonY,
		newGameButtonW,
		newGameButtonH,
		newGameButtonText,
		buttonFont
	);

	/* OTHER BUTTONS WILL INCLUDE :
	* ---load game button
	* ---settings button? (much later!)
	*/

	buttons = { newGameButton };
	Panel menuPanel = Panel(x, y, w, h, buttons);
	return menuPanel;
}


