// FEMeshBase.cpp: implementation of the FEMeshBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMesh.h"
#include <stack>

//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
Post::FEMeshBase::FEMeshBase()
{
}

//-----------------------------------------------------------------------------
// Destructor
Post::FEMeshBase::~FEMeshBase()
{
	CleanUp();
}

//-----------------------------------------------------------------------------
// clean all depdendant structures
void Post::FEMeshBase::CleanUp()
{
	m_NEL.Clear();
	m_NFL.Clear();

	ClearDomains();
	ClearParts();
	ClearSurfaces();
	ClearNodeSets();
}

//-----------------------------------------------------------------------------
// clear everything
void Post::FEMeshBase::ClearAll()
{
	CleanUp();
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	ClearElements();
}

//-----------------------------------------------------------------------------
void Post::FEMeshBase::UpdateMeshData()
{

}

//-----------------------------------------------------------------------------
// Clear all the domains
void Post::FEMeshBase::ClearDomains()
{
	for (int i=0; i<(int) m_Dom.size(); ++i) delete m_Dom[i];
	m_Dom.clear();
}

//-----------------------------------------------------------------------------
// Clear all the parts
void Post::FEMeshBase::ClearParts()
{
	for (int i=0; i<(int) m_Part.size(); ++i) delete m_Part[i];
	m_Part.clear();
}

//-----------------------------------------------------------------------------
// Clear all the surfaces
void Post::FEMeshBase::ClearSurfaces()
{
	for (int i=0; i<(int) m_Surf.size(); ++i) delete m_Surf[i];
	m_Surf.clear();
}

//-----------------------------------------------------------------------------
// Clear all the node sets
void Post::FEMeshBase::ClearNodeSets()
{
	for (int i=0; i<(int) m_NSet.size(); ++i) delete m_NSet[i];
	m_NSet.clear();
}


//-----------------------------------------------------------------------------
void Post::FEMeshBase::Create(int nodes, int elems, int faces, int edges)
{
	if (nodes)
	{
		CleanUp();

		m_Node.resize(nodes);

		// assign default IDs
		for (int i=0; i<nodes; ++i) m_Node[i].SetID(i+1);
	}

	if (elems)
	{
		CreateElements(elems);

		// set default element ID's
		for (int i=0; i<elems; i++) 
		{
			FEElement_& el = ElementRef(i);
			el.SetID(i+1); 
			el.m_lid = i;	
		}
	}
}

//-----------------------------------------------------------------------------
// Find the element neighbours
void Post::FEMeshBase::FindNeighbours()
{
	// Build the node-element list
	m_NEL.Build(this);

	// reset all neighbors
	for (int i=0; i<Elements(); i++)
	{
		FEElement_& e = ElementRef(i);

		// solid elements
		for (int  j=0; j<e.Faces(); j++) e.m_nbr[j] = -1;

		// shells
		for (int j=0; j<e.Edges(); ++j) e.m_nbr[j] = -1;
	}

	// set up the element's neighbour pointers
	FEFace face, f2;
	FEEdge edge;
	FEElement_* pne;
	bool bfound;
	for (int i=0; i<Elements(); i++)
	{
		FEElement_& e = ElementRef(i);

		// first, do the solid elements
		for (int j=0; j<e.Faces(); j++)
		{
			if (e.m_nbr[j] == -1)
			{
				e.GetFace(j, face);

				// find the neighbour element
				const vector<NodeElemRef>& nel = m_NEL.ElementList(face.n[0]);
				bfound = false;
				for (int k=0; k < (int) nel.size(); k++)
				{
					pne = &ElementRef(nel[k].eid);
					if (pne != &e)
					{
						for (int l=0; l<pne->Faces(); l++) 
						{
							pne->GetFace(l, f2);
							if (face == f2)
							{
								e.m_nbr[j] = nel[k].eid;
								pne->m_nbr[l] = i;
								bfound = true;
								break;
							}
						}
					}

					if (bfound)
					{
						break;
					}
				}
			}
		}

		// next, do the shell elements
		for (int j=0; j<e.Edges(); ++j)
		{
			e.m_nbr[j] = -1;
			edge = e.GetEdge(j);

			// find the neighbour element
			const vector<NodeElemRef>& nel = m_NEL.ElementList(edge.n[0]);
			bfound = false;
			for (int k=0; k < (int) nel.size(); k++)
			{
				pne = &ElementRef(nel[k].eid);
				if ((pne != &e) && (*pne != e))
				{
					for (int l=0; l<pne->Edges(); l++) 
						if (edge == pne->GetEdge(l))
						{
							bfound = true;
							break;
						}

					if (bfound)
					{
						e.m_nbr[j] = nel[k].eid;
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Build the parts
void Post::FEMeshBase::UpdateDomains()
{
	ClearDomains();

	// figure out how many domains there are
	int ndom = 0;
	int NE = Elements();
	for (int i=0; i<NE; ++i) if (ElementRef(i).m_MatID > ndom) ndom = ElementRef(i).m_MatID;
	++ndom;

	// figure out the domain sizes
	vector<int> elemSize(ndom, 0);
	vector<int> faceSize(ndom, 0);

	for (int i = 0; i<NE; ++i)
	{
		FEElement_& el = ElementRef(i);
		elemSize[el.m_MatID]++;
	}

	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& face = Face(i);
		faceSize[face.m_mat]++;
	}

	m_Dom.resize(ndom);
	for (int i=0; i<ndom; ++i) 
	{
		m_Dom[i] = new FEDomain(this);
		m_Dom[i]->SetMatID(i);
		m_Dom[i]->Reserve(elemSize[i], faceSize[i]);
	}

	for (int i=0; i<NE; ++i)
	{
		FEElement_& el = ElementRef(i);
		m_Dom[el.m_MatID]->AddElement(i);
	}

	for (int i=0; i<NF; ++i)
	{
		FEFace& face = Face(i);
		m_Dom[face.m_mat]->AddFace(i);
	}
}

//-----------------------------------------------------------------------------
// Build the edges. 
// Currently, only edges from outside facets are created
void Post::FEMeshBase::BuildEdges()
{
	int NF = Faces();
	for (int i=0; i<NF; ++i) Face(i).m_ntag = i;

	m_Edge.reserve(NF);
	int nid = 1;
	for (int i=0; i<NF; ++i)
	{
		FEFace& fi = Face(i);
		int ne = fi.Edges();
		for (int j=0; j<ne; ++j)
		{
			FEFace* pfj = (fi.m_nbr[j] == -1 ? 0 : &Face(fi.m_nbr[j]));
			if ((pfj == 0) || (pfj->m_ntag > fi.m_ntag))
			{
				FEEdge e = fi.GetEdge(j);
				e.SetID(nid++);
				m_Edge.push_back(e);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Build the FE faces. Note that we only create exterior faces
void Post::FEMeshBase::BuildFaces()
{
	// make sure we only call this once
	assert(m_Face.empty());
	int i, j;

	// let's count the faces
	int NF = 0;
	int NE = Elements();
	for (i=0; i<NE; ++i)
	{
		FEElement_& e = ElementRef(i);

		// solid elements
		int nf = e.Faces();
		for (j=0; j<nf; ++j)
			if (e.m_nbr[j] < 0) ++NF;

		// shell elements
		if (e.Edges()) ++NF;
	}

	// allocate storage
	m_Face.resize(NF);

	// create faces
	NF = 0;
	for (i=0; i<NE; ++i)
	{
		FEElement_& e = ElementRef(i);
		
		// solid elements
		int nf = e.Faces();
		for (j=0; j<nf; ++j)
			if (e.m_nbr[j] < 0)
			{
				FEFace& f = m_Face[NF++];
				e.GetFace(j, f);
				f.m_elem[0] = i;
				f.m_elem[1] = j;
				f.m_mat = e.m_MatID;
				f.SetID(NF);
			}

		// shell elements
		if (e.Edges()>0)
		{
			FEFace& f = m_Face[NF++];
			f.n[0] = e.m_node[0];
			f.n[1] = e.m_node[1];
			f.n[2] = e.m_node[2];
			if (e.Type() == FE_QUAD4) 
			{
				f.n[3] = e.m_node[3];
				f.m_type = FE_FACE_QUAD4;
			}
            else if (e.Type() == FE_QUAD8)
            {
                f.n[3] = e.m_node[3];
                f.n[4] = e.m_node[4];
                f.n[5] = e.m_node[5];
                f.n[6] = e.m_node[6];
                f.n[7] = e.m_node[7];
                f.m_type = FE_FACE_QUAD8;
            }
            else if (e.Type() == FE_QUAD9)
            {
                f.n[3] = e.m_node[3];
                f.n[4] = e.m_node[4];
                f.n[5] = e.m_node[5];
                f.n[6] = e.m_node[6];
                f.n[7] = e.m_node[7];
                f.n[8] = e.m_node[8];
                f.m_type = FE_FACE_QUAD9;
            }
			else if (e.Type() == FE_TRI3)
			{
				f.n[3] = e.m_node[2];
				f.m_type = FE_FACE_TRI3;
			}
            else if (e.Type() == FE_TRI6)
            {
                f.n[3] = e.m_node[3];
                f.n[4] = e.m_node[4];
                f.n[5] = e.m_node[5];
                f.m_type = FE_FACE_TRI6;
            }

			f.m_elem[0] = i;
			f.m_elem[1] = 0;
			f.m_mat = e.m_MatID;
			f.SetID(NF);
		}
	}

	// build the node-face list
	m_NFL.Build(this);
}

//-----------------------------------------------------------------------------
void Post::FEMeshBase::FindFaceNeighbors()
{
	int nodes = Nodes();
	int faces = Faces();

	// calculate the valences
	vector<int> pnv(nodes, 0);
	for(int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int n = f.Nodes();
		for (int j=0; j<n; ++j) pnv[ f.n[j] ]++;
	}

	// figure out which face is attached to which node
	int nsize = 0;
	for (int i=0; i<nodes; ++i) nsize += pnv[i];

	vector<int> pnf(nsize);
	vector<int*> ppnf(nodes);
	ppnf[0] = &pnf[0];
	for (int i=1; i<nodes; ++i)	ppnf[i] = ppnf[i-1] + pnv[i-1];
	for (int i=0; i<nodes; ++i) pnv[i] = 0;

	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int n = f.Nodes();
		for (int j=0; j<n; ++j)
		{
			int nj = f.n[j];
			ppnf[nj][pnv[nj]] = i;
			pnv[nj]++;
		}
	}

	// clear the neighbours
	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		f.m_nbr[0] = -1;
		f.m_nbr[1] = -1;
		f.m_nbr[2] = -1;
		f.m_nbr[3] = -1;
	}

	// find neighbours
	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];

		// find all neighbours of this face
		int n = f.Edges();
		int jp1;
		int n1, n2, nval;
		for (int j=0; j<n; ++j)
		{
			jp1 = (j+1)%n;
			n1 = f.n[j];
			n2 = f.n[jp1];
			nval = pnv[n1];

			for (int k=0; k<nval; ++k)
			{
				FEFace& f2 = m_Face[ ppnf[n1][k] ];
				if ((&f2 != &f) && (f2.HasEdge(n1, n2)))
				{
					// we found the neighbour
					// also make sure that they are of similar type,
					// that is: shells can connect only to shells and solids to solids
					int e1 = (ElementRef(f .m_elem[0]).IsSolid()?1:0);
					int e2 = (ElementRef(f2.m_elem[0]).IsSolid()?1:0);
					if (e1 == e2)
					{
						// Eureka! We found one!
						f.m_nbr[j] = ppnf[n1][k];
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Update the FE data
void Post::FEMeshBase::Update()
{
	// find the element's neighbours
	if (m_NEL.IsEmpty()) FindNeighbours();

	// now that we have found the neighbours, let's find the faces
	if (m_Face.empty()) 
	{
		BuildFaces();
		FindFaceNeighbors();
	}

	// next, we build the edges
	if (m_Edge.empty()) BuildEdges();

	// create the parts
	UpdateDomains();

	// Calculate SG and normals
	AutoSmooth(60.0);

	// now we can figure out which nodes are interior and which are exterior
	UpdateNodes();
}

//-----------------------------------------------------------------------------
// Find the interior and exterior nodes
void Post::FEMeshBase::UpdateNodes()
{
	int i, j;

	int nodes = Nodes();
	int faces = Faces();
	int elems = Elements();

	for (i=0; i<nodes; ++i) 
	{
		FENode& n = m_Node[i];
		n.m_bext = false;
	}
	
	for (i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int nf = f.Nodes();
		for (j=0; j<nf; ++j) m_Node[f.n[j]].m_bext = true;
	}
}

//-----------------------------------------------------------------------------
bool IsInsideElement(FEElement_& el, double r[3], const double tol)
{
	switch (el.Type())
	{
	case FE_TET4:
	case FE_TET10:
	case FE_TET15:
	case FE_TET20:
		return (r[0] >= -tol) && (r[1] >= -tol) && (r[2] >= -tol) && (r[0] + r[1] + r[2] <= 1.0 + tol);
	case FE_HEX8:
	case FE_HEX20:
	case FE_HEX27:
	case FE_PYRA5:
		return ((r[0]>=-1.0-tol)&&(r[0]<= 1.0+tol)&&
			    (r[1]>=-1.0-tol)&&(r[1]<= 1.0+tol)&&
				(r[2]>=-1.0-tol)&&(r[2]<= 1.0+tol));
	}
	return false;
}

//-----------------------------------------------------------------------------
void project_inside_element(FEElement_& el, const vec3f& p, double r[3], vec3f* x)
{
	const double tol = 0.0001;
	const int nmax = 10;

	int ne = el.Nodes();
	double dr[3], R[3];
	Mat3d K;
	double u2, N[FEElement::MAX_NODES], G[3][FEElement::MAX_NODES];
	int n = 0;
	do
	{
		el.shape(N, r[0], r[1], r[2]);
		el.shape_deriv(G[0], G[1], G[2], r[0], r[1], r[2]);

		R[0] = p.x;
		R[1] = p.y;
		R[2] = p.z;
		for (int i = 0; i<ne; ++i)
		{
			R[0] -= N[i] * x[i].x;
			R[1] -= N[i] * x[i].y;
			R[2] -= N[i] * x[i].z;
		}

		K.zero();
		for (int i = 0; i<ne; ++i)
		{
			K[0][0] -= G[0][i] * x[i].x; K[0][1] -= G[1][i] * x[i].x; K[0][2] -= G[2][i] * x[i].x;
			K[1][0] -= G[0][i] * x[i].y; K[1][1] -= G[1][i] * x[i].y; K[1][2] -= G[2][i] * x[i].y;
			K[2][0] -= G[0][i] * x[i].z; K[2][1] -= G[1][i] * x[i].z; K[2][2] -= G[2][i] * x[i].z;
		}

		K.Invert();

		dr[0] = K[0][0] * R[0] + K[0][1] * R[1] + K[0][2] * R[2];
		dr[1] = K[1][0] * R[0] + K[1][1] * R[1] + K[1][2] * R[2];
		dr[2] = K[2][0] * R[0] + K[2][1] * R[1] + K[2][2] * R[2];

		r[0] -= dr[0];
		r[1] -= dr[1];
		r[2] -= dr[2];

		u2 = dr[0] * dr[0] + dr[1] * dr[1] + dr[2] * dr[2];
		++n;
	} 
	while ((u2 > tol*tol) && (n < nmax));
}

//-----------------------------------------------------------------------------
bool Post::ProjectInsideReferenceElement(FEMeshBase& m, FEElement_& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FEElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = to_vec3f(m.Node(el.m_node[i]).r);

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool Post::ProjectInsideElement(FEMeshBase& m, FEElement_& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FEElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = to_vec3f(m.Node(el.m_node[i]).r);

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool Post::FindElementRef(FEMeshBase& m, const vec3f& p, int& nelem, double r[3])
{
	vec3d y[FEElement::MAX_NODES];
	int NE = m.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement_& e = m.ElementRef(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3d r0 = m.Node(e.m_node[0]).r;
		vec3d r1 = r0;
		for (int j=1; j<ne; ++j)
		{
			vec3d& rj = m.Node(e.m_node[j]).r;
			if (rj.x < r0.x) r0.x = rj.x;
			if (rj.y < r0.y) r0.y = rj.y;
			if (rj.z < r0.z) r0.z = rj.z;
			if (rj.x > r1.x) r1.x = rj.x;
			if (rj.y > r1.y) r1.y = rj.y;
			if (rj.z > r1.z) r1.z = rj.z;
		}

		double dx = fabs(r0.x - r1.x);
		double dy = fabs(r0.y - r1.y);
		double dz = fabs(r0.z - r1.z);

		double R = dx;
		if (dy > R) R = dy;
		if (dz > R) R = dz;
		double eps = R*0.001;

		r0.x -= eps;
		r0.y -= eps;
		r0.z -= eps;

		r1.x += eps;
		r1.y += eps;
		r1.z += eps;

		if ((p.x  >= r0.x)&&(p.x <= r1.x)&&
			(p.y  >= r0.y)&&(p.y <= r1.y)&&
			(p.z  >= r0.z)&&(p.z <= r1.z))
		{
			if (ProjectInsideElement(m, e, p, r)) return true;	
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Post::FindElementInReferenceFrame(FEMeshBase& m, const vec3f& p, int& nelem, double r[3])
{
	vec3f y[FEElement::MAX_NODES];
	int NE = m.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = m.ElementRef(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3f r0 = to_vec3f(m.Node(e.m_node[0]).r);
		vec3f r1 = r0;
		for (int j = 1; j<ne; ++j)
		{
			vec3f rj = to_vec3f(m.Node(e.m_node[j]).r);
			if (rj.x < r0.x) r0.x = rj.x;
			if (rj.y < r0.y) r0.y = rj.y;
			if (rj.z < r0.z) r0.z = rj.z;
			if (rj.x > r1.x) r1.x = rj.x;
			if (rj.y > r1.y) r1.y = rj.y;
			if (rj.z > r1.z) r1.z = rj.z;
		}

		float dx = fabs(r0.x - r1.x);
		float dy = fabs(r0.y - r1.y);
		float dz = fabs(r0.z - r1.z);

		float R = dx;
		if (dy > R) R = dy;
		if (dz > R) R = dz;
		float eps = R*0.001f;

		r0.x -= eps;
		r0.y -= eps;
		r0.z -= eps;

		r1.x += eps;
		r1.y += eps;
		r1.z += eps;

		if ((p.x >= r0.x) && (p.x <= r1.x) &&
			(p.y >= r0.y) && (p.y <= r1.y) &&
			(p.z >= r0.z) && (p.z <= r1.z))
		{
			if (ProjectInsideReferenceElement(m, e, p, r)) return true;
		}
	}

	return false;
}

//=================================================================================================

Post::FEFindElement::OCTREE_BOX::OCTREE_BOX()
{
	m_elem = -1;
	m_level = -1;
}

Post::FEFindElement::OCTREE_BOX::~OCTREE_BOX()
{
	for (size_t i=0; i<m_child.size(); ++i) delete m_child[i];
	m_child.clear();
}

void Post::FEFindElement::OCTREE_BOX::split(int levels)
{
	m_level = levels;
	if (m_level == 0) return;

	double x0 = m_box.x0, x1 = m_box.x1;
	double y0 = m_box.y0, y1 = m_box.y1;
	double z0 = m_box.z0, z1 = m_box.z1;

	double dx = x1 - x0;
	double dy = y1 - y0;
	double dz = z1 - z0;

	m_child.clear();
	for (int i=0; i<2; i++)
		for (int j = 0; j<2; j++)
			for (int k = 0; k <2; k++)
			{
				double xa = x0 + i*dx*0.5, xb = x0 + (i + 1)*dx*0.5;
				double ya = y0 + j*dy*0.5, yb = y0 + (j + 1)*dy*0.5;
				double za = z0 + k*dz*0.5, zb = z0 + (k + 1)*dz*0.5;

				BOX b(xa, ya, za, xb, yb, zb);
				double R = b.GetMaxExtent();
				b.Inflate(R*0.0001);

				OCTREE_BOX* box = new OCTREE_BOX;
				box->m_box = b;

				m_child.push_back(box);
			}

	for (int i=0; i<(int)m_child.size(); ++i)
	{
		m_child[i]->split(levels - 1);
	}
}

void Post::FEFindElement::OCTREE_BOX::Add(BOX& b, int nelem)
{
	if (m_level == 0)
	{
		if (m_box.Intersects(b))
		{
			OCTREE_BOX* box = new OCTREE_BOX;
			box->m_box = b;
			box->m_elem = nelem;
			box->m_level = -1;
			m_child.push_back(box);
			return;
		}
	}
	else
	{
		for (size_t i=0; i<m_child.size(); ++i)
		{
			m_child[i]->Add(b, nelem);
		}
	}
}


Post::FEFindElement::OCTREE_BOX* Post::FEFindElement::OCTREE_BOX::Find(const vec3f& r)
{
	if (m_level == 0)
	{
		bool inside = m_box.IsInside(r);
		return (inside ? this : 0);
	}

	// try to find the child
	for (size_t i = 0; i<m_child.size(); ++i)
	{
		OCTREE_BOX* c = m_child[i];
		OCTREE_BOX* ret = c->Find(r);
		if (ret) return ret;
	}

	return 0;
}


Post::FEFindElement::OCTREE_BOX* Post::FEFindElement::FindBox(const vec3f& r)
{
	// make sure it's in the master box
	if (m_bound.IsInside(r) == false) return 0;

	// try to find the child
	OCTREE_BOX* b = &m_bound;
	do
	{
		if (b->m_level == 0)
		{
			bool inside = b->IsInside(r);
			return (inside ? b : 0);
		}

		bool bfound = false;
		for (size_t i = 0; i<b->m_child.size(); ++i)
		{
			OCTREE_BOX* c = b->m_child[i];
			if (c->IsInside(r))
			{
				b = c;
				bfound = true;
				break;
			}
		}

		assert(bfound);
		if (bfound == false) break;
	}
	while (1);

	return 0;
}

Post::FEFindElement::FEFindElement(FEMeshBase& mesh) : m_mesh(mesh)
{
	m_nframe = -1;
}

void Post::FEFindElement::InitReferenceFrame(vector<bool>& flags)
{
	assert(m_nframe == 0);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3f r = to_vec3f(m_mesh.Node(0).r);
	BOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = to_vec3f(m_mesh.Node(i).r);
		box += r;
	}
	double R = box.GetMaxExtent();
	box.Inflate(R*0.001);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = (int)flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = m_mesh.ElementRef(i);

		bool badd = true;
		if (flags.empty() == false)
		{
			int mid = e.m_MatID;
			if ((mid >= 0) && (mid < cflags)) badd = flags[mid];
		}

		if (badd)
		{
			int ne = e.Nodes();

			// do a quick bounding box test
			vec3f r0 = to_vec3f(m_mesh.Node(e.m_node[0]).r);
			vec3f r1 = r0;
			BOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3f rj = to_vec3f(m_mesh.Node(e.m_node[j]).r);
				box += rj;
			}
			double R = box.GetMaxExtent();
			box.Inflate(R*0.001);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void Post::FEFindElement::InitCurrentFrame(vector<bool>& flags)
{
	assert(m_nframe == 1);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3d r = m_mesh.Node(0).r;
	BOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = m_mesh.Node(i).r;
		box += r;
	}
	double R = box.GetMaxExtent();
	box.Inflate(R*0.001);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = (int)flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FEElement_& e = m_mesh.ElementRef(i);

		bool badd = true;
		if (flags.empty() == false)
		{
			int mid = e.m_MatID;
			if ((mid >= 0) && (mid < cflags)) badd = flags[mid];
		}

		if (badd)
		{
			int ne = e.Nodes();

			// do a quick bounding box test
			vec3d r0 = m_mesh.Node(e.m_node[0]).r;
			vec3d r1 = r0;
			BOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3d rj = m_mesh.Node(e.m_node[j]).r;
				box += rj;
			}
			double R = box.GetMaxExtent();
			box.Inflate(R*0.001);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void Post::FEFindElement::Init(int nframe)
{
	vector<bool> dummy;
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(dummy);
	else InitCurrentFrame(dummy);
}

void Post::FEFindElement::Init(vector<bool>& flags, int nframe)
{
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(flags);
	else InitCurrentFrame(flags);
}

bool Post::FEFindElement::FindInReferenceFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 0);

	vec3f y[FEElement::MAX_NODES];
	OCTREE_BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		OCTREE_BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FEElement_& e = m_mesh.ElementRef(nid);
		nelem = nid;

		// do a quick bounding box test
		if (c->m_box.IsInside(x))
		{
			// do a more complete search
			if (ProjectInsideReferenceElement(m_mesh, e, x, r)) return true;
		}
	}

	nelem = -1;
	return false;
}

bool Post::FEFindElement::FindInCurrentFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 1);

	vec3f y[FEElement::MAX_NODES];
	OCTREE_BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		OCTREE_BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FEElement_& e = m_mesh.ElementRef(nid);
		nelem = nid;

		// do a quick bounding box test
		if (c->m_box.IsInside(x))
		{
			// do a more complete search
			if (ProjectInsideElement(m_mesh, e, x, r)) return true;
		}
	}

	nelem = -1;
	return false;
}
