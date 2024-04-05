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

#include "xpltArchive.h"
#include <assert.h>
#include <FSCore/Archive.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

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

//////////////////////////////////////////////////////////////////////
// xpltArchive
//////////////////////////////////////////////////////////////////////

class xpltArchive::Imp 
{
public:
	FileStream* m_fp;		// the file pointer
	bool	m_bswap;		// swap data when reading
	bool	m_bend;			// chunk end flag
	int		m_ncompress;	// compression flag
	bool	m_bSaving;		// read or write mode?

	unsigned int	m_nversion;	// stores the version nr of the file being loaded

	// read data
	stack<CHUNK*>	m_Chunk;

#ifdef HAVE_ZLIB
	z_stream		strm;
#endif
	char* m_buf;		// data buffer
	void* m_pdata;	// data pointer
	unsigned int	m_bufsize;	// size of data buffer

	// write data
	OBranch* m_pRoot;	// chunk tree root
	OBranch* m_pChunk;	// current chunk

	Imp()
	{
		m_fp = 0;
		m_bend = true;
		m_bswap = false;
		m_nversion = 0;
		m_buf = 0;
		m_pdata = 0;
		m_bufsize = 0;
		m_ncompress = 0;
		m_pRoot = 0;
		m_pChunk = 0;
		m_bSaving = true;
	}
};

xpltArchive::xpltArchive() : im(*new xpltArchive::Imp)
{
}

xpltArchive::~xpltArchive()
{
	Close();
}

void xpltArchive::AddChild(OChunk* c)
{
	im.m_pChunk->AddChild(c);
}

void xpltArchive::SetVersion(unsigned int n) { im.m_nversion = n; }
unsigned int xpltArchive::Version() { return im.m_nversion; }

// set/get compression method
int xpltArchive::GetCompression() { return im.m_ncompress; }
void xpltArchive::SetCompression(int n) { im.m_ncompress = n; }

void xpltArchive::Close()
{
	if (im.m_bSaving)
	{
		if (im.m_pRoot) Flush();
	}
	else {
		// clear the stack
		while (im.m_Chunk.empty() == false)
		{
			// pop the last chunk
			CHUNK* pc = im.m_Chunk.top(); im.m_Chunk.pop();
			delete pc;
		}
	}

	// close the file pointer
	im.m_fp = 0;

	// delete the buffer
	if (im.m_buf) delete[] im.m_buf;
	im.m_buf = 0;
	im.m_pdata = 0;
	im.m_bufsize = 0;

	// reset flags
	im.m_bend = true;
	im.m_bswap = false;
}


void xpltArchive::Flush()
{
	if (im.m_fp && im.m_pRoot)
	{
		im.m_fp->BeginStreaming();
		im.m_pRoot->Write(im.m_fp);
		im.m_fp->EndStreaming();
	}
	delete im.m_pRoot;
	im.m_pRoot = 0;
	im.m_pChunk = 0;
}


bool xpltArchive::Create(const char* szfile)
{
	// attempt to create the file
	assert(im.m_fp == 0);
	im.m_fp = new FileStream();
	if (im.m_fp->Create(szfile) == false) return false;

	// write the master tag 
	unsigned int ntag = 0x00464542;
	im.m_fp->Write(&ntag, sizeof(int), 1);

	im.m_bSaving = true;

	return true;
}

void xpltArchive::BeginChunk(unsigned int id)
{
	if (im.m_pRoot == 0)
	{
		im.m_pRoot = new OBranch(id);
		im.m_pChunk = im.m_pRoot;
	}
	else
	{
		// create a new branch
		OBranch* pbranch = new OBranch(id);

		// attach it to the current branch
		im.m_pChunk->AddChild(pbranch);

		// move the current branch pointer
		im.m_pChunk = pbranch;
	}
}

void xpltArchive::EndChunk()
{
	if (im.m_pChunk != im.m_pRoot)
		im.m_pChunk = im.m_pChunk->GetParent();
	else
	{
		Flush();
	}
}

bool xpltArchive::Open(FileStream* fp)
{
	// store a copy of the file pointer
	im.m_fp = fp;

	// read the master tag
	unsigned int ntag;
	if (im.m_fp->read(&ntag, sizeof(int), 1) != 1)
	{
		Close();
		return false;
	}

	// see if the file needs to be byteswapped
	if (ntag == 0x00464542) im.m_bswap = false;
	else
	{
		bswap(ntag);
		if (ntag == 0x00464542) im.m_bswap = true;
		else
		{
			// unknown file format
			Close();
			return false;
		}
	}

	// set the end flag to false
	im.m_bend = false;

	// initialize decompression stream
#ifdef HAVE_ZLIB
	im.strm.zalloc = Z_NULL;
	im.strm.zfree = Z_NULL;
	im.strm.opaque = Z_NULL;
	im.strm.avail_in = 0;
	im.strm.next_in = Z_NULL;
	im.strm.avail_out = 0;
#endif

	return true;
}

bool xpltArchive::DecompressChunk(unsigned int& nid, unsigned int& nsize)
{
#ifdef HAVE_ZLIB
	const int CHUNK = 16384;
	nsize = -1;

	int ret;
	unsigned have;
	static unsigned char in[CHUNK];
	static unsigned char out[CHUNK];

	/* allocate inflate state */
	ret = inflateInit(&im.strm);
	if (ret != Z_OK) return ret;

	// the uncompressed buffer
	IOMemBuffer buf;

	/* decompress until deflate stream ends or end of file */
	do {
		if (im.strm.avail_in == 0)
		{
			im.strm.avail_in = im.m_fp->read(in, 1, CHUNK);
			if (ferror(im.m_fp->FilePtr())) {
				(void)inflateEnd(&im.strm);
				return Z_ERRNO;
			}
			if (im.strm.avail_in == 0) break;
			im.strm.next_in = in;
		}

		/* run inflate() on input until output buffer not full */
		do {
			im.strm.avail_out = CHUNK;
			im.strm.next_out = out;
			ret = inflate(&im.strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&im.strm);
//				return ret;
				return false;
			}
			have = CHUNK - im.strm.avail_out;

			buf.append(out, have);

		} while (im.strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	char* pbuf = buf.data();
	if (pbuf)
	{
		memcpy(&nid, pbuf, sizeof(int)); pbuf += sizeof(int); if (im.m_bswap) bswap(nid);
		memcpy(&nsize, pbuf, sizeof(int)); pbuf += sizeof(int); if (im.m_bswap) bswap(nsize);

		im.m_bufsize = buf.size() - 2 * sizeof(int);
		im.m_buf = new char[im.m_bufsize];
		memcpy(im.m_buf, pbuf, im.m_bufsize);
		im.m_pdata = im.m_buf;
	}

	/* clean up and return */
	(void)inflateEnd(&im.strm);
//	return (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
	return (ret == Z_STREAM_END ? true : false);
#endif
	return false;
}


bool xpltArchive::Append(const char* szfile)
{
	// reopen the plot file for appending
	assert(im.m_fp == 0);
	im.m_fp = new FileStream();
	if (im.m_fp->Append(szfile) == false) return false;
	im.m_bSaving = true;
	return true;
}

int xpltArchive::OpenChunk()
{
	// see if the end flag was set
	// in that case we first need to clear the flag
	if (im.m_bend)
	{
		im.m_bend = false;
		return IO_END;
	}

	// see if we have a buffer allocated
	if (im.m_buf == 0)
	{
		unsigned int id, nsize;
		if (im.m_ncompress == 0)
		{
			// see if we have reached the end of the file
			if (feof(im.m_fp->FilePtr()) || ferror(im.m_fp->FilePtr())) return IO_ERROR;

			// get the master chunk id and size
			int nret = im.m_fp->read(&id, sizeof(unsigned int), 1); if (nret != 1) return IO_ERROR;
			if (im.m_bswap) bswap(id);
			nret = im.m_fp->read(&nsize, sizeof(unsigned int), 1); if (nret != 1) return IO_ERROR;
			if (im.m_bswap) bswap(nsize);

			if (nsize == 0)
			{
				im.m_bend = true;
				return IO_END;
			}
			else
			{
				// allocate the buffer
				im.m_bufsize = nsize;
				im.m_buf = new char[im.m_bufsize];

				// read the buffer from file
				int nread = im.m_fp->read(im.m_buf, sizeof(char), nsize);
				if (nread != nsize) return IO_ERROR;

				// set the data pointer
				im.m_pdata = im.m_buf;
			}
		}
		else
		{
			// we need to decompress the chunk
			if (DecompressChunk(id, nsize) == false) return IO_ERROR;
		}

		// create a new chunk
		CHUNK* pc = new CHUNK;
		pc->id = id;
		pc->nsize = nsize;
		pc->pdata = im.m_pdata;
		// add it to the stack
		im.m_Chunk.push(pc);
	}
	else
	{
		// create a new chunk
		CHUNK* pc = new CHUNK;

		// read the chunk ID
		if (read(pc->id) == IO_ERROR) return IO_ERROR;

		// read the chunk size
		if (read(pc->nsize) == IO_ERROR) return IO_ERROR;
		if (pc->nsize == 0) im.m_bend = true;

		// store the data pointer
		pc->pdata = im.m_pdata;

		// add it to the stack
		im.m_Chunk.push(pc);
	}

	return IO_OK;
}

void xpltArchive::CloseChunk()
{
	// pop the last chunk
	CHUNK* pc = im.m_Chunk.top(); im.m_Chunk.pop();

	// calculate the offset to the end of the chunk
	off_type noff = pc->nsize - ((char*)im.m_pdata - (char*)pc->pdata);

	// skip any remaining part in the chunk
	// I wonder if this can really happen
	if (noff != 0) im.m_pdata = (char*)im.m_pdata + noff;

	// delete this chunk
	delete pc;

	// take a peek at the parent
	if (im.m_Chunk.empty())
	{
		// we just deleted the master chunk
		im.m_bend = true;

		// delete the buffer
		delete[] im.m_buf;
		im.m_buf = 0;
		im.m_pdata = 0;
		im.m_bufsize = 0;
	}
	else
	{
		pc = im.m_Chunk.top();
		off_type noff = pc->nsize - ((char*)im.m_pdata - (char*)pc->pdata);
		if (noff == 0) im.m_bend = true;
	}
}

unsigned int xpltArchive::GetChunkID()
{
	CHUNK* pc = im.m_Chunk.top();
	assert(pc);
	return pc->id;
}

unsigned int xpltArchive::GetChunkSize()
{
	CHUNK* pc = im.m_Chunk.top();
	assert(pc);
	return pc->nsize;
}

xpltArchive::IOResult xpltArchive::read(char& c) { mread(&c, sizeof(char), 1, &im.m_pdata); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(int& n) { mread(&n, sizeof(int), 1, &im.m_pdata); if (im.m_bswap) bswap(n); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(bool& b) { mread(&b, sizeof(bool), 1, &im.m_pdata); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(float& f) { mread(&f, sizeof(float), 1, &im.m_pdata); if (im.m_bswap) bswap(f); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(double& g) { mread(&g, sizeof(double), 1, &im.m_pdata); if (im.m_bswap) bswap(g); return IO_OK; }

xpltArchive::IOResult xpltArchive::read(unsigned int& n) { mread(&n, sizeof(unsigned int), 1, &im.m_pdata); if (im.m_bswap) bswap(n); return IO_OK; }

xpltArchive::IOResult xpltArchive::read(char* pc, int n) { mread(pc, sizeof(char), n, &im.m_pdata); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(int* pi, int n) { mread(pi, sizeof(int), n, &im.m_pdata); if (im.m_bswap) bswapv(pi, n); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(bool* pb, int n) { mread(pb, sizeof(bool), n, &im.m_pdata); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(float* pf, int n) { mread(pf, sizeof(float), n, &im.m_pdata); if (im.m_bswap) bswapv(pf, n); return IO_OK; }
xpltArchive::IOResult xpltArchive::read(double* pg, int n) { mread(pg, sizeof(double), n, &im.m_pdata); if (im.m_bswap) bswapv(pg, n); return IO_OK; }

xpltArchive::IOResult xpltArchive::read(char* sz)
{
	IOResult ret;
	int l;
	ret = read(l); if (ret != IO_OK) return ret;
	mread(sz, 1, l, &im.m_pdata);
	sz[l] = 0;
	return IO_OK;
}

xpltArchive::IOResult xpltArchive::sread(char* sz, int max_len)
{
	IOResult ret;
	int l;
	ret = read(l); if (ret != IO_OK) return ret;

	if (l < max_len)
	{
		mread(sz, 1, l, &im.m_pdata);
		sz[l] = 0;
	}
	else
	{
		char* tmp = new char[l + 1];
		mread(tmp, 1, l, &im.m_pdata);
		tmp[l] = 0;
		strncpy(sz, tmp, max_len - 1);
		sz[max_len - 1] = 0;
		delete tmp;
	}

	return IO_OK;
}