#include "stdafx.h"
#include "xpltFileReader.h"
#include "xpltReader.h"
#include "xpltReader2.h"
#include <PostLib/FEPostModel.h>

xpltParser::xpltParser(xpltFileReader* xplt) : m_xplt(xplt), m_ar(xplt->GetArchive())
{
}

xpltParser::~xpltParser()
{
}

bool xpltParser::errf(const char* szerr)
{
	return m_xplt->errf(szerr);
}

void xpltParser::addWarning(int n)
{
	for (int i=0; i<warnings(); ++i)
		if (warning(i) == n) return;
	m_wrng.push_back(n); 
}

xpltFileReader::xpltFileReader() : FEFileReader("FEBio plot")
{
	m_xplt = 0;
	m_read_state_flag = XPLT_READ_ALL_STATES;
}

xpltFileReader::~xpltFileReader()
{
}

bool xpltFileReader::Load(Post::FEPostModel& fem, const char* szfile)
{
	// open the file
	if (Open(szfile, "rb") == false) return errf("Failed opening file.");

	// attach the file to the archive
	IOFileStream fs(m_fp, false);
	if (m_ar.Open(&fs) == false) return errf("This is not a valid XPLT file.");

	// open the root chunk (no compression for this sectio)
	m_ar.SetCompression(0);
	if (m_ar.OpenChunk() != xpltArchive::IO_OK) return errf("Error opening root section");
	{
		if (m_ar.GetChunkID() != PLT_ROOT) return errf("Error opening root section");

		// read header section
		if (m_ar.OpenChunk() != xpltArchive::IO_OK) return errf("Error opening header section");
		{
			if (m_ar.GetChunkID() != PLT_HEADER) return errf("Error opening header section");
			if (ReadHeader() == false) return false;
		}
		m_ar.CloseChunk();
	}

	// set the model's meta data
	Post::MetaData& meta = fem.GetMetaData();
	meta.author = m_hdr.author;
	meta.software = m_hdr.software;

	// create a file parser
	if (m_xplt) { delete m_xplt; m_xplt = 0; }
	if (m_hdr.nversion <= 5) m_xplt = new XpltReader(this);
	else m_xplt = new XpltReader2(this);

	// load the rest of the file
	bool bret = m_xplt->Load(fem);

	// clean up
	m_ar.Close();
	Close();

	if (m_xplt->warnings() > 0)
	{
		for (int i=0; i<m_xplt->warnings(); ++i)
		{
			switch (m_xplt->warning(i))
			{
			case XPLT_READ_DUPLICATE_FACES: errf("WARNING: Duplicate surface values encountered.");
			break;
			default:
				errf("WARNING: unknown warning occurred.");
			}
		}
	}

	// all done
	return bret;
}


//-----------------------------------------------------------------------------
bool xpltFileReader::ReadHeader()
{
	m_hdr.nversion = 0;
	m_hdr.nn = 0;
	m_hdr.nmax_facet_nodes = 4;	// default for version 0.1
	m_hdr.ncompression = 0;	// default for version < 0.3
	m_hdr.author[0] = 0;
	m_hdr.software[0] = 0;
	while (m_ar.OpenChunk() == xpltArchive::IO_OK)
	{
		int nid = m_ar.GetChunkID();
		switch (nid)
		{
		case PLT_HDR_VERSION        : m_ar.read(m_hdr.nversion); break;
		case PLT_HDR_COMPRESSION    : m_ar.read(m_hdr.ncompression); break;
		// version < 2.0
		case PLT_HDR_NODES          : m_ar.read(m_hdr.nn); break;
		case PLT_HDR_MAX_FACET_NODES: m_ar.read(m_hdr.nmax_facet_nodes); break;
		// version 2.0 and up
		case PLT_HDR_AUTHOR         : m_ar.read(m_hdr.author); break;
		case PLT_HDR_SOFTWARE       : m_ar.read(m_hdr.software); break;
		default:
			return errf("Error while reading header.");
		}
		m_ar.CloseChunk();
	}
	return true;
}
