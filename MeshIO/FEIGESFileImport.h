#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEIGESFileImport : public FEFileImport
{
protected:
	enum SECTION_CODE
	{
		IGS_FLAG,
		IGS_START,
		IGS_GLOBAL,
		IGS_DIRECTORY,
		IGS_PARAMETER,
		IGS_TERMINATE
	};

	struct RECORD
	{
		char	szdata[73];	// data field
		int		nsec;		// section identifier
		int		nnum;		// sequence number
	};

	struct DIRECTORY_ENTRY
	{
		int		ntype;		// entity type number
		int		pdata;		// pointer to parameter data
		int		pstruct;	// pointer to defining structure
		int		nfont;		// line font pattern
		int		nlevel;		// number of level
		int		pview;		// pointer to view
		int		ptrans;		// pointer to transformation matrix
		int		pdisplay;	// pointer to label display entitiy
		int		nstatus;	// status number
		int		nweight;	// line thickness
		int		ncolor;		// color number
		int		ncount;		// line count in parameter section
		int		nform;		// form number
		int		nlabel;		// label number		
	};

public:
	FEIGESFileImport(FEProject& prj);
	~FEIGESFileImport(void);

	bool Load(const char* szfile);

protected:
	bool read_record(RECORD& rec);

	bool ReadStartSection    (RECORD& rec);
	bool ReadGlobalSection   (RECORD& rec);
	bool ReadDirectorySection(RECORD& rec);
	bool ReadParameterSection(RECORD& rec);
	bool ReadTerminateSection(RECORD& rec);

protected:
	FEModel*	m_pfem;
};
