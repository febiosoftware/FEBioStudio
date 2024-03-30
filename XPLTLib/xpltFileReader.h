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
#include "PostLib/FEFileReader.h"
#include "xpltArchive.h"

enum XPLT_READ_STATE_FLAG { 
	XPLT_READ_ALL_STATES, 
	XPLT_READ_ALL_CONVERGED_STATES, 
	XPLT_READ_LAST_STATE_ONLY, 
	XPLT_READ_STATES_FROM_LIST,
	XPLT_READ_FIRST_AND_LAST 
};

enum XPLT_READ_WARNING {
	XPLT_READ_DUPLICATE_FACES			// warning issued when surface values are overwritten
};

class xpltParser;

class xpltFileReader;

class xpltParser
{
public:
	xpltParser(xpltFileReader* xplt);
	virtual ~xpltParser();

	virtual bool Load(Post::FEPostModel& fem) = 0;

	bool errf(const char* sz);

	void addWarning(int n);
	int warnings() const { return (int) m_wrng.size(); }
	int warning(int n) const { return m_wrng[n]; }

	int FileVersion() const;

protected:
	xpltFileReader*		m_xplt;
	xpltArchive&		m_ar;
	std::vector<int>	m_wrng;	// warning list
};

class xpltFileReader : public Post::FEFileReader
{
protected:
	// file tags
	enum {
		PLT_ROOT = 0x01000000,
		PLT_HEADER = 0x01010000,
		PLT_HDR_VERSION = 0x01010001,
		PLT_HDR_NODES = 0x01010002,	// obsolete in 2.0
		PLT_HDR_MAX_FACET_NODES = 0x01010003,	// obsolete in 2.0 (redefined in each Surface section)
		PLT_HDR_COMPRESSION = 0x01010004,
		PLT_HDR_AUTHOR = 0x01010005,	// new in 2.0
		PLT_HDR_SOFTWARE = 0x01010006,	// new in 2.0
		PLT_HDR_UNITS = 0x01010007,	// new in 4.0
	};

	// size of name variables
	enum { DI_NAME_SIZE = 64 };

public:
	struct HEADER
	{
		int	nversion;
		int ncompression;				//!< compression method (or level)
		int	nn;							//!< nodes (not used for >= 2.0)
		int	nmax_facet_nodes;			//!< max nodes per facet (depends on version; not used >= 2.0)
		char author[DI_NAME_SIZE];		//!< name of author
		char software[DI_NAME_SIZE];	//!< name of software that generated the file
		char units[DI_NAME_SIZE];
	};

public:
	xpltFileReader(Post::FEPostModel* fem);
	~xpltFileReader();

	using FEFileReader::Load;
	bool Load(const char* szfile) override;

	void SetReadStateFlag(int n) { m_read_state_flag = n; }
	void SetReadStatesList(const std::vector<int>& l) { m_state_list = l; }

	int GetReadStateFlag() const { return m_read_state_flag; }
	std::vector<int> GetReadStates() const { return m_state_list; }

public:
	xpltArchive& GetArchive() { return m_ar; }

	const HEADER& GetHeader() const { return m_hdr; }

	const char* GetUnits() const { return (m_hdr.units[0] ? m_hdr.units : nullptr); }

protected:
	bool ReadHeader();

private:
	xpltParser*		m_xplt;
	xpltArchive		m_ar;
	HEADER			m_hdr;

	// Options
	int			m_read_state_flag;	//!< flag setting option for reading states
	std::vector<int>	m_state_list;		//!< list of states to read (only when m_read_state_flag == XPLT_READ_STATES_FROM_LIST)

	friend class xpltParser;
};
