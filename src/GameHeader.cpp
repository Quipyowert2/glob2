/*
  Copyright (C) 2007 Bradley Arsenault

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

#include "GameHeader.h"

GameHeader::GameHeader()
{
	numberOfPlayers = 0;
	gameLatency = 5;
}



bool GameHeader::load(GAGCore::InputStream *stream, Sint32 versionMinor)
{
	stream->readEnterSection("GameHeader");
	numberOfPlayers = stream->readSint32("numberOfPlayers");
	gameLatency = stream->readSint32("gameLatency");
	stream->readEnterSection("players");
	for(int i=0; i<32; ++i)
	{
		stream->readEnterSection(i);
		players[i].load(stream, versionMinor);
		stream->readLeaveSection(i);
	}
	stream->readLeaveSection();
	stream->readLeaveSection();
}



void GameHeader::save(GAGCore::OutputStream *stream)
{
	stream->writeEnterSection("GameHeader");
	stream->writeSint32(numberOfPlayers, "numberOfPlayers");
	stream->writeSint32(gameLatency, "gameLatency");
	stream->writeEnterSection("players");
	for(int i=0; i<32; ++i)
	{
		stream->writeEnterSection(i);
		players[i].save(stream);
		stream->writeLeaveSection(i);
	}
	stream->writeLeaveSection();
	stream->writeLeaveSection();
}



Sint32 GameHeader::getNumberOfPlayers() const
{
	return numberOfPlayers;
}



Sint32 GameHeader::getGameLatency() const
{
	return gameLatency;
}



void GameHeader::setGameLatency(Sint32 latency)
{
	gameLatency = latency;
}