#include "stdafx.h"
#include "FETetGenMesher.h"
#include <GeomLib/GObject.h>
#include <GeomLib/geom.h>
#include <GeomLib/GSurfaceMeshObject.h>
#include "FEModifier.h"

//-----------------------------------------------------------------------------
// We need to undefine it since tetgen defines this parameter
#undef PI

//-----------------------------------------------------------------------------
#ifdef TETLIBRARY
	#include <tetgen.h>
#endif

//-----------------------------------------------------------------------------
// Constructor
FETetGenMesher::FETetGenMesher(GObject* po) : m_po(po)
{
	AddDoubleParam(  0, "elsize", "Element Size");
	AddDoubleParam(2.0, "Quality", "Quality");
	AddIntParam(0, "eltype", "element type")->SetEnumNames("Tet4\0Tet10\0Tet15\0Tet20\0");
	AddBoolParam(true, "splitfaces", "split faces");
	AddBoolParam(false, "hole", "hole");
	AddVecParam(vec3d(0,0,0), "", "");
}

//-----------------------------------------------------------------------------
double FETetGenMesher::ElementSize()
{
	return GetParam(0).GetFloatValue();
}

//-----------------------------------------------------------------------------
bool FETetGenMesher::SplitFaces()
{
	return GetParam(SPLIT_FACES).GetBoolValue();
}

//-----------------------------------------------------------------------------
double FETetGenMesher::ElementQuality()
{
	return GetParam(QUALITY).GetFloatValue();
}

//-----------------------------------------------------------------------------
int FETetGenMesher::ElementType()
{
	int type = GetIntValue(ELTYPE);
	switch (type)
	{
	case 0: return FE_TET4; break;
	case 1: return FE_TET10; break;
	case 2: return FE_TET15; break;
	case 3: return FE_TET20; break;
	}
	assert(false);
	return FE_INVALID_ELEMENT_TYPE;
}

//-----------------------------------------------------------------------------
// This function first converts the object to the tetgen PLC data structure
// and then passes this structure to tetgen which builds the tet mesh. On
// a successful return the FE mesh is processed and partitioned. 
//
FEMesh* FETetGenMesher::BuildMesh()
{
	GSurfaceMeshObject* surfObj = dynamic_cast<GSurfaceMeshObject*>(m_po);
	if (surfObj)
	{
		return CreateMesh(surfObj->GetSurfaceMesh());
	}

#ifdef TETLIBRARY
	// allocate tetgen structures
	tetgenio in, out;
	in.initialize();
	out.initialize();

	bool bremesh = false;

	// build the tetgen input structure
	if (bremesh == false)
	{
		if (build_tetgen_in(in) == false) return 0;
	}
	else
	{
		if (build_tetgen_in_remesh(in) == false) return 0;
	}

	// get the meshing parameters
	double q = GetFloatValue(QUALITY);
	double a = GetFloatValue(ELSIZE);
	double tol = 0;
	double v = a*a*a/6.0;
	bool bsplit = GetBoolValue(SPLIT_FACES);

	// do the triangulation
	char sz[64] = {0};
	char* ch = sz;
	if (bremesh) sprintf(ch, "r"); else sprintf(ch, "p"); ++ch;
	if (q   > 0) sprintf(ch, "q%lg", q); ch += strlen(ch);
	if (v   > 0) sprintf(ch, "a%lg", v); ch += strlen(ch);
	if (tol > 0) sprintf(ch, "T%lg", tol); ch += strlen(ch);
	if (bsplit == false) sprintf(ch, "Y"); ++ch;
//	sprintf(ch, "m"); ++ch;

	try
	{
		tetrahedralize(sz, &in, &out);
	}
	catch (...)
	{
//		flx_error("Tetgen was not able to mesh this object.");
		return 0;
	}

	// build and return a new tet mesh
	return build_tet_mesh(out);

#else 
	return 0;
#endif // TETLIBRARY
}

//-----------------------------------------------------------------------------
#ifdef TETLIBRARY

bool FETetGenMesher::build_tetgen_in(tetgenio& in)
{
	int i, j, n;

	// get the requested element size
	double h = GetFloatValue(ELSIZE);
	if (h == 0) h = 1.0;

	// Build a PLC from the object
	PLC plc;
	if (plc.Build(m_po, h) == false) return false;

	// all indices start from 0
	in.firstnumber = 0;

	// allocate nodes
	int NN = plc.Nodes();
	in.numberofpoints = NN;
	in.pointlist = new REAL[3*NN];
	for (i=0, n=0; i<NN; ++i, ++n) 
	{
		vec3d& r = plc.Node(i).r;
		in.pointlist[3*n  ] = r.x;
		in.pointlist[3*n+1] = r.y;
		in.pointlist[3*n+2] = r.z;
	}
/*	in.pointmtrlist = new REAL[NN];
	for (i=0; i<NN; ++i) in.pointmtrlist[i] = 0.2;
	in.pointmtrlist[0] = 0.02;
	in.numberofpointmtrs = 1;
*/
	// allocate faces
	int NF = plc.Faces();
	in.numberoffacets = NF;
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];
	for (i=0, n = 0; i<NF; ++i)
	{
		tetgenio::facet* pf; 
		tetgenio::polygon* pp;

		PLC::FACE& f = plc.Face(i);
		int nf = f.Nodes();

		pf = &in.facetlist[n++];
		pf->numberofpolygons = 1;
		pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
		pf->numberofholes = 0;
		pf->holelist = 0;

		pp = &(pf->polygonlist[0]);
		pp->numberofvertices = nf;
		pp->vertexlist = new int[pp->numberofvertices];
		for (j=0; j<nf; ++j) pp->vertexlist[j] = f.node[j];

		in.facetmarkerlist[i] = f.nid;
	}

	// allocate edges
	int nedges = 0;
	for (int i=0; i<plc.Edges(); ++i) nedges += plc.Edge(i).Nodes()-1;
	in.numberofedges = nedges;
	in.edgelist = new int[2*in.numberofedges];
	in.edgemarkerlist = new int[in.numberofedges];
	for (int i=0, n=0; i<plc.Edges(); ++i)
	{
		PLC::EDGE& e = plc.Edge(i);
		for (int j=0; j<e.Nodes()-1; ++j, ++n)
		{
			in.edgelist[2*n  ] = e.node[j];
			in.edgelist[2*n+1] = e.node[j+1];
			in.edgemarkerlist[n] = e.nid+2;
		}
	}

	// holes
	bool hole = GetBoolValue(HOLE);
	if (hole)
	{
		vec3d ri = GetVecValue(HOLE_COORD);
		int nh = 1;
		in.numberofholes = nh;
		in.holelist = new double[3 * nh];

		in.holelist[0] = ri.x;
		in.holelist[1] = ri.y;
		in.holelist[2] = ri.z;
	}

	return true;
}

bool FETetGenMesher::build_tetgen_in_remesh(tetgenio& in)
{
	// get the FE mesh
	FEMesh& mesh = *m_po->GetFEMesh();

	// make sure this is a tet mesh
	for (int i=0; i<mesh.Elements(); ++i)
	{
		FEElement& el = mesh.Element(i);
		if (el.Type() != FE_TET4) return false;
	}

	// build the node list
	int NN = mesh.Nodes();
	in.numberofpoints = NN;
	in.pointlist = new REAL[3*NN];
	for (int i=0; i<NN; ++i) 
	{
		vec3d& r = mesh.Node(i).r;
		in.pointlist[3*i  ] = r.x;
		in.pointlist[3*i+1] = r.y;
		in.pointlist[3*i+2] = r.z;
	}

	// build the element list
	int NE = mesh.Elements();
	in.numberoftetrahedra = NE;
	in.tetrahedronlist = new int[NE*4];
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = mesh.Element(i);
		in.tetrahedronlist[4*i  ] = el.m_node[0];
		in.tetrahedronlist[4*i+1] = el.m_node[1];
		in.tetrahedronlist[4*i+2] = el.m_node[2];
		in.tetrahedronlist[4*i+3] = el.m_node[3];
	}

	return true;
}


/*
void FETetGenMesher::build_tetgen_in(tetgenio& in)
{
	int i, j, n;

	// get the requested element size
	double h = m_Param.GetFloatValue(ELSIZE);
	if (h == 0) h = 1.0;

	// all indices start from 0
	in.firstnumber = 0;

	// 1. Find all nodes that are being used.
	//    Some nodes are only used to construct geometry (e.g. the center node for arcs)
	//    TODO: Maybe I should add them
	int NN = m_po->Nodes();
	int NE = m_po->Edges();
	for (i=0; i<NN; ++i) m_po->Node(i)->m_ntag = 0;
	for (i=0; i<NE; ++i)
	{
		GEdge& e = *m_po->Edge(i);
		m_po->Node(e.m_node[0])->m_ntag = 1;
		m_po->Node(e.m_node[1])->m_ntag = 1;
	}

	// count them
	int nn = 0;
	for (i=0; i<NN; ++i) if (m_po->Node(i)->m_ntag == 1) nn++;

	// divide all (curved) edges into segments of size ~ h
	for (i=0; i<NE; ++i)
	{
		GEdge& e = *m_po->Edge(i);
		if (e.m_ntype == EDGE_3P_CIRC_ARC)
		{
			double l = e.Length();
			int n = (int) (l / h) + 1;
			nn += n - 1;
		}
	}

	// allocate nodes
	in.numberofpoints = nn;
	in.pointlist = new REAL[3*nn];
	for (i=0, n=0; i<NN; ++i, ++n) 
	{
		if (m_po->Node(i)->m_ntag == 1)
		{
			vec3d& r = m_po->Node(i)->m_r;
			in.pointlist[3*n  ] = r.x;
			in.pointlist[3*n+1] = r.y;
			in.pointlist[3*n+2] = r.z;
		}
	}

	// allocate points on curved boundaries
	for (i=0; i<NE; ++i)
	{
		GEdge& e = *m_po->Edge(i);
		if (e.m_ntype == EGDE_3PARC)
		{
		}
	}

	// allocate faces
	int NF = m_po->Faces();
	in.numberoffacets = NF;
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];
	for (i=0, n = 0; i<NF; ++i)
	{
		tetgenio::facet* pf; 
		tetgenio::polygon* pp;

		GFace& f = *m_po->Face(i);
		int nf = f.Nodes();

		pf = &in.facetlist[n++];
		pf->numberofpolygons = 1;
		pf->polygonlist = new tetgenio::polygon[pf->numberofpolygons];
		pf->numberofholes = 0;
		pf->holelist = 0;

		pp = &(pf->polygonlist[0]);
		pp->numberofvertices = nf;
		pp->vertexlist = new int[pp->numberofvertices];
		for (j=0; j<nf; ++j) pp->vertexlist[j] = f.m_node[j];

		in.facetmarkerlist[i] = f.GetLocalID();
	}
}
*/

//-----------------------------------------------------------------------------
FEMesh* FETetGenMesher::build_tet_mesh(tetgenio& out)
{
	int i, j;

	// create a new mesh
	FEMesh* pmesh = new FEMesh;
	int nodes = out.numberofpoints;
	int elems = out.numberoftetrahedra;
	int faces = out.numberoftrifaces;

	// count the number of edges with a marker > 0
	int edges = 0;
	for (int i=0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1) edges++;
	}
	
	// allocate the mesh data
	pmesh->Create(nodes, elems, faces, edges);

	// copy nodes
	for (i=0; i<nodes; ++i)
	{
		FENode& node = pmesh->Node(i);
		vec3d& r = node.r;
		r.x = out.pointlist[3*i  ];
		r.y = out.pointlist[3*i+1];
		r.z = out.pointlist[3*i+2];
	}

	assert(out.numberofcorners == 4);

	// copy elements
	for (i=0; i<elems; ++i)
	{
		FEElement& el = pmesh->Element(i);
		el.SetType(FE_TET4);
		el.m_node[0] = out.tetrahedronlist[4*i  ];
		el.m_node[1] = out.tetrahedronlist[4*i+1];
		el.m_node[2] = out.tetrahedronlist[4*i+2];
		el.m_node[3] = out.tetrahedronlist[4*i+3];
		el.m_gid = 0;
	}

	// copy faces
	for (i=0; i<faces; ++i)
	{
		FEFace& f = pmesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.n[0] = out.trifacelist[3*i+2];
		f.n[1] = out.trifacelist[3*i+1];
		f.n[2] = out.trifacelist[3*i  ];
		f.n[3] = f.n[2];
		f.m_gid  = out.trifacemarkerlist[i];
	}

	// copy edges
	int n = 0;
	for (i=0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1)
		{
			FEEdge& e = pmesh->Edge(n++);
			e.SetType(FE_EDGE2);
			e.n[0] = out.edgelist[2*i  ];
			e.n[1] = out.edgelist[2*i+1];
			e.m_gid = out.edgemarkerlist[i]-2;
		}
	}

	if (GetIntValue(ELTYPE) == 1)
	{
		FEMesh* pold = pmesh;
		pmesh = build_tet10_mesh(pold);
		delete pold;
	}
	else if (GetIntValue(ELTYPE) == 2)
	{
		FETet4ToTet15 mod;
		pmesh->Update();
		FEMesh* pold = pmesh;
		pmesh = mod.Apply(pold);
		delete pold;
	}

	// update the element neighbours
	pmesh->UpdateElementNeighbors();

	// update faces
	pmesh->UpdateFaces();
	pmesh->AutoSmooth(60.0);

	pmesh->UpdateEdgeNeighbors();

	// update the mesh
	pmesh->MarkExteriorNodes();
	pmesh->UpdateBox();

	// associate the FE nodes with the GNodes
	double R2 = pmesh->GetBoundingBox().GetMaxExtent();
	if (R2 == 0) R2 = 1.0; else R2 *= R2;	
	for (i=0; i<pmesh->Nodes(); ++i)
	{
		FENode& node = pmesh->Node(i);
		vec3d& ri = node.r;
		node.m_gid = -1;
		for (j=0; j<m_po->Nodes(); ++j)
		{
			GNode& gn = *m_po->Node(j);
			vec3d& rj = gn.LocalPosition();
			double L2 = (ri - rj).SqrLength();
			if (L2/R2 < 1e-6) 
			{
				node.m_gid = j;
				break;
			}
		}
	}

	return pmesh;
}

//-----------------------------------------------------------------------------
FEMesh* FETetGenMesher::build_tet10_mesh(FEMesh* pm)
{
	const int EL[6][2] = {{0,1},{1,2},{2,0},{0,3},{1,3},{2,3}};

	int NN = pm->Nodes();
	int NF = pm->Faces();
	int NE = pm->Elements();
	int NC = pm->Edges();

	// find all the edges
	vector< vector<int> > NEL; NEL.resize(NN);
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i); assert(el.IsType(FE_TET4));
		for (int j=0; j<6; ++j)
		{
			int n0 = el.m_node[EL[j][0]];
			int n1 = el.m_node[EL[j][1]];
			assert(n0 != n1);
			if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }

			vector<int>& nel = NEL[n0];
			int nk = (int) nel.size();
			int k = 0;
			while ((k<nk)&&(nel[k] != n1)) k++;
			if (k == nk) nel.push_back(n1);
		}
	}

	// count edges
	int NL = 0;
	for (int i=0; i<NN; ++i) NL += (int) NEL[i].size();

	// create the edge table
	vector<pair<int, int> > ET; ET.reserve(NL);
	int m = 0;
	for (int i=0; i<NN; ++i)
	{
		vector<int>& nel = NEL[i];
		int nk = (int) nel.size();
		for (int k=0; k<nk; ++k) 
		{
			ET.push_back(pair<int, int>(i, nel[k]));
			nel[k] = m++;
		}
	}
	assert(NL == (int) ET.size());

	// create the element-edge table
	vector< vector<int> > EE; EE.assign(NE, vector<int>(6));
	for (int i=0; i<NE; ++i)
	{
		FEElement& e = pm->Element(i);
		vector<int>& ee = EE[i];
		for (int j=0; j<6; ++j)
		{
			int n0 = e.m_node[EL[j][0]];
			int n1 = e.m_node[EL[j][1]];
			if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }

			vector<int>& ne = NEL[n0];
			int nk = ne.size();
			for (int k=0; k<nk; ++k)
			{
				pair<int, int>& ek = ET[ne[k]];
				if ((ek.first == n0)&&(ek.second == n1))
				{
					ee[j] = ne[k];
					break;
				}
			}
		}
	}

	// create the face-edge table
	vector< vector<int> > FE; FE.assign(NF, vector<int>(3));
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		vector<int>& fe = FE[i];
		for (int j=0; j<3; ++j)
		{
			int n0 = f.n[EL[j][0]];
			int n1 = f.n[EL[j][1]];
			if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }

			vector<int>& ne = NEL[n0];
			int nk = ne.size();
			for (int k=0; k<nk; ++k)
			{
				pair<int, int>& ek = ET[ne[k]];
				if ((ek.first == n0)&&(ek.second == n1))
				{
					fe[j] = ne[k];
					break;
				}
			}
		}
	}

	// create the edge-edge table
	vector<int> CE; CE.assign(NC, -1);
	for (int i=0; i<NC; ++i)
	{
		FEEdge& e = pm->Edge(i);
		int n0 = e.n[0];
		int n1 = e.n[1];
		if (n0 > n1) { n0 ^= n1; n1 ^= n0; n0 ^= n1; }

		vector<int>& ne = NEL[n0];
		int nk = ne.size();
		for (int k=0; k<nk; ++k)
		{
			pair<int, int>& ek = ET[ne[k]];
			if ((ek.first == n0)&&(ek.second == n1))
			{
				CE[i] = ne[k];
				break;
			}
		}
	}

	// the total number of new nodes is the number of old nodes plus the number of edges
	int NN1 = NN + NL;

	// allocate a new mesh
	FEMesh* pnew = new FEMesh;
	pnew->Create(NN1, NE, NF, NC);

	// copy the old nodes
	for (int i=0; i<NN; ++i)
	{
		FENode& n0 = pm->Node(i);
		FENode& n1 = pnew->Node(i);
		n1.r = n0.r;
		n1.m_gid = n0.m_gid;
	}

	// create the new edge nodes
	for (int i=0; i<(int) ET.size(); ++i)
	{
		FENode& na = pm->Node(ET[i].first);
		FENode& nb = pm->Node(ET[i].second);

		FENode& n1 = pnew->Node(i + NN);
		n1.r = (na.r +nb.r)*0.5;
	}

	// create the elements
	for (int i=0; i<NE; ++i)
	{
		FEElement& e0 = pm->Element(i);
		FEElement& e1 = pnew->Element(i);

		e1.m_gid = e0.m_gid;

		e1.SetType(FE_TET10);
		e1.m_node[0] = e0.m_node[0];
		e1.m_node[1] = e0.m_node[1];
		e1.m_node[2] = e0.m_node[2];
		e1.m_node[3] = e0.m_node[3];

		e1.m_node[4] = EE[i][0] + NN;
		e1.m_node[5] = EE[i][1] + NN;
		e1.m_node[6] = EE[i][2] + NN;
		e1.m_node[7] = EE[i][3] + NN;
		e1.m_node[8] = EE[i][4] + NN;
		e1.m_node[9] = EE[i][5] + NN;
	}

	// create the new faces
	for (int i=0; i<NF; ++i)
	{
		FEFace& f0 = pm->Face(i);
		FEFace& f1 = pnew->Face(i);

		f1.SetType(FE_FACE_TRI6);
		f1.m_gid = f0.m_gid;
		f1.m_sid = f0.m_sid;
		f1.n[0] = f0.n[0];
		f1.n[1] = f0.n[1];
		f1.n[2] = f0.n[2];
		f1.n[3] = FE[i][0] + NN;
		f1.n[4] = FE[i][1] + NN;
		f1.n[5] = FE[i][2] + NN;
		f1.m_elem[0] = f0.m_elem[0];
		f1.m_elem[1] = f0.m_elem[1];
	}

	// create the new edges
	for (int i=0; i<NC; ++i)
	{
		FEEdge& e0 = pm->Edge(i);
		FEEdge& e1 = pnew->Edge(i);

		e1.SetType(FE_EDGE3);
		e1.n[0] = e0.n[0];
		e1.n[1] = e0.n[1];
		e1.n[2] = CE[i] + NN;
		e1.m_gid = e0.m_gid;
		e1.m_nbr[0] = e0.m_nbr[0];
		e1.m_nbr[1] = e0.m_nbr[1];
		e1.m_elem = e1.m_elem;
	}

	return pnew;
}

//=============================================================================
PLC::PLC()
{
	m_po = 0;
	m_h = 0.0;
}

//-----------------------------------------------------------------------------
int PLC::AddNode(vec3d r, int nid)
{
	NODE n;

	int l = -1;
	if (nid == -1)
	{
		n.r = r;
		n.nid = nid;
		m_Node.push_back(n);
		l = (int) m_Node.size()-1;
	}
	else
	{
		// find the node first
		l = FindNode(nid);
		if (l == -1)
		{
			n.r = r;
			n.nid = nid;
			m_Node.push_back(n);
			l = (int) m_Node.size()-1;
		}
	}

	return l;
}

//-----------------------------------------------------------------------------
int PLC::FindNode(int nid)
{
	for (int i=0; i<(int) m_Node.size(); ++i)
	{
		if (m_Node[i].nid == nid) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
bool PLC::Build(GObject *po, double h)
{
	m_po = po;
	m_h = h;
	if (BuildNodes() == false) return false;
	if (BuildEdges() == false) return false;
	if (BuildFaces() == false) return false;

	return true;
}

//-----------------------------------------------------------------------------
bool PLC::BuildNodes()
{
	int i;
	int NN = m_po->Nodes();
	int NE = m_po->Edges();

	// Find all nodes that are being used.
	// Some nodes are only used to construct geometry (e.g. the center node for arcs)
	for (i=0; i<NN; ++i) m_po->Node(i)->m_ntag = 0;
	for (i=0; i<NE; ++i)
	{
		GEdge& e = *m_po->Edge(i);
		m_po->Node(e.m_node[0])->m_ntag = 1;
		m_po->Node(e.m_node[1])->m_ntag = 1;
	}

	// add them to the PLC
	for (i=0; i<NN; ++i)
	{
		GNode& n = *m_po->Node(i);
		if (n.m_ntag == 1) AddNode(n.LocalPosition(), i);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool PLC::BuildEdges()
{
	// build the edges
	int NE = m_po->Edges();
	for (int i=0; i<NE; ++i)
	{
		GEdge& es = *m_po->Edge(i);
		EDGE ed;
		ed.nid = es.GetLocalID();
		ed.node.clear();

		switch (es.m_ntype)
		{
		case EDGE_LINE:
			ed.node.push_back(FindNode(es.m_node[0]));
			ed.node.push_back(FindNode(es.m_node[1]));
			break;
		case EDGE_3P_CIRC_ARC:
			{
				ed.node.push_back(FindNode(es.m_node[0]));
				vec3d r0 = m_po->Node(es.m_cnode)->LocalPosition();
				vec3d r1 = m_po->Node(es.m_node[0])->LocalPosition();
				vec3d r2 = m_po->Node(es.m_node[1])->LocalPosition();

				vec2d a(r0.x, r0.y);
				vec2d b(r1.x, r1.y);
				vec2d c(r2.x, r2.y);

				GM_CIRCLE_ARC ca(a, b, c);

				double L = ca.Length();
				int n = (int) (L/m_h)+1;
				if (n<3) n = 3;
				for (int j=1; j<n; ++j)
				{
					double l = (double) j / (double) n;
					vec3d r = ca.Point(l);
					r.z = r0.z;
					ed.node.push_back(AddNode(r, -1));
				}
				ed.node.push_back(FindNode(es.m_node[1]));
			}
			break;
		case EDGE_YARC:
			{
				ed.node.push_back(FindNode(es.m_node[0]));
				vec3d r0 = m_po->Node(es.m_node[0])->LocalPosition();
				vec3d r1 = m_po->Node(es.m_node[1])->LocalPosition();
				double y = r0.y;

				vec2d a(r0.x, r0.z);
				vec2d b(r1.x, r1.z);

				GM_CIRCLE_ARC c(vec2d(0,0), a, b);
				double L = c.Length();
				int n = (int) (L/m_h)+1;
				if (n<3) n = 3;
				for (int j=1; j<n; ++j)
				{
					double l = (double) j / (double) n;
					vec2d p = c.Point(l);
					vec3d r(p.x, y, p.y);
					ed.node.push_back(AddNode(r, -1));
				}
				ed.node.push_back(FindNode(es.m_node[1]));
			}
			break;
		default:
			assert(false);
			return false;
		}

		m_Edge.push_back(ed);
	}

	return true;
}

//-----------------------------------------------------------------------------
bool PLC::BuildFaces()
{
	// build the faces
	int NF = m_po->Faces();
	for (int i=0; i<NF; ++i)
	{
		GFace& fs = *m_po->Face(i);
		switch (fs.m_ntype)
		{
		case FACE_QUAD   : if (BuildFaceQuad   (fs) == false) return false; break;
		case FACE_POLYGON: if (BuildFacePolygon(fs) == false) return false; break;
		case FACE_EXTRUDE: if (BuildFaceExtrude(fs) == false) return false; break;
//		case FACE_REVOLVE: if (BuildFaceRevolve(fs) == false) return false; break;
		default:
			assert(false);
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
//! Adds a QUAD face to the PLC
bool PLC::BuildFaceQuad(GFace& fs)
{
	assert(fs.m_ntype == FACE_QUAD);
	FACE fd;
	assert((int) fs.m_node.size() == 4);
	fd.nid = fs.GetLocalID();
	fd.node.resize(4);
	fd.node[0] = fs.m_node[0];
	fd.node[1] = fs.m_node[1];
	fd.node[2] = fs.m_node[2];
	fd.node[3] = fs.m_node[3];
	m_Face.push_back(fd);
	return true;
}

//-----------------------------------------------------------------------------
//! Adds a polygon face to the PLC
bool PLC::BuildFacePolygon(GFace& fs)
{
	assert(fs.m_ntype == FACE_POLYGON);
	FACE fd;
	fd.nid = fs.GetLocalID();
	fd.node.clear();

	int ne = fs.m_edge.size();
	for (int j=0; j<ne; ++j)
	{
		EDGE& e = m_Edge[fs.m_edge[j].nid];
		int nwn = fs.m_edge[j].nwn;
		int n = e.Nodes();
		if (nwn == 1)
			for (int k=0; k<n-1; ++k) fd.node.push_back(e.node[k]);
		else
			for (int k=0; k<n-1; ++k) fd.node.push_back(e.node[n - k - 1]);
	}
	m_Face.push_back(fd);
	return true;
}

//-----------------------------------------------------------------------------
//! Adds an extruded face to the PLC
bool PLC::BuildFaceExtrude(GFace& fs)
{
	assert(fs.m_ntype == FACE_EXTRUDE);
	FACE fd;
	int ne = fs.m_edge.size();
	assert(ne == 4);
	EDGE& e0 = m_Edge[fs.m_edge[0].nid];
	EDGE& e1 = m_Edge[fs.m_edge[1].nid];
	EDGE& e2 = m_Edge[fs.m_edge[2].nid];
	EDGE& e3 = m_Edge[fs.m_edge[3].nid];

	assert(e0.Nodes() == e2.Nodes());
	assert(e1.Nodes() == 2);
	assert(e3.Nodes() == 2);

	int nf = e0.Nodes()-1;
	for (int j=0; j<nf; ++j)
	{
		fd.nid = fs.GetLocalID();
		fd.node.resize(4);
		fd.node[0] = e0.node[j];
		fd.node[1] = e0.node[j+1];
		fd.node[2] = e2.node[j+1];
		fd.node[3] = e2.node[j];
		m_Face.push_back(fd);
	}
	return true;
}

//-----------------------------------------------------------------------------
//! Adds an extruded face to the PLC
bool PLC::BuildFaceRevolve(GFace& fs)
{
	assert(fs.m_ntype == FACE_REVOLVE);
	FACE fd;
	return false;
}

//-----------------------------------------------------------------------------
bool FETetGenMesher::build_plc(FESurfaceMesh* pm, tetgenio& in)
{
	int i, j, n;

	// first we need to tag all nodes that define the boundary of the mesh
	for (i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;
	for (i = 0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		for (j = 0; j<f.Nodes(); ++j)
		{
			pm->Node(f.n[j]).m_ntag = 1;
		}
	}

	// count the nodes
	int nodes = 0;
	for (i = 0; i<pm->Nodes(); ++i) if (pm->Node(i).m_ntag == 1) ++nodes;

	// all indices start from 0
	in.firstnumber = 0;

	// allocate nodes
	in.numberofpoints = nodes;
	in.pointlist = new REAL[3 * nodes];
	vector<int> tag(pm->Nodes());
	for (i = 0, n = 0; i<pm->Nodes(); ++i)
	{
		if (pm->Node(i).m_ntag == 1)
		{
			vec3d& r = pm->Node(i).r;
			in.pointlist[3 * n] = r.x;
			in.pointlist[3 * n + 1] = r.y;
			in.pointlist[3 * n + 2] = r.z;
			tag[i] = n++;
		}
		else tag[i] = -1;
	}

	// count the faces
	int faces = 0;
	for (i = 0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		if (f.Type() == FE_FACE_TRI3) { ++faces; f.m_ntag = 1; }
		else
		{
			// see if this face is coplanar or if we need to split it in triangles
			vec3d r[4];
			r[0] = pm->Node(f.n[0]).r;
			r[1] = pm->Node(f.n[1]).r;
			r[2] = pm->Node(f.n[2]).r;
			r[3] = pm->Node(f.n[3]).r;

			faces += 2;
			f.m_ntag = 2;
		}
	}

	// allocate faces
	in.numberoffacets = faces;
	in.facetlist = new tetgenio::facet[in.numberoffacets];
	in.facetmarkerlist = new int[in.numberoffacets];
	for (i = 0, n = 0; i<pm->Faces(); ++i)
	{
		tetgenio::facet* pf;
		tetgenio::polygon* pp;
		FEFace& f = pm->Face(i);
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
	in.edgelist = new int[2 * in.numberofedges];
	in.edgemarkerlist = new int[in.numberofedges];
	for (int i = 0; i<in.numberofedges; ++i)
	{
		FEEdge& e = pm->Edge(i);
		in.edgelist[2 * i] = e.n[0];
		in.edgelist[2 * i + 1] = e.n[1];
		in.edgemarkerlist[i] = e.m_gid + 2;
	}

	// holes
	bool hole = GetBoolValue(HOLE);
	if (hole)
	{
		vec3d ri = GetVecValue(HOLE_COORD);
		const int nh = 1;
		in.numberofholes = nh;
		in.holelist = new double[3 * nh];
		in.holelist[0] = ri.x;
		in.holelist[1] = ri.y;
		in.holelist[2] = ri.z;
	}

	return true;
}


#endif


// Generate a volume mesh from a surface mesh
FEMesh* FETetGenMesher::CreateMesh(FESurfaceMesh* surfMesh)
{
#ifdef TETLIBRARY
	// allocate tetgen structures
	tetgenio in, out;
	in.initialize();
	out.initialize();

	// build the PLC
	if (build_plc(surfMesh, in) == false) return 0;

	// convert element size to volume
	double h = ElementSize();
	double a = h*h*h / 6.0;

	// set the parameters
	double q = ElementQuality();
	bool bsplit = SplitFaces();
	char sz[64] = { 0 };
	char* ch = sz;
	sprintf(ch, "p"); ++ch;
	if (q   > 0) sprintf(ch, "q%lg", q); ch += strlen(ch);
	if (h   > 0) sprintf(ch, "a%lg", a); ch += strlen(ch);
	//	if (m_tol > 0) sprintf(ch, "T%lg", m_tol); ch += strlen(ch);
	//	if (bsplit == false) sprintf(ch, "Y"); ++ch;
	// don't split faces for surface mesh objects since we want to preserve the surface mesh
	sprintf(ch, "Y"); ++ch;

	// create a tet mesh
	tetrahedralize(sz, &in, &out);

	// create a new mesh
	FEMesh* pmesh = new FEMesh;
	int nodes = out.numberofpoints;
	int elems = out.numberoftetrahedra;
	int faces = out.numberoftrifaces;
	int edges = 0;
	for (int i = 0; i<out.numberofedges; ++i) if (out.edgemarkerlist[i] > 1) edges++;

	// make sure tetgen did something
	if ((elems == 0) || (nodes == 0) || (faces == 0)) return 0;

	// create a new mesh
	pmesh->Create(nodes, elems, faces, edges);

	// copy nodes
	for (int i = 0; i<nodes; ++i)
	{
		vec3d& r = pmesh->Node(i).r;
		r.x = out.pointlist[3 * i];
		r.y = out.pointlist[3 * i + 1];
		r.z = out.pointlist[3 * i + 2];
	}

	assert(out.numberofcorners == 4);

	// copy elements
	for (int i = 0; i<elems; ++i)
	{
		FEElement& el = pmesh->Element(i);
		el.SetType(FE_TET4);
		el.m_node[0] = out.tetrahedronlist[4 * i];
		el.m_node[1] = out.tetrahedronlist[4 * i + 1];
		el.m_node[2] = out.tetrahedronlist[4 * i + 2];
		el.m_node[3] = out.tetrahedronlist[4 * i + 3];
		el.m_gid = 0;
	}

	// copy faces
	for (int i = 0; i<faces; ++i)
	{
		FEFace& f = pmesh->Face(i);
		f.SetType(FE_FACE_TRI3);
		f.n[0] = out.trifacelist[3 * i + 2];
		f.n[1] = out.trifacelist[3 * i + 1];
		f.n[2] = out.trifacelist[3 * i];
		f.n[3] = f.n[2];
		f.m_gid = out.trifacemarkerlist[i];
	}

	// copy edges		out.trifacemarkerlist[0]	1	int

	int n = 0;
	for (int i = 0; i<out.numberofedges; ++i)
	{
		if (out.edgemarkerlist[i] > 1)
		{
			FEEdge& e = pmesh->Edge(n++);
			e.SetType(FE_EDGE2);
			e.n[0] = out.edgelist[2 * i]; assert(e.n[0] >= 0);
			e.n[1] = out.edgelist[2 * i + 1]; assert(e.n[1] >= 0);
			e.m_gid = out.edgemarkerlist[i] - 2;
		}
	}
	assert(n == pmesh->Edges());

	// update the element neighbours
	pmesh->UpdateElementNeighbors();

	// update faces
	pmesh->UpdateFaces();
	pmesh->AutoSmooth(60.0);

	pmesh->UpdateEdgeNeighbors();

	// update the mesh
	pmesh->MarkExteriorNodes();
	pmesh->UpdateBox();

	// associate the FE nodes with the GNodes
	/*	GObject* po = pm->GetGObject();
	double R2 = pmesh->GetBoundingBox().GetMaxExtent();
	if (R2 == 0) R2 = 1.0; else R2 *= R2;
	for (int i = 0; i<pmesh->Nodes(); ++i)
	{
	FENode& node = pmesh->Node(i);
	vec3d& ri = node.r;
	node.m_gid = -1;
	for (int j = 0; j<po->Nodes(); ++j)
	{
	GNode& gn = *po->Node(j);
	vec3d& rj = gn.m_r;
	double L2 = (ri - rj).SqrLength();
	if (L2 / R2 < 1e-6)
	{
	node.m_gid = j;
	break;
	}
	}
	}
	*/
	int etype = ElementType();
	if ((etype != FE_TET4) && (etype != FE_INVALID_ELEMENT_TYPE))
	{
		switch (etype)
		{
		case FE_TET10:
		{
			FETet4ToTet10 mod;
			FEMesh* pnew = mod.Apply(pmesh);
			delete pmesh;
			pmesh = pnew;
		}
		break;
		case FE_TET15:
		{
			FETet4ToTet15 mod;
			FEMesh* pnew = mod.Apply(pmesh);
			delete pmesh;
			pmesh = pnew;
		}
		break;
		case FE_TET20:
		{
			FETet4ToTet20 mod;
			FEMesh* pnew = mod.Apply(pmesh);
			delete pmesh;
			pmesh = pnew;
		}
		break;
		default:
			assert(false);
		}
	}

	return pmesh;
#else
	return 0;
#endif // TETLIBRARY
}

