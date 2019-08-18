// Archive.cpp: implementation of the Archive class.
//
//////////////////////////////////////////////////////////////////////

#include "Archive.h"

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
	return Open(fp, signature);
}

//-----------------------------------------------------------------------------
bool IArchive::Open(FILE* fp, unsigned int signature)
{
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

//////////////////////////////////////////////////////////////////////
// OArchive
//////////////////////////////////////////////////////////////////////

OArchive::OArchive()
{
	m_fp = 0;
	m_pRoot = 0;
	m_pChunk = 0;
}

OArchive::~OArchive()
{
	Close();
}

void OArchive::Close()
{
	if (m_fp)
	{
		m_pRoot->Write(m_fp);
		fclose(m_fp);
	}
	m_fp = 0;

	delete m_pRoot;
	m_pRoot = 0;
	m_pChunk = 0;
}

bool OArchive::Create(const char* szfile, unsigned int signature)
{
	// attempt to create the file
	m_fp = fopen(szfile, "wb");
	if (m_fp == 0) return false;

	// write the master tag 
	fwrite(&signature, sizeof(int), 1, m_fp);

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
