// FETorus.cpp: implementation of the FETorus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FETorus.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>
#include <vector>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FETorus::FETorus(GTorus* po)
{
	m_pobj = po;

	m_nd = 2;
	m_ns = 8;

	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments" );
}

FEMesh* FETorus::BuildMesh()
{
	assert(m_pobj);

	int i, j, k;

	// get the object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GTorus::RIN);
	double R1 = param.GetFloatValue(GTorus::ROUT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);

	// check parameters
	if (m_nd < 1) m_nd = 1;
	if (m_ns < 1) m_ns = 1;

	int ns = 4*m_ns;
	int nd = 2*m_nd;

	// count nodes and elements
	int nodes = ns*((nd+1)*(nd+1) + 4*nd*nd);
	int elems = ns*(nd*nd+4*nd*nd);

	// allocate storage for mesh
	FEMesh* pm = new FEMesh;
	pm->Create(nodes, elems);

	// --- create the first layer of nodes ---
	// create the inner nodes
	FENode* pn = pm->NodePtr();
	vec3d r;
	double h = R1/sqrt(2.0)*0.5;
	double R, f, dr;
	for (i=0; i<=nd; ++i)
		for (j=0; j<=nd; ++j, ++pn)
		{
			r.x = 0;
			r.y = -h + 2*h*i/nd;
			r.z = -h + 2*h*j/nd;

			R = r.Length();
			f = 1 - 0.15*R/h;

			pn->r = vec3d(0,-R0,0) + r*f;
		}

	// create node index loop
	vector<int>	Nd; Nd.resize(4*nd);
	vector<vec3d> Nr; Nr.resize(4*nd);
	for (i=0; i<nd; ++i) Nd[i     ] = nd - i - 1;
	for (i=0; i<nd; ++i) Nd[i+nd  ] = (i+1)*(nd+1);
	for (i=0; i<nd; ++i) Nd[i+nd*2] = i + nd*(nd+1)+1;
	for (i=0; i<nd; ++i) Nd[i+nd*3] = nd*(nd+1) - i*(nd+1) - 1;

	for (i=0; i<4*nd; ++i) Nr[i] = pm->Node(Nd[i]).r;

	// create the loop nodes
	int noff = (nd+1)*(nd+1);
	for (k=0; k<nd; ++k)
	{
		// create the nodes
		for (i=0; i<4*nd; ++i, ++pn)
		{
			r = Nr[i] - vec3d(0,-R0,0);
			R = r.Length();
			dr = (k+1)*(R1 - R)/nd;
			f = (R + dr)/R;
			pn->r = vec3d(0,-R0,0) + r*f;	
		}
	}

	// --- create the mesh
	int eid = 0;
	int nn = (nd+1)*(nd+1) + 4*nd*nd;
	double cw = cos(-2.*PI/ns);
	double sw = sin(-2.*PI/ns);
	for (k=0; k<ns; ++k)
	{
		// create the next nodes
		if (k<ns-1)
		{
			for (i=0; i<nn; ++i, ++pn)
			{
				r = pn[-nn].r;

				pn->r.x =  cw*r.x + sw*r.y;
				pn->r.y = -sw*r.x + cw*r.y;
				pn->r.z = r.z;
			}
		}

		// reset node loop
		noff = k*nn + (nd+1)*(nd+1);
		for (i=0; i<nd; ++i) Nd[i     ] = k*nn + nd - i - 1;
		for (i=0; i<nd; ++i) Nd[i+nd  ] = k*nn + (i+1)*(nd+1);
		for (i=0; i<nd; ++i) Nd[i+nd*2] = k*nn + i + nd*(nd+1)+1;
		for (i=0; i<nd; ++i) Nd[i+nd*3] = k*nn + nd*(nd+1) - i*(nd+1) - 1;

		// create the inner elements
		for (i=0; i<nd; ++i)
			for (j=0; j<nd; ++j)
			{
				FEElement_* pe = pm->ElementPtr(eid++);

				int* n = pe->m_node;

				n[0] = k*nn + j*(nd+1) + i;
				n[1] = k*nn + (j+1)*(nd+1) + i;
				n[2] = k*nn + (j+1)*(nd+1) + i+1;
				n[3] = k*nn + j*(nd+1) + i + 1;
				n[4] = (k<ns-1? n[0] + nn : n[0] - k*nn);
				n[5] = (k<ns-1? n[1] + nn : n[1] - k*nn);
				n[6] = (k<ns-1? n[2] + nn : n[2] - k*nn);
				n[7] = (k<ns-1? n[3] + nn : n[3] - k*nn);

				pe->SetType(FE_HEX8);
				pe->m_gid = 0;
			}

		// create the outer elements
		for (j=0; j<nd; ++j)
		{
			int* n;
			for (i=0; i<4*nd; ++i)
			{
				FEElement_* pe = pm->ElementPtr(eid++);

				n = pe->m_node;

				n[0] = Nd[i];
				n[1] = noff + i;
				n[2] = noff + (i+1)%(4*nd);
				n[3] = Nd[(i+1)%(4*nd)];
				n[4] = (k<ns-1? n[0] + nn : n[0] - k*nn);
				n[5] = (k<ns-1? n[1] + nn : n[1] - k*nn);
				n[6] = (k<ns-1? n[2] + nn : n[2] - k*nn);
				n[7] = (k<ns-1? n[3] + nn : n[3] - k*nn);

				pe->SetType(FE_HEX8);
				pe->m_gid = 0;
			}

			// set the next layer of node indices
			for (i=0; i<4*nd; ++i) Nd[i] = noff+i;
			noff += 4*nd;
		}
	}

	pm->Node(NodeIndex(   0,0)).m_gid = 0;
	pm->Node(NodeIndex(  nd,0)).m_gid = 1;
	pm->Node(NodeIndex(2*nd,0)).m_gid = 2;
	pm->Node(NodeIndex(3*nd,0)).m_gid = 3;

	pm->Node(NodeIndex(   0, m_ns)).m_gid = 4;
	pm->Node(NodeIndex(  nd, m_ns)).m_gid = 5;
	pm->Node(NodeIndex(2*nd, m_ns)).m_gid = 6;
	pm->Node(NodeIndex(3*nd, m_ns)).m_gid = 7;

	pm->Node(NodeIndex(   0, 2*m_ns)).m_gid = 8;
	pm->Node(NodeIndex(  nd, 2*m_ns)).m_gid = 9;
	pm->Node(NodeIndex(2*nd, 2*m_ns)).m_gid = 10;
	pm->Node(NodeIndex(3*nd, 2*m_ns)).m_gid = 11;

	pm->Node(NodeIndex(   0, 3*m_ns)).m_gid = 12;
	pm->Node(NodeIndex(  nd, 3*m_ns)).m_gid = 13;
	pm->Node(NodeIndex(2*nd, 3*m_ns)).m_gid = 14;
	pm->Node(NodeIndex(3*nd, 3*m_ns)).m_gid = 15;

	BuildFaces(pm);
	BuildEdges(pm);

	pm->BuildMesh();

	return pm;
}

void FETorus::BuildFaces(FEMesh* pm)
{
	int nd = 2*m_nd;
	int ns = 4*m_ns;

	int i, j, n = 0;
	pm->Create(0,0,4*nd*ns);
	for (j=0; j<ns; ++j)
		for (i=0; i<4*nd; ++i, ++n)
		{
			FEFace& f = pm->Face(n);
			f.SetType(FE_FACE_QUAD4);
			f.m_gid = 4*(4*j/ns) + i/nd;
			f.m_sid = 0;
			f.n[0] = NodeIndex(i  ,j  );
			f.n[1] = NodeIndex(i+1,j  );
			f.n[2] = NodeIndex(i+1,j+1);
			f.n[3] = NodeIndex(i  ,j+1);
		}
}

void FETorus::BuildEdges(FEMesh* pm)
{
	int nd = 2*m_nd;
	int ns = 4*m_ns;

	int i, j, n=0;
	pm->Create(0,0,0,4*ns + 16*nd);
	for (j=0; j<4; ++j)
		for (i=0; i<ns; ++i, ++n)
		{
			FEEdge& e = pm->Edge(n);
			e.SetType(FE_EDGE2);
			e.m_gid = 4*j + 4*i/ns;
			e.n[0] = NodeIndex(j*nd, i);
			e.n[1] = NodeIndex(j*nd, i+1);
		}

	for (j=0; j<4; ++j)
		for (i=0; i<4*nd; ++i, ++n)
		{
			FEEdge& e = pm->Edge(n);
			e.SetType(FE_EDGE2);
			e.m_gid = 16 + j * 4 + i / nd;
			e.n[0] = NodeIndex(i  , j*ns/4);
			e.n[1] = NodeIndex(i+1, j*ns/4);
		}
}
