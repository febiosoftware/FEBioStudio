#pragma once
#include "FEFileReader.h"
#include <list>

namespace Post {

class FEMeshBase;

class FELSDYNAimport : public FEFileReader  
{
protected:
	struct ELEMENT_SHELL
	{
		int		id;
		int		mid;
		int		n[4];
		double	h[4];

		ELEMENT_SHELL() { h[0] = h[1] = h[2] = h[3] = 0; }
	};

	struct ELEMENT_SOLID
	{
		int id;
		int	mid;
		int n[8];
	};

	struct NODE
	{
		int id;
		int n;
		double	x, y, z;
		double v;
	};

public:
	FELSDYNAimport();
	virtual ~FELSDYNAimport();

	bool Load(FEModel& fem, const char* szfile);

	int FindNode(int id, std::list<NODE>::iterator& pn);

	void read_displacements(bool b) { m_bdispl = b; }

protected:
	char* get_line(char* szline);

	bool Read_Element_Solid();
	bool Read_Element_Shell();
	bool Read_Element_Shell_Thickness();
	bool Read_Node();
	bool Read_Nodal_Results();

	void BuildMaterials(FEModel& fem);
	bool BuildMesh(FEModel& fem);

protected:
	std::list<ELEMENT_SOLID>		m_solid;
	std::list<ELEMENT_SHELL>		m_shell;
	std::list<NODE>					m_node;

	FEMeshBase*			m_pm;

	bool	m_bnresults;	// nodal results included?
	bool	m_bshellthick;	// shell thicknesses included?
	bool	m_bdispl;		// define displacement field?

	char			m_szline[256];
};

}
