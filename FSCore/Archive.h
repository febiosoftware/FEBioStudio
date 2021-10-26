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
#include <MathLib/math3d.h>
#include <FECore/mat3d.h>
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

//-----------------------------------------------------------------------------
//! helper class for writing buffered data to file
class IOFileStream
{
public:
	IOFileStream(FILE* fp = nullptr, bool owner = true);
	~IOFileStream();

	bool Create(const char* szfile);
	bool Open(const char* szfile);
	bool Append(const char* szfile);
	void Close();

	void Write(void* pd, size_t Size, size_t Count);

	void Flush();

	// \todo temporary reading functions. Needs to be replaced with buffered functions
	size_t read(void* pd, size_t Size, size_t Count);
	long tell();
	void seek(long noff, int norigin);

	void BeginStreaming();
	void EndStreaming();

	void SetCompression(int n) { m_ncompress = n; }

	FILE* FilePtr() { return m_fp; }

	bool IsValid() { return (m_fp != nullptr); }

private:
	FILE*	m_fp;
	bool	m_fileOwner;
	size_t	m_bufsize;		//!< buffer size
	size_t	m_current;		//!< current index
	unsigned char*	m_buf;	//!< buffer
	unsigned char*	m_pout;	//!< temp buffer when writing
	int		m_ncompress;	//!< compression level
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

class OBranch;

class OChunk
{
public:
	OChunk(unsigned int nid) { m_nID = nid; m_pParent = 0; }
	virtual ~OChunk() {}

	unsigned int GetID() { return m_nID; }

	virtual void Write(IOFileStream* fp) = 0;
	virtual int Size() = 0;

	void SetParent(OBranch* pparent) { m_pParent = pparent; }
	OBranch* GetParent() { return m_pParent; }

protected:
	int			m_nID;
	OBranch*	m_pParent;
};

class OBranch : public OChunk
{
public:
	OBranch(unsigned int nid) : OChunk(nid) {}
	~OBranch()
	{
		list<OChunk*>::iterator pc;
		for (pc = m_child.begin(); pc != m_child.end(); ++pc) delete (*pc);
		m_child.clear();
	}

	int Size()
	{
		int nsize = 0;
		list<OChunk*>::iterator pc;
		for (pc = m_child.begin(); pc != m_child.end(); ++pc) nsize += (*pc)->Size() + 2*sizeof(unsigned int);
		return nsize;
	}

	void Write(IOFileStream* fp)
	{
		fp->Write(&m_nID, sizeof(unsigned int), 1);

		unsigned int nsize = Size();
		fp->Write(&nsize, sizeof(unsigned int), 1);

		list<OChunk*>::iterator pc;
		for (pc = m_child.begin(); pc != m_child.end(); ++pc) (*pc)->Write(fp);
	}

	void AddChild(OChunk* pc) { m_child.push_back(pc); pc->SetParent(this); }

protected:
	list<OChunk*>	m_child;
};

template <typename T>
class OLeaf : public OChunk
{
public:
	OLeaf(unsigned int nid, const T& d) : OChunk(nid) { m_d = d; }

	int Size() { return sizeof(T); }

	void Write(IOFileStream* fp)
	{
		fp->Write(&m_nID  , sizeof(unsigned int), 1);
		unsigned int nsize = sizeof(T);
		fp->Write(&nsize, sizeof(unsigned int), 1);
		fp->Write(&m_d, sizeof(T), 1);
	}

protected:
	T	m_d;
};

template <typename T>
class OLeaf<T*> : public OChunk
{
public:
	OLeaf(unsigned int nid, const T* pd, int nsize) : OChunk(nid)
	{
		assert(nsize > 0);
		m_pd = new T[nsize];
		memcpy(m_pd, pd, sizeof(T)*nsize);
		m_nsize = nsize;
	}
	~OLeaf() { delete m_pd; }

	int Size() { return sizeof(T)*m_nsize; }
	void Write(IOFileStream* fp)
	{
		fp->Write(&m_nID, sizeof(unsigned int), 1);
		unsigned int nsize = Size();
		fp->Write(&nsize, sizeof(unsigned int), 1);
		fp->Write(m_pd, sizeof(T), m_nsize);
	}

protected:
	T*		m_pd;
	int		m_nsize;
};

template <>
class OLeaf<const char*> : public OChunk
{
public:
	OLeaf(unsigned int nid, const char* sz) : OChunk(nid)
	{
		int l = (int)strlen(sz);
		m_psz = new char[l+1];
		memcpy(m_psz, sz, l+1);
	}
	~OLeaf() { delete m_psz; }

	int Size() { return (int)strlen(m_psz) + sizeof(int); }
	void Write(IOFileStream* fp)
	{
		fp->Write(&m_nID, sizeof(unsigned int), 1);
		unsigned int nsize = Size();
		fp->Write(&nsize, sizeof(unsigned int), 1);
		int l = nsize - sizeof(int);
		fp->Write(&l, sizeof(int), 1);
		fp->Write(m_psz, sizeof(char), l);
	}

protected:
	char*	m_psz;
};

template <typename T>
class OLeaf<vector<T> > : public OChunk
{
public:
	OLeaf(unsigned int nid, const vector<T>& a) : OChunk(nid)
	{
		m_nsize = (int)a.size();
		assert(m_nsize > 0);
		m_pd = new T[m_nsize];
		memcpy(m_pd, &a[0], sizeof(T)*m_nsize);
	}
	~OLeaf() { delete m_pd; }

	int Size() { return sizeof(T)*m_nsize; }
	void Write(IOFileStream* fp)
	{
		fp->Write(&m_nID, sizeof(unsigned int), 1);
		unsigned int nsize = Size();
		fp->Write(&nsize, sizeof(unsigned int), 1);
		fp->Write(m_pd, sizeof(T), m_nsize);
	}

protected:
	T*		m_pd;
	int		m_nsize;
};

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
	IOFileStream	m_fp;		// the file pointer

	OBranch*	m_pRoot;	// chunk tree root
	OBranch*	m_pChunk;	// current chunk
};
