#include "stdafx.h"
#include "FECylinderInBox.h"
#include <GeomLib/GPrimitive.h>
#include "FEModifier.h"

//-----------------------------------------------------------------------------
FECylinderInBox::FECylinderInBox()
{
	m_po = 0;
}

//-----------------------------------------------------------------------------
FECylinderInBox::FECylinderInBox(GCylinderInBox* po)
{
	m_po = po;
	m_nx = 1;
	m_ny = 1;
	m_nz = 1;
	m_nr = 1;

	m_gz = 1;
	m_gr = 1;
	m_bz = false;
	m_br = false;

	m_nelem = 0;

	// define the tube parameters
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
	AddIntParam(m_nz, "nz", "Nz");
	AddIntParam(m_nr, "nr", "Nr");

	AddDoubleParam(m_gz, "gz", "Z-bias");
	AddDoubleParam(m_gr, "gr", "R-bias");

	AddBoolParam(m_bz, "bz", "Z-mirrored bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");

	AddIntParam(m_nelem, "elem", "Element Type")->SetEnumNames("Hex8\0Hex20\0");
}

//-----------------------------------------------------------------------------
FEMesh* FECylinderInBox::BuildMesh()
{
	assert(m_po);

	// get the object parameters
	ParamBlock& param = m_po->GetParamBlock();
	double W = param.GetFloatValue(GCylinderInBox::WIDTH );
	double H = param.GetFloatValue(GCylinderInBox::HEIGHT);
	double D = param.GetFloatValue(GCylinderInBox::DEPTH );
	double R = param.GetFloatValue(GCylinderInBox::RADIUS);

	double w = W*0.5;
	double h = H*0.5;
	double d = D;
	double r = R / sqrt(2.0);

	// get meshing parameters
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	m_nz = GetIntValue(NZ);
	m_nr = GetIntValue(NR);

	m_gz = GetFloatValue(GZ);
	m_gr = GetFloatValue(GR);

	m_bz = GetBoolValue(BZ);
	m_br = GetBoolValue(BR);

	int nelem = GetIntValue(NELEM);

	// create the MB nodes
	m_MBNode.resize(16);
	m_MBNode[ 0].m_r = vec3d(-2, -2, 0);
	m_MBNode[ 1].m_r = vec3d( 2, -2, 0);
	m_MBNode[ 2].m_r = vec3d( 2,  2, 0);
	m_MBNode[ 3].m_r = vec3d(-2,  2, 0);
	m_MBNode[ 4].m_r = vec3d(-2, -2, d);
	m_MBNode[ 5].m_r = vec3d( 2, -2, d);
	m_MBNode[ 6].m_r = vec3d( 2,  2, d);
	m_MBNode[ 7].m_r = vec3d(-2,  2, d);
	
	m_MBNode[ 8].m_r = vec3d(-1, -1, 0);
	m_MBNode[ 9].m_r = vec3d( 1, -1, 0);
	m_MBNode[10].m_r = vec3d( 1,  1, 0);
	m_MBNode[11].m_r = vec3d(-1,  1, 0);
	m_MBNode[12].m_r = vec3d(-1, -1, d);
	m_MBNode[13].m_r = vec3d( 1, -1, d);
	m_MBNode[14].m_r = vec3d( 1,  1, d);
	m_MBNode[15].m_r = vec3d(-1,  1, d);

	// create the blocks
	m_MBlock.resize(4);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,9,8,4,5,13,12);
	b1.SetSizes(m_nx, m_nr, m_nz);
	b1.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(0);
	b2.SetNodes(1,2,10,9,5,6,14,13);
	b2.SetSizes(m_ny, m_nr, m_nz);
	b2.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(0);
	b3.SetNodes(2,3,11,10,6,7,15,14);
	b3.SetSizes(m_nx, m_nr, m_nz);
	b3.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(0);
	b4.SetNodes(3,0,8,11,7,4,12,15);
	b4.SetSizes(m_ny, m_nr, m_nz);
	b4.SetZoning(1, m_gr, m_gz, false, m_br, m_bz);

	// update the MB data
	UpdateMB();

	// assign face ID's
	SetBlockFaceID(b1, 0, -1, 4, -1,  8, 12);
	SetBlockFaceID(b2, 1, -1, 5, -1,  9, 13);
	SetBlockFaceID(b3, 2, -1, 6, -1, 10, 14);
	SetBlockFaceID(b4, 3, -1, 7, -1, 11, 15);

	MBFace& F1 = GetBlockFace( 0, 0); SetFaceEdgeID(F1,  0, 9,   4,  8);
	MBFace& F2 = GetBlockFace( 1, 0); SetFaceEdgeID(F2,  1, 10,  5,  9);
	MBFace& F3 = GetBlockFace( 2, 0); SetFaceEdgeID(F3,  2, 11,  6, 10);
	MBFace& F4 = GetBlockFace( 3, 0); SetFaceEdgeID(F4,  3,  8,  7, 11);
	MBFace& F5 = GetBlockFace( 0, 2); SetFaceEdgeID(F5, 12, 20, 16, 21);
	MBFace& F6 = GetBlockFace( 1, 2); SetFaceEdgeID(F6, 13, 21, 17, 22);
	MBFace& F7 = GetBlockFace( 2, 2); SetFaceEdgeID(F7, 14, 22, 18, 23);
	MBFace& F8 = GetBlockFace( 3, 2); SetFaceEdgeID(F8, 15, 23, 19, 20);

	MBFace& F9  = GetBlockFace( 0, 4); SetFaceEdgeID(F9 , 12, 25,  0, 24);
	MBFace& F10 = GetBlockFace( 1, 4); SetFaceEdgeID(F10, 13, 26,  1, 25);
	MBFace& F11 = GetBlockFace( 2, 4); SetFaceEdgeID(F11, 14, 27,  2, 26);
	MBFace& F12 = GetBlockFace( 3, 4); SetFaceEdgeID(F12, 15, 24,  3, 27);

	MBFace& F13 = GetBlockFace( 0, 5); SetFaceEdgeID(F13,  4, 29, 16, 28);
	MBFace& F14 = GetBlockFace( 1, 5); SetFaceEdgeID(F14,  5, 30, 17, 29);
	MBFace& F15 = GetBlockFace( 2, 5); SetFaceEdgeID(F15,  6, 31, 18, 30);
	MBFace& F16 = GetBlockFace( 3, 5); SetFaceEdgeID(F16,  7, 28, 19, 31);

	// set the node ID's
	m_MBNode[ 0].SetID(0);
	m_MBNode[ 1].SetID(1);
	m_MBNode[ 2].SetID(2);
	m_MBNode[ 3].SetID(3);
	m_MBNode[ 4].SetID(4);
	m_MBNode[ 5].SetID(5);
	m_MBNode[ 6].SetID(6);
	m_MBNode[ 7].SetID(7);
	m_MBNode[ 8].SetID(8);
	m_MBNode[ 9].SetID(9);
	m_MBNode[10].SetID(10);
	m_MBNode[11].SetID(11);
	m_MBNode[12].SetID(12);
	m_MBNode[13].SetID(13);
	m_MBNode[14].SetID(14);
	m_MBNode[15].SetID(15);

	// create the MB
	FEMesh* pm = FEMultiBlockMesh::BuildMesh();

	// convert to hex20
	if (nelem == 1)
	{
		FEHex8ToHex20 mod;
		mod.SetSmoothing(false);
		FEMesh* pnew = mod.Apply(pm);
		delete pm;
		pm = pnew;
	}

	// project the nodes onto a cylinder
	for (int i=0; i<pm->Nodes(); ++i)
	{
		// get the nodal coordinate in the template
		vec3d& rn = pm->Node(i).r;
		double x = rn.x;
		double y = rn.y;

		// get the max-distance 
		double D = fmax(fabs(x),fabs(y));

		// "normalize" the coordinates
		// with respect to the max distance
		double r = x/D;
		double s = y/D;

		vec3d r0;
		if (fabs(x) >= fabs(y))
		{
			double u = x/fabs(x);
			r0.x = u*R*cos(PI*0.25*s);
			r0.y =   R*sin(PI*0.25*s);
		}
		else 
		{
			double u = y/fabs(y);
			r0.y = u*R*cos(PI*0.25*r);
			r0.x =   R*sin(PI*0.25*r);
		}

		vec3d r1(r*w, s*h, 0);
		double a = D - 1;

		rn.x = r0.x*(1-a) + r1.x*a;
		rn.y = r0.y*(1-a) + r1.y*a;
	}

	// update the mesh
	pm->UpdateMesh();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	return pm;
}
