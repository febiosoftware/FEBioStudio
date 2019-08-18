#pragma once
#include "FEFileReader.h"

namespace Post {

struct PLOTHEADER
{
	char	Title[40];		// title of the problem
	int		UnUsed0[5];		// blanks (unused)
	int		ndim;			// number of dimensions
	int		nump;			// number of nodal points
	int		icode;			// code descriptor
	int		nglbv;			// number of global state variables
	int		flagT;			// state nodal temperatures included ?
	int		flagU;			// state nodal coordinates included ?
	int		flagV;			// state nodal velocities included ?
	int		flagA;			// state nodal accelerations included ?
	int		nel8;			// number of 8-node hexahedral elements
	int		nummat8;		// number of materials used by hexahedral elements
	int		UnUsed1[2];		// blanks (unused)
	int		nv3d;			// number of variables for hexahedral elements
	int		nel2;			// number of 2-node beam elements
	int		nummat2;		// number of materials used by beam elements
	int		nv1d;			// number of variables for beam elements
	int		nel4;			// number of 4-node shell elements
	int		nummat4;		// number of materials used by shell elements
	int		nv2d;			// number of variables for shell elements
	int		neiph;			// number of additional variables per solid element
	int		neips;			// number of additional variables per shell integration point
	int		maxint;			// number of integration points dumped for each shell
	int		UnUsed3[7];		// blank (unused)
	int		ioshl1;			// 6 stress component flag for shells
	int		ioshl2;			// plastic strain flag for shells
	int		ioshl3;			// shell force resultant flag
	int		ioshl4;			// shell thickness, energy + 2 more
	int		UnUsed4[16];	// blank (unused)
};

//-----------------------------------------------------------------------------
class FELSDYNAPlotImport : public FEFileReader
{
	enum { LSDYNA_MAXFIELDS = 8 };

	enum {
		LSDYNA_DISP,
		LSDYNA_VEL,
		LSDYNA_ACC,
		LSDYNA_TEMP,
		LSDYNA_STRESS,
		LSDYNA_PRESSURE,
		LSDYNA_SHELL_STRAIN,
		LSDYNA_PLASTIC
	};

public:
	FELSDYNAPlotImport();
	~FELSDYNAPlotImport();

	bool Load(FEModel& fem, const char* szfile);

protected:
	bool ReadHeader   (FEModel& fem);
	bool ReadMesh     (FEModel& fem);
	bool ReadStates   (FEModel& fem);

	void CreateMaterials(FEModel& fem);

protected:
	int ReadData(void* pd, size_t nsize, size_t ncnt, bool bdump = false);

public:
	bool	m_brepeat;
	int		m_naction;

protected:
	int		m_ifile;	//!< file counter (used when reading family of plot files)
	bool	m_bswap;	//!< need to swap data or not

	PLOTHEADER	m_hdr;							//!< plot file header
	int			m_nfield[LSDYNA_MAXFIELDS];		//!< pre-defined data fields
};

//-----------------------------------------------------------------------------
class FELSDYNAPlotExport
{
public:
	bool Save(FEModel& fem, const char* szfile, bool bflag[6], int ncode[6]);
};
}
