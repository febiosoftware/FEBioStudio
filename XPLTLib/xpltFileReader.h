#pragma once
#include "PostLib/FEFileReader.h"
#include "xpltArchive.h"

enum XPLT_READ_STATE_FLAG { 
	XPLT_READ_ALL_STATES, 
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

protected:
	xpltFileReader*		m_xplt;
	xpltArchive&		m_ar;
	vector<int>			m_wrng;	// warning list
};

class xpltFileReader : public Post::FEFileReader
{
protected:
	// file tags
	enum { 
		PLT_ROOT						= 0x01000000,
		PLT_HEADER						= 0x01010000,
			PLT_HDR_VERSION				= 0x01010001,
			PLT_HDR_NODES				= 0x01010002,	// obsolete in 2.0
			PLT_HDR_MAX_FACET_NODES		= 0x01010003,	// obsolete in 2.0 (redefined in each Surface section)
			PLT_HDR_COMPRESSION			= 0x01010004,	
			PLT_HDR_AUTHOR				= 0x01010005,	// new in 2.0
			PLT_HDR_SOFTWARE			= 0x01010006,	// new in 2.0
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
	};

public:
	xpltFileReader(Post::FEPostModel* fem);
	~xpltFileReader();

	using FEFileReader::Load;
	bool Load(const char* szfile) override;

	void SetReadStateFlag(int n) { m_read_state_flag = n; }
	void SetReadStatesList(const vector<int>& l) { m_state_list = l; }

	int GetReadStateFlag() const { return m_read_state_flag; }
	vector<int> GetReadStates() const { return m_state_list; }

public:
	xpltArchive& GetArchive() { return m_ar; }

	const HEADER& GetHeader() const { return m_hdr; }

protected:
	bool ReadHeader();

private:
	xpltParser*		m_xplt;
	xpltArchive		m_ar;
	HEADER			m_hdr;

	// Options
	int			m_read_state_flag;	//!< flag setting option for reading states
	vector<int>	m_state_list;		//!< list of states to read (only when m_read_state_flag == XPLT_READ_STATES_FROM_LIST)

	friend class xpltParser;
};
