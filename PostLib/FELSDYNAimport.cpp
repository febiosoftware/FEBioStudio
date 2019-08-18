// FELSDYNAimport.cpp: implementation of the FELSDYNAimport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FELSDYNAimport.h"
#include "FEDataManager.h"
#include "FEMeshData_T.h"
#include "constants.h"

using namespace Post;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
FELSDYNAimport::FELSDYNAimport() : FEFileReader("LSDYNA keyword")
{
	m_bdispl = false;
}

//-----------------------------------------------------------------------------
FELSDYNAimport::~FELSDYNAimport()
{

}

//-----------------------------------------------------------------------------
char* FELSDYNAimport::get_line(char* szline)
{
	do
	{
		fgets(szline, 255, m_fp);
		if (feof(m_fp)) return 0;
	}
	while (szline[0] == '$');

	char* ch = strrchr(szline, '\n');
	if (ch) *ch = 0;

	ch = strrchr(szline, '\r');
	if (ch) *ch = 0;

	return szline;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Load(FEModel& fem, const char* szfile)
{
	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file.");

	// make sure the first line is a *KEYWORD
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");
	if (strcmp(m_szline, "*KEYWORD") != 0) return errf("FATAL ERROR: This is not a LSDYNA Keyword file.");

	// clear all data
	m_node.clear();
	m_shell.clear();
	m_solid.clear();

	m_bnresults = false;
	m_bshellthick = false;

#ifdef LINUX
	fprintf(stderr, "\nReading file ...");
#endif

	// get the next line
	if (get_line(m_szline) == 0) return errf("FATAL ERROR: Unexpected end of file.");

	// repeat until done
	bool bdone = false;
	do
	{
		if (strcmp(m_szline, "*ELEMENT_SOLID") == 0)
		{
			if (Read_Element_Solid() == false) return errf("FATAL ERROR: error while reading ELEMENT_SOLID section.");
		}
		else if (strcmp(m_szline, "*ELEMENT_SHELL") == 0)
		{
			if (Read_Element_Shell() == false) return errf("FATAL ERROR: error while reading ELEMENT_SHELL section.");
		}
		else if (strcmp(m_szline, "*ELEMENT_SHELL_THICKNESS") == 0)
		{
			if (Read_Element_Shell_Thickness() == false) return errf("FATAL ERROR: error while readin ELEMENT_SHELL_THCICKNESS section.");
			m_bshellthick = true;
		}
		else if (strcmp(m_szline, "*NODE") == 0)
		{
			if (Read_Node() == false) return errf("FATAL ERROR: error while reading NODE section.");
		}
		else if (strcmp(m_szline, "*NODAL_RESULTS") == 0)
		{
			if (Read_Nodal_Results() == false) return errf("FATAL ERROR: error while readin NODAL_RESULTS section.");
			m_bnresults = true;
		}
		else if (strcmp(m_szline, "*END") == 0)
		{
			bdone = true;
		}
		else if (get_line(m_szline) == 0) return errf("FATAL ERROR: unexpected end of file.");
	}
	while (!bdone);

	// close the file
	Close();

#ifdef LINUX
	fprintf(stderr, "done\n");
#endif

	// build the mesh
	return BuildMesh(fem);
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Read_Element_Solid()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SOLID el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3], &el.n[4], &el.n[5], &el.n[6], &el.n[7]);
		if (nread != 10) return false;

		m_solid.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Read_Element_Shell()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SHELL el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3]);
		if (nread != 6) return false;

		m_shell.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Read_Element_Shell_Thickness()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	ELEMENT_SHELL el;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%d%d%d%d%d", &el.id, &el.mid, &el.n[0], &el.n[1], &el.n[2], &el.n[3]);
		if (nread != 6) return false;

		if (get_line(m_szline) == 0) return false;
		nread = sscanf(m_szline, "%lg%lg%lg%lg", &el.h[0], &el.h[1], &el.h[2], &el.h[3]);
		if (nread != 4) return false;

		m_shell.push_back(el);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Read_Node()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	NODE n;
	while (m_szline[0] != '*')
	{
		nread = sscanf(m_szline, "%d%lg%lg%lg", &n.id, &n.x, &n.y, &n.z);
		if (nread != 4) return false;

		m_node.push_back(n);

		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::Read_Nodal_Results()
{
	if (get_line(m_szline) == 0) return false;
	int nread;
	list<NODE>::iterator it = m_node.begin();
	while (m_szline[0] != '*')
	{
		if (it == m_node.end()) return false;

		nread = sscanf(m_szline, "%*d%lg", &it->v);
		if (nread != 1) return false;

		++it;
		if (get_line(m_szline) == 0) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
void FELSDYNAimport::BuildMaterials(FEModel& fem)
{
	int shells = m_shell.size();
	int solids = m_solid.size();
	int i;

	int nm0 = 9999, nm1 = -9999;
	list<ELEMENT_SOLID>::iterator ih = m_solid.begin();
	for (i=0; i<solids; ++i, ++ih)
	{
		if (ih->mid > nm1) nm1 = ih->mid;
		if (ih->mid < nm0) nm0 = ih->mid;
	}
	list<ELEMENT_SHELL>::iterator is = m_shell.begin();
	for (i=0; i<shells; ++i, ++is)
	{
		if (is->mid > nm1) nm1 = is->mid;
		if (is->mid < nm0) nm0 = is->mid;
	}

	int nmat = nm1 - nm0 + 1;
	for (i=0; i<nmat; ++i)
	{
		FEMaterial mat;
		fem.AddMaterial(mat);
	}

	ih = m_solid.begin();
	for (i=0; i<solids; ++i, ++ih) ih->mid -= nm0;
	is = m_shell.begin();
	for (i=0; i<shells; ++i, ++is) is->mid -= nm0;
}

//-----------------------------------------------------------------------------
bool FELSDYNAimport::BuildMesh(FEModel& fem)
{
	int i;

#ifdef LINUX
	fprintf(stderr, "Building geometry...");
#endif

	int nodes  = m_node.size();
	int shells = m_shell.size();
	int solids = m_solid.size();

	int elems = shells + solids;

	if (nodes == 0) return errf("FATAL ERROR: No nodal data defined in file.");
	if (elems == 0) return errf("FATAL ERROR: No element data defined in file.");

	fem.Clear();

	// create the materials
	BuildMaterials(fem);

	// build the mesh
	FEMeshBase* pm = m_pm = new FEMesh;
	pm->Create(nodes, elems);
	fem.AddMesh(pm);

	// create nodes
	list<NODE>::iterator in = m_node.begin();
	for (i=0; i<nodes; ++i, ++in)
	{
		FENode& n = pm->Node(i);
		in->n = i;
		n.m_r0.x = n.m_rt.x = (float) in->x;
		n.m_r0.y = n.m_rt.y = (float) in->y;
		n.m_r0.z = n.m_rt.z = (float) in->z;
	}

	// create solids
	int ne = 0;
	if (solids > 0)
	{
		list<NODE>::iterator in = m_node.begin();
		list<ELEMENT_SOLID>::iterator ih = m_solid.begin();
		for (i=0; i<solids; ++i, ++ih)
		{
			FEGenericElement& el = static_cast<FEGenericElement&>(pm->Element(ne++));
			int* n = ih->n;
			if ((n[7] == n[6]) && (n[7]==n[5]) && (n[7]==n[4]) && (n[7]==n[3])) el.SetType(FE_TET4);
			else if ((n[7] == n[6]) && (n[7]==n[5])) el.SetType(FE_PENTA6);
			else el.SetType(FE_HEX8);

			el.m_MatID = ih->mid;

			el.m_node[0] = FindNode(ih->n[0], in); if (el.m_node[0] < 0) return false;
			el.m_node[1] = FindNode(ih->n[1], in); if (el.m_node[1] < 0) return false;
			el.m_node[2] = FindNode(ih->n[2], in); if (el.m_node[2] < 0) return false;
			el.m_node[3] = FindNode(ih->n[3], in); if (el.m_node[3] < 0) return false;
			el.m_node[4] = FindNode(ih->n[4], in); if (el.m_node[4] < 0) return false;
			el.m_node[5] = FindNode(ih->n[5], in); if (el.m_node[5] < 0) return false;
			el.m_node[6] = FindNode(ih->n[6], in); if (el.m_node[6] < 0) return false;
			el.m_node[7] = FindNode(ih->n[7], in); if (el.m_node[7] < 0) return false;
		}
	}

	// create shells
	if (shells > 0)
	{
		list<NODE>::iterator in = m_node.begin();
		list<ELEMENT_SHELL>::iterator is = m_shell.begin();
		for (i=0; i<shells; ++i, ++is)
		{
			FEGenericElement& el = static_cast<FEGenericElement&>(pm->Element(ne++));
			el.m_node[0] = FindNode(is->n[0], in); if (el.m_node[0] < 0) return false;
			el.m_node[1] = FindNode(is->n[1], in); if (el.m_node[1] < 0) return false;
			el.m_node[2] = FindNode(is->n[2], in); if (el.m_node[2] < 0) return false;

			el.m_MatID = is->mid;

			if (is->n[3] == is->n[2])
			{
				el.SetType(FE_TRI3);
				el.m_node[3] = el.m_node[2];
			}
			else
			{
				el.SetType(FE_QUAD4);
				el.m_node[3] = FindNode(is->n[3], in); if (el.m_node[3] < 0) return false;
			}
		}
	}

	// update the mesh
	m_pm->Update();
	fem.UpdateBoundingBox();

	// add some data
	FEDataManager* pdm = fem.GetDataManager();
	int ndata[2] = {-1, -1}, nd=0;
	if (m_bnresults)
	{
		pdm->AddDataField(new FEDataField_T<FENodeData<float> >("Nodal Results", EXPORT_DATA)); 
		ndata[0] = nd;	nd++;
	}

	if (m_bdispl)
	{
		pdm->AddDataField(new FEDataField_T<FENodeData<vec3f> >("Displacement", EXPORT_DATA));
		ndata[1] = nd; 
		fem.SetDisplacementField(BUILD_FIELD(1, nd, 0));
		nd++;
	}

	if (m_bshellthick)
	{
		pdm->AddDataField(new FEDataField_T<FEElementData<float ,DATA_COMP> >("shell thickness", EXPORT_DATA));
	}

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	// add some data
	if (m_bnresults)
	{
		FENodeData<float>& d = dynamic_cast<FENodeData<float>&>(ps->m_Data[ndata[0]]);
		list<NODE>::iterator pn = m_node.begin();
		for (i=0; i<nodes; ++i, ++pn) d[i] = (float) pn->v;
	}

	if (m_bshellthick)
	{
		int nel8 = m_solid.size();
		int nel2 = 0;	// we don't read beams yet
		ELEMDATA* pd = &ps->m_ELEM[0] + (nel8 + nel2);

		list<ELEMENT_SHELL>::iterator pe = m_shell.begin();
		for (i=0; i<(int) m_shell.size(); ++i, ++pe)
		{
			double* h = pe->h;
			pd[i].m_h[0] = (float) h[0];
			pd[i].m_h[1] = (float) h[1];
			pd[i].m_h[2] = (float) h[2];
			pd[i].m_h[3] = (float) h[3];
		}

		FEElementData<float,DATA_COMP>& d = dynamic_cast<FEElementData<float,DATA_COMP>&>(ps->m_Data[0]);
		pe = m_shell.begin();
		float h[4];
		for (i=0; i<(int) m_shell.size(); ++i, ++pe)
		{
			int ne = i;
			h[0] = (float) pe->h[0];
			h[1] = (float) pe->h[1];
			h[2] = (float) pe->h[2];
			h[3] = (float) pe->h[3];
			d.add(ne, 4, h);
		}
	}

	if (m_bdispl)
	{
		FENodeData<vec3f>& d = dynamic_cast<FENodeData<vec3f>&>(ps->m_Data[ndata[1]]);
		for (i=0; i<nodes; ++i) d[i] = vec3f(0.f, 0.f, 0.f);
	}

	// clean up
	m_node.clear();
	m_shell.clear();
	m_solid.clear();


#ifdef LINUX
	fprintf(stderr, "done\n\n");
#endif

	// we're good!
	return true;
}

//-----------------------------------------------------------------------------
int FELSDYNAimport::FindNode(int id, list<NODE>::iterator& pn)
{
	int N = m_node.size();
	int m = 0;
	do
	{
		if (id == pn->id) return pn->n;
		else if (id < pn->id)
		{
			if (pn->n > 0) --pn; else return -1;
		}
		else
		{
			if (pn->n < N-1) ++pn; else return -1;
		}
		++m;
	}
	while (m <= N);

	return -1;
}
