#include "xpltArchive.h"
#include <assert.h>
#include <FSCore/Archive.h>

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

xpltArchive::xpltArchive()
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

xpltArchive::~xpltArchive()
{
	Close();
}

void xpltArchive::Close()
{
	if (m_bSaving)
	{
		if (m_pRoot) Flush();
	}
	else {
		// clear the stack
		while (m_Chunk.empty() == false)
		{
			// pop the last chunk
			CHUNK* pc = m_Chunk.top(); m_Chunk.pop();
			delete pc;
		}
	}

	// close the file pointer
	m_fp = 0;

	// delete the buffer
	if (m_buf) delete[] m_buf;
	m_buf = 0;
	m_pdata = 0;
	m_bufsize = 0;

	// reset flags
	m_bend = true;
	m_bswap = false;
}


void xpltArchive::Flush()
{
	if (m_fp && m_pRoot)
	{
		m_fp->BeginStreaming();
		m_pRoot->Write(m_fp);
		m_fp->EndStreaming();
	}
	delete m_pRoot;
	m_pRoot = 0;
	m_pChunk = 0;
}


bool xpltArchive::Create(const char* szfile)
{
	// attempt to create the file
	assert(m_fp == 0);
	m_fp = new IOFileStream();
	if (m_fp->Create(szfile) == false) return false;

	// write the master tag 
	unsigned int ntag = 0x00464542;
	m_fp->Write(&ntag, sizeof(int), 1);

	m_bSaving = true;

	return true;
}

void xpltArchive::BeginChunk(unsigned int id)
{
	if (m_pRoot == 0)
	{
		m_pRoot = new OBranch(id);
		m_pChunk = m_pRoot;
	}
	else
	{
		// create a new branch
		OBranch* pbranch = new OBranch(id);

		// attach it to the current branch
		m_pChunk->AddChild(pbranch);

		// move the current branch pointer
		m_pChunk = pbranch;
	}
}

void xpltArchive::EndChunk()
{
	if (m_pChunk != m_pRoot)
		m_pChunk = m_pChunk->GetParent();
	else
	{
		Flush();
	}
}

bool xpltArchive::Open(IOFileStream* fp)
{
	// store a copy of the file pointer
	m_fp = fp;

	// read the master tag
	unsigned int ntag;
	if (m_fp->read(&ntag, sizeof(int), 1) != 1)
	{
		Close();
		return false;
	}

	// see if the file needs to be byteswapped
	if (ntag == 0x00464542) m_bswap = false;
	else
	{
		bswap(ntag);
		if (ntag == 0x00464542) m_bswap = true;
		else
		{
			// unknown file format
			Close();
			return false;
		}
	}

	// set the end flag to false
	m_bend = false;

	// initialize decompression stream
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	strm.avail_out = 0;

	return true;
}

int xpltArchive::DecompressChunk(unsigned int& nid, unsigned int& nsize)
{
	const int CHUNK = 16384;
	nsize = -1;

	int ret;
	unsigned have;
	static unsigned char in[CHUNK];
	static unsigned char out[CHUNK];

	/* allocate inflate state */
	ret = inflateInit(&strm);
	if (ret != Z_OK) return ret;

	// the uncompressed buffer
	IOMemBuffer buf;

	/* decompress until deflate stream ends or end of file */
	do {
		if (strm.avail_in == 0)
		{
			strm.avail_in = m_fp->read(in, 1, CHUNK);
			if (ferror(m_fp->FilePtr())) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
			if (strm.avail_in == 0) break;
			strm.next_in = in;
		}

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			switch (ret) {
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;     /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}
			have = CHUNK - strm.avail_out;

			buf.append(out, have);

		} while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	char* pbuf = buf.data();
	if (pbuf)
	{
		memcpy(&nid, pbuf, sizeof(int)); pbuf += sizeof(int); if (m_bswap) bswap(nid);
		memcpy(&nsize, pbuf, sizeof(int)); pbuf += sizeof(int); if (m_bswap) bswap(nsize);

		m_bufsize = buf.size() - 2 * sizeof(int);
		m_buf = new char[m_bufsize];
		memcpy(m_buf, pbuf, m_bufsize);
		m_pdata = m_buf;
	}

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


bool xpltArchive::Append(const char* szfile)
{
	// reopen the plot file for appending
	assert(m_fp == 0);
	m_fp = new IOFileStream();
	if (m_fp->Append(szfile) == false) return false;
	m_bSaving = true;
	return true;
}

int xpltArchive::OpenChunk()
{
	// see if the end flag was set
	// in that case we first need to clear the flag
	if (m_bend)
	{
		m_bend = false;
		return IO_END;
	}

	// see if we have a buffer allocated
	if (m_buf == 0)
	{
		unsigned int id, nsize;
		if (m_ncompress == 0)
		{
			// see if we have reached the end of the file
			if (feof(m_fp->FilePtr()) || ferror(m_fp->FilePtr())) return IO_ERROR;

			// get the master chunk id and size
			int nret = m_fp->read(&id, sizeof(unsigned int), 1); if (nret != 1) return IO_ERROR;
			if (m_bswap) bswap(id);
			nret = m_fp->read(&nsize, sizeof(unsigned int), 1); if (nret != 1) return IO_ERROR;
			if (m_bswap) bswap(nsize);

			if (nsize == 0)
			{
				m_bend = true;
				return IO_END;
			}
			else
			{
				// allocate the buffer
				m_bufsize = nsize;
				m_buf = new char[m_bufsize];

				// read the buffer from file
				int nread = m_fp->read(m_buf, sizeof(char), nsize);
				if (nread != nsize) return IO_ERROR;

				// set the data pointer
				m_pdata = m_buf;
			}
		}
		else
		{
			// we need to decompress the chunk
			if (DecompressChunk(id, nsize) != Z_OK) return IO_ERROR;
		}

		// create a new chunk
		CHUNK* pc = new CHUNK;
		pc->id = id;
		pc->nsize = nsize;
		pc->pdata = m_pdata;
		// add it to the stack
		m_Chunk.push(pc);

	}
	else
	{
		// create a new chunk
		CHUNK* pc = new CHUNK;

		// read the chunk ID
		if (read(pc->id) == IO_ERROR) return IO_ERROR;

		// read the chunk size
		if (read(pc->nsize) == IO_ERROR) return IO_ERROR;
		if (pc->nsize == 0) m_bend = true;

		// store the data pointer
		pc->pdata = m_pdata;

		// add it to the stack
		m_Chunk.push(pc);
	}

	return IO_OK;
}

void xpltArchive::CloseChunk()
{
	// pop the last chunk
	CHUNK* pc = m_Chunk.top(); m_Chunk.pop();

	// calculate the offset to the end of the chunk
	off_type noff = pc->nsize - ((char*)m_pdata - (char*)pc->pdata);

	// skip any remaining part in the chunk
	// I wonder if this can really happen
	if (noff != 0) m_pdata = (char*)m_pdata + noff;

	// delete this chunk
	delete pc;

	// take a peek at the parent
	if (m_Chunk.empty())
	{
		// we just deleted the master chunk
		m_bend = true;

		// delete the buffer
		delete[] m_buf;
		m_buf = 0;
		m_pdata = 0;
		m_bufsize = 0;
	}
	else
	{
		pc = m_Chunk.top();
		off_type noff = pc->nsize - ((char*)m_pdata - (char*)pc->pdata);
		if (noff == 0) m_bend = true;
	}
}

unsigned int xpltArchive::GetChunkID()
{
	CHUNK* pc = m_Chunk.top();
	assert(pc);
	return pc->id;
}
