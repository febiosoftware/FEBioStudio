// FEMeshBase.cpp: implementation of the FEMeshBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMesh.h"
#include "FEState.h"
#include <MeshLib/MeshTools.h>
#include <stack>

//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
Post::FEPostMesh::FEPostMesh()
{
}

//-----------------------------------------------------------------------------
// Destructor
Post::FEPostMesh::~FEPostMesh()
{
	CleanUp();
}

//-----------------------------------------------------------------------------
// clean all depdendant structures
void Post::FEPostMesh::CleanUp()
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
void Post::FEPostMesh::ClearAll()
{
	CleanUp();
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	ClearElements();
}

//-----------------------------------------------------------------------------
void Post::FEPostMesh::UpdateMeshData()
{

}

//-----------------------------------------------------------------------------
// Clear all the domains
void Post::FEPostMesh::ClearDomains()
{
	for (int i=0; i<(int) m_Dom.size(); ++i) delete m_Dom[i];
	m_Dom.clear();
}

//-----------------------------------------------------------------------------
// Clear all the parts
void Post::FEPostMesh::ClearParts()
{
	for (int i=0; i<(int) m_Part.size(); ++i) delete m_Part[i];
	m_Part.clear();
}

//-----------------------------------------------------------------------------
// Clear all the surfaces
void Post::FEPostMesh::ClearSurfaces()
{
	for (int i=0; i<(int) m_Surf.size(); ++i) delete m_Surf[i];
	m_Surf.clear();
}

//-----------------------------------------------------------------------------
// Clear all the node sets
void Post::FEPostMesh::ClearNodeSets()
{
	for (int i=0; i<(int) m_NSet.size(); ++i) delete m_NSet[i];
	m_NSet.clear();
}


//-----------------------------------------------------------------------------
void Post::FEPostMesh::Create(int nodes, int elems, int faces, int edges)
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

		// allocate storage for element data
		m_data.Clear();
	}
}

//-----------------------------------------------------------------------------
// Find the element neighbours
void Post::FEPostMesh::FindNeighbours()
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
void Post::FEPostMesh::UpdateDomains()
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

		int ma = ElementRef(face.m_elem[0].eid).m_MatID;
		int mb = (face.m_elem[1].eid >= 0 ? ElementRef(face.m_elem[1].eid).m_MatID : -1);

		faceSize[ma]++;
		if (mb >= 0) faceSize[mb]++;
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

		int ma = ElementRef(face.m_elem[0].eid).m_MatID;
		int mb = (face.m_elem[1].eid >= 0 ? ElementRef(face.m_elem[1].eid).m_MatID : -1);

		m_Dom[ma]->AddFace(i);
		if (mb >= 0) m_Dom[mb]->AddFace(i);
	}
}

//-----------------------------------------------------------------------------
// Build the edges. 
// Currently, only edges from outside facets are created
void Post::FEPostMesh::BuildEdges()
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
// Build the FE faces. Facets are created for all external and internal surfaces.
void Post::FEPostMesh::BuildFaces()
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
		for (j = 0; j < nf; ++j)
		{
			e.m_face[j] = -1;
			if (e.m_nbr[j] < 0)
			{
				// this adds an external facet
				++NF;
			}
			else
			{
				// this adds an internal facet if adjacent elemetns have a different material
				int ma = e.m_MatID;
				int mb = ElementRef(e.m_nbr[j]).m_MatID;
				if (ma < mb) ++NF;
			}
		}

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
		for (j = 0; j < nf; ++j)
		{
			if (e.m_nbr[j] < 0)
			{
				// this adds an external facet
				e.m_face[j] = NF;

				FEFace& f = m_Face[NF++];
				e.GetFace(j, f);
				f.m_elem[0].eid = i; f.m_elem[0].lid = j;
				f.m_elem[1].eid = -1;
				f.SetID(NF);
			}
			else
			{
				// this adds an internal facet if adjacent elements have a different material
				FEElement_& nel = ElementRef(e.m_nbr[j]);
				int ma = e.m_MatID;
				int mb = nel.m_MatID;
				if (ma < mb)
				{
					FEFace& f = m_Face[NF];
					e.GetFace(j, f);
					f.m_elem[0].eid = i; 
					f.m_elem[0].lid = j;
					
					f.m_elem[1].eid = e.m_nbr[j];
					f.m_elem[1].lid = nel.FindFace(f); assert(f.m_elem[1].lid >= 0);
					f.SetID(NF);

					e.m_face[j] = NF;
					nel.m_face[f.m_elem[1].lid] = NF;

					NF++;
				}
			}
		}


		// shell elements
		if (e.Edges()>0)
		{
			e.m_face[0] = NF;

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

			f.m_elem[0].eid =  i; f.m_elem[0].lid = 0;
			f.m_elem[1].eid = -1; f.m_elem[1].lid = -1;
			f.SetID(NF);
		}
	}

	// build the node-face list
	m_NFL.Build(this);
}

//-----------------------------------------------------------------------------
void Post::FEPostMesh::FindFaceNeighbors()
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
					int e1 = (ElementRef(f .m_elem[0].eid).IsSolid()?1:0);
					int e2 = (ElementRef(f2.m_elem[0].eid).IsSolid()?1:0);
					// make sure external surfaces only connect to external surfaces
					e1 += (f .m_elem[1].eid == -1 ? 2 : 0);
					e2 += (f2.m_elem[1].eid == -1 ? 2 : 0);
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
void Post::FEPostMesh::Update()
{
	// find the element's neighbours
	if (m_NEL.IsEmpty()) FindNeighbours();

	// now that we have found the neighbours, let's find the faces
	if (m_Face.empty()) 
	{
		BuildFaces();
		FindFaceNeighbors();
	}

	// partition the surface
	AutoPartitionSurface();

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
void Post::FEPostMesh::UpdateNodes()
{
	int i, j;

	int nodes = Nodes();
	int faces = Faces();
	int elems = Elements();

	for (i=0; i<nodes; ++i) 
	{
		FENode& n = m_Node[i];
		n.SetExterior(false);
	}
	
	for (i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int nf = f.Nodes();
		for (j=0; j<nf; ++j) m_Node[f.n[j]].SetExterior(true);
	}
}

//-----------------------------------------------------------------------------
bool Post::ProjectInsideReferenceElement(FECoreMesh& m, FEElement_& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FEElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = to_vec3f(m.Node(el.m_node[i]).r);

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool Post::FindElementInReferenceFrame(FECoreMesh& m, const vec3f& p, int& nelem, double r[3])
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

Post::FEFindElement::FEFindElement(FECoreMesh& mesh) : m_mesh(mesh)
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

//-----------------------------------------------------------------------------
double Post::IntegrateNodes(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	int N = mesh.Nodes();
	for (int i = 0; i<N; ++i)
	{
		FENode& node = mesh.Node(i);
		if (node.IsSelected() && (ps->m_NODE[i].m_ntag > 0))
		{
			res += ps->m_NODE[i].m_val;
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
double Post::IntegrateEdges(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	assert(false);
	return 0.0;
}

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double Post::IntegrateFaces(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	float v[FEFace::MAX_NODES];
	vec3f r[FEFace::MAX_NODES];
	for (int i = 0; i<mesh.Faces(); ++i)
	{
		FEFace& f = mesh.Face(i);
		if (f.IsSelected() && f.IsActive())
		{
			int nn = f.Nodes();

			// get the nodal values
			for (int j = 0; j<nn; ++j) v[j] = ps->m_NODE[f.n[j]].m_val;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				v[3] = v[2];
				break;
			}

			// get the nodal coordinates
			for (int j = 0; j<nn; ++j) r[j] = ps->m_NODE[f.n[j]].m_rt;
			switch (f.Type())
			{
			case FE_FACE_TRI3:
			case FE_FACE_TRI6:
			case FE_FACE_TRI7:
			case FE_FACE_TRI10:
				r[3] = r[2];
				break;
			}

			// add to integral
			res += IntegrateQuad(r, v);
		}
	}
	return res;
}

//-----------------------------------------------------------------------------
// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double Post::IntegrateElems(Post::FEPostMesh& mesh, Post::FEState* ps)
{
	double res = 0.0;
	float v[FEElement::MAX_NODES];
	vec3f r[FEElement::MAX_NODES];
	for (int i = 0; i<mesh.Elements(); ++i)
	{
		FEElement_& e = mesh.ElementRef(i);
		if (e.IsSelected() && (e.IsSolid()) && (ps->m_ELEM[i].m_state & Post::StatusFlags::ACTIVE))
		{
			int nn = e.Nodes();

			// get the nodal values and coordinates
			for (int j = 0; j<nn; ++j) v[j] = ps->m_NODE[e.m_node[j]].m_val;
			for (int j = 0; j<nn; ++j) r[j] = ps->m_NODE[e.m_node[j]].m_rt;
			switch (e.Type())
			{
			case FE_PENTA6:
				v[7] = v[5]; r[7] = r[5];
				v[6] = v[5]; r[6] = r[5];
				v[5] = v[4]; r[5] = r[4];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			case FE_TET4:
			case FE_TET5:
			case FE_TET10:
			case FE_TET15:
				v[7] = v[3]; r[7] = r[3];
				v[6] = v[3]; r[6] = r[3];
				v[5] = v[3]; r[5] = r[3];
				v[4] = v[3]; r[4] = r[3];
				v[3] = v[2]; r[3] = r[2];
				v[2] = v[2]; r[2] = r[2];
				v[1] = v[1]; r[1] = r[1];
				v[0] = v[0]; r[0] = r[0];
				break;
			}

			// add to integral
			res += IntegrateHex(r, v);
		}
	}
	return res;
}
