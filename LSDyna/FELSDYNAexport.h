#pragma once

#include "MeshTools/FEFileExport.h"

//-----------------------------------------------------------------------------
struct LSDYNAEXPORT
{
	bool	bselonly;		// selection only
	bool	bshellthick;	// shell thickness
};

//-----------------------------------------------------------------------------
class FELSDYNAexport : public FEFileExport
{
public:
	FELSDYNAexport(void);
	~FELSDYNAexport(void);

	void SetOptions(LSDYNAEXPORT o) { m_ops = o; }

	bool Export(FEProject& prj, const char* szfile);

protected:
	bool write_NODE();
	bool write_ELEMENT_SOLID();
	bool write_ELEMENT_SHELL();
	bool write_ELEMENT_SHELL_THICKNESS();
	bool write_SET_SHELL_LIST();

protected:
	int		m_npart;	// counter for part number

protected:
	FILE*			m_fp;
	FEProject*		m_pprj;
	LSDYNAEXPORT	m_ops;
};
