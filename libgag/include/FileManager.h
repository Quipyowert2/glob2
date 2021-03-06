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

#ifndef __FILEMANAGER_H
#define __FILEMANAGER_H

#include "GAGSys.h"
#include <vector>
#include <fstream>
#include <string>

//! this is the host filesystem directory separator
#ifndef DIR_SEPARATOR
#define DIR_SEPARATOR '/'
#endif
#ifndef DIR_SEPARATOR_S
#define DIR_SEPARATOR_S "/"
#endif

namespace GAGCore
{
	class StreamBackend;
	
	//! File Manager (filesystem abstraction)
	class FileManager
	{
	public:
		//! Type of Stream
		enum StreamType
		{
			//! Binary stream, content directly written in endian-safe binary
			STREAM_BINARY = 0,
			//! Compatibility stream, not human-readable but uses the metas infos for backward compatibility
			STREAM_COMPAT,
			//! Text stream, human readable and backward compatible
			STREAM_TEXT,
		};
		
	private:
		//! List of directory where to search for requested file
		std::vector<std::string> dirList;
		//! List of file relative to virtual base address after call to initDirectoryListing
		std::vector<std::string> fileList;
		//! Index in the dirFileList vector
		int fileListIndex;
	
	private:
		//! clear the list of file for directory listing
		void clearFileList(void);
		//! internal function that does the real listing job
		bool addListingForDir(const std::string realDir, const std::string extension="", const bool dirs=false);
		//! open a file, if it is in writing, do a backup
		SDL_RWops *openWithbackup(const std::string filename, const std::string mode);
		//! open a file, if it is in writing, do a backup, fopen version
		FILE *openWithbackupFP(const std::string filename, const std::string mode);
		//! open a file, if it is in writing, do a backup, std::ofstream version
		std::ofstream *openWithbackupOFS(const std::string filename, std::ofstream::openmode mode);
	
	public:
		//! FileManager constructor
		FileManager(const std::string gameName);
		//! FileManager destructor
		virtual ~FileManager();
	
		//! Add a directory to the search list
		void addDir(const std::string dir);
		//! Return the number of directory in the search list
		unsigned getDirCount(void) const { return dirList.size(); }
		//! Return a direction in the search list from its index
		std::string getDir(unsigned index) const { if (index < getDirCount()) return dirList[index]; else return std::string(); }
		//! Add a new subdir (create it if needed) which will be used to open file in write mode in it
		void addWriteSubdir(const std::string subdir);
	
		//! Remove a file or a directory in the virtual filesystem, std::string version
		void remove(const std::string filename);
		//! Returns true if filename is a directory
		bool isDir(const std::string filename);
		
		//! Compress source to dest uzing gzip, returns true on success
		bool gzip(const std::string &source, const std::string &dest);
		//! Uncompress source to dest uzing gzip, returns true on success
		bool gunzip(const std::string &source, const std::string &dest);
	
		//! Open an output stream backend, use it to construct specific output streams
		StreamBackend *openOutputStreamBackend(const std::string filename);
		
		//! Open an input stream backend, use it to construct specific input streams
		StreamBackend *openInputStreamBackend(const std::string filename);
		
		//! Open a compressed output stream backend, use it to construct specific output streams
		StreamBackend *openCompressedOutputStreamBackend(const std::string filename);
		
		//! Open a compressed input stream backend, use it to construct specific input streams
		StreamBackend *openCompressedInputStreamBackend(const std::string filename);
		
		
		//! Open a file in the SDL_RWops format, COMPAT for GraphicContext PNG loader, can be removed on others backends
		SDL_RWops *open(const std::string filename, const std::string mode="rb");
		//! Open a file in the FILE* format
		FILE *openFP(const std::string filename, const std::string mode="rb");
		//! Open a file in the c++ stream format for reading
		std::ifstream *openIFStream(const std::string &fileName);
		//! Return the checksum of a file
		Uint32 checksum(const std::string filename);
		//! Return the modification date of a file
		time_t mtime(const std::string filename);
	
		// FIXME : the following functions are not thread-safe :
		//! must be call before directory listening, return true if success
		bool initDirectoryListing(const std::string virtualDir, const std::string extension="", const bool dirs=false);
		//! get the next name, return NULL if none
		const std::string getNextDirectoryEntry(void);
	};
}

#endif
