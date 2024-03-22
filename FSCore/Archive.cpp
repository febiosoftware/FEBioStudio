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

#include "stdafx.h"
#include "Archive.h"
#include <sstream>
#include <stdarg.h>

using std::stringstream;

//=============================================================================
IOMemBuffer::IOMemBuffer()
{
	m_pbuf = 0;
	m_nsize = 0;
	m_nalloc = 0;
}

IOMemBuffer::~IOMemBuffer()
{
	delete[] m_pbuf;
	m_nsize = 0;
	m_nalloc = 0;
}

void IOMemBuffer::append(void* pd, int n)
{
	// make sure we need to do anything
	if ((n <= 0) || (pd == 0)) return;

	if (m_pbuf == 0)
	{
		m_pbuf = new char[n];
		m_nalloc = n;
	}
	else if (m_nsize + n > m_nalloc)
	{
		m_nalloc = 4 * ((3 * m_nalloc / 2) / 4);
		if (m_nalloc < 4) m_nalloc = 4;
		if (m_nalloc < m_nsize + n) m_nalloc = m_nsize + n;
		char* ptmp = new char[m_nalloc];
		if (m_nsize > 0) memcpy(ptmp, m_pbuf, m_nsize);
		delete[] m_pbuf;
		m_pbuf = ptmp;
	}

	// copy the data
	memcpy(m_pbuf + m_nsize, pd, n);

	// increase size
	m_nsize += n;
}

//////////////////////////////////////////////////////////////////////
// IArchive
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
IArchive::IArchive()
{
	m_bend = true;
	m_bswap = false;
	m_delfp = false;
	m_nversion = 0;
	m_fp = 0;
}

//-----------------------------------------------------------------------------
IArchive::~IArchive()
{
	Close();
}

//-----------------------------------------------------------------------------
// see if there is a valid file pointer
bool IArchive::IsValid() const
{
	return (m_fp != 0);
}

//-----------------------------------------------------------------------------
void IArchive::Close()
{
	while (m_Chunk.empty() == false) CloseChunk();

	// reset pointers
	if (m_delfp) fclose(m_fp);
	m_fp = 0;
	m_bend = true;
	m_bswap = false;
	m_delfp = false;
}

//-----------------------------------------------------------------------------
bool IArchive::Open(const char* szfile, unsigned int signature)
{
	FILE* fp = fopen(szfile, "rb");
	if (fp == 0) return false;
	m_delfp = true;
	return Open(fp, signature, szfile);
}

//-----------------------------------------------------------------------------
bool IArchive::Open(FILE* fp, unsigned int signature, const char* szfile)
{
    m_filename = szfile;

	m_log.clear();

	// make sure the file pointer is valid
	if (fp == 0) return false;

	// store the file pointer
	m_fp = fp;

	// read the master tag
	unsigned int ntag;
	if (fread(&ntag, sizeof(int), 1, m_fp) != 1) 
	{
		Close();
		return false;
	}

	unsigned byteSwappedSignature = signature;
	bswap(byteSwappedSignature);

	// see if the file needs to be byteswapped
	if (ntag == signature) m_bswap = false;
	else if (ntag == byteSwappedSignature) m_bswap = true;
	else
	{
		// unknown file format
		Close();
		return false;
	}

	// open the master chunk
	m_bend = false;
	int nret = OpenChunk();
	if (nret != IO_OK) return false;

	return true;
}

int IArchive::OpenChunk()
{
	// see if the end flag was set
	// in that case we first need to clear the flag
	if (m_bend)
	{
		m_bend = false;
		return IO_END;
	}

	// create a new chunk
	CHUNK* pc = new CHUNK;

	// read the chunk ID
	read(pc->id);

	// read the chunk size
	read(pc->nsize);

	if (pc->nsize == 0) m_bend = true;

	// record the position
	pc->lpos = ftell(m_fp);

	// add it to the stack
	m_Chunk.push(pc);

	return IO_OK;
}

void IArchive::CloseChunk()
{
	// pop the last chunk
	CHUNK* pc = m_Chunk.top(); m_Chunk.pop();

	// get the current file position
	long lpos = ftell(m_fp);

	// calculate the offset to the end of the chunk
	int noff = pc->nsize - (lpos - pc->lpos);

	// skip any remaining part in the chunk
	// I wonder if this can really happen
	if (noff != 0)
	{
		fseek(m_fp, noff, SEEK_CUR);
		lpos = ftell(m_fp);
	}

	// delete this chunk
	delete pc;

	// take a peek at the parent
	if (m_Chunk.empty())
	{
		// we just deleted the master chunk
		m_bend = true;
	}
	else
	{
		pc = m_Chunk.top();
		int noff = pc->nsize - (lpos - pc->lpos);
		if (noff == 0) m_bend = true;
	}
}

unsigned int IArchive::GetChunkID()
{
	CHUNK* pc = m_Chunk.top();
	assert(pc);
	return pc->id;
}

IArchive::IOResult IArchive::read(std::vector<int>& v)
{
	CHUNK* pc = m_Chunk.top();

	int nsize = pc->nsize / sizeof(int);
	v.resize(nsize);
	if (nsize > 0)
	{
		int nread = (int)fread(&v[0], sizeof(int), nsize, m_fp);
		if (nread != nsize) return IO_ERROR;
	}
	return IO_OK;
}

IArchive::IOResult IArchive::read(std::vector<double>& v)
{
	CHUNK* pc = m_Chunk.top();

	int nsize = pc->nsize / sizeof(double);
	if (nsize > 0)
	{
		v.resize(nsize);
		int nread = (int)fread(&v[0], sizeof(double), nsize, m_fp);
		if (nread != nsize) return IO_ERROR;
	}
	else v.clear();

	return IO_OK;
}

IArchive::IOResult IArchive::read(std::vector<vec2d>& v)
{
	CHUNK* pc = m_Chunk.top();

	int nsize = pc->nsize / sizeof(vec2d);
	v.resize(nsize);
	int nread = (int)fread(&v[0], sizeof(vec2d), nsize, m_fp);
	if (nread != nsize) return IO_ERROR;
	return IO_OK;
}

void IArchive::log(const char* sz, ...)
{
	if (sz == 0) return;

	// get a pointer to the argument list
	va_list	args;
	va_start(args, sz);

	// count how many chars we need to allocate
	va_list argscopy;
	va_copy(argscopy, args);
	char* szlog = NULL;
	int l = vsnprintf(nullptr, 0, sz, argscopy) + 1;
	va_end(argscopy);
	if (l > 1)
	{
		szlog = new char[l]; assert(szlog);
		if (szlog)
		{
			vsnprintf(szlog, l, sz, args);
		}
	}
	va_end(args);
	if (szlog == NULL) return;

	l = (int)strlen(szlog);
	if (l == 0) return;

	stringstream ss;
	if (m_log.empty() == false) ss << "\n";
	ss << szlog;
	m_log += ss.str();
}

std::string IArchive::GetLog() const
{
	return m_log;
}

std::string IArchive::GetFilename() const
{
    return m_filename;
}

//////////////////////////////////////////////////////////////////////
// OArchive
//////////////////////////////////////////////////////////////////////

OArchive::OArchive()
{
	m_pRoot = 0;
	m_pChunk = 0;
}

OArchive::~OArchive()
{
	Close();
}

void OArchive::Close()
{
	if (m_fp.IsValid())
	{
		m_pRoot->Write(&m_fp);
		m_fp.Close();
	}

	delete m_pRoot;
	m_pRoot = 0;
	m_pChunk = 0;
}

bool OArchive::Create(const char* szfile, unsigned int signature)
{
    m_filename = szfile;

	// attempt to create the file
	if (m_fp.Create(szfile) == false) return false;

	// write the master tag 
	m_fp.Write(&signature, sizeof(int), 1);

	assert(m_pRoot == 0);
	m_pRoot = new OBranch(0);
	m_pChunk = m_pRoot;

	return true;
}

void OArchive::BeginChunk(unsigned int id)
{
	// create a new branch
	OBranch* pbranch = new OBranch(id);

	// attach it to the current branch
	m_pChunk->AddChild(pbranch);

	// move the current branch pointer
	m_pChunk = pbranch;
}

void OArchive::EndChunk()
{
	m_pChunk = m_pChunk->GetParent();
}

std::string OArchive::GetFilename() const
{
    return m_filename;
}