/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include "FEElement.h"
#include "FEMeshBase.h"
#include "Mesh_Data.h"
#include <FSCore/Archive.h>
#include <vector>

//-----------------------------------------------------------------------------
class TriMesh;

//-----------------------------------------------------------------------------
// Class for representing surface meshes
class FSSurfaceMesh : public FSMeshBase
{
public:
	FSSurfaceMesh();
	FSSurfaceMesh(const FSSurfaceMesh& mesh);
	FSSurfaceMesh(TriMesh& triMesh);
	virtual ~FSSurfaceMesh();

	void Clear();

	FSSurfaceMesh& operator = (const FSSurfaceMesh& mesh);

public:
	// allocate mesh data structures
	void Create(unsigned int nodes, unsigned int edges, unsigned int faces);

	// Build the mesh data structures
	void BuildMesh() override;

	void RebuildMesh(double smoothingAngle = 60.0);

	void AutoPartition(double smoothingAngle);

	void Update();

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	bool IsType(FEFaceType type);

	// remove duplicate edges
	void RemoveDuplicateEdges();

	// attach another surface mesh
	void Attach(const FSSurfaceMesh& mesh);

	// attach another surface mesh, welding all edge nodes that are within a certain distance
	void AttachAndWeld(const FSSurfaceMesh& mesh, double weldTolerance);

	// add a facet
	void AddFacet(int n0, int n1, int n2);

public:
	void PartitionEdgeSelection(int partition = -1);
	void PartitionNodeSelection();

public:
	void ShowFaces(const std::vector<int>& face, bool bshow = true);
	void ShowAllFaces();
	void UpdateItemVisibility() override;

	void SelectFaces(const std::vector<int>& faceList);

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

	void AutoPartitionEdges();
	void AutoPartitionNodes();

	Mesh_Data& GetMeshData();

private:
	// mesh data (used for data evaluation)
	Mesh_Data	m_data;
};

// Create a TriMesh from a surface mesh
void BuildTriMesh(TriMesh& dyna, FSSurfaceMesh* pm);

namespace MeshTools {
	// Is the mesh closed (i.e. do all faces have neighbors)
	bool IsMeshClosed(FSSurfaceMesh& m);
}