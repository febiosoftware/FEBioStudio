#include "stdafx.h"
#include "GLModel.h"
using namespace Post;

//-----------------------------------------------------------------------------
// Render a 4-noded quad
void CGLModel::RenderQUAD4(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_QUAD4);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[4]; pm->FaceNodePosition(f, r);

	vec3f n[4]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = f.m_fn; }

	float t[4]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	// render the quads
	glBegin(GL_QUADS);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a 8-noded quad
void CGLModel::RenderQUAD8(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_QUAD8);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[8]; pm->FaceNodePosition(f, r);

	vec3f n[8]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = f.m_fn; }

	float t[8]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);

		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[7].x, n[7].y, n[7].z); glTexCoord1f(t[7]); glVertex3f(r[7].x, r[7].y, r[7].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a 9-noded quad
void CGLModel::RenderQUAD9(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_QUAD9);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[9]; pm->FaceNodePosition(f, r);

	vec3f n[9]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = f.m_fn; }

	float t[9]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	int T[8][3] = {
		{0,4,8},{8,7,0},{4,1,5},{5,8,4},
		{7,8,6},{6,3,7},{8,5,2},{2,6,8}};

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[T[0][0]].x, n[T[0][0]].y, n[T[0][0]].z); glTexCoord1f(t[T[0][0]]); glVertex3f(r[T[0][0]].x, r[T[0][0]].y, r[T[0][0]].z);
		glNormal3f(n[T[0][1]].x, n[T[0][1]].y, n[T[0][1]].z); glTexCoord1f(t[T[0][1]]); glVertex3f(r[T[0][1]].x, r[T[0][1]].y, r[T[0][1]].z);
		glNormal3f(n[T[0][2]].x, n[T[0][2]].y, n[T[0][2]].z); glTexCoord1f(t[T[0][2]]); glVertex3f(r[T[0][2]].x, r[T[0][2]].y, r[T[0][2]].z);

		glNormal3f(n[T[1][0]].x, n[T[1][0]].y, n[T[1][0]].z); glTexCoord1f(t[T[1][0]]); glVertex3f(r[T[1][0]].x, r[T[1][0]].y, r[T[1][0]].z);
		glNormal3f(n[T[1][1]].x, n[T[1][1]].y, n[T[1][1]].z); glTexCoord1f(t[T[1][1]]); glVertex3f(r[T[1][1]].x, r[T[1][1]].y, r[T[1][1]].z);
		glNormal3f(n[T[1][2]].x, n[T[1][2]].y, n[T[1][2]].z); glTexCoord1f(t[T[1][2]]); glVertex3f(r[T[1][2]].x, r[T[1][2]].y, r[T[1][2]].z);

		glNormal3f(n[T[2][0]].x, n[T[2][0]].y, n[T[2][0]].z); glTexCoord1f(t[T[2][0]]); glVertex3f(r[T[2][0]].x, r[T[2][0]].y, r[T[2][0]].z);
		glNormal3f(n[T[2][1]].x, n[T[2][1]].y, n[T[2][1]].z); glTexCoord1f(t[T[2][1]]); glVertex3f(r[T[2][1]].x, r[T[2][1]].y, r[T[2][1]].z);
		glNormal3f(n[T[2][2]].x, n[T[2][2]].y, n[T[2][2]].z); glTexCoord1f(t[T[2][2]]); glVertex3f(r[T[2][2]].x, r[T[2][2]].y, r[T[2][2]].z);

		glNormal3f(n[T[3][0]].x, n[T[3][0]].y, n[T[3][0]].z); glTexCoord1f(t[T[3][0]]); glVertex3f(r[T[3][0]].x, r[T[3][0]].y, r[T[3][0]].z);
		glNormal3f(n[T[3][1]].x, n[T[3][1]].y, n[T[3][1]].z); glTexCoord1f(t[T[3][1]]); glVertex3f(r[T[3][1]].x, r[T[3][1]].y, r[T[3][1]].z);
		glNormal3f(n[T[3][2]].x, n[T[3][2]].y, n[T[3][2]].z); glTexCoord1f(t[T[3][2]]); glVertex3f(r[T[3][2]].x, r[T[3][2]].y, r[T[3][2]].z);

		glNormal3f(n[T[4][0]].x, n[T[4][0]].y, n[T[4][0]].z); glTexCoord1f(t[T[4][0]]); glVertex3f(r[T[4][0]].x, r[T[4][0]].y, r[T[4][0]].z);
		glNormal3f(n[T[4][1]].x, n[T[4][1]].y, n[T[4][1]].z); glTexCoord1f(t[T[4][1]]); glVertex3f(r[T[4][1]].x, r[T[4][1]].y, r[T[4][1]].z);
		glNormal3f(n[T[4][2]].x, n[T[4][2]].y, n[T[4][2]].z); glTexCoord1f(t[T[4][2]]); glVertex3f(r[T[4][2]].x, r[T[4][2]].y, r[T[4][2]].z);

		glNormal3f(n[T[5][0]].x, n[T[5][0]].y, n[T[5][0]].z); glTexCoord1f(t[T[5][0]]); glVertex3f(r[T[5][0]].x, r[T[5][0]].y, r[T[5][0]].z);
		glNormal3f(n[T[5][1]].x, n[T[5][1]].y, n[T[5][1]].z); glTexCoord1f(t[T[5][1]]); glVertex3f(r[T[5][1]].x, r[T[5][1]].y, r[T[5][1]].z);
		glNormal3f(n[T[5][2]].x, n[T[5][2]].y, n[T[5][2]].z); glTexCoord1f(t[T[5][2]]); glVertex3f(r[T[5][2]].x, r[T[5][2]].y, r[T[5][2]].z);

		glNormal3f(n[T[6][0]].x, n[T[6][0]].y, n[T[6][0]].z); glTexCoord1f(t[T[6][0]]); glVertex3f(r[T[6][0]].x, r[T[6][0]].y, r[T[6][0]].z);
		glNormal3f(n[T[6][1]].x, n[T[6][1]].y, n[T[6][1]].z); glTexCoord1f(t[T[6][1]]); glVertex3f(r[T[6][1]].x, r[T[6][1]].y, r[T[6][1]].z);
		glNormal3f(n[T[6][2]].x, n[T[6][2]].y, n[T[6][2]].z); glTexCoord1f(t[T[6][2]]); glVertex3f(r[T[6][2]].x, r[T[6][2]].y, r[T[6][2]].z);

		glNormal3f(n[T[7][0]].x, n[T[7][0]].y, n[T[7][0]].z); glTexCoord1f(t[T[7][0]]); glVertex3f(r[T[7][0]].x, r[T[7][0]].y, r[T[7][0]].z);
		glNormal3f(n[T[7][1]].x, n[T[7][1]].y, n[T[7][1]].z); glTexCoord1f(t[T[7][1]]); glVertex3f(r[T[7][1]].x, r[T[7][1]].y, r[T[7][1]].z);
		glNormal3f(n[T[7][2]].x, n[T[7][2]].y, n[T[7][2]].z); glTexCoord1f(t[T[7][2]]); glVertex3f(r[T[7][2]].x, r[T[7][2]].y, r[T[7][2]].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a 3-noded tri
void CGLModel::RenderTRI3(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_TRI3);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[3]; pm->FaceNodePosition(f, r);

	vec3f n[3]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = f.m_fn; }

	float t[3]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a 6-noded tri
void CGLModel::RenderTRI6(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_TRI6);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[6]; pm->FaceNodePosition(f, r);

	vec3f n[6]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = f.m_fn; }

	float t[6]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a 7-noded tri
void CGLModel::RenderTRI7(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_TRI7);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[7]; pm->FaceNodePosition(f, r);

	vec3f n[7]; 
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = f.m_fn; }

	float t[7]; 
	pm->FaceNodeTexCoords(f, t, bnode);

	glBegin(GL_TRIANGLES);
	{
		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[3].x, n[3].y, n[3].z); glTexCoord1f(t[3]); glVertex3f(r[3].x, r[3].y, r[3].z);

		glNormal3f(n[1].x, n[1].y, n[1].z); glTexCoord1f(t[1]); glVertex3f(r[1].x, r[1].y, r[1].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[4].x, n[4].y, n[4].z); glTexCoord1f(t[4]); glVertex3f(r[4].x, r[4].y, r[4].z);

		glNormal3f(n[2].x, n[2].y, n[2].z); glTexCoord1f(t[2]); glVertex3f(r[2].x, r[2].y, r[2].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);

		glNormal3f(n[0].x, n[0].y, n[0].z); glTexCoord1f(t[0]); glVertex3f(r[0].x, r[0].y, r[0].z);
		glNormal3f(n[6].x, n[6].y, n[6].z); glTexCoord1f(t[6]); glVertex3f(r[6].x, r[6].y, r[6].z);
		glNormal3f(n[5].x, n[5].y, n[5].z); glTexCoord1f(t[5]); glVertex3f(r[5].x, r[5].y, r[5].z);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
inline void glxVertex(const vec3f& n, const vec3f& r, float t)
{
	glNormal3f(n.x, n.y, n.z);
	glTexCoord1f(t);
	glVertex3f(r.x, r.y, r.z);
}

//-----------------------------------------------------------------------------
// Render a 10-noded tri
void CGLModel::RenderTRI10(FEFace& f, bool bsmooth, bool bnode)
{
	assert(f.m_ntype == FACE_TRI10);

	// get the mesh
	FEMeshBase* pm = GetActiveMesh();

	// get the nodal data
	vec3f r[10]; pm->FaceNodePosition(f, r);

	vec3f n[10];
	if (bsmooth) pm->FaceNodeNormals(f, n);
	else { n[0] = n[1] = n[2] = n[3] = n[4] = n[5] = n[6] = n[7] = n[8] = n[9] = f.m_fn; }

	float t[10];
	pm->FaceNodeTexCoords(f, t, bnode);

	glBegin(GL_TRIANGLES);
	{
		glxVertex(n[0], r[0], t[0]); glxVertex(n[3], r[3], t[3]); glxVertex(n[7], r[7], t[7]);
		glxVertex(n[1], r[1], t[1]); glxVertex(n[5], r[5], t[5]); glxVertex(n[4], r[4], t[4]);
		glxVertex(n[2], r[2], t[2]); glxVertex(n[8], r[8], t[8]); glxVertex(n[6], r[6], t[6]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[7], r[7], t[7]); glxVertex(n[3], r[3], t[3]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[3], r[3], t[3]); glxVertex(n[4], r[4], t[4]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[4], r[4], t[4]); glxVertex(n[5], r[5], t[5]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[5], r[5], t[5]); glxVertex(n[6], r[6], t[6]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[6], r[6], t[6]); glxVertex(n[8], r[8], t[8]);
		glxVertex(n[9], r[9], t[9]); glxVertex(n[8], r[8], t[8]); glxVertex(n[7], r[7], t[7]);
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void CGLModel::RenderSmoothQUAD4(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	vec3f r[4] = {
		pm->Node(face.node[0]).m_rt,
		pm->Node(face.node[1]).m_rt,
		pm->Node(face.node[2]).m_rt,
		pm->Node(face.node[3]).m_rt
	};

	vec3f n[4] = {face.m_nn[0], face.m_nn[1], face.m_nn[2], face.m_nn[3]};

	float t[4];
	pm->FaceNodeTexCoords(face, t, bnode);
	RenderSmoothQUAD4(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 4-noded quadrilateral
void CGLModel::RenderSmoothQUAD4(vec3f r[4], vec3f n[4], float q[4], int ndivs)
{
	float sa[4], ta[4], s, t, tk, h[4];
	vec3f nk, rk;
	int i, j, k;
	glBegin(GL_QUADS);
	for (i=0; i<ndivs; ++i)
	{
		sa[0] = -1.f + i*2.f/ndivs;
		sa[1] = -1.f + (i+1)*2.f/ndivs;
		sa[2] = -1.f + (i+1)*2.f/ndivs;
		sa[3] = -1.f + i*2.f/ndivs;

		for (j=0; j<ndivs; ++j)
		{
			ta[0] = -1.f + j*2.f/ndivs;
			ta[1] = -1.f + j*2.f/ndivs;
			ta[2] = -1.f + (j+1)*2.f/ndivs;
			ta[3] = -1.f + (j+1)*2.f/ndivs;

			for (k=0; k<4; ++k)
			{
				s = sa[k];
				t = ta[k];
				h[0] = 0.25f*(1 - s)*(1 - t);
				h[1] = 0.25f*(1 + s)*(1 - t);
				h[2] = 0.25f*(1 + s)*(1 + t);
				h[3] = 0.25f*(1 - s)*(1 + t);

				nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3];
				rk = r[0]*h[0] + r[1]*h[1] + r[2]*h[2] + r[3]*h[3];
				tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3];

				glNormal3f(nk.x, nk.y, nk.z); 
				glTexCoord1f(tk); 
				glVertex3f(rk.x, rk.y, rk.z);
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 8-noded quadrilateral
void CGLModel::RenderSmoothQUAD8(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	assert(face.m_ntype == FACE_QUAD8);

	vec3f r[8];
	r[0] = pm->Node(face.node[0]).m_rt;
	r[1] = pm->Node(face.node[1]).m_rt;
	r[2] = pm->Node(face.node[2]).m_rt;
	r[3] = pm->Node(face.node[3]).m_rt;
	r[4] = pm->Node(face.node[4]).m_rt;
	r[5] = pm->Node(face.node[5]).m_rt;
	r[6] = pm->Node(face.node[6]).m_rt;
	r[7] = pm->Node(face.node[7]).m_rt;

	vec3f n[8];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];

	float t[8];
	pm->FaceNodeTexCoords(face, t, bnode);

	RenderSmoothQUAD8(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 8-noded quadrilateral
void CGLModel::RenderSmoothQUAD8(vec3f r[8], vec3f n[8], float q[8], int ndivs)
{
	float sa[4], ta[4], s, t, tk, h[8];
	vec3f nk, rk;
	int i, j, k;
	glBegin(GL_QUADS);
	{
		for (i=0; i<ndivs; ++i)
		{
			sa[0] = -1.f + i*2.f/ndivs;
			sa[1] = -1.f + (i+1)*2.f/ndivs;
			sa[2] = -1.f + (i+1)*2.f/ndivs;
			sa[3] = -1.f + i*2.f/ndivs;

			for (j=0; j<ndivs; ++j)
			{
				ta[0] = -1.f + j*2.f/ndivs;
				ta[1] = -1.f + j*2.f/ndivs;
				ta[2] = -1.f + (j+1)*2.f/ndivs;
				ta[3] = -1.f + (j+1)*2.f/ndivs;

				for (k=0; k<4; ++k)
				{
					s = sa[k];
					t = ta[k];
					h[4] = 0.5f*(1 - s*s)*(1 - t);
					h[5] = 0.5f*(1 - t*t)*(1 + s);
					h[6] = 0.5f*(1 - s*s)*(1 + t);
					h[7] = 0.5f*(1 - t*t)*(1 - s);

					h[0] = 0.25f*(1 - s)*(1 - t) - (h[4] + h[7])*0.5f;
					h[1] = 0.25f*(1 + s)*(1 - t) - (h[4] + h[5])*0.5f;
					h[2] = 0.25f*(1 + s)*(1 + t) - (h[5] + h[6])*0.5f;
					h[3] = 0.25f*(1 - s)*(1 + t) - (h[6] + h[7])*0.5f;

					nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5] + n[6]*h[6] + n[7]*h[7];
					rk = r[0]*h[0] + r[1]*h[1] + r[2]*h[2] + r[3]*h[3] + r[4]*h[4] + r[5]*h[5] + r[6]*h[6] + r[7]*h[7];
					tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5] + q[6]*h[6] + q[7]*h[7];

					glNormal3f(nk.x, nk.y, nk.z); 
					glTexCoord1f(tk); 
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 9-noded quadrilateral
void CGLModel::RenderSmoothQUAD9(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	assert(face.m_ntype == FACE_QUAD9);

	vec3f r[9];
	r[0] = pm->Node(face.node[0]).m_rt;
	r[1] = pm->Node(face.node[1]).m_rt;
	r[2] = pm->Node(face.node[2]).m_rt;
	r[3] = pm->Node(face.node[3]).m_rt;
	r[4] = pm->Node(face.node[4]).m_rt;
	r[5] = pm->Node(face.node[5]).m_rt;
	r[6] = pm->Node(face.node[6]).m_rt;
	r[7] = pm->Node(face.node[7]).m_rt;
	r[8] = pm->Node(face.node[8]).m_rt;

	vec3f n[9];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];

	float t[9];
	pm->FaceNodeTexCoords(face, t, bnode);

	RenderSmoothQUAD9(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 9-noded quadrilateral
void CGLModel::RenderSmoothQUAD9(vec3f r[9], vec3f n[9], float q[9], int ndivs)
{
	float sa[4], ta[4], s, t, tk, h[9], R[3], S[3];
	vec3f nk, rk;
	int i, j, k;
	glBegin(GL_QUADS);
	{
		for (i=0; i<ndivs; ++i)
		{
			sa[0] = -1.f + i*2.f/ndivs;
			sa[1] = -1.f + (i+1)*2.f/ndivs;
			sa[2] = -1.f + (i+1)*2.f/ndivs;
			sa[3] = -1.f + i*2.f/ndivs;

			for (j=0; j<ndivs; ++j)
			{
				ta[0] = -1.f + j*2.f/ndivs;
				ta[1] = -1.f + j*2.f/ndivs;
				ta[2] = -1.f + (j+1)*2.f/ndivs;
				ta[3] = -1.f + (j+1)*2.f/ndivs;

				for (k=0; k<4; ++k)
				{
					s = sa[k];
					t = ta[k];

					R[0] = 0.5*s*(s - 1.0);
					R[1] = 0.5*s*(s + 1.0);
					R[2] = 1.0 - s*s;

					S[0] = 0.5*t*(t - 1.0);
					S[1] = 0.5*t*(t + 1.0);
					S[2] = 1.0 - t*t;

					h[0] = R[0]*S[0];
					h[1] = R[1]*S[0];
					h[2] = R[1]*S[1];
					h[3] = R[0]*S[1];
					h[4] = R[2]*S[0];
					h[5] = R[1]*S[2];
					h[6] = R[2]*S[1];
					h[7] = R[0]*S[2];
					h[8] = R[2]*S[2];

					nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5] + n[6]*h[6] + n[7]*h[7] + n[8]*h[8];
					rk = r[0]*h[0] + r[1]*h[1] + r[2]*h[2] + r[3]*h[3] + r[4]*h[4] + r[5]*h[5] + r[6]*h[6] + r[7]*h[7] + r[8]*h[8];
					tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5] + q[6]*h[6] + q[7]*h[7] + q[8]*h[8];

					glNormal3f(nk.x, nk.y, nk.z); 
					glTexCoord1f(tk); 
					glVertex3f(rk.x, rk.y, rk.z);
				}
			}
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void CGLModel::RenderSmoothTRI6(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	assert(face.m_ntype == FACE_TRI6);

	vec3f r[6];
	r[0] = pm->Node(face.node[0]).m_rt;
	r[1] = pm->Node(face.node[1]).m_rt;
	r[2] = pm->Node(face.node[2]).m_rt;
	r[3] = pm->Node(face.node[3]).m_rt;
	r[4] = pm->Node(face.node[4]).m_rt;
	r[5] = pm->Node(face.node[5]).m_rt;

	vec3f n[6];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];

	float t[6];
	pm->FaceNodeTexCoords(face, t, bnode);

	RenderSmoothTRI6(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 6-noded triangle
void CGLModel::RenderSmoothTRI6(vec3f x[6], vec3f n[6], float q[6], int ndivs)
{
	float sa[2], ta[2], r, s, t, tk, h[8];
	vec3f nk, xk;
	int i, j, k;
	int nj = ndivs;

	int sl[2][3] = {{0,1,0},{1,1,0}};
	int tl[2][3] = {{0,0,1},{0,1,1}};

	glBegin(GL_TRIANGLES);
	{
		for (i=0; i<ndivs; ++i)
		{
			ta[0] = (float) i / ndivs;
			ta[1] = (float) (i+1) / ndivs;

			for (j=0; j<nj; ++j)
			{
				sa[0] = (float) j / ndivs;
				sa[1] = (float) (j+1) /ndivs;

				for (k=0; k<3; ++k)
				{
					s = sa[sl[0][k]];
					t = ta[tl[0][k]];
					r = 1.f - s - t;

					h[0] = r*(2.f*r - 1.f);
					h[1] = s*(2.f*s - 1.f);
					h[2] = t*(2.f*t - 1.f);
					h[3] = 4.f*r*s;
					h[4] = 4.f*s*t;
					h[5] = 4.f*r*t;

					nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5];
					xk = x[0]*h[0] + x[1]*h[1] + x[2]*h[2] + x[3]*h[3] + x[4]*h[4] + x[5]*h[5];
					tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5];

					glNormal3f(nk.x, nk.y, nk.z); 
					glTexCoord1f(tk); 
					glVertex3f(xk.x, xk.y, xk.z);
				}

				if (j != nj-1)
				{
					for (k=0; k<3; ++k)
					{
						s = sa[sl[1][k]];
						t = ta[tl[1][k]];
						r = 1.f - s - t;

						h[0] = r*(2.f*r - 1.f);
						h[1] = s*(2.f*s - 1.f);
						h[2] = t*(2.f*t - 1.f);
						h[3] = 4.f*r*s;
						h[4] = 4.f*s*t;
						h[5] = 4.f*r*t;

						nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5];
						xk = x[0]*h[0] + x[1]*h[1] + x[2]*h[2] + x[3]*h[3] + x[4]*h[4] + x[5]*h[5];
						tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5];

						glNormal3f(nk.x, nk.y, nk.z); 
						glTexCoord1f(tk); 
						glVertex3f(xk.x, xk.y, xk.z);
					}
				}
			}
			nj -= 1;
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 7-noded triangle
void CGLModel::RenderSmoothTRI7(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	assert(face.m_ntype == FACE_TRI7);

	vec3f r[7];
	r[0] = pm->Node(face.node[0]).m_rt;
	r[1] = pm->Node(face.node[1]).m_rt;
	r[2] = pm->Node(face.node[2]).m_rt;
	r[3] = pm->Node(face.node[3]).m_rt;
	r[4] = pm->Node(face.node[4]).m_rt;
	r[5] = pm->Node(face.node[5]).m_rt;
	r[6] = pm->Node(face.node[6]).m_rt;

	vec3f n[7];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];

	float t[7];
	pm->FaceNodeTexCoords(face, t, bnode);

	RenderSmoothTRI7(r, n, t, ndivs);
}


//-----------------------------------------------------------------------------
// Render a sub-divided 7-noded triangle
void CGLModel::RenderSmoothTRI7(vec3f x[7], vec3f n[7], float q[7], int ndivs)
{
	float sa[2], ta[2], r, s, t, tk, h[7];
	vec3f nk, xk;
	int i, j, k;
	int nj = ndivs;

	int sl[2][3] = {{0,1,0},{1,1,0}};
	int tl[2][3] = {{0,0,1},{0,1,1}};

	glBegin(GL_TRIANGLES);
	{
		for (i=0; i<ndivs; ++i)
		{
			ta[0] = (float) i / ndivs;
			ta[1] = (float) (i+1) / ndivs;

			for (j=0; j<nj; ++j)
			{
				sa[0] = (float) j / ndivs;
				sa[1] = (float) (j+1) /ndivs;

				for (k=0; k<3; ++k)
				{
					s = sa[sl[0][k]];
					t = ta[tl[0][k]];
					r = 1.f - s - t;

					h[6] = 27.f*r*s*t;
					h[0] = r*(2.f*r - 1.f) + h[6]/9.f;
					h[1] = s*(2.f*s - 1.f) + h[6]/9.f;
					h[2] = t*(2.f*t - 1.f) + h[6]/9.f;
					h[3] = 4.f*r*s - 4.f*h[6]/9.f;
					h[4] = 4.f*s*t - 4.f*h[6]/9.f;
					h[5] = 4.f*t*r - 4.f*h[6]/9.f;

					nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5] + n[6]*h[6];
					xk = x[0]*h[0] + x[1]*h[1] + x[2]*h[2] + x[3]*h[3] + x[4]*h[4] + x[5]*h[5] + x[6]*h[6];
					tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5] + q[6]*h[6];

					glNormal3f(nk.x, nk.y, nk.z); 
					glTexCoord1f(tk); 
					glVertex3f(xk.x, xk.y, xk.z);
				}

				if (j != nj-1)
				{
					for (k=0; k<3; ++k)
					{
						s = sa[sl[1][k]];
						t = ta[tl[1][k]];
						r = 1.f - s - t;

						h[6] = 27.f*r*s*t;
						h[0] = r*(2.f*r - 1.f) + h[6]/9.f;
						h[1] = s*(2.f*s - 1.f) + h[6]/9.f;
						h[2] = t*(2.f*t - 1.f) + h[6]/9.f;
						h[3] = 4.f*r*s - 4.f*h[6]/9.f;
						h[4] = 4.f*s*t - 4.f*h[6]/9.f;
						h[5] = 4.f*t*r - 4.f*h[6]/9.f;

						nk = n[0]*h[0] + n[1]*h[1] + n[2]*h[2] + n[3]*h[3] + n[4]*h[4] + n[5]*h[5] + n[6]*h[6];
						xk = x[0]*h[0] + x[1]*h[1] + x[2]*h[2] + x[3]*h[3] + x[4]*h[4] + x[5]*h[5] + x[6]*h[6];
						tk = q[0]*h[0] + q[1]*h[1] + q[2]*h[2] + q[3]*h[3] + q[4]*h[4] + q[5]*h[5] + q[6]*h[6];

						glNormal3f(nk.x, nk.y, nk.z); 
						glTexCoord1f(tk); 
						glVertex3f(xk.x, xk.y, xk.z);
					}
				}
			}
			nj -= 1;
		}
	}
	glEnd();
}

//-----------------------------------------------------------------------------
// Render a sub-divided 10-noded triangle
void CGLModel::RenderSmoothTRI10(FEFace& face, FEMeshBase* pm, int ndivs, bool bnode)
{
	assert(face.m_ntype == FACE_TRI10);

	vec3f r[10];
	r[0] = pm->Node(face.node[0]).m_rt;
	r[1] = pm->Node(face.node[1]).m_rt;
	r[2] = pm->Node(face.node[2]).m_rt;
	r[3] = pm->Node(face.node[3]).m_rt;
	r[4] = pm->Node(face.node[4]).m_rt;
	r[5] = pm->Node(face.node[5]).m_rt;
	r[6] = pm->Node(face.node[6]).m_rt;
	r[7] = pm->Node(face.node[7]).m_rt;
	r[8] = pm->Node(face.node[8]).m_rt;
	r[9] = pm->Node(face.node[9]).m_rt;

	vec3f n[10];
	n[0] = face.m_nn[0];
	n[1] = face.m_nn[1];
	n[2] = face.m_nn[2];
	n[3] = face.m_nn[3];
	n[4] = face.m_nn[4];
	n[5] = face.m_nn[5];
	n[6] = face.m_nn[6];
	n[7] = face.m_nn[7];
	n[8] = face.m_nn[8];
	n[9] = face.m_nn[9];

	float t[10];
	pm->FaceNodeTexCoords(face, t, bnode);

	RenderSmoothTRI10(r, n, t, ndivs);
}

//-----------------------------------------------------------------------------
// Render a sub-divided 10-noded triangle
void CGLModel::RenderSmoothTRI10(vec3f x[10], vec3f n[10], float q[10], int ndivs)
{
	float sa[2], ta[2], r, s, t, tk, h[10];
	vec3f nk, xk;
	int i, j, k;
	int nj = ndivs;

	int sl[2][3] = { { 0, 1, 0 }, { 1, 1, 0 } };
	int tl[2][3] = { { 0, 0, 1 }, { 0, 1, 1 } };

	glBegin(GL_TRIANGLES);
	{
		for (i = 0; i<ndivs; ++i)
		{
			ta[0] = (float)i / ndivs;
			ta[1] = (float)(i + 1) / ndivs;

			for (j = 0; j<nj; ++j)
			{
				sa[0] = (float)j / ndivs;
				sa[1] = (float)(j + 1) / ndivs;

				for (k = 0; k<3; ++k)
				{
					s = sa[sl[0][k]];
					t = ta[tl[0][k]];
					r = 1.f - s - t;

					h[0] = 0.5f*(3.f*r - 1.f)*(3.f*r - 2.f)*r;
					h[1] = 0.5f*(3.f*s - 1.f)*(3.f*s - 2.f)*s;
					h[2] = 0.5f*(3.f*t - 1.f)*(3.f*t - 2.f)*t;
					h[3] = 9.f/2.f*(3.f*r - 1.f)*r*s;
					h[4] = 9.f/2.f*(3.f*s - 1.f)*r*s;
					h[5] = 9.f/2.f*(3.f*s - 1.f)*s*t;
					h[6] = 9.f/2.f*(3.f*t - 1.f)*s*t;
					h[7] = 9.f/2.f*(3.f*r - 1.f)*r*t;
					h[8] = 9.f/2.f*(3.f*t - 1.f)*r*t;
					h[9] = 27.f*r*s*t;

					nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
					xk = x[0] * h[0] + x[1] * h[1] + x[2] * h[2] + x[3] * h[3] + x[4] * h[4] + x[5] * h[5] + x[6] * h[6] + x[7] * h[7] + x[8] * h[8] + x[9] * h[9];
					tk = q[0] * h[0] + q[1] * h[1] + q[2] * h[2] + q[3] * h[3] + q[4] * h[4] + q[5] * h[5] + q[6] * h[6] + q[7] * h[7] + q[8] * h[8] + q[9] * h[9];

					glNormal3f(nk.x, nk.y, nk.z);
					glTexCoord1f(tk);
					glVertex3f(xk.x, xk.y, xk.z);
				}

				if (j != nj - 1)
				{
					for (k = 0; k<3; ++k)
					{
						s = sa[sl[1][k]];
						t = ta[tl[1][k]];
						r = 1.f - s - t;

						h[0] = 0.5f*(3.f*r - 1.f)*(3.f*r - 2.f)*r;
						h[1] = 0.5f*(3.f*s - 1.f)*(3.f*s - 2.f)*s;
						h[2] = 0.5f*(3.f*t - 1.f)*(3.f*t - 2.f)*t;
						h[3] = 9.f / 2.f*(3.f*r - 1.f)*r*s;
						h[4] = 9.f / 2.f*(3.f*s - 1.f)*r*s;
						h[5] = 9.f / 2.f*(3.f*s - 1.f)*s*t;
						h[6] = 9.f / 2.f*(3.f*t - 1.f)*s*t;
						h[7] = 9.f / 2.f*(3.f*r - 1.f)*r*t;
						h[8] = 9.f / 2.f*(3.f*t - 1.f)*r*t;
						h[9] = 27.f*r*s*t;

						nk = n[0] * h[0] + n[1] * h[1] + n[2] * h[2] + n[3] * h[3] + n[4] * h[4] + n[5] * h[5] + n[6] * h[6] + n[7] * h[7] + n[8] * h[8] + n[9] * h[9];
						xk = x[0] * h[0] + x[1] * h[1] + x[2] * h[2] + x[3] * h[3] + x[4] * h[4] + x[5] * h[5] + x[6] * h[6] + x[7] * h[7] + x[8] * h[8] + x[9] * h[9];
						tk = q[0] * h[0] + q[1] * h[1] + q[2] * h[2] + q[3] * h[3] + q[4] * h[4] + q[5] * h[5] + q[6] * h[6] + q[7] * h[7] + q[8] * h[8] + q[9] * h[9];

						glNormal3f(nk.x, nk.y, nk.z);
						glTexCoord1f(tk);
						glVertex3f(xk.x, xk.y, xk.z);
					}
				}
			}
			nj -= 1;
		}
	}
	glEnd();
}

void CGLModel::RenderTexQUAD4(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_QUADS);
	{
		vec3f r1 = pm->Node(face.node[0]).m_rt;
		vec3f r2 = pm->Node(face.node[1]).m_rt;
		vec3f r3 = pm->Node(face.node[2]).m_rt;
		vec3f r4 = pm->Node(face.node[3]).m_rt;

		vec3f nf = (r2 - r1) ^ (r3 - r1);
		nf.Normalize();
		glNormal3f(nf.x, nf.y, nf.z);

		float t1 = pm->Node(face.node[0]).m_tex;
		float t2 = pm->Node(face.node[1]).m_tex;
		float t3 = pm->Node(face.node[2]).m_tex;
		float t4 = pm->Node(face.node[3]).m_tex;

		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);
	}
	glEnd();
}

void CGLModel::RenderTexQUAD8(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		vec3f r1 = pm->Node(face.node[0]).m_rt;
		vec3f r2 = pm->Node(face.node[1]).m_rt;
		vec3f r3 = pm->Node(face.node[2]).m_rt;
		vec3f r4 = pm->Node(face.node[3]).m_rt;
		vec3f r5 = pm->Node(face.node[4]).m_rt;
		vec3f r6 = pm->Node(face.node[5]).m_rt;
		vec3f r7 = pm->Node(face.node[6]).m_rt;
		vec3f r8 = pm->Node(face.node[7]).m_rt;

		vec3f nf = (r2 - r1) ^ (r3 - r1);
		nf.Normalize();
		glNormal3f(nf.x, nf.y, nf.z);

		float t1 = pm->Node(face.node[0]).m_tex;
		float t2 = pm->Node(face.node[1]).m_tex;
		float t3 = pm->Node(face.node[2]).m_tex;
		float t4 = pm->Node(face.node[3]).m_tex;
		float t5 = pm->Node(face.node[4]).m_tex;
		float t6 = pm->Node(face.node[5]).m_tex;
		float t7 = pm->Node(face.node[6]).m_tex;
		float t8 = pm->Node(face.node[7]).m_tex;

		glTexCoord1f(t8); glVertex3f(r8.x, r8.y, r8.z);
		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);

		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);
		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);

		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);

		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);
		glTexCoord1f(t8); glVertex3f(r8.x, r8.y, r8.z);

		glTexCoord1f(t8); glVertex3f(r8.x, r8.y, r8.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);

		glTexCoord1f(t8); glVertex3f(r8.x, r8.y, r8.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);
	}
	glEnd();
}

void CGLModel::RenderTexTRI3(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		vec3f r1 = pm->Node(face.node[0]).m_rt;
		vec3f r2 = pm->Node(face.node[1]).m_rt;
		vec3f r3 = pm->Node(face.node[2]).m_rt;

		vec3f nf = (r2 - r1) ^ (r3 - r1);
		nf.Normalize();
		glNormal3f(nf.x, nf.y, nf.z);

		float t1 = pm->Node(face.node[0]).m_tex;
		float t2 = pm->Node(face.node[1]).m_tex;
		float t3 = pm->Node(face.node[2]).m_tex;

		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
	}
	glEnd();
}

void CGLModel::RenderTexTRI6(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		vec3f r1 = pm->Node(face.node[0]).m_rt;
		vec3f r2 = pm->Node(face.node[1]).m_rt;
		vec3f r3 = pm->Node(face.node[2]).m_rt;
		vec3f r4 = pm->Node(face.node[3]).m_rt;
		vec3f r5 = pm->Node(face.node[4]).m_rt;
		vec3f r6 = pm->Node(face.node[5]).m_rt;

		vec3f nf = (r2 - r1) ^ (r3 - r1);
		nf.Normalize();
		glNormal3f(nf.x, nf.y, nf.z);

		float t1 = pm->Node(face.node[0]).m_tex;
		float t2 = pm->Node(face.node[1]).m_tex;
		float t3 = pm->Node(face.node[2]).m_tex;
		float t4 = pm->Node(face.node[3]).m_tex;
		float t5 = pm->Node(face.node[4]).m_tex;
		float t6 = pm->Node(face.node[5]).m_tex;

		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);

		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);

		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);

		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
	}
	glEnd();
}

void CGLModel::RenderTexTRI7(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_TRIANGLES);
	{
		vec3f r1 = pm->Node(face.node[0]).m_rt;
		vec3f r2 = pm->Node(face.node[1]).m_rt;
		vec3f r3 = pm->Node(face.node[2]).m_rt;
		vec3f r4 = pm->Node(face.node[3]).m_rt;
		vec3f r5 = pm->Node(face.node[4]).m_rt;
		vec3f r6 = pm->Node(face.node[5]).m_rt;
		vec3f r7 = pm->Node(face.node[6]).m_rt;

		vec3f nf = (r2 - r1) ^ (r3 - r1);
		nf.Normalize();
		glNormal3f(nf.x, nf.y, nf.z);

		float t1 = pm->Node(face.node[0]).m_tex;
		float t2 = pm->Node(face.node[1]).m_tex;
		float t3 = pm->Node(face.node[2]).m_tex;
		float t4 = pm->Node(face.node[3]).m_tex;
		float t5 = pm->Node(face.node[4]).m_tex;
		float t6 = pm->Node(face.node[5]).m_tex;
		float t7 = pm->Node(face.node[6]).m_tex;

		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);

		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);
		glTexCoord1f(t4); glVertex3f(r4.x, r4.y, r4.z);

		glTexCoord1f(t2); glVertex3f(r2.x, r2.y, r2.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);

		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);
		glTexCoord1f(t5); glVertex3f(r5.x, r5.y, r5.z);

		glTexCoord1f(t3); glVertex3f(r3.x, r3.y, r3.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);

		glTexCoord1f(t1); glVertex3f(r1.x, r1.y, r1.z);
		glTexCoord1f(t7); glVertex3f(r7.x, r7.y, r7.z);
		glTexCoord1f(t6); glVertex3f(r6.x, r6.y, r6.z);
	}
	glEnd();
}

void CGLModel::RenderFace1Outline(FEFace& face, FEMeshBase* pm)
{
	glBegin(GL_LINE_LOOP);
	{
		int N = face.Nodes();
		for (int i = 0; i < N; ++i)
		{
			vec3f r = pm->Node(face.node[i]).m_rt; glVertex3f(r.x, r.y, r.z);
		}
	}
	glEnd();
}

void CGLModel::RenderFace2Outline(FEFace& face, FEMeshBase* pm, int ndivs)
{
	vec3f a[3];
	glBegin(GL_LINE_LOOP);
	{
		int NE = face.Edges();
		for (int i = 0; i<NE; ++i)
		{
			FEEdge e = face.Edge(i);
			a[0] = pm->Node(e.node[0]).m_rt;
			a[1] = pm->Node(e.node[1]).m_rt;
			a[2] = pm->Node(e.node[2]).m_rt;
			const int M = 2 * ndivs;
			for (int n = 0; n<M; ++n)
			{
				double t = n / (double)M;
				vec3f p = e.eval(a, t);
				glVertex3f(p.x, p.y, p.z);
			}
		}
	}
	glEnd();

}

void CGLModel::RenderFace3Outline(FEFace& face, FEMeshBase* pm, int ndivs)
{
	vec3f a[4];
	glBegin(GL_LINE_LOOP);
	{
		int NE = face.Edges();
		for (int i = 0; i<NE; ++i)
		{
			FEEdge e = face.Edge(i);
			a[0] = pm->Node(e.node[0]).m_rt;
			a[1] = pm->Node(e.node[1]).m_rt;
			a[2] = pm->Node(e.node[2]).m_rt;
			a[3] = pm->Node(e.node[3]).m_rt;
			const int M = 2 * ndivs;
			for (int n = 0; n<M; ++n)
			{
				double t = n / (double)M;
				vec3f p = e.eval(a, t);
				glVertex3f(p.x, p.y, p.z);
			}
		}
	}
	glEnd();
}
