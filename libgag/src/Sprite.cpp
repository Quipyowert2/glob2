/*
  Copyright (C) 2001-2004 Stephane Magnenat & Luc-Olivier de Charrière
  for any question or comment contact us at <stephane at magnenat dot net> or <NuageBleu at gmail dot com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <GraphicContext.h>
#include <math.h>
#include <Toolkit.h>
#include <FileManager.h>
#include <assert.h>
#include <glad/glad.h>
#include <SDL_image.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#define GL_GLEXT_PROTOTYPES
#ifdef HAVE_OPENGL
#if defined(__APPLE__) || defined(OPENGL_HEADER_DIRECTORY_OPENGL)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#ifdef WIN32
#include <GL/glext.h>
#endif


namespace GAGCore
{
	Sprite::RotatedImage::~RotatedImage()
	{
		delete orig;
		for (RotationMap::iterator it = rotationMap.begin(); it != rotationMap.end(); ++it)
		{
			delete it->second;
		}
	}
	
	bool Sprite::load(const std::string filename)
	{
		SDL_RWops *frameStream;
		SDL_RWops *rotatedStream;
		unsigned i = 0;
		
		this->fileName = filename;
		
		while (true)
		{
			std::ostringstream frameName;
			frameName << filename << i << ".png";
			frameStream = Toolkit::getFileManager()->open(frameName.str().c_str(), "rb");
	
			std::ostringstream frameNameRot;
			frameNameRot << filename << i << "r.png";
			rotatedStream = Toolkit::getFileManager()->open(frameNameRot.str().c_str(), "rb");
	
			if (!((frameStream) || (rotatedStream)))
				break;
	
			loadFrame(frameStream, rotatedStream);
	
			if (frameStream)
				SDL_RWclose(frameStream);
			if (rotatedStream)
				SDL_RWclose(rotatedStream);
			i++;
		}
		// TODO: How to cache rotated images?
		if (!images.empty() && rotated.empty())
			createTextureAtlas();
		
		return getFrameCount() > 0;
	}

	// Create texture atlas for images array
	// Using a sprite sheet lets us efficiently drawn terrain and water with a few calls
	// to glDrawArrays, rather than 272 individual calls to glBegin...glEnd.
	void Sprite::createTextureAtlas()
	{
#ifdef HAVE_OPENGL
		size_t numImages = images.size();
		int w = 0, h = 0;
		for (auto image : images)
		{
			if (!image)
				return;
			if (!w || !h)
			{
				w = image->getW();
				h = image->getH();
			}
			if (image->getW() != w || image->getH() != h)
				return;
		}
		int tileWidth = images[0]->getW();
		int tileHeight = images[0]->getH();
		int sheetWidth = tileWidth * (sqrt(numImages) + 1);
		int sheetHeight = tileHeight * (sqrt(numImages) + 1);
		atlas = new DrawableSurface(sheetWidth, sheetHeight);
		int x = 0, y = 0;
		for (auto image: images)
		{
			atlas->drawSurface(x, y, image);
			image->texX = x;
			image->texY = y;
			image->texMultX = 1.f;
			image->texMultY = 1.f;
			image->w = tileWidth;
			image->h = tileHeight;
			x += tileWidth;
			if (sheetWidth - x < tileWidth) {
				x = 0;
				y += tileHeight;
			}
		}
		atlas->uploadToTexture();
		for (auto image : images)
		{
			image->texture = atlas->texture;
			image->usingAtlas = true;
			image->sprite = this;
			image->setRes(sheetWidth, sheetHeight);
		}
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &texCoordBuffer);
#endif
	}
	
	DrawableSurface *Sprite::getRotatedSurface(int index)
	{
		RotatedImage::RotationMap::const_iterator it = rotated[index]->rotationMap.find(actColor);
		DrawableSurface *ds;
		if (it == rotated[index]->rotationMap.end())
		{
			// compute hue shift
			float baseHue, actHue, lum, sat;
			float hueShift;
			Color(51, 255, 153).getHSV(&baseHue, &sat, &lum);
			actColor.getHSV(&actHue, &sat, &lum);
			hueShift = actHue - baseHue;
			
			// rotate image
			ds = rotated[index]->orig->clone();
			ds->shiftHSV(hueShift, 0.0f, 0.0f);
			
			// write back
			rotated[index]->rotationMap[actColor] = ds;
		}
		else
		{
			ds = it->second;
		}
		return ds;
	}
	
	Sprite::~Sprite()
	{
		for (std::vector <DrawableSurface *>::iterator imagesIt = images.begin(); imagesIt != images.end(); ++imagesIt)
		{
			if (*imagesIt)
				delete (*imagesIt);
		}
		for (std::vector <RotatedImage *>::iterator rotatedIt=rotated.begin(); rotatedIt!=rotated.end(); ++rotatedIt)
		{
			if (*rotatedIt)
				delete (*rotatedIt);
		}
		delete atlas;
	}
	
	void Sprite::loadFrame(SDL_RWops *frameStream, SDL_RWops *rotatedStream)
	{
		if (frameStream)
		{
			SDL_Surface *sprite = IMG_Load_RW(frameStream, 0);
			assert(sprite);
			images.push_back(new DrawableSurface(sprite));
			SDL_FreeSurface(sprite);
		}
		else
			images.push_back(NULL);
	
		if (rotatedStream)
		{
			SDL_Surface *sprite = IMG_Load_RW(rotatedStream, 0);
			assert(sprite);
			rotated.push_back(new RotatedImage(new DrawableSurface(sprite)));
			SDL_FreeSurface(sprite);
		}
		else
			rotated.push_back(NULL);
	}
	
	int Sprite::getW(int index)
	{
		if (!checkBound(index))
			return 0;
		if (images[index])
			return images[index]->getW();
		else if (rotated[index])
			return rotated[index]->orig->getW();
		else
			return 0;
	}
	
	int Sprite::getH(int index)
	{
		if (!checkBound(index))
			return 0;
		if (images[index])
			return images[index]->getH();
		else if (rotated[index])
			return rotated[index]->orig->getH();
		else
			return 0;
	}
	
	int Sprite::getFrameCount(void)
	{
		return std::max(images.size(), rotated.size());
	}
	
	bool Sprite::checkBound(int index)
	{
		if ((index < 0) || (index >= getFrameCount()))
		{
			Toolkit::SpriteMap::const_iterator it = Toolkit::spriteMap.begin();
			while (it != Toolkit::spriteMap.end())
			{
				if (it->second == this)
				{
					std::cerr << "GAG : Sprite " << fileName << " ::checkBound(" << index << ") : error : out of bound access for " << it->first << std::endl;
					assert(false);
					return false;
				}
				++it;
			}
			std::cerr << "GAG : Sprite " << fileName << " ::checkBound(" << index << ") : error : sprite is not in the sprite server" << std::endl;
			assert(false);
			return false;
		}
		else
			return true;
	}
}
