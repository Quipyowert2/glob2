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

#ifndef __GAGSYS_H
#define __GAGSYS_H

#ifndef MAX_SINT32
#define MAX_SINT32 0x7FFFFFFF
#endif

#ifdef WIN32

   #include <windows.h>
   #include <sys/types.h>
   #include <sys/stat.h>

   #define S_IFDIR _S_IFDIR

   #if defined(_MSC_VER) && _MSC_VER < 1900
	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#pragma warning (disable : 4786)
	#pragma warning (disable : 4250)
   #endif

   #undef max
   #undef min

	#define HAVE_OPENGL
	#define _USE_MATH_DEFINES // To get M_PI
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
// This is the only one which should be left... In theory :-)
// Remove this comment once all other SDL deps have been removed.
#include <SDL.h>

// usefull macros
#ifndef MAX
#define MAX(a, b) ((a)>(b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) ((a)<(b) ? (a) : (b))
#endif

#ifndef VARARRAY
#ifdef _MSC_VER
#include <malloc.h>
#define VARARRAY(t,n,s) t *n=(t*)_alloca((s)*sizeof(t))
#define strcasecmp _stricmp
#else
#define VARARRAY(t,n,s) t n[s]
#endif
#endif

#endif 
