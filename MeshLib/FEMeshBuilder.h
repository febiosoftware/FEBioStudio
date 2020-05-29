#pragma once
#include <MathLib/math3d.h>

class FENode;
class FEMesh;
class FEEdgeSet;
class FENodeSet;

class FEMeshBuilder
{
public:
	FEMeshBuilder(FEMesh& mesh);

	// Rebuild all mesh data
	void RebuildMesh(double smoothingAngle = 60.0, bool partitionMesh = false);

	// Partition a node (may split an edge)
	void PartitionNode(int node);

	// Partition all nodes in a node set
	void PartitionNodeSet(FENodeSet* pg);

	// auto-partition the surface
	void AutoPartition(double smoothingAngle);

	// partition selections
	void PartitionFaceSelection(int gid = -1);
	void PartitionEdgeSelection(int gid = -1);
	void PartitionElementSelection(int gid = -1);

	// auto-partition selections
	bool AutoPartitionEdges(double w, FEEdgeSet* pg);

	// Add an (isolated) node. The node will be partitioned.
	FENode* AddNode(const vec3d& r);

	// Remove nodes that are not attached to anything
	void RemoveIsolatedNodes();

	// delete selected nodes
	void DeleteSelectedNodes();

	// delete selected faces
	void DeleteSelectedFaces();

	// delete selected elements
	void DeleteSelectedElements();

	// delete tagged nodes
	void DeleteTaggedNodes(int tag);

	// delete tagged edges
	void DeleteTaggedEdges(int tag);

	// delete tagged faces
	void DeleteTaggedFaces(int tag);

	// Delete tagged elements
	void DeleteTaggedElements(int tag);

	// Attach another mesh to this mesh
	void Attach(FEMesh& fem);

	// Attach and weld another mesh to this mesh
	void AttachAndWeld(FEMesh& mesh, double tol);

	void InvertTaggedElements(int ntag);
	void InvertSelectedElements();

	void InvertTaggedFaces(int ntag);
	void InvertSelectedFaces();

	// detach the selected elements and create a new mesh
	FEMesh* DetachSelectedMesh();

	// remove duplicate edges
	void RemoveDuplicateEdges();

	// remove duplicate edges
	void RemoveDuplicateFaces();

private:
	void BuildFaces();
	void BuildEdges();
	void AutoPartitionElements();
	void AutoPartitionSurface();
	void AutoPartitionEdges();
	void AutoPartitionNodes();

private:
	FEMesh&	m_mesh;
};
