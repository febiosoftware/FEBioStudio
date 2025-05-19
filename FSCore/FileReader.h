/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include "FSThreadedTask.h"

using std::string;
using std::ifstream;

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

//-----------------------------------------------------------------------------
class FileReader : public FSThreadedTask
{
public:
	FileReader();
	virtual ~FileReader();

	// This function must be overloaded in derived classes
	virtual bool Load(const char* szfile) = 0;

	// Cancel the file read
	void Cancel();

	// get the amount of the file read so far
	// expressed in percentage of total file size
	virtual float GetFileProgress() const;

	// get the file title, i.e. the file name w/o the path
	void FileTitle(char* sz);

	// set the file name
	void SetFileName(const std::string& fileName);

	// get the file name
	std::string GetFileName() const;

protected:
	// open the file
	bool Open(const char* szfile, const char* szmode);

    // Set file stream
    bool SetFileStream(ifstream* stream);

	// close the file
	virtual void Close();

	// get the file pointer
	FILE* FilePtr();

protected:
	FILE*			m_fp;
    ifstream*       m_stream;

private:
	std::string		m_fileName;	//!< file name
	off_type		m_nfilesize;	// size of file
};

// helper function to compare strings
inline int szcmp(const char* sz1, const char* sz2)
{
	return strncmp(sz1, sz2, strlen(sz2));
}

// helper function for getting the file path from a file name
std::string getFilePath(const std::string& file);
