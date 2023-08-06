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

#include "FileReader.h"
#include <stdarg.h>
//using namespace std;

#ifdef WIN32
#define ftell64(a)     _ftelli64(a)
#define fseek64(a,b,c) _fseeki64(a,b,c)
#endif

#ifdef LINUX // same for Linux and Mac OS X
#define ftell64(a)     ftello(a)
#define fseek64(a,b,c) fseeko(a,b,c)
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
#define ftell64(a)     ftello(a)
#define fseek64(a,b,c) fseeko(a,b,c)
#endif

FileReader::FileReader()
{
	m_fp = 0;
	m_stream = nullptr;
	m_nfilesize = 0;
}

FileReader::~FileReader()
{
	Close();
}

void FileReader::Cancel()
{
	Terminate();
}

bool FileReader::Open(const char* szfile, const char* szmode)
{
	Reset();

	if (m_fp) Close();

	if ((szfile == 0) || (szfile[0] == 0)) return false;

	m_fp = fopen(szfile, szmode);
	if (m_fp)
	{
		m_fileName = szfile;

		// get the filesize
		fseek(m_fp, 0, SEEK_END);
		m_nfilesize = ftell64(m_fp);
		fseek64(m_fp, 0, SEEK_SET);
	}

	return (m_fp != 0);
}

bool FileReader::SetFileStream(ifstream* stream)
{
    m_stream = stream;

	if (m_stream)
	{
		std::streampos pos = m_stream->tellg();
		m_stream->seekg(0, std::ios_base::end);
		m_nfilesize = m_stream->tellg();
		m_stream->seekg(pos, std::ios_base::beg);
	}

	return true;
}

void FileReader::Close()
{
	if (m_fp) 
	{
		FILE* fp = m_fp;
		m_fp = 0;	// we set this to zero first, in order to avoid a race condition
		fclose(fp);
	}
	m_stream = nullptr;
	m_nfilesize = 0;
}

FILE* FileReader::FilePtr()
{
	return m_fp;
}

float FileReader::GetFileProgress() const
{
	if (m_fp)
	{
		off_type npos = ftell64(m_fp);
		float pct = (float)npos / (float)m_nfilesize;
		return pct;
	}
    else if(m_stream)
    {
        std::streampos npos = m_stream->tellg();
        return (float)npos / (float)m_nfilesize;
    }
	else return 1.0f;
}

void FileReader::FileTitle(char *sz)
{
	const char* szfile = m_fileName.c_str();
	const char *c1 = strrchr(szfile, '/');
	if (c1 == 0)
	{	
		c1 = strrchr(szfile, '\\'); 
		if (c1 == 0) c1 = szfile; else ++c1;
	}
	else ++c1;

	const char* c2 = strrchr(c1, '.');

	int l = strlen(c1);
	if (c2) l -= strlen(c2);

	strncpy(sz, c1, l);
	sz[l] = 0;
}

void FileReader::SetFileName(const std::string& fileName)
{
	m_fileName = fileName;
}

// get the file name
std::string FileReader::GetFileName() const
{
	return m_fileName;
}

