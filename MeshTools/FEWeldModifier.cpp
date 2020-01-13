#include "stdafx.h"
#include "FEWeldModifier.h"

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
FEMesh* FEWeldNodes::Apply(FEMesh* pm)
{
	// create a copy of the mesh
	FEMesh* pnm = new FEMesh(*pm);
	FEMesh& m = *pnm;

	// weld the nodes and figure out the new node numbering
	UpdateNodes(pnm);

	// now we modify the element node numbers
	UpdateElements(pnm);

	pnm->DeleteTaggedElements(1);

	pnm->RemoveIsolatedNodes();

	pnm->RebuildMesh();

	return pnm;
}

//-----------------------------------------------------------------------------
void FEWeldNodes::UpdateNodes(FEMesh* pm)
{
	FEMesh& m = *pm;

	// find out how many nodes are selected
	// and put them in a list
	int nodes = m.Nodes();
	vector<int> sel; sel.reserve(nodes);
	for (int i=0; i<nodes; ++i)
	{
		FENode& ni = m.Node(i);
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
void FEWeldNodes::UpdateElements(FEMesh* pnm)
{
	FEMesh& m = *pnm;

	int elems = m.Elements();
	for (int i=0; i<elems; ++i)
	{
		FEElement& el = m.Element(i);
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
void FEWeldNodes::UpdateFaces(FEMesh* pnm)
{
	FEMesh& m = *pnm;

	int faces = m.Faces();
	for (int i=0; i<faces; ++i)
	{
		FEFace& face = m.Face(i);
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
void FEWeldNodes::UpdateEdges(FEMesh* pm)
{
	FEMesh& m = *pm;
	int edges = m.Edges();
	for (int i=0; i<edges; ++i)
	{
		FEEdge& edge = m.Edge(i);
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
