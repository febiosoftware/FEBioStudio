// FEMeshBase.cpp: implementation of the FEMeshBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMesh.h"
#include <stack>

using namespace Post;

//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
FEMeshBase::FEMeshBase()
{
}

//-----------------------------------------------------------------------------
// Destructor
FEMeshBase::~FEMeshBase()
{
	CleanUp();
}

//-----------------------------------------------------------------------------
// clean all depdendant structures
void FEMeshBase::CleanUp()
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
void FEMeshBase::ClearAll()
{
	CleanUp();
	m_Node.clear();
	m_Edge.clear();
	m_Face.clear();
	ClearElements();
}

//-----------------------------------------------------------------------------
// Clear all the domains
void FEMeshBase::ClearDomains()
{
	for (int i=0; i<(int) m_Dom.size(); ++i) delete m_Dom[i];
	m_Dom.clear();
}

//-----------------------------------------------------------------------------
// Clear all the parts
void FEMeshBase::ClearParts()
{
	for (int i=0; i<(int) m_Part.size(); ++i) delete m_Part[i];
	m_Part.clear();
}

//-----------------------------------------------------------------------------
// Clear all the surfaces
void FEMeshBase::ClearSurfaces()
{
	for (int i=0; i<(int) m_Surf.size(); ++i) delete m_Surf[i];
	m_Surf.clear();
}

//-----------------------------------------------------------------------------
// Clear all the node sets
void FEMeshBase::ClearNodeSets()
{
	for (int i=0; i<(int) m_NSet.size(); ++i) delete m_NSet[i];
	m_NSet.clear();
}


//-----------------------------------------------------------------------------
void FEMeshBase::Create(int nodes, int elems)
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
			FEElement& el = Element(i);
			el.SetID(i+1); 
			el.m_lid = i;	
		}
	}
}

//-----------------------------------------------------------------------------
// Count nr of beam elements
int FEMeshBase::BeamElements()
{
	int n = 0;
	for (int i=0; i<Elements(); ++i)
	{
		if (Element(i).IsBeam()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of shell elements
int FEMeshBase::ShellElements()
{
	int n = 0;
	for (int i=0; i<Elements(); ++i)
	{
		if (Element(i).IsShell()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of solid elements
int FEMeshBase::SolidElements()
{
	int n = 0;
	for (int i=0; i<Elements(); ++i)
	{
		if (Element(i).IsSolid()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Find the element neighbours
void FEMeshBase::FindNeighbours()
{
	// Build the node-element list
	m_NEL.Build(this);

	// reset all neighbors
	for (int i=0; i<Elements(); i++)
	{
		FEElement& e = Element(i);

		// solid elements
		for (int  j=0; j<e.Faces(); j++) e.m_pElem[j] = 0;

		// shells
		for (int j=0; j<e.Edges(); ++j) e.m_pElem[j] = 0;
	}

	// set up the element's neighbour pointers
	FEFace face, f2;
	FEEdge edge;
	FEElement* pne;
	bool bfound;
	for (int i=0; i<Elements(); i++)
	{
		FEElement& e = Element(i);

		// first, do the solid elements
		for (int j=0; j<e.Faces(); j++)
		{
			if (e.m_pElem[j] == 0)
			{
				e.GetFace(j, face);

				// find the neighbour element
				vector<NodeElemRef>& nel = m_NEL.ElemList(face.node[0]);
				bfound = false;
				for (int k=0; k < (int) nel.size(); k++)
				{
					pne = &Element(nel[k].first);
					if (pne != &e)
					{
						for (int l=0; l<pne->Faces(); l++) 
						{
							pne->GetFace(l, f2);
							if (face == f2)
							{
								e.m_pElem[j] = pne;
								pne->m_pElem[l] = &e;
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
			e.m_pElem[j] = 0;
			edge = e.GetEdge(j);

			// find the neighbour element
			vector<NodeElemRef>& nel = m_NEL.ElemList(edge.node[0]);
			bfound = false;
			for (int k=0; k < (int) nel.size(); k++)
			{
				pne = &Element(nel[k].first);
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
						e.m_pElem[j] = pne;
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Build the parts
void FEMeshBase::UpdateDomains()
{
	ClearDomains();

	// figure out how many domains there are
	int ndom = 0;
	int NE = Elements();
	for (int i=0; i<NE; ++i) if (Element(i).m_MatID > ndom) ndom = Element(i).m_MatID;
	++ndom;

	// figure out the domain sizes
	vector<int> elemSize(ndom, 0);
	vector<int> faceSize(ndom, 0);

	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = Element(i);
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
		FEElement& el = Element(i);
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
void FEMeshBase::BuildEdges()
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
				FEEdge e = fi.Edge(j);
				e.SetID(nid++);
				m_Edge.push_back(e);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Build the FE faces. Note that we only create exterior faces
void FEMeshBase::BuildFaces()
{
	// make sure we only call this once
	assert(m_Face.empty());
	int i, j;

	// let's count the faces
	int NF = 0;
	int NE = Elements();
	for (i=0; i<NE; ++i)
	{
		FEElement& e = Element(i);

		// solid elements
		int nf = e.Faces();
		for (j=0; j<nf; ++j)
			if (e.m_pElem[j] == 0) ++NF;

		// shell elements
		if (e.Edges()) ++NF;
	}

	// allocate storage
	m_Face.resize(NF);

	// create faces
	NF = 0;
	for (i=0; i<NE; ++i)
	{
		FEElement& e = Element(i);
		
		// solid elements
		int nf = e.Faces();
		for (j=0; j<nf; ++j)
			if (e.m_pElem[j] == 0)
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
			f.node[0] = e.m_node[0];
			f.node[1] = e.m_node[1];
			f.node[2] = e.m_node[2];
			if (e.Type() == FE_QUAD4) 
			{
				f.node[3] = e.m_node[3];
				f.m_ntype = FACE_QUAD4;
			}
            else if (e.Type() == FE_QUAD8)
            {
                f.node[3] = e.m_node[3];
                f.node[4] = e.m_node[4];
                f.node[5] = e.m_node[5];
                f.node[6] = e.m_node[6];
                f.node[7] = e.m_node[7];
                f.m_ntype = FACE_QUAD8;
            }
            else if (e.Type() == FE_QUAD9)
            {
                f.node[3] = e.m_node[3];
                f.node[4] = e.m_node[4];
                f.node[5] = e.m_node[5];
                f.node[6] = e.m_node[6];
                f.node[7] = e.m_node[7];
                f.node[8] = e.m_node[8];
                f.m_ntype = FACE_QUAD9;
            }
			else if (e.Type() == FE_TRI3)
			{
				f.node[3] = e.m_node[2];
				f.m_ntype = FACE_TRI3;
			}
            else if (e.Type() == FE_TRI6)
            {
                f.node[3] = e.m_node[3];
                f.node[4] = e.m_node[4];
                f.node[5] = e.m_node[5];
                f.m_ntype = FACE_TRI6;
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
void FEMeshBase::FindFaceNeighbors()
{
	int nodes = Nodes();
	int faces = Faces();

	// calculate the valences
	vector<int> pnv(nodes, 0);
	for(int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int n = f.Nodes();
		for (int j=0; j<n; ++j) pnv[ f.node[j] ]++;
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
			int nj = f.node[j];
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
			n1 = f.node[j];
			n2 = f.node[jp1];
			nval = pnv[n1];

			for (int k=0; k<nval; ++k)
			{
				FEFace& f2 = m_Face[ ppnf[n1][k] ];
				if ((&f2 != &f) && (f2.HasEdge(n1, n2)))
				{
					// we found the neighbour
					// also make sure that they are of similar type,
					// that is: shells can connect only to shells and solids to solids
					int e1 = (Element(f .m_elem[0]).IsSolid()?1:0);
					int e2 = (Element(f2.m_elem[0]).IsSolid()?1:0);
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
void FEMeshBase::Update()
{
	// find the element's neighbours
	if (m_NEL.Empty()) FindNeighbours();

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
	AutoSmooth(60.*PI / 180.0);

	// now we can figure out which nodes are interior and which are exterior
	UpdateNodes();
}

//-----------------------------------------------------------------------------
// Find the interior and exterior nodes
void FEMeshBase::UpdateNodes()
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
		for (j=0; j<nf; ++j) m_Node[f.node[j]].m_bext = true;
	}
}

//-----------------------------------------------------------------------------
// Partition the surface depending on a smoothing tolerance. Face neighbours
// are only set when the faces belong to the same smoothing group.
void FEMeshBase::AutoSmooth(double angleRadians)
{
	int faces = Faces();
	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];

		// calculate the face normals
		vec3f& r0 = Node(f.node[0]).m_r0;
		vec3f& r1 = Node(f.node[1]).m_r0;
		vec3f& r2 = Node(f.node[2]).m_r0;

		f.m_fn = (r1 - r0)^(r2 - r0);
		f.m_fn.Normalize();
		f.m_nsg = 0;
	}

	// smoothing threshold
	double eps = cos(angleRadians);

	//calculate the node normals
	int nodes = Nodes();
	vector<vec3f> pnorm(nodes);
	for (int i=0; i<nodes; ++i) pnorm[i] = vec3f(0,0,0);

	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		f.m_ntag = -1;
	}

	vector<FEFace*> F(faces);
	int NF = 0;

	// unprocessed face list
	int ui = 0;

	stack<FEFace*> stack;
	int nsg = 0;
	FEFace* pf = 0;
	do
	{
		if (stack.empty())
		{
			if (nsg > 0)
			{
				// assign node normals
				for (int i=0; i<NF; ++i)
				{
					FEFace& f = *F[i];
					assert(f.m_ntag == nsg);
					int nf = f.Nodes();
					for (int k=0; k<nf; ++k) f.m_nn[k] = pnorm[ f.node[k] ];
				}

				// clear normals
				for (int i=0; i<NF; ++i)
				{
					FEFace& f = *F[i];
					int nf = f.Nodes();
					for (int k=0; k<nf; ++k) pnorm[ f.node[k] ] = vec3f(0,0,0);
				}
			}

			// find an unprocessed face
			pf = 0;
			for (int i = ui; i<faces; ++i, ++ui) if (Face(i).m_ntag == -1) { pf = &m_Face[i]; break; }

			if (pf) stack.push(pf);
			++nsg;
			NF = 0;
		}
		else
		{
			// pop a face
			pf = stack.top(); stack.pop();

			// mark as processed
			pf->m_ntag = nsg;
			pf->m_nsg = nsg;
			F[NF++] = pf;

			int nf = pf->Nodes();
			int n = -1;
			if ((nf==3)||(nf==6)||(nf==7)||(n==10)) n = 3;
			if ((nf==4)||(nf==8)||(nf==9)) n = 4;

			// add face normal to node normal
			for (int i=0; i<nf; ++i) pnorm[pf->node[i]] += pf->m_fn;

			// push unprocessed neighbours
			for (int i=0; i<n; ++i)
			{
				if (pf->m_nbr[i] >= 0)
				{
					FEFace& f2 = m_Face[pf->m_nbr[i]];
					if ((f2.m_ntag == -1) && (pf->m_fn*f2.m_fn > eps))
					{
						f2.m_ntag = -2;
						stack.push(&f2);
					}
				}
			}
		}
	}
	while (pf);

	// normalize face normals
	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		int nf = f.Nodes();
		for (int k=0; k<nf; ++k) f.m_nn[k].Normalize();
	}
}

//-----------------------------------------------------------------------------
// Update the face normals. If bsmooth, the smoothing groups are used
// to create a smoothed surface representation.
void FEMeshBase::UpdateNormals(bool bsmooth)
{
	int faces = Faces();
	int nodes = Nodes();

	// calculate face normals
	for (int i=0; i<faces; ++i)
	{
		FEFace& face = Face(i);

		vec3f& r1 = Node(face.node[0]).m_rt;
		vec3f& r2 = Node(face.node[1]).m_rt;
		vec3f& r3 = Node(face.node[2]).m_rt;

		face.m_fn = (r2-r1)^(r3-r1);
		face.m_fn.Normalize();
	}

	for (int i=0; i<faces; ++i)
	{
		FEFace& f = m_Face[i];
		f.m_ntag = -1;
	}

	// calculate node normals based on smoothing groups
	if (bsmooth)
	{
		vector<FEFace*> stack(2*faces);
		int ns = 0;

		int nsg = 0;
		vector<vec3f> nt(nodes);
		vector<int> ntag; ntag.assign(nodes, 0);
		vector<FEFace*> faceList(faces);
		vector<int> nodeList(nodes);
		int nfl = 0, nnl = 0;
		for (int i=0; i<faces; ++i)
		{
			// find the next unprocessed face
			FEFace* pf = &m_Face[i];
			if (pf->m_ntag == -1)
			{
				// push this face on the stack
				stack[ns++] = pf;

				// find all connected faces
				nfl = 0;
				nnl = 0;
				while (ns > 0)
				{
					// pop a face
					pf = stack[--ns];
					faceList[nfl++] = pf;

					// mark as processed
					pf->m_ntag = nsg;

					int fn = pf->Nodes();
					int fe = pf->Edges();

					// add face normal to node normal
					for (int j=0; j<fn; ++j)
					{
						int nj = pf->node[j];
						nt[nj] += pf->m_fn;
						if (ntag[nj] != 1)
						{
							ntag[nj] = 1;
							nodeList[nnl++] = nj;
						}
					}

					// push unprocessed neighbors
					FEFace* pf2;
					for (int j=0; j<fe; ++j)
					{	
						if (pf->m_nbr[j] >= 0)
						{
							pf2 = &m_Face[pf->m_nbr[j]];
							if ((pf2->m_ntag == -1) && (pf2->m_nsg == pf->m_nsg))
							{
								pf2->m_ntag = nsg;
								stack[ns++] = pf2;
							}
						}
					}
				}

				// normalize normals
				for (int j=0; j<nnl; ++j)
				{
					int nj = nodeList[j];
					assert(ntag[nj] == 1);
					nt[nj].Normalize();
					ntag[nj] = 0;
				}

				// assign node normals
				for (int j=0; j<nfl; ++j)
				{
					FEFace& f = *faceList[j];
					assert(f.m_ntag == nsg);
					int nf = f.Nodes();
					for (int k=0; k<nf; ++k) f.m_nn[k] = nt[ f.node[k] ];
				}

				// clear normals for next group
				for (int j=0; j<nnl; ++j) nt[nodeList[j]] = vec3f(0,0,0);
				++nsg;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// area of triangle
double triangle_area(const vec3f& r0, const vec3f& r1, const vec3f& r2)
{
	return ((r1 - r0)^(r2 - r0)).Length()*0.5f;
}

//-----------------------------------------------------------------------------
double FEMeshBase::FaceArea(FEFace &f)
{
	const int N = f.Nodes();
	vector<vec3f> nodes(N);
	for (int i = 0; i < N; ++i)
	{
		nodes[i] = m_Node[f.node[i]].m_rt;
	}
	return FaceArea(nodes, N);
}

//-----------------------------------------------------------------------------
double FEMeshBase::FaceArea(const vector<vec3f>& r, int faceType)
{
	switch (faceType)
	{
	case 3: 
		{
			return triangle_area(r[0], r[1], r[2]);
		}
		break;
	case 6:
		{
			double A = 0.0;
			A += triangle_area(r[0], r[3], r[5]);
			A += triangle_area(r[3], r[1], r[4]);
			A += triangle_area(r[2], r[5], r[4]);
			A += triangle_area(r[3], r[4], r[5]);
			return A;
		}
		break;
	case 7:
		{
			return triangle_area(r[0], r[1], r[2]);
		}
		break;
	case 4:
		{
			int i, n;

			// gauss-point data
			const float a = 1.f / (float) sqrt(3.0);
			const int NELN = 4;
			const int NINT = 4;
			static float gr[NINT] = { -a,  a,  a, -a };
			static float gs[NINT] = { -a, -a,  a,  a };
			static float gw[NINT] = {  1,  1,  1,  1 };

			static float H[NINT][NELN] = {0};
			static float Gr[NINT][NELN] = {0};
			static float Gs[NINT][NELN] = {0};
			static bool bfirst = true;

			if (bfirst)
			{

				// calculate shape function values at gauss points
				for (n=0; n<NINT; ++n)
				{
					H[n][0] = 0.25f*(1 - gr[n])*(1 - gs[n]);
					H[n][1] = 0.25f*(1 + gr[n])*(1 - gs[n]);
					H[n][2] = 0.25f*(1 + gr[n])*(1 + gs[n]);
					H[n][3] = 0.25f*(1 - gr[n])*(1 + gs[n]);
				}

				// calculate local derivatives of shape functions at gauss points
				for (n=0; n<NINT; ++n)
				{
					Gr[n][0] = -0.25f*(1 - gs[n]);
					Gr[n][1] =  0.25f*(1 - gs[n]);
					Gr[n][2] =  0.25f*(1 + gs[n]);
					Gr[n][3] = -0.25f*(1 + gs[n]);

					Gs[n][0] = -0.25f*(1 - gr[n]);
					Gs[n][1] = -0.25f*(1 + gr[n]);
					Gs[n][2] =  0.25f*(1 + gr[n]);
					Gs[n][3] =  0.25f*(1 - gr[n]);
				}

				bfirst = false;
			}

			float A = 0.f;
			for (n=0; n<NINT; ++n)
			{
				// calculate jacobian
				vec3f dxr, dxs;
				for (i=0; i<NELN; ++i)
				{
					dxr.x += Gr[n][i]*r[i].x;
					dxr.y += Gr[n][i]*r[i].y;
					dxr.z += Gr[n][i]*r[i].z;

					dxs.x += Gs[n][i]*r[i].x;
					dxs.y += Gs[n][i]*r[i].y;
					dxs.z += Gs[n][i]*r[i].z;
				}

				float detJ = (dxr ^ dxs).Length();

				A += gw[n]*detJ;
			}

			return A;
		}
		break;
	}

	return 0.0;
}


//-----------------------------------------------------------------------------
// Calculate the volume of an element
float FEMeshBase::ElementVolume(int iel)
{
	FEElement& el = Element(iel);
	switch (el.Type())
	{
	case FE_HEX8   : return HexVolume(el); break;
	case FE_HEX20  : return HexVolume(el); break;
	case FE_HEX27  : return HexVolume(el); break;
	case FE_TET4   : return TetVolume(el); break;
    case FE_TET10  : return TetVolume(el); break;
    case FE_TET15  : return TetVolume(el); break;
    case FE_TET20  : return TetVolume(el); break;
	case FE_PENTA6 : return PentaVolume(el); break;
    case FE_PENTA15: return PentaVolume(el); break;
	case FE_PYRA5  : return PyramidVolume(el); break;
    }

	return 0.f;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a hex element
float FEMeshBase::HexVolume(const FEElement& el)
{
	assert((el.Type() == FE_HEX8) || (el.Type() == FE_HEX20) || (el.Type() == FE_HEX27));

	// gauss-point data
	const float a = 1.f / (float) sqrt(3.0);
	const int NELN = 8;
	const int NINT = 8;
	static float gr[NINT] = { -a,  a,  a, -a, -a,  a, a, -a };
	static float gs[NINT] = { -a, -a,  a,  a, -a, -a, a,  a };
	static float gt[NINT] = { -a, -a, -a, -a,  a,  a, a,  a };
	static float gw[NINT] = {  1,  1,  1,  1,  1,  1,  1, 1 };

	static float H[NINT][NELN] = {0};
	static float Gr[NINT][NELN] = {0};
	static float Gs[NINT][NELN] = {0};
	static float Gt[NINT][NELN] = {0};
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n=0; n<NINT; ++n)
		{
			H[n][0] = 0.125f*(1 - gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][1] = 0.125f*(1 + gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][2] = 0.125f*(1 + gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][3] = 0.125f*(1 - gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][4] = 0.125f*(1 - gr[n])*(1 - gs[n])*(1 + gt[n]);
			H[n][5] = 0.125f*(1 + gr[n])*(1 - gs[n])*(1 + gt[n]);
			H[n][6] = 0.125f*(1 + gr[n])*(1 + gs[n])*(1 + gt[n]);
			H[n][7] = 0.125f*(1 - gr[n])*(1 + gs[n])*(1 + gt[n]);
		}

		// calculate local derivatives of shape functions at gauss points
		for (n=0; n<NINT; ++n)
		{
			Gr[n][0] = -0.125f*(1 - gs[n])*(1 - gt[n]);
			Gr[n][1] =  0.125f*(1 - gs[n])*(1 - gt[n]);
			Gr[n][2] =  0.125f*(1 + gs[n])*(1 - gt[n]);
			Gr[n][3] = -0.125f*(1 + gs[n])*(1 - gt[n]);
			Gr[n][4] = -0.125f*(1 - gs[n])*(1 + gt[n]);
			Gr[n][5] =  0.125f*(1 - gs[n])*(1 + gt[n]);
			Gr[n][6] =  0.125f*(1 + gs[n])*(1 + gt[n]);
			Gr[n][7] = -0.125f*(1 + gs[n])*(1 + gt[n]);

			Gs[n][0] = -0.125f*(1 - gr[n])*(1 - gt[n]);
			Gs[n][1] = -0.125f*(1 + gr[n])*(1 - gt[n]);
			Gs[n][2] =  0.125f*(1 + gr[n])*(1 - gt[n]);
			Gs[n][3] =  0.125f*(1 - gr[n])*(1 - gt[n]);
			Gs[n][4] = -0.125f*(1 - gr[n])*(1 + gt[n]);
			Gs[n][5] = -0.125f*(1 + gr[n])*(1 + gt[n]);
			Gs[n][6] =  0.125f*(1 + gr[n])*(1 + gt[n]);
			Gs[n][7] =  0.125f*(1 - gr[n])*(1 + gt[n]);

			Gt[n][0] = -0.125f*(1 - gr[n])*(1 - gs[n]);
			Gt[n][1] = -0.125f*(1 + gr[n])*(1 - gs[n]);
			Gt[n][2] = -0.125f*(1 + gr[n])*(1 + gs[n]);
			Gt[n][3] = -0.125f*(1 - gr[n])*(1 + gs[n]);
			Gt[n][4] =  0.125f*(1 - gr[n])*(1 - gs[n]);
			Gt[n][5] =  0.125f*(1 + gr[n])*(1 - gs[n]);
			Gt[n][6] =  0.125f*(1 + gr[n])*(1 + gs[n]);
			Gt[n][7] =  0.125f*(1 - gr[n])*(1 + gs[n]);
		}

		bfirst = false;
	}

	float *Grn, *Gsn, *Gtn;
	float vol = 0, detJ;
	float J[3][3];
	int i, n;

	vec3f rt[NELN];
	for (i=0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].m_rt;

	for (n=0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i=0; i<NELN; ++i)
		{
			const float& Gri = Grn[i];
			const float& Gsi = Gsn[i];
			const float& Gti = Gtn[i];

			const float& x = rt[i].x;
			const float& y = rt[i].y;
			const float& z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0]*(J[1][1]*J[2][2] - J[1][2]*J[2][1]) 
			+ J[0][1]*(J[1][2]*J[2][0] - J[2][2]*J[1][0]) 
			+ J[0][2]*(J[1][0]*J[2][1] - J[1][1]*J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pentahedral element
float FEMeshBase::PentaVolume(const FEElement& el)
{
	assert((el.Type() == FE_PENTA6) || (el.Type() == FE_PENTA15));

	// gauss-point data
	//gauss intergration points
	const float a = 1.f/6.f;
	const float b = 2.f/3.f;
	const float c = 1.f / (float) sqrt(3.0);
	const float w = 1.f / 6.f;

	const int NELN = 6;
	const int NINT = 6;

	static float gr[NINT] = { a, b, a, a, b, a };
	static float gs[NINT] = { a, a, b, a, a, b };
	static float gt[NINT] = { -c, -c, -c, c, c, c };
	static float gw[NINT] = { w, w, w, w, w, w };

	static float H[NINT][NELN] = {0};
	static float Gr[NINT][NELN] = {0};
	static float Gs[NINT][NELN] = {0};
	static float Gt[NINT][NELN] = {0};
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n=0; n<NINT; ++n)
		{
			H[n][0] = 0.5f*(1.f - gt[n])*(1.f - gr[n] - gs[n]);
			H[n][1] = 0.5f*(1.f - gt[n])*gr[n];
			H[n][2] = 0.5f*(1.f - gt[n])*gs[n];
			H[n][3] = 0.5f*(1.f + gt[n])*(1.f - gr[n] - gs[n]);
			H[n][4] = 0.5f*(1.f + gt[n])*gr[n];
			H[n][5] = 0.5f*(1.f + gt[n])*gs[n];
		}

		// calculate local derivatives of shape functions at gauss points
		for (n=0; n<NINT; ++n)
		{
			Gr[n][0] = -0.5f*(1.f - gt[n]);
			Gr[n][1] =  0.5f*(1.f - gt[n]);
			Gr[n][2] =  0.0f;
			Gr[n][3] = -0.5f*(1.f + gt[n]);
			Gr[n][4] =  0.5f*(1.f + gt[n]);
			Gr[n][5] =  0.0f;

			Gs[n][0] = -0.5f*(1.f - gt[n]);
			Gs[n][1] =  0.0f;
			Gs[n][2] =  0.5f*(1.f - gt[n]);
			Gs[n][3] = -0.5f*(1.f + gt[n]);
			Gs[n][4] =  0.0f;
			Gs[n][5] =  0.5f*(1.f + gt[n]);

			Gt[n][0] = -0.5f*(1.f - gr[n] - gs[n]);
			Gt[n][1] = -0.5f*gr[n];
			Gt[n][2] = -0.5f*gs[n];
			Gt[n][3] =  0.5f*(1.f - gr[n] - gs[n]);
			Gt[n][4] =  0.5f*gr[n];
			Gt[n][5] =  0.5f*gs[n];
		}

		bfirst = false;
	}

	float *Grn, *Gsn, *Gtn;
	float vol = 0, detJ;
	float J[3][3];
	int i, n;

	vec3f rt[NELN];
	for (i=0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].m_rt;

	for (n=0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i=0; i<NELN; ++i)
		{
			const float& Gri = Grn[i];
			const float& Gsi = Gsn[i];
			const float& Gti = Gtn[i];

			const float& x = rt[i].x;
			const float& y = rt[i].y;
			const float& z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0]*(J[1][1]*J[2][2] - J[1][2]*J[2][1]) 
			+ J[0][1]*(J[1][2]*J[2][0] - J[2][2]*J[1][0]) 
			+ J[0][2]*(J[1][0]*J[2][1] - J[1][1]*J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pyramid element
float FEMeshBase::PyramidVolume(const FEElement& el)
{
	assert(el.Type() == FE_PYRA5);

	// gauss-point data
	const float a = 1.f / (float)sqrt(3.0);
	const int NELN = 5;
	const int NINT = 8;
	static float gr[NINT] = { -a, a, a, -a, -a, a, a, -a };
	static float gs[NINT] = { -a, -a, a, a, -a, -a, a, a };
	static float gt[NINT] = { -a, -a, -a, -a, a, a, a, a };
	static float gw[NINT] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	static float H[NINT][NELN] = { 0 };
	static float Gr[NINT][NELN] = { 0 };
	static float Gs[NINT][NELN] = { 0 };
	static float Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n = 0; n<NINT; ++n)
		{
			H[n][0] = 0.125f*(1 - gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][1] = 0.125f*(1 + gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][2] = 0.125f*(1 + gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][3] = 0.125f*(1 - gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][4] = 0.5f*(1 + gt[n]);
		}

		// calculate local derivatives of shape functions at gauss points
		for (n = 0; n<NINT; ++n)
		{
			float r = gr[n], s = gs[n], t = gt[n];
			Gr[n][0] = -0.125f*(1.f - s)*(1.f - t);
			Gr[n][1] =  0.125f*(1.f - s)*(1.f - t);
			Gr[n][2] =  0.125f*(1.f + s)*(1.f - t);
			Gr[n][3] = -0.125f*(1.f + s)*(1.f - t);
			Gr[n][4] = 0.f;

			Gs[n][0] = -0.125f*(1.f - r)*(1.f - t);
			Gs[n][1] = -0.125f*(1.f + r)*(1.f - t);
			Gs[n][2] =  0.125f*(1.f + r)*(1.f - t);
			Gs[n][3] =  0.125f*(1.f - r)*(1.f - t);
			Gs[n][4] =  0.f;

			Gt[n][0] = -0.125f*(1.f - r)*(1.f - s);
			Gt[n][1] = -0.125f*(1.f + r)*(1.f - s);
			Gt[n][2] = -0.125f*(1.f + r)*(1.f + s);
			Gt[n][3] = -0.125f*(1.f - r)*(1.f + s);
			Gt[n][4] = 0.5f;
		}

		bfirst = false;
	}

	float *Grn, *Gsn, *Gtn;
	float vol = 0, detJ;
	float J[3][3];
	int i, n;

	vec3f rt[NELN];
	for (i = 0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].m_rt;

	for (n = 0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i = 0; i<NELN; ++i)
		{
			const float& Gri = Grn[i];
			const float& Gsi = Gsn[i];
			const float& Gti = Gtn[i];

			const float& x = rt[i].x;
			const float& y = rt[i].y;
			const float& z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a tetrahedral element
float FEMeshBase::TetVolume(const FEElement& el)
{
	assert((el.Type() == FE_TET4) || (el.Type() == FE_TET10)
           || (el.Type() == FE_TET15) || (el.Type() == FE_TET20));

	// gauss-point data
	const float a = 0.58541020f;
	const float b = 0.13819660f;
	const float w = 1.f / 24.f;

	const int NELN = 4;
	const int NINT = 4;

	static float gr[NINT] = { b, a, b, b };
	static float gs[NINT] = { b, b, a, b };
	static float gt[NINT] = { b, b, b, a };
	static float gw[NINT] = { w, w, w, w };

	static float H[NINT][NELN] = {0};
	static float Gr[NINT][NELN] = {0};
	static float Gs[NINT][NELN] = {0};
	static float Gt[NINT][NELN] = {0};
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n=0; n<NINT; ++n)
		{
			H[n][0] = 1.f - gr[n] - gs[n] - gt[n];
			H[n][1] = gr[n];
			H[n][2] = gs[n];
			H[n][3] = gt[n];
		}

		// calculate local derivatives of shape functions at gauss points
		for (n=0; n<NINT; ++n)
		{
			Gr[n][0] = -1.f;
			Gr[n][1] =  1.f;
			Gr[n][2] =  0.f;
			Gr[n][3] =  0.f;

			Gs[n][0] = -1.f;
			Gs[n][1] =  0.f;
			Gs[n][2] =  1.f;
			Gs[n][3] =  0.f;

			Gt[n][0] = -1.f;
			Gt[n][1] =  0.f;
			Gt[n][2] =  0.f;
			Gt[n][3] =  1.f;
		}

		bfirst = false;
	}

	float *Grn, *Gsn, *Gtn;
	float vol = 0, detJ;
	float J[3][3];
	int i, n;

	vec3f rt[NELN];
	for (i=0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].m_rt;

	for (n=0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i=0; i<NELN; ++i)
		{
			const float& Gri = Grn[i];
			const float& Gsi = Gsn[i];
			const float& Gti = Gtn[i];

			const float& x = rt[i].x;
			const float& y = rt[i].y;
			const float& z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0]*(J[1][1]*J[2][2] - J[1][2]*J[2][1]) 
			+ J[0][1]*(J[1][2]*J[2][0] - J[2][2]*J[1][0]) 
			+ J[0][2]*(J[1][0]*J[2][1] - J[1][1]*J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
void FEMeshBase::FaceNodePosition(const FEFace& f, vec3f* r) const
{
	switch (f.m_ntype)
	{
	case FACE_TRI10:
		r[9] = m_Node[f.node[9]].m_rt;
	case FACE_QUAD9:
		r[8] = m_Node[f.node[8]].m_rt;
	case FACE_QUAD8:
		r[7] = m_Node[f.node[7]].m_rt;
	case FACE_TRI7:
		r[6] = m_Node[f.node[6]].m_rt;
	case FACE_TRI6:
		r[5] = m_Node[f.node[5]].m_rt;
		r[4] = m_Node[f.node[4]].m_rt;
	case FACE_QUAD4:
		r[3] = m_Node[f.node[3]].m_rt;
	case FACE_TRI3:
		r[2] = m_Node[f.node[2]].m_rt;
		r[1] = m_Node[f.node[1]].m_rt;
		r[0] = m_Node[f.node[0]].m_rt;
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEMeshBase::FaceNodeNormals(FEFace& f, vec3f* n)
{
	switch (f.m_ntype)
	{
	case FACE_TRI10:
		n[9] = f.m_nn[9];
	case FACE_QUAD9:
		n[8] = f.m_nn[8];
	case FACE_QUAD8:
		n[7] = f.m_nn[7];
	case FACE_TRI7:
		n[6] = f.m_nn[6];
	case FACE_TRI6:
		n[5] = f.m_nn[5];
		n[4] = f.m_nn[4];
	case FACE_QUAD4:
		n[3] = f.m_nn[3];
	case FACE_TRI3:
		n[2] = f.m_nn[2];
		n[1] = f.m_nn[1];
		n[0] = f.m_nn[0];
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEMeshBase::FaceNodeTexCoords(FEFace& f, float* t, bool bnode)
{
	if (bnode)
	{
		for (int i=0; i<f.Nodes(); ++i) t[i] = m_Node[f.node[i]].m_tex;
	}
	else
	{
		for (int i=0; i<f.Nodes(); ++i) t[i] = f.m_tex[i];
	}
}

//-----------------------------------------------------------------------------
bool IsInsideElement(FEElement& el, double r[3], const double tol)
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
void project_inside_element(FEElement& el, const vec3f& p, double r[3], vec3f* x)
{
	const double tol = 0.0001;
	const int nmax = 10;

	int ne = el.Nodes();
	double dr[3], R[3];
	Mat3d K;
	double u2, N[FEGenericElement::MAX_NODES], G[3][FEGenericElement::MAX_NODES];
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
bool Post::ProjectInsideReferenceElement(FEMeshBase& m, FEElement& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FEGenericElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = m.Node(el.m_node[i]).m_r0;

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool Post::ProjectInsideElement(FEMeshBase& m, FEElement& el, const vec3f& p, double r[3])
{
	r[0] = r[1] = r[2] = 0.f;
	int ne = el.Nodes();
	vec3f x[FEGenericElement::MAX_NODES];
	for (int i = 0; i<ne; ++i) x[i] = m.Node(el.m_node[i]).m_rt;

	project_inside_element(el, p, r, x);

	return IsInsideElement(el, r, 0.001);
}

//-----------------------------------------------------------------------------
bool Post::FindElementRef(FEMeshBase& m, const vec3f& p, int& nelem, double r[3])
{
	vec3f y[FEGenericElement::MAX_NODES];
	int NE = m.Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& e = m.Element(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3f r0 = m.Node(e.m_node[0]).m_rt;
		vec3f r1 = r0;
		for (int j=1; j<ne; ++j)
		{
			vec3f& rj = m.Node(e.m_node[j]).m_rt;
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
	vec3f y[FEGenericElement::MAX_NODES];
	int NE = m.Elements();
	for (int i = 0; i<NE; ++i)
	{
		FEElement& e = m.Element(i);
		int ne = e.Nodes();
		nelem = i;

		// do a quick bounding box test
		vec3f r0 = m.Node(e.m_node[0]).m_r0;
		vec3f r1 = r0;
		for (int j = 1; j<ne; ++j)
		{
			vec3f& rj = m.Node(e.m_node[j]).m_r0;
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

int FEMeshBase::CountSelectedFaces() const
{
	int N = 0, NF = Faces();
	for (int i = 0; i<NF; ++i)
	{
		if (Face(i).IsSelected()) N++;
	}
	return N;
}

void FEMeshBase::SetNodeTags(int ntag)
{
	for (int i=0; i<Nodes(); ++i) Node(i).m_ntag = ntag;
}

void FEMeshBase::SetEdgeTags(int ntag)
{
	for (int i = 0; i<Edges(); ++i) Edge(i).m_ntag = ntag;
}

void FEMeshBase::SetFaceTags(int ntag)
{
	for (int i = 0; i<Faces(); ++i) Face(i).m_ntag = ntag;
}

void FEMeshBase::SetElementTags(int ntag)
{
	for (int i = 0; i<Elements(); ++i) Element(i).m_ntag = ntag;
}

//=================================================================================================

FEFindElement::BOX::BOX()
{
	m_elem = -1;
	m_level = -1;
}

FEFindElement::BOX::~BOX()
{
	for (size_t i=0; i<m_child.size(); ++i) delete m_child[i];
	m_child.clear();
}

void FEFindElement::BOX::split(int levels)
{
	m_level = levels;
	if (m_level == 0) return;

	float x0 = m_box.x0, x1 = m_box.x1;
	float y0 = m_box.y0, y1 = m_box.y1;
	float z0 = m_box.z0, z1 = m_box.z1;

	float dx = x1 - x0;
	float dy = y1 - y0;
	float dz = z1 - z0;

	m_child.clear();
	for (int i=0; i<2; i++)
		for (int j = 0; j<2; j++)
			for (int k = 0; k <2; k++)
			{
				float xa = x0 + i*dx*0.5f, xb = x0 + (i + 1)*dx*0.5f;
				float ya = y0 + j*dy*0.5f, yb = y0 + (j + 1)*dy*0.5f;
				float za = z0 + k*dz*0.5f, zb = z0 + (k + 1)*dz*0.5f;

				BOUNDINGBOX b(xa, ya, za, xb, yb, zb);
				float R = b.GetMaxExtent();
				b.Inflate(R*0.0001f);

				BOX* box = new BOX;
				box->m_box = b;

				m_child.push_back(box);
			}

	for (int i=0; i<(int)m_child.size(); ++i)
	{
		m_child[i]->split(levels - 1);
	}
}

void FEFindElement::BOX::Add(BOUNDINGBOX& b, int nelem)
{
	if (m_level == 0)
	{
		if (m_box.Intersects(b))
		{
			BOX* box = new BOX;
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


FEFindElement::BOX* FEFindElement::BOX::Find(const vec3f& r)
{
	if (m_level == 0)
	{
		bool inside = m_box.IsInside(r);
		return (inside ? this : 0);
	}

	// try to find the child
	for (size_t i = 0; i<m_child.size(); ++i)
	{
		BOX* c = m_child[i];
		BOX* ret = c->Find(r);
		if (ret) return ret;
	}

	return 0;
}


FEFindElement::BOX* FEFindElement::FindBox(const vec3f& r)
{
	// make sure it's in the master box
	if (m_bound.IsInside(r) == false) return 0;

	// try to find the child
	BOX* b = &m_bound;
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
			BOX* c = b->m_child[i];
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

FEFindElement::FEFindElement(FEMeshBase& mesh) : m_mesh(mesh)
{
	m_nframe = -1;
}

void FEFindElement::InitReferenceFrame(vector<bool>& flags)
{
	assert(m_nframe == 0);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3f r = m_mesh.Node(0).m_r0;
	BOUNDINGBOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = m_mesh.Node(i).m_r0;
		box += r;
	}
	float R = box.GetMaxExtent();
	box.Inflate(R*0.001f);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FEElement& e = m_mesh.Element(i);

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
			vec3f r0 = m_mesh.Node(e.m_node[0]).m_r0;
			vec3f r1 = r0;
			BOUNDINGBOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3f& rj = m_mesh.Node(e.m_node[j]).m_r0;
				box += rj;
			}
			float R = box.GetMaxExtent();
			box.Inflate(R*0.001f);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void FEFindElement::InitCurrentFrame(vector<bool>& flags)
{
	assert(m_nframe == 1);

	// calculate bounding box for the entire mesh
	int NN = m_mesh.Nodes();
	int NE = m_mesh.Elements();
	if ((NN == 0) || (NE == 0)) return;

	vec3f r = m_mesh.Node(0).m_rt;
	BOUNDINGBOX box(r, r);
	for (int i = 1; i<m_mesh.Nodes(); ++i)
	{
		r = m_mesh.Node(i).m_rt;
		box += r;
	}
	float R = box.GetMaxExtent();
	box.Inflate(R*0.001f);

	// split this box recursively
	m_bound.m_box = box;

	int l = (int)(log(NE) / log(8.0));
	if (l < 0) l = 0;
	if (l > 3) l = 3;
	m_bound.split(l);

	// calculate bounding boxes for all elements
	int cflags = flags.size();
	for (int i = 0; i<NE; ++i)
	{
		FEElement& e = m_mesh.Element(i);

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
			vec3f r0 = m_mesh.Node(e.m_node[0]).m_rt;
			vec3f r1 = r0;
			BOUNDINGBOX box(r0, r1);
			for (int j = 1; j<ne; ++j)
			{
				vec3f& rj = m_mesh.Node(e.m_node[j]).m_rt;
				box += rj;
			}
			float R = box.GetMaxExtent();
			box.Inflate(R*0.001f);

			// add it to the octree
			m_bound.Add(box, i);
		}
	}
}

void FEFindElement::Init(int nframe)
{
	vector<bool> dummy;
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(dummy);
	else InitCurrentFrame(dummy);
}

void FEFindElement::Init(vector<bool>& flags, int nframe)
{
	m_nframe = nframe;
	if (m_nframe == 0) InitReferenceFrame(flags);
	else InitCurrentFrame(flags);
}

bool FEFindElement::FindInReferenceFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 0);

	vec3f y[FEGenericElement::MAX_NODES];
	BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FEElement& e = m_mesh.Element(nid);
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

bool FEFindElement::FindInCurrentFrame(const vec3f& x, int& nelem, double r[3])
{
	assert(m_nframe == 1);

	vec3f y[FEGenericElement::MAX_NODES];
	BOX* b = FindBox(x);
	if (b == 0) return false;
	assert(b->m_level == 0);

	int NE = (int)b->m_child.size();
	for (int i = 0; i<NE; ++i)
	{
		BOX* c = b->m_child[i];
		assert(c->m_level == -1);

		int nid = c->m_elem; assert(nid >= 0);

		FEElement& e = m_mesh.Element(nid);
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
