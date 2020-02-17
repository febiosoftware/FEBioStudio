#pragma once
#include "FEElement.h"
#include "FEMeshBase.h"
#include <FSCore/Archive.h>
#include <vector>

//-----------------------------------------------------------------------------
class TriMesh;

//-----------------------------------------------------------------------------
// Class for representing surface meshes
class FESurfaceMesh : public FEMeshBase
{
public:
	FESurfaceMesh();
	FESurfaceMesh(const FESurfaceMesh& mesh);
	FESurfaceMesh(TriMesh& triMesh);
	virtual ~FESurfaceMesh();

	FESurfaceMesh& operator = (const FESurfaceMesh& mesh);

	void Create(unsigned int nodes, unsigned int edges, unsigned int faces);

	void RebuildMesh(double smoothingAngle = 60.0);

	void AutoPartition(double smoothingAngle);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void Update();

	void BuildNodeFaceTable(vector< vector<int> >& NFT);

	bool IsType(FEFaceType type);

public:
	// remove duplicate edges
	void RemoveDuplicateEdges();

	// attach another surface mesh
	void Attach(const FESurfaceMesh& mesh);

	// attach another surface mesh, welding all edge nodes that are within a certain distance
	void AttachAndWeld(const FESurfaceMesh& mesh, double weldTolerance);

	// add a facet
	void AddFacet(int n0, int n1, int n2);

public:
	void PartitionEdgeSelection(int partition = -1);
	void PartitionNodeSelection();

	void AutoPartitionEdges();
	void AutoPartitionNodes();

public:
	void ShowFaces(const vector<int>& face, bool bshow = true);
	void ShowAllFaces();
	void UpdateItemVisibility() override;

	void SelectFaces(const vector<int>& faceList);

public:
	void DeleteSelectedNodes();
	void DeleteSelectedEdges();
	void DeleteSelectedFaces();

	void DeleteTaggedNodes(int tag);
	void DeleteTaggedEdges(int tag);
	void DeleteTaggedFaces(int tag);

	void RemoveIsolatedNodes();
	void RemoveIsolatedEdges();

public:
	int CountNodePartitions() const;
	int CountEdgePartitions() const;
	int CountFacePartitions() const;
	int CountSmoothingGroups() const;

	void UpdateNodePartitions();
	void UpdateEdgePartitions();
	void UpdateFacePartitions();
	void UpdateSmoothingGroups();

public:
	void BuildEdges();
	void UpdateFaces();
	void UpdateFaceNeighbors();
	void UpdateFaceEdges();
	void UpdateEdgeNeighbors();

	// resize arrays
	void ResizeNodes(int newSize);
	void ResizeEdges(int newSize);
	void ResizeFaces(int newSize);
};

// Create a TriMesh from a surface mesh
void BuildTriMesh(TriMesh& dyna, FESurfaceMesh* pm);

namespace MeshTools {
	// Is the mesh closed (i.e. do all faces have neighbors)
	bool IsMeshClosed(FESurfaceMesh& m);
}