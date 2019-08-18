// Archive.h: interface for the Archive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVE_H__B95A81B1_BBFB_46E5_B9B3_7675ED8A6029__INCLUDED_)
#define AFX_ARCHIVE_H__B95A81B1_BBFB_46E5_B9B3_7675ED8A6029__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <stdio.h>
#include <string.h>
#include <MathLib/math3d.h>
#include "CallTracer.h"
#include <stack>
#include <list>
#include <string>
using namespace std;

void inline bswap(short& s)
{
	unsigned char* c = (unsigned char*)(&s);
	c[0] ^= c[1]; c[1] ^= c[0]; c[0] ^= c[1];
}

void inline bswap(int& n)
{
	unsigned char* c = (unsigned char*)(&n);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(unsigned int& n)
{
	unsigned char* c = (unsigned char*)(&n);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(float& f)
{
	unsigned char* c = (unsigned char*)(&f);
	c[0] ^= c[3]; c[3] ^= c[0]; c[0] ^= c[3];
	c[1] ^= c[2]; c[2] ^= c[1]; c[1] ^= c[2];
}

void inline bswap(double& g)
{
	unsigned char* c = (unsigned char*)(&g);
	c[0] ^= c[7]; c[7] ^= c[0]; c[0] ^= c[7];
	c[1] ^= c[6]; c[6] ^= c[1]; c[1] ^= c[6];
	c[2] ^= c[5]; c[5] ^= c[2]; c[2] ^= c[5];
	c[3] ^= c[4]; c[4] ^= c[3]; c[3] ^= c[4];
}

template <typename T> void bswapv(T* pd, int n)
{
	for (int i=0; i<n; ++i) bswap(pd[i]);
}

enum IOResult { IO_ERROR, IO_OK, IO_END };

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
	IOResult read(quatd& q) { read(q.x); read(q.y); read(q.z); read(q.w); return IO_OK; }
	IOResult read(GLCOLOR& c) { int nr = (int) fread(&c, sizeof(GLCOLOR), 1, m_fp); if (nr != 1) return IO_ERROR; return IO_OK; }

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

	// conversion to FILE* 
	operator FILE* () { return m_fp; }

	void SetVersion(unsigned int n) { m_nversion = n; }
	unsigned int Version() { return m_nversion; }

private:
	bool Load(const char* szfile) { return false; }

protected:
	bool	m_bswap;	// swap data when reading
	bool	m_bend;		// chunk end flag
	bool	m_delfp;	// delete fp pointer

	unsigned int	m_nversion;	// stores the version nr of the file being loaded

	stack<CHUNK*>	m_Chunk;

	FILE*	m_fp;		// the file pointer
};

//----------------------
// Output archive

class OBranch;

class OChunk
{
public:
	OChunk(unsigned int nid) { m_nID = nid; m_pParent = 0; }
	virtual ~OChunk(){}

	unsigned int GetID() { return m_nID; }

	virtual void Write(FILE* fp) = 0;
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

	void Write(FILE* fp)
	{
		fwrite(&m_nID  , sizeof(unsigned int), 1, fp);

		unsigned int nsize = Size();
		fwrite(&nsize, sizeof(unsigned int), 1, fp);

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

	void Write(FILE* fp)
	{
		fwrite(&m_nID  , sizeof(unsigned int), 1, fp);
		unsigned int nsize = sizeof(T);
		fwrite(&nsize, sizeof(unsigned int), 1, fp);
		fwrite(&m_d, sizeof(T), 1, fp);
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
	void Write(FILE* fp)
	{
		fwrite(&m_nID , sizeof(unsigned int), 1, fp);
		unsigned int nsize = Size();
		fwrite(&nsize , sizeof(unsigned int), 1, fp);
		fwrite(m_pd   , sizeof(T), m_nsize, fp);
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
	void Write(FILE* fp)
	{
		fwrite(&m_nID , sizeof(unsigned int), 1, fp);
		unsigned int nsize = Size();
		fwrite(&nsize , sizeof(unsigned int), 1, fp);
		int l = nsize - sizeof(int);
		fwrite(&l, sizeof(int), 1, fp);
		fwrite(m_psz, sizeof(char), l, fp);
	}

protected:
	char*	m_psz;
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
	FILE*	m_fp;		// the file pointer

	OBranch*	m_pRoot;	// chunk tree root
	OBranch*	m_pChunk;	// current chunk
};

#endif // !defined(AFX_ARCHIVE_H__B95A81B1_BBFB_46E5_B9B3_7675ED8A6029__INCLUDED_)
