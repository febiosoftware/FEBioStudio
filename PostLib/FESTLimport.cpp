#include "stdafx.h"
#include "FESTLimport.h"
#include "FEModel.h"
#include <ctype.h>
#include "color.h"

using namespace Post;

FESTLimport::FESTLimport(void) : FEFileReader("STL")
{

}

FESTLimport::~FESTLimport(void)
{
}

bool FESTLimport::read_line(char* szline, const char* sz)
{
	m_nline++;

	int l = strlen(sz);
	if (fgets(szline, 255, m_fp) == 0) return false;

	// remove leading white space
	int n = 0;
	while (isspace(szline[n])) ++n;
	if (n != 0)
	{
		char* ch = szline + n;
		n = 0;
		while (*ch) szline[n++] = *ch++;
	}
	
	if (strncmp(szline, sz, l) != 0) return false;

	return true;
}

bool FESTLimport::Load(FEModel& fem, const char* szfile)
{
	fem.Clear();
	m_pfem = &fem;
	m_nline = 0;

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed opening file %s.", szfile);

	// read the first line
	char szline[256] = {0};
	if (read_line(szline, "solid ") == false) return errf("First line must be solid definition.");

	// clear the list
	m_Face.clear();

	// read all the triangles
	FACET face;
	do
	{
		// read the facet line
		if (read_line(szline, "facet normal") == false)
		{
			// check for the endsolid tag
			if (strncmp(szline, "endsolid", 8) == 0) break; 
			else return errf("Error encountered at line %d", m_nline);
		}

		// read the outer loop line
		if (read_line(szline, "outer loop") == false) return errf("Error encountered at line %d", m_nline);

		// read the vertex data
		float x, y, z;
		for (int i=0; i<3; ++i)
		{
			if (read_line(szline, "vertex ") == false) return errf("Error encountered at line %d", m_nline);
			sscanf(szline, "vertex %g%g%g", &x, &y, &z);

			face.r[i].x = x;
			face.r[i].y = y;
			face.r[i].z = z;
		}

		// read the endloop tag
		if (read_line(szline, "endloop") == false) return errf("Error encountered at line %d", m_nline);

		// read the endfacet tag
		if (read_line(szline, "endfacet") == false) return errf("Error encountered at line %d", m_nline);

		// add the facet to the list
		m_Face.push_back(face);
	}
	while (1);

	// close the file
	Close();

	// build the nodes
	build_mesh();

	return true;
}

void FESTLimport::build_mesh()
{
	int i;

	// add one material to the scene
	FEMaterial mat;
	m_pfem->AddMaterial(mat);

	// reserve space for nodes
	int NF = m_Face.size();
	m_Node.reserve(NF*3);

	// create the nodes
	list<FACET>::iterator pf = m_Face.begin();
	for (int i=0; i<NF; ++i, ++pf)
	{
		FACET& f = *pf;
		for (int j=0; j<3; ++j) f.n[j] = find_node(f.r[j]);
	}
	int NN = m_Node.size();

	// create the mesh
	FETriMesh* pm = new FETriMesh();
	pm->Create(NN, NF);
	m_pfem->AddMesh(pm);

	// create nodes
	for (i=0; i<NN; ++i)
	{
		FENode& n = pm->Node(i);
		vec3f& ri = m_Node[i];
		n.m_r0 = n.m_rt = ri;
	}

	// create elements
	list<FACET>::iterator is = m_Face.begin();
	for (i=0; i<NF; ++i, ++is)
	{
		FACET& f = *is;
		FEElement& e = pm->Element(i);
		e.m_node[0] = f.n[0];
		e.m_node[1] = f.n[1];
		e.m_node[2] = f.n[2];
		e.m_MatID = 0;
	}

	// update the mesh
	pm->Update();
	m_pfem->UpdateBoundingBox();

	// we need a single state
	FEState* ps = new FEState(0.f, m_pfem, m_pfem->GetFEMesh(0));
	m_pfem->AddState(ps);
}

int FESTLimport::find_node(vec3f& r, const double eps)
{
	int N = m_Node.size();
	for (int i=0; i<N; ++i)
	{
		vec3f& ri = m_Node[i];
		if ((ri - r)*(ri - r) < eps) return i;
	}

	m_Node.push_back(r);
	return m_Node.size() - 1;
}
