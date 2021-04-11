/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <vector>
#include <set>
#include <FSCore/box.h>
#include "FENode.h"
#include "FEEdge.h"
#include "FEFace.h"
#include "FELineMesh.h"

//-------------------------------------------------------------------
// Base class for mesh classes.
// Essentially manages the nodes, edges, and faces
class FEMeshBase : public FELineMesh
{
public:
	FEMeshBase();
	virtual ~FEMeshBase();

	BOX GetBoundingBox() const { return m_box; }

	// from FELineMesh
	void UpdateMesh() override;

public:
	// get the local positions of a face
	void FaceNodeLocalPositions(const FEFace& f, vec3d* r) const;

public:
	// calculate smoothing IDs based on face normals.
	void AutoSmooth(double angleDegrees);

	// assign smoothing IDs based on surface partition
	void SmoothByPartition();

	// update the normals
	void UpdateNormals();

	// update item visibility
	virtual void UpdateItemVisibility() {}

	vec3d FaceCenter(FEFace& f) const;

	vec3d EdgeCenter(FEEdge& e) const;

	// face area
	double FaceArea(FEFace& f);
	double FaceArea(const vector<vec3d>& f, int faceType);

	// --- F A C E   D A T A ---
	void FaceNodePosition(const FEFace& f, vec3d* r) const;
	void FaceNodeNormals(FEFace& f, vec3f* n);
	void FaceNodeTexCoords(FEFace& f, float* t);

	void ClearFaceSelection();

public: // interface for accessing mesh items
	int Faces() const { return (int)m_Face.size(); }
	FEFace& Face(int n) { return m_Face[n]; }
	const FEFace& Face(int n) const { return m_Face[n]; }
	FEFace* FacePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }
	const FEFace* FacePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }

	void DeleteFaces() { if (!m_Face.empty()) m_Face.clear(); }
	void DeleteEdges() { if (!m_Edge.empty()) m_Edge.clear(); }
	void DeleteNodes() { if (!m_Node.empty()) m_Node.clear(); }

public:
	void TagAllFaces(int ntag);

	int CountSelectedNodes() const;
	int CountSelectedEdges() const;
	int CountSelectedFaces() const;

	// see if the nodes form an edge
	bool IsEdge(int n0, int n1);

	// find an edge if it exists (or null otherwise)
	FEEdge* FindEdge(int n0, int n1);

	bool IsCreaseEdge(int n0, int n1);

protected:
	void RemoveEdges(int ntag);
	void RemoveFaces(int ntag);

protected:
	std::vector<FEFace>		m_Face;	//!< FE faces
};

//-------------------------------------------------------------------
namespace MeshTools {
	// get a list of face indices, connected to a face 
	// INPUT:
	// pm               : the mesh
	// nface            : the index of the start face
	// tolAngleDeg      : angle of selection tolerance (degrees). Set to zero to turn off.
	// respectPartitions: do not cross surface partitions if true
	std::vector<int> GetConnectedFaces(FEMeshBase* pm, int nface, double tolAngleDeg, bool respectPartitions);
}
