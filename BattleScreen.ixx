/**
* 
*  ____    _  _____ _____ _     _____
* | __ )  / \|_   _|_   _| |   | ____|
* |  _ \ / _ \ | |   | | | |   |  _|
* | |_) / ___ \| |   | | | |___| |___
* |____/_/___\_\_|_  |_|_|_____|_____|
* / ___| / ___|  _ \| ____| ____| \ | |
* \___ \| |   | |_) |  _| |  _| |  \| |
*  ___) | |___|  _ <| |___| |___| |\  |
* |____/ \____|_| \_\_____|_____|_| \_|
* 
* Battle Screen.
* 
* -- Move panels into BattleScreen object.
* 
* 
* BATTLE SYSTEM:
* 
* - When attacking a limb, some of the damage inevitably spreads to the connected limbs instead.
* ---- Higher "intelligence" means more accuracy.
* - Type of attack depend on dominance cycle.
* 
* 
* 
* TO DO:
* - Create Battle Class.
* - Save battle object to DB (from MapScreen).
* - Load battle oject from DB (based on battle_id saved to PLAYER character).
* - Display both opponents (full drawn limbs, no avatar).
* - Animate both opponents (constantly rotating one limbs or another... just one at a time?).
* - HUD of player stats.
* - HUD of opponent stats.
* - Menu of UNEQUIPPED player limbs (to throw)
* ---- Include dominance cycle
* - Select EQUIPPED opponent Limb to attack via right-hand button.
* --- selected limb flashes.
* - Player limb to HEAL (by consuming LIMB from unequipped limbs).
* - THROW unequipped Limb (only if we have arm or leg to throw or kick).
* ---- THROW is more accurate but less power.
* ---- KICK is less accurate but more power.
* - PassingMessage panel shows effects of latest attack.
* - DROPPED limbs are returned to the map ON the opponent's square and scattered around.
* 
*/



module;
export module BattleScreen;

import "include/json.hpp";
import "SDL.h";
import "SDL_image.h";
import <stdio.h>;
import <string>;
import <iostream>;
import "SDL_ttf.h";
import <vector>;
import <cstdlib>;
import <time.h>;
import <unordered_map>;
import <unordered_set>;

import TypeStorage;
import GameState;
import Resources;
import UI;
import CharacterClasses;
import BattleClasses;
import Database;
import Audio;

using namespace std;



bool intsContainInt(vector<int> ints, int myInt) {
	for (int thisInt : ints) {
		if (myInt == thisInt) {
			return true;
		}
	}
	return false;
}

bool intsContainInt(unordered_set<int> ints, int myInt) {
	for (int thisInt : ints) {
		if (myInt == thisInt) {
			return true;
		}
	}
	return false;
}


void bringSumTo100(int& numA, int& numB) {
	if (numA + numB > 100) {
		--numA;
		bringSumTo100(numB, numA);
	}
	else if (numA + numB < 100) {
		++numA;
		bringSumTo100(numB, numA);
	}
}



/* Map Screen class: where we navigate worlds, dungeons, and buildings. */
export class BattleScreen {
public:
	/* constructor */
	BattleScreen() {
		GameState& gameState = GameState::getInstance();
		UI& ui = UI::getInstance();

		screenToLoadStruct = ScreenStruct(ScreenType::Menu, 0);

		id = gameState.getScreenStruct().id;
		screenType = ScreenType::Battle;
		battle = loadBattle(id);

		if (battle.getBattleStatus() == BattleStatus::RebuildRequired) {
			battle.setBattleStatus(BattleStatus::PlayerTurn);
			updateBattleStatus(battle.getId(), battle.getBattleStatus());
		}

		getBackgroundTexture(ui);
		createTitleTexture(ui);

		settingsPanel = ui.createSettingsPanel(ScreenType::Map);
		settingsPanel.setShow(false);

		unordered_map<string, ButtonOption> buttonOptions = {
			{"OPTIONS", ButtonOption::Options },
			{"EXIT", ButtonOption::Exit }
		};

		optionsMenu = ui.createGeneralMenuPanel(buttonOptions, false);
		optionsMenu.setShow(true);

		showTitle = true;
		titleCountdown = 140;

		/* Make sure characters have avatars (for now... later will draw limbs). */
		Character& playerCharacter = battle.getPlayerCharacter();
		Character& npc = battle.getNpc();

		npc.buildDrawLimbList();
		playerCharacter.buildDrawLimbList();

		playerCharacter.setTexture(playerCharacter.createAvatar());
		npc.setTexture(npc.createAvatar());

		/* Get the draw start points (WILL CHANGE after we implement button panels.) */
		int playerAvatarWidth, playerAvatarHeight, npcAvatarWidth, npcAvatarHeight;
		SDL_QueryTexture(playerCharacter.getTexture(), NULL, NULL, &playerAvatarWidth, &playerAvatarHeight);
		SDL_QueryTexture(npc.getTexture(), NULL, NULL, &npcAvatarWidth, &npcAvatarHeight);

		int windowHeight, windowWidth;
		SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);

		SDL_Rect playerAnchorLimbDrawRect = playerCharacter.getAnchorLimb().getDrawRect();
		SDL_Rect npcAnchorLimbDrawRect = npc.getAnchorLimb().getDrawRect();

		int playerX = ((windowWidth / 2) - playerAvatarWidth + playerAnchorLimbDrawRect.x) - 100 ;
		int playerY = (windowHeight / 2) - (playerAvatarHeight / 2) + playerAnchorLimbDrawRect.y;

		int npcX = (windowWidth / 2) + npcAnchorLimbDrawRect.x + 100;
		int npcY = (windowHeight / 2) - (npcAvatarHeight / 2) + npcAnchorLimbDrawRect.y;

		playerDrawStartPoint = Point( playerX, playerY );
		npcDrawStartPoint = Point(npcX, npcY);

		setLimbIdList();
		rotationCountdown = 30;
		resetRotationAmount();
		limbToRotateId = 0;
		skyBG = SkyAndCloudsBackground(true);

		unrotatePlayer = false;
		unrotateNpc = false;

		bobbingMeter = 0;
		bobbingMax = 20;
		reverseBob = false;

		playerStatsPanel = ui.createStatsPanel(ScreenType::Battle, playerCharacter.getCharStatsData(), false);
		playerStatsPanel.setShow(true);
		SDL_Rect playerStatsPanelRect = playerStatsPanel.getRect();

		npcStatsPanel = ui.createStatsPanel(ScreenType::Battle, npc.getCharStatsData());
		npcStatsPanel.setShow(true);

		playerAttackStructs = playerCharacter.getAttacks();
		npcAttackStructs = npc.getAttacks();

		playerTurnPanel = ui.createBattlePanel(playerAttackStructs, playerStatsPanelRect.h);
		playerTurnPanel.setShow(true);

		confirmationPanel = ui.createConfirmationPanel("Ready for Battle!", ConfirmationButtonType::OkCancel, false);
		confirmationPanel.setShow(true);

		passingMessagePanel = ui.createPassingMessagePanel("", true, true);
		passingMessagePanel.setShow(false);

		createNpcLimbPanel();
		createPlayerLimbPanels();

		flashingLimbCountdown = 10;
		drawFlashingLimb = true;
		flashLimb = false;

		attackAdvancePlayer = 0;
		attackAdvanceNpc = 0;
		passingMessageCountdown = 0;

		attackAdvanceHitTarget = false;
		animateBrainDrain = false;
		animateAttack = false;
		headRotation = 0;
		headSpins = 0;
		stealSuccess = false;
	}

	void createPlayerLimbPanels();
	void createNpcLimbPanel();
	void calculatePlayerDamageAttackStruct(int sourceLimbId, int targetLimbId);
	void calculateNpcDamageAttackStruct(int sourceLimbId, int targetLimbId);

	void calculatePlayerBrainDrain();
	void calculateNpcBrainDrain();
	void calculatePlayerSteal();

	ScreenType getScreenType() { return screenType; }
	void run();

	bool applyPlayerAttackEffects();
	bool applyPlayerStealEffects();
	bool applyNpcAttackEffects();

	void setExitMessage(BattleStatus battleStatus);

private:
	ScreenType screenType;
	int id;
	ScreenStruct screenToLoadStruct;

	SDL_Texture* bgTexture = NULL;
	SDL_Rect bgSourceRect;
	SDL_Rect bgDestinationRect;

	SDL_Texture* titleTexture;
	SDL_Rect titleRect;

	void draw(UI& ui);
	void drawPlayer(UI& ui);
	void drawNpc(UI& ui);
	void setPlayerAttackAdvance();
	void setNpcAttackAdvance();

	void handleEvent(SDL_Event& e, bool& running, GameState& gameState);
	void checkMouseLocation(SDL_Event& e);
	void handlePlayerMove(ButtonClickStruct clickStruct);

	void launchNpcTurn();

	void getBackgroundTexture(UI& ui);
	void rebuildDisplay(Panel& settingsPanel);
	void createTitleTexture(UI& ui);

	void raiseTitleRect() { --titleRect.y; }
	int getTitleBottomPosition() { return titleRect.y + titleRect.h; }

	bool showTitle;
	int titleCountdown;

	Point playerDrawStartPoint;
	Point npcDrawStartPoint;

	int rotationAmount;
	int rotationCountdown;
	int limbToRotateId;

	void resetRotationAmount();
	void setLimbIdList();
	void restartRotation();

	vector<int> limbIds;

	/* panels */
	Panel settingsPanel;
	Panel playerStatsPanel;
	Panel npcStatsPanel;
	Panel playerTurnPanel;
	Panel equippedLimbsPanel;
	Panel unequippedLimbsPanel;
	Panel npcLimbsPanel;
	Panel passingMessagePanel;
	Panel confirmationPanel;
	Panel optionsMenu;

	int passingMessageCountdown;
	Battle battle;
	SkyAndCloudsBackground skyBG;
	bool unrotatePlayer;
	bool unrotateNpc;

	int bobbingMeter;
	int bobbingMax;
	bool reverseBob;
	bool stealSuccess;

	vector<AttackStruct> playerAttackStructs;
	vector<AttackStruct> npcAttackStructs;

	int flashingLimbCountdown;
	bool drawFlashingLimb; /* switches on and off WHILE flashLimb is true. */
	bool flashLimb;

	AttackStruct playerAttackLoaded;
	AttackStruct npcAttackLoaded;

	bool animateBrainDrain;
	bool animateAttack;
	bool animateEffect;
	int animationCountdown;

	int headRotation;
	int headSpins;
	int attackAdvancePlayer;
	int attackAdvanceNpc;
	bool attackAdvanceHitTarget;

	unordered_set<int> limbIdsToUpdate;

	bool running;
};



void BattleScreen::createNpcLimbPanel() {
	UI& ui = UI::getInstance();

	/* Destroy textures if they already exist. */
	npcLimbsPanel.destroyTextures();

	/* Build a vector of data structures so the UI can build the panel of Limb buttons. */
	vector<LimbButtonData> limbBtnDataStructs;
	vector<Limb>& limbs = battle.getNpc().getLimbs();

	for (int i = 0; i < limbs.size(); ++i) {
		Limb& thisLimb = limbs[i];
		if (thisLimb.isEquipped()) {
			LimbButtonData newStruct = thisLimb.getLimbButtonData();
			limbBtnDataStructs.emplace_back(newStruct);
		}
	}

	npcLimbsPanel = ui.createChooseLimbModePanel(limbBtnDataStructs, true, "CHOOSE AN OPPONENT LIMB TO ATTACK:", 1);
	npcLimbsPanel.setShow(false);
}


void BattleScreen::createPlayerLimbPanels() {
	UI& ui = UI::getInstance();

	/* Destroy textures if they already exist. */
	if (unequippedLimbsPanel.getButtons().size() > 0) {
		unequippedLimbsPanel.destroyTextures();
	}

	if (equippedLimbsPanel.getButtons().size() > 0) {
		equippedLimbsPanel.destroyTextures();
	}

	/* Build a vector of data structures so the UI can build the panel of Limb buttons. */
	vector<LimbButtonData> equippedLimbBtnDataStructs;
	vector<LimbButtonData> unequippedLimbBtnDataStructs;
	vector<Limb>& limbs = battle.getPlayerCharacter().getLimbs();

	for (int i = 0; i < limbs.size(); ++i) {
		Limb& thisLimb = limbs[i];
		if (thisLimb.isEquipped()) {
			equippedLimbBtnDataStructs.push_back(thisLimb.getLimbButtonData());
		}
		else {
			unequippedLimbBtnDataStructs.push_back(thisLimb.getLimbButtonData());
		}
	}

	unequippedLimbsPanel = ui.createChooseLimbModePanel(unequippedLimbBtnDataStructs, true, "NON-EQUIPPED LIMBS", 1);
	equippedLimbsPanel = ui.createChooseLimbModePanel(equippedLimbBtnDataStructs, true, "EQUIPPED LIMBS", 1);

	equippedLimbsPanel.setShow(false);
	unequippedLimbsPanel.setShow(false);
}




/* We need a prebuilt list of limb ids
* so we can randomly select one to rotate.
*/
void BattleScreen::setLimbIdList() {
	limbIds = {};

	for (Limb& limb : battle.getPlayerCharacter().getLimbs()) {
		if (limb.isEquipped()) {
			limbIds.push_back(limb.getId());
		}
	}

	for (Limb& limb : battle.getNpc().getLimbs()) {
		if (limb.isEquipped()) {
			limbIds.push_back(limb.getId());
		}
	}
}

void BattleScreen::resetRotationAmount() {
	rotationAmount = 1;

	if (rand() % 2 == 0) {
		rotationAmount *= -1;
	}
}


void BattleScreen::restartRotation() {

	/* Occaisionally reset the player and npc. */
	int resetCoin = rand() % 40;

	if (resetCoin == 0) {
		unrotatePlayer = true;
		return;
	} else if (resetCoin == 1) {
		unrotateNpc = true;
		return;
	}

	resetRotationAmount();

	if ((rand() % 4) > 0) {
		int index = rand() % limbIds.size();
		limbToRotateId = limbIds[index];
	}
	else {
		limbToRotateId = 0;
	}

	rotationCountdown = (rand() % 46) + 7;
}

void BattleScreen::getBackgroundTexture(UI& ui) {
	int windowHeight, windowWidth;
	SDL_GetWindowSize(ui.getMainWindow(), &windowWidth, &windowHeight);
	bgSourceRect = { 0, 0, windowWidth, windowHeight };
	bgDestinationRect = { 0, 0, windowWidth, windowHeight };
	bgTexture = ui.createBackgroundTexture();
}

export void BattleScreen::run() {
	/* singletons */
	GameState& gameState = GameState::getInstance();
	UI& ui = UI::getInstance();
	
	/* Timeout data */
	const int TARGET_FPS = 60;
	const int FRAME_DELAY = 600 / TARGET_FPS; // milliseconds per frame
	Uint32 frameStartTime; // Tick count when this particular frame began
	int frameTimeElapsed; // how much time has elapsed during this frame

	/* loop and event control */
	SDL_Event e;
	running = true;

	if (gameState.getScreenStruct().id < 1) {
		/* Invalid battle id. Give a notice to the user on the way out? Or after getting back?
		* Maybe GameState deserves a message string to display.
		*/
		running = false;
	}

	while (running) {
		/* Get the total running time(tick count) at the beginning of the frame, for the frame timeout at the end */
		frameStartTime = SDL_GetTicks();

		/* Check for events in queue, and handle them(really just checking for X close now */
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_MOUSEBUTTONDOWN) {
				handleEvent(e, running, gameState);
			}
		}

		/* Deal with blinking loaded limb */
		if (flashLimb) {
			--flashingLimbCountdown;
			if (flashingLimbCountdown < 1) {
				drawFlashingLimb = !drawFlashingLimb;
				flashingLimbCountdown = 10;
			}
		}

		/* Deal with the Passing Message Panel. */
		if (passingMessagePanel.getShow()) {
			if (passingMessageCountdown > 0) {
				--passingMessageCountdown;
			}
			else {
				/* If we JUST hit zero: */
				passingMessagePanel.setShow(false);
			}
		}
		

		/* Deal with animation. */
		if (animationCountdown > 0) {
			--animationCountdown;

			if (animateBrainDrain) {
				headRotation += 15;
				if (headRotation >= 360) {
					headRotation = 0;
					++headSpins;
				}

				if (headSpins > 3) {

					animationCountdown = 0;
					headRotation = 0;
					headSpins = 0;
					animateBrainDrain = false;

					/* CALCULATE damage so we can animate effect. */
					if (battle.getBattleStatus() == BattleStatus::PlayerTurn) {
						calculatePlayerBrainDrain();
					} else if (battle.getBattleStatus() == BattleStatus::NpcTurn) {
						calculateNpcBrainDrain();
					}
					
					animateEffect = true;
					animationCountdown = 100;
					flashingLimbCountdown = 10;
					drawFlashingLimb = false;
					flashLimb = true;
				}
			}
		}
		else {
			/* End of either animation (Player or NPC). */

			if (animateEffect) {
				/* LAST FRAME. End of animateEffect animations (for either character). */
				animateEffect = !animateEffect;

				if (battle.isPlayerTurn()) {
					/* End of Player's animateEffect animation. */
					flashLimb = false;

					/* NOW actually change the limbs, clear the queue, and make it the  */

					if (playerAttackLoaded.attackType == AttackType::Steal) {
						cout << "Applying STEAL effects\n";
						applyPlayerStealEffects();
						setLimbIdList();
						limbIdsToUpdate = {};						
						createNpcLimbPanel();
						battle.switchTurn();
					}
					else {
						applyPlayerAttackEffects();
						Character& thisNpc = battle.getNpc();
					}

					playerAttackLoaded = AttackStruct();
					limbIdsToUpdate = {};

					/* RESETTING PLAYER TURN.
					* We will switch to NPC turn LATER.
					* After it's a smooth  cycle.
					*/

					playerTurnPanel.setShow(false);
					npcLimbsPanel.setShow(false);
					playerStatsPanel.setShow(true);
					npcStatsPanel.setShow(true);

				}
				else if(battle.isNpcTurn()) {
					/* End of NPC's animateEffect animation. */
					flashLimb = false;

					applyNpcAttackEffects();

					npcAttackLoaded = AttackStruct();
					limbIdsToUpdate = {};
					
					playerTurnPanel.destroyTextures();
					playerTurnPanel = ui.createBattlePanel(playerAttackStructs, playerStatsPanel.getRect().h);
					playerTurnPanel.setShow(true);
					npcLimbsPanel.setShow(false);
					playerStatsPanel.setShow(true);
					npcStatsPanel.setShow(true);

					if (battle.getBattleStatus() != BattleStatus::PlayerDefeat) {
						battle.setBattleStatus(BattleStatus::PlayerTurn);
						updateBattleStatus(battle.getId(), battle.getBattleStatus());
					}
				}
			}
		}

		/* Initiate NPC turn. */
		if (battle.getBattleStatus() == BattleStatus::NpcTurn && animationCountdown < 1 && npcAttackLoaded.attackType == AttackType::NoAttack) {
			cout << "lauching NPC TURN!\n";
			launchNpcTurn();
		}


		/* Deal with showing the title. */
		if (showTitle) {
			if (titleCountdown > 0) {
				--titleCountdown; }
			else {
				raiseTitleRect(); }

			if (getTitleBottomPosition() < -1) {
				showTitle = false; }
		}

		checkMouseLocation(e);
		draw(ui);

		/* Delay so the app doesn't just crash */
		frameTimeElapsed = SDL_GetTicks() - frameStartTime; // Calculate how long the frame took to process
		/* Delay loop */
		if (frameTimeElapsed < FRAME_DELAY) {
			SDL_Delay(FRAME_DELAY - frameTimeElapsed);
		}

		if (rotationCountdown < 1) {
			restartRotation();
		}

		--rotationCountdown;

		if(reverseBob) {
			--bobbingMeter;
			if (bobbingMeter < (bobbingMax * -1)) {
				reverseBob = !reverseBob;
			}			
		}
		else {
			++bobbingMeter;
			if (bobbingMeter > bobbingMax) {
				reverseBob = !reverseBob;
			}
		}
	} /* END of run() */

	/* set the next screen to load */
	if (screenToLoadStruct.screenType == ScreenType::Map) {
		cout << "GOING to the MAP\n";
	} else if (screenToLoadStruct.screenType == ScreenType::Menu) {
		cout << "GOING to the MENU\n";
	}

	gameState.setScreenStruct(screenToLoadStruct);
}

void BattleScreen::drawNpc(UI& ui) {
	Character& npc = battle.getNpc();
	vector<Limb>& limbs = npc.getLimbs();
	bool limbsLeftToUnrotate = false;
	SDL_Rect limbRect = { 0, 0, 0, 0 };

	setNpcAttackAdvance();

	for (int i = 0; i < limbs.size(); ++i) {
		Limb& limb = limbs[i];
		if (limb.isEquipped()) {
			bool drawLimb = !animateEffect || drawFlashingLimb || !intsContainInt(limbIdsToUpdate, limb.getId());

			SDL_Point rotationPoint = limb.getRotationPointSDL();
			limbRect = limb.getDrawRect();
			limbRect.x += npcDrawStartPoint.x + attackAdvanceNpc;
			limbRect.y += npcDrawStartPoint.y - bobbingMeter;

			int limbRotationAngle = limb.getRotationAngle();

			if (animateBrainDrain && limb.getBodyPartType() == BodyPartType::Head) {
				limbRotationAngle -= headRotation;
			}

			if (drawLimb) {
				SDL_RenderCopyEx(
					ui.getMainRenderer(),
					limb.getTexture(),
					NULL,
					&limbRect,
					limbRotationAngle,
					&rotationPoint,
					SDL_FLIP_NONE);
			}			

			/* Rotate limbs to make the NPC dance. */

			if (limbToRotateId == limb.getId()) {
				limb.rotate(rotationAmount);
				npc.setChildLimbDrawRects(limb, ui);
			}

			/* Player rotation logic. */
			if (!unrotateNpc) {
				if (limbToRotateId == limb.getId()) {
					limb.rotate(rotationAmount);
					npc.setChildLimbDrawRects(limb, ui);
				}
			}
			else {
				if (limbRotationAngle > 2) {
					if (limbRotationAngle < 180) {
						limb.rotate(-2);
						npc.setChildLimbDrawRects(limb, ui);
						limbsLeftToUnrotate = true;
					}
					else {
						limb.rotate(2);
						npc.setChildLimbDrawRects(limb, ui);
						limbsLeftToUnrotate = true;
					}					
				}
				else  if (limbRotationAngle < -2) {
					limb.rotate(2);
					npc.setChildLimbDrawRects(limb, ui);
					limbsLeftToUnrotate = true;
				}
			}
		}
	}

	if (unrotateNpc && !limbsLeftToUnrotate) {
		unrotateNpc = false;
	}
}


void BattleScreen::drawPlayer(UI& ui) {
	Character& playerCharacter = battle.getPlayerCharacter();
	vector<Limb>& limbs = playerCharacter.getLimbs();

	setPlayerAttackAdvance();

	SDL_Rect limbRect = { 0, 0, 0, 0 };
	bool limbsLeftToUnrotate = false;

	for (int i = 0; i < limbs.size(); ++i) {
		Limb& limb = limbs[i];
		if (limb.isEquipped()) {

			bool drawLimb = !animateEffect || drawFlashingLimb || !intsContainInt(limbIdsToUpdate, limb.getId());

			SDL_Point rotationPoint = limb.getRotationPointSDL();
			limbRect = limb.getDrawRect();
			limbRect.x += playerDrawStartPoint.x + attackAdvancePlayer;
			limbRect.y += playerDrawStartPoint.y + bobbingMeter;

			int limbRotationAngle = limb.getRotationAngle();

			if (animateBrainDrain && limb.getBodyPartType() == BodyPartType::Head) {
				limbRotationAngle += headRotation;
			}

			if (drawLimb) {
				SDL_RenderCopyEx(
					ui.getMainRenderer(),
					limb.getTexture(),
					NULL,
					&limbRect,
					limbRotationAngle,
					&rotationPoint,
					SDL_FLIP_NONE);
			}
						

			/* Player rotation logic. */
			if (!unrotatePlayer) {
				if (limbToRotateId == limb.getId()) {
					limb.rotate(rotationAmount);
					playerCharacter.setChildLimbDrawRects(limb, ui);
				}
			}
			else {
				if (limbRotationAngle > 1) {
					if (limbRotationAngle < 180) {
						limb.rotate(-2);
						playerCharacter.setChildLimbDrawRects(limb, ui);
						limbsLeftToUnrotate = true;
					}
					else {
						limb.rotate(2);
						playerCharacter.setChildLimbDrawRects(limb, ui);
						limbsLeftToUnrotate = true;
					}
				}
				else  if (limbRotationAngle < -1) {
					limb.rotate(2);
					playerCharacter.setChildLimbDrawRects(limb, ui);
					limbsLeftToUnrotate = true;
				}
			}
		}
	}

	if (unrotatePlayer && !limbsLeftToUnrotate) {
		unrotatePlayer = false;
	}
}


void BattleScreen::draw(UI& ui) {
	/* draw panel(make this a function of the UI object which takes a panel as a parameter) */
	SDL_SetRenderDrawColor(ui.getMainRenderer(), 0, 0, 0, 1);
	SDL_RenderClear(ui.getMainRenderer());

	/* draw BG for now */
	//SDL_RenderCopyEx(ui.getMainRenderer(), bgTexture, &bgSourceRect, &bgDestinationRect, 0, NULL, SDL_FLIP_NONE);
	skyBG.draw();

	/* Draw the characters. */
	if (battle.isPlayerTurn()) {
		drawNpc(ui);
		drawPlayer(ui);
	}
	else if (battle.isNpcTurn()) {
		drawPlayer(ui);
		drawNpc(ui);
	}
	
	
	/* draw the title */
	if (showTitle) {
		SDL_RenderCopyEx(ui.getMainRenderer(), titleTexture, NULL, &titleRect, 0, NULL, SDL_FLIP_NONE);
	}

	settingsPanel.draw(ui);
	playerStatsPanel.draw(ui);
	npcStatsPanel.draw(ui);
	playerTurnPanel.draw(ui);
	npcLimbsPanel.draw(ui);
	confirmationPanel.draw(ui);
	passingMessagePanel.draw(ui);
	optionsMenu.draw(ui);


	SDL_RenderPresent(ui.getMainRenderer()); /* update window */
}


/* Create the texture with the name of the game */
void BattleScreen::createTitleTexture(UI& ui) {
	string titleString = "BATTLE!";
	auto [incomingTitleTexture, incomingTitleRect] = ui.createTitleTexture(titleString);
	titleTexture = incomingTitleTexture;
	titleRect = incomingTitleRect;
}


/* Screen has been resized. Rebuild! */
void BattleScreen::rebuildDisplay(Panel& settingsPanel) {
	UI& ui = UI::getInstance();
	ui.rebuildSettingsPanel(settingsPanel, ScreenType::Map);
	getBackgroundTexture(ui);
	createTitleTexture(ui);
}


/* Process user input */
void BattleScreen::handleEvent(SDL_Event& e, bool& running, GameState& gameState) {
	/* User pressed X to close */
	if (e.type == SDL_QUIT) {
		cout << "\nQUIT\n";
		running = false;
		return;
	}
	else {
		// user clicked
		if (e.type == SDL_MOUSEBUTTONDOWN) {
			int mouseX, mouseY;
			SDL_GetMouseState(&mouseX, &mouseY);
			AudioBooth& audioBooth = AudioBooth::getInstance();

			/* Check if confirmation panel is shown first (should deactivate other panels. */

			if (confirmationPanel.getShow()) {

				if (confirmationPanel.isInPanel(mouseX, mouseY)) {
					/* 
					* 
					*			YES/NO CONFIRMATION PANEL. 
					* 
					*/

					ButtonClickStruct clickStruct = confirmationPanel.checkButtonClick(mouseX, mouseY);
					cout << "\n\nCLICK CONFIRMATION PANEL \n\n";

					if (clickStruct.buttonOption == ButtonOption::Agree) {
						audioBooth.playClick();
						BattleStatus battleStatus = battle.getBattleStatus();

						if (battleStatus == BattleStatus::PlayerDefeat ||
							battleStatus == BattleStatus::PlayerVictory ||
							battleStatus == BattleStatus::RebuildRequired ||
							screenToLoadStruct.screenType == ScreenType::CharacterCreation
						) {
							running = false;
						}
						else if (battleStatus == BattleStatus::RanAway) {
							screenToLoadStruct.screenType = ScreenType::Map;
							running = false;
						}

						confirmationPanel.setShow(false);
					}
					else if (clickStruct.buttonOption == ButtonOption::Refuse) {
						audioBooth.playClick();
					}
				}				

			}
			else {
				audioBooth.playClick();
				if (playerTurnPanel.getShow() && playerTurnPanel.isInPanel(mouseX, mouseY)) {
					/* 
					* 
					*				BATTLE MENU.
					* 
					*/

					cout << "\n\nCLICKED BATTLE MENU \n\n";

					ButtonClickStruct clickStruct = playerTurnPanel.checkButtonClick(mouseX, mouseY);

					UI& ui = UI::getInstance();
					/* see what button might have been clicked : */
					switch (clickStruct.buttonOption) {
					case ButtonOption::BattleMove:
						cout << "\nBATTLE MOVE!\n";

						handlePlayerMove(clickStruct);

						break;
					case ButtonOption::Build:
						screenToLoadStruct.screenType = ScreenType::CharacterCreation;
						running = false;
						break;
					default:
						cout << "ERROR\n";

					}
				}
				else if (npcLimbsPanel.getShow() && npcLimbsPanel.isInPanel(mouseX, mouseY)) {
					/* NPC LIMBS PANEL. */

					cout << "\n\nCLICK NPC LIMB MENU \n\n";
					ButtonClickStruct clickStruct = npcLimbsPanel.checkButtonClick(mouseX, mouseY);


					/* It might be the "back" button (but usually won't be so deal with that first). */
					if (clickStruct.buttonOption == ButtonOption::LoadLimb) {
						audioBooth.playClick();
						/* Do the animation.
						* When it counts down we'll launch the calculations.
						* After the NEXT animation we'll execute the results.
						*/
						UI& ui = UI::getInstance();
						AudioBooth& audioBooth = AudioBooth::getInstance();
						playerAttackLoaded.targetLimbId = clickStruct.itemID;
						animateAttack = true;
						animationCountdown = 1000;
						attackAdvanceHitTarget = false;

						string attackMessage = "Player uses " + playerAttackLoaded.name + "!";
						passingMessageCountdown = 250;
						passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
						passingMessagePanel.setShow(true);

						playerStatsPanel.setShow(true);
						npcStatsPanel.setShow(true);
						npcLimbsPanel.setShow(false);

						/* Play swoop before the advance begins. Swoop is the sound of advance, not impact. */
						if (playerAttackLoaded.attackType == AttackType::Swoop || playerAttackLoaded.attackType == AttackType::Steal) {
							audioBooth.playSwoop();
						}
					}
					else if (clickStruct.buttonOption == ButtonOption::Back) {
						audioBooth.playClick();
						/* unload attack, reset panels. */
						playerTurnPanel.setShow(true);
						npcLimbsPanel.setShow(false);
						playerStatsPanel.setShow(true);
						npcStatsPanel.setShow(true);
						playerAttackLoaded = AttackStruct();
						return;
					}
				}
				else if (optionsMenu.getShow() && optionsMenu.isInPanel(mouseX, mouseY)) {
					/* OPTIONS MENU */

					ButtonClickStruct clickStruct = optionsMenu.checkButtonClick(mouseX, mouseY);

					if (clickStruct.buttonOption == ButtonOption::Exit) {
						cout << "Clicked EXIT" << endl;
						running = false;
					}

				}
			}
		}
	}
}


void BattleScreen::checkMouseLocation(SDL_Event& e) {
	/* check for mouse over(for button hover) */
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	/* send the x and y to the panel and its buttons to change the color */	
	if (confirmationPanel.getShow()) {
		confirmationPanel.checkMouseOver(mouseX, mouseY);
	}
	else {
		if (settingsPanel.getShow()) { settingsPanel.checkMouseOver(mouseX, mouseY); }
		if (playerTurnPanel.getShow()) { playerTurnPanel.checkMouseOver(mouseX, mouseY); }
		if (npcLimbsPanel.getShow()) { npcLimbsPanel.checkMouseOver(mouseX, mouseY); }
		if (optionsMenu.getShow()) { optionsMenu.checkMouseOver(mouseX, mouseY); }
	}
}


/*
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* BATTLE MOVE FUNCTIONS
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/




/*
* This interprets the user's input and sets up the Battle Move Sequence.
*/
void BattleScreen::handlePlayerMove(ButtonClickStruct clickStruct) {
	int clickID = clickStruct.itemID;

	UI& ui = UI::getInstance();
	AudioBooth& audioBooth = AudioBooth::getInstance();

	cout << "\nBATTLE MOVE!\n";

	/* Check which attack. */
	for (AttackStruct aStruct : playerAttackStructs) {
		if (aStruct.attackType == clickID) {
			cout << "Attack Type: " << aStruct.name << "\n";

			/* Now we know that ATTACK TYPE matches existing attack options,
			* Go through those options and do the attack.
			*/

			if (aStruct.attackType == AttackType::Attack ||
				aStruct.attackType == AttackType::Punch ||
				aStruct.attackType == AttackType::DoublePunch ||
				aStruct.attackType == AttackType::Kick ||
				aStruct.attackType == AttackType::BodySlam ||
				aStruct.attackType == AttackType::Swoop
				) {
				/* REGULAR HP-DAMAGE ATTACKS */

				/*
				* If we do BODY SLAM then it should automatically choose anchoredLimb.
				*/


				/* Load attack
				* Open opponent Limb List
				* Select Opponent Limb
				* Do Attack
				* Animate Attack (flashing limb)
				* Show message panel with effects of attack.
				*/

				playerAttackLoaded = aStruct;

				/* Open the NPC Limb panel and user chooses targetId. */
				playerTurnPanel.setShow(false);
				npcLimbsPanel.setShow(true);
				playerStatsPanel.setShow(false);
				npcStatsPanel.setShow(false);
			}
			else if (aStruct.attackType == AttackType::BrainDrain) {

				/* 
				* 1. No need to select a target.
				* 2. Turning heads instead of advanceAttack.
				* 3. Calculate damage in the run() function after spinHead animation is over.
				*/
				playerAttackLoaded = aStruct;

				UI& ui = UI::getInstance();
				playerAttackLoaded.targetLimbId = clickStruct.itemID;
				animateBrainDrain = true;
				headRotation = 0;
				headSpins = 0;
				animationCountdown = 1000;
				attackAdvanceHitTarget = false;

				string attackMessage = battle.getPlayerCharacter().getName() + " uses " + playerAttackLoaded.name + "!";
				passingMessageCountdown = 250;
				passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
				passingMessagePanel.setShow(true);

				playerStatsPanel.setShow(true);
				npcStatsPanel.setShow(true);
				npcLimbsPanel.setShow(false);
				playerTurnPanel.setShow(false);

				cout << "BRAIN DRAIN!\n";
				audioBooth.playBrainDrain();
			}
			else if (aStruct.attackType == AttackType::Steal) {

				/* This attack is general (all limbs), but is LESS effective on Torso body parts.
				* This is where SPEED and INTELLIGENCE help.
				*/

				cout << "STEALIMG LIMB!\n";

				/* Show the panel of limbs and choose something to steal */

				npcLimbsPanel.setShow(true);
				playerStatsPanel.setShow(false);
				npcStatsPanel.setShow(false);
				playerTurnPanel.setShow(false);

				for (AttackStruct attackStruct : playerAttackStructs) {
					if (attackStruct.attackType == AttackType::Steal) {
						playerAttackLoaded = attackStruct;
						break;
					}
				}
			}
			else if (aStruct.attackType == AttackType::RunAway) {
				/* 
				* calculate chances to run away
				* give a confirmation box saying "you ran away"
				* save battle status and return to map.
				* 
				* FOR NOW it always works. Add chances later.
				*/

				int numberOfPlayerLegs = 0;
				int numberOfNpcLegs = 0;

				Character& playerCharacter = battle.getPlayerCharacter();
				Character& npc = battle.getNpc();
				vector<Limb>& npcLimbs = npc.getLimbs();
				vector<Limb>& playerLimbs = playerCharacter.getLimbs();

				for (Limb& limb : playerLimbs) {
					if (limb.isEquipped() && (limb.getBodyPartType() == BodyPartType::Leg || limb.getBodyPartType() == BodyPartType::Wing)) {
						++numberOfPlayerLegs;
					}
				}

				for (Limb& limb : npcLimbs) {
					if (limb.isEquipped() && (limb.getBodyPartType() == BodyPartType::Leg || limb.getBodyPartType() == BodyPartType::Wing)) {
						++numberOfNpcLegs;
					}
				}

				bool playerRunsAway = false;

				if (playerCharacter.getSpeed() > npc.getSpeed() || numberOfPlayerLegs > numberOfNpcLegs) {
					playerRunsAway = true;
				}
				else {
					/* Roll the dice! */
					int playerShare = playerCharacter.getSpeed() + numberOfPlayerLegs;
					int npcShare = npc.getSpeed() + numberOfNpcLegs;
					int totalSides = playerShare + npcShare;
					int randomNumber = rand() % totalSides;

					if(randomNumber <= playerShare){
						playerRunsAway = true;
					}
				}

				if (playerRunsAway) {
					battle.setBattleStatus(BattleStatus::RanAway);
					updateBattleStatus(battle.getId(), BattleStatus::RanAway);
					string ranAwayString = battle.getPlayerCharacter().getName() + " ran away!";

					confirmationPanel.destroyTextures();
					confirmationPanel = getNewConfirmationMessage(confirmationPanel, ranAwayString, ConfirmationButtonType::OkCancel, false);
					confirmationPanel.setShow(true);
					audioBooth.playSwoop();
				}
				else {
					string failMessage = battle.getPlayerCharacter().getName() + " failed to run away!";
					passingMessageCountdown = 250;
					passingMessagePanel = ui.getNewPassingMessagePanel(failMessage, passingMessagePanel, true, true);
					passingMessagePanel.setShow(true);
					battle.switchTurn();
				}
			}
		}
	}
}

/* When player tries to steal a limb, this calculates the chances and takes a shot.
* Also, steals immediately.
*/
void BattleScreen::calculatePlayerSteal() {
	Character& npc = battle.getNpc();
	Character& playerCharacter = battle.getPlayerCharacter();
	vector<Limb>& npcLimbs = npc.getLimbs();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();
	int targetLimbId = playerAttackLoaded.targetLimbId;

	/*
	* 1. Get target limb and info about player stats
	* 2. Algorithm: The more hurt they are, the more likely the steal. Intelligence and Speed both boost it.
	* 
	* -- compare PLAYER speeds
	*/

	Limb& targetLimb = targetLimbId > 0 ? npc.getLimbById(targetLimbId) : npcLimbs[0];

	int numNpcEqLimbs = 0;
	int numPlayerEqLimbs = 0;

	for (Limb& limb : npcLimbs) {
		if (limb.isEquipped()) {
			++numNpcEqLimbs;
		}
	}

	
	for (Limb& limb : playerLimbs) {
		if (limb.isEquipped()) {
			++numPlayerEqLimbs;
		}
	}

	int boostDividerNpc = numNpcEqLimbs / 2;
	int boostDividerPlayer = numPlayerEqLimbs / 2;

	if (boostDividerNpc < 1) {
		boostDividerNpc = 1;
	}

	if (boostDividerPlayer < 1) {
		boostDividerPlayer = 1;
	}

	int playerSpeed = playerCharacter.getSpeed() / boostDividerPlayer;
	int playerIntelligence = playerCharacter.getIntelligence() / boostDividerPlayer;
	int npcIntelligence = npc.getIntelligence() / boostDividerNpc;
	int npcSpeed = npc.getSpeed() / boostDividerNpc;

	DominancePosition playerDomPosition = attackerDominancePosition(playerCharacter.getDominanceNode(), targetLimb.getDominanceNode());

	int drawSize = targetLimb.getForm().hp;

	/* 
	* Chance of steal STARTS at 0 with full health.
	* % chance of steal =  percentage damage taken.
	* 
	*/

	int chances = (drawSize - targetLimb.getHP()) +
		(playerSpeed - npcSpeed) +
		((playerIntelligence - npcIntelligence) / 2);

	int multiplier = chances >= 0 ? chances : -chances;
	if (playerDomPosition == DominancePosition::Dom) {
		chances += (multiplier / 3);
	} else if (playerDomPosition == DominancePosition::Sub) {
		chances -= (multiplier / 3);
	}

	int bingo = rand() % drawSize;
	stealSuccess = bingo <= chances;

	if (stealSuccess) {
		limbIdsToUpdate.insert(targetLimbId);
	}

	UI& ui = UI::getInstance();
	string stealMessage = stealSuccess ?
		playerCharacter.getName() + " steals limb!" :
		"STEAL FAILED";
	passingMessageCountdown = 250;
	passingMessagePanel = ui.getNewPassingMessagePanel(stealMessage, passingMessagePanel, true, true);
	passingMessagePanel.setShow(true);
}

void BattleScreen::calculatePlayerBrainDrain() {
	/*
	* Damage ATTACK but also STEAL INTELLIGENCE.
	* Add all affected limbs to the idsToUpdate.
	* Make that vector into an unordered_set.
	* 
	*/

	AttackType attackType = playerAttackLoaded.attackType;
	Character& playerCharacter = battle.getPlayerCharacter();
	Character& npc = battle.getNpc();
	vector<Limb>& npcLimbs = npc.getLimbs();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();
	Limb& targetLimb = npcLimbs[0];

	/* Calculate the attack. */
	int attack = 0;
	int numberOfEquippedLimbs = 0;
	int targetLimbId = -1;

	/* Score some player character data. */
	vector<int> equippedLimbIds = {};
	for (Limb& limb : playerCharacter.getLimbs()) {
		if (limb.isEquipped()) {
			equippedLimbIds.emplace_back(limb.getId());
			attack += limb.getStrength() + 1;
			++numberOfEquippedLimbs;
		}
	}

	/* Score some NPC data. */
	vector<int> equippedNpcLimbIds = {};
	for (Limb& limb : npcLimbs) {
		if (limb.isEquipped()) {
			equippedNpcLimbIds.emplace_back(limb.getId());

			if (limb.getBodyPartType() == BodyPartType::Head) {
				targetLimbId = limb.getId();
				targetLimb = limb;
			}
		}
	}


	if (targetLimbId < 1) {
		targetLimbId = equippedNpcLimbIds[rand() % equippedNpcLimbIds.size()];
		targetLimb = npc.getLimbById(targetLimbId);
	}

	attack = (attack / 8) + (numberOfEquippedLimbs / 2);
	int spreadAttack = 0;

	/* Give a possible Dominance Cycle boost or detriment. */
	DominancePosition playerDomPosition = attackerDominancePosition(playerCharacter.getDominanceNode(), targetLimb.getDominanceNode());
	if (playerDomPosition == DominancePosition::Dom) {
		attack += (attack / 3);
	} else if (playerDomPosition == DominancePosition::Sub) {
		attack -= (attack / 3);
	}

	/* Add some randomness. */
	int attackMod = rand() % ((attack / 10) + 1);
	if (rand() % 2 == 0) {
		attackMod *= -1;
	}

	attack += attackMod;

	/* Get the target limb and make a vector of ids for spread attack. */

	vector<int> connectedLimbIds;
	int totalDamage = 0;

	int precision = playerAttackLoaded.precision;
	int intensity = playerAttackLoaded.intensity;
	bringSumTo100(precision, intensity);

	int precisionDivided = precision / 10;

	if (precisionDivided < 1) {
		precisionDivided = 1;
	}

	spreadAttack = attack / precisionDivided;
	attack = attack - spreadAttack;

	/* attack must be higher. */
	if (spreadAttack > attack) {
		int tempNumber = spreadAttack;
		spreadAttack = attack;
		attack = tempNumber;
	}

	/* First get all touching limbs. */
	for (int i = npcLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = npcLimbs[i];

		for (Joint& joint : limb.getJoints()) {
			int connectedLimbId = joint.getConnectedLimbId();

			if (connectedLimbId == targetLimbId) {
				connectedLimbIds.push_back(limb.getId());
			}
			else if (limb.getId() == targetLimbId) {
				if (connectedLimbId >= 0) {
					connectedLimbIds.push_back(connectedLimbId);
				}
			}
		}
	}

	int intelligenceDrained = 0;
	int intelModMax = 15;

	/* Now get the actual target limb, while also attacking the other limbs. */
	for (int i = npcLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = npcLimbs[i];

		/* Attack the target limb. */
		if (limb.getId() == targetLimbId) {
			int intelMod = (rand() % intelModMax) + 5;
			intelligenceDrained += intelMod;
			limb.modifyIntelligence(-intelMod);
			limb.modifyHP(-attack);
			totalDamage -= attack;
			limbIdsToUpdate.insert(limb.getId());
		}
		else {
			if (precision < 90) {
				/* Attack the spread limbs. */
				for (int connectedLimbId : connectedLimbIds) {
					if (limb.getId() == connectedLimbId) {
						limb.modifyHP(-spreadAttack);
						totalDamage -= spreadAttack;
						limbIdsToUpdate.insert(limb.getId());
					}
				}
			}
		}
	}

	/* Now also give those intelligenceDrained points to the Player. */
	int intelDecreaser = intelligenceDrained + 0;
	while (intelDecreaser > 0) {
		
		int limbToBoostId = equippedLimbIds[rand() % equippedLimbIds.size()];
		int amountToBoost = (rand() % intelDecreaser) + 1;
		intelDecreaser -= amountToBoost;
		playerCharacter.getLimbById(limbToBoostId).modifyIntelligence(amountToBoost);
		limbIdsToUpdate.insert(limbToBoostId);
	}

	int damageDisplayNumber = totalDamage * -1;
	UI& ui = UI::getInstance();

	string attackMessage = playerCharacter.getName() + " does " + to_string(damageDisplayNumber) + " damage, and stole " + to_string(intelligenceDrained) + " intelligence.";
	passingMessageCountdown = 250;
	passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
	passingMessagePanel.setShow(true);


	/* Attack is calculated and saved to BattleScreen variables, to be used after Effect animation. */
}


void BattleScreen::calculateNpcBrainDrain() {
	/*
	* Damage ATTACK but also STEAL INTELLIGENCE.
	* Add all affected limbs to the idsToUpdate.
	* Make that vector into an unordered_set.
	*
	*/

	AttackType attackType = npcAttackLoaded.attackType;
	Character& playerCharacter = battle.getPlayerCharacter();
	Character& npc = battle.getNpc();
	vector<Limb>& npcLimbs = npc.getLimbs();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();

	/* Calculate the attack. */
	int attack = 0;
	int numberOfEquippedLimbs = 0;
	int targetLimbId = -1;
	Limb& targetLimb = playerLimbs[0];

	/* Score some player NPC data. */
	vector<int> equippedLimbIds = {};
	for (Limb& limb : npcLimbs) {
		if (limb.isEquipped()) {
			equippedLimbIds.emplace_back(limb.getId());
			attack += limb.getStrength() + 1;
			++numberOfEquippedLimbs;
		}
	}

	/* Score some Player data. */
	vector<int> equippedPlayerLimbIds = {};
	for (Limb& limb : playerLimbs) {
		if (limb.isEquipped()) {
			equippedPlayerLimbIds.emplace_back(limb.getId());

			if (limb.getBodyPartType() == BodyPartType::Head) {
				targetLimbId = limb.getId();
				targetLimb = limb;
			}
		}
	}

	if (targetLimbId < 1) {
		targetLimbId = equippedPlayerLimbIds[rand() % equippedPlayerLimbIds.size()];
	}

	attack = (attack / 8) + (numberOfEquippedLimbs / 2);
	int spreadAttack = 0;

	/* Get dominance cycle advantage. */
	DominancePosition playerDomPosition = attackerDominancePosition(npc.getDominanceNode(), targetLimb.getDominanceNode());
	if (playerDomPosition == DominancePosition::Dom) {
		attack += (attack / 3);
	}
	else if (playerDomPosition == DominancePosition::Sub) {
		attack -= (attack / 3);
	}

	/* Add some randomness. */
	int attackMod = rand() % ((attack / 10) + 1);
	if (rand() % 2 == 0) {
		attackMod *= -1;
	}

	attack += attackMod;

	/* Get the target limb and make a vector of ids for spread attack. */

	vector<int> connectedLimbIds;
	int totalDamage = 0;

	int precision = npcAttackLoaded.precision;
	int intensity = npcAttackLoaded.intensity;
	bringSumTo100(precision, intensity);

	int precisionDivided = precision / 10;

	if (precisionDivided < 1) {
		precisionDivided = 1;
	}

	spreadAttack = attack / precisionDivided;
	attack = attack - spreadAttack;

	/* attack must be higher. */
	if (spreadAttack > attack) {
		int tempNumber = spreadAttack;
		spreadAttack = attack;
		attack = tempNumber;
	}

	/* First get all touching limbs. */
	for (int i = playerLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = playerLimbs[i];

		for (Joint& joint : limb.getJoints()) {
			int connectedLimbId = joint.getConnectedLimbId();

			if (connectedLimbId == targetLimbId) {
				connectedLimbIds.push_back(limb.getId());
			}
			else if (limb.getId() == targetLimbId) {
				if (connectedLimbId >= 0) {
					connectedLimbIds.push_back(connectedLimbId);
				}
			}
		}
	}

	int intelligenceDrained = 0;
	int intelModMax = 15;

	/* Now get the actual target limb, while also attacking the other limbs. */
	for (int i = playerLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = playerLimbs[i];

		/* Attack the target limb. */
		if (limb.getId() == targetLimbId) {
			int intelMod = (rand() % intelModMax) + 5;
			intelligenceDrained += intelMod;
			limb.modifyIntelligence(-intelMod);
			limb.modifyHP(-attack);
			totalDamage -= attack;
			limbIdsToUpdate.insert(limb.getId());
		}
		else {
			if (precision < 90) {
				/* Attack the spread limbs. */
				for (int connectedLimbId : connectedLimbIds) {
					if (limb.getId() == connectedLimbId) {
						limb.modifyHP(-spreadAttack);
						totalDamage -= spreadAttack;
						limbIdsToUpdate.insert(limb.getId());
					}
				}
			}
		}
	}

	/* Now also give those intelligenceDrained points to the Player. */
	int intelDecreaser = intelligenceDrained + 0;
	while (intelDecreaser > 0) {

		int limbToBoostId = equippedLimbIds[rand() % equippedLimbIds.size()];
		int amountToBoost = (rand() % intelDecreaser) + 1;
		intelDecreaser -= amountToBoost;
		npc.getLimbById(limbToBoostId).modifyIntelligence(amountToBoost);
		limbIdsToUpdate.insert(limbToBoostId);
	}

	int damageDisplayNumber = totalDamage * -1;
	UI& ui = UI::getInstance();

	string attackMessage = npc.getName() + " does " + to_string(damageDisplayNumber) + " damage, and stole " + to_string(intelligenceDrained) + " intelligence.";
	passingMessageCountdown = 250;
	passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
	passingMessagePanel.setShow(true);


	/* Attack is calculated and saved to BattleScreen variables, to be used after Effect animation. */
}

/*
* BASIC ATTACK LOGIC is contained in this function.
* I may later move attack logic to dedicated functions, in a dedicated module.
* This is separate from the animations.
* Attack animation happens, then this function happens (and stores data), then effect animation happens (based on stored data).
* These calculations provide info for the next animation, but we don't execute on the calculations until after the last animation.
*/
void BattleScreen::calculatePlayerDamageAttackStruct(int sourceLimbId, int targetLimbId) {
	AudioBooth& audioBooth = AudioBooth::getInstance();
	/* This should happen AFTER the animation.*/
	AttackType attackType = playerAttackLoaded.attackType;
	bool isSwoop = attackType == AttackType::Swoop;

	Character& playerCharacter = battle.getPlayerCharacter();
	Character& npc = battle.getNpc();
	vector<Limb>& npcLimbs = npc.getLimbs();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();

	Limb& targetLimb = targetLimbId > 0 ? npc.getLimbById(targetLimbId) : npcLimbs[0];
	audioBooth.playAttack();
	npcLimbsPanel.setShow(false);

	/*
	* Calculate how much of which attributes to take from which limbs.
	* When a limb dies, figure out what to do with limbs attached to it.
	* What are the rules?
	* -- 0 hp limbs are stolen.
	* -- If it's the anchored limb, find another limb to be an anchored limb (prefer ones with their own child limbs).
	* sourceLimb to fly spinning at targetLimb -- calculated to do full 360s and return upright as normal... wants to share x/y with oppo
	*
	*
	* CREATE ANIMATION STRUCT.
	* BattleAnimationStruct
	* EffectAnimationStruct
	* 
	* 
	* ATTACK LOGIC:
	* 
	* 1 -  all Strength of equipped limbs, plus number of equipped limbs.
	* 2 - divide by five, add number of equipped limbs (again).
	* 3 - SPREAD ATTACK : attack spreads some percentage to other limbs.
	* ----> High precision and high intelligence mean less spread.
	* 
	* TO DO:
	* --- Actually use intelligence.
	* --- Implement randomness.
	*/

	/* Calculate the attack. */
	int attack = 0;
	int numberOfEquippedLimbs = 0;
	for (Limb& limb : playerCharacter.getLimbs()) {
		if (limb.isEquipped()) {
			attack += limb.getStrength() + 1;
			++numberOfEquippedLimbs;
		}
	}

	attack = (attack / 5) + numberOfEquippedLimbs;
	int spreadAttack = 0;

	int attackDivider = !isSwoop ? 10 : 14;

	/* Get dominance cycle advantage. */
	DominancePosition playerDomPosition = attackerDominancePosition(playerCharacter.getDominanceNode(), targetLimb.getDominanceNode());
	if (playerDomPosition == DominancePosition::Dom) {
		attack += (attack / 3);
	}
	else if (playerDomPosition == DominancePosition::Sub) {
		attack -= (attack / 3);
	}

	/* Add some randomness. */
	int attackMod = rand() % ((attack / attackDivider) + 1);
	if (rand() % 2 == 0) {
		attackMod *= -1;
	}

	attack += attackMod;

	/* Get the target limb and make a vector of ids for spread attack. */

	vector<int> connectedLimbIds;
	int totalDamage = 0;

	int precision = playerAttackLoaded.precision;
	int intensity = playerAttackLoaded.intensity;
	bringSumTo100(precision, intensity);

	if (precision < 90) {
		int precisionDivided = precision / 10;

		if (precisionDivided < 1) {
			precisionDivided = 1;
		}

		spreadAttack = attack / precisionDivided;
		attack = attack - spreadAttack;

		/* attack must be higher. */
		if (spreadAttack > attack) {
			int tempNumber = spreadAttack;
			spreadAttack = attack;
			attack = tempNumber;
		}

		/* First get all touching limbs. */
		for (int i = npcLimbs.size() - 1; i >= 0; --i) {
			Limb& limb = npcLimbs[i];

			for (Joint& joint : limb.getJoints()) {
				int connectedLimbId = joint.getConnectedLimbId();

				if (connectedLimbId == targetLimbId) {
					connectedLimbIds.push_back(limb.getId());
				}
				else if (limb.getId() == targetLimbId) {
					if (connectedLimbId >= 0) {
						connectedLimbIds.push_back(connectedLimbId);
					}
				}
			}
		}
	}

	int stolenSpeed = 0;
	int speedStealAmount = !isSwoop ? 0 : (rand() % 10) + 9;

	/* Get dominance cycle advantage. */
	if (playerDomPosition == DominancePosition::Dom) {
		speedStealAmount += (speedStealAmount / 3);
	}
	else if (playerDomPosition == DominancePosition::Sub) {
		speedStealAmount -= (speedStealAmount / 3);
	}

	int quarterSteal = speedStealAmount / 4;

	/* Now get the actual target limb, while also attacking the other limbs. */
	for (int i = npcLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = npcLimbs[i];

		/* Attack the target limb. */
		if (limb.getId() == targetLimbId) {
			limb.modifyHP(-attack);
			totalDamage -= attack;
			limbIdsToUpdate.insert(limb.getId());

			if (isSwoop) {
				stolenSpeed += speedStealAmount;
				limb.modifySpeed(-speedStealAmount);
			}
		}
		else {
			if (precision < 90) {
				/* Attack the spread limbs. */
				for (int connectedLimbId : connectedLimbIds) {
					if (limb.getId() == connectedLimbId) {
						limb.modifyHP(-spreadAttack);
						totalDamage -= spreadAttack;
						limbIdsToUpdate.insert(limb.getId());

						if (isSwoop) {
							stolenSpeed += quarterSteal;
							limb.modifySpeed(-quarterSteal);
						}
					}
				}
			}
		}
	}

	if (isSwoop) {
		int speedAbsorbed = 0;
		for (Limb& limb: playerLimbs) {
			if (limb.isEquipped()) {
				int amountToGiveBack = (rand() % (stolenSpeed - speedAbsorbed)) + 1;
				limb.modifySpeed(amountToGiveBack);
				speedAbsorbed += amountToGiveBack;
			}

			if (speedAbsorbed >= stolenSpeed) {
				break;
			}
		}
	}

	int damageDisplayNumber = totalDamage * -1;
	UI& ui = UI::getInstance();

	string attackMessage = playerCharacter.getName() + " does " + to_string(damageDisplayNumber) + " damage";

	if (isSwoop) {
		attackMessage += ", and stole " + to_string(stolenSpeed) + " speed";
	}

	attackMessage += " with " + attackTypeText(playerAttackLoaded.attackType) + "!";

	passingMessageCountdown = 250;
	passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
	passingMessagePanel.setShow(true);

	/* Attack is calculated and saved to BattleScreen variables, to be used after Effect animation. */
}


/*
* BASIC ATTACK LOGIC is contained in this function.
* 
* Attack animation happens first, then this function happens (and stores data), then effect animation happens (based on stored data).
* These calculations provide info for the next animation, but we don't execute on the calculations until after the last animation.
* The reason we don't execute yet is because the affected Limbs and character must still be available and together for their animation.
*/
void BattleScreen::calculateNpcDamageAttackStruct(int sourceLimbId, int targetLimbId) {
	AttackType attackType = npcAttackLoaded.attackType;
	bool isSwoop = attackType == AttackType::Swoop;
	Character& playerCharacter = battle.getPlayerCharacter();
	Character& npc = battle.getNpc();
	vector<Limb>& npcLimbs = npc.getLimbs();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();
	Limb& targetLimb = targetLimbId > 0 ? playerCharacter.getLimbById(targetLimbId) : playerLimbs[0];

	AudioBooth& audioBooth = AudioBooth::getInstance();
	audioBooth.playAttack();

	/*
	* Calculate how much of which attributes to take from which limbs.
	* When a limb dies, figure out what to do with limbs attached to it.
	* What are the rules?
	*/

	/* Calculate the attack. */
	int attack = 0;
	int numberOfEquippedLimbs = 0;
	for (Limb& limb : npcLimbs) {
		if (limb.isEquipped()) {
			attack += limb.getStrength() + 1;
			++numberOfEquippedLimbs;
		}
	}

	attack = (attack / 5) + numberOfEquippedLimbs; // add some RANDOMNESS.
	int spreadAttack = 0;

	/* Get dominance cycle advantage. */
	DominancePosition playerDomPosition = attackerDominancePosition(npc.getDominanceNode(), targetLimb.getDominanceNode());
	if (playerDomPosition == DominancePosition::Dom) {
		attack += (attack / 3);
	}
	else if (playerDomPosition == DominancePosition::Sub) {
		attack -= (attack / 3);
	}

	int attackDivider = !isSwoop ? 11 : 14;

	/* Add some randomness. */
	int attackMod = rand() % ((attack / attackDivider) + 1);
	if (rand() % 2 == 0) { attackMod *= -1; }
	attack += attackMod;

	/* Get the target limb and make a vector of ids for spread attack. */

	vector<int> connectedLimbIds;
	int totalDamage = 0;

	int precision = npcAttackLoaded.precision;
	int intensity = npcAttackLoaded.intensity;
	bringSumTo100(precision, intensity); /* may be dangerous? */

	if (precision < 90) {

		int precisionDivided = precision / 10;
		if (precisionDivided < 1) {
			precisionDivided = 1;
		}

		spreadAttack = attack / precisionDivided;
		attack = attack - spreadAttack;

		/* attack must be higher. */
		if (spreadAttack > attack) {
			int tempNumber = spreadAttack;
			spreadAttack = attack;
			attack = tempNumber;
		}

		/* First get all touching limbs. */
		for (int i = playerLimbs.size() - 1; i >= 0; --i) {
			Limb& limb = playerLimbs[i];

			for (Joint& joint : limb.getJoints()) {
				int connectedLimbId = joint.getConnectedLimbId();

				if (connectedLimbId == targetLimbId) {
					connectedLimbIds.push_back(limb.getId());
				}
				else if (limb.getId() == targetLimbId) {
					if (connectedLimbId >= 0) {
						connectedLimbIds.push_back(connectedLimbId);
					}
				}
			}
		}
	}

	int stolenSpeed = 0;
	int speedStealAmount = !isSwoop ? 0 : (rand() % 14) + 5;

	if (playerDomPosition == DominancePosition::Dom) {
		speedStealAmount += (speedStealAmount / 3);
	}
	else if (playerDomPosition == DominancePosition::Sub) {
		speedStealAmount -= (speedStealAmount / 3);
	}

	int quarterSteal = speedStealAmount / 4;

	/* Now get the actual target limb, while also attacking the other limbs. */
	for (int i = playerLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = playerLimbs[i];

		/* Attack the target limb. */
		if (limb.getId() == targetLimbId) {
			limb.modifyHP(-attack);
			totalDamage -= attack;
			limbIdsToUpdate.insert(limb.getId());

			if (isSwoop) {
				stolenSpeed += speedStealAmount;
				limb.modifySpeed(-speedStealAmount);
			}
		}
		else {
			if (precision < 90) {
				/* Attack the spread limbs. */
				for (int connectedLimbId : connectedLimbIds) {
					if (limb.getId() == connectedLimbId) {
						limb.modifyHP(-spreadAttack);
						totalDamage -= spreadAttack;
						limbIdsToUpdate.insert(limb.getId());

						if (isSwoop) {
							stolenSpeed += quarterSteal;
							limb.modifySpeed(-quarterSteal);
						}
					}
				}
			}
		}
	}

	if (isSwoop) {
		int speedAbsorbed = 0;
		for (Limb& limb : npcLimbs) {
			if (limb.isEquipped()) {
				int amountToGiveBack = (rand() % (stolenSpeed - speedAbsorbed)) + 1;
				limb.modifySpeed(amountToGiveBack);
				speedAbsorbed += amountToGiveBack;
			}

			if (speedAbsorbed >= stolenSpeed) {
				break;
			}
		}
	}

	int damageDisplayNumber = totalDamage * -1;
	UI& ui = UI::getInstance();

	string attackMessage = npc.getName() + " does " + to_string(damageDisplayNumber) + " damage";

	if (isSwoop) {
		attackMessage += ", and stole " + to_string(stolenSpeed) + " speed";
	}

	attackMessage += " with " + attackTypeText(npcAttackLoaded.attackType) + "!";

	passingMessageCountdown = 250;
	passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
	passingMessagePanel.setShow(true);

	/* Attack is calculated and saved to BattleScreen variables, to be used after Effect animation. */

}


/*
* Apply the effects that we just calculated and animated.
* If player loses anchor limb, send them to Character Creation screen.
*/
bool BattleScreen::applyNpcAttackEffects() {
	Character& npc = battle.getNpc();
	Character& playerCharacter = battle.getPlayerCharacter();
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();
	vector<Limb>& npcLimbs = npc.getLimbs();
	int playerId = playerCharacter.getId();
	int npcId = npc.getId();
	UI& ui = UI::getInstance();

	sqlite3* db = startTransaction();

	/*
	* 1. Check for <1 hp and give those limbs to NPC.
	*   ---> remove them from player object and vector.
	* 2. Save all limbs and both characters.
	* 
	*/

	bool defeatedAnchorLimb = false;
	bool defeatedAnyLimb = false;
	int numberOfEquippableLimb = 0;
	unordered_set<int> removedIds = {};

	for (int i = playerLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = playerLimbs[i];
		int limbId = limb.getId();
		
		if (limb.getHP() > 0) {
			++numberOfEquippableLimb;

		} else if (limb.isEquipped()) {
			/* Equipped limb is defeated. Give it to NPC. */
			defeatedAnyLimb = true;

			if (playerCharacter.getAnchorLimbId() == limbId) {
				defeatedAnchorLimb = true;
			}

			removedIds.insert(limbId);
		}
	}


	for (int idToRemove : removedIds) {
		if (playerCharacter.limbsContainId(idToRemove)) {
			Limb limbToRemove = playerCharacter.getLimbById(idToRemove);
			playerCharacter.unEquipLimb(idToRemove);
			limbToRemove.unEquip(); /* Must unEquip this one specifically... it's a COPY that we're sending. */
			limbToRemove.setCharacterId(npc.getId());
			npc.addLimb(limbToRemove);
			updateLimbBattleEffectsInTransaction(limbToRemove, db);
			playerLimbs.erase(std::remove(playerLimbs.begin(), playerLimbs.end(), limbToRemove), playerLimbs.end());
		}
	}


	/* If limb is still connected to a limbs that's been removed, disconnect. */
	for (Limb& limb : playerLimbs) {
		for (Joint& joint : limb.getJoints()) {
			int connectedLimbId = joint.getConnectedLimbId();
			if (connectedLimbId > 0 && removedIds.count(connectedLimbId) > 0) {
				cout << "Manually detached something FROM limb " << limb.getName() << "\n";
				joint.detachLimb();
				updateLimbBattleEffectsInTransaction(limb, db);
			}
		}
	}
	
	if (!playerCharacter.limbsContainId(playerCharacter.getAnchorLimbId())) {
		defeatedAnchorLimb = true;
	}

	if (defeatedAnchorLimb) {
		cout << "DEFEATED ANCHOR LIMB.\n Sending to Character Creation Screen.\n";
		playerCharacter.clearSuit();
		updateCharacterAnchorIdInTrans(playerId, -1, db);

		if (numberOfEquippableLimb > 0) {
			cout << "Sending to Character Creation Screen.\n";
			screenToLoadStruct.screenType = ScreenType::CharacterCreation;
			setExitMessage(BattleStatus::RebuildRequired);
			updateBattleStatusInTrans(battle.getId(), BattleStatus::RebuildRequired, db);
		}
		else {
			cout << "Sending to Menu DEFEATED.\n";
			screenToLoadStruct.screenType = ScreenType::Menu;
			setExitMessage(BattleStatus::PlayerDefeat);
			cout << "PLAyer DEFOoTED\n";
			updateBattleStatusInTrans(battle.getId(), BattleStatus::PlayerDefeat, db);
		}
	}

	playerCharacter.buildDrawLimbList();

	/* Save all the limbs. */
	
	for (Limb& limb : playerLimbs) {
		updateLimbBattleEffectsInTransaction(limb, db);	
	}
	if (defeatedAnyLimb) {
		npc.buildDrawLimbList();
		for (Limb& limb : npcLimbs) {
			updateLimbBattleEffectsInTransaction(limb, db);
		}
	}

	updateCharacterAnchorIdInTrans(playerCharacter.getId(), playerCharacter.getAnchorLimbId(), db);

	commitTransactionAndCloseDatabase(db);
	createPlayerLimbPanels();
	playerStatsPanel.destroyTextures();
	playerStatsPanel = ui.createStatsPanel(ScreenType::Battle, playerCharacter.getCharStatsData(), false);

	npcAttackLoaded = AttackStruct();
	limbIdsToUpdate = {};

	/* If player has no EQUIPPABLE limbs they lose. */

	int equippableLimbsCount = 0;
	for (Limb& limb : playerLimbs) {
		if (limb.getHP() > 0) {
			++equippableLimbsCount;
		}
	}

	if (equippableLimbsCount < 1) {
		setExitMessage(BattleStatus::PlayerDefeat);
		updateBattleStatus(battle.getId(), BattleStatus::PlayerDefeat);
	}
	else {
		/* Reload playerTurnPanel in case they lost a limb. */
		playerTurnPanel.destroyTextures();
		playerTurnPanel = ui.createBattlePanel(playerAttackStructs, playerStatsPanel.getRect().h);
	}

	return true;
}


bool BattleScreen::applyPlayerStealEffects() {
	if (!stealSuccess || playerAttackLoaded.targetLimbId < 1) { return false; }

	Character& npc = battle.getNpc();
	Character& playerCharacter = battle.getPlayerCharacter();
	vector<Limb>& npcLimbs = npc.getLimbs();
	int targetLimbId = playerAttackLoaded.targetLimbId;
	Limb& targetLimb = npc.getLimbById(targetLimbId);

	/*
	* STEAL SUCCESS.
	* Take the limb immediately.
	*/

	/* Take the limb immediately. */
	sqlite3* db = startTransaction();
	unordered_set<int> stolenLimbChildIds = npc.getChildLimbIdsRecursively(targetLimb);

	for (int i = npcLimbs.size() - 1; i >= 0; --i) {
		Limb& limb = npcLimbs[i];
		if (limb.getId() == targetLimbId) {
			/* Steal the target limb. */

			npc.unEquipLimb(targetLimbId);
			targetLimb.setCharacterId(playerCharacter.getId());
			playerCharacter.addLimb(targetLimb);
			updateLimbBattleEffectsInTransaction(limb, db);
			npcLimbs.erase(npcLimbs.begin() + i);
			continue;
		}
		else if (stolenLimbChildIds.count(limb.getId()) > 0) {
			/* Unequip the child limbs. */
			npc.unEquipLimb(limb.getId());
		}
	}

	npc.buildDrawLimbList();
	/* Update the rest, which might have been referencing the removed limbs. */
	for (int i = 0; i < npcLimbs.size(); ++i) {
		updateLimbBattleEffectsInTransaction(npcLimbs[i], db);
	}

	if (stolenLimbChildIds.size() > 0) {
		/* Rebuild suit. Save. */

		for (Limb& limb : npcLimbs) {
			npc.unEquipLimb(limb.getId());
		}

		npc.sortLimbsByNumberOfJoints();
		bool keepEquippingLimbs = true;

		for (Limb& limb : npcLimbs) {
			if (keepEquippingLimbs) {
				if (stolenLimbChildIds.count(limb.getId()) > 0) {
					keepEquippingLimbs = npc.equipLimb(limb.getId());
				}
			}
			else { break; }
		}

		npc.buildDrawLimbList();
		updateCharacterLimbsInTransaction(npc.getId(), npc.getAnchorLimbId(), npcLimbs, db);
		if (npc.getNumberOfEquippedLimbs() > 0) {
			npc.setTexture(npc.createAvatar());
		}
		
	}

	updateCharacterAnchorIdInTrans(npc.getId(), npc.getAnchorLimbId(), db);
	setLimbIdList();

	UI& ui = UI::getInstance();

	npcStatsPanel.destroyTextures();
	npcStatsPanel = ui.createStatsPanel(ScreenType::Battle, npc.getCharStatsData());
	createNpcLimbPanel();

	/* Now check for NPC defeat. */

	if (npc.getNumberOfEquippedLimbs() < 1) {
		/* NPC defeat. */

		/* Check to see if they have any limbs to drop. */
		if (npcLimbs.size() > 0) {
			for (Limb& limb: npcLimbs) {
				limb.unEquip();
				limb.setCharacterId(playerCharacter.getId());
				updateLimbBattleEffectsInTransaction(limb, db);
			}
		}

		npc.buildDrawLimbList();
		/* Now destroy the NPC from the database. */
		deleteCharacterInTrans(npc.getId(), db);
		setLimbIdList();

		setExitMessage(BattleStatus::PlayerVictory);
		updateBattleStatusInTrans(battle.getId(), BattleStatus::PlayerVictory, db);
	}

	/* Now switch turns. */

	createNpcLimbPanel();
	npcStatsPanel.destroyTextures();
	npcStatsPanel = ui.createStatsPanel(ScreenType::Battle, npc.getCharStatsData());
	playerStatsPanel.setShow(true);

	playerAttackLoaded = AttackStruct();
	limbIdsToUpdate = {};
	stealSuccess = false;

	updateBattleStatusInTrans(battle.getId(), battle.getBattleStatus(), db);

	string thisTurn = battle.getBattleStatus() == BattleStatus::NpcTurn ?
		"NPC turn" :
		battle.getBattleStatus() == BattleStatus::PlayerTurn ?
		"Player turn" :
		"Neither Turn (end of battle)";

	commitTransactionAndCloseDatabase(db);

	return true;
}

/*
* Apply the effects that we just calculated and animated.
* If npc loses any limbs, rebuild character with already-equipped limbs ONLY.
*/
bool BattleScreen::applyPlayerAttackEffects() {
	Character& npc = battle.getNpc();
	Character& playerCharacter = battle.getPlayerCharacter();
	int playerId = playerCharacter.getId();
	int npcId = npc.getId();
	UI& ui = UI::getInstance();

	sqlite3* db = startTransaction();

	vector<Limb>& npcLimbs = npc.getLimbs();
	bool erasedLimb = false;

	unordered_set<int> limbIdsToEquip; /* CURRENTLY EQUIPPED limb ids. We will remove the ones that get destroyed (never rebuild from whole set for NPC). */
	unordered_set<int> removedIds = {};
	for (Limb& limb : npcLimbs) {
		if (limb.isEquipped()) {
			limbIdsToEquip.insert(limb.getId());
		}
	}

	/* Damage has already been done (to attributes). Now save those changes. */
	for (int i = 0; i < npcLimbs.size(); ++i) {
		Limb& limb = npcLimbs[i];

		if (limb.isEquipped() && limb.getHP() < 1) {
			erasedLimb = true;
			int limbId = limb.getId();

			if (limbIdsToEquip.count(limbId) > 0) {
				limbIdsToEquip.erase(limbId);
			}

			removedIds.insert(limbId);
		}
	}

	for (int idToRemove : removedIds) {
		if (npc.limbsContainId(idToRemove)) {
			Limb limbToRemove = npc.getLimbById(idToRemove);
			cout << "11111 Going to remove NPC limb " << limbToRemove.getId() << ", " << limbToRemove.getName() << endl;
			npc.unEquipLimb(idToRemove);
			limbToRemove.unEquip(); /* Must unEquip this one specifically... it's a COPY that we're sending. */
			limbToRemove.setCharacterId(playerCharacter.getId());
			playerCharacter.addLimb(limbToRemove); /* Limb must be added AFTER we change the characterId! */
			cout << "Limb to remove character id is " << limbToRemove.getCharacterId() << endl;
			updateLimbBattleEffectsInTransaction(limbToRemove, db);
			npcLimbs.erase(std::remove(npcLimbs.begin(), npcLimbs.end(), limbToRemove), npcLimbs.end());
		}
	}

	/* Update all survivors in the DB (also updates the joints which might previously have linked to now-removed limbs). */
	for (int i = 0; i < npcLimbs.size(); ++i) {
		if (removedIds.count(npcLimbs[i].getId()) > 0) {
			cout << "Removed limb STILL INSIDE NPC LIST (erase didn't work)\n";
		}
		updateLimbBattleEffectsInTransaction(npcLimbs[i], db);
	}

	npc.buildDrawLimbList();
	bool npcDefeated = false;

	if (erasedLimb) {

		/* REBUILD CHARACTER.
		* We need a function to REBUILD CHARACTER after losing a limb.
		* But we can only rebuild with limbs that are ALREADY equipped.
		* Don't make the player fight through the entire catalog of limbs.
		*
		* ALSO check for VICTORY CONDITIONS.
		*/
		npc.clearSuit();
		for (Limb& limb : npcLimbs) {
			if (removedIds.count(limb.getId()) > 0) {
				cout << "Removed limb STILL INSIDE NPC LIST (erase didn't work) 22222 \n";
			}
			npc.unEquipLimb(limb.getId());
			updateLimbBattleEffectsInTransaction(limb, db);
		}


		if (limbIdsToEquip.size() > 0) {
			npc.sortLimbsByNumberOfJoints();
			bool keepEquippingLimbs = true;

			for (int i = 0; i < npcLimbs.size(); ++i) {
				Limb& limb = npcLimbs[i];

				if (keepEquippingLimbs) {
					for (int limbIdToEquip : limbIdsToEquip) {
						if (limb.getId() == limbIdToEquip) {
							keepEquippingLimbs = npc.equipLimb(limbIdToEquip);
							updateLimbBattleEffectsInTransaction(limb, db);
						}
					}
				}
				else { break; }
			}

			npc.setTexture(npc.createAvatar());
			npc.buildDrawLimbList();
			updateCharacterAnchorIdInTrans(npc.getId(), npc.getAnchorLimbId(), db);
			setLimbIdList();
		}
		else {
			npcDefeated = true;
		}
	}

	/* After all this, make sure that the npc has equipped limbs. */
	if (npc.getNumberOfEquippedLimbs() < 1 or npc.getAnchorLimbId() < 1) {
		npcDefeated = true;
	}

	if (npcDefeated) {
		cout << "\n NPC DEFEATED\n\n";
		/* VICTORY CONDITIONS.
	* Player won the battle.
	*
	* Destroy NPC.
	* Spread limbs
	* ---> Player gets previously-equipped limbs.
	* ---> Non-equipped limbs return to Roaming Limbs (on the NPCs block).
	*/
		vector<Limb>& npcsLimbs = npc.getLimbs();
		for (Limb& limb : npcsLimbs) {
			npc.unEquipLimb(limb.getId());
			limb.unEquip();

			/* Move the Limb to either Player or RoamingLimbs, and save to the DB.
			* Character keeps defeated limbs. Limbs with HP shall roam.
			*/
			if (limb.getHP() > 0) {
				limb.setCharacterId(-1);
				limb.setPosition(npc.getPosition());
			}
			else {
				limb.setCharacterId(playerId);
			}
			updateLimbBattleEffectsInTransaction(limb, db); /* The limbs don't belong to the npc anymore. */
		}

		npc.buildDrawLimbList();
		/* Now destroy the NPC from the database. */
		deleteCharacterInTrans(npc.getId(), db);
		setLimbIdList();

		setExitMessage(BattleStatus::PlayerVictory);
		updateBattleStatusInTrans(battle.getId(), BattleStatus::PlayerVictory, db);

		/* Victory Condition gives you a popup message, and then go back to the map. */
	}
	else {
		cout << "NPC NOT DEFEATED YET \n";
	}

	for (Limb& limb : playerCharacter.getLimbs()) {
		updateLimbBattleEffectsInTransaction(limb, db);
	}

	commitTransactionAndCloseDatabase(db);
	createNpcLimbPanel();
	npcStatsPanel.destroyTextures();
	npcStatsPanel = ui.createStatsPanel(ScreenType::Battle, npc.getCharStatsData());

	playerStatsPanel.destroyTextures();
	playerStatsPanel = ui.createStatsPanel(ScreenType::Battle, playerCharacter.getCharStatsData(), false);
	playerStatsPanel.setShow(true);

	playerAttackLoaded = AttackStruct();
	limbIdsToUpdate = {};

	battle.switchTurn();
	updateBattleStatus(battle.getId(), battle.getBattleStatus());
	setLimbIdList();


	return true;
}


/* When the npc attacks the player, they must theatrically "advance" on their position.
* This function incrementally moves the NPC toward the player's position, and then bounces it back.
*/
void BattleScreen::setNpcAttackAdvance() {
	if (!animateAttack || battle.isPlayerTurn()) {
		attackAdvanceNpc = 0;
		return;
	}

	if (!attackAdvanceHitTarget && npcDrawStartPoint.x + attackAdvanceNpc > playerDrawStartPoint.x) {
		attackAdvanceNpc = attackAdvanceNpc - 20;
	}
	else if (!attackAdvanceHitTarget) {
		attackAdvanceHitTarget = true;
		attackAdvanceNpc = attackAdvanceNpc + 20;

		if (
			npcAttackLoaded.attackType == AttackType::Attack ||
			npcAttackLoaded.attackType == AttackType::Punch ||
			npcAttackLoaded.attackType == AttackType::DoublePunch ||
			npcAttackLoaded.attackType == AttackType::Kick ||
			npcAttackLoaded.attackType == AttackType::BodySlam ||
			npcAttackLoaded.attackType == AttackType::Swoop
			) {
			calculateNpcDamageAttackStruct(-1, npcAttackLoaded.targetLimbId);
		}

		animateEffect = true;
		animationCountdown = 100;
		flashingLimbCountdown = 10;
		drawFlashingLimb = false;
		flashLimb = true;

		/* We deal with end of Effect Animation in the run() function. */
	}
	else if (attackAdvanceNpc < 0) {
		attackAdvanceNpc = attackAdvanceNpc + 20;
	}
	else {
		animateAttack = !animateAttack;
		attackAdvanceNpc = 0;
	}
}


/* When the player attacks the npc, they must theatrically "advance" on their position.
* This function incrementally moves the player toward the NPC's position, and then bounces it back.
*/
void BattleScreen::setPlayerAttackAdvance() {
	if (!animateAttack || battle.isNpcTurn()) {
		attackAdvancePlayer = 0;
		return;
	}

	if (!attackAdvanceHitTarget && playerDrawStartPoint.x + attackAdvancePlayer < npcDrawStartPoint.x) { 
		attackAdvancePlayer = attackAdvancePlayer + 20;
	}
	else if (!attackAdvanceHitTarget) {
		attackAdvanceHitTarget = true;
		attackAdvancePlayer = attackAdvancePlayer - 20;

		if (playerAttackLoaded.attackType == AttackType::Steal) {
			calculatePlayerSteal();
			animateEffect = true;
			animationCountdown = 100;
			flashingLimbCountdown = 10;
			drawFlashingLimb = false;
			flashLimb = true;
		}
		else {
			calculatePlayerDamageAttackStruct(-1, playerAttackLoaded.targetLimbId);
			animateEffect = true;
			animationCountdown = 100;
			flashingLimbCountdown = 10;
			drawFlashingLimb = false;
			flashLimb = true;
		}


		/* We deal with end of Effect Animation in the run() function. */
	}
	else if (attackAdvancePlayer > 0) {
		attackAdvancePlayer = attackAdvancePlayer - 20;
	}
	else {
		animateAttack = !animateAttack;
		attackAdvancePlayer = 0;
	}
}



/*
* 
* 
* 
* 
* 
* 
* 
* 
* 
* 
*		NPC BATTLE MOVE FUNCTIONS
* 
* 
* 
* 
* 
* 
* 
* 
* 
*/


/* The equivalent of NPC pressing an attack button. */
void BattleScreen::launchNpcTurn() {
	cout << "Launching NPC turn\n";
	Character& npc = battle.getNpc();
	Character& playerCharacter = battle.getPlayerCharacter();

	/* Get the ATTACKS. */

	vector<AttackStruct> attackStructs = npc.getAttacks();

	if (attackStructs.size() < 1 && battle.isNpcTurn()) {
		battle.switchTurn();
		updateBattleStatus(battle.getId(), battle.getBattleStatus());
		playerTurnPanel.setShow(true);
		playerStatsPanel.setShow(true);
		npcStatsPanel.setShow(true);
		return;
	}

	npcAttackLoaded = attackStructs[rand() % attackStructs.size()];

	cout << "NPC chose " << attackTypeText(npcAttackLoaded.attackType) << endl;

	/* If player has no EQUIPPABLE limbs they lose. */

	int equippableLimbsCount = 0;
	vector<Limb>& playerLimbs = playerCharacter.getLimbs();
	for (Limb& limb : playerLimbs) {
		if (limb.getHP() >= 0) {
			++equippableLimbsCount;
		}
	}

	/* Make sure the player has limbs to attack. */
	if (equippableLimbsCount < 1) {

		cout << "PLAYER HAS NO --equippable-- LIMBS\n";
		// LOSING CONDITIONS.
		sqlite3* db = startTransaction();
		playerCharacter.clearSuit();
		updateCharacterAnchorIdInTrans(playerCharacter.getId(), playerCharacter.getAnchorLimbId(), db);
		updateCharacterLimbsInTransaction(npc.getId(), npc.getAnchorLimbId(), npc.getLimbs(), db);

		setExitMessage(BattleStatus::PlayerDefeat);
		updateBattleStatusInTrans(battle.getId(), BattleStatus::PlayerDefeat, db);
		commitTransactionAndCloseDatabase(db);

		return;
	}

	/* Get the TARGET LIMB ID. */

	vector<int> targetLimbIds;

	for (Limb& limb : playerCharacter.getLimbs()) {
		if (limb.isEquipped()) {
			targetLimbIds.push_back(limb.getId());
		}
	}

	if (targetLimbIds.size() < 1) {
		cout << "WE CANNOT FIGHT because the player has no limbs equipped\n";
		setExitMessage(BattleStatus::RebuildRequired);
		return;
		/* 
		* SEND the PLAYER to the CC screen...
		* unless they have no limbs at all... in that case player loses.
		*/
	}

	AttackType attackType = npcAttackLoaded.attackType;

	if (
		attackType == AttackType::Attack ||
		attackType == AttackType::Punch ||
		attackType == AttackType::DoublePunch ||
		attackType == AttackType::Kick ||
		attackType == AttackType::BodySlam ||
		attackType == AttackType::Steal || /* Must REMOVE thse TO DO */
		attackType == AttackType::Swoop || 
		attackType == AttackType::Throw ||
		attackType == AttackType::Heal 
	) {
		/* Load the targetLimbId, start the animation. */

		npcAttackLoaded.targetLimbId = targetLimbIds[rand() % targetLimbIds.size()];

		attackAdvanceHitTarget = false;
		animateAttack = true;

		string attackMessage = npc.getName() + " uses " + npcAttackLoaded.name + "!";
	}
	else if(attackType == AttackType::BrainDrain) {

		UI& ui = UI::getInstance();

		AudioBooth& audioBooth = AudioBooth::getInstance();
		audioBooth.playBrainDrain();

		npcAttackLoaded.targetLimbId = targetLimbIds[rand() % targetLimbIds.size()];
		animateBrainDrain = true;
		headRotation = 0;
		headSpins = 0;
		animationCountdown = 1000;
		attackAdvanceHitTarget = false;

		string attackMessage = npc.getName() + " uses " + npcAttackLoaded.name + "!";
		passingMessageCountdown = 250;
		passingMessagePanel = ui.getNewPassingMessagePanel(attackMessage, passingMessagePanel, true, true);
		passingMessagePanel.setShow(true);

		playerStatsPanel.setShow(true);
		npcStatsPanel.setShow(true);
		playerTurnPanel.setShow(false);
	}
	int number11 = 11;
	
}

/*
* 
* 
*    EXIT FUNCTIONS
* 
* 
*/

/*
* The exit message is the gatekeeper to actually leaving.
*/
void BattleScreen::setExitMessage(BattleStatus battleStatus) {
	
	battle.setBattleStatus(battleStatus);
	string exitMessage = "";
	Character& playerCharacter = battle.getPlayerCharacter();
	Character& npc = battle.getNpc();

	if (battleStatus == BattleStatus::PlayerDefeat) {
		exitMessage = npc.getName() + " has defeeeated " + playerCharacter.getName() + "!\n";
		
		screenToLoadStruct.screenType = ScreenType::Map;
		confirmationPanel.destroyTextures();
		confirmationPanel = getNewConfirmationMessage(confirmationPanel, exitMessage, ConfirmationButtonType::OkCancel, false);
		confirmationPanel.setShow(true);

	} else if (battleStatus == BattleStatus::PlayerVictory) {
		exitMessage = playerCharacter.getName() + " has defeated " + npc.getName() + "!\n";
		
		screenToLoadStruct.screenType = ScreenType::Map;
		confirmationPanel.destroyTextures();
		confirmationPanel = getNewConfirmationMessage(confirmationPanel, exitMessage, ConfirmationButtonType::OkCancel, false);
		confirmationPanel.setShow(true);

	} else if (battleStatus == BattleStatus::RebuildRequired) {
		exitMessage = "Rebuild required\n";
		screenToLoadStruct.screenType = ScreenType::CharacterCreation;
		confirmationPanel.destroyTextures();
		confirmationPanel = getNewConfirmationMessage(confirmationPanel, exitMessage, ConfirmationButtonType::OkCancel, false);
		confirmationPanel.setShow(true);
	}
	else {
		cout << "Exit message is WRONG\n";
	}
}