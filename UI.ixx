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

export module UI;

import  "include/json.hpp";
import  "SDL.h";
import  "SDL_image.h";
import  "SDL_ttf.h";
import  <stdio.h>;
import  <string>;
import  <iostream>;
import  <vector>;
import  <unordered_map>;
import  <cmath>;
import  <time.h>;
import <cstdlib>; /* Needed for rand() and srand() */
import <ctime>;   /* Needed for time() */
import Resources;
import TypeStorage;

using namespace std;

class Panel;
class Button;
struct PreButtonStruct;

Uint32 convertSDL_ColorToUint32(const SDL_PixelFormat* format, SDL_Color color);
bool isInRect(SDL_Rect rect, int mouseX, int mouseY);
SDL_Surface* flipSurface(SDL_Surface* surface, bool horizontal);
vector<SDL_Rect> createSurfaceOverlay(SDL_Rect bgRect);
SDL_Surface* createTransparentSurface(int w, int h);




/* UI cannot import actual characters or their limbs, so Character object creates this struct and sends IT it. */
export struct CharStatsData {
	CharStatsData(
		string name, int hp,
		int strength, int speed, int intelligence,
		DominanceNode dNode
	) :
		name(name), hp(hp), strength(strength), speed(speed), dNode(dNode),
		intelligence(intelligence) { }

	
	string name;
	int intelligence;
	int strength;
	int hp;
	int speed;
	DominanceNode dNode;
};

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

		/* MAIN MENU PANEL. */
		Panel createMainMenuPanel();
		void rebuildMainMenuPanel(Panel& mainMenuPanel);

		Panel createSettingsPanel(ScreenType context = ScreenType::Menu);
		Panel createBattlePanel(vector<AttackStruct> playerAttackStructs, int y = -1);
		void rebuildSettingsPanel(Panel& settingsPanel, ScreenType context = ScreenType::Menu);

		tuple<SDL_Rect, vector<Button>> createGeneralMenuPanelComponents(unordered_map<string, ButtonOption> buttonOptions, bool left = true);
		Panel createGeneralMenuPanel(unordered_map<string, ButtonOption> buttonOptions, bool left);
		vector<PreButtonStruct> getGeneralMenuPreButtonStructs(unordered_map<string, ButtonOption> buttonOptions, bool left);

		Panel createGameMenuPanel(ScreenType context);
		void rebuildGameMenuPanel(Panel& gameMenuPanel, ScreenType context);

		/* CHARACTER CREATION PANELS. */
		Panel createReviewModePanel();
		Panel createLimbLoadedModePanel(
			bool loadedLimbHasExtraJoints, bool characterHasExtraJoints); /* Does this need a limb id? */
		Panel createChooseLimbModePanel(
			vector<LimbButtonData> limbBtnDataStructs, bool drawBackButton, string label, int page = 1);

		/* TO DO: make rebuildPanel functions for Character Creation screen. */

		
		/* PANELS USED BY VARIOUS SCREENS. */

		/* Confirmation Text Panel. */
		Panel createConfirmationPanel(
			string confirmationText,
			ConfirmationButtonType confirmationType = ConfirmationButtonType::OkCancel,
			bool includeRefuseButton = true
		);

		Panel createPassingMessagePanel(string message, bool topPlacement, bool isBold);
		Panel getNewPassingMessagePanel(string newMessage, Panel& oldPassingMessagePanel, bool topPlacement, bool isBold);
		Panel createStatsPanel(ScreenType screenType, CharStatsData statsData, bool topRight = true);
		Panel createTrackerPanel(Point trackedPoint, Point playerPoint, string name);
		Panel createKeyControlsPanel(ScreenType screenType);


		/* OTHER FUNCTIONS. */

		int getPanelPadding() { return PANEL_PADDING; }
		int getWindowHeight() { return windowHeight; }
		int getWindowWidth() { return windowWidth; }
		void resizeWindow(WindowResType newResType);
		void refreshFonts(Resources& resources);
		tuple<SDL_Texture*, SDL_Rect> createTitleTexture(string title); /* get title texture for any screen */
		SDL_Texture* createBackgroundTexture(); /* Background with overlay */
		unordered_map<string, SDL_Color> getColorsByFunction() { return colorsByFunction; }
		bool isInitialized() { return initialized; }
		SDL_Renderer* getMainRenderer() { return mainRenderer; }
		SDL_Surface* getWindowSurface() { return mainWindowSurface; }
		SDL_Window* getMainWindow() { return mainWindow; }
		TTF_Font* getButtonFont() { return buttonFont; }
		TTF_Font* getTitleFont() { return titleFont; }
		TTF_Font* getBodyFont() { return bodyFont; }
		TTF_Font* getMonoFont() { return monoFont; }
		TTF_Font* getMonoFontLarge() { return monoFontLarge; }
		unordered_map<string, SDL_Color> getColors() { return colorsByFunction; }


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

		/* Only for initial display. */
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
		TTF_Font* monoFont = NULL;
		TTF_Font* monoFontLarge = NULL;
		TTF_Font* headlineFont = NULL;
		TTF_Font* headlineFontLarge = NULL;

		PreButtonStruct buildPreButtonStruct(string text, ButtonOption buttonOption, int optionID = -1);
		SDL_Rect buildVerticalPanelRectFromButtonTextRects(vector<PreButtonStruct> preButtonStructs, bool left = true, int y = -1);
		vector<Button> buildButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect);
		vector<Button> buildLimbButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect);

		/* settings panel building functions */
		tuple<SDL_Rect, vector<Button>> createSettingsPanelComponents(ScreenType context = ScreenType::Menu);
		vector<PreButtonStruct> getSettingsPreButtonStructs(ScreenType context = ScreenType::Menu);

		/* Battle panel building functions */
		tuple<SDL_Rect, vector<Button>> createBattlePanelComponents(vector<AttackStruct> playerAttackStructs, int y);
		vector<PreButtonStruct> getBattlePreButtonStructs(vector<AttackStruct> playerAttackStructs);

		
		/* main menu panel building functions */
		tuple<SDL_Rect, vector<Button>> createMainMenuPanelComponents();
		vector<PreButtonStruct> getMainMenuPreButtonStructs();

		/* Map Menu panel building functions */
		tuple<SDL_Rect, vector<Button>> createGameMenuPanelComponents(ScreenType context);
		vector<PreButtonStruct> getGameMenuPreButtonStructs(ScreenType context);

		/* Character Creation panel building functions. */

		tuple<SDL_Rect, vector<Button>> createReviewModePanelComponents();
		tuple<SDL_Rect, vector<Button>> createLimbLoadedModePanelComponents(bool loadedLimbHasExtraJoints, bool characterHasExtraJoints);
		tuple<SDL_Rect, vector<Button>> createChooseLimbModePanelComponents(
			vector<LimbButtonData> limbBtnDataStructs, bool drawBackButton, int labelOffsetY, int page = 1);

		vector<PreButtonStruct> getReviewModePreButtonStructs();
		vector<PreButtonStruct> getLimbLoadedModePreButtonStructs(bool loadedLimbHasExtraJoints, bool characterHasExtraJoints);

		void prepareColors();
		void getAndStoreWindowSize();
		/* when you need to dictate dimensions (for dev purposes) */
		void setWindowSize(int x, int y) {
			windowWidth = x;
			windowHeight = y; }

		// color maps
		unordered_map<string, SDL_Color> colorsByFunction; /* colors by function, which reference colors by name. */
		unordered_map<string, SDL_Color> colorsByName; /* raw colors. */

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
			string text,
			TTF_Font* buttonFont,
			unordered_map<string, SDL_Color> colors,
			SDL_Renderer* mainRenderer,
			ButtonClickStruct clickStruct
		) : text(text), clickStruct(clickStruct), rect(buttonRect)
		{
			mouseOver = false;
			createButtonTextures(buttonRect, textRect, text, buttonFont, colors, mainRenderer);
		}

		/* 
		* CONSTRUCTOR FOR PRE-BUILT TEXTURES.
		* Constructor for when you created the two textures beforehand, and are just handing them in.
		* Specifically for the LIMB BUTTONS, but can be used for others too.
		*/
		Button(SDL_Rect rect, SDL_Texture* hoverTexture, SDL_Texture* normalTexture, string text, ButtonClickStruct clickStruct) :
			rect(rect), hoverTexture(hoverTexture), normalTexture(normalTexture),
			text(text), clickStruct(clickStruct), mouseOver(false) { }

		SDL_Rect getRect() { return rect; }
		string getText() { return text; } // turn this completely into char for the printing
		SDL_Texture* getHoverTexture() { return hoverTexture; }
		SDL_Texture* getNormalTexture() { return normalTexture; }

		void setHoverTexture(SDL_Texture* newTexture) {
			if (hoverTexture != NULL) {
				SDL_DestroyTexture(hoverTexture);
			}
			hoverTexture = newTexture;
		}

		void setNormalTexture(SDL_Texture* newTexture) {
			if (normalTexture != NULL) {
				SDL_DestroyTexture(normalTexture);
			}
			normalTexture = newTexture;
		}

		/* Check if click is inside button. */
		bool isInButton(int mouseX, int mouseY) { return isInRect(getRect(), mouseX, mouseY); }
		void setMouseOver(bool incomingMouseOver) { mouseOver = incomingMouseOver; }
		bool isMouseOver() { return mouseOver; }
		ButtonClickStruct getClickStruct() { return clickStruct; }


	private:
		SDL_Rect rect;
		SDL_Texture* hoverTexture = NULL;
		SDL_Texture* normalTexture = NULL;
		string text = "";
		bool mouseOver;
		ButtonClickStruct clickStruct;
		SDL_Rect createTextRect(SDL_Rect buttonRect, string buttonText, TTF_Font* buttonFont); /* For normal all-text buttons. */
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


/* Generate a randomized overlay for button backgrounds */
vector<SDL_Rect> createSurfaceOverlay(SDL_Rect bgRect) {
	/*
	* TODO: Simplify the variable sized blocks.
	*/

	/* we will store all the rects here and later draw them */
	vector<SDL_Rect> rects;
	/* Pick a random starting point */
	enum StartingRect { WholeSide, Corner, Top };
	int startingRectNumber = (rand() % 3) + 1;
	StartingRect startingRect = startingRectNumber == 3 ? WholeSide :
		startingRectNumber == 2 ? Corner : Top;

	float whRatio = (float)bgRect.w / (float)bgRect.h;
	/* not totally random dimensions. Always in increments, defined as blocks (of pixels) based on resolution.
	* Resolution variable based on dimensions. */
	int xRes = bgRect.w < 350 ? 10 : bgRect.w < 800 ? 6 : 8;
	//int yRes = bgRect.h < 200 ? 4 : bgRect.h < 350 ? 10 : bgRect.h < 800 ? 6 : bgRect.h < 900 ? 15 : 12;
	float yResFloat = xRes / whRatio;
	int yRes = (int)yResFloat;

	if (yRes == 0) { yRes = 1; }
	if (xRes == 0) { xRes = 1; }

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
	int columnsToFill = bgRect.w < 350 ? 3 : 0;
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
	int reStarts = bgRect.w < 350 ? 0 : bgRect.w < 1000 ? 2 :3;

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

		/* If we can't move, break the loop (try to find a new place first) */
		if (nowhereToGo) {
			/* don't restart if it's close to the end anyway */
			if (reStarts < 1 || seeds < 5) {
				seeds = 0;
				break;
			}

			/* make some attempt to pick a random false block */

			int attempts = 5;
			while (attempts > 0) {
				/* get random X and Y */
				int randX = rand() % rows[0].size();
				int randY = rand() % rows.size();

				if (!rows[randY][randX]) {
					currY = randY;
					currX = randX;
					rows[randY][randX] = true;
					++seeds; /* give this new cluster a little extra */
				}
				--attempts;
			}
			--reStarts;
			continue;
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
	int panelPadding = ui.getPanelPadding();

	SDL_Surface* buttonSurface = createTransparentSurface(buttonRect.w, buttonRect.h);

	SDL_Rect bgColorRect = {
		panelPadding, panelPadding,
		buttonRect.w - (panelPadding * 2),
		buttonRect.h - (panelPadding * 2),
	};

	// fill the rects with beautiful color
	SDL_FillRect(buttonSurface, &bgColorRect, convertSDL_ColorToUint32(buttonSurface->format, color));

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
	vector<SDL_Rect> overlayRects = createSurfaceOverlay(buttonRect);

	/* make two button surfaces with the correct BG colors, to blit the text surfaces onto */
	SDL_Surface* hoverButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_HOVER_BRDR"], colors["BTN_HOVER_BG"], mainRenderer, overlayRects);
	//SDL_Surface* normalButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_BG"], colors["BTN_BRDR"], mainRenderer, overlayRects);
	SDL_Surface* normalButtonSurface = createButtonSurfaceBG(buttonRect, colors["BTN_OVERLAY"], colors["BTN_BRDR"], mainRenderer, overlayRects);

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
		Panel(SDL_Rect incomingRect, vector<Button> incomingButtons, SDL_Texture* texture = NULL) {
			rect = incomingRect;
			buttons = incomingButtons;
			mouseOver = false;
			show = false;
			bgTexture = texture;
		}

		Panel() {}

		vector<Button>& getButtons() { return buttons; }

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

		void draw(UI& ui = UI::getInstance()) {
			if (!getShow()) { return; }

			if (bgTexture != NULL) {
				SDL_RenderCopyEx(
					ui.getMainRenderer(),
					bgTexture,
					NULL, &rect,
					0, NULL, SDL_FLIP_NONE
				);
			}

			for (Button& button : buttons) {
				/* get the rect, send it a reference(to be converted to a pointer) */
				SDL_Rect buttonRect = button.getRect();

				/* now draw the button texture */
				SDL_RenderCopyEx(
					ui.getMainRenderer(),
					button.isMouseOver() ? button.getHoverTexture() : button.getNormalTexture(),
					NULL, &buttonRect,
					0, NULL, SDL_FLIP_NONE
				);
			}
		}

		void destroyTextures() {
			for (Button& button : buttons) {
				if (button.getHoverTexture() != NULL) {
					SDL_DestroyTexture(button.getHoverTexture());
					button.setHoverTexture(NULL);
				}
				if (button.getNormalTexture() != NULL) {
					SDL_DestroyTexture(button.getNormalTexture());
					button.setNormalTexture(NULL);
				}
			}
			buttons = {};
			if (bgTexture != NULL) {
				SDL_DestroyTexture(bgTexture);
				bgTexture = NULL;
			}
		}

		void setTexture(SDL_Texture* newTexture) {
			if (bgTexture != NULL) {
				SDL_DestroyTexture(bgTexture);
			}

			bgTexture = newTexture;
		}

		void setRect(SDL_Rect newRect) { rect = newRect; }
		SDL_Rect getRect() { return rect; }

	private:
		SDL_Rect rect;
		vector<Button> buttons;
		bool mouseOver;
		bool show;
		SDL_Texture* bgTexture = NULL;
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
* Building panels is complicated.
* We must build buttons to populate the panels,
* and the size of the panels depends on the dimensions of the buttons.
* But sometimes a button's size depends on the size of other buttons.
* So we build PreButtonStructs with SOME button data,
* and use that to build the panel's rect(angle),
* then use that panel's rect to build the ACTUAL buttons.
* Once we have the ACTUAL buttons and the panel's rect,
* then we can construct (and return) the actual panel.
* 
* Some panels follow a similar pattern, but others are more unique.
* The panel of limbs on the character creation screen has rows AND columns.
* Its buttons have pictures. Will they have text too??
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
SDL_Rect UI::buildVerticalPanelRectFromButtonTextRects(vector<PreButtonStruct> preButtonStructs, bool left, int y) {
	int panelHeight = PANEL_PADDING; // start with one padding
	int longestButtonTextLength = 0;
	for (PreButtonStruct preButtonStruct : preButtonStructs) {
		/* add up the heights of the buttons plus padding. */
		panelHeight += preButtonStruct.textRectHeight + (buttonPadding * 2) + PANEL_PADDING;

		/* set the longest text length. */
		if (preButtonStruct.textRectWidth > longestButtonTextLength) {
			longestButtonTextLength = preButtonStruct.textRectWidth;
		}
	}

	/* Make the width first (just wide enough to accomodate the longest button). */
	int width = longestButtonTextLength + (buttonPadding * 2) + (PANEL_PADDING * 2);
	/* x is always 0 except when it's not. */
	int rectX = left ? 0 : (getWindowWidth() - width);
	int rectY = y < 1 ? (windowHeight - panelHeight) : (y + PANEL_PADDING);

	return {
		rectX,
		rectY,
		width,
		panelHeight
	};
}
 
/* NOW that we have both some info about the buttons, plus the panel rect, make the actual buttons. */
vector<Button> UI::buildButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect) {
	vector<Button> buttons;
	const int xForAll = panelRect.x + PANEL_PADDING;
	int widthForAll = panelRect.w - (PANEL_PADDING * 2);
	int heightSoFar = panelRect.y + PANEL_PADDING;

	for (int i = 0; i < preButtonStructs.size(); ++i) {
		PreButtonStruct thisStruct = preButtonStructs[i];

		/* start at the top of the panelRect PLUS panelPadding */

		SDL_Rect thisButtonRect = {
			xForAll,
			heightSoFar,
			widthForAll,
			thisStruct.textRectHeight + (buttonPadding * 2)
		};

		/* RELATIVE rect to be later painted onto the button itself (when fed into the constructor)*/
		SDL_Rect thisTextRect = {
			// text x position is HALF of the difference b/w button width and text width
			(thisButtonRect.w - thisStruct.textRectWidth) / 2,
			buttonPadding,
			thisStruct.textRectWidth,
			thisStruct.textRectHeight
		};

		buttons.push_back(
			Button(
				thisButtonRect,
				thisTextRect,
				thisStruct.text,
				buttonFont,
				colorsByFunction,
				mainRenderer,
				thisStruct.clickStruct));

		/* increment heightSoFar */
		heightSoFar += thisButtonRect.h + PANEL_PADDING;
	}

	return buttons;
}

/*
* Building buttons specifically for the Inventory Limbs button panel.
*/
vector<Button> UI::buildLimbButtonsFromPreButtonStructsAndPanelRect(vector<PreButtonStruct> preButtonStructs, SDL_Rect panelRect) {
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
	colorsByName["PERIDOT_DEMI"] = { 242, 222, 6, 50 };  /* THEME */ /* yellow orangey */
	colorsByName["VIVID_YELLOW"] = { 217, 168, 6 }; /* THEME */ /* brighter yellow */
	colorsByName["GOLD"] = { 255,214, 10 }; /* bright yellowish */
	colorsByName["YALE_BLUE"] = { 0, 53, 102 }; /* slightly lighter blue */
	colorsByName["OXFORD_BLUE"] = { 0, 29, 61 }; /* slightly darker blue */
	colorsByName["FRENCH_BLUE"] = { 0, 99, 191 }; // almost light (normal) blue-green
	colorsByName["FRENCH_BLUE_LIGHT"] = { 29, 131, 227 }; // Lighter than ever

	/* NOT SURE YET */
	colorsByName["ONYX"] = { 14, 14, 14 }; // almost black
	colorsByName["FERN_GREEN"] = { 79, 119, 45 };
	colorsByName["LIME"] = { 0, 199, 43 };
	colorsByName["MIKADO_YELLOW"] = { 255, 195, 0 };
	colorsByName["RICH_BLACK"] = { 0, 8, 20 }; // blue tinted dark
	colorsByName["BLACK"] = { 3, 3, 3 }; // almost absolute... not quite
	colorsByName["WHITE"] = { 250, 250, 250 }; // almost white
	colorsByName["PAPAYA_WHIP"] = { 253, 240, 213 }; // beige
	colorsByName["DESAT_ORANGE"] = { 213, 194, 154 }; // Slightly desaturated orange.
	colorsByName["SCARLET"] = { 193, 18, 31 }; // red
	colorsByName["SCARLIGHT"] = { 255, 57, 70 }; // lighter red
	colorsByName["WOODLAND"] = { 97, 89, 30 }; // brown-green
	colorsByName["SMOKEY_GREY"] = { 117, 117, 113 };
	colorsByName["DARKISH_GRAYISH_BLUE"] = {142, 146, 169};
	colorsByName["GRAPE_BRUISE"] = { 37, 35, 51 };
	colorsByName["CORNFLOWER_BLUE"] = { 143, 154, 255 };
	colorsByName["SALMON"] = { 255, 118, 118 };
	colorsByName["SPRING_GREEN"] = { 111, 255, 111 };

	colorsByName["BLOOD_RED"] = { 203, 1, 1 };
	colorsByName["MED_BLUE"] = { 2, 2, 200 };
	colorsByName["DARK_GREEN"] = { 0, 122, 2 };


	/* COLORS BY FUNCTION */
	colorsByFunction["BTN_HOVER_BG"] = colorsByName["PERIDOT"];
	colorsByFunction["BTN_HOVER_BG_DEMI"] = colorsByName["PERIDOT_DEMI"];
	colorsByFunction["BTN_BG"] = colorsByName["OXFORD_BLUE"];
	colorsByFunction["BTN_HOVER_BRDR"] = colorsByName["VIVID_YELLOW"];
	colorsByFunction["BTN_BRDR"] = colorsByName["ONYX"];
	colorsByFunction["BTN_OVERLAY"] = colorsByName["GRAPE_BRUISE"];
	/* STILL DECIDING */
	colorsByFunction["DARK_TEXT"] = colorsByName["BLACK"];
	colorsByFunction["LIGHT_TEXT"] = colorsByName["WHITE"];
	colorsByFunction["WARNING_RED"] = colorsByName["SCARLET"];
	colorsByFunction["LOGO_COLOR"] = colorsByName["PERIDOT"];
	colorsByFunction["OK_GREEN"] = colorsByName["FERN_GREEN"];
	colorsByFunction["BTN_TEXT"] = colorsByName["GOLD"];

	colorsByFunction["BG_LIGHT"] = colorsByName["FRENCH_BLUE"];
	colorsByFunction["BG_MED"] = colorsByName["YALE_BLUE"];
	colorsByFunction["PANEL_BG"] = colorsByName["PAPAYA_WHIP"];

	colorsByFunction["RED_BG"] = colorsByName["SALMON"];
	colorsByFunction["BLUE_BG"] = colorsByName["CORNFLOWER_BLUE"];
	colorsByFunction["GREEN_BG"] = colorsByName["SPRING_GREEN"];

	colorsByFunction["RED_FG"] = colorsByName["BLOOD_RED"];
	colorsByFunction["BLUE_FG"] = colorsByName["MED_BLUE"];
	colorsByFunction["GREEN_FG"] = colorsByName["DARK_GREEN"];

	colorsByFunction["LIMB_BTN_BG"] = colorsByName["OXFORD_BLUE"];
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
	int monoFontSize = bodyFontSize;
	int monoFontLargeSize = resources.getFontSize(FontContext::Title, mainWindowSurface->w);
	int headlineFontSize = resources.getFontSize(FontContext::Headline, mainWindowSurface->w);
	int headlineFontLargeSize = resources.getFontSize(FontContext::HeadlineLarge, mainWindowSurface->w);


	/* Initialize TTF font library */
	if (TTF_Init() == -1) {
		SDL_Log("WTTF failed to initialize. TTF_Error: %s\n", TTF_GetError());
		std::cerr << "TTF failed to initialize. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	/* Load the fontss */

	monoFont = TTF_OpenFont("assets/sono_reg.ttf", monoFontSize);

	if (!monoFont) {
		SDL_Log("Font (sono_reg) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (sono_reg) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	monoFontLarge = TTF_OpenFont("assets/sono_reg.ttf", monoFontLargeSize);

	if (!monoFontLarge) {
		SDL_Log("Font (sono_reg) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (sono_reg) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	headlineFont = TTF_OpenFont("assets/pr_viking.ttf", headlineFontSize);

	if (!headlineFont) {
		SDL_Log("Font (pr_viking) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (pr_viking) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	//headlineFontLarge = TTF_OpenFont("assets/classica_bold.ttf", headlineFontLargeSize);
	headlineFontLarge = TTF_OpenFont("assets/pr_viking.ttf", headlineFontLargeSize);

	if (!headlineFontLarge) {
		SDL_Log("Font (classica_bold) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (classica_bold) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	titleFont = TTF_OpenFont("assets/pr_viking.ttf", titleFontSize);

	if (!titleFont) {
		SDL_Log("Font (pr_viking) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (pr_viking) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	//  classica_italic
	//buttonFont = TTF_OpenFont("assets/alte_haas_bold.ttf", buttonFontSize);
	//buttonFont = TTF_OpenFont("assets/knife_princess.ttf", buttonFontSize);
	buttonFont = TTF_OpenFont("assets/pr_viking.ttf", buttonFontSize);


	if (!buttonFont) {
		SDL_Log("Font (alte_haas_bold) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (alte_haas_bold) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	//bodyFont = TTF_OpenFont("assets/classica_book.ttf", bodyFontSize);
	bodyFont = TTF_OpenFont("assets/imperator.ttf", bodyFontSize);
	TTF_SetFontLineSkip(bodyFont, 27); // SET LINE HEIGHT (imperator line height smushes them all together)

	if (!bodyFont) {
		SDL_Log("Font (classica_book) failed to load. TTF_Error: %s\n", TTF_GetError());
		cerr << "Font (classica_book) failed to load. TTF_Error: " << TTF_GetError() << std::endl;
		return false;
	}

	// backup original file
	//dialogFont = TTF_OpenFont("assets/la_belle_aurore.ttf", dialogFontSize);
	dialogFont = TTF_OpenFont("assets/imperator.ttf", dialogFontSize);

	

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
* 
* Creating a button panel requires three stages:
* 1. create the preButtonStructs.
* 2. create the components (including buttons) from the structs.
* 3. create the panel which contains the buttons.
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
		buildPreButtonStruct(resources.getButtonText("LOAD_GAME"), ButtonOption::LoadGame),
		buildPreButtonStruct(resources.getButtonText("NEW_GAME"), ButtonOption::NewGame),
		buildPreButtonStruct(resources.getButtonText("SETTINGS"), ButtonOption::Settings),
		buildPreButtonStruct(resources.getButtonText("ABOUT"), ButtonOption::About),
		buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit)
	};
}

/* create all the components for the main menu panel */
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

/* 
* 
* 
* SETTINGS PANEL CREATION AND RE-BUILDING
* 
* 
*/


/* build and deliver basic info for settings panel buttons */
vector<PreButtonStruct> UI::getSettingsPreButtonStructs(ScreenType context) {
	Resources& resources = Resources::getInstance();
	/* preButonStructs don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	vector<PreButtonStruct> preButtonStructs = {
		buildPreButtonStruct(resources.getButtonText("MOBILE"), ButtonOption::Mobile),
		buildPreButtonStruct(resources.getButtonText("TABLET"), ButtonOption::Tablet),
		buildPreButtonStruct(resources.getButtonText("DESKTOP"), ButtonOption::Desktop),
		buildPreButtonStruct(resources.getButtonText("FULLSCREEN"), ButtonOption::Fullscreen)		
	};

	/* Some items might only be available in certain screens */

	if (context == ScreenType::Map) {
		/* to EXIT the screen (and the game) back to the main menu. */
		preButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit));
	}

	/* BACK from this menu to the original on-screen menu */
	preButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("BACK"), ButtonOption::Back));

	return preButtonStructs;
}

/* create all the components for the settings panel */
tuple<SDL_Rect, vector<Button>> UI::createSettingsPanelComponents(ScreenType context) {
	vector<PreButtonStruct> preButtonStructs = getSettingsPreButtonStructs(context);
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons};
}

/*
* Settings available in every screen.
*/
Panel UI::createSettingsPanel(ScreenType context) {
	auto [panelRect, buttons] = createSettingsPanelComponents(context);
	return Panel(panelRect, buttons);
}


/* rebuild the settings panel after resize */
void UI::rebuildSettingsPanel(Panel& settingsPanel, ScreenType context) {
	auto [panelRect, buttons] = createSettingsPanelComponents(context);
	settingsPanel.rebuildSelf(panelRect, buttons);
}



/*
*
*
* BATTLE TURN PANEL CREATION AND RE-BUILDING
*
*
*/

/* build and deliver basic info for Battle panel buttons */
vector<PreButtonStruct> UI::getBattlePreButtonStructs(vector<AttackStruct> playerAttackStructs) {
	Resources& resources = Resources::getInstance();
	/* preButonStructs don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	vector<PreButtonStruct> preButtonStructs;
	
	for (AttackStruct aStruct : playerAttackStructs) {
		preButtonStructs.push_back(buildPreButtonStruct(
			aStruct.name,
			ButtonOption::BattleMove,
			aStruct.attackType
		));
	}
	
	preButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("BUILD"), ButtonOption::Build));
	return preButtonStructs;
}


/* create all the components for the Battle panel */
tuple<SDL_Rect, vector<Button>> UI::createBattlePanelComponents(vector<AttackStruct> playerAttackStructs, int y) {
	vector<PreButtonStruct> preButtonStructs = getBattlePreButtonStructs(playerAttackStructs);
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs, true, y);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}


/*
* List of battle move buttons for the Player in BattleScreen.
*/
Panel UI::createBattlePanel(vector<AttackStruct> playerAttackStructs, int y) {
	auto [panelRect, buttons] = createBattlePanelComponents(playerAttackStructs, y);
	return Panel(panelRect, buttons);
}





/* 
* 
* 
* 
*							MAP MENU COMPONENTS
* 
* 
* 
*/

/* build and deliver basic info for main menu panel buttons */
vector<PreButtonStruct> UI::getGameMenuPreButtonStructs(ScreenType context) {
	Resources& resources = Resources::getInstance();
	/* preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */

	if (context == ScreenType::Map) {
		return {
			buildPreButtonStruct(resources.getButtonText("BUILD"), ButtonOption::Build),
			buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit)
		};
	}
	else {
		return {
			buildPreButtonStruct(resources.getButtonText("EXIT"), ButtonOption::Exit)
		};
	}
	
}

/* create all the components for the main menu panel */
tuple<SDL_Rect, vector<Button>> UI::createGameMenuPanelComponents(ScreenType context) {
	vector<PreButtonStruct> preButtonStructs = getGameMenuPreButtonStructs(context);
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}

/*  Settings available in every screen. */
Panel UI::createGameMenuPanel(ScreenType context) {
	auto [panelRect, buttons] = createGameMenuPanelComponents(context);
	return Panel(panelRect, buttons);
}

/* rebuild the settings panel after resize */
void UI::rebuildGameMenuPanel(Panel& gameMenuPanel, ScreenType context) {
	auto [panelRect, buttons] = createGameMenuPanelComponents(context);
	gameMenuPanel.rebuildSelf(panelRect, buttons);
}

/*	GENERIC MENU PANEL.  */

/* 1 */
Panel UI::createGeneralMenuPanel(unordered_map<string, ButtonOption> buttonOptions, bool left) {
	auto [panelRect, buttons] = createGeneralMenuPanelComponents(buttonOptions, left);
	return Panel(panelRect, buttons);
}

/* 2 */
/* create all the components for the main menu panel */
tuple<SDL_Rect, vector<Button>> UI::createGeneralMenuPanelComponents(unordered_map<string, ButtonOption> buttonOptions, bool left) {
	vector<PreButtonStruct> preButtonStructs = getGeneralMenuPreButtonStructs(buttonOptions, left);
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs, left);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}

/* 3 */
/* build and deliver basic info for menu panel buttons */
vector<PreButtonStruct> UI::getGeneralMenuPreButtonStructs(unordered_map<string, ButtonOption> buttonOptions, bool left) {
	Resources& resources = Resources::getInstance();
	/* preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */

	vector<PreButtonStruct> preButtonStructs = {};

	for (const auto& pair : buttonOptions) {
		preButtonStructs.push_back(buildPreButtonStruct(pair.first, pair.second));
	}

	return preButtonStructs;
}



/*
* 
* 
* 
* 
*				CHARACTER CREATION PANELS
* 
* 
* 
* 
* 
*/

/* Make Pre-Button Structs. */

/* build and deliver basic info for default Character Creation panel buttons. */
vector<PreButtonStruct> UI::getReviewModePreButtonStructs() {
	Resources& resources = Resources::getInstance();
	/* preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	return {
		buildPreButtonStruct(resources.getButtonText("FREE_LIMBS"), ButtonOption::ShowLimbs),
		buildPreButtonStruct(resources.getButtonText("SAVE"), ButtonOption::SaveSuit),
		buildPreButtonStruct(resources.getButtonText("CLEAR"), ButtonOption::ClearSuit),
		buildPreButtonStruct(resources.getButtonText("CONTINUE"), ButtonOption::Continue)
	};
}


/* build and deliver basic info for default Character Creation panel buttons. */
vector<PreButtonStruct> UI::getLimbLoadedModePreButtonStructs(bool loadedLimbHasExtraJoints, bool characterHasExtraJoints) {
	Resources& resources = Resources::getInstance();
	/* preButonStructs just don't know their positions (will get that from choice of PANEL (horizontal vs vertical) */
	vector<PreButtonStruct> limbLoadedModePreButtonStructs;
	limbLoadedModePreButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("EQUIP_LIMB"), ButtonOption::Equip));
	if (characterHasExtraJoints) {
		limbLoadedModePreButtonStructs.push_back(
			buildPreButtonStruct(resources.getButtonText("NEXT_CHARACTER_JOINT"), ButtonOption::NextCharJoint)); }
	if (loadedLimbHasExtraJoints) {
		limbLoadedModePreButtonStructs.push_back(
			buildPreButtonStruct(resources.getButtonText("NEXT_LIMB_JOINT"), ButtonOption::NextLimbJoint)); }	
	limbLoadedModePreButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("ROTATE_CLOCKWISE"), ButtonOption::RotateClockwise));
	limbLoadedModePreButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("ROTATE_COUNTERCLOCKWISE"), ButtonOption::RotateCounterClockwise));
	limbLoadedModePreButtonStructs.push_back(buildPreButtonStruct(resources.getButtonText("UNLOAD_LIMB"), ButtonOption::UnloadLimb));

	return limbLoadedModePreButtonStructs;
}

/* Get the components (including panel object and buttons) for the Character Creation panels. */

tuple<SDL_Rect, vector<Button>> UI::createReviewModePanelComponents() {
	vector<PreButtonStruct> preButtonStructs = getReviewModePreButtonStructs();
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}


tuple<SDL_Rect, vector<Button>> UI::createLimbLoadedModePanelComponents(bool loadedLimbHasExtraJoints, bool characterHasExtraJoints) {
	vector<PreButtonStruct> preButtonStructs = getLimbLoadedModePreButtonStructs(loadedLimbHasExtraJoints, characterHasExtraJoints);
	SDL_Rect panelRect = buildVerticalPanelRectFromButtonTextRects(preButtonStructs);
	vector<Button> buttons = buildButtonsFromPreButtonStructsAndPanelRect(preButtonStructs, panelRect);
	return { panelRect, buttons };
}

/* Returns a surface with the text centered, broken up on spaces. */
export SDL_Surface* createCenteredWrappedText(string text, TTF_Font* font, SDL_Color color) {
	vector<string> words;
	vector<SDL_Surface*> wordSurfaces;

	int xOffset = 6;
	int yOffset = 6;

	int widestSurfaceWidth = 0;
	int tallestSurfaceHeight = 0;

	istringstream stringStream(text);
	string word;

	/* make a vector of words from the string */
	while (getline(stringStream, word, ' ')) { words.push_back(word); }

	/* make a vector of text surfaces from the words */
	for (string word : words) {
		SDL_Surface* wordSurface = TTF_RenderUTF8_Blended(font, word.c_str(), color);
		wordSurfaces.push_back(wordSurface);
		if (wordSurface->w > widestSurfaceWidth) { widestSurfaceWidth = wordSurface->w; }
		if (wordSurface->h > tallestSurfaceHeight) { tallestSurfaceHeight = wordSurface->h; }}

	int newSurfaceWidth = widestSurfaceWidth + (xOffset * 2);
	int newSurfaceHeight = (tallestSurfaceHeight + yOffset) * words.size();

	SDL_Surface* textSurface = createTransparentSurface(newSurfaceWidth, newSurfaceHeight);

	for (int i = 0; i < wordSurfaces.size(); i++) {

		/* blit each surface onto the new surfaces */
		int wordWidth = wordSurfaces[i]->w;
		int wordHeight = wordSurfaces[i]->h;

		SDL_Rect wordRect = {
			(newSurfaceWidth - wordWidth) / 2,
			(wordHeight * i) + (yOffset * i),
			wordWidth,
			wordHeight
		};

		SDL_BlitSurface(wordSurfaces[i], NULL, textSurface, &wordRect);
		SDL_FreeSurface(wordSurfaces[i]);
	}

	return textSurface;
}

/*
* Creates a new surface based on the incoming rect, and centers the incoming surface in THAT new surface,
* and returns the new surface.
*/
export SDL_Surface* centerSurfaceInRect(SDL_Surface* surfaceToCenter, SDL_Rect rect, bool destroyOriginal = false) {

	SDL_Surface* newSurface = createTransparentSurface(rect.w, rect.h);

	int newX = (rect.w - surfaceToCenter->w) / 2;
	int newY = (rect.h - surfaceToCenter->h) / 2;
	SDL_Rect textRect = { newX, newY, surfaceToCenter->w , surfaceToCenter->h };

	SDL_BlitSurface(
		surfaceToCenter,
		NULL,
		newSurface,
		&textRect
	);

	if (destroyOriginal) {
		SDL_FreeSurface(surfaceToCenter);
	}

	return newSurface;
}

/*
* Make a full-screen panel which displays a button for each Limb in a given vector.
* Also adds a "BACK" button as the final item.
* 
* TO DO:
* -- make it responsive (columnsCount decreases with screen size).
* -- Do pagination (still keep Back button for EVERY page... but also add NEXT and PREVIOUS).
*/
tuple<SDL_Rect, vector<Button>> UI::createChooseLimbModePanelComponents(
	vector<LimbButtonData> limbBtnDataStructs, bool drawBackButton, int labelOffsetY, int page
) {
	SDL_Rect panelRect = { 0, 0, windowWidth, windowHeight };
	int columnsCount = 10;
	int rowsPerPage = 3;
	int pages = (limbBtnDataStructs.size() / (columnsCount * rowsPerPage)) + 1;
	int nextPage = page < pages ? page + 1 : 1;
	int buttonWidth = (windowWidth - ((PANEL_PADDING * columnsCount) + PANEL_PADDING)) / columnsCount;
	int buttonHeight = buttonWidth; // buttonWidth * 2 + (PANEL_PADDING * 3);

	vector<Button> buttons;
	int startingIndex = (page - 1) * (columnsCount * rowsPerPage);
	int excludedFinalIndex = page * (columnsCount * rowsPerPage);

	/* Keep it within this page's range. */
	for (int i = startingIndex; i < excludedFinalIndex + 2 && i < limbBtnDataStructs.size() + 2; ++i) {
		
		/* Many things change for the back button option (final option). */
		bool isBackButton = i == limbBtnDataStructs.size() || i == excludedFinalIndex;
		bool isMoreButton = i == limbBtnDataStructs.size() + 1 || i == excludedFinalIndex + 1;
		if (
			(isBackButton && !drawBackButton) ||
			(isMoreButton && pages < 2)
			) { continue; }

		//bool isFirstRow = i < columnsCount;
		bool isFirstRow = page == 1 ?
			i < columnsCount :
			i < ((page - 1) * columnsCount * rowsPerPage) + columnsCount;

		int ySubtractorForPagination = (buttonHeight + PANEL_PADDING) * rowsPerPage * (page - 1);

		int rectX = PANEL_PADDING + ((i % columnsCount) * (buttonWidth + PANEL_PADDING));
		int rectY = isFirstRow ? PANEL_PADDING :
			(((i / columnsCount) * (PANEL_PADDING + buttonHeight)) + PANEL_PADDING) - ySubtractorForPagination;

		rectY += labelOffsetY;
		if (isMoreButton && !drawBackButton) {
			rectX -= (buttonWidth + PANEL_PADDING);
		}
		/* This rectangle defines the size and position of the button. */
		SDL_Rect rect = {
			rectX,
			rectY,
			buttonWidth,
			buttonHeight
		};
		/* create main surfaces */
		SDL_Surface* normalSurface = SDL_CreateRGBSurface(0, buttonWidth, buttonHeight, 32, 0, 0, 0, 0xFF000000);
		SDL_Surface* hoverSurface = SDL_CreateRGBSurface(0, buttonWidth, buttonHeight, 32, 0, 0, 0, 0xFF000000);

		SDL_FillRect(normalSurface, NULL, convertSDL_ColorToUint32(normalSurface->format, colorsByFunction["LIMB_BTN_BG"]));

		SDL_Color hoverColor = colorsByFunction["BTN_HOVER_BG"];
		SDL_FillRect(hoverSurface, NULL, convertSDL_ColorToUint32(hoverSurface->format, hoverColor));

		string domNodeText = "";
		Resources& resources = Resources::getInstance();

		/* Get the TEXT surface. */
		SDL_Surface* textSurface = NULL;
		string buttonText = "";
		if (!isBackButton && !isMoreButton && i < limbBtnDataStructs.size()) {
			LimbButtonData& limbButtonDataStruct = limbBtnDataStructs[i];

			if (limbButtonDataStruct.domNode == DominanceNode::Green) {
				hoverColor = colorsByFunction["GREEN_BG"];
				domNodeText = "GREEN";
			}else if (limbButtonDataStruct.domNode == DominanceNode::Red) {
				hoverColor = colorsByFunction["RED_BG"];
				domNodeText = "RED";
			} else if (limbButtonDataStruct.domNode == DominanceNode::Blue) {
				hoverColor = colorsByFunction["BLUE_BG"];
				domNodeText = "BLUE";
			}

			SDL_FillRect(hoverSurface, NULL, convertSDL_ColorToUint32(hoverSurface->format, hoverColor));
		
			buttonText = buttonText + "HP:  " + to_string(limbButtonDataStruct.hp) + "\n";
			buttonText = buttonText + "SPD: " + to_string(limbButtonDataStruct.speed) + "\n";
			buttonText = buttonText + "STR: " + to_string(limbButtonDataStruct.strength) + "\n";
			buttonText = buttonText + "INT: " + to_string(limbButtonDataStruct.intelligence) + "\n";
			buttonText = buttonText + domNodeText;

			textSurface = TTF_RenderUTF8_Blended_Wrapped(monoFontLarge, buttonText.c_str(), colorsByFunction["DARK_TEXT"], 0);
		}
		else if(isBackButton) {
			buttonText = resources.getButtonText("BACK");
			textSurface = createCenteredWrappedText(buttonText, getButtonFont(), colorsByFunction["DARK_TEXT"]);
		}
		else if (isMoreButton) {
			buttonText = resources.getButtonText("MORE");
			textSurface = createCenteredWrappedText(buttonText, getButtonFont(), colorsByFunction["DARK_TEXT"]);
		}
				
		if (!textSurface) {
			std::cerr << "TTF_RenderUTF8_Blended_Wrapped Error: " << TTF_GetError() << std::endl; }
		
		
		ButtonClickStruct clickStruct = !isBackButton && !isMoreButton ?
			ButtonClickStruct(ButtonOption::LoadLimb, limbBtnDataStructs[i].id) :
			isBackButton ? ButtonClickStruct(ButtonOption::Back, -1) :
			isMoreButton ? ButtonClickStruct(ButtonOption::NextPage, nextPage) :
			ButtonClickStruct(ButtonOption::NoOption, -1);

		
		if (!isBackButton && !isMoreButton) {
			/* NORMAL button */
			LimbButtonData limbBtnDataStruct = limbBtnDataStructs[i];
			SDL_Surface* limbSurface = IMG_Load(limbBtnDataStruct.texturePath.c_str());
			SDL_BlitScaled(limbSurface, NULL, normalSurface, NULL);
			SDL_FreeSurface(limbSurface);
			limbSurface = NULL;

			/* HOVER button */
			/* Make textSurface square by blitting onto a new transparent surface. */

			int textWidth = textSurface->w;
			int textHeight = textSurface->h;
			int dimension = textHeight > textWidth ? textHeight : textWidth;
			SDL_Surface* textSquareSurface = createTransparentSurface(dimension, dimension);
			SDL_Rect textRect = { 0, 0, textWidth, textHeight };
			SDL_BlitScaled(textSurface, NULL, textSquareSurface, &textRect);

			SDL_FreeSurface(textSurface);
			textSurface = textSquareSurface;
		}

		if (isBackButton || isMoreButton) {
			/* " "BACK" drawn onto BOTH surfaces.*/
			if (textSurface->h < rect.h && textSurface->w < rect.w) {
				textSurface = centerSurfaceInRect(textSurface, rect, true);
			}
			SDL_BlitScaled(textSurface, NULL, hoverSurface, NULL);

			if (textSurface != NULL) {
				SDL_FreeSurface(textSurface);
				textSurface = NULL;
			}

			textSurface = centerSurfaceInRect(
				createCenteredWrappedText(buttonText, getButtonFont(), colorsByFunction["BTN_TEXT"]),
				rect,
				true
			);

			SDL_BlitScaled(textSurface, NULL, normalSurface, NULL);
		}
		else {
			/* STATS BACKGROUND finally drawn onto the hoverSurface. */
			SDL_Rect hoverRect = {
				PANEL_PADDING,
				PANEL_PADDING,
				rect.w - PANEL_PADDING *2,
				rect.h - PANEL_PADDING *2
			};
			SDL_BlitScaled(textSurface, NULL, hoverSurface, &hoverRect);
		}
		
		SDL_Texture* normalTexture = SDL_CreateTextureFromSurface(mainRenderer, normalSurface);
		SDL_Texture* hoverTexture = SDL_CreateTextureFromSurface(mainRenderer, hoverSurface);
		
		SDL_FreeSurface(hoverSurface);
		SDL_FreeSurface(normalSurface);

		if (textSurface != NULL) {
			SDL_FreeSurface(textSurface);
			textSurface = NULL;
		}

		buttons.emplace_back(
			rect,
			hoverTexture,
			normalTexture,
			buttonText,
			clickStruct
		);
	}
	return { panelRect, buttons };
}

/* Create the actual panels. */

Panel UI::createReviewModePanel() {
	auto [panelRect, buttons] = createReviewModePanelComponents();
	return Panel(panelRect, buttons);
}

/* Does this need a limb id? */
Panel UI::createLimbLoadedModePanel(bool loadedLimbHasExtraJoints, bool characterHasExtraJoints) {
	auto [panelRect, buttons] = createLimbLoadedModePanelComponents(loadedLimbHasExtraJoints, characterHasExtraJoints);
	return Panel(panelRect, buttons);
}

/* Entry point for any panel that hosts limb buttons. */
Panel UI::createChooseLimbModePanel(vector<LimbButtonData> limbBtnDataStructs, bool drawBackButton, string label, int page) {
	Resources& resources = Resources::getInstance();
	unordered_map<string, SDL_Color> colors = getColorsByFunction();

	int labelHeight = 0;
	int labelWidth = 0;
	int btnOffsetY = 0;
	SDL_Surface* labelSurface = NULL;
	SDL_Texture* panelTexture = NULL;

	/* Create the label first and use its dimensions to create the buttons' locations.
	* But we can't finish creating the panel background until after we create the buttons.
	*/
	if (label != "") {
		labelSurface = TTF_RenderUTF8_Blended(headlineFontLarge, label.c_str(), colors["DARK_TEXT"]);
		labelHeight = labelSurface->h;
		labelWidth = labelSurface->w;
		btnOffsetY = labelHeight + (PANEL_PADDING * 3);
	}

	auto [panelRect, buttons] = createChooseLimbModePanelComponents(limbBtnDataStructs, drawBackButton, btnOffsetY, page);

	/* Now blit the label onto the actual panel bg. */
	if (label != "") {
		SDL_Surface* panelSurface = createTransparentSurface(panelRect.w, panelRect.h);

		SDL_Rect labelRect = { (PANEL_PADDING * 2), (PANEL_PADDING * 2), labelWidth, labelHeight };
		SDL_Rect labelRectBG = {
			PANEL_PADDING,
			PANEL_PADDING,
			labelWidth + (PANEL_PADDING * 2),
			labelHeight + (PANEL_PADDING * 2)
		};

		SDL_FillRect(panelSurface, &labelRectBG, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));
		SDL_BlitSurface(labelSurface, NULL, panelSurface, &labelRect);
		SDL_FreeSurface(labelSurface);
		panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);
		SDL_FreeSurface(panelSurface);
	}

	return Panel(panelRect, buttons, panelTexture);
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
export Uint32 convertSDL_ColorToUint32(const SDL_PixelFormat* format, SDL_Color color) {
	return SDL_MapRGB(
		format,
		color.r,
		color.g,
		color.b);
}

/* Check if an x/y location falls within a rect */
export bool isInRect(SDL_Rect rect, int mouseX, int mouseY) {
	/* check horizontal */
	if (mouseX >= rect.x && mouseX <= rect.x + rect.w) {
		/* check vertical */
		if (mouseY >= rect.y && mouseY <= rect.y + rect.h) {
			return true;
		}
	}
	return false;
}


/* whenever we want to flip a surface */
export SDL_Surface* flipSurface(SDL_Surface* surface, bool horizontal) {
	/* new surface onto which we'll flip the original surface */
	//SDL_Surface* flippedSurface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 0, 0, 0, 0xFF000000);
	SDL_Surface* flippedSurface = createTransparentSurface(surface->w, surface->h);

	/* error check */
	if (!flippedSurface) {
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

/* The default BG texture for any screen. */
SDL_Texture* UI::createBackgroundTexture() {
	int w = mainWindowSurface->w;
	int h = mainWindowSurface->h;

	SDL_Rect bgRect = { 0, 0, w, h };
	vector<SDL_Rect> overlayRects = createSurfaceOverlay(bgRect);
	SDL_Surface* bgSurface = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0xFF000000);
	SDL_FillRect(bgSurface, NULL, convertSDL_ColorToUint32(bgSurface->format, colorsByFunction["BG_MED"]));

	/* draw the overlay */
	if (overlayRects.size() > 0) {
		for (SDL_Rect rect : overlayRects) {
			SDL_FillRect(bgSurface, &rect, convertSDL_ColorToUint32(bgSurface->format, colorsByFunction["BG_LIGHT"]));
		}
	}

	/* possibility of flipping */
	int hFlipInt = rand() % 2;
	int vFlipInt = rand() % 2;

	if (hFlipInt == 0) { bgSurface = flipSurface(bgSurface, true); }
	if (vFlipInt == 0) { bgSurface = flipSurface(bgSurface, false); }

	SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(mainRenderer, bgSurface);
	SDL_FreeSurface(bgSurface);

	return bgTexture;
}

/* Title texture for any screen */
tuple<SDL_Texture*, SDL_Rect> UI::createTitleTexture(string title) {
	Resources& resources = Resources::getInstance();
	// YELLOW text with BLACK offset underlay
	unordered_map<string, SDL_Color> colors = getColors();
	SDL_Color logoColor = colors["LOGO_COLOR"];
	SDL_Color textColor = colors["DARK_TEXT"];
	SDL_SetRenderDrawColor(mainRenderer, logoColor.r, logoColor.g, logoColor.b, 1);
	string titleText = title;

	/* make one yellow, one black, blit them onto a slightly larger one so the black is beneath but offset by 10px */
	SDL_Surface* titleTextSurfaceFG = TTF_RenderUTF8_Blended(getTitleFont(), titleText.c_str(), logoColor);
	SDL_Surface* titleTextSurfaceBG = TTF_RenderUTF8_Blended(getTitleFont(), titleText.c_str(), textColor);

	int xOffset = 6;
	int yOffset = 6;

	/* If text won't fit on screen, this is how we wrap and center it. */
	if (titleTextSurfaceFG->w > mainWindowSurface->w - (xOffset * 3)) {

		vector<string> words;
		vector<SDL_Surface*> wordSurfacesFG;
		vector<SDL_Surface*> wordSurfacesBG;

		int widestSurfaceWidth = 0;
		int tallestSurfaceHeight = 0;

		istringstream iss(title);
		string word;

		/* make a vector of words from the string */
		while (getline(iss, word, ' ')) { words.push_back(word); }

		/* make two vectors (FG & BG) of text surfaces from the words */
		for (string word: words) {
			SDL_Surface* newFG = TTF_RenderUTF8_Blended(getTitleFont(), word.c_str(), logoColor);
			SDL_Surface* newBG = TTF_RenderUTF8_Blended(getTitleFont(), word.c_str(), textColor);

			wordSurfacesFG.push_back(newFG);
			wordSurfacesBG.push_back(newBG);

			if (newFG->w > widestSurfaceWidth) { widestSurfaceWidth = newFG->w; }
			if (newFG->h > tallestSurfaceHeight) { tallestSurfaceHeight = newFG->h; }
		}

		int newSurfaceWidth = widestSurfaceWidth + (xOffset * 2);
		int newSurfaceHeight = (tallestSurfaceHeight + yOffset) * words.size();

		SDL_FreeSurface(titleTextSurfaceFG);
		SDL_FreeSurface(titleTextSurfaceBG);

		titleTextSurfaceFG = createTransparentSurface(newSurfaceWidth, newSurfaceHeight);
		titleTextSurfaceBG = createTransparentSurface(newSurfaceWidth, newSurfaceHeight);

		for (int i = 0; i < wordSurfacesFG.size(); i++) {

			/* blit each surface onto the new surfaces */
			int wordWidth = wordSurfacesFG[i]->w;
			int wordHeight = wordSurfacesFG[i]->h;

			SDL_Rect wordRect = {
				(newSurfaceWidth - wordWidth) / 2,
				(wordHeight * i) + (yOffset * i),
				wordWidth,
				wordHeight
			};

			SDL_BlitSurface(wordSurfacesFG[i], NULL, titleTextSurfaceFG, &wordRect);
			SDL_BlitSurface(wordSurfacesBG[i], NULL, titleTextSurfaceBG, &wordRect);

			SDL_FreeSurface(wordSurfacesFG[i]);
			SDL_FreeSurface(wordSurfacesBG[i]);
		}
	}

	/* create a blank surface */
	SDL_Surface* titleTextSurface = createTransparentSurface(
		titleTextSurfaceFG->w + xOffset,
		titleTextSurfaceFG->h + yOffset);

	SDL_Rect bgRect = {
		xOffset,
		yOffset,
		titleTextSurface->w,
		titleTextSurface->h
	};

	/* blit */
	SDL_BlitSurface(titleTextSurfaceBG, NULL, titleTextSurface, &bgRect);
	SDL_BlitSurface(titleTextSurfaceFG, NULL, titleTextSurface, NULL);

	/* Finally create the texture */
	SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(mainRenderer, titleTextSurface);

	SDL_FreeSurface(titleTextSurface);
	SDL_FreeSurface(titleTextSurfaceBG);
	SDL_FreeSurface(titleTextSurfaceFG);

	/* create title text rect */

	/* get the width and height of the title texture, calculate the x& y for the rect on which to draw it */
	int titleTextWidth, titleTextHeight;
	SDL_QueryTexture(titleTexture, NULL, NULL, &titleTextWidth, &titleTextHeight);

	/* create the rect to draw the title */
	SDL_Rect titleRect = {
		(mainWindowSurface->w / 2) - (titleTextWidth / 2),
		titleTextHeight,
		titleTextWidth,
		titleTextHeight
	};

	return { titleTexture, titleRect };
}

/* create a blank surface */
export SDL_Surface* createTransparentSurface(int w, int h) {
	SDL_Surface* transparentSurface = SDL_CreateRGBSurface(
		0, w, h, 32,  // bits per pixel
		0x00FF0000, // Red mask
		0x0000FF00, // Green mask
		0x000000FF, // Blue mask
		0xFF000000  // Alpha mask
	);
	return transparentSurface;
}

enum StartDirection { Left, Right };

struct FloatingObject {
	FloatingObject(SDL_Texture* newTexture, int screenWidth, int screenHeight) :
		texture(newTexture), screenWidth(screenWidth), screenHeight(screenHeight)
	{
		width = 1;
		height = 1;

		SDL_QueryTexture(texture, NULL, NULL, &width, &height);
		drawRect = {
			-width,
			rand() % (screenHeight - height), /* random y location */
			width,
			height
		};

		startDirection = rand() % 2 == 0 ? StartDirection::Left : StartDirection::Right;
		drawRect.x = startDirection == StartDirection::Left ? -width : screenWidth;

		/* xSpeed should be higher and less variable than ySpeed. */
		xSpeed = (rand() % 3) + 3;
		ySpeed = (rand() % 3);

		/* In some situations, make the "speed" (trajectory?) negative. */
		if (startDirection == StartDirection::Right) {
			xSpeed *= -1;
		}

		if (ySpeed > 0 && rand() % 2 == 0) {
			ySpeed *= -1;
		}
	}

	void move() {
		drawRect.x += xSpeed;
		drawRect.y += ySpeed;
	}

	bool needsReset() {
		if (startDirection == StartDirection::Right) {
			if (drawRect.x < -width) {
				return true;
			}
			else { return false; }
		}
		else {
			if (drawRect.x > screenWidth) {
				return true;
			}
			else { return false; }
		}
	}

	SDL_Texture* texture;
	SDL_Rect drawRect;
	StartDirection startDirection;
	int xSpeed;
	int ySpeed;
	int width;
	int height;
	int screenWidth;
	int screenHeight;
};

export SDL_Texture* getSkyBackgroundTexture() {
	UI& ui = UI::getInstance();
	SDL_Renderer* renderer = ui.getMainRenderer();

	int windowWidth = ui.getWindowWidth();
	int windowHeight = ui.getWindowHeight();
	/* Make the SKY surface. */
	SDL_Surface* skyImageSurface = IMG_Load("assets/sky_and_clouds/sky.png");
	int skyImageWidth = skyImageSurface->w;
	int skyImageHeight = skyImageSurface->h;

	int skySurfaceWidth = windowWidth;
	int skySurfaceHeight = windowHeight;

	/* make skySurface the same ratio as the window. */
	if (skyImageWidth >= windowWidth && skyImageHeight >= windowHeight) {
		/* Image is bigger than screen. Just get a rectangle from within the pic. */
		skySurfaceWidth = windowWidth;
		skySurfaceHeight = windowHeight;
	}
	else if (skyImageWidth < windowWidth && skyImageHeight >= windowHeight) {
		/* Image is LESS WIDE than screen, but just as tall.
		* So we HAVE THE WIDTH.
		* We will use IMAGE WIDTH, and get a ratio (from the screen) for new height. */
		float ratio = static_cast<float>(windowHeight) / static_cast<float>(windowWidth);

		skySurfaceWidth = skyImageWidth;
		skySurfaceHeight = static_cast<int>(skyImageWidth * ratio);
	}
	else {
		/* Image is SHORTER than screen.
		* So we HAVE THE HEIGHT.
		* We will use IMAGE HEIGHT, and get a ratio (from window) to calculate NEW WIDTH from that. */
		float ratio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
		skySurfaceWidth = static_cast<int>(ratio * skyImageHeight);
		skySurfaceHeight = skyImageWidth;
	}

	SDL_Rect sourceRect = { 0, 0, skySurfaceWidth, skySurfaceHeight };
	SDL_Surface* skySurface = createTransparentSurface(skySurfaceWidth, skySurfaceHeight);

	SDL_BlitSurface(skyImageSurface, &sourceRect, skySurface, NULL);
	SDL_Texture* skyTexture = SDL_CreateTextureFromSurface(renderer, skySurface);

	SDL_FreeSurface(skyImageSurface);
	SDL_FreeSurface(skySurface);

	return skyTexture;
}

/*
* For the opening screen, the background will be a blue sky with various clouds flying around.
* Later we will add limbs flying together and snapping together, then flying apart as new limbs fly in.
* Maybe we will also add land at the bottom.
* YES: land at the bottom and LIMBS growing up from the land, and dancing like living trees or grass.
*/
export class SkyAndCloudsBackground {
public:
	SkyAndCloudsBackground(bool actuallyMake = false) {
		if (!actuallyMake) { return; }
		UI& ui = UI::getInstance();
		renderer = ui.getMainRenderer();

		windowWidth = ui.getWindowWidth();
		windowHeight = ui.getWindowHeight();

		/* Get the CLOUD images as surfaces. */
		SDL_Surface* cloudSurface001 = IMG_Load("assets/sky_and_clouds/cloud_001.png");
		SDL_Surface* cloudSurface002 = IMG_Load("assets/sky_and_clouds/cloud_002.png");
		SDL_Surface* cloudSurface003 = IMG_Load("assets/sky_and_clouds/cloud_003.png");
		SDL_Surface* cloudSurface004 = IMG_Load("assets/sky_and_clouds/cloud_004.png");
		SDL_Surface* cloudSurface005 = IMG_Load("assets/sky_and_clouds/cloud_005.png");

		/* Make textures from the cloud surfaces, and destroy the surfaces. */
		cloudTextures.emplace_back(SDL_CreateTextureFromSurface(renderer, cloudSurface001));
		cloudTextures.emplace_back(SDL_CreateTextureFromSurface(renderer, cloudSurface002));
		cloudTextures.emplace_back(SDL_CreateTextureFromSurface(renderer, cloudSurface003));
		cloudTextures.emplace_back(SDL_CreateTextureFromSurface(renderer, cloudSurface004));
		cloudTextures.emplace_back(SDL_CreateTextureFromSurface(renderer, cloudSurface005));

		SDL_FreeSurface(cloudSurface001);
		SDL_FreeSurface(cloudSurface002);
		SDL_FreeSurface(cloudSurface003);
		SDL_FreeSurface(cloudSurface004);
		SDL_FreeSurface(cloudSurface005);

		skyTexture = getSkyBackgroundTexture();

		floatingObjects.push_back(makeNewFloatingObject());
		floatingObjects.push_back(makeNewFloatingObject());
	}

	FloatingObject makeNewFloatingObject() {
		int textureIndex = rand() % cloudTextures.size();
		return FloatingObject(cloudTextures[textureIndex], windowWidth, windowHeight);
	}

	void draw() {
		/* First draw the background. */
		SDL_RenderCopyEx(renderer, skyTexture, NULL, NULL, 0, NULL, SDL_FLIP_NONE);
		/* Now draw and move (and possibly recreate) each FloatingObject. */
		for (int i = 0; i < floatingObjects.size(); ++i) {
			FloatingObject& object = floatingObjects[i];
			SDL_RenderCopyEx(renderer, object.texture, NULL, &object.drawRect, 0, NULL, SDL_FLIP_NONE);

			/* Check reset conditions. */
			if (object.needsReset()) {
				floatingObjects[i] = makeNewFloatingObject();
			}
			else {
				object.move();
			}
		}
	}

	void destroyTextures() {
		if (skyTexture != NULL) { SDL_DestroyTexture(skyTexture); }

		for (int i = cloudTextures.size() - 1; i >= 0; --i) {
			SDL_Texture* cloudTexture = cloudTextures[i];
			if (cloudTexture != NULL && cloudTexture != nullptr) {
				SDL_DestroyTexture(cloudTexture);
			}
		}
	}

	/*
	* The Screen holding this object will call the draw function.
	* Draw function will include the animate function.
	* 
	* NEXT:
	* 
	*/

private:
	vector<SDL_Texture*> cloudTextures;
	SDL_Texture* skyTexture = NULL;
	vector<FloatingObject> floatingObjects;
	SDL_Renderer* renderer;
	int windowWidth;
	int windowHeight;

};


/* 
* Whenever there is a new message to display in the Confirmation Panel,
* we must actually destroy that panel and create a new one.
*/
export Panel getNewConfirmationMessage(
	Panel& oldConfirmationPanel,
	string newMessage,
	ConfirmationButtonType confirmationType,
	bool includeRefuseButton,
	UI& ui = UI::getInstance()
) {
	oldConfirmationPanel.destroyTextures();
	return ui.createConfirmationPanel(newMessage, confirmationType, includeRefuseButton);
}

/*
* Special panel to contain warning or message along with agree and (optional) disagree buttons.
* Must be resized every time there is a new message, to accomodate the text.
*/
Panel UI::createConfirmationPanel(
	string confirmationText,
	ConfirmationButtonType confirmationType,
	bool includeRefuseButton
) {
	if (confirmationText == "") {
		/* NO text? Return an empty panel with no buttons or dimensions. */
		Panel newPanel = Panel();
		newPanel.setRect({0,0,0,0});
		return newPanel;
	}

	Resources& resources = Resources::getInstance();
	unordered_map<string, SDL_Color> colors = getColors();
	TTF_Font* buttonFont = getButtonFont();
	TTF_Font* bodyFont = getBodyFont();

	/* Build the agree / disagree buttons (Must be same size).
	* So we find which button text is LONGER, then query how wide that SURFACE would be.
	* That lets us set the dimensions for the buttons,
	* which lets us set the dimensions for the panel,
	* which lets us set the positions of the buttons.
	* THEN we can create the actual buttons and the panel.
	*/

	string agreeText =
		confirmationType == ConfirmationButtonType::OkCancel ?
		resources.getButtonText("OK") :
		resources.getButtonText("YES");
	string refuseText =
		confirmationType == ConfirmationButtonType::OkCancel ?
		resources.getButtonText("CANCEL") :
		resources.getButtonText("NO");

	string longestButtonText = agreeText.size() > refuseText.size() ? agreeText : refuseText;
	int agreeBtnTxtWidth, agreeBtnTxtHeight, refuseBtnTxtWidth, refuseBtnTxtHeight;
	TTF_SizeUTF8(buttonFont, agreeText.c_str(), &agreeBtnTxtWidth, &agreeBtnTxtHeight);
	TTF_SizeUTF8(buttonFont, refuseText.c_str(), &refuseBtnTxtWidth, &refuseBtnTxtHeight);

	int longestBtnTxtWidth = agreeBtnTxtWidth > refuseBtnTxtWidth ? agreeBtnTxtWidth : refuseBtnTxtWidth;
	int longestBtnTextHeight = refuseBtnTxtHeight;

	int btnRectWidth = longestBtnTxtWidth + (buttonPadding * 2);
	int btnRectHeight = longestBtnTextHeight + (buttonPadding * 2);

	/* Now we have the button dimensions, but not their positions.
	* Figure out the dimensions of the panel.
	*/

	SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(bodyFont, confirmationText.c_str(), colors["DARK_TEXT"], 300);
	int textWidth = textSurface->w;
	int textHeight = textSurface->h;

	int totalButtonsWidth = (btnRectWidth * 2) + (PANEL_PADDING * 3);
	int totalPanelTextWidth = textWidth + (PANEL_PADDING * 2);

	int panelRectWidth = totalPanelTextWidth > totalButtonsWidth ? totalPanelTextWidth : totalButtonsWidth;
	int panelRectHeight = textHeight + btnRectHeight + (PANEL_PADDING * 3);

	/* Cerate many SDL_Rects */

	SDL_Rect panelRect = {
		(getWindowWidth() / 2) - (panelRectWidth / 2),
		(getWindowHeight() / 2) - (panelRectHeight / 2),
		panelRectWidth,
		panelRectHeight
	};

	/* This rect is relative to the panel. */
	SDL_Rect textRect = {
		(panelRect.w / 2) - (textWidth / 2),
		PANEL_PADDING,
		textWidth,
		textHeight
	};

	int agreeBtnRectX =
		includeRefuseButton ? panelRect.x + (panelRect.w / 2) - (btnRectWidth + (PANEL_PADDING / 2)) :
		panelRect.x + (panelRect.w / 2) - (btnRectWidth / 2);

	SDL_Rect agreeBtnRect = {
		agreeBtnRectX,
		panelRect.y + panelRect.h - (btnRectHeight + PANEL_PADDING),
		btnRectWidth,
		btnRectHeight
	};

	SDL_Rect refuseBtnRect = {
		agreeBtnRect.x + agreeBtnRect.w + PANEL_PADDING,
		agreeBtnRect.y,
		btnRectWidth,
		btnRectHeight
	};

	SDL_Rect agreeBtnTxtRect = {
		(agreeBtnRect.w / 2) - (agreeBtnTxtWidth / 2),
		(agreeBtnRect.h / 2) - (agreeBtnTxtHeight / 2),
		agreeBtnTxtWidth,
		agreeBtnTxtHeight
	};

	SDL_Rect refuseBtnTxtRect = {
		(refuseBtnRect.w / 2) - (refuseBtnTxtWidth / 2),
		(refuseBtnRect.h / 2) - (refuseBtnTxtHeight / 2),
		refuseBtnTxtWidth,
		refuseBtnTxtHeight
	};

	/* Create the panel surface. */
	SDL_Surface* panelSurface = createTransparentSurface(panelRectWidth, panelRectHeight);
	SDL_FillRect(panelSurface, NULL, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));

	/* Blit the text onto the panel, make the texture, destroy the surfaces. */
	SDL_BlitSurface(textSurface, NULL, panelSurface, &textRect);
	SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);
	SDL_FreeSurface(textSurface);
	SDL_FreeSurface(panelSurface);

	/* Create the buttons. */

	vector<Button> buttons;

	buttons.emplace_back(
		agreeBtnRect,
		agreeBtnTxtRect,
		agreeText,
		buttonFont,
		colors,
		mainRenderer,
		ButtonClickStruct(ButtonOption::Agree, -1)
	);

	if (includeRefuseButton) {
		buttons.emplace_back(
			refuseBtnRect,
			refuseBtnTxtRect,
			refuseText,
			buttonFont,
			colors,
			mainRenderer,
			ButtonClickStruct(ButtonOption::Refuse, -1)
		);
	}

	return Panel(panelRect, buttons, panelTexture);
}



/*
* Whenever there is a new message to display in the Confirmation Panel,
* we must actually destroy that panel and create a new one.
*/
export Panel UI::getNewPassingMessagePanel(
	string newMessage,
	Panel& oldPassingMessagePanel,
	bool topPlacement,
	bool isBold
) {
	oldPassingMessagePanel.destroyTextures();
	return createPassingMessagePanel(newMessage, topPlacement, isBold);
}


/* Similar to the Confirmation Panel.
* But this one will have no buttons, just a message.
* It "passes" either with time, or some other non-button trigger.
* Also, it doesn't stop the game.
*/
Panel UI::createPassingMessagePanel(string message, bool topPlacement, bool isBold) {
	if (message == "") {
		/* NO text? Return an empty panel with no buttons or dimensions. */
		Panel newPanel = Panel();
		newPanel.setRect({ 0,0,0,0 });
		return newPanel;
	}

	Resources& resources = Resources::getInstance();
	unordered_map<string, SDL_Color> colors = getColors();
	TTF_Font* bodyFont = getBodyFont();

	/* 
	* Figure out the dimensions of the panel.
	*/

	SDL_Surface* textSurface = TTF_RenderUTF8_Blended_Wrapped(bodyFont, message.c_str(), colors["DARK_TEXT"], 340);
	int textWidth = textSurface->w;
	int textHeight = textSurface->h;

	int panelRectWidth = textWidth + (PANEL_PADDING * 2);
	int panelRectHeight = textHeight + (PANEL_PADDING * 2);

	/* Create many SDL_Rects */

	int panelHeight = topPlacement ? PANEL_PADDING : getWindowHeight() - (panelRectHeight + PANEL_PADDING);

	SDL_Rect panelRect = {
		(getWindowWidth() / 2) - (panelRectWidth / 2),
		panelHeight,
		panelRectWidth,
		panelRectHeight
	};

	/* This rect is relative to the panel. */
	SDL_Rect textRect = {
		(panelRect.w / 2) - (textWidth / 2),
		PANEL_PADDING,
		textWidth,
		textHeight
	};

	SDL_Rect overlayRect = {
		0,0, panelRectWidth, panelRectHeight
	};


	/* Create the panel surface. */

	// BACKUPS of simply filling in with a color
	//SDL_Surface* panelSurface = createTransparentSurface(panelRectWidth, panelRectHeight);
	//SDL_FillRect(panelSurface, NULL, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));


	// create the transparent panel surface (to be drawn on)
	SDL_Surface* panelSurface = createTransparentSurface(panelRectWidth, panelRectHeight);

	// load the texture overlay
	SDL_Surface* overlaySurface = IMG_Load("assets/msg_paper.jpg");

	// blit the overlay onto the transparent panelSurface
	SDL_BlitSurface(overlaySurface, NULL, panelSurface, &overlayRect);

	/* Blit the text onto the panel, make the texture, destroy the surfaces. */
	SDL_BlitSurface(textSurface, NULL, panelSurface, &textRect);
	SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);
	SDL_FreeSurface(textSurface);
	SDL_FreeSurface(panelSurface);
	SDL_FreeSurface(overlaySurface);

	/* Empty buttons vector because the constructor needs a vector. */
	vector<Button> buttons;

	return Panel(panelRect, buttons, panelTexture);
}

/*
* Shows the distance (in map tiles) between the player and the last known position of the tracked thing.
* Format:
*
* Last known position of Fairy Head
* 32 East, 7 North
*
* 
* TO DO: catch SDL failures.
*/
Panel UI::createTrackerPanel(Point trackedPoint, Point playerPoint, string name) {
	Resources& resources = Resources::getInstance();
	unordered_map<string, SDL_Color> colors = getColorsByFunction();
	Panel trackerPanel = Panel();
	trackerPanel.setRect({ 0,0,0,0 });

	/* First make the two text strings as data. */

	int xDistance = trackedPoint.x - playerPoint.x;
	int yDistance = trackedPoint.y - playerPoint.y;

	string xDirectionKey = xDistance > 0 ? "WEST" : "EAST";
	string yDirectionKey = yDistance > 0 ? "NORTH" : "SOUTH";

	string xDirectionString = resources.getCardinalDirectionText(xDirectionKey);
	string yDirectionString = resources.getCardinalDirectionText(yDirectionKey);

	if (xDistance < 0) {
		xDistance *= -1;
	}

	if (yDistance < 0) {
		yDistance *= -1;
	}

	string labelText = "Last known position of " + name;
	string trackerText = to_string(xDistance) + " " + xDirectionKey + ", ";
	trackerText += to_string(yDistance) + " " + yDirectionKey;

	/* Now make them into surfaces with max width of 300. */

	SDL_Surface* labelSurface = TTF_RenderUTF8_Blended_Wrapped(bodyFont, labelText.c_str(), colors["DARK_TEXT"], 300);
	int labelWidth = labelSurface->w;
	int labelHeight = labelSurface->h;

	SDL_Surface* trackerTextSurface = TTF_RenderUTF8_Blended_Wrapped(monoFont, trackerText.c_str(), colors["DARK_TEXT"], 300);
	int trackerTextWidth = trackerTextSurface->w;
	int trackerTextHeight = trackerTextSurface->h;

	/* Set the dimensions of the panel, based on the text surfaces. */

	int widestTextSurfaceWidth = labelWidth > trackerTextWidth ? labelWidth : trackerTextWidth;
	int panelWidth = widestTextSurfaceWidth + (PANEL_PADDING * 2);
	int panelHeight = trackerTextHeight + labelHeight + (PANEL_PADDING * 3);

	/* Set the rects for both surfaces. */

	SDL_Rect labelSurfaceRect = { PANEL_PADDING, PANEL_PADDING, labelWidth, labelHeight };
	SDL_Rect trackerTextRect = {
		PANEL_PADDING,
		labelHeight + (PANEL_PADDING * 2),
		trackerTextWidth, trackerTextHeight
	};

	/* Create the panel surface. */

	SDL_Surface* panelSurface = createTransparentSurface(panelWidth, panelHeight);
	SDL_FillRect(panelSurface, NULL, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));

	/* Blit the text surfaces onto the panel surface, and make the texture. */

	SDL_BlitSurface(labelSurface, NULL, panelSurface, &labelSurfaceRect);
	SDL_BlitSurface(trackerTextSurface, NULL, panelSurface, &trackerTextRect);
	SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);

	SDL_FreeSurface(trackerTextSurface);
	SDL_FreeSurface(labelSurface);
	SDL_FreeSurface(panelSurface);

	trackerPanel.setRect({ PANEL_PADDING, PANEL_PADDING, panelWidth, panelHeight });
	trackerPanel.setTexture(panelTexture);

	return trackerPanel;
}


/* A collection of boxes upon a transparent panel.
* Will show different things depending on the screen.
*/
Panel UI::createStatsPanel(ScreenType screenType, CharStatsData statsData, bool topRight) {
	Panel hudPanel = Panel();
	hudPanel.setRect({ 0,0,0,0 });
	Resources& resources = Resources::getInstance();
	unordered_map<string, SDL_Color> colors = getColorsByFunction();

	/*
	* 
	* 
	* MAKE THE BOXES THAT GO INSIDE THE PANEL
	* 
	* 
	* Future boxes:
	* --> Avatar from surface (requires altering createAvatar() to deliver just a surface).
	* --> Tracking Point box.
	* ----> Must include a string of text for name of limb.
	* 
	*/

	/* First make the nameSurface */


	DominanceNode dNode = statsData.dNode;
	SDL_Color dNodeColor = dNode == DominanceNode::Green ? colors["GREEN_FG"] :
		dNode == DominanceNode::Blue ? colors["BLUE_FG"] :
		dNode == DominanceNode::Red ? colors["RED_FG"] :
		colors["DARK_TEXT"];
	SDL_Surface* nameSurface = NULL;
	SDL_Rect nameRect = { PANEL_PADDING, PANEL_PADDING, 0, 0 };

	if (screenType == ScreenType::Battle) {
		string nameString = statsData.name;
		nameSurface = TTF_RenderUTF8_Blended_Wrapped(
			headlineFont, nameString.c_str(), colors["DARK_TEXT"], 285);

		/* We have the surface. Now make the rect. */
		nameRect = {
			PANEL_PADDING, PANEL_PADDING,
			nameSurface->w,
			nameSurface->h
		};
	}
	string dNodeString = dNodeText(dNode);
	SDL_Surface* dNodeSurface = TTF_RenderUTF8_Blended_Wrapped(headlineFont, dNodeString.c_str(), dNodeColor, 0);
	int dNodeWidth = dNodeSurface->w;
	int dNodeHeight = dNodeSurface->h;


	SDL_Rect dNodeRect = {
		nameRect.x,
		nameRect.y + nameRect.h,
		dNodeWidth,
		dNodeHeight
	};

	/* Gather info for Stats Box (for any screen). */
	string attsString = "";
	attsString = attsString + "HP:           " + to_string(statsData.hp) + "\n";
	attsString = attsString + "SPEED:        " + to_string(statsData.speed) + "\n";
	attsString = attsString + "STRENGTH:     " + to_string(statsData.strength) + "\n";
	attsString = attsString + "INTELLIGENCE: " + to_string(statsData.intelligence) + "\n";
	
	/* Make surfaces for other boxes
	* Name should be word-wrapped on spaces or 14 characters.
	*/

	SDL_Surface* attributesSurface = TTF_RenderUTF8_Blended_Wrapped(monoFont, attsString.c_str(), colors["DARK_TEXT"], 0);
	int attsWidth = attributesSurface->w;
	int attsHeight = attributesSurface->h;


	/*
	* 
	* ENTIRE PANEL
	* 
	*/

	/* Get the dimensions for the entire panel. */
	int statsPanelWidth = attsWidth + (PANEL_PADDING * 2);
	int statsPanelHeight = attsHeight + dNodeHeight + (PANEL_PADDING * 3);
	if (screenType == ScreenType::Battle && nameSurface != NULL) {
		statsPanelHeight += nameSurface->h;
	}

	/* Make the draw rects for the textures within the hud panel. */
	int attsSurfaceDrawX = (statsPanelWidth - attsWidth) / 2;
	int attsSurfaceDrawY = dNodeRect.y + dNodeRect.h + PANEL_PADDING;
	SDL_Rect attsRect = { attsSurfaceDrawX, attsSurfaceDrawY, attsWidth, attsHeight };

	/*
	*
	*
	* MAKE THE HUD PANEL SURFACE AND BLIT THE SMALLER TEXTURES ONTO IT.
	*
	*
	*/

	SDL_Surface* panelSurface = createTransparentSurface(statsPanelWidth, statsPanelHeight);
	SDL_FillRect(panelSurface, NULL, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));


	/*
	*      BLITTING
	*/

	if (nameSurface != NULL) {
		SDL_BlitSurface(nameSurface, NULL, panelSurface, &nameRect);
	}

	SDL_BlitSurface(attributesSurface, NULL, panelSurface, &attsRect);
	SDL_BlitSurface(dNodeSurface, NULL, panelSurface, &dNodeRect);
	SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);

	/*
	*
	*
	* MAKE THE HUD PANEL TEXTURE AND RETURN IT.
	*
	*
	*/

	int hudDrawX = topRight ? getWindowWidth() - (statsPanelWidth + PANEL_PADDING) : PANEL_PADDING;
	int hudDrawY = PANEL_PADDING;

	hudPanel.setRect({ hudDrawX, hudDrawY, statsPanelWidth , statsPanelHeight });
	hudPanel.setTexture(panelTexture);

	SDL_FreeSurface(attributesSurface);
	SDL_FreeSurface(dNodeSurface);
	SDL_FreeSurface(panelSurface);
	if (nameSurface != NULL) { SDL_FreeSurface(nameSurface); }

	return hudPanel;
}

string getCommandLineFromPair(pair<string, string> pair) {
	return "[ " + pair.first + " ] " + pair.second + "\n";
}

/* Show a map of keys to their functions. */
Panel UI::createKeyControlsPanel(ScreenType screenType) {
	Panel panel = Panel();

	if (screenType == ScreenType::NoScreen) {
		cerr << "ERROR: NO SCREEN\n";
		return panel;
	}

	panel.setRect({ 0,0,0,0 });
	Resources& resources = Resources::getInstance();
	unordered_map<string, string> keyControlsMap = resources.getKeyCommands(screenType);
	unordered_map<string, SDL_Color> colors = getColorsByFunction();
	string keyCommandsString = resources.getMessageText("KEY_COMMANDS_TITLE");
	string titlePrefix = screenType == ScreenType::Battle ? "BATTLE " :
		screenType == ScreenType::CharacterCreation ? "BUILD " :
		screenType == ScreenType::Map ? "MAP " : "";
	string titleString = titlePrefix + keyCommandsString + "\n";

	/* First make the title surface */

	SDL_Surface* titleSurface = NULL;

	titleSurface = TTF_RenderUTF8_Blended_Wrapped(monoFont, titleString.c_str(), colors["DARK_TEXT"], 285);

	/* We have the surface. Now make the rect. */
	SDL_Rect titleRect = {
		PANEL_PADDING, PANEL_PADDING,
		titleSurface->w,
		titleSurface->h
	};

	/* Now make the key map surface. Do Map in a specific order. */
	string keyMapString = "";

	for (const auto& pair : keyControlsMap) {
		keyMapString += getCommandLineFromPair(pair);
	}
	

	SDL_Surface* keyMapSurface = TTF_RenderUTF8_Blended_Wrapped(monoFont, keyMapString.c_str(), colors["DARK_TEXT"], 0);

	if (!keyMapSurface) {
		std::cerr << "SDL failed to initialize... SDL_Error: " << SDL_GetError() << std::endl;
		SDL_FreeSurface(titleSurface);
		return panel;
	}

	int keyMapWidth = keyMapSurface->w;
	int keyMapHeight = keyMapSurface->h;


	SDL_Rect keyMapRect = {
		titleRect.x,
		titleRect.y + titleRect.h + PANEL_PADDING,
		keyMapWidth,
		keyMapHeight
	};

	int widestWidth = keyMapRect.w > titleRect.w ? keyMapRect.w : titleRect.w;


	/*
	*
	* ENTIRE PANEL
	*
	*/

	/* Get the dimensions for the entire panel. */
	int panelWidth = widestWidth + (PANEL_PADDING * 2);
	int panelHeight = titleRect.h + keyMapRect.h + (PANEL_PADDING * 3);


	/*
	*
	*
	* MAKE THE HUD PANEL SURFACE AND BLIT THE SMALLER TEXTURES ONTO IT.
	*
	*
	*/

	SDL_Surface* panelSurface = createTransparentSurface(panelWidth, panelHeight);
	SDL_FillRect(panelSurface, NULL, convertSDL_ColorToUint32(panelSurface->format, colors["PANEL_BG"]));


	/*
	*      BLITTING
	*/

	if (titleSurface != NULL) {
		SDL_BlitSurface(titleSurface, NULL, panelSurface, &titleRect);
	}

	SDL_BlitSurface(keyMapSurface, NULL, panelSurface, &keyMapRect);
	SDL_Texture* panelTexture = SDL_CreateTextureFromSurface(mainRenderer, panelSurface);

	/*
	*
	*
	* MAKE THE HUD PANEL TEXTURE AND RETURN IT.
	*
	*
	*/

	int drawX = (getWindowWidth() / 2) - (panelWidth / 2);
	int drawY = PANEL_PADDING;

	panel.setRect({ drawX, drawY, panelWidth , panelHeight });
	panel.setTexture(panelTexture);

	SDL_FreeSurface(keyMapSurface);
	SDL_FreeSurface(panelSurface);
	SDL_FreeSurface(titleSurface);

	return panel;
}