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

#ifndef __GLOBALCONTAINER_H
#define __GLOBALCONTAINER_H

#include <string>

#include "BuildingsTypes.h"
#include "Header.h"
#include "RessourcesTypes.h"
#include "Settings.h"

namespace GAGCore
{
	class FileManager;
	class GraphicContext;
	class Sprite;
	class Font;
}
using namespace GAGCore;

class SoundMixer;
class LogFileManager;

class GlobalContainer
{
public:
	enum { USERNAME_MAX_LENGTH=32 };
	enum { OPTION_LOW_SPEED_GFX=0x1 };

private:
	void initProgressBar(void);
	void updateLoadProgressBar(int value);
	void destroyProgressBar(void);

	const char *userName;

public:
	GlobalContainer(void);
	virtual ~GlobalContainer(void);

	void parseArgs(int argc, char *argv[]);
	void load(void);

	void pushUserName(const char *name);
	void popUserName();
	void setUserName(const char *name);
	const char *getUsername(void) { return userName; }

public:
	FileManager *fileManager;
	LogFileManager *logFileManager;

	GraphicContext *gfx;
	SoundMixer *mix;
	Sprite *terrain;
	Sprite *terrainWater;
	Sprite *terrainCloud;
	Sprite *terrainBlack;
	Sprite *terrainShader;
	Sprite *ressources;
	Sprite *ressourceMini;
	Sprite *bullet;
	Sprite *bulletExplosion;
	Sprite *units;
	Sprite *unitmini;
	Sprite *gamegui;
	Sprite *brush;
	Font *menuFont;
	Font *standardFont;
	Font *littleFont;

	Settings settings;

	BuildingsTypes buildingsTypes;
	RessourcesTypes ressourcesTypes;

	bool runNoX;
	char runNoXGameName[32];
	
	bool hostServer;
	char hostServerMapName[32];
	char hostServerUserName[32];
	char hostServerPassWord[32];
};

extern GlobalContainer *globalContainer;

#endif

