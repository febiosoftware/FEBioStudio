#pragma once
#include <stdio.h>
#include <string.h>
#include <stack>
#include <list>
#include <vector>
#include <zlib.h>
#include "math3d.h"

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

using namespace std;

namespace Post {

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

//-----------------------------------------------------------------------------
// helper function for reading from a memory buffer
void mread(void* pdest, size_t Size, size_t Cnt, void** psrc);

//-----------------------------------------------------------------------------
class MemBuffer
{
public:
	MemBuffer();
	~MemBuffer();

	void append(void* pd, int n);

	char* data() { return m_pbuf; }

	int size() { return m_nsize; }

private:
	char*	m_pbuf;		// buffer
	int		m_nsize;	// size of buffer
	int		m_nalloc;	// actual amount of allocated data
};

//-----------------------------------------------------------------------------
// Input archive
class IArchive  
{
	struct CHUNK
	{
		unsigned int	id;		// chunk ID
		unsigned int	nsize;	// size of chunk
		void*			pdata;	// pointer to data
	};

public:
	//! class constructor
	IArchive();

	//! destructor
	virtual ~IArchive();

	// Close the archive
	void Close();

	// Open for reading
	bool Open(FILE* fp);

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
	FILE*	m_fp;			// the file pointer
	bool	m_bswap;		// swap data when reading
	bool	m_bend;			// chunk end flag
	int		m_ncompress;	// compression flag

	unsigned int	m_nversion;	// stores the version nr of the file being loaded

	stack<CHUNK*>	m_Chunk;

    z_stream		strm;
	char*			m_buf;		// data buffer
	void*			m_pdata;	// data pointer
	unsigned int	m_bufsize;	// size of data buffer
};
}
