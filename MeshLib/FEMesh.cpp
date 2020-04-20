#include "FEMesh.h"
#include <GeomLib/GObject.h>
#include "triangulate.h"
#include "FESurfaceMesh.h"
#include "MeshMetrics.h"
#include "FENodeElementList.h"
#include "FENodeFaceList.h"
#include "FENodeEdgeList.h"
#include "MeshTools/FENodeData.h"
#include "MeshTools/FESurfaceData.h"
#include "MeshTools/FEElementData.h"
#include <algorithm>
#include <unordered_set>
#include <map>
using namespace std;

double bias(double b, double x)
{
	const double f = 1.f / (double) log(0.5);
	return (double) pow(x, log(b)*f);
}

double gain(double g, double x)
{
	if (x < 0.5f)
		return bias(1.f-g, 2.f*x)*0.5f;
	else
		return 1.f - bias(1.f-g, 2.f - 2.f*x)*0.5f;
}

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data()
{
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
Mesh_Data::Mesh_Data(const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::operator = (const Mesh_Data& d)
{
	m_data = d.m_data;
	m_min = d.m_min;
	m_max = d.m_max;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Clear()
{
	m_data.clear();
	m_min = m_max = 0.0;
}

//-----------------------------------------------------------------------------
void Mesh_Data::Init(FEMesh* mesh, double initVal, int initTag)
{
	int NE = mesh->Elements();
	m_data.resize(NE);
	for (int i = 0; i < NE; ++i)
	{
		FEElement& el = mesh->Element(i);
		DATA& di = m_data[i];
		int ne = el.Nodes();
		di.nval = ne;
		di.tag = initTag;
		for (int j = 0; j < ne; ++j)
		{
			di.val[j] = initVal;
		}
	}
}

//-----------------------------------------------------------------------------
// get the average element value
double Mesh_Data::GetElementAverageValue(int elem)
{
	double v = 0.0;
	if (m_data[elem].tag != 0)
	{
		for (int i = 0; i < m_data[elem].nval; ++i) v += m_data[elem].val[i];
		v /= (double)m_data[elem].nval;
	}
	return v;
}

//-----------------------------------------------------------------------------
// set the element (average) value
void Mesh_Data::SetElementValue(int elem, double v)
{
	int ne = m_data[elem].nval;
	for (int i = 0; i < ne; ++i) m_data[elem].val[i] = v;
}

//-----------------------------------------------------------------------------
void Mesh_Data::UpdateValueRange()
{
	m_min = m_max = 0;

	// find the first active value
	int N = (int)m_data.size();
	int i = 0;
	for (i = 0; i<N; ++i)
	{
		if (m_data[i].tag != 0)
		{
			m_min = m_max = m_data[i].val[0];
			break;
		}
	}

	// update range
	for (i=0; i<N; ++i)
	{
		DATA& di = m_data[i];
		if (di.tag != 0)
		{
			for (int j = 0; j < di.nval; ++j)
			{
				if (di.val[j] > m_max) m_max = di.val[j];
				if (di.val[j] < m_min) m_min = di.val[j];
			}
		}
	}
}

//-----------------------------------------------------------------------------
void Mesh_Data::GetValueRange(double& vmin, double& vmax) const
{
	vmin = m_min;
	vmax = m_max;
}

//-----------------------------------------------------------------------------
// default constructor
FEMesh::FEMesh()
{
	m_pobj = 0;
}

//-----------------------------------------------------------------------------
// copy constructor
FEMesh::FEMesh(FEMesh& m)
{
	// create the nodes
	m_Node.resize(m.Nodes());
	for (int i=0; i<Nodes(); ++i) m_Node[i] = m.m_Node[i];

	// create the elements
	m_Elem.resize(m.Elements());
	for (int i = 0; i<Elements(); ++i) m_Elem[i] = m.m_Elem[i];

	// create the faces
	m_Face.resize(m.Faces());
	for (int i = 0; i<Faces(); ++i) m_Face[i] = m.m_Face[i];

	// create the edges
	m_Edge.resize(m.Edges());
	for (int i = 0; i<Edges(); ++i) m_Edge[i] = m.m_Edge[i];

	// copy element data
	m_data = m.m_data;

	// copy bounding box
	m_box = m.m_box;

	// don't copy object (two meshes cannot be owned by the same object)
	m_pobj = 0;
}

//-----------------------------------------------------------------------------
// destructor
FEMesh::~FEMesh()
{
	Clear();
}

//-----------------------------------------------------------------------------
// Clear the mesh data
void FEMesh::Clear()
{
	m_Edge.clear();
	m_Face.clear();
	m_Elem.clear();
	m_Node.clear();

	ClearMeshData();
}

//-----------------------------------------------------------------------------
void FEMesh::ClearMeshData()
{
	m_data.Clear();
	for (int i = 0; i < m_meshData.size(); ++i) delete m_meshData[i];
	m_meshData.clear();
}

//-----------------------------------------------------------------------------
void FEMesh::ClearElements() { m_Elem.clear(); }

//-----------------------------------------------------------------------------
void FEMesh::CreateElements(int elems) { m_Elem.resize(elems); }

//-----------------------------------------------------------------------------
// Allocate storage for the mesh data. If bclear is true (default = true) all 
// existing groups are deleted.
void FEMesh::Create(int nodes, int elems, int faces, int edges)
{
	// allocate storage
	if (nodes > 0) { if (nodes) m_Node.resize(nodes); else m_Node.clear(); }
	if (elems > 0) { if (elems) m_Elem.resize(elems); else m_Elem.clear(); }
	if (faces > 0) { if (faces) m_Face.resize(faces); else m_Face.clear(); }
	if (edges > 0) { if (edges) m_Edge.resize(edges); else m_Edge.clear(); }

	// clear mesh data
	ClearMeshData();
}

//-----------------------------------------------------------------------------
// Detach the selection and return it as a new object
// TODO: I want to remove this function and replace it with a modifier who
// does all the work.
FEMesh* FEMesh::DetachSelectedMesh()
{
	int i, j, n;

	// count selected elements
	int elems = 0;
	for (i=0; i<Elements(); ++i) if (ElementPtr(i)->IsSelected()) ++elems;

	// make sure there is a selection
	if (elems == 0) return 0;

	// make sure there are fewer selected elements than elements
	// otherwise we don't have anything left.
	if (elems == Elements()) return 0;

	// tag nodes that will be moved to the new mesh
	FENode* pn = NodePtr();
	for (i=0; i<Nodes(); ++i, ++pn) pn->m_ntag = -1;

	for (i=0; i<Elements(); ++i)
	{
		FEElement_* pe = ElementPtr(i);
		if (pe->IsSelected())
		{
			n = pe->Nodes();
			for (j=0; j<n; ++j) Node(pe->m_node[j]).m_ntag = 1;
		}
	}

	// count nodes
	int nodes = 0;
	pn = NodePtr();
	for (i=0; i<Nodes(); ++i, ++pn) if (pn->m_ntag > 0) ++nodes;

	// create a new mesh
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);

	// create the new nodes
	pn = NodePtr();
	n = 0;
	for (i=0; i<Nodes(); ++i, ++pn)
	{
		if (pn->m_ntag > 0)
		{
			FENode& node = pm->Node(n);
			node.r  = pn->r;
			pn->m_ntag = n++;
		}
	}
	assert(n == nodes);

	// create the new elements
	n = 0;
	for (i=0; i<Elements(); ++i)
	{
		FEElement_* pe = ElementPtr(i);

		if (pe->IsSelected())
		{
			FEElement& el = pm->Element(n);
			el.SetType(pe->Type());
			
			for (j=0; j<pe->Nodes(); ++j)
			{
                el.m_h[j] = pe->m_h[j];
				el.m_node[j] = Node(pe->m_node[j]).m_ntag;
				assert(el.m_node[j] >= 0);
			}
			++n;
		}
	}
	assert(n==elems);

	// update the new mesh (is done later)
//	pm->Update();

	vector<int> ELT;
	ELT.assign(Elements(), -1);

	// the new mesh is created, so let's clean up this mesh
	// delete selected elements
	n = 0;
	for (i=0; i<Elements(); ++i)
	{
		FEElement& e0 = Element(i);
		FEElement& e1 = Element(n);

		if (!e0.IsSelected())
		{
			e1 = e0;
			ELT[i] = n;
			++n;
		}
	}
	m_Elem.resize(n);
	m_data.Clear();

	// tag nodes which will be kept
	pn = NodePtr();
	for (i=0; i<Nodes(); ++i, ++pn) pn->m_ntag = -1;

	for (i=0; i<Elements(); ++i)
	{
		FEElement_* pe = ElementPtr(i);
		n = pe->Nodes();
		for (j=0; j<n; ++j) Node(pe->m_node[j]).m_ntag = 1;
	}

	// reindex the nodes
	n = 0;
	pn = NodePtr();
	for (i=0; i<Nodes(); ++i, ++pn) if (pn->m_ntag > 0) pn->m_ntag = n++;

	for (i=0; i<Elements(); ++i)
	{
		FEElement_* pe = ElementPtr(i);
		n = pe->Nodes();
		for (j=0; j<n; ++j)	pe->m_node[j] = Node(pe->m_node[j]).m_ntag;
	}

	// delete untagged nodes
	n = 0;
	for (i=0; i<Nodes(); ++i)
	{
		FENode& n0 = Node(i);
		FENode& n1 = Node(n);

		if (n0.m_ntag >= 0)
		{
			n1 = n0;
			++n;
		}
	}
	m_Node.resize(n);

	// update the mesh (is done later)
///	Update();

	RebuildMesh();
	pm->RebuildMesh();

	// Done!
	return pm;
}

//-----------------------------------------------------------------------------
void FEMesh::ResizeNodes(int newSize)
{
	m_Node.resize(newSize);
}

//-----------------------------------------------------------------------------
void FEMesh::ResizeEdges(int newSize)
{
	m_Edge.resize(newSize);
}

//-----------------------------------------------------------------------------
void FEMesh::ResizeFaces(int newSize)
{
	m_Face.resize(newSize);
}

//-----------------------------------------------------------------------------
void FEMesh::ResizeElems(int newSize)
{
	m_Elem.resize(newSize);
}

//-----------------------------------------------------------------------------
// Remove nodes that are not referenced by any elements.
void FEMesh::RemoveIsolatedNodes()
{
	// find the isolated nodes
	TagAllNodes(-1);
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = Element(i);
		int n = el.Nodes();
		for (int j=0; j<n; ++j) Node(el.m_node[j]).m_ntag = 1;
	}

	// reindex the nodes
	int n = 0;
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_ntag == 1) node.m_ntag = n++;
	}

	// fix element node numbering
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = Element(i);
		int n = el.Nodes();
		for (int j=0; j<n; ++j) el.m_node[j] = Node(el.m_node[j]).m_ntag;
	}

	// fix face node numbering
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		int n = face.Nodes();
		for (int j=0; j<n; ++j) face.n[j] = Node(face.n[j]).m_ntag;
	}

	// fix edge node numbering
	for (int i=0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		int n = edge.Nodes();
		for (int j=0; j<n; ++j) edge.n[j] = Node(edge.n[j]).m_ntag;
	}

	// remove the isolated nodes
	n = 0;
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& n1 = Node(i);
		FENode& n2 = Node(n);

		if (n1.m_ntag >= 0)
		{
			n2 = n1;
			++n;
		}
	}

	// adjust the node container size
	m_Node.resize(n);

	// If the deleted nodes had GIDs set, we need to readjust the GIDs.
	UpdateNodePartitions();
}

//-----------------------------------------------------------------------------
// This functions update the node GIds to make sure that no indices are skipped.
// This needs to be called after the number of nodes changes.
void FEMesh::UpdateNodePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_gid > max_gid) max_gid = node.m_gid;
	}

	// if no node has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_gid >= 0) gid[node.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i=0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Nodes(); ++i)
		{
			FENode& node = Node(i);
			if (node.m_gid >= 0) node.m_gid = gid[node.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the edge GIds to make sure that no indices are skipped.
// This needs to be called after the number of edges changes.
void FEMesh::UpdateEdgePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if (edge.m_gid > max_gid) max_gid = edge.m_gid;
	}

	// if no edge has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if (edge.m_gid >= 0) gid[edge.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Edges(); ++i)
		{
			FEEdge& edge = Edge(i);
			if (edge.m_gid >= 0) edge.m_gid = gid[edge.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the face GIds to make sure that no indices are skipped.
// This needs to be called after the number of faces changes.
void FEMesh::UpdateFacePartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_gid > max_gid) max_gid = face.m_gid;
	}

	// if no face has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_gid >= 0) gid[face.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Faces(); ++i)
		{
			FEFace& face = Face(i);
			if (face.m_gid >= 0) face.m_gid = gid[face.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the face smoothing Ids to make sure that no indices are skipped.
// This needs to be called after the number of faces changes.
void FEMesh::UpdateSmoothingGroups()
{
	// find the largest SG
	int max_sg = -1;
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_sid > max_sg) max_sg = face.m_sid;
	}

	// if no face has a GID we are done
	if (max_sg < 0) return;

	// build a SID lookup table
	vector<int> sg(max_sg + 1, -1);
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_sid >= 0) sg[face.m_sid] = 1;
	}

	// assign new SIDs
	int n = 0;
	for (int i = 0; i<sg.size(); ++i)
	{
		if (sg[i] != -1) sg[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < sg.size())
	{
		for (int i = 0; i<Faces(); ++i)
		{
			FEFace& face = Face(i);
			if (face.m_sid >= 0) face.m_sid = sg[face.m_sid];
		}
	}
}

//-----------------------------------------------------------------------------
// This functions update the node GIds to make sure that no indices are skipped.
// This needs to be called after the number of elements changes.
void FEMesh::UpdateElementPartitions()
{
	// find the largest GID
	int max_gid = -1;
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement& elem = Element(i);
		if (elem.m_gid > max_gid) max_gid = elem.m_gid;
	}

	// if no element has a GID we are done
	if (max_gid < 0) return;

	// build a GID lookup table
	vector<int> gid(max_gid + 1, -1);
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement& elem = Element(i);
		if (elem.m_gid >= 0) gid[elem.m_gid] = 1;
	}

	// assign new GIDs
	int n = 0;
	for (int i = 0; i<gid.size(); ++i)
	{
		if (gid[i] != -1) gid[i] = n++;
	}

	// only update node GIDs when necessary
	if (n < gid.size())
	{
		for (int i = 0; i<Elements(); ++i)
		{
			FEElement& elem = Element(i);
			if (elem.m_gid >= 0) elem.m_gid = gid[elem.m_gid];
		}
	}
}

//-----------------------------------------------------------------------------
// Delete selected elements
void FEMesh::DeleteSelectedElements()
{
	// tag all selected elements
	TagAllElements(0);
	for (int i=0; i<Elements(); ++i)
		if (Element(i).IsSelected()) Element(i).m_ntag = 1;

	// delete tagged elements
	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Delete tagged elements
void FEMesh::DeleteTaggedElements(int tag)
{
	// Let's go ahead and remove all tagged elements
	RemoveElements(tag);
	UpdateElementPartitions();

	// remove isolated nodes
	RemoveIsolatedNodes();

	// rebuild mesh
	RebuildMesh();
}

/*
 TODO: This doesn't seem to work when there are shells on top of solids. Need to revisit.
//-----------------------------------------------------------------------------
// Delete tagged elements
void FEMesh::DeleteTaggedElements(int tag)
{
	// first figure out which nodes will be deleted
	TagAllNodes(1);
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = Element(i);
		if (el.m_ntag != tag)
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) Node(el.m_node[j]).m_ntag = 0;
		}
	}

	// remove all edges that reference a node tagged for removal
	for (int i=0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		edge.m_ntag = 0;
		if ((Node(edge.n[0]).m_ntag == 1) || (Node(edge.n[1]).m_ntag == 1))
		{
			edge.m_ntag = 1;
		}
	}
	RemoveEdges(1);
	UpdateEdgePartitions();

	// tag all faces for removal
	TagAllFaces(0);
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		
		if (face.m_elem[0] >= 0)
		{
			FEElement& el = Element(face.m_elem[0]);
			if (el.m_ntag == tag) face.m_ntag = 1;
		}

		if (face.m_elem[1] >= 0)
		{
			FEElement& el = Element(face.m_elem[1]);
			if (el.m_ntag == tag) face.m_ntag = 1;
		}
	}
	RemoveFaces(1);
	UpdateFacePartitions();
	UpdateSmoothingGroups();

	// Removing elements may expose new faces, so we need to add those new faces first, before deleting any elements
	// For now, all these faces will be given a new GID
	int faceGID = CountFacePartitions();
	int faceSG = CountSmoothingGroups();
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = Element(i);
		if (el.m_ntag != tag)
		{
			int nf = el.Faces();
			for (int j=0; j<nf; ++j)
			{
				int neighbor = el.m_nbr[j];
				if (neighbor >= 0)
				{
					FEElement& elj = Element(neighbor);
					if (elj.m_ntag == tag)
					{
						FEFace facej = el.GetFace(j);
						facej.m_gid = faceGID;
						facej.m_sid = faceSG;
						facej.m_elem[0] = i;
						AddFace(facej);
					}
				}
			}
		}
	}

	// Let's go ahead and remove all tagged elements
	RemoveElements(tag);
	UpdateElementPartitions();

	// remove isolated nodes
	RemoveIsolatedNodes();

	// At this point, the mesh is almost back in order.
	// Just need to update a few more things
	// First restore all element neighbors
	UpdateElementNeighbors();

	// Then, restore all face neighbors
	UpdateFaces();

	// now we insert new edges where a new face meets an old face
	int edgeID = CountEdgePartitions();
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_gid == faceGID)
		{
			int ne = face.Edges();
			for (int j=0; j<ne; ++j)
			{
				if ((face.m_nbr[j] == -1) || (Face(face.m_nbr[j]).m_gid != faceGID))
				{
					FEEdge ej = face.GetEdge(j);
					ej.m_gid = edgeID;
					AddEdge(ej);
				}
			}
		}
	}

	// if the new faces defines a multiple connected region, a different GID is needed for each region.
	SplitFacePartition(faceGID);

	// update the edge neighbors
	UpdateEdgeNeighbors();

	// it is required that node partitions are placed at all edge end points.
	int nodeID = CountNodePartitions();
	for (int i=0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if (edge.m_nbr[0] == -1)
		{
			FENode& n0 = Node(edge.n[0]);
			if (n0.m_gid == -1) n0.m_gid = nodeID++;
		}
		if (edge.m_nbr[1] == -1)
		{
			FENode& n1 = Node(edge.n[1]);
			if (n1.m_gid == -1) n1.m_gid = nodeID++;
		}
	}

	// okay, but now the edge neighbors need to be fixed.
	for (int i=0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if (Node(edge.n[0]).m_gid != -1)
		{
			edge.m_nbr[0] = -1;
		}
		if (Node(edge.n[1]).m_gid != -1)
		{
			edge.m_nbr[1] = -1;
		}
	}

	// Great, now split the edges into multiple components
	SplitEdgePartition(edgeID);

	// let's update the normals
	UpdateNormals();

	// recalculate the box
	UpdateBox();

	// and we're done!
}
*/

//-----------------------------------------------------------------------------
// Splits edge partitions so that each edge curve has only two ends.
// Assumes that edge neighbors are correct
void FEMesh::SplitEdgePartition(int edgeID)
{
	// tag all edges that we'll be updating
	TagAllEdges(0);
	for (int i=0; i<Edges(); ++i)
		if (Edge(i).m_gid == edgeID) Edge(i).m_ntag = 1;

	stack<int> s;
	int i0 = 0;
	do
	{
		// find an edge
		for (int i=i0; i<Edges(); ++i, ++i0)
		{
			FEEdge& edge = Edge(i);
			if (edge.m_ntag == 1)
			{
				s.push(i);
				break;	
			}
		}

		// if we didn't find anything, we're done
		if (s.empty()) break;

		// process edge curve
		while (s.empty() == false)
		{
			int eid = s.top(); s.pop();
			FEEdge& edge = Edge(eid);
			edge.m_gid = edgeID;
			edge.m_ntag = 0;

			if (edge.m_nbr[0] >= 0)
			{
				if (Edge(edge.m_nbr[0]).m_ntag == 1) s.push(edge.m_nbr[0]);
			}
			if (edge.m_nbr[1] >= 0)
			{
				if (Edge(edge.m_nbr[1]).m_ntag == 1) s.push(edge.m_nbr[1]);
			}
		}

		// increase edge ID
		edgeID++;
	}
	while (true);
}

//-----------------------------------------------------------------------------
// Splits face partitions based on connectivity. This splits disconnected partitions
// Assumes that face neighbors are correct
void FEMesh::SplitFacePartition(int faceID)
{
	// tag all faces that we'll be updating
	TagAllFaces(0);
	for (int i = 0; i<Faces(); ++i)
		if (Face(i).m_gid == faceID) Face(i).m_ntag = 1;

	stack<int> s;
	int i0 = 0;
	do
	{
		// find a face
		for (int i = i0; i<Faces(); ++i, ++i0)
		{
			FEFace& face = Face(i);
			if (face.m_ntag == 1)
			{
				s.push(i);
				break;
			}
		}

		// if we didn't find anything, we're done
		if (s.empty()) break;

		// process surface
		while (s.empty() == false)
		{
			int fid = s.top(); s.pop();
			FEFace& face = Face(fid);
			face.m_gid = faceID;
			face.m_ntag = 0;

			for (int i=0; i<face.Edges(); ++i)
			{
				if (face.m_nbr[i] >= 0)
				{
					if (Face(face.m_nbr[i]).m_ntag == 1) s.push(face.m_nbr[i]);
				}
			}
		}

		// increase face ID
		faceID++;
	}
	while (true);
}

//-----------------------------------------------------------------------------
// Remove elements with tag ntag
void FEMesh::RemoveElements(int ntag)
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement& e1 = Element(i);
		FEElement& e2 = Element(n);

		if (e1.m_ntag != ntag)
		{
			if (i != n)
			{
				e2 = e1;
				m_data[n] = m_data[i];
			}
			n++;
		}
	}

	m_Elem.resize(n);
	m_data.Clear();
}

//-----------------------------------------------------------------------------
// Delete tagged faces
void FEMesh::DeleteTaggedFaces(int tag)
{
	TagAllElements(0);
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_ntag == tag)
		{
			if (face.m_elem[0].eid >= 0)
			{
				Element(face.m_elem[0].eid).m_ntag = 1;
			}
			if (face.m_elem[1].eid >= 0)
			{
				Element(face.m_elem[1].eid).m_ntag = 1;
			}
		}
	}

	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Delete tagged edges
void FEMesh::DeleteTaggedEdges(int ntag)
{
	// TODO: identify elements which should be deleted instead.
	assert(false);

	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
//! This function identifies duplicate faces and returns a list with the duplicates
void FEMesh::FindDuplicateFaces(vector<int>& l)
{
	l.clear();
	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& fi = Face(i);
		for (int j=i+1; j<NF; ++j)
		{
			FEFace& fj = Face(j);
			if (fi == fj) l.push_back(j);
		}
	}
}

//-----------------------------------------------------------------------------
//! This function identifies duplicate edges and returns a list with the duplicates
void FEMesh::FindDuplicateEdges(vector<int>& l)
{
	l.clear();
	int NL = Edges();
	for (int i=0; i<NL; ++i)
	{
		FEEdge& ei = Edge(i);
		for (int j=i+1; j<NL; ++j)
		{
			FEEdge& ej = Edge(j);
			if (ei == ej) l.push_back(j);
		}
	}
}


//-----------------------------------------------------------------------------
// Delete selected faces
void FEMesh::DeleteSelectedFaces()
{
	// tag all selected faces
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		face.m_ntag = (face.IsSelected() ? 1 : 0);
	}

	// delete tagged faces
	DeleteTaggedFaces(1);
}

//-----------------------------------------------------------------------------
// Delete selected nodes
void FEMesh::DeleteSelectedNodes()
{
	// tag all selected nodes
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		node.m_ntag = (node.IsSelected() ? 1 : 0);
	}

	// delete tagged nodes
	DeleteTaggedNodes(1);
}

//-----------------------------------------------------------------------------
// This function deletes tagged nodes by deleting all the elements that contain this node
void FEMesh::DeleteTaggedNodes(int tag)
{
	// tag all the elements that has this node
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& el = Element(i);
		el.m_ntag = 0;
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			if (Node(el.m_node[j]).m_ntag == tag)
			{
				el.m_ntag = 1;
				break;
			}
		}
	}

	// now delete all the tagged elements
	DeleteTaggedElements(1);
}

//-----------------------------------------------------------------------------
// Build the node-node table for surface nodes only. That is table of node indices that each node
// connects to.
void FEMesh::BuildSurfaceNodeNodeTable(vector<set<int> >& NNT)
{
	// reset node-node table
	int NN = Nodes();
	NNT.resize(NN);
	for (int i=0; i<NN; ++i) NNT[i].clear();

	// loop over all faces
	int NF = Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j)
		{
			int nj = f.n[j];
			for (int k=0; k<nf; ++k)
			{
				int nk = f.n[k];
				if (nj != nk) NNT[nj].insert(nk);										
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Update mesh data
void FEMesh::Update()
{
	// update the element neighbours
	UpdateElementNeighbors();

	// update face neighbours
	UpdateFaces();

	// update the edges
	if (Edges() == 0)
	{
		BuildEdges();
		AutoPartitionEdges();
		AutoPartitionNodes();
	}
	else UpdateEdgeNeighbors();

	// update normals
	UpdateNormals();

	// now we can figure out which nodes are interior and which are exterior
	MarkExteriorNodes();

	// update the bounding box
	UpdateBox();
}

//-----------------------------------------------------------------------------
// This function finds the element neighbours.
//
void FEMesh::UpdateElementNeighbors()
{
	// get number of elements
	int elems = Elements();

	// reset all element neighbor and face ptrs
	for (int i=0; i<elems; i++)
	{
		FEElement_& el = ElementRef(i);
		el.m_ntag = i;
		for (int j=0; j<6; ++j) 
		{
			el.m_nbr[j] = -1;
			el.m_face[j] = -1;
		}
	}

	// calculate the node-element table
	FENodeElementList NET;
	NET.Build(this);

	// set up the element's neighbour pointers
	FEEdge edge;
	int nbrf, nbre;
	FEFace f1, f2, f3;

	// assign neighbor elements
	for (int i = 0; i<elems; i++)
	{
		FEElement_* pe = ElementPtr(i);
		// do the solid elements
		int n = pe->Faces();
		for (int j = 0; j<n; j++)
		{
			if (pe->m_nbr[j] == -1)
			{
				pe->GetFace(j, f1);

				// find the neighbour element
				int inode = f1.n[0];
				int nval = NET.Valence(inode);
				bool bfound = false;
				for (int k=0; k < nval; k++)
				{
					int nbe = NET.ElementIndex(inode, k);
					FEElement_* pne = ElementPtr(nbe);
					if (pne != pe)
					{
						nbrf = pne->Faces();
						int l;
						for (l = 0; l<nbrf; l++)
						{
							pne->GetFace(l, f2);
							if (f1 == f2)
							{
								bfound = true;
								break;
							}
						}

						if (bfound)
						{
							pe->m_nbr[j] = nbe;
							pne->m_nbr[l] = i;
							break;
						}
					}
				}
			}
		}

		// do the shell elements
		n = pe->Edges();
		for (int j=0; j<n; j++)
		{
			if (pe->m_nbr[j] == -1)
			{
				edge = pe->GetEdge(j);

				// find the neighbour element
				int inode = edge.n[0];
				int nval = NET.Valence(inode);
				bool bfound = false;
				for (int k=0; k < nval; k++)
				{
					FEElement_* pne = NET.Element(inode, k);
					if ((pne != pe) && (pe->is_equal(*pne) == false))
					{
						nbre = pne->Edges();
						for (int l=0; l<nbre; l++) 
							if (edge == pne->GetEdge(l))
							{
								pe->m_nbr[j] = NET.ElementIndex(inode, k);
								pne->m_nbr[l] = i;
								break;
							}
					}
				}
			}
		}

		// do the beam elements
		if (pe->IsType(FE_BEAM2))
		{
			for (int j = 0; j<2; ++j)
			{				
				pe->m_nbr[j] = -1;
				pe->m_face[j] = -1;
				int inode = pe->m_node[j];
				int nval = NET.Valence(inode);
				for (int k=0; k<nval; ++k)
				{
					FEElement_* pne = NET.Element(inode, k);
					if (pne != pe)
					{
						if ((pne->IsType(FE_BEAM2)) && ((pne->m_node[0] == pe->m_node[j]) || (pne->m_node[1] == pe->m_node[j])))
						{
							pe->m_nbr[j] = NET.ElementIndex(inode, k);
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// helper function for checking if two faces can be neighbours
bool isValidFaceNeighbor(FEFace& f0, FEFace& f1)
{
	// Make sure they are both external or both non-external
	if (f0.IsExternal() != f1.IsExternal()) return false;

	// Make sure they are not the same
	if (f0 == f1) return false;

	// check some special cases
	if ((f0.Nodes() == 3) && (f1.Nodes() == 4))
	{
		// see if f0 overlaps with f1
		if ((f1.FindNode(f0.n[0]) != -1) && (f1.FindNode(f0.n[1]) != -1) && (f1.FindNode(f0.n[2]) != -1)) return false;	
	}

	if ((f1.Nodes() == 3) && (f0.Nodes() == 4))
	{
		// see if f1 overlaps with f0
		if ((f0.FindNode(f1.n[0]) != -1) && (f0.FindNode(f1.n[1]) != -1) && (f0.FindNode(f1.n[2]) != -1) ) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// This function sets the face-element connectivity
void FEMesh::UpdateFaceElementTable()
{
	int NF = Faces(); 
	int NE = Elements();
	if ((NF == 0) || (NE == 0)) return;

	// clear all face-element connectivity
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = Face(i);
		f.m_elem[0].eid = -1;
		f.m_elem[1].eid = -1;
	}

	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = Element(i);

		// solid elements
		int nf = el.Faces();
		for (int i = 0; i<nf; ++i)
		{
			el.m_face[i] = -1;
		}

		// shell elements
		int ne = el.Edges();
		if (ne > 0)
		{
			el.m_face[0] = -1;
		}
	}

	// first build the node element table
	FENodeElementList NET;
	NET.Build(this);

	// loop over all faces
	FEFace f2;
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = Face(i);

		int n0 = face.n[0];
		int nval = NET.Valence(n0);
		int m = 0;
		for (int j=0; j<nval; ++j)
		{
			int eid = NET.ElementIndex(n0, j);
			FEElement_* pej = ElementPtr(eid);

			// solid elements
			int n = pej->Faces();
			for (int k = 0; k<n; ++k)
			{
				if (pej->m_face[k] == -1)
				{
					pej->GetFace(k, f2);
					if (f2 == face)
					{
//						assert(m<2);
						if (m == 0)
						{
							face.m_elem[m  ].eid = eid;
							face.m_elem[m++].lid = k;
						}
						else if (m < 2)
						{
							// set the element with the lowest GID first
							FEElement_* p0 = ElementPtr(face.m_elem[0].eid);
							if (p0->m_gid < pej->m_gid)
							{
								face.m_elem[m  ].eid = eid;
								face.m_elem[m++].lid = k;
							}
							else
							{
								face.m_elem[m  ].eid = face.m_elem[0].eid;
								face.m_elem[m++].lid = face.m_elem[0].lid;

								face.m_elem[0].eid = eid;
								face.m_elem[0].lid = k;
							}
						}
						pej->m_face[k] = i;
					}
				}
			}
		}

		// shells
		for (int j=0; j<nval; ++j)
		{
			int eid = NET.ElementIndex(n0, j);
			FEElement_* pej = ElementPtr(eid);

			int n = pej->Edges();
			if (n > 0)
			{
				if (pej->m_face[0] == -1)
				{
					pej->GetShellFace(f2);
					if (f2 == face)
					{
						if (m == 0) 
						{	
							face.m_elem[m  ].eid = eid;
							face.m_elem[m++].lid = 0;
							pej->m_face[0] = i;
						}
					}
				}
			}
		}

		assert(face.m_elem[0].eid != -1);
	}
}

//-----------------------------------------------------------------------------
//! This function finds the face neighbours. Note that internal faces cannot
//! be neighbours of external faces. 
void FEMesh::UpdateFaceNeighbors()
{
	int NF = Faces();

	// make sure we have faces
	if (NF == 0) return;

	// Tag faces based on their part connectivity
	TagAllFaces(0);
	TagAllElements(0);
	stack<int> S;
	S.push(0);
	int np = 1;
	int i0 = 1;
	do
	{
		while (S.empty() == false)
		{
			int elem = S.top(); S.pop();
			FEElement& el = Element(elem);
			el.m_ntag = 1;

			int nf = el.Faces();
			for (int i=0; i<nf; ++i)
			{
				if (el.m_face[i] != -1)
				{
					Face(el.m_face[i]).m_ntag = np;
				}

				if (el.m_nbr[i] != -1)
				{
					FEElement& ej = Element(el.m_nbr[i]);
					if (ej.m_ntag == 0)
					{
						ej.m_ntag = 1;
						S.push(el.m_nbr[i]);
					}
				}
			}
		}
		np++;

		for (int i=i0; i<Elements(); ++i)
		{
			FEElement& el = Element(i);
			if (el.m_ntag == 0)
			{
				S.push(i);
				i0 = i + 1;
				break;
			}
		}
	}
	while (S.empty() == false);

	// build the node-face table
	FENodeFaceList NFT;
	NFT.Build(this);

	// find all face neighbours
	int n[4];
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);

		int ne = pf->Edges();
		for (int j = 0; j<ne; ++j)
		{
			pf->GetEdgeNodes(j, n);
			int nval = NFT.Valence(n[0]);
			pf->m_nbr[j] = -1;
			for (int k = 0; k<nval; ++k)
			{
				FEFace* pfn = NFT.Face(n[0], k);
				if (pfn != pf)
				{
					// See if the faces share an edge
					if (pfn->HasEdge(n[0], n[1]) && (pf->m_ntag == pfn->m_ntag))
					{
						// see if they are both external or both internal
						if (isValidFaceNeighbor(*pf, *pfn))
						{
							pf->m_nbr[j] = NFT.FaceIndex(n[0], k);
							break;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// This function finds the edge neighbours.
// An edge can only have a neighbor that has the same group ID.
void FEMesh::UpdateEdgeNeighbors()
{
	// build the node-edge table
	FENodeEdgeList NET; NET.Build(this);

	// find neighbours
	int NE = Edges();
	for (int i=0; i<NE; ++i)
	{
		FEEdge* pe = EdgePtr(i);
		for (int j=0; j<2; ++j)
		{
			pe->m_nbr[j] = -1;
			int n = pe->n[j];
			int nval = NET.Edges(n);
			if (nval == 2)
			{
				for (int k=0; k<2; ++k)
				{
					const FEEdge* pen = NET.Edge(n, k);
					if ((pen != pe)&&(pen->m_gid == pe->m_gid)) pe->m_nbr[j] = NET.EdgeIndex(n, k);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Attach another to this mesh.
//
void FEMesh::Attach(FEMesh& fem)
{
	int i, j, n;

	int nn0 = Nodes();
	int nn1 = fem.Nodes();

	int ne0 = Elements();
	int ne1 = fem.Elements();

	int nf0 = Faces();
	int nf1 = fem.Faces();

	int nl0 = Edges();
	int nl1 = fem.Edges();

	int nodes = nn0 + nn1;
	int elems = ne0 + ne1;
	int faces = nf0 + nf1;
	int edges = nl0 + nl1;

	GObject* po1 = m_pobj;
	GObject* po2 = fem.m_pobj;

	// create the nodes
	if (nodes > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i=0; i<nn0; ++i)
		{
			FENode& n = m_Node[i];
			if (n.m_gid > ng) ng = n.m_gid;
		}
		++ng;

		m_Node.resize(nodes);
		for (i=0; i<nn1; ++i)
		{
			FENode& n0 = m_Node[nn0 + i];
			FENode& n1 = fem.m_Node[i];
			n0 = n1;
			if (n0.m_gid >= 0) n0.m_gid = n1.m_gid + ng;
			assert(po2);
			if (po2) n0.r = po1->GetTransform().GlobalToLocal(po2->GetTransform().LocalToGlobal(n1.r));
			else n0.r = n1.r;
		}
	}

	// create the faces
	if (faces > 0)
	{
		// find the largest GID
		int ng = -1, sg = -1;
		for (i=0; i<nf0; ++i)
		{
			FEFace& f = m_Face[i];
			if (f.m_gid > ng) ng = f.m_gid;
			if (f.m_sid > sg) sg = f.m_gid;
		}
		++ng; ++sg;

		m_Face.resize(faces);
		for (i=0; i<nf1; ++i)
		{
			FEFace& f0 = m_Face[nf0 + i];
			FEFace& f1 = fem.m_Face[i];
			f0 = f1;
			f0.m_gid = f1.m_gid + ng;
			f0.m_sid = f1.m_sid + sg;

			f0.m_elem[0].eid = f1.m_elem[0].eid + ne0;
			f0.m_elem[1].eid = -1;

			for (int j=0; j<f0.Nodes(); ++j) f0.n[j] = f1.n[j] + nn0;

			f0.m_nbr[0] = f1.m_nbr[0] + nf0;
			f0.m_nbr[1] = f1.m_nbr[1] + nf0;
			f0.m_nbr[2] = f1.m_nbr[2] + nf0;
			if (f0.Nodes() == 4) f0.m_nbr[3] = f1.m_nbr[3] + nf0; else f0.m_nbr[3] = -1;
		}
	}

	// create the solid elements
	if (elems > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i=0; i<ne0; ++i)
		{
			FEElement& e = m_Elem[i];
			if (e.m_gid > ng) ng = e.m_gid;
		}
		++ng;

		m_Elem.resize(elems);
		m_data.Clear();
		for (i=0; i<ne1; ++i) 
		{
			FEElement& e0 = m_Elem[ne0 + i];
			FEElement& e1 = fem.m_Elem[i];
			e0 = e1;
			e0.m_gid = e1.m_gid + ng;

			for (j=0; j<6; ++j)
			{
				e0.m_nbr[j] = (e1.m_nbr[j] >= 0 ? e1.m_nbr[j] + ne0 : -1);
			}

			n = e1.Nodes();
			for (j=0; j<n; ++j) e0.m_node[j] = nn0 + e1.m_node[j];

			int nf = e1.Faces();
			for (j=0; j<nf; ++j) e0.m_face[j] = nf0 + e1.m_face[j];

			if (e1.IsShell())
			{
				e0.m_face[0] = nf0 + e1.m_face[0];
			}
		}
	}

	// create the edges
	if (edges > 0)
	{
		// find the largest GID
		int ng = -1;
		for (i=0; i<nl0; ++i)
		{
			FEEdge& e = m_Edge[i];
			if (e.m_gid > ng) ng = e.m_gid;
		}
		++ng;

		m_Edge.resize(edges);
		for (i=0; i<nl1; ++i)
		{
			FEEdge& l0 = m_Edge[nl0 + i];
			FEEdge& l1 = fem.m_Edge[i];

			l0 = l1;
			l0.m_gid = l1.m_gid + ng;

			int ne = l1.Nodes();
			for (int j=0; j<ne; ++j) l0.n[j] = nn0 + l1.n[j];

			l0.m_nbr[0] = (l1.m_nbr[0] >= 0 ? l1.m_nbr[0] + nl0 : -1);
			l0.m_nbr[1] = (l1.m_nbr[1] >= 0 ? l1.m_nbr[1] + nl0 : -1);
		}
	}

	// update the mesh
	UpdateNormals();
	UpdateBox();
}

//-----------------------------------------------------------------------------
void FEMesh::AddNode(const vec3d& r)
{
	// create a new node
	FENode node;
	node.r = r;
	node.SetExterior(true);	// we'll assume this node is not attached to anything

	AddNode(node);
}

//-----------------------------------------------------------------------------
// Attach another mesh to this mesh and weld the nodes
void FEMesh::AttachAndWeld(FEMesh& mesh, double tol)
{
	// get the number of nodes
	int nn0 = Nodes();
	int nn1 = mesh.Nodes();

	// get the number of elements
	int ne0 = Elements();
	int ne1 = mesh.Elements();

	// get the number of faces
	int nf0 = Faces();
	int nf1 = mesh.Faces();

	// get the number of edges
	int nl0 = Edges();
	int nl1 = mesh.Edges();

	// calculate new counts (before welding)
	int nodes = nn0 + nn1;
	int elems = ne0 + ne1;
	int faces = nf0 + nf1;
	int edges = nl0 + nl1;

	// attach the two parts
	Attach(mesh);

	// do the weld only if tolerance is positive
	if (tol <= 0.0) return;

	// Okay, do the weld. We only weld nodes from the outer surface of one mesh
	// to the outer surface of the other mesh

	// First, build the target node list
	for (int i=0; i<nn0; ++i) Node(i).m_ntag = 0;
	for (int i=0; i<nf0; ++i)
	{
		FEFace& face = Face(i);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j) Node(face.n[j]).m_ntag = 1;
	}
	int ntag = 0;
	for (int i=0; i<nn0; ++i) if (Node(i).m_ntag == 1) ntag++;
	vector<int> trg(ntag); ntag=0;
	for (int i=0; i<nn0; ++i) if (Node(i).m_ntag == 1) trg[ntag++] = i;

	// Next, build the source node list
	for (int i=nn0; i<nodes; ++i) Node(i).m_ntag = 0;
	for (int i=nf0; i<faces; ++i)
	{
		FEFace& face = Face(i);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j) Node(face.n[j]).m_ntag = 1;
	}
	ntag = 0;
	for (int i=nn0; i<nodes; ++i) if (Node(i).m_ntag == 1) ntag++;
	vector<int> src(ntag); ntag=0;
	for (int i=nn0; i<nodes; ++i)
		if (Node(i).m_ntag == 1) src[ntag++] = i;


	// create the nodal reorder list
	vector<int> order(nodes);
	for (int i=0; i<nodes; ++i) order[i] = i;

	// sqr distance treshold
	double tol2 = tol*tol;

	// loop over the selected nodes
	int nsrc = (int) src.size();
	int ntrg = (int) trg.size();
	for (int i=0; i<nsrc; ++i)
		for (int j=0; j<ntrg; ++j)
		{
			FENode& ni = Node(src[i]);
			FENode& nj = Node(trg[j]);

			// calculate (squared) distance between nodes
			vec3d& ri = ni.r;
			vec3d& rj = nj.r;

			int gi = ni.m_gid;
			int gj = nj.m_gid;

			double d2 = (ri - rj).SqrLength();
			if (d2 <= tol2)
			{
				// nodes coindice, so weld.
				// If one of the nodes has a gid, we don't want to loose it.
				if (gi >= 0) order[trg[j]] = src[i];
				else order[src[i]] = trg[j];
			}
		}

	// update element numbers
	for (int i=0; i<elems; ++i)
	{
		FEElement& el = Element(i);
		int ne = el.Nodes();
		int* en = el.m_node;

		// reassign node numbers
		for (int j=0; j<ne; ++j) en[j] = order[en[j]];
	}

	// update face numbers
	for (int i=0; i<faces; ++i)
	{
		FEFace& face = Face(i);
		int nf = face.Nodes();
		for (int j=0; j<nf; ++j) face.n[j] = order[face.n[j]];
	}

	// update edge numbers
	for (int i = 0; i<edges; ++i)
	{
		FEEdge& edge = Edge(i);
		int ne = edge.Nodes();
		for (int j = 0; j<ne; ++j) edge.n[j] = order[edge.n[j]];
	}

	// remove isolated vertices
	RemoveIsolatedNodes();

	// At this point, the welding is done but there may be several issues with the mesh
	// so, let's clean up
	RebuildMesh();
/*
	// update element and face neighbors
	UpdateElementNeighbors();
	UpdateFaces();


	// Remove all duplicate edges
	RemoveDuplicateEdges();
	AutoPartitionNodes();

	// Remove all duplicate faces
	RemoveDuplicateFaces();
*/
	// TODO: I can still have duplicate shells
	return;
}

//-----------------------------------------------------------------------------
void FEMesh::RemoveDuplicateEdges()
{
	// clear tags
	TagAllEdges(0);

	int ng = CountEdgePartitions();

	// loop over all edges
	int NE = Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge& ei = Edge(i);
		for (int j = i + 1; j<NE; ++j)
		{
			FEEdge& ej = Edge(j);
			if (ei == ej) 
			{
				if (ei.m_gid == -1)
				{
					ei.m_ntag = 1;
					ej.m_gid += ng;
				}
				else if (ej.m_gid == -1)
				{
					ej.m_ntag = 1;
					ei.m_gid += ng;
				}
				else
				{
					ei.m_ntag = 1;
					ej.m_gid = (ei.m_gid * ng + ej.m_gid);
				}
			}
		}
	}

	RemoveEdges(1);
	UpdateEdgeNeighbors();
	UpdateEdgePartitions();
}

//-----------------------------------------------------------------------------
void FEMesh::RemoveDuplicateFaces()
{
	// clear tags
	TagAllFaces(0);

	// loop over all faces
	int NF = Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& fi = Face(i);
		for (int j = i + 1; j<NF; ++j)
		{
			FEFace& fj = Face(j);
			if (fi == fj) 
			{
				fi.m_ntag = 1;

				// we set the element indices to 0 to avoid deleting these elements
				fi.m_elem[0].eid = fi.m_elem[1].eid = -1;
			}
		}
	}

	// delete tagged faces
	RemoveFaces(1);
	UpdateFaces();
	UpdateFacePartitions();
}

//-----------------------------------------------------------------------------
// select elements based on face selection
vector<int> FEMesh::GetElementsFromSelectedFaces()
{
	// tag elements for selection
	int ne0 = Elements();
	vector<bool> selem(ne0, false);

	int faces = Faces();

	// map faces to their element
	std::map<int, vector<int>> fel;
	std::map<int, vector<int>>::iterator it;
	for (int i = 0; i<faces; ++i)
	{
		FEFace& face = Face(i);
		// get element to which this face belongs
		int iel = face.m_elem[0].eid;
		// store faces that share this element
		fel[iel].push_back(i);
	}

	// mark all nodes on the selected faces
	for (int i = 0; i<Nodes(); ++i) Node(i).m_ntag = -1;
	for (int i = 0; i < faces; ++i)
	{
		FEFace& face = Face(i);
		for (int j = 0; j < face.Nodes(); ++j)
			Node(face.n[j]).m_ntag = 1;
	}

	// find all elements that share nodes with these faces
	// fne key = non-face element
	// fne mapped values = vector of entries into fdata faces
	std::map<int, vector<int>> fne;
	std::map<int, vector<int>>::iterator ie;
	for (int i = 0; i<Elements(); ++i) {
		FEElement& el = Element(i);
		vector<int> shared_nodes;
		shared_nodes.reserve(el.Nodes());
		for (int j = 0; j<el.Nodes(); ++j) {
			if (Node(el.m_node[j]).m_ntag == 1)
				shared_nodes.push_back(el.m_node[j]);
		}
		if (shared_nodes.size() > 0)
			fne[i] = shared_nodes;
	}

	vector<int> selection;

	for (it = fel.begin(); it != fel.end(); ++it) {
		if (it->second.size() == 1) {
			// only one face connected to this element
			int iel = (int)it->first;
			selem[iel] = true;
		}
		else if (it->second.size() == 2) {
			// two faces connected to this element
			FEFace& face0 = Face(it->second[0]);
			FEFace& face1 = Face(it->second[1]);
			// check if they share common nodes
			vector<int> cn;
			for (int i = 0; i<face0.Nodes(); ++i)
				for (int j = 0; j<face1.Nodes(); ++j)
					if (face0.n[i] == face1.n[j]) cn.push_back(face0.n[i]);
			// only allow two shared nodes
			if (cn.size() != 2) return selection;
			int iel = (int)it->first;
			selem[iel] = true;
		}
		else if (it->second.size() > 2)
			// more than two faces share same element
			return selection;
	}

	// add hex and penta elements that belong to internal corner edges
	// add tet elements that share one or two nodes with selected faces
	for (ie = fne.begin(); ie != fne.end(); ++ie) {
		// we have an internal corner
		int iel = (int)ie->first;
		selem[iel] = true;
	}

	// select all the tagged elements
	for (int i = 0; i<ne0; ++i)
		if (selem[i]) selection.push_back(i);

	return selection;
}

//-----------------------------------------------------------------------------
// Extract faces as a shell mesh
FEMesh* FEMesh::ExtractFaces(bool selectedOnly)
{
	// clear face tags
	TagAllFaces(0);

	// count selected faces
	int faces = 0;
	if (selectedOnly)
	{
		for (int i=0; i<Faces(); ++i) if (Face(i).IsSelected()) { Face(i).m_ntag = 1; ++faces; }
	}
	else
	{
		faces = Faces();
		for (int i=0; i<Faces(); ++i) Face(i).m_ntag = 1;
	}

	// tag nodes that need to be copied
	for (int i=0; i<Nodes(); ++i) Node(i).m_ntag = -1;
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& f = Face(i);
		if (f.m_ntag == 1)
		{
			int n = f.Nodes();
			for (int j=0; j<n; ++j) Node(f.n[j]).m_ntag = 1;
		}
	}

	// count nodes
	int nodes = 0;
	for (int i=0; i<Nodes(); ++i) 
	{
		FENode& node = Node(i);
		if (node.m_ntag == 1) 
		{
			node.m_ntag = nodes;
			++nodes;
		}
	}

	assert( (nodes>0) && (faces>0));

	// allocate new mesh
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, faces);

	// create the nodes
	FENode* pn = pm->NodePtr();
	for (int i=0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_ntag >= 0)
		{
			*pn = node;
			++pn;
		}
	}

	// create the elements
	int eid = 0;
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_ntag)
		{
			FEElement_* pe = pm->ElementPtr(eid++);

			int n = face.Nodes();
			switch (n)
			{
			case 3: pe->SetType(FE_TRI3 ); break;
			case 4: pe->SetType(FE_QUAD4); break;
			case 6: pe->SetType(FE_TRI6 ); break;
			case 7: pe->SetType(FE_TRI7 ); break;
			case 8: pe->SetType(FE_QUAD8); break;
			case 9: pe->SetType(FE_QUAD9); break;
			default:
				assert(false);
				delete pm;
				return 0;
			}

			for (int j=0; j<n; ++j) pe->m_node[j] = Node(face.n[j]).m_ntag;
		}
	}

	// rebuild the mesh
	pm->RebuildMesh();

	return pm;
}

//-----------------------------------------------------------------------------
// Save mesh data to archive
//
void FEMesh::Save(OArchive &ar)
{
	int nodes = Nodes();
	int elems = Elements();
	int faces = Faces();
	int edges = Edges();

	// write the header
	ar.BeginChunk(CID_MESH_HEADER);
	{
		ar.WriteChunk(CID_MESH_NODES, nodes);
		ar.WriteChunk(CID_MESH_ELEMENTS, elems);
		ar.WriteChunk(CID_MESH_FACES, faces);
		ar.WriteChunk(CID_MESH_EDGES, edges);
	}
	ar.EndChunk();

	// write the nodes
	ar.BeginChunk(CID_MESH_NODE_SECTION);
	{
		FENode* pn = NodePtr();
		for (int i=0; i<nodes; ++i, ++pn)
		{
			ar.BeginChunk(CID_MESH_NODE);
			{
				ar.WriteChunk(CID_MESH_NODE_GID, pn->m_gid);
				ar.WriteChunk(CID_MESH_NODE_POSITION, pn->r);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// write the elements
	ar.BeginChunk(CID_MESH_ELEMENT_SECTION);
	{
		for (int i=0; i<elems; ++i)
		{
			FEElement_* pe = ElementPtr(i);

			ar.BeginChunk(CID_MESH_ELEMENT);
			{
				int ntype = pe->Type();
				ar.WriteChunk(CID_MESH_ELEMENT_TYPE    , ntype);
				ar.WriteChunk(CID_MESH_ELEMENT_GID     , pe->m_gid);
				ar.WriteChunk(CID_MESH_ELEMENT_NODES   , pe->m_node, pe->Nodes());
				ar.WriteChunk(CID_MESH_ELEMENT_FIBER   , pe->m_fiber);
				ar.WriteChunk(CID_MESH_ELEMENT_Q_ACTIVE, pe->m_Qactive);
				ar.WriteChunk(CID_MESH_ELEMENT_Q       , pe->m_Q);
				if (pe->IsShell())
					ar.WriteChunk(CID_MESH_SHELL_THICKNESS , pe->m_h, pe->Nodes());
//				ar.WriteChunk(CID_MESH_ELEMENT_MATERIAL, mid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// write the faces
	ar.BeginChunk(CID_MESH_FACE_SECTION);
	{
		FEFace* pf = FacePtr();
		for (int i=0; i<faces; ++i, ++pf)
		{
			ar.BeginChunk(CID_MESH_FACE);
			{
				int nn = pf->Nodes();
				int ntype = pf->Type();
				assert(ntype != FE_FACE_INVALID_TYPE);
				ar.WriteChunk(CID_MESH_FACE_TYPE    , ntype);
				ar.WriteChunk(CID_MESH_FACE_GID     , pf->m_gid);
				ar.WriteChunk(CID_MESH_FACE_NODES   , pf->n, pf->Nodes());
				ar.WriteChunk(CID_MESH_FACE_SMOOTHID, pf->m_sid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// write the edges
	ar.BeginChunk(CID_MESH_EDGE_SECTION);
	{
		FEEdge* pe = EdgePtr();
		for (int i=0; i<edges; ++i, ++pe)
		{
			int nn = pe->Nodes();
			int ntype = pe->Type();
			ar.BeginChunk(CID_MESH_EDGE);
			{
				ar.WriteChunk(CID_MESH_EDGE_TYPE, ntype);
				ar.WriteChunk(CID_MESH_EDGE_GID, pe->m_gid);
				ar.WriteChunk(CID_MESH_EDGE_NODES, pe->n, nn);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// TODO: Move this stuff to the GObject serialization
	GObject* po = GetGObject();
	int parts = po->FEParts();
	int surfs = po->FESurfaces();
	int nsets = po->FENodeSets();

	// write the parts
	if (parts > 0)
	{
		ar.BeginChunk(CID_MESH_PART_SECTION);
		{
			for (int i=0; i<parts; ++i)
			{
				// get the boundary condition
				FEPart* pg = po->GetFEPart(i);

				// store the group data
				ar.BeginChunk(CID_MESH_PART);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the surfaces
	if (surfs > 0)
	{
		ar.BeginChunk(CID_MESH_SURF_SECTION);
		{
			for (int i=0; i<surfs; ++i)
			{
				// get the boundary condition
				FESurface* pg = po->GetFESurface(i);

				// store the group data
				ar.BeginChunk(CID_MESH_SURFACE);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the parts
	if (nsets > 0)
	{
		ar.BeginChunk(CID_MESH_NSET_SECTION);
		{
			for (int i=0; i<nsets; ++i)
			{
				// get the boundary condition
				FENodeSet* pg = po->GetFENodeSet(i);

				// store the group data
				ar.BeginChunk(CID_MESH_NODESET);
				{
					pg->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// write the mesh data 
	if (MeshDataFields() > 0)
	{
		ar.BeginChunk(CID_MESH_DATA_SECTION);
		{
			// node data
			for (int n = 0; n<(int)m_meshData.size(); ++n)
			{
				FEMeshData* meshData = m_meshData[n];
				switch (meshData->GetDataClass())
				{
				case FEMeshData::NODE_DATA:
				{
					FENodeData* map = dynamic_cast<FENodeData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_NODE_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::SURFACE_DATA:
				{
					FESurfaceData* map = dynamic_cast<FESurfaceData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_SURFACE_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::ELEMENT_DATA:
				{
					FEElementData* map = dynamic_cast<FEElementData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_ELEM_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				case FEMeshData::PART_DATA:
				{
					FEPartData* map = dynamic_cast<FEPartData*>(meshData); assert(map);
					ar.BeginChunk(CID_MESH_PART_DATA);
					{
						map->Save(ar);
					}
					ar.EndChunk();
				}
				break;
				}
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load mesh data from archive
//
void FEMesh::Load(IArchive& ar)
{
	TRACE("FEMesh::Load");

	int nodes;
	int elems;
	int faces;
	int edges;

	GObject* po = GetGObject();
	vec3d pos;
	quatd rot;

	// the first chunk must be the header
	ar.OpenChunk();
	assert(ar.GetChunkID() == CID_MESH_HEADER);
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch(nid)
		{
		case CID_MESH_NODES: ar.read(nodes); break;
		case CID_MESH_ELEMENTS: ar.read(elems); break;
		case CID_MESH_FACES: ar.read(faces); break;
		case CID_MESH_EDGES: ar.read(edges); break;
		}
		ar.CloseChunk();
	}
	ar.CloseChunk();

	// allocate storage
	Create(nodes, elems,faces, edges);

	// buffer for storing shell thickness
	// NOTE: Problem is that in older versions pe->Nodes() values 
	// were stored. But the max buffer size for shell thickness is 9
	// so this could crash PreView when reading shell thickness values for elements
	// that have more than 9 nodes.
	vector<double> h(FEElement::MAX_NODES);

	// read the rest of the mesh data
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch(nid)
		{
		case CID_MESH_NODE_SECTION:
			{
				int n = 0;
				FENode* pn = NodePtr();
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					if (nid == CID_MESH_NODE)
					{
						assert(n < nodes);
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();
							switch (nid)
							{
							case CID_MESH_NODE_GID: ar.read(pn->m_gid); break;
							case CID_MESH_NODE_POSITION: ar.read(pn->r); break;
							}
							ar.CloseChunk();
						}
						++pn;
						++n;
					}
					ar.CloseChunk();
				}
			}
			break;
		case CID_MESH_ELEMENT_SECTION:
			{
				int n = 0;
				FEElement* pe = &m_Elem[0];
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					if (nid == CID_MESH_ELEMENT)
					{
						assert(n < elems);
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();
							switch (nid)
							{
							case CID_MESH_ELEMENT_TYPE: { int ntype; ar.read(ntype); pe->SetType(ntype); }; break;
							case CID_MESH_ELEMENT_GID: ar.read(pe->m_gid); break;
							case CID_MESH_ELEMENT_NODES: 
								{
									if (ar.Version() < 0x00010008) ar.read(pe->m_node, 8);
									else ar.read(pe->m_node, pe->Nodes());
								}
								break;
							case CID_MESH_ELEMENT_FIBER   : ar.read(pe->m_fiber); break;
							case CID_MESH_ELEMENT_Q_ACTIVE: ar.read(pe->m_Qactive); break;
							case CID_MESH_ELEMENT_Q       :  ar.read(pe->m_Q); break;

							case CID_MESH_SHELL_THICKNESS:
								{
									ar.read(&h[0], pe->Nodes());
									int n = pe->Nodes();
									if (n > 9) n = 9;
									for (int i=0; i<n; ++i) pe->m_h[i] = h[i];
								}
								break;
//							case CID_MESH_ELEMENT_MATERIAL: ar.read(pe->m_matid); break;
							}
							ar.CloseChunk();
						}
						++n;
						++pe;
					}
					else assert(false);
					ar.CloseChunk();
				}
			}
			break;
		case CID_MESH_FACE_SECTION:
			{
				int n = 0;
				FEFace* pf = FacePtr();
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					if (nid != CID_MESH_FACE) throw ReadError("error parsing CID_MESH_FACE_SECTION (FEMesh::Load)");

					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid = ar.GetChunkID();
						int ntype;
						switch (nid)
						{
						case CID_MESH_FACE_TYPE: 
							{
								ar.read(ntype); 
								if (ar.Version() < 0x00020000)
								{
									switch (ntype)
									{
									case FE_TRI3 : pf->SetType(FE_FACE_TRI3 ); break;
									case FE_QUAD4: pf->SetType(FE_FACE_QUAD4); break;
									case FE_TRI6 : pf->SetType(FE_FACE_TRI6 ); break;
									case FE_QUAD8: pf->SetType(FE_FACE_QUAD8); break;
									case FE_TRI7 : pf->SetType(FE_FACE_TRI7 ); break;
									case FE_QUAD9: pf->SetType(FE_FACE_QUAD9); break;
									case FE_TRI10: pf->SetType(FE_FACE_TRI10); break;
									default:
										assert(false);
									}
								}
								else
								{
									switch (ntype)
									{
									case FE_FACE_TRI3 : pf->SetType(FE_FACE_TRI3 ); break;
									case FE_FACE_QUAD4: pf->SetType(FE_FACE_QUAD4); break;
									case FE_FACE_TRI6 : pf->SetType(FE_FACE_TRI6 ); break;
									case FE_FACE_QUAD8: pf->SetType(FE_FACE_QUAD8); break;
									case FE_FACE_TRI7 : pf->SetType(FE_FACE_TRI7 ); break;
									case FE_FACE_QUAD9: pf->SetType(FE_FACE_QUAD9); break;
									case FE_FACE_TRI10: pf->SetType(FE_FACE_TRI10); break;
									default:
										assert(false);
									}
								}
							}
							break;
						case CID_MESH_FACE_GID     : ar.read(pf->m_gid); break;
						case CID_MESH_FACE_NODES   : ar.read(pf->n, pf->Nodes()); break;
						case CID_MESH_FACE_SMOOTHID: ar.read(pf->m_sid); break;
						}
						ar.CloseChunk();
					}

					assert(n < faces);
					++pf;
					++n;

					ar.CloseChunk();
				}
			}
			break;
		case CID_MESH_EDGE_SECTION:
			{
				int n = 0;
				FEEdge* pe = EdgePtr();
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					if (nid != CID_MESH_EDGE) throw ReadError("error parsing CID_MESH_EDGE_SECTION (FEMesh::Load)");

					int ntype;
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid = ar.GetChunkID();
						switch (nid)
						{
						case CID_MESH_EDGE_TYPE: 
						{ 
							ar.read(ntype); 
							if (ar.Version() < 0x00020000)
							{
								switch (ntype)
								{
								case FE_BEAM2: pe->SetType(FE_EDGE2);  break;
								case FE_BEAM3: pe->SetType(FE_EDGE3);  break;
								default:
									assert(false);
									throw ReadError("error parsing CID_MESH_EDGE_SECTION (FEMesh::Load)");
								}
							}
							else
							{
								switch (ntype)
								{
								case FE_EDGE2: pe->SetType(FE_EDGE2);  break;
								case FE_EDGE3: pe->SetType(FE_EDGE3);  break;
								case FE_EDGE4: pe->SetType(FE_EDGE4);  break;
								default:
									assert(false);
									throw ReadError("error parsing CID_MESH_EDGE_SECTION (FEMesh::Load)");
								}
							}
						} 
						break;
						case CID_MESH_EDGE_GID: ar.read(pe->m_gid); break;
						case CID_MESH_EDGE_NODES: 
							{
								int nn = pe->Nodes();
								assert(nn > 0);
								if (nn <= 0) throw ReadError("error parsing CID_MESH_EDGE_SECTION (FEMesh::Load)");
								ar.read(pe->n, nn); break;
							}
						}
						ar.CloseChunk();
					}
					
					assert(n < edges);
					++pe;
					++n;

					ar.CloseChunk();
				}
			}
			break;
		case CID_MESH_PART_SECTION:
			{
				// TODO: move to GObject serialization
				FEPart* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_PART);
					pg = new FEPart(po);
					pg->Load(ar);
					po->AddFEPart(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_SURF_SECTION:
			{
				// TODO: move to GObject serialization
				FESurface* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_SURFACE);
					pg = new FESurface(po);
					pg->Load(ar);
					po->AddFESurface(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_NSET_SECTION:
			{
				// TODO: move to GObject serialization
				FENodeSet* pg = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					pg = 0;
					assert(ar.GetChunkID() == CID_MESH_NODESET);
					pg = new FENodeSet(po);
					pg->Load(ar);
					po->AddFENodeSet(pg);

					ar.CloseChunk();
				}			
			}
			break;
		case CID_MESH_DATA_SECTION:
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					switch (nid)
					{
					case CID_MESH_NODE_DATA:
						{
							FENodeData* pmap = AddNodeDataField("(unnamed)");
							int NN = Nodes();
							pmap->Create(NN);
							pmap->Load(ar);
						}
						break;
					case CID_MESH_SURFACE_DATA:
						{
							FESurfaceData* pmap = new FESurfaceData(this);
//							FESurfaceData* pmap = AddSurfaceDataField("(unnamed)", nullptr, FEMeshData::DATA_TYPE::DATA_SCALAR);
//							int NF = Faces();
//							pmap->Create(this, NF);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					case CID_MESH_ELEM_DATA:
						{
							FEElementData* pmap = new FEElementData(this);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					case CID_MESH_PART_DATA:
						{
							FEPartData* pmap = new FEPartData(this);
							pmap->Load(ar);
							m_meshData.push_back(pmap);
						}
						break;
					}

					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}

	// rebuild mesh' data
	Update();
}

//-----------------------------------------------------------------------------
// Partition the selected faces
//
void FEMesh::PartitionFaceSelection()
{
	int i;
	int N = Faces();

	int nsg = 0;

	// tag all selected faces
	for (i=0; i<N; ++i)
	{
		FEFace& f = Face(i);
		if (f.IsSelected()) 
		{
			f.m_ntag = 1; 
			f.m_gid = -1;
		}
		else 
		{
			f.m_ntag = 0;
			if (f.m_gid > nsg) nsg = f.m_gid;
		}
	}

	// reassign smoothing groups
	++nsg;
	for (i=0; i<N; ++i)
	{
		FEFace& f = Face(i);
		if (f.m_ntag == 1) f.m_gid = nsg;
	}

	UpdateNormals();

	// rebuild the edges
	BuildEdges();

	// partition the edges
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();

	// update the nodes
	MarkExteriorNodes();
}

//-----------------------------------------------------------------------------
// Partition the selected edges
//
void FEMesh::PartitionEdgeSelection()
{
	int i;
	int N = Edges();

	int nsg = 0;

	// tag all selected edges
	for (i=0; i<N; ++i)
	{
		FEEdge& e = Edge(i);
		if (e.IsSelected()) 
		{
			e.m_ntag = 1; 
		}
		else 
		{
			e.m_ntag = 0;
			if (e.m_gid > nsg) nsg = e.m_gid;
		}
	}

	// table of new indices
	++nsg;
	vector<int> T(nsg, -1);

	// reassign smoothing groups
	for (i=0; i<N; ++i)
	{
		FEEdge& e = Edge(i);
		if (e.m_ntag == 1)
		{
			if (T[e.m_gid] == -1) T[e.m_gid] = nsg++;
			e.m_gid = T[e.m_gid];
		}
	}

	// recalculate edge neighbors
	UpdateEdgeNeighbors();

	// partition the nodes
	AutoPartitionNodes();

	// update the nodes
	MarkExteriorNodes();
}

//-----------------------------------------------------------------------------
// Create a shallow-copy of the mesh
void FEMesh::ShallowCopy(FEMesh* pm)
{
	m_Node = pm->m_Node;
	m_Edge = pm->m_Edge;
	m_Face = pm->m_Face;
	m_Elem = pm->m_Elem;

	m_data = pm->m_data;

	m_box = pm->m_box;
}

//-----------------------------------------------------------------------------
// Invert selected elements (or all if none are selected)
void FEMesh::InvertSelectedElements()
{
	// tag all selected elements
	int nsel = 0;
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& e = Element(i);
		if (e.IsSelected()) { e.m_ntag = 1; nsel++; }
		else e.m_ntag = 0;
	}
	if (nsel == 0) TagAllElements(1);

	// invert the tagged elements
	InvertTaggedElements(1);
}

//-----------------------------------------------------------------------------
void FEMesh::InvertSelectedFaces()
{
	// tag all selected faces
	int nsel = 0;
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f = Face(i);
		if (f.IsSelected()) { f.m_ntag = 1; nsel++; }
		else f.m_ntag = 0;
	}
	if (nsel == 0) TagAllFaces(1);

	// invert the tagged elements
	InvertTaggedFaces(1);
}

//-----------------------------------------------------------------------------
// Invert selected faces
void FEMesh::InvertTaggedFaces(int ntag)
{
	// invert tagged elements
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f = Face(i);
		if (f.m_ntag == ntag)
		{
			int n = f.Nodes(), m;

			switch (f.Type())
			{
			case FE_FACE_QUAD4:
			case FE_FACE_TRI3:
			{
				m = f.n[0]; f.n[0] = f.n[2]; f.n[2] = m;
			}
			break;
			default:
				assert(false);
			}
		}
	}

	UpdateElementNeighbors();
	UpdateFaces();
	UpdateNormals();
}

//-----------------------------------------------------------------------------
// Invert selected elements.
void FEMesh::InvertTaggedElements(int ntag)
{
	// invert tagged elements
	for (int i=0; i<Elements(); ++i)
	{
		FEElement& e = Element(i);
		if (e.m_ntag == ntag)
		{
			int n = e.Nodes(), m;

			switch (e.Type())
			{
			case FE_QUAD4:
			case FE_TRI3:
				{
					for (int j=0; j<n/2; ++j) 
					{
						m = e.m_node[j];
						e.m_node[j] = e.m_node[n-j-1];
						e.m_node[n-j-1] = m;
					}
				}
				break;
			case FE_TET4:
			case FE_TET5:
				{
					m = e.m_node[0]; e.m_node[0] = e.m_node[3]; e.m_node[3] = m;
				}
				break;
            case FE_TET10:
                {
                    m = e.m_node[1]; e.m_node[1] = e.m_node[2]; e.m_node[2] = m;
                    m = e.m_node[4]; e.m_node[4] = e.m_node[6]; e.m_node[6] = m;
                    m = e.m_node[8]; e.m_node[8] = e.m_node[9]; e.m_node[9] = m;
                }
                break;
			case FE_PENTA6:
				{
					m = e.m_node[0]; e.m_node[0] = e.m_node[2]; e.m_node[2] = m;
					m = e.m_node[3]; e.m_node[3] = e.m_node[5]; e.m_node[5] = m;
				}
				break;
            case FE_PENTA15:
                {
                    m = e.m_node[1]; e.m_node[1] = e.m_node[2]; e.m_node[2] = m;
                    m = e.m_node[4]; e.m_node[4] = e.m_node[5]; e.m_node[5] = m;
                    m = e.m_node[6]; e.m_node[6] = e.m_node[8]; e.m_node[8] = m;
                    m = e.m_node[9]; e.m_node[9] = e.m_node[11]; e.m_node[11] = m;
                    m = e.m_node[13]; e.m_node[13] = e.m_node[14]; e.m_node[14] = m;
                }
                    break;
			case FE_HEX8:
				{
					m = e.m_node[0]; e.m_node[0] = e.m_node[4]; e.m_node[4] = m;
					m = e.m_node[1]; e.m_node[1] = e.m_node[5]; e.m_node[5] = m;
					m = e.m_node[2]; e.m_node[2] = e.m_node[6]; e.m_node[6] = m;
					m = e.m_node[3]; e.m_node[3] = e.m_node[7]; e.m_node[7] = m;
				}
				break;
            case FE_HEX20:
                {
                    m = e.m_node[1]; e.m_node[1] = e.m_node[3]; e.m_node[3] = m;
                    m = e.m_node[5]; e.m_node[5] = e.m_node[7]; e.m_node[7] = m;
                    m = e.m_node[8]; e.m_node[8] = e.m_node[11]; e.m_node[11] = m;
                    m = e.m_node[9]; e.m_node[9] = e.m_node[10]; e.m_node[10] = m;
                    m = e.m_node[12]; e.m_node[12] = e.m_node[15]; e.m_node[15] = m;
                    m = e.m_node[13]; e.m_node[13] = e.m_node[14]; e.m_node[14] = m;
                    m = e.m_node[17]; e.m_node[17] = e.m_node[19]; e.m_node[19] = m;
                }
                    break;
            case FE_PYRA5:
                {
                    m = e.m_node[1]; e.m_node[1] = e.m_node[3]; e.m_node[3] = m;
                }
                    break;
			default:
				assert(false);
			}
		}
	}

	// mirror the faces
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& f = Face(i);
		FEElement_* pe = ElementPtr(f.m_elem[0].eid);

		if (pe->m_ntag == ntag)
		{
			FEFace g;
			FindFace(pe, f, g);

            for (int j=0; j<FEElement::MAX_NODES; ++j)
                f.n[j] = g.n[j];
		}
	}

	UpdateElementNeighbors();
	UpdateFaces();
	UpdateNormals();
	UpdateBox();
}

//-----------------------------------------------------------------------------
FEElementData* FEMesh::AddElementDataField(const string& sz, FEPart* part, FEMeshData::DATA_TYPE dataType)
{
	FEElementData* map = new FEElementData;
	map->Create(this, part, dataType);
	map->SetName(sz);
	m_meshData.push_back(map);
	return map;
}

//-----------------------------------------------------------------------------
int FEMesh::MeshDataFields() const { return (int)m_meshData.size(); }

//-----------------------------------------------------------------------------
FEMeshData* FEMesh::GetMeshDataField(int i) { return m_meshData[i]; }

//-----------------------------------------------------------------------------
Mesh_Data& FEMesh::GetMeshData() { return m_data; }

//-----------------------------------------------------------------------------
FEMeshData* FEMesh::FindMeshDataField(const string& sz)
{
	if (m_meshData.empty()) return 0;
	for (int i = 0; i<m_meshData.size(); ++i)
	{
		const string& name = m_meshData[i]->GetName();
		if (name == sz) return m_meshData[i];
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEMesh::RemoveMeshDataField(int i)
{
	m_meshData.erase(m_meshData.begin() + i);
}

//-----------------------------------------------------------------------------
int FEMesh::GetMeshDataIndex(FEMeshData* data)
{
	for (int i = 0; i < m_meshData.size(); ++i)
		if (m_meshData[i] == data) return i;
	return -1;
}

//-----------------------------------------------------------------------------
void FEMesh::InsertMeshData(int i, FEMeshData* data)
{
	m_meshData.insert(m_meshData.begin() + i, data);
}

//-----------------------------------------------------------------------------
void FEMesh::AddMeshDataField(FEMeshData* data)
{
	assert(data);
	if (data) m_meshData.push_back(data);
}

//-----------------------------------------------------------------------------
FENodeData* FEMesh::AddNodeDataField(const string& sz, double v)
{
	FENodeData* data = new FENodeData(GetGObject());
	data->Create(v);
	data->SetName(sz);
	m_meshData.push_back(data);
	return data;
}

//-----------------------------------------------------------------------------
FESurfaceData* FEMesh::AddSurfaceDataField(const string& name, FESurface* surface, FEMeshData::DATA_TYPE dataType)
{
	FESurfaceData* data = new FESurfaceData;
	data->Create(this, surface, dataType);
	data->SetName(name);
	m_meshData.push_back(data);
	return data;
}

//-----------------------------------------------------------------------------
FEMesh* ConvertSurfaceToMesh(FESurfaceMesh* surfaceMesh)
{
	int nodes = surfaceMesh->Nodes();
	int faces = surfaceMesh->Faces();

	FEMesh* mesh = new FEMesh;
	mesh->Create(nodes, faces);

	for (int i = 0; i<nodes; ++i)
	{
		FENode& surfNode = surfaceMesh->Node(i);
		FENode& meshNode = mesh->Node(i);

		meshNode = surfNode;
	}

	for (int i = 0; i<faces; ++i)
	{
		FEFace& face = surfaceMesh->Face(i);
		FEElement& el = mesh->Element(i);

		el.m_gid = face.m_gid;

		int nf = face.Nodes();
		switch (face.Nodes())
		{
		case 3: el.SetType(FE_TRI3); break;
		case 4: el.SetType(FE_QUAD4); break;
		default:
			assert(false);
			delete mesh;
			return 0;
		}

		for (int j = 0; j<nf; ++j) el.m_node[j] = face.n[j];
	}

	mesh->RebuildMesh();

	return mesh;
}

//-----------------------------------------------------------------------------
void FEMesh::BuildFaces()
{
	// let's count them first
	int faces = 0;
	int elems = Elements();
	for (int i = 0; i<elems; i++)
	{
		FEElement& el = Element(i);

		// solid elements
		// we create a face if an element does not have a neighbor
		// or if the neighbor has a different gid and is not a shell.
		// Note that we need to make sure we don't double-count.
		int n = el.Faces();
		for (int j = 0; j<n; ++j)
		{
			if (el.m_nbr[j] == -1) ++faces;
			else
			{
				FEElement_* pen = ElementPtr(el.m_nbr[j]);
				if (el.m_gid < pen->m_gid)
				{
					++faces;
				}
			}
		}

		// shell elements always add a face
		if (el.IsShell())
		{
			++faces;
		}
	}

	// make sure we have faces
	if (faces == 0) 
	{
		m_Face.clear();
		return;
	}

	// allocate storage
	Create(0, 0, faces);

	// create the faces
	FEFace* pf = FacePtr();
	int nf = 0;

	// solid elements
	for (int i = 0; i<elems; i++)
	{
		FEElement& el = Element(i);

		int n = el.Faces();
		for (int j = 0; j<n; j++)
		{
			el.m_face[j] = -1;
			int nbid = el.m_nbr[j];
			FEElement_* pen = (nbid == -1 ? 0 : ElementPtr(nbid));
			if (pen == 0)
			{
				el.GetFace(j, *pf);
				++pf; ++nf;
			}
			else if (el.m_gid < pen->m_gid)
			{
				*pf = el.GetFace(j);
				++pf; ++nf;
			}
		}
	}

	// shell elements
	for (int i = 0; i<elems; i++)
	{
		FEElement& el = Element(i);

		// shell elements
		int n = el.Edges();
		if (n>0)
		{
			el.GetShellFace(*pf);
			++pf; ++nf;
		}
	}

	// update faces
	UpdateFaces();
}

//-----------------------------------------------------------------------------
void FEMesh::BuildEdges()
{
	// let's create a set of edges 
	vector<FEEdge> edgeSet;

	// assign edges
	int n[FEEdge::MAX_NODES];
	int NF = Faces();
	int NN = Nodes();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& f = Face(i);
		if (f.IsExternal())
		{
			f.m_ntag = 1;
			int ne = f.Edges();
			for (int j = 0; j<ne; ++j)
			{
				FEFace* pfn = FacePtr(f.m_nbr[j]);
				if ((pfn == 0) || (f.m_gid < pfn->m_gid))
				{
					int en = f.GetEdgeNodes(j, n);
					FEEdge e;
					switch (en)
					{
					case 2: e.SetType(FE_EDGE2); break;
					case 3: e.SetType(FE_EDGE3); break;
					case 4: e.SetType(FE_EDGE4); break;
					default:
						assert(false);
					}

					e.n[0] = n[0]; assert((n[0] >= 0) && (n[0]<NN));
					e.n[1] = n[1]; assert((n[1] >= 0) && (n[1]<NN));
					e.n[2] = n[2];
					e.n[3] = n[3];
					e.m_gid = -1;

					// try to insert it
					bool bexist = false;
					for (int k=0; k<(int)edgeSet.size(); ++k)
					{
						if (edgeSet[k] == e)
						{
							bexist = true;
							break;
						}
					}

					if (bexist == false) edgeSet.push_back(e);					
				}
			}
		}
	}

	// delete current edges
	DeleteEdges();

	// allocate edges
	int nedges = (int) edgeSet.size();
	if (nedges == 0) return;
	Create(0, 0, 0, nedges);

	nedges = 0;
	for (auto e:edgeSet)
	{
		FEEdge& edge = Edge(nedges++);
		edge = e;
	}

	UpdateEdgeNeighbors();
}

//-----------------------------------------------------------------------------
// This functions evaluates the FEFace::m_elem members and FEElement::m_face members
void FEMesh::UpdateFaces()
{
	// update the face-element connectivity data
	UpdateFaceElementTable();

	// update face neighbours
	UpdateFaceNeighbors();
}

//-----------------------------------------------------------------------------
// This function builds the surface, edges and node of the mesh
void FEMesh::RebuildMesh(double smoothingAngle, bool autoSurface, bool partitionMesh)
{
	// update the element neighbours
	UpdateElementNeighbors();

	// partition the elements based on element connectivity
	if (partitionMesh)
	{
		AutoPartitionElements();
	}

	// build the faces
	BuildFaces();

	// calculate auto GID's
	AutoSmooth(smoothingAngle);

	// partition the surface
	if (autoSurface) AutoPartitionSurface();
	else AutoPartitionSurfaceQuick();

	// build the edges
	BuildEdges();

	// partition the edges
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();

	// update the mesh
	MarkExteriorNodes();
	UpdateBox();
}

//-----------------------------------------------------------------------------
void FEMesh::AutoPartition(double smoothingAngle)
{
	// calculate auto GID's
	AutoSmooth(smoothingAngle);

	// partition the surface
	AutoPartitionSurface();

	// build the edges
	BuildEdges();

	// partition the edges
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();
}

//-----------------------------------------------------------------------------
// This partitions elements based on connectivity
void FEMesh::AutoPartitionElements()
{
	int NE = Elements();
	for (int i=0; i<NE; ++i) Element(i).m_gid = -1;
	TagAllElements(0);

	int i0 = 0;
	int np = 0;
	while (true)
	{
		// find an unprocessed element
		int n = -1;
		for (int i=i0; i<NE; ++i)
		{
			FEElement& el = Element(i);
			if (el.m_gid == -1)
			{
				i0 = i + 1;
				n = i;
				break;
			}
		}
		if (n == -1) break;

		// find all connected elements
		stack<int> s;
		s.push(n);
		while (s.empty() == false)
		{
			n = s.top(); s.pop();

			FEElement& el = Element(n);
			el.m_ntag = 1;
			el.m_gid = np;

			// solid elements
			int nf = el.Faces();
			for (int i=0; i<nf; ++i)
			{
				FEElement* pj = (el.m_nbr[i] == -1 ? 0 : &Element(el.m_nbr[i]));
				if (pj && (pj->m_ntag == 0))
				{
					pj->m_ntag = 1;
					s.push(el.m_nbr[i]);
				}
			}

			// shells
			int ne = el.Edges();
			for (int i=0; i<ne; ++i)
			{
				FEElement* pj = (el.m_nbr[i] == -1 ? 0 : &Element(el.m_nbr[i]));
				if (pj && (pj->m_ntag == 0))
				{
					pj->m_ntag = 1;
					s.push(el.m_nbr[i]);
				}
			}
		}
		++np;
	}

#ifdef _DEBUG
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = Element(i);
		assert(el.m_gid >= 0);
	}
#endif
}

//-----------------------------------------------------------------------------
void FEMesh::AutoPartitionSurface()
{
	// Get the mesh and number of faces
	int NF = Faces();

	// face that still require processing 
	// will be placed on a stack
	// The partitioning is done when the stack is empty
	vector<FEFace*> stack(NF);
	int ns = 0;

	// reset face ID's 
	for (int i = 0; i<NF; ++i) Face(i).m_gid = -1;

	// let's get to work
	int ngid = 0;
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);
		if (pf->m_gid == -1)
		{
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// mark as processed
				pf->m_gid = ngid;

				// get the element part ID's
				assert(pf->m_elem[0].eid >= 0);
				FEElement_* pe11 = ElementPtr(pf->m_elem[0].eid);
				FEElement_* pe12 = ElementPtr(pf->m_elem[1].eid);
				int gid11 = (pe11 ? pe11->m_gid : -1);
				int gid12 = (pe12 ? pe12->m_gid : -1);

				int n = pf->Edges();
				for (int j = 0; j<n; ++j)
				{
					FEFace* pf2 = FacePtr(pf->m_nbr[j]);
					if (pf2)
					{
						assert(pf2->m_elem[0].eid >= 0);
						FEElement_* pe21 = ElementPtr(pf2->m_elem[0].eid);
						FEElement_* pe22 = ElementPtr(pf2->m_elem[1].eid);

						int gid21 = (pe21 ? pe21->m_gid : -1);
						int gid22 = (pe22 ? pe22->m_gid : -1);

						if ((pf2->m_gid == -1) && (pf->m_sid == pf2->m_sid) && (gid11 == gid21) && (gid12 == gid22))
						{
							pf2->m_gid = -2;
							stack[ns++] = pf2;
						}
					}
				}
			}
			++ngid;
		}
	}
}

//-----------------------------------------------------------------------------
void FEMesh::AutoPartitionSurfaceQuick()
{
	// Get the mesh and number of faces
	int NF = Faces();

	// face that still require processing 
	// will be placed on a stack
	// The partitioning is done when the stack is empty
	vector<FEFace*> stack(NF);
	int ns = 0;

	// reset face ID's 
	for (int i = 0; i<NF; ++i) Face(i).m_gid = -1;

	// let's get to work
	int ngid = 0;
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);
		if (pf->m_gid == -1)
		{
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// mark as processed
				pf->m_gid = ngid;

				// get the element part ID's
				assert(pf->m_elem[0].eid >= 0);
				FEElement_* pe11 = ElementPtr(pf->m_elem[0].eid);
				FEElement_* pe12 = ElementPtr(pf->m_elem[1].eid);
				int gid11 = (pe11 ? pe11->m_gid : -1);
				int gid12 = (pe12 ? pe12->m_gid : -1);

				int n = pf->Edges();
				for (int j = 0; j<n; ++j)
				{
					FEFace* pf2 = FacePtr(pf->m_nbr[j]);
					if (pf2)
					{
						assert(pf2->m_elem[0].eid >= 0);
						FEElement_* pe21 = ElementPtr(pf2->m_elem[0].eid);
						FEElement_* pe22 = ElementPtr(pf2->m_elem[1].eid);

						int gid21 = (pe21 ? pe21->m_gid : -1);
						int gid22 = (pe22 ? pe22->m_gid : -1);

						if ((pf2->m_gid == -1) && (gid11 == gid21) && (gid12 == gid22))
						{
							pf2->m_gid = -2;
							stack[ns++] = pf2;
						}
					}
				}
			}
			++ngid;
		}
	}
}

//-----------------------------------------------------------------------------
void FEMesh::AutoPartitionEdges()
{
	// get the number of edges
	int NE = Edges();

	// get the edge pointer
	FEEdge* pe = EdgePtr();
	UpdateEdgeNeighbors();

	// intialize all group ID's
	int i, j;
	for (i = 0; i<NE; ++i, ++pe) pe->m_gid = -1;

	// build a stack
	stack<FEEdge*> se;

	int ue = 0;
	int ng = 0;
	vec3d n1, n2;
	while (true)
	{
		// find an unassigned edge
		if (se.empty())
		{
			pe = 0;
			for (i = ue; i<NE; ++i, ++ue)
			{
				pe = EdgePtr(i);
				if (pe->m_gid == -1)
				{
					ng++;
					se.push(pe);
					break;
				}
				else pe = 0;
			}

			if (pe == 0) break;
		}
		else
		{
			pe = se.top(); se.pop();
			pe->m_gid = ng - 1;

			// assign all unassigned neighbours
			for (j = 0; j<2; ++j)
			{
				FEEdge* pnb = EdgePtr(pe->m_nbr[j]);
				if (pnb && (pnb->m_gid == -1))
				{
					pnb->m_gid = -2;
					se.push(pnb);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Assign gids to the nodes based on the edge gids.
void FEMesh::AutoPartitionNodes()
{
	// get the mesh
	int NE = Edges();

	// reset node tags
	int nn = 0;
	for (int i = 0; i<Nodes(); ++i) Node(i).m_gid = -1;

	// find the largest node tag
	/*	for (int i=0; i<m.Nodes(); ++i)
	{
	int gid = m.Node(i).m_gid;
	if (gid+1 > nn) nn = gid+1;
	}
	*/
	// get the edge pointer
	FEEdge* pe = EdgePtr();

	// loop over all edges
	for (int i = 0; i<NE; ++i, ++pe)
	{
		if ((pe->m_nbr[0] == -1) || (EdgePtr(pe->m_nbr[0])->m_gid != pe->m_gid))
		{
			FENode& node = Node(pe->n[0]);
			if (node.m_gid == -1) { node.m_gid = nn++; }
		}
		if ((pe->m_nbr[1] == -1) || (EdgePtr(pe->m_nbr[1])->m_gid != pe->m_gid))
		{
			FENode& node = Node(pe->n[1]);
			if (node.m_gid == -1) { node.m_gid = nn++; }
		}
	}
}

//-----------------------------------------------------------------------------
// This function is called when element ID's were reassigned (e.g. after loading
// the mesh or after the user created a new partition. This requires that we rebuild the faces
void FEMesh::Repartition()
{
	BuildFaces();

	// repartition the surface
	AutoPartitionSurface();
	UpdateNormals();

	// rebuild the edges since the number of edges may have changed
	BuildEdges();

	// partition the edges
	AutoPartitionEdges();

	// partition the nodes
	AutoPartitionNodes();

	// update the mesh
	MarkExteriorNodes();
}

//-----------------------------------------------------------------------------

void FEMesh::PartitionElementSelection()
{
	int NE = Elements();

	// first see how many parts we already have
	int np = CountElementPartitions();

	// now, assign a new part ID to the selected elements
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = Element(i);
		if (el.IsSelected()) el.m_ntag = 1; else el.m_ntag = 0;
	}

	vector<int> s(NE, -1); int ne = 0;
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = Element(i);
		if (el.m_ntag == 1)
		{
			el.m_ntag = 0;
			s[ne++] = i;
			while (ne > 0)
			{
				FEElement& el = Element(s[--ne]);
				int nbr = (el.IsSolid() ? el.Faces() : el.Edges());
				for (int j = 0; j < nbr; ++j)
				{
					FEElement_* pe = ElementPtr(el.m_nbr[j]);
					if (pe && (pe->m_gid == el.m_gid) && (pe->m_ntag == 1))
					{
						s[ne++] = el.m_nbr[j];
						pe->m_ntag = 0;
					}
				}
				el.m_gid = np;
			}
			np++;
		}
	}

	// since we may have lost a partition, reindex the partitions
	UpdateElementPartitions();

	// add new faces, if found
	int nfp = CountFacePartitions();
	int newFaces = 0;
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = Element(i);
		if (el.IsSelected())
		{
			// solid elements
			int nf = el.Faces();
			for (int j=0; j<nf; ++j)
			{
				if (el.m_face[j] == -1)
				{
					assert(el.m_nbr[j] >= 0);
					if (el.m_nbr[j] >= 0)
					{
						FEElement& ej = Element(el.m_nbr[j]);
						if (ej.m_gid != el.m_gid)
						{
							FEFace fj = el.GetFace(j);
							fj.m_gid = -1;
							AddFace(fj);
							newFaces++;
						}
					}
				}
				else
				{
					FEFace& face = Face(el.m_face[j]);
					face.m_gid += nfp + 1;
				}
			}

			// shell elements
			int ne = el.Edges();
			if (ne > 0)
			{
				assert(el.m_face[0] != -1);
				Face(el.m_face[0]).m_gid += nfp + 1;
			}
		}
	}
	// we need to update all the face neighbors
	// This updates also the element's face indices
	UpdateFacePartitions();
	UpdateFaces();

	// assign new face partitions where necessary
	nfp = CountFacePartitions();
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		if (face.m_gid == -1)
		{
			stack<int> s; s.push(i);
			face.m_gid = nfp;
			while (s.empty() == false)
			{
				FEFace& face = Face(s.top()); s.pop();
				for (int j=0; j<face.Edges(); ++j)
				{
					if (face.m_nbr[j] >= 0)
					{
						FEFace& fj = Face(face.m_nbr[j]);
						if (fj.m_gid == -1)
						{
							fj.m_gid = nfp;
							s.push(face.m_nbr[j]);
						}
					}
				}
			}
			nfp++;
		}
	}
	UpdateFacePartitions();

	// repartition the edges and nodes
	BuildEdges();
	AutoPartitionEdges();
	AutoPartitionNodes();
}

//-----------------------------------------------------------------------------
void FEMesh::PartitionNodeSet(FENodeSet* pg)
{
	if ((pg == 0) || (pg->size() == 0)) return;

	FEItemListBuilder::Iterator it = pg->begin();
	for (; it != pg->end(); ++it)
	{
		PartitionNode(*it);
	}
}

//-----------------------------------------------------------------------------

void FEMesh::AssignElementsToPartition(int lid)
{
	int NE = Elements();

	// assign the selected elements to the part
	for (int i = 0; i<NE; ++i)
	{
		FEElement& el = Element(i);
		if (el.IsSelected()) el.m_gid = lid;
	}

	// repartition the surface
	Repartition();
}

//-----------------------------------------------------------------------------
void FEMesh::PartitionNode(int nodeIndex)
{
	// get the node
	FENode& node = Node(nodeIndex);

	// see if this node is already partitioned or not
	// If it is, we don't need to do anything
	if (node.m_gid >= 0) return;

	// okay, partition the node then
	int ng = CountNodePartitions();
	node.m_gid = ng;

	// It's possible this node splits an edge, so let's check for that
	ng = CountEdgePartitions();
	for (int i=0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if (edge.m_gid >= 0)
		{
			if (edge.n[0] == nodeIndex)
			{
				FEEdge* pe = EdgePtr(edge.m_nbr[0]);
				edge.m_nbr[0] = -1;
				if (pe)
				{
					if      (pe->n[0] == nodeIndex) pe->m_nbr[0] = -1;
					else if (pe->n[1] == nodeIndex) pe->m_nbr[1] = -1;
					else assert(false);
				}

				int m = edge.n[1];
				edge.m_gid = ng;
				FEEdge* e = EdgePtr(edge.m_nbr[1]);
				while (e)
				{
					e->m_gid = ng;
					if (e->n[0] == m) { e = EdgePtr(e->m_nbr[1]); m = e->n[1]; }
					else { e = EdgePtr(e->m_nbr[0]); m = e->n[0]; }
				}

				break;
			}
			else if (edge.n[1] == nodeIndex)
			{
				FEEdge* pe = EdgePtr(edge.m_nbr[1]);
				edge.m_nbr[1] = -1;
				if (pe)
				{
					if      (pe->n[0] == nodeIndex) pe->m_nbr[0] = -1;
					else if (pe->n[1] == nodeIndex) pe->m_nbr[1] = -1;
					else assert(false);
				}

				int m = edge.n[0];
				edge.m_gid = ng;
				FEEdge* e = EdgePtr(edge.m_nbr[0]);
				while (e)
				{
					e->m_gid = ng;
					if (e->n[0] == m) { m = e->n[1]; e = EdgePtr(e->m_nbr[1]); }
					else { m = e->n[0]; e = EdgePtr(e->m_nbr[0]); }
				}

				break;
			}
		}
	}
}
