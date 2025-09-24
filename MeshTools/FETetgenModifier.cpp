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

#include "stdafx.h"
#include "FETetGenModifier.h"
#include <MeshLib/MeshMetrics.h>
#include <GeomLib/GObject.h>
using namespace std;

#undef PI

#ifdef TETLIBRARY
	#include <tetgen.h>
#endif

bool coplanar(vec3d r[4], double tol)
{
	// TODO: There seems to be a bug in this function that causes tetgen to crash.
	//       Therefore, for now I just return false
/*
	mat3d A1, A2, A3, A4;

	A1[0][0] = r[1].x; A1[0][1] = r[2].x; A1[0][2] = r[3].x;
	A1[1][0] = r[1].y; A1[1][1] = r[2].y; A1[1][2] = r[3].y;
	A1[2][0] = r[1].z; A1[2][1] = r[2].z; A1[2][2] = r[3].z;

	A2[0][0] = r[0].x; A2[0][1] = r[2].x; A2[0][2] = r[3].x;
	A2[1][0] = r[0].y; A2[1][1] = r[2].y; A2[1][2] = r[3].y;
	A2[2][0] = r[0].z; A2[2][1] = r[2].z; A2[2][2] = r[3].z;

	A3[0][0] = r[0].x; A3[0][1] = r[1].x; A3[0][2] = r[3].x;
	A3[1][0] = r[0].y; A3[1][1] = r[1].y; A3[1][2] = r[3].y;
	A3[2][0] = r[0].z; A3[2][1] = r[1].z; A3[2][2] = r[3].z;

	A4[0][0] = r[0].x; A4[0][1] = r[1].x; A4[0][2] = r[2].x;
	A4[1][0] = r[0].y; A4[1][1] = r[1].y; A4[1][2] = r[2].y;
	A4[2][0] = r[0].z; A4[2][1] = r[1].z; A4[2][2] = r[2].z;

	double V = (A1.det() - A2.det() + A3.det() - A4.det())/6.0;
	if (V == 0) return true;

	double l1 = (r[0] - r[1]).Length();
	double l2 = (r[1] - r[2]).Length();
	double l3 = (r[2] - r[0]).Length();
	double l4 = (r[0] - r[3]).Length();
	double l5 = (r[1] - r[3]).Length();
	double l6 = (r[2] - r[3]).Length();

	double l = (l1 + l2 + l3 + l4 + l5 + l6)/6.0;

	double q = V / (l*l*l);

	if (q < tol) return true;
*/
	return false;
}

//-----------------------------------------------------------------------------
FETetGenModifier::FETetGenModifier() : FEModifier("TetGen Mesher")
{
	AddDoubleParam(2.0, "Q", "Minimum radius-edge ratio");
	AddDoubleParam(0.0, "H", "Element size");
	AddBoolParam(false, "split", "split facets");
	AddIntParam(0, "feather", "feather");

	m_tol = 0.0;
	m_bremesh = false;
}

//-----------------------------------------------------------------------------
FSMesh* FETetGenModifier::Apply(FSMesh* pm)
{
	FSMesh* pmnew = 0;

	try {
		if (m_bremesh)
			pmnew = RefineMesh(pm);
		else
			pmnew = CreateMesh(pm);
	}
	catch (...)
	{
		FEModifier::SetError("An error has occurred in Tetgen");
		return 0;
	}

	return pmnew;
}

//-----------------------------------------------------------------------------
FSMesh* FETetGenModifier::Apply(FSGroup* pg)
{
	FSElemSet* part = dynamic_cast<FSElemSet*>(pg);
	if (part)
		m_bremesh = true;
	else
		m_bremesh = false;

	return Apply(pg->GetMesh());
}

//-----------------------------------------------------------------------------
FSMesh* FETetGenModifier::CreateMesh(FSMesh* pm)
{
#ifdef TETLIBRARY

	// make sure that the surface is triangular and is closed
	int NF = pm->Faces();
	for (int i=0; i<NF; ++i)
	{
		FSFace& face = pm->Face(i);
		if (face.Type() != FE_FACE_TRI3)
		{
			FEModifier::SetError("Invalid mesh type.");
			return 0;
		}

		// TODO: This test fails if the mesh has internal surfaces (for which faces don't 
		//       have neighbors. I need to modify this so that I only check exterior faces.
/*		if ((face.m_nbr[0] == -1) || (face.m_nbr[1] == -1) || (face.m_nbr[2] == -1))
		{
			FEModifier::SetError("This mesh is not closed.");
			return 0;
		}
*/	}

	// allocate tetgen structures
	tetgenio in, out;
	in.initialize();
	out.initialize();

	// build the tetgen structure
	if (build_tetgen_plc(pm, in) == false) return 0;

	// convert element size to volume
	double h = GetFloatValue(1);
	double a = h*h*h;

	// set the parameters
	double q = GetFloatValue(0);
	bool bsplit = GetBoolValue(2);
	char sz[64] = {0};
	char* ch = sz;
	sprintf(ch, "p"); ++ch;
	if (q   > 0) sprintf(ch, "q%lg", q); ch += strlen(ch);
	if (h   > 0) sprintf(ch, "a%lg",   a); ch += strlen(ch);
	if (m_tol > 0) sprintf(ch, "T%lg", m_tol); ch += strlen(ch);
	if (bsplit == false) sprintf(ch, "Y"); ++ch;

	// create a tet mesh
	try
	{
		tetrahedralize(sz, &in, &out);
	}
	catch (int x)
	{
		switch (x) {
		case 1: // Out of memory.
			SetError("Out of memory.");
			break;
		case 2: // Encounter an internal error.
			SetError("Internal error");
			break;
		case 3:
			SetError("A self-intersection was detected.");
//			printf("Hint: use -d option to detect all self-intersections.\n");
			break;
		case 4:
			SetError("A very small input feature size was detected.");
//			printf("Hint: use -T option to set a smaller tolerance.\n");
			break;
		case 5:
			SetError("Two very close input facets were detected.");
//			printf("Hint: use -Y option to avoid adding Steiner points in boundary.\n");
			break;
		case 10:
			SetError("An input error was detected.");
			break;
		default:
			SetError("Unknown Tetgen error (%d)", x);
		}
		return 0;
	}
	catch (...)
	{
		SetError("Unknown exception raised while executing Tetgen.");
	}

	// create a new mesh
	FSMesh* pmesh = new FSMesh;
	int nodes = out.numberofpoints;
	int elems = out.numberoftetrahedra;
	int faces = out.numberoftrifaces;
	int edges = 0;
	for (int i=0; i<out.numberofedges; ++i) if (out.edgemarkerlist[i] > 1) edges++;

	// make sure tetgen did something
	if ((elems == 0)||(nodes == 0)||(faces == 0)) return 0;

	// create a new mesh
	pmesh->Create(nodes, elems, faces, edges);

	// copy nodes
	for (int i=0; i<nodes; ++i)
	{
		vec3d& r = pmesh->Node(i).r;
		r.x = out.pointlist[3*i  ];
		r.y = out.pointlist[3*i+1];
		r.z = out.pointlist[3*i+2];
	}

	assert(out.numberofcorners == 4);

	// copy elements
	for (int i=0; i<elems; ++i)
	{
		FSElement& el = pmesh->Element(i);
		el.SetType(FE_TET4);
		el.m_node[0] = out.tetrahedronlist[4*i  ];
		el.m_node[1] = out.tetrahedronlist[4*i+1];
		el.m_node[2] = out.tetrahedronlist[4*i+2];
		el.m_node[3] = out.tetrahedronlist[4*i+3];
		el.m_gid = 0;
	}

	// copy faces
	for (int i=0; i<faces; ++i)
	{
		FSFace& f = pmesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.n[0] = out.trifacelist[3*i+2];
		f.n[1] = out.trifacelist[3*i+1];
		f.n[2] = out.trifacelist[3*i  ];
		f.n[3] = f.n[2];
		f.m_gid  = out.trifacemarkerlist[i];
	}

	// copy edges		out.trifacemarkerlist[0]	1	int

	int n = 0;
	for (int i=0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1)
		{
			FSEdge& e = pmesh->Edge(n++);
			e.SetType(FE_EDGE2);
			e.n[0] = out.edgelist[2*i  ]; assert(e.n[0] >= 0);
			e.n[1] = out.edgelist[2*i+1]; assert(e.n[1] >= 0);
			e.m_gid = out.edgemarkerlist[i]-2;
		}
	}
	assert(n==pmesh->Edges());

	// update the element neighbours
	pmesh->BuildMesh();

	// update faces
	pmesh->SmoothByPartition();

	// associate the FE nodes with the GNodes
	GObject* po = pm->GetGObject();
	if (po)
	{
		double R2 = pmesh->GetBoundingBox().GetMaxExtent();
		if (R2 == 0) R2 = 1.0; else R2 *= R2;
		for (int i = 0; i < pmesh->Nodes(); ++i)
		{
			FSNode& node = pmesh->Node(i);
			vec3d& ri = node.r;
			node.m_gid = -1;
			for (int j = 0; j < po->Nodes(); ++j)
			{
				GNode& gn = *po->Node(j);
				vec3d& rj = gn.LocalPosition();
				double L2 = (ri - rj).SqrLength();
				if (L2 / R2 < 1e-6)
				{
					node.m_gid = j;
					break;
				}
			}
		}
	}
	return pmesh;
#else 
	return 0;
#endif // TETLIBRARY
}

//-----------------------------------------------------------------------------
FSMesh* FETetGenModifier::RefineMesh(FSMesh* pm)
{
#ifdef TETLIBRARY
	if (pm->IsType(FE_TET4)==false)
	{
		FEModifier::SetError("This object is not a TET4 mesh");
		return 0;
	}

	// allocate tetgen structures
	tetgenio in, out;
	in.initialize();
	out.initialize();

	int NF = pm->Faces();
	vector<int> gid(NF), sid(NF);
	for (int i=0; i<NF; ++i) 
	{
		gid[i] = pm->Face(i).m_gid;
		sid[i] = pm->Face(i).m_sid;
	}

	// build the tetgen structure
	if (build_tetgen_remesh(pm, in) == false) return 0;

	// convert element size to volume
	double h = GetFloatValue(1);
	double a = h*h*h;

	// set the parameters
	double q = GetFloatValue(0);
	bool bsplit = GetBoolValue(2);

	char sz[64] = { 0 };
	char* ch = sz;
	sprintf(ch, "r");  ++ch;
	if (q   > 0) sprintf(ch, "q%lg", q); ch += strlen(ch);
	if (h   > 0) sprintf(ch, "a"); ch += strlen(ch);
	if (m_tol > 0) sprintf(ch, "T%lg", m_tol); ch += strlen(ch);
	if (bsplit == false) sprintf(ch, "Y"); ++ch;

	// create a tet mesh
	tetrahedralize(sz, &in, &out);

	// create a new mesh
	FSMesh* pmesh = new FSMesh;
	int nodes = out.numberofpoints;
	int elems = out.numberoftetrahedra;
	int faces = out.numberoftrifaces;

	// count the number of edges with a marker > 1
	int edges = 0;
	for (int i=0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1) edges++;
	}

	// allocate the mesh
	pmesh->Create(nodes, elems, faces, edges);

	// copy nodes
	for (int i=0; i<nodes; ++i)
	{
		vec3d& r = pmesh->Node(i).r;
		r.x = out.pointlist[3*i  ];
		r.y = out.pointlist[3*i+1];
		r.z = out.pointlist[3*i+2];
	}

	assert(out.numberofcorners == 4);

	// copy elements
	for (int i=0; i<elems; ++i)
	{
		FSElement& el = pmesh->Element(i);
		el.SetType(FE_TET4);
		el.m_node[0] = out.tetrahedronlist[4*i  ];
		el.m_node[1] = out.tetrahedronlist[4*i+1];
		el.m_node[2] = out.tetrahedronlist[4*i+2];
		el.m_node[3] = out.tetrahedronlist[4*i+3];
		el.m_gid = 0;
	}

	// copy faces
	for (int i=0; i<faces; ++i)
	{
		FSFace& f = pmesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.n[0] = out.trifacelist[3 * i + 2];
		f.n[1] = out.trifacelist[3*i+1];
		f.n[2] = out.trifacelist[3*i  ];
		f.n[3] = f.n[2];

		int marker = out.trifacemarkerlist[i];
		if ((marker >= 0) && (marker < gid.size()))
		{
			f.m_gid = gid[marker];
			f.m_sid = sid[marker];
		}
		else
		{
			f.m_gid = 0;
			f.m_sid = 0;
		}
	}
/*
	// copy edges
	int n = 0;
	for (int i=0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1)
		{
			FSEdge& e = pmesh->Edge(n++);
			e.SetType(FE_EDGE2);
			e.n[0] = out.edgelist[2*i  ];
			e.n[1] = out.edgelist[2*i+1];
			e.m_gid = out.edgemarkerlist[i]-2;
		}
	}

	// update the element neighbours
	pmesh->UpdateElementNeighbors();

	// update faces
	pmesh->UpdateFaces();
	pmesh->UpdateNormals();

	pmesh->UpdateEdgeNeighbors();

	// update the mesh
	pmesh->MarkExteriorNodes();
	pmesh->UpdateBox();

	// associate the FE nodes with the GNodes
	GObject* po = pm->GetGObject();
	double R2 = pmesh->GetBoundingBox().GetMaxExtent();
	if (R2 == 0) R2 = 1.0; else R2 *= R2;	
	for (int i=0; i<pmesh->Nodes(); ++i)
	{
		FSNode& node = pmesh->Node(i);
		vec3d& ri = node.r;
		node.m_gid = -1;
		for (int j=0; j<po->Nodes(); ++j)
		{
			GNode& gn = *po->Node(j);
			vec3d& rj = gn.LocalPosition();
			double L2 = (ri - rj).SqrLength();
			if (L2/R2 < 1e-6) 
			{
				node.m_gid = j;
				break;
			}
		}
	}
*/
	pmesh->RebuildMesh();

	return pmesh;
#else 
	return 0;
#endif // TETLIBRARY
}

//-----------------------------------------------------------------------------
bool FETetGenModifier::build_tetgen_plc(FSMesh* pm, tetgenio& in)
{
#ifdef TETLIBRARY
	int i, j, n;

	// first we need to tag all nodes that define the boundary of the mesh
	for (i=0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;
	for (i=0; i<pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		for (j=0; j<f.Nodes(); ++j)
		{
			pm->Node(f.n[j]).m_ntag = 1;
		}
	}

	// count the nodes
	int nodes = 0;
	for (i=0; i<pm->Nodes(); ++i) if (pm->Node(i).m_ntag == 1) ++nodes;

	// all indices start from 0
	in.firstnumber = 0;

	// allocate nodes
	in.numberofpoints = nodes;
	in.pointlist = new REAL[3*nodes];
	vector<int> tag(pm->Nodes());
	for (i=0, n=0; i<pm->Nodes(); ++i) 
	{
		if (pm->Node(i).m_ntag == 1)
		{
			vec3d& r = pm->Node(i).r;
			in.pointlist[3*n  ] = r.x;
			in.pointlist[3*n+1] = r.y;
			in.pointlist[3*n+2] = r.z;
			tag[i] = n++;
		}
		else tag[i] = -1;
	}

	// count the faces
	int faces = 0;
	for (i=0; i<pm->Faces(); ++i)
	{
		FSFace& f = pm->Face(i);
		if (f.Type() == FE_FACE_TRI3) { ++faces; f.m_ntag = 1; }
		else 
		{
			// see if this face is coplanar or if we need to split it in triangles
			vec3d r[4];
			r[0] = pm->Node(f.n[0]).r;
			r[1] = pm->Node(f.n[1]).r;
			r[2] = pm->Node(f.n[2]).r;
			r[3] = pm->Node(f.n[3]).r;

			if (coplanar(r, m_tol)) 
			{
				faces++;
				f.m_ntag = 1;
			}
			else
			{
				faces += 2;
				f.m_ntag = 2;
			}
		}
	}

	// allocate faces
	in.numberoffacets = faces;
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];
	for (i=0, n = 0; i<pm->Faces(); ++i)
	{
		tetgenio::facet* pf; 
		tetgenio::polygon* pp;
		FSFace& f = pm->Face(i);
		if (f.Type() == FE_FACE_TRI3)
		{
			pf = &in.facetlist[n];
			pf->numberofpolygons = 1;
			pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
			pf->numberofholes = 0;
			pf->holelist = 0;

			pp = &(pf->polygonlist[0]);
			pp->numberofvertices = 3;
			pp->vertexlist = new int[pp->numberofvertices];
			pp->vertexlist[0] = tag[f.n[0]];
			pp->vertexlist[1] = tag[f.n[1]];
			pp->vertexlist[2] = tag[f.n[2]];

			in.facetmarkerlist[n++] = f.m_gid;
		}
		else
		{
			if (f.m_ntag == 1)
			{
				pf = &in.facetlist[n];
				pf->numberofpolygons = 1;
				pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
				pf->numberofholes = 0;
				pf->holelist = 0;

				pp = &(pf->polygonlist[0]);
				pp->numberofvertices = 4;
				pp->vertexlist = new int[pp->numberofvertices];
				pp->vertexlist[0] = tag[f.n[0]];
				pp->vertexlist[1] = tag[f.n[1]];
				pp->vertexlist[2] = tag[f.n[2]];
				pp->vertexlist[3] = tag[f.n[3]];

				in.facetmarkerlist[n++] = f.m_gid;
			}
			else
			{
				pf = &in.facetlist[n];
				pf->numberofpolygons = 1;
				pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
				pf->numberofholes = 0;
				pf->holelist = 0;

				pp = &(pf->polygonlist[0]);
				pp->numberofvertices = 3;
				pp->vertexlist = new int[pp->numberofvertices];
				pp->vertexlist[0] = tag[f.n[0]];
				pp->vertexlist[1] = tag[f.n[1]];
				pp->vertexlist[2] = tag[f.n[2]];

				in.facetmarkerlist[n++] = f.m_gid;

				pf = &in.facetlist[n];
				pf->numberofpolygons = 1;
				pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
				pf->numberofholes = 0;
				pf->holelist = 0;

				pp = &(pf->polygonlist[0]);
				pp->numberofvertices = 3;
				pp->vertexlist = new int[pp->numberofvertices];
				pp->vertexlist[0] = tag[f.n[2]];
				pp->vertexlist[1] = tag[f.n[3]];
				pp->vertexlist[2] = tag[f.n[0]];

				in.facetmarkerlist[n++] = f.m_gid;
			}
		}
	}

	// define the edges
	in.numberofedges = pm->Edges();
	in.edgelist = new int[2*in.numberofedges];
	in.edgemarkerlist = new int[in.numberofedges];
	for (int i=0; i<in.numberofedges; ++i)
	{
		FSEdge& e = pm->Edge(i);
		in.edgelist[2*i  ] = e.n[0];
		in.edgelist[2*i+1] = e.n[1];
		in.edgemarkerlist[i] = e.m_gid+2;
	}

	// holes
	if (m_hole.empty() == false)
	{
		const int nh = (const int) m_hole.size();
		in.numberofholes = nh;
		in.holelist = new double[3*nh];
		for (int i=0; i<nh; ++i)
		{
			const vec3d& ri = m_hole[i];
			in.holelist[0] = ri.x;
			in.holelist[1] = ri.y;
			in.holelist[2] = ri.z;
		}
	}

	return true;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
bool FETetGenModifier::build_tetgen_remesh(FSMesh* pm, tetgenio& in)
{
#ifdef TETLIBRARY
	// make sure this is a tetmesh
	for (int i=0; i<pm->Elements(); ++i)
	{
		FSElement& el = pm->Element(i);
		if (el.Type() != FE_TET4) return false;
	}

	// all indices start from 0
	in.firstnumber = 0;

	// allocate nodes
	int nodes = pm->Nodes();
	in.numberofpoints = nodes;
	in.pointlist = new REAL[3*nodes];
	for (int i=0; i<nodes; ++i) 
	{
		vec3d& r = pm->Node(i).r;
		in.pointlist[3*i  ] = r.x;
		in.pointlist[3*i+1] = r.y;
		in.pointlist[3*i+2] = r.z;
	}

	// build the element list
	int elems = pm->Elements();
	in.numberoftetrahedra = elems;
	in.tetrahedronlist = new int[elems*4];
	for (int i=0; i<elems; ++i)
	{
		FSElement& el = pm->Element(i);
		in.tetrahedronlist[4*i  ] = el.m_node[0];
		in.tetrahedronlist[4*i+1] = el.m_node[1];
		in.tetrahedronlist[4*i+2] = el.m_node[2];
		in.tetrahedronlist[4*i+3] = el.m_node[3];
	}

	// build the facet list
	int faces = pm->Faces();
	in.numberoftrifaces = faces;
	in.trifacelist = new int[faces*3];
	for (int i=0, n = 0; i<faces; ++i)
	{
		FSFace& f = pm->Face(i);
		assert(f.Type() == FE_FACE_TRI3);
		in.trifacelist[3*i  ] = f.n[0];
		in.trifacelist[3*i+1] = f.n[1];
		in.trifacelist[3*i+2] = f.n[2];
	}

	// set the facet markers
	in.trifacemarkerlist = new int[faces];
	for (int i=0; i<faces; ++i) in.trifacemarkerlist[i] = i;

	// build the facet constraint list
	in.numberoffacetconstraints = faces;
	in.facetconstraintlist = new double[2*faces];
	double h = GetFloatValue(1);
	double a = h*h;
	for (int i=0; i<faces; ++i)
	{
		FSFace& f = pm->Face(i);
		double A = FEMeshMetrics::SurfaceArea(*pm, f);
		FSElement& el = pm->Element(f.m_elem[0].eid);
		in.facetconstraintlist[2*i  ] = i;
		if (el.IsSelected() || f.IsSelected())
		{
			in.facetconstraintlist[2*i+1] = a;
		}
		else
		{
			in.facetconstraintlist[2*i+1] = 0;
		}
	}

	int nfeather = GetIntValue(3);
	if (nfeather > 0)
	{
		vector<double> area; area.assign(faces, 0.0);
		for (int i=0; i<faces; ++i)
		{
			FSFace& f = pm->Face(i);
			area[i] = FEMeshMetrics::SurfaceArea(*pm, f);
		}

		for (int i=0; i<faces; ++i)
		{
			FSFace& f = pm->Face(i);
			FSElement& el = pm->Element(f.m_elem[0].eid);
			if (el.IsSelected() || f.IsSelected()) f.m_ntag = 1; else f.m_ntag = 0;
		}

		for (int n=0; n<nfeather; ++n)
		{
			double w = (n+1.0)/(nfeather+1.0); 
			w *= w;
			for (int i=0; i<faces; ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.m_ntag == 1)
				{
					int nf = 3; assert(f.Nodes() == 3);
					for (int j=0; j<nf; ++j)
					{
						FSFace* f2 = pm->FacePtr(f.m_nbr[j]);
						if (f2 && (f2->m_ntag == 0)) 
						{
							in.facetconstraintlist[2*f.m_nbr[j]+1] = a*(1.0-w) + w*area[f.m_nbr[j]];
							f2->m_ntag = 2; 
						}
					}
				}
			}
			for (int i=0; i<faces; ++i)
			{
				FSFace& f = pm->Face(i);
				if (f.m_ntag == 2) f.m_ntag = 1;
			}
		}
	}

	// build the element volume list
	if (h > 0)
	{
		double a = h*h*h;
		in.tetrahedronvolumelist = new REAL[elems];
		for (int i=0; i<elems; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.IsSelected()) in.tetrahedronvolumelist[i] = a; else in.tetrahedronvolumelist[i] = 0.0;
		}

		if (nfeather > 0)
		{
			vector<double> evol; evol.assign(elems, 0.0);
			for (int i=0; i<elems; ++i)
			{
				FSElement& el = pm->Element(i);
				evol[i] = FEMeshMetrics::ElementVolume(*pm, el);
			}

			for (int i=0; i<elems; ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.IsSelected()) el.m_ntag = 1; else el.m_ntag = 0;
			}

			for (int n=0; n<nfeather; ++n)
			{
				double w = (n+1.0)/(nfeather+1.0); 
				w *= w;
				for (int i=0; i<elems; ++i)
				{
					FSElement& el = pm->Element(i);
					if (el.m_ntag == 1)
					{
						int nf = el.Faces();
						for (int j=0; j<nf; ++j)
						{
							FSElement_* pe2 = pm->ElementPtr(el.m_nbr[j]);
							if (pe2 && (pe2->m_ntag == 0)) 
							{
								in.tetrahedronvolumelist[el.m_nbr[j]] = a*(1.0-w) + w*evol[el.m_nbr[j]];
								pe2->m_ntag = 2; 
							}
						}
					}
				}
				for (int i=0; i<elems; ++i)
				{
					FSElement& el = pm->Element(i);
					if (el.m_ntag == 2) el.m_ntag = 1;
				}
			}
		}
	}

	// define the edges
	in.numberofedges = pm->Edges();
	in.edgelist = new int[2*in.numberofedges];
	in.edgemarkerlist = new int[in.numberofedges];
	for (int i=0; i<in.numberofedges; ++i)
	{
		FSEdge& e = pm->Edge(i);
		in.edgelist[2*i  ] = e.n[0];
		in.edgelist[2*i+1] = e.n[1];
		in.edgemarkerlist[i] = e.m_gid+2;
	}

	return true;
#else
	return false;
#endif
}
