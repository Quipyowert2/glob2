/*
  Copyright (C) 2001-2004 Stephane Magnenat & Luc-Olivier de Charrière
  for any question or comment contact us at nct@ysagoon.com or nuage@ysagoon.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __GAME_GUI_H
#define __GAME_GUI_H

#include <queue>
#include <valarray>

#include "Header.h"
#include "Game.h"
#include "Order.h"
#include "Brush.h"

namespace GAGCore
{
	class Font;
}
using namespace GAGCore;

namespace GAGGUI
{
	class OverlayScreen;
}
using namespace GAGGUI;

class TeamStats;
class InGameTextInput;
class InGameScrollableText;


//! max unit working at a building
#define MAX_UNIT_WORKING 20
//! range of ratio for swarm
#define MAX_RATIO_RANGE 16

//! The Game Graphic User Interface
/*!
	Handle all user input during game, draw & handle menu.
*/
class GameGUI
{
public:
	Game game;
	bool gamePaused;
	bool hardPause;
	bool isRunning;
	//! true if user close the glob2 window.
	bool exitGlobCompletely;
	//! if this is not empty, then Engine should load the map with this filename.
	char toLoadGameFileName[SessionGame::MAP_NAME_MAX_SIZE+5];
	//bool showExtendedInformation;
	bool drawHealthFoodBar, drawPathLines, drawAccessibilityAids;
	int localPlayer, localTeamNo;
	int viewportX, viewportY;

public:
	GameGUI();
	~GameGUI();

	void init();
	void adjustInitialViewport();
	void adjustLocalTeam();
	//! Handle mouse, keyboard and window resize inputs, and stats
	void step(void);
	//! Get order from gui, return NullOrder if
	Order *getOrder(void);
	//! Return position on x
	int getViewportX() { return viewportX; }
	//! Return position on y
	int getViewportY() { return viewportY; }

	void drawAll(int team);
	void executeOrder(Order *order);

	//!
	bool loadBase(const SessionInfo *initial);
	//!
	bool load(GAGCore::InputStream *stream);
	void save(GAGCore::OutputStream *stream, const char *name);

	void processEvent(SDL_Event *event);

	// Engine has to call this every "real" steps. (or game steps)
	void syncStep(void);
	//! return the local team of the player who is running glob2
	Team *getLocalTeam(void) { return localTeam; }

	// Script interface
	void enableBuildingsChoice(const std::string &name);
	void disableBuildingsChoice(const std::string &name);
	void enableFlagsChoice(const std::string &name);
	void disableFlagsChoice(const std::string &name);
	void enableGUIElement(int id);
	void disableGUIElement(int id);

	// Stats for engine
	void setCpuLoad(int s);
	
private:
	// Helper function for key and menu
	void repairAndUpgradeBuilding(Building *building, bool repair, bool upgrade);
	
	bool processGameMenu(SDL_Event *event);
	bool processScrollableWidget(SDL_Event *event);
	void handleRightClick(void);
	void handleKey(SDLKey key, bool pressed, bool shift);
	void handleKeyAlways(void);
	void handleKeyDump(SDL_KeyboardEvent key);
	void handleMouseMotion(int mx, int my, int button);
	void handleMapClick(int mx, int my, int button);
	void handleMenuClick(int mx, int my, int button);
	void handleActivation(Uint8 state, Uint8 gain);
	void nextDisplayMode(void);
	void minimapMouseToPos(int mx, int my, int *cx, int *cy, bool forScreenViewport);

	// Drawing support functions
	void drawScrollBox(int x, int y, int value, int valueLocal, int act, int max);
	void drawXPProgressBar(int x, int y, int act, int max);
	void drawButton(int x, int y, const char *caption, bool doLanguageLookup=true);
	void drawBlueButton(int x, int y, const char *caption, bool doLanguageLookup=true);
	void drawRedButton(int x, int y, const char *caption, bool doLanguageLookup=true);
	void drawTextCenter(int x, int y, const char *caption, int i=-1);
	void drawValueAlignedRight(int y, int v);
	void drawCosts(int ressources[BASIC_COUNT], Font *font);

	void iterateSelection(void);
	void centerViewportOnSelection(void);
	
	//! Draw the top of screen bar, called by drawOverlayInfos
	void drawTopScreenBar(void);
	//! Draw the infos that are over the others, like the message, the waiting players, ...
	void drawOverlayInfos(void);

	//! Draw the panel
	void drawPanel(void);
	//! Draw the buttons associated to the panel
	void drawPanelButtons(int pos);
	//! Draw a choice of buildings or flags
	void drawChoice(int pos, std::vector<std::string> &types, std::vector<bool> &states, unsigned numberPerLine = 2);
	//! Draw the infos from a unit
	void drawUnitInfos(void);
	//! Draw the infos and actions from a building
	void drawBuildingInfos(void);
	//! Draw the infos about a ressource on map (type and number left)
	void drawRessourceInfos(void);

	//! Draw the menu during game
	void drawInGameMenu(void);
	//! Draw the message input field
	void drawInGameTextInput(void);
	//! Draw the message history field
	void drawInGameScrollableText(void);
	
	void moveFlag(int mx, int my, bool drop);
	//! Update the brush and the local map due to mouse motion
	void brushStep(int mx, int my);
	//! Send a brush order and reinitialize the brush accumulator
	void sendBrushOrders(void);
	//! One viewport has moved and a flag or a brush is selected, update its position
	void dragStep(void);
	//! on each step, check if we have won or lost
	void checkWonConditions(void);
	
	//! given the game state, change the music
	void musicStep(void);
	
	friend class InGameAllianceScreen;

	//! Display mode
	enum DisplayMode
	{
		BUILDING_VIEW=0,
		FLAG_VIEW,
		STAT_TEXT_VIEW,
		STAT_GRAPH_VIEW,
		NB_VIEWS,
	} displayMode;

	//! Selection mode
	enum SelectionMode
	{
		NO_SELECTION=0,
		BUILDING_SELECTION,
		UNIT_SELECTION,
		RESSOURCE_SELECTION,
		TOOL_SELECTION,
		BRUSH_SELECTION
	} selectionMode;
	union
	{
		Building* building;
		Unit* unit;
		const char *build;
		int ressource;
	} selection;
	
	//! Value between 0 and 1 that indicates if selection has to be highlighten
	float highlightSelection;

	//! Unset and clean everything related to the selection so a new one can be set
	void cleanOldSelection(void);
	void setSelection(SelectionMode newSelMode, void* newSelection=NULL);
	void setSelection(SelectionMode newSelMode, unsigned newSelection);
	void clearSelection(void) { setSelection(NO_SELECTION); }
	void checkSelection(void);
	
	// Brushes
	enum BrushType
	{
		FORBIDDEN_BRUSH = 0,
		GUARD_AREA_BRUSH = 1,
		CLEAR_AREA_BRUSH = 2
		
	} brushType;
	BrushTool brush;
	BrushAccumulator brushAccumulator;

	// What's visible or hidden on GUI
	std::vector<std::string> buildingsChoiceName;
	std::vector<bool> buildingsChoiceState;
	std::vector<std::string> flagsChoiceName;
	std::vector<bool> flagsChoiceState;
	enum HidableGUIElements
	{
		HIDABLE_BUILDINGS_LIST = 0x1,
		HIDABLE_FLAGS_LIST = 0x2,
		HIDABLE_TEXT_STAT = 0x4,
		HIDABLE_GFX_STAT = 0x8,
		HIDABLE_ALLIANCE = 0x10,
	};
	Uint32 hiddenGUIElements;

	//! True if the mouse's button way never relased since selection.
	bool selectionPushed;
	//! The position of the flag when it was pushed.
	Sint32 selectionPushedPosX, selectionPushedPosY;
	//! True if the mouse's button way never relased since click im minimap.
	bool miniMapPushed;
	//! True if we try to put a mark in the minimap
	bool putMark;
	//! True if we are panning
	bool panPushed;
	//! Coordinate of mouse when began panning
	int panMouseX, panMouseY;
	//! Coordinate of viewport when began panning
	int panViewX, panViewY;

	bool showUnitWorkingToBuilding;

	TeamStats *teamStats;
	Team *localTeam;

	Uint32 chatMask;

	std::list<Order *> orderQueue;

	int mouseX, mouseY;
	//! for mouse motion
	int viewportSpeedX, viewportSpeedY;

	// menu related functions
	enum
	{
		IGM_NONE=0,
		IGM_MAIN,
		IGM_LOAD,
		IGM_SAVE,
		IGM_OPTION,
		IGM_ALLIANCE,
		IGM_END_OF_GAME
	} inGameMenu;
	OverlayScreen *gameMenuScreen;

	bool hasEndOfGameDialogBeenShown;
	// On screen message handling
	struct Message
	{
		enum { DEFAULT_MESSAGE_SHOW_TICKS = 180 };
		int showTicks; // since when it is shown
		std::string text;
		Uint8 r, g, b, a; // color
	};
	
	std::list<Message> messagesList;
	std::vector<std::string> messageHistory;
	
	//! Transform a text to multi line according to screen width
	void setMultiLine(const std::string &input, std::vector<std::string> *output);

	//! add a message to the window message list
	void addMessage(Uint8 r, Uint8 g, Uint8 b, const char *msgText, ...);
	
	// Typing stuff :
	InGameTextInput *typingInputScreen;
	InGameScrollableText *scrollableText;
	int typingInputScreenPos;
	int typingInputScreenInc;
	
	// Minimap marking handling
	struct Mark
	{
		enum { DEFAULT_MARK_SHOW_TICKS = 50 };
		int showTicks; // since when it is shown
		int x, y; // position
		Uint8 r, g, b; // color
	};

	std::list<Mark> markList;

	//! add a minimap mark
	void addMark(MapMarkOrder *mmo);
	
	// how long the COU has been idle last tick
	#define SMOOTH_CPU_LOAD_WINDOW_LENGTH 32
	int smoothedCpuLoad[SMOOTH_CPU_LOAD_WINDOW_LENGTH];
	unsigned smoothedCpuLoadPos;
};

#endif

