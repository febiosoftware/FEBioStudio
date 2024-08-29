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
#include <FSCore/math3d.h>
#include <FEBioPlot/PltArchive.h>
#include "color.h"
#include "CallTracer.h"
#include <stack>
#include <list>
#include <string>
#include "memtool.h"
//using namespace std;

using std::string;
using std::stack;
using std::list;
using std::pair;

//-----------------------------------------------------------------------------
// Used for reading archives
class IOMemBuffer
{
public:
	IOMemBuffer();
	~IOMemBuffer();

	void append(void* pd, int n);

	char* data() { return m_pbuf; }

	int size() { return m_nsize; }

private:
	char*	m_pbuf;		// buffer
	int		m_nsize;	// size of buffer
	int		m_nalloc;	// actual amount of allocated data
};

//----------------------
// Input archive

class IArchive
{
	struct CHUNK
	{
		unsigned int	id;		// chunk ID
		long			lpos;	// file position of size field
		unsigned int	nsize;	// size of chunk
	};

public:
	enum IOResult { IO_ERROR, IO_OK, IO_END };

public:
	//! class constructor
	IArchive();

	//! destructor
	virtual ~IArchive();

	// Close the archive
	void Close();

	// open for reading
	bool Open(const char* szfile, unsigned int signature);
	bool Open(FILE* file, unsigned int signature);

	// see if there is a valid file pointer
	bool IsValid() const;

	// Open a chunk
	int OpenChunk();

	// Get the current chunk ID
	unsigned int GetChunkID();

	// Close a chunk
	virtual void CloseChunk();

	// input functions
	IOResult read(char&   c) { int nr = (int) fread(&c, sizeof(char  ), 1, m_fp); if (nr != 1) return IO_ERROR; return IO_OK; }
	IOResult read(int&    n) { int nr = (int) fread(&n, sizeof(int   ), 1, m_fp); if (nr != 1) return IO_ERROR; if (m_bswap) bswap(n); return IO_OK; }
	IOResult read(bool&   b) { int nr = (int) fread(&b, sizeof(bool  ), 1, m_fp); if (nr != 1) return IO_ERROR; return IO_OK; }
	IOResult read(float&  f) { int nr = (int) fread(&f, sizeof(float ), 1, m_fp); if (nr != 1) return IO_ERROR; if (m_bswap) bswap(f); return IO_OK; }
	IOResult read(double& g) { int nr = (int) fread(&g, sizeof(double), 1, m_fp); if (nr != 1) return IO_ERROR; if (m_bswap) bswap(g); return IO_OK; }

	IOResult read(unsigned int& n) { size_t nr = fread(&n, sizeof(unsigned int), 1, m_fp); if (nr != 1) return IO_ERROR; if (m_bswap) bswap(n); return IO_OK; }


	IOResult read(int*    pi, int n) { int nr = (int) fread(pi, sizeof(int   ), n, m_fp); if (nr != n) return IO_ERROR; if (m_bswap) bswapv(pi, n); return IO_OK; }
	IOResult read(bool*   pb, int n) { int nr = (int) fread(pb, sizeof(bool  ), n, m_fp); if (nr != n) return IO_ERROR; return IO_OK; }
	IOResult read(float*  pf, int n) { int nr = (int) fread(pf, sizeof(float ), n, m_fp); if (nr != n) return IO_ERROR; if (m_bswap) bswapv(pf, n); return IO_OK; }
	IOResult read(double* pg, int n) { int nr = (int) fread(pg, sizeof(double), n, m_fp); if (nr != n) return IO_ERROR; if (m_bswap) bswapv(pg, n); return IO_OK; }
	IOResult read(vec3d*  pv, int n) { for (int i=0; i<n; ++i) read(pv[i]); return IO_OK; }

	IOResult read(vec3d& r) { read(r.x); read(r.y); read(r.z); return IO_OK; }
	IOResult read(vec2i& r) { read(r.x); read(r.y); return IO_OK; }
	IOResult read(vec2d& r) { read(r.x()); read(r.y()); return IO_OK; }
	IOResult read(quatd& q) { read(q.x); read(q.y); read(q.z); read(q.w); return IO_OK; }
	IOResult read(GLColor& c) { int nr = (int) fread(&c, sizeof(GLColor), 1, m_fp); if (nr != 1) return IO_ERROR; return IO_OK; }

	IOResult read(mat3d& a) 
	{ 
		double d[9];
		read(d, 9);
		a = mat3d(d);
		return IO_OK; 
	}

	IOResult read(mat3ds& a)
	{
		double d[6];
		read(d, 6);
		a = mat3ds(d[0], d[1], d[2], d[3], d[4], d[5]);
		return IO_OK;
	}

	IOResult read(char* sz)
	{
		IOResult ret;
		int l, nr;
		ret = read(l); if (ret != IO_OK) return ret;
		nr = (int) fread(sz, 1, l, m_fp); if (nr != l) return IO_ERROR;
		sz[l] = 0;
		return IO_OK;
	}

	IOResult read(std::string& s)
	{
		int l = 0;
		IOResult ret = read(l); if (ret != IO_OK) return ret;

		if (l > 0)
		{
			char* tmp = new char[l+1];
			int nr = (int) fread(tmp, 1, l, m_fp); if (nr != l) return IO_ERROR;
			tmp[l] = 0;
			s = tmp;
			delete [] tmp;
		}
		else s.clear();
		return IO_OK;
	}

	IOResult read(std::vector<int>& v);
	IOResult read(std::vector<double>& v);
	IOResult read(std::vector<vec2d>& v);

	template <class T> IOResult read(std::vector<T>& v)
	{
		CHUNK* pc = m_Chunk.top();
		int nsize = pc->nsize / sizeof(T);
		v.resize(nsize);
		int nread = (int)fread(&v[0], sizeof(T), nsize, m_fp);
		if (nread != nsize) return IO_ERROR;
		return IO_OK;
	}

	// conversion to FILE* 
	operator FILE* () { return m_fp; }

	void SetVersion(unsigned int n) { m_nversion = n; }
	unsigned int Version() { return m_nversion; }

	void log(const char* sztxt, ...);
	std::string GetLog() const;

private:
	bool Load(const char* szfile) { return false; }

protected:
	bool	m_bswap;	// swap data when reading
	bool	m_bend;		// chunk end flag
	bool	m_delfp;	// delete fp pointer

	unsigned int	m_nversion;	// stores the version nr of the file being loaded

	stack<CHUNK*>	m_Chunk;

	FILE*	m_fp;		// the file pointer

protected:
	std::string		m_log;
};

//----------------------
// Output archive
class OArchive  
{
public:
	OArchive();
	virtual ~OArchive();

	// Close archive
	void Close();

	// Open for writing
	bool Create(const char* szfile, unsigned int signature);

	// begin a chunk
	void BeginChunk(unsigned int id);

	// end a chunck
	void EndChunk();

	void WriteChunk(unsigned int nid, char* sz)
	{
		m_pChunk->AddChild(new OLeaf<const char*>(nid, sz));
	}

	void WriteChunk(unsigned int nid, const char* sz)
	{
		m_pChunk->AddChild(new OLeaf<const char*>(nid, sz));
	}

	void WriteChunk(unsigned int nid, const string& s)
	{
		m_pChunk->AddChild(new OLeaf<const char*>(nid, s.c_str()));
	}

	template <typename T> void WriteChunk(unsigned int nid, T* po, int n)
	{
		m_pChunk->AddChild(new OLeaf<T*>(nid, po, n));
	}

	template <typename T> void WriteChunk(unsigned int nid, const T& o)
	{
		m_pChunk->AddChild(new OLeaf<T>(nid, o));
	}

protected:
	FileStream	m_fp;		// the file pointer

	OBranch*	m_pRoot;	// chunk tree root
	OBranch*	m_pChunk;	// current chunk
};
