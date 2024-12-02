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
#include <SDL_image.h>
#include <algorithm>
#include <iostream>
#include <sstream>

#if __cplusplus >= 201402L
#include <memory>
using std::make_unique;
#else
#if BOOST_VERSION >= 107500
#include <boost/smart_ptr/make_unique.hpp>
#elif BOOST_VERSION >= 106300
#include <boost/make_unique.hpp>
#elif BOOST_VERSION >= 105700
#include <boost/move/make_unique.hpp>
#else
#error "Can't make_unique when there's no Boost and C++ standard is earlier than C++14"
#endif
using boost::make_unique;
#endif // __cplusplus

#define GL_GLEXT_PROTOTYPES
#ifdef HAVE_OPENGL
#if defined(__APPLE__) || defined(OPENGL_HEADER_DIRECTORY_OPENGL)
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#include <OpenGL/glu.h>
#define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#include <epoxy/gl.h>
#ifdef _MSC_VER
#include <epoxy/wgl.h>
#else
#include <epoxy/glx.h>
#endif // _MSC_VER
#endif // defined(__APPLE__)
#endif // ifdef HAVE_OPENGL


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
		if (std::any_of(images.begin(), images.end(), [](DrawableSurface *s) {return s != nullptr; }) &&
			std::all_of(rotated.begin(), rotated.end(), [](RotatedImage *s) {return s == nullptr; }))
		{
			createTextureAtlas();
		}
		
		return getFrameCount() > 0;
	}

	void Sprite::addVertices(int sheetNo, const std::initializer_list<float> &verts)
	{
#ifdef HAVE_OPENGL
		assert(sheetNo != -1);
		vertices[sheetNo].insert(vertices[sheetNo].end(), verts);
#endif
	}

	void Sprite::addTextureCoordinates(int sheetNo, const std::initializer_list<float> &coords)
	{
#ifdef HAVE_OPENGL
		assert(sheetNo != -1);
		texCoords[sheetNo].insert(texCoords[sheetNo].end(), coords);
#endif
	}

#ifdef DEBUG_SPRITE_NOT_DRAWN
	std::vector<Sprite*> Sprite::sprites;
#endif

	void Sprite::checkAllSpritesDrawn()
	{
#ifdef DEBUG_SPRITE_NOT_DRAWN
		for (const Sprite* sprite : sprites)
			for (int i = 0;i < sprite->atlas.size();i++)
				if (sprite->vertices[i].size() || sprite->texCoords[i].size())
				{
					std::cout << "Warning: Sprite " << sprite->fileName << " has not been drawn" << std::endl;
				}
#endif
	}

	// Create texture atlas for images array
	// Using a sprite sheet lets us efficiently drawn terrain and water with a few calls
	// to glDrawArrays, rather than 272 individual calls to glBegin...glEnd.
	void Sprite::createTextureAtlas()
	{
#ifdef HAVE_OPENGL
		static int maxTextureSize = 0;
#ifdef DEBUG_SPRITE_NOT_DRAWN
		sprites.push_back(this);
#endif
		if (!maxTextureSize)
		{
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		}
		assert(maxTextureSize);
		size_t numImages = images.size();
		int tileWidth = 0, tileHeight = 0;
		// Check all tiles have the same size
		for (auto image : images)
		{
			if (!image)
				return;
			if (!tileWidth || !tileHeight)
			{
				tileWidth = image->getW();
				tileHeight = image->getH();
			}
			if (image->getW() != tileWidth || image->getH() != tileHeight)
				return;
		}
		int texturesNeeded = 0;
		typedef struct {
			int width;
			int height;
			int numImages;
		} TextureDim;
		std::vector<TextureDim> possibleDimensions;
		// Given maximum texture size, determine how many sprite sheets are needed.
		for (int widthInTiles = 1; widthInTiles < maxTextureSize / tileWidth; widthInTiles++)
		{
			int filledRows = numImages / widthInTiles;
			int leftOver = numImages % widthInTiles;
			int totalRows = filledRows + (bool)(leftOver > 0);
			if (totalRows * tileHeight <= maxTextureSize)
			{
				// It fits.
				TextureDim td = { widthInTiles, filledRows + (leftOver > 0), widthInTiles * filledRows + leftOver };
				possibleDimensions.push_back(td);
			}
		}
		// Pick the squarish one.
		auto bestSize = std::min_element(possibleDimensions.begin(), possibleDimensions.end(),
			[](const TextureDim& lhs, const TextureDim& rhs) -> bool {
				return abs(lhs.width - lhs.height) < abs(rhs.width - rhs.height);
		});
		assert(!possibleDimensions.empty());
		texturesNeeded = numImages / bestSize->numImages + numImages % bestSize->numImages;
		assert(texturesNeeded);
		// Create n texture atlases
		int currentImage = 0;
		int sheetNo = 0;
		atlas.reserve(texturesNeeded);
		vbo.reserve(texturesNeeded);
		texCoordBuffer.reserve(texturesNeeded);
		vertices.reserve(texturesNeeded);
		texCoords.reserve(texturesNeeded);
		// For each sprite sheet
		while (sheetNo < texturesNeeded)
		{
			int sheetWidth = bestSize->width * tileWidth;
			int sheetHeight = bestSize->height * tileWidth;
			std::unique_ptr<DrawableSurface> atlas = make_unique<DrawableSurface>(sheetWidth, sheetHeight);
			int x = 0, y = 0;
			// For each tile in the current sprite sheet
			for (int i = 0; i < bestSize->numImages; i++)
			{
				DrawableSurface* image = images[i + currentImage];
				atlas->drawSurface(x, y, image);
				TextureInfo info = { this, x, y, tileWidth, tileHeight, sheetNo };
				image->texMultX = 1.f;
				image->texMultY = 1.f;
				image->textureInfo = info;
				image->texture = atlas->texture;
				image->setRes(sheetWidth, sheetHeight);
				x += tileWidth;
				if (tileWidth + x < sheetWidth) {
					x = 0;
					y += tileHeight;
				}
			}
			// Finish drawing the sprite sheet.
			atlas->uploadToTexture();
			this->atlas.push_back(std::move(atlas));
			currentImage += bestSize->numImages;
			sheetNo++;
			std::vector<float> v, v2;
			vertices.push_back(v);
			texCoords.push_back(v2);
		}
		// Generate Opengl stuff
		for (int i = 0; i < texturesNeeded; i++)
		{
			unsigned int vbo;
			unsigned int texCoordBuffer;
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &texCoordBuffer);
			this->vbo.push_back(vbo);
			this->texCoordBuffer.push_back(texCoordBuffer);
		}
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
