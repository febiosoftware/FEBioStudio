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
#include <FECore/vec3d.h>

class FENode;
class FEMesh;
class FEEdgeSet;
class FENodeSet;
class FESurface;

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
	bool AutoPartitionFaces(double w, FESurface* pg);

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

	// delete all elements of a part
	FEMesh* DeletePart(FEMesh& mesh, int partId);

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

	// repair edges (and node data)
	void RepairEdges();

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
