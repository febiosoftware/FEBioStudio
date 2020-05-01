#pragma once
#include "MeshIO/FileReader.h"
#include "MeshTools/FEProject.h"
#include "LSDYNAModel.h"

#include <list>
using namespace std;

class FELSDYNAimport : public FEFileImport
{
protected:
	class CARD
	{
	public:
		CARD(int field = 10);

	protected:
		bool nexti(int&    n, int nwidth = -1);		// return the next integer parameter
		bool nextd(double& d, int nwidth = -1);		// return the next double parameter

		const char* szvalue() { return m_szline; }

		bool IsKeyword() { return m_szline[0] == '*'; }

	protected:
		enum { MAX_LINE = 256 };		// max characters per line
		char	m_szline[MAX_LINE];		// line read in from file
		bool	m_bfree;				// free format flag
		char*	m_ch;					// current position in line
		int		m_nfield;				// field width
		int		m_l;					// length of line

		friend class FELSDYNAimport;
	};

public:
	FELSDYNAimport(FEProject& prj);
	virtual ~FELSDYNAimport();

	bool Load(const char* szfile);

protected:
	bool ReadCard(CARD& c);
	char* get_line(char* szline);

	bool Read_Element_Solid();
	bool Read_Element_Shell();
	bool Read_Element_Shell_Thickness();
    bool Read_Domain_Shell_Thickness();
	bool Read_Node();
	bool Read_Nodal_Results();
	bool Read_Part();
    bool Read_Part_Contact();
    bool Read_Material();
    bool Read_Mat_Elastic();
    bool Read_Mat_Rigid();
    bool Read_Mat_Viscoelastic();
    bool Read_Mat_Kelvin_Maxwell_Viscoelastic();
    bool Read_Mat_Elastic_Spring_Discrete_Beam();
    bool Read_Mat_Other();
	bool Read_Set_Segment_Title();

public:
	double NodeData(int i, int j) { return m_dyna.NodeData(i, j); }

protected:
	LSDYNAModel	m_dyna;

	char			m_szline[256];
    size_t      m_lineno;
	FEModel*	m_pfem;
	FEProject*	m_pprj;
};
