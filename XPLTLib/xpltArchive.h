/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <stack>
#include <list>
#include <vector>
#include <zlib.h>
#include <MathLib/math3d.h>
#include <FSCore/memtool.h>
#include <FSCore/Archive.h>

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

//using namespace std;

//-----------------------------------------------------------------------------
// Input archive
class xpltArchive  
{
	struct CHUNK
	{
		unsigned int	id;		// chunk ID
		unsigned int	nsize;	// size of chunk
		void*			pdata;	// pointer to data
	};

public:
	enum IOResult { IO_ERROR, IO_OK, IO_END };

public:
	//! class constructor
	xpltArchive();

	//! destructor
	virtual ~xpltArchive();

public:
	// --- Writing ---

	// Open for writing
	bool Create(const char* szfile);

	// begin a chunk
	void BeginChunk(unsigned int id);

	// end a chunck
	void EndChunk();

	template <typename T> void WriteChunk(unsigned int nid, T& o)
	{
		m_pChunk->AddChild(new OLeaf<T>(nid, o));
	}

	void WriteChunk(unsigned int nid, const char* sz)
	{
		m_pChunk->AddChild(new OLeaf<const char*>(nid, sz));
	}

	template <typename T> void WriteChunk(unsigned int nid, T* po, int n)
	{
		m_pChunk->AddChild(new OLeaf<T*>(nid, po, n));
	}

	template <typename T> void WriteChunk(unsigned int nid, vector<T>& a)
	{
		m_pChunk->AddChild(new OLeaf<vector<T> >(nid, a));
	}

	// (overridden from Archive)
	virtual void WriteData(int nid, std::vector<float>& data)
	{
		WriteChunk(nid, data);
	}

public: // reading 

	// Close the archive
	void Close();

	void Flush();

	// Open for reading
	bool Open(IOFileStream* fp);

	// open for appending
	bool Append(const char* szfile);

	// Open a chunk
	int OpenChunk();

	// Get the current chunk ID
	unsigned int GetChunkID();

	// Close a chunk
	void CloseChunk();

	// input functions
	IOResult read(char&   c) { mread(&c, sizeof(char  ), 1, &m_pdata); return IO_OK; }
	IOResult read(int&    n) { mread(&n, sizeof(int   ), 1, &m_pdata); if (m_bswap) bswap(n); return IO_OK; }
	IOResult read(bool&   b) { mread(&b, sizeof(bool  ), 1, &m_pdata); return IO_OK; }
	IOResult read(float&  f) { mread(&f, sizeof(float ), 1, &m_pdata); if (m_bswap) bswap(f); return IO_OK; }
	IOResult read(double& g) { mread(&g, sizeof(double), 1, &m_pdata); if (m_bswap) bswap(g); return IO_OK; }

	IOResult read(unsigned int& n) { mread(&n, sizeof(unsigned int), 1, &m_pdata); if (m_bswap) bswap(n); return IO_OK; }


	IOResult read(char*   pc, int n) { mread(pc, sizeof(char  ), n, &m_pdata); return IO_OK; }
	IOResult read(int*    pi, int n) { mread(pi, sizeof(int   ), n, &m_pdata); if (m_bswap) bswapv(pi, n); return IO_OK; }
	IOResult read(bool*   pb, int n) { mread(pb, sizeof(bool  ), n, &m_pdata); return IO_OK; }
	IOResult read(float*  pf, int n) { mread(pf, sizeof(float ), n, &m_pdata); if (m_bswap) bswapv(pf, n); return IO_OK; }
	IOResult read(double* pg, int n) { mread(pg, sizeof(double), n, &m_pdata); if (m_bswap) bswapv(pg, n); return IO_OK; }

	IOResult read(char* sz)
	{
		IOResult ret;
		int l;
		ret = read(l); if (ret != IO_OK) return ret;
		mread(sz, 1, l, &m_pdata);
		sz[l] = 0;
		return IO_OK;
	}

	IOResult read(vec3f&   a) { return read(&(a.x), 3); }
	IOResult read(mat3fs&  a) { return read(&(a.x), 6); }
	IOResult read(mat3fd&  a) { return read(&(a.x), 3); }
	IOResult read(tens4fs& a) { return read(&(a.d[0]), 21); }
	IOResult read(mat3f&   a) { return read(&(a.m_data[0][0]), 9); }

	IOResult read(vector<int    >& a) { return read(&a[0], (int) a.size()); }
	IOResult read(vector<float  >& a) { return read(&a[0], (int) a.size()); }
	IOResult read(vector<vec3f  >& a) { return read(&(a[0].x), 3*(int) a.size()); }
	IOResult read(vector<mat3fs >& a) { return read(&(a[0].x), 6*(int) a.size()); }
	IOResult read(vector<mat3fd >& a) { return read(&(a[0].x), 3*(int) a.size()); }
	IOResult read(vector<tens4fs>& a) { return read(&(a[0].d[0]), 21*(int) a.size()); }
	IOResult read(vector<mat3f  >& a) { return read(&(a[0].m_data[0][0]), 9*(int) a.size()); }
	IOResult read(vector<unsigned int>& a) { return read((int*)&a[0], (int)a.size()); }

	// conversion to FILE* 
//	operator FILE* () { return m_fp; }

	void SetVersion(unsigned int n) { m_nversion = n; }
	unsigned int Version() { return m_nversion; }

	// set/get compression method
	int GetCompression() { return m_ncompress; }
	void SetCompression(int n) { m_ncompress = n; }

	int DecompressChunk(unsigned int& nid, unsigned int& nsize);

protected:
	IOFileStream*	m_fp;		// the file pointer
	bool	m_bswap;		// swap data when reading
	bool	m_bend;			// chunk end flag
	int		m_ncompress;	// compression flag
	bool	m_bSaving;		// read or write mode?

	unsigned int	m_nversion;	// stores the version nr of the file being loaded

	// read data
	stack<CHUNK*>	m_Chunk;

    z_stream		strm;
	char*			m_buf;		// data buffer
	void*			m_pdata;	// data pointer
	unsigned int	m_bufsize;	// size of data buffer

	// write data
	OBranch*	m_pRoot;	// chunk tree root
	OBranch*	m_pChunk;	// current chunk
};
