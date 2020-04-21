#include "FELineMesh.h"
#include <GeomLib/GObject.h>

FELineMesh::FELineMesh() : m_pobj(0)
{
}

//-----------------------------------------------------------------------------
void FELineMesh::UpdateSelection()
{
	GObject* po = GetGObject();
	if (po) po->UpdateSelection();
}

//-----------------------------------------------------------------------------
// Tag all nodes
void FELineMesh::TagAllNodes(int ntag)
{
	const int NN = Nodes();
	for (int i = 0; i<NN; ++i) Node(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
// Tag all edges
void FELineMesh::TagAllEdges(int ntag)
{
	const int NE = Edges();
	for (int i = 0; i<NE; ++i) Edge(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
void FELineMesh::SetGObject(GObject* po) { m_pobj = po; }

//-----------------------------------------------------------------------------
GObject* FELineMesh::GetGObject() { return m_pobj; }

//-----------------------------------------------------------------------------
// convert local coordinates to global coordinates. This uses the transform
// info from the parent object.
vec3d FELineMesh::LocalToGlobal(const vec3d& r) const
{
	if (m_pobj) return m_pobj->GetTransform().LocalToGlobal(r);
	else return r;
}

//-----------------------------------------------------------------------------
vec3d FELineMesh::GlobalToLocal(const vec3d& r) const
{
	if (m_pobj) return m_pobj->GetTransform().GlobalToLocal(r);
	else return r;
}

//-----------------------------------------------------------------------------
// return the position of a node in global coordinates
vec3d FELineMesh::NodePosition(int i) const
{
	return LocalToGlobal(Node(i).r);
}

//-----------------------------------------------------------------------------
// return the local position of a node
vec3d FELineMesh::NodeLocalPosition(int i) const
{
	return Node(i).r;
}
