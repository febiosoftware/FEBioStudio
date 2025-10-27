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
#include "FEWeldModifier.h"
#include <MeshLib/FSMeshBuilder.h>
#include <MeshLib/FSSurfaceMesh.h>
using namespace std;

//! constructor
FEWeldNodes::FEWeldNodes() : FEModifier("Weld nodes")
{ 
	AddDoubleParam(0.0, "threshold", "threshold");
}

void FEWeldNodes::SetThreshold(double d)
{
	SetFloatValue(0, d);
}

//-----------------------------------------------------------------------------
//! Apply the weld modifier to the mesh and return a new mesh with the nodes
//! welded. Note that this modifier only works on surface meshes (quad/tri).
//! This modifier welds only the selected nodes.
FSMesh* FEWeldNodes::Apply(FSMesh* pm)
{
	// create a copy of the mesh
	FSMesh* pnm = new FSMesh(*pm);
	FSMesh& m = *pnm;

	// weld the nodes and figure out the new node numbering
	UpdateNodes(pnm);

	// now we modify the element node numbers
	UpdateElements(pnm);

	FSMeshBuilder meshBuilder(*pnm);

	if (meshBuilder.DeleteTaggedElements(1) == 0)
	{
		pnm->RebuildMesh();
		meshBuilder.RemoveIsolatedNodes();
	}

	return pnm;
}

//-----------------------------------------------------------------------------
void FEWeldNodes::UpdateNodes(FSMesh* pm)
{
	FSMesh& m = *pm;

	// find out how many nodes are selected
	// and put them in a list
	int nodes = m.Nodes();
	vector<int> sel; sel.reserve(nodes);
	for (int i=0; i<nodes; ++i)
	{
		FSNode& ni = m.Node(i);
		if (ni.IsSelected()) sel.push_back(i);
	}

	// create the nodal reorder list
	m_order.resize(nodes);
	for (int i=0; i<nodes; ++i) m_order[i] = i;

	// sqr distance treshold
	double threshold = GetFloatValue(0);
	double eps = threshold*threshold;

	// loop over the selected nodes
	int n = (int) sel.size();
	for (int i=0; i<n-1; ++i)
		for (int j=i+1; j<n; ++j)
		{
			int ni = m_order[sel[i]];
			int nj = m_order[sel[j]];

			if (ni != nj)
			{
				// calculate distance between nodes
				vec3d& ri = m.Node(ni).r;
				vec3d& rj = m.Node(nj).r;

				double d = (ri.x-rj.x)*(ri.x-rj.x)+(ri.y-rj.y)*(ri.y-rj.y)+(ri.z-rj.z)*(ri.z-rj.z);
				if (d <= eps)
				{
					// weld nodes ni and nj
					m_order[sel[j]] = ni;

					// move nodes to the average of the two
					ri = (ri+rj)*0.5;
				}
			}
		}

	// reassign node numbers
	for (int i=0; i<nodes; ++i)
	{
		if (m_order[i] != i) m_order[i] = m_order[ m_order[i] ];
	}
}

//-----------------------------------------------------------------------------
void FEWeldNodes::UpdateElements(FSMesh* pnm)
{
	FSMesh& m = *pnm;

	int elems = m.Elements();
	for (int i=0; i<elems; ++i)
	{
		FSElement& el = m.Element(i);
		int ne = el.Nodes();
		int* en = el.m_node;
		el.m_ntag = 0;

		// reassign node numbers
		for (int j=0; j<ne; ++j) en[j] = m_order[en[j]];

		// see if any nodes are now duplicated
		if (ne == 3)
		{
			assert(el.IsType(FE_TRI3));
			if ((en[0]==en[1])||(en[0]==en[2])||(en[1]==en[2]))
			{
				// mark for deletion
				el.m_ntag = 1;
			}
		}
		else if (ne == 4)
		{
			switch(el.Type())
			{
			case FE_QUAD4:
				if ((en[0]==en[1])||(en[0]==en[2])||(en[0]==en[3])||
						(en[1]==en[2])||(en[1]==en[3])||(en[2]==en[3]))
				{
					int n = 1;
					for (int j=1; j<4; ++j)
					{
						bool b = true;
						for (int k=0; k<n; ++k)
							if (en[j] == en[k]) { b = false; break; }

						if (b) { en[n++] = en[j]; }
					}

					if (n==3)
					{
						// downgrade to triangle
						el.SetType(FE_TRI3);
					}
					else if (n<3)
					{
						// mark for deletion
						el.m_ntag = 1;
					}
				}
				break;
			case FE_TET4:
				if ((en[0]==en[1])||(en[0]==en[2])||(en[0]==en[3])||
						(en[1]==en[2])||(en[1]==en[3])||(en[2]==en[3]))
				{
					// mark for deletion
					el.m_ntag = 1;
				}
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEWeldNodes::UpdateFaces(FSMesh* pnm)
{
	FSMesh& m = *pnm;

	int faces = m.Faces();
	for (int i=0; i<faces; ++i)
	{
		FSFace& face = m.Face(i);
		int nf = face.Nodes();
		int* fn = face.n;
		face.m_ntag = 0;

		// reassign node numbers
		for (int j=0; j<nf; ++j) fn[j] = m_order[fn[j]];

		// see if any nodes are now duplicated
		if (nf == 3)
		{
			if ((fn[0]==fn[1])||(fn[0]==fn[2])||(fn[1]==fn[2]))
			{
				// mark for deletion
				face.m_ntag = 1;
			}
		}
		else if (nf == 4)
		{
			if ((fn[0]==fn[1])||(fn[0]==fn[2])||(fn[0]==fn[3])||
				(fn[1]==fn[2])||(fn[1]==fn[3])||(fn[2]==fn[3]))
			{
				int n = 1;
				for (int j=1; j<4; ++j)
				{
					bool b = true;
					for (int k=0; k<n; ++k)
						if (fn[j] == fn[k]) { b = false; break; }

					if (b) { fn[n++] = fn[j]; }
				}

				if (n==3)
				{
					// downgrade to triangle
					face.SetType(FE_FACE_TRI3);
				}
				else if (n<3)
				{
					// mark for deletion
					face.m_ntag = 1;
				}
			}
		}
	}

	// find all duplicate faces
	vector<int> l;
	m.FindDuplicateFaces(l);

	// and mark them for deletion as well
	for (int i=0; i<(int)l.size(); ++i) m.Face(l[i]).m_ntag = 1;
}

//-----------------------------------------------------------------------------
void FEWeldNodes::UpdateEdges(FSMesh* pm)
{
	FSMesh& m = *pm;
	int edges = m.Edges();
	for (int i=0; i<edges; ++i)
	{
		FSEdge& edge = m.Edge(i);
		edge.m_ntag = 0;
		int* en = edge.n;

		// reassign node numbers
		en[0] = m_order[en[0]];
		en[1] = m_order[en[1]];

		// see if we need to delete this edge
		if (en[0] == en[1]) edge.m_ntag = 1;
	}

	// find all duplicate edges
	vector<int> l;
	m.FindDuplicateEdges(l);

	// and mark them for deletion as well
	for (int i=0; i<(int)l.size(); ++i) m.Edge(l[i]).m_ntag = 1;
}


//======================================================================================

//! constructor
FEWeldSurfaceNodes::FEWeldSurfaceNodes() : FESurfaceModifier("Weld nodes")
{
	AddDoubleParam(0.0, "threshold", "threshold");
}

void FEWeldSurfaceNodes::SetThreshold(double d)
{
	SetFloatValue(0, d);
}

//-----------------------------------------------------------------------------
//! Apply the weld modifier to the mesh and return a new mesh with the nodes
//! welded. Note that this modifier only works on surface meshes (quad/tri).
//! This modifier welds only the selected nodes.
FSSurfaceMesh* FEWeldSurfaceNodes::Apply(FSSurfaceMesh* pm)
{
	// create a copy of the mesh
	FSSurfaceMesh* pnm = new FSSurfaceMesh(*pm);
	FSSurfaceMesh& m = *pnm;

	// weld the nodes and figure out the new node numbering
	UpdateNodes(pnm);

	// now we modify the element node numbers
	UpdateFaces(pnm);

	pnm->BuildMesh();

	return pnm;
}

//-----------------------------------------------------------------------------
void FEWeldSurfaceNodes::UpdateNodes(FSSurfaceMesh* pm)
{
	FSSurfaceMesh& m = *pm;

	// find out how many nodes are selected
	// and put them in a list
	int nodes = m.Nodes();
	vector<int> sel; sel.reserve(nodes);
	for (int i = 0; i < nodes; ++i)
	{
		FSNode& ni = m.Node(i);
		if (ni.IsSelected()) sel.push_back(i);
	}
	if (sel.empty())
	{
		for (int i = 0; i < nodes; ++i) sel.push_back(i);
	}

	// create the nodal reorder list
	m_order.resize(nodes);
	for (int i = 0; i < nodes; ++i) m_order[i] = i;

	// sqr distance treshold
	double threshold = GetFloatValue(0);
	double eps = threshold * threshold;

	// loop over the selected nodes
	int n = (int)sel.size();
	for (int i = 0; i < n - 1; ++i)
	{
		int ni = m_order[sel[i]];
		vec3d& ri = m.Node(ni).r;

		// find the closest node
		int jmin = -1;
		double dmin = 0.0;
		for (int j = i + 1; j < n; ++j)
		{
			int nj = m_order[sel[j]];

			if (ni != nj)
			{
				// calculate distance between nodes
				vec3d& rj = m.Node(nj).r;

				double d = (ri.x - rj.x)*(ri.x - rj.x) + (ri.y - rj.y)*(ri.y - rj.y) + (ri.z - rj.z)*(ri.z - rj.z);
				if ((d <= eps) && ((d < dmin) || jmin == -1))
				{
					jmin = j;
					dmin = d;
				}
			}
		}

		if (jmin != -1)
		{
			int nj = m_order[sel[jmin]];

			vec3d& ri = m.Node(ni).r;
			vec3d& rj = m.Node(nj).r;

			// weld nodes ni and nj
			m_order[sel[jmin]] = ni;

			// move nodes to the average of the two
			ri = (ri + rj)*0.5;
		}
	}

	// reassign node numbers
	for (int i = 0; i < nodes; ++i)
	{
		if (m_order[i] != i) m_order[i] = m_order[m_order[i]];
	}
}

//-----------------------------------------------------------------------------
void FEWeldSurfaceNodes::UpdateFaces(FSSurfaceMesh* pnm)
{
	FSSurfaceMesh& m = *pnm;

	int faces = m.Faces();
	for (int i = 0; i < faces; ++i)
	{
		FSFace& face = m.Face(i);
		int nf = face.Nodes();
		int* fn = face.n;
		face.m_ntag = 0;

		// reassign node numbers
		for (int j = 0; j < nf; ++j) fn[j] = m_order[fn[j]];

		// see if any nodes are now duplicated
		if (nf == 3)
		{
			if ((fn[0] == fn[1]) || (fn[0] == fn[2]) || (fn[1] == fn[2]))
			{
				// mark for deletion
				face.m_ntag = 1;
			}
		}
		else if (nf == 4)
		{
			if ((fn[0] == fn[1]) || (fn[0] == fn[2]) || (fn[0] == fn[3]) ||
				(fn[1] == fn[2]) || (fn[1] == fn[3]) || (fn[2] == fn[3]))
			{
				int n = 1;
				for (int j = 1; j < 4; ++j)
				{
					bool b = true;
					for (int k = 0; k < n; ++k)
						if (fn[j] == fn[k]) { b = false; break; }

					if (b) { fn[n++] = fn[j]; }
				}

				if (n == 3)
				{
					// downgrade to triangle
					face.SetType(FE_FACE_TRI3);
				}
				else if (n < 3)
				{
					// mark for deletion
					face.m_ntag = 1;
				}
			}
		}
	}

	pnm->RemoveIsolatedNodes();
}
