#include "stdafx.h"
#include "GLMesh.h"

//-----------------------------------------------------------------------------
GLMesh::GLMesh(void)
{
}

//-----------------------------------------------------------------------------
GLMesh::GLMesh(GLMesh& m)
{
	m_Node = m.m_Node;
	m_Edge = m.m_Edge;
	m_Face = m.m_Face;
	m_box = m.m_box;
	Update();
}

//-----------------------------------------------------------------------------
GLMesh::~GLMesh(void)
{
}
