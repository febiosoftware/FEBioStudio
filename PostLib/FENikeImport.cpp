#include "stdafx.h"
#include "FENikeImport.h"
#include <stdlib.h>
#include "FEModel.h"
#include "color.h"

using namespace Post;

FENikeImport::FENikeImport(void) : FEFileReader("NIKE3D input")
{
	m_pm = 0;
	m_pfem = 0;
}

FENikeImport::~FENikeImport(void)
{

}

bool FENikeImport::Load(FEModel &fem, const char *szfile)
{
	// open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening NIKE file.");

	fem.Clear();
	m_pfem = &fem;

	// read control section
	if (ReadControlSection() == false) return false;

	// read material section
	if (ReadMaterialSection() == false) return false;

	// read geometry section
	if (ReadGeometrySection() == false) return false;

	// close the file
	Close();

	// update the mesh
	if (m_pm) m_pm->Update(); else return false;
	fem.UpdateBoundingBox();

	// we need a single state
	FEState* ps = new FEState(0.f, &fem, fem.GetFEMesh(0));
	fem.AddState(ps);

	return true;
}

char* FENikeImport::get_line(char* szline)
{
	char* ch;
	do
	{
		ch = fgets(szline, 255, m_fp);
		if (ch == 0) return 0;
	}
	while (szline[0] == '*');

	return szline;
}

bool FENikeImport::ReadControlSection()
{
	char szline[256];

	// read card 1
	if (get_line(szline) == 0) return errf("failed reading control card %d", 1);
	char* ch = strchr(szline, '\n');
	if (ch) *ch = 0;
	m_pfem->SetTitle(szline);

	// read card 2
	if (get_line(szline) == 0) return errf("failed reading control card %d", 2);
	sscanf(szline, "%*2s%d%d%d%d%d", &m_nmat, &m_nn, &m_nhel, &m_nbel, &m_nsel);

	if (m_nmat <= 0) return errf("Invalid nr of materials.");
	if (m_nn <= 0) return errf("Invalid nr of nodes.");
	if (m_nbel < 0) return errf("Invalid nr of solid elements.");
	if (m_nsel < 0) return errf("Invalid nr of shell elements.");

	// read card 3-10
	for (int i=3; i<=10; ++i)
		if (get_line(szline) == 0) return errf("failed reading control card %d", i);

	return true;
}

bool FENikeImport::ReadMaterialSection()
{
	char szline[256];

	for (int i=0; i<m_nmat; ++i)
	{
		// read material card 1
		if (get_line(szline) == 0) return errf("failed reading control card %d for material %d", 1, i+1);

		szline[25] = 0;
		int nelem = atoi(szline+20);

		int m = 7;
		if (nelem == 2) m = 9;
		for (int j=0; j<m; ++j)
		{
			if (get_line(szline) == 0) return errf("failed reading control card %d for material %d", j+2, i+1);
		}

		// add a material to the scene
		FEMaterial mat;
		m_pfem->AddMaterial(mat);
	}

	return true;
}

bool FENikeImport::ReadGeometrySection()
{
	int i, j;
	char szline[256];

	// create the geometry
	m_pm = new FEMesh;
	m_pm->Create(m_nn, m_nhel+m_nsel);	
	m_pfem->AddMesh(m_pm);

	// read the nodes
	float x, y, z;
	for (i=0; i<m_nn; ++i)
	{
		FENode& n = m_pm->Node(i);
		if (get_line(szline) == 0) return errf("failed data for node", i+1);
		sscanf(szline, "%*8d%*5d%g%g%g", &x, &y, &z);
		n.m_r0 = n.m_rt = vec3f(x,y,z);
	}

	// read the solid elements
	int n[8], nm;
	int ne = 0;
	for (i=0; i<m_nhel; ++i)
	{
		FEGenericElement& el = static_cast<FEGenericElement&>(m_pm->Element(ne++));

		if (get_line(szline) == 0) return errf("failed data for node", i+1);
		sscanf(szline, "%*8d%5d%8d%8d%8d%8d%8d%8d%8d%8d", &nm, n,n+1,n+2,n+3,n+4,n+5,n+6,n+7);

		// since arrays in C are zero-based we need do decrease the nodes and material number
		if ((n[7]==n[3]) && (n[6]==n[3]) && (n[5]==n[3]) && (n[4]==n[3]))
		{
			el.SetType(FE_TET4);
			el.m_node[0] = n[0] - 1;
			el.m_node[1] = n[1] - 1;
			el.m_node[2] = n[2] - 1;
			el.m_node[3] = n[3] - 1;
		}
		else if ((n[7]==n[5]) && (n[6]==n[5]))
		{
			// note the strange mapping. This is because NIKE's wedge elements use a
			// different node numbering.
			el.SetType(FE_PENTA6);
			el.m_node[0] = n[0]-1;
			el.m_node[1] = n[4]-1;
			el.m_node[2] = n[1]-1;
			el.m_node[3] = n[3]-1;
			el.m_node[4] = n[6]-1;
			el.m_node[5] = n[2]-1;
		}
		else
		{
			el.SetType(FE_HEX8);
			for (j=0; j<8; j++) el.m_node[j] = n[j] - 1;
		}
		el.m_MatID = nm-1;
	}

	// read the beam element
	for (i=0; i<m_nbel; ++i)
		if (get_line(szline) == 0) return errf("failed data for beam element", i+1);

	// read shell elemets
	for (i=0; i<m_nsel; ++i)
	{
		FEGenericElement& el = static_cast<FEGenericElement&>(m_pm->Element(ne++));

		if (get_line(szline) == 0) return errf("failed data for shell element", i+1);
		sscanf(szline, "%*8d%5d%8d%8d%8d%8d", &nm, n,n+1,n+2,n+3);
		el.m_node[0] = n[0] - 1;
		el.m_node[1] = n[1] - 1;
		el.m_node[2] = n[2] - 1;
		el.m_node[3] = n[3] - 1;
		el.m_MatID = nm-1;

		if (n[3] == n[2])
			el.SetType(FE_TRI3);
		else
			el.SetType(FE_QUAD4);

		if (get_line(szline) == 0) return errf("failed data for shell element", i+1);
	}
	
	return true;
}
