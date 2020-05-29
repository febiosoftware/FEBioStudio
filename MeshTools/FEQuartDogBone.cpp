#include "stdafx.h"
#include "FEQuartDogBone.h"
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
FEQuartDogBone::FEQuartDogBone(GQuartDogBone* po)
{
	m_pobj = po;

	AddIntParam(10, "nx", "Nx");
	AddIntParam(30, "ny", "Ny");
	AddIntParam( 1, "nz", "Nz");
}

//-----------------------------------------------------------------------------
FEMesh* FEQuartDogBone::BuildMesh()
{
	assert(m_pobj);

	ParamBlock& param = m_pobj->GetParamBlock();
	double cw = param.GetFloatValue(GQuartDogBone::CWIDTH );
	double ch = param.GetFloatValue(GQuartDogBone::CHEIGHT);
	double R  = param.GetFloatValue(GQuartDogBone::RADIUS );
	double h  = param.GetFloatValue(GQuartDogBone::DEPTH  );
	double L  = param.GetFloatValue(GQuartDogBone::LENGTH );
	double W  = param.GetFloatValue(GQuartDogBone::WING   );

	double a = sqrt(2.0)*0.5;
	double px = cw - W - R*a;
	double py = ch + R*(1.0-a);

	int NX = GetIntValue(N_X);
	int NY = GetIntValue(N_Y);
	int NZ = GetIntValue(N_Z);

	if (NX < 3) NX = 3;
	if (NY < 3) NY = 3;

	double fx1 = px/cw;
	double fx2 = (cw - W - px)/(cw - px);
	double fy1 = py / (ch+R+L);
	double fy2 = (ch+R-py)/(ch+R+L-py);


	int nx = (int) (NX*fx1); if (nx < 1) nx = 1;
	int lx = (int) (fx2*(NX - nx)); if (lx < 1) lx = 1;
	int mx = NX - nx - lx;

	int ny = (int) (NY*fy1); if (ny < 1) ny = 1;
	int ly = (int) (fy2*(NY - ny)); if (ly < 1) ly = 1;
	int my = NY - ny - ly;

	int nz = NZ;

	// create the MB nodes
	m_MBNode.resize(24);
	m_MBNode[ 0].m_r = vec3d(0     ,0     ,0);
	m_MBNode[ 1].m_r = vec3d(px    ,0     ,0);
	m_MBNode[ 2].m_r = vec3d(cw-W  ,0     ,0);
	m_MBNode[ 3].m_r = vec3d(cw    ,0     ,0);
	m_MBNode[ 4].m_r = vec3d(cw    ,ch    ,0);
	m_MBNode[ 5].m_r = vec3d(cw-W  ,ch    ,0);
	m_MBNode[ 6].m_r = vec3d(px    ,py    ,0);
	m_MBNode[ 7].m_r = vec3d(cw-W-R,ch+R  ,0);
	m_MBNode[ 8].m_r = vec3d(cw-W-R,ch+R+L,0);
	m_MBNode[ 9].m_r = vec3d(0     ,ch+R+L,0);
	m_MBNode[10].m_r = vec3d(0     ,ch+R  ,0);
	m_MBNode[11].m_r = vec3d(0     ,py    ,0);

	m_MBNode[12].m_r = vec3d(0     ,0     ,h);
	m_MBNode[13].m_r = vec3d(px    ,0     ,h);
	m_MBNode[14].m_r = vec3d(cw-W  ,0     ,h);
	m_MBNode[15].m_r = vec3d(cw    ,0     ,h);
	m_MBNode[16].m_r = vec3d(cw    ,ch    ,h);
	m_MBNode[17].m_r = vec3d(cw-W  ,ch    ,h);
	m_MBNode[18].m_r = vec3d(px    ,py    ,h);
	m_MBNode[19].m_r = vec3d(cw-W-R,ch+R  ,h);
	m_MBNode[20].m_r = vec3d(cw-W-R,ch+R+L,h);
	m_MBNode[21].m_r = vec3d(0     ,ch+R+L,h);
	m_MBNode[22].m_r = vec3d(0     ,ch+R  ,h);
	m_MBNode[23].m_r = vec3d(0     ,py    ,h);

	// create the MB blocks
	m_MBlock.resize(5);
	MBBlock& b1 = m_MBlock[0];
	b1.SetID(0);
	b1.SetNodes(0,1,6,11,12,13,18,23);
	b1.SetSizes(nx,ny,nz);

	MBBlock& b2 = m_MBlock[1];
	b2.SetID(1);
	b2.SetNodes(1,2,5,6,13,14,17,18);
	b2.SetSizes(lx,ny,nz);

	MBBlock& b3 = m_MBlock[2];
	b3.SetID(2);
	b3.SetNodes(2,3,4,5,14,15,16,17);
	b3.SetSizes(mx,ny,nz);

	MBBlock& b4 = m_MBlock[3];
	b4.SetID(3);
	b4.SetNodes(11,6,7,10,23,18,19,22);
	b4.SetSizes(nx,ly,nz);

	MBBlock& b5 = m_MBlock[4];
	b5.SetID(4);
	b5.SetNodes(10,7,8,9,22,19,20,21);
	b5.SetSizes(nx,my,nz);


	UpdateMB();

	SetBlockFaceID(b1, 1, -1, -1,  7, 0, 8);
	SetBlockFaceID(b2, 1, -1,  4, -1, 0, 8);
	SetBlockFaceID(b3, 1,  2,  3, -1, 0, 8);
	SetBlockFaceID(b4,-1,  4, -1,  7, 0, 8);
	SetBlockFaceID(b5,-1,  5,  6,  7, 0, 8);

	MBFace& F1  = GetBlockFace( 0, 0); SetFaceEdgeID(F1, 0, -1,  7, 14);
	MBFace& F2  = GetBlockFace( 1, 0); SetFaceEdgeID(F2, 0, -1,  7, -1);
	MBFace& F3  = GetBlockFace( 2, 0); SetFaceEdgeID(F3, 0, 15,  7, -1);
	MBFace& F4  = GetBlockFace( 2, 1); SetFaceEdgeID(F4, 1, 16,  8, 15);
	MBFace& F5  = GetBlockFace( 2, 2); SetFaceEdgeID(F5, 2, 17,  9, 16);
	MBFace& F6  = GetBlockFace( 1, 2); SetFaceEdgeID(F6, 3, -1, 10, 17);
	MBFace& F7  = GetBlockFace( 3, 1); SetFaceEdgeID(F7, 3, 18, 10, -1);
	MBFace& F8  = GetBlockFace( 4, 1); SetFaceEdgeID(F8, 4, 19, 11, 18);
	MBFace& F9  = GetBlockFace( 4, 2); SetFaceEdgeID(F9, 5, 20, 12, 19);
	MBFace& F10 = GetBlockFace( 4, 3); SetFaceEdgeID(F10, 6,-1, 13, 20);
	MBFace& F11 = GetBlockFace( 3, 3); SetFaceEdgeID(F11, 6,-1, 13, -1);
	MBFace& F12 = GetBlockFace( 0, 3); SetFaceEdgeID(F12, 6, 14, 13,-1);

	m_MBNode[0].SetID(0);
	m_MBNode[3].SetID(1);
	m_MBNode[4].SetID(2);
	m_MBNode[5].SetID(3);
	m_MBNode[7].SetID(4);
	m_MBNode[8].SetID(5);
	m_MBNode[9].SetID(6);

	m_MBNode[12].SetID(8);
	m_MBNode[15].SetID(9);
	m_MBNode[16].SetID(10);
	m_MBNode[17].SetID(11);
	m_MBNode[19].SetID(12);
	m_MBNode[20].SetID(13);
	m_MBNode[21].SetID(14);

	// create the MB
	FEMesh* pm = FEMultiBlockMesh::BuildMesh();

	// get all the nodes from block 1
	vector<int> node;
	pm->FindNodesFromPart(1, node);
	
	// project these nodes onto the cylinder
	double x0 = cw-W, y0 = ch + R;
	for (int i=0; i<(int)node.size(); ++i)
	{
		vec3d& q = pm->Node(node[i]).r;

		double r = (q.x - px)/(cw - W - px);
		double h = py*(1.0 - r) + r*ch;
		double s = q.y / h;
		double y = y0 - sqrt(R*R - (q.x - x0)*(q.x - x0));
		q.y = s*y;
	}

	// get all the nodes from block 3
	pm->FindNodesFromPart(3, node);
	
	// project these nodes onto the cylinder
	for (int i=0; i<(int)node.size(); ++i)
	{
		vec3d& q = pm->Node(node[i]).r;

		double s = (q.y - py)/(ch + R - py);
		double w = px*(1.0 - s) + s*(cw-W-R);
		double r = q.x / w;
		double x = x0 - sqrt(R*R - (q.y - y0)*(q.y - y0));
		q.x = r*x;
	}

	// reset the element group ID's
	for (int i=0; i<pm->Elements(); ++i) pm->Element(i).m_gid = 0;

	// update the mesh
	pm->BuildMesh();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	// finally, we update the normals and we are good to go
	pm->UpdateNormals();

	return pm;
}
