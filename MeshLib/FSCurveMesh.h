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
#include <FSCore/math3d.h>
#include "FSNode.h"
#include "FSElement.h"
#include "FSLineMesh.h"
#include <FSCore/box.h>
#include <FSCore/Serializable.h>
#include <vector>

//! A class that represents a mesh for a curve
//! This mesh only consists of nodes and edges
class FSCurveMesh : public FSLineMesh
{
public:
	//! curve types
	enum CurveType
	{
		INVALID_CURVE,		//!< there is an error in this mesh
		EMPTY_CURVE,		//!< an empty curve (no nodes or edges)
		SIMPLE_CURVE,		//!< a single curve with two end-points
		CLOSED_CURVE,		//!< a single closed loop
		COMPLEX_CURVE,		//!< anything else
	};

public:
	//! constructor
	FSCurveMesh();

	//! allocate storage for mesh
	void Create(int nodes, int edges);

	//! Create curve mesh from a vector of points
	void CreateFromPoints(const std::vector<vec3d>& points, bool isClosed);

	//! return the type of the curve
	CurveType Type() const;

	//! clear curve data
	void Clear();

	//! update mesh data
	void UpdateMesh() override;

	//! rebuild mesh data
	void BuildMesh() override;

public:
	//! Add a node to the mesh. Returns the index of the newly added node
	//! If the snap flag is on, then it will be checked if the point already exists in the mesh
	int AddNode(const vec3d& r, bool snap = true, double snapTolerance = 0.01);

	//! adds an edge between nodes n0, n1
	//! If the edge already exists, it will not be added
	//! Returns the index of the new (or existing) edge
	//! (or -1 if adding the edge has failed (e.g. if n0==n1))
	//! Note that this will set the type to INVALID_CURVE
	//! Call Update to reevaluate the curve mesh
	int AddEdge(int n0, int n1);

	//! tag all the nodes of this mesh
	void TagAllNodes(int ntag);

	//! tag all the edges of this mesh
	void TagAllEdges(int ntag);

	//! sort the nodes and edges based on edge connectivity
	void Sort();

	//! invert the order of the nodes
	void Invert();

	//! reorder nodes based on look-up table
	void ReorderNodes(std::vector<int>& NLT);

	//! reorder edges based on look-up table
	void ReorderEdges(std::vector<int>& ELT);

	//! attach another curve to this one
	void Attach(const FSCurveMesh& c);

	//! get the bounding box of the mesh
	BOX BoundingBox() const;

	//! returns the total lenght of all edge segments
	double Length() const;

	//! count the number of end points
	int EndPoints() const;

	//! return a list of end points
	std::vector<int> EndPointList() const;

	//! count the curve segments (a segment is a simply-connected curve)
	int Segments() const;

	//! remove a node (and connecting edges)
	void RemoveNode(int node);

	//! Serialization
	void Save(OArchive& ar);
	//! Load from archive
	void Load(IArchive& ar);

private:
	//! Flip the orientation of an edge
	void FlipEdge(FSEdge& e);

	//! Update edge neighbor information
	void UpdateEdgeNeighbors();
	//! Update edge IDs
	void UpdateEdgeIDs();
	//! Update node IDs
	void UpdateNodeIDs();
	//! Update the curve type classification
	void UpdateType();

private:
	CurveType		m_type;		//!< curve type
};