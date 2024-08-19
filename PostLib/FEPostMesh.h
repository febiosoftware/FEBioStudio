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

#include <MeshLib/FENode.h>
#include <MeshLib/FEElement.h>
#include <MeshLib/FEMesh.h>
#include "FEGroup.h"
#include <MeshLib/FENodeElementList.h>
#include <MeshLib/FENodeFaceList.h>
#include <FSCore/box.h>
#include <utility>
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
class FEPostMesh : public FSMesh
{
public:
	// --- M E M O R Y   M A N A G M E N T ---
	//! constructor
	FEPostMesh();

	//! destructor
	virtual ~FEPostMesh();

	//! allocate storage for mesh
	void Create(int nodes, int elems, int faces = 0, int edges = 0) override;

	//! Clean up all data
	void CleanUp();

	//! clean mesh and all data
	void ClearAll();

public:
	// --- G E O M E T R Y ---

	//! return domains
	int Domains() const { return (int) m_Dom.size(); }

	//! return a domain
	MeshDomain& Domain(int i) { return *m_Dom[i]; }

	//! nr of element sets
	int ElemSets() const { return (int) m_ESet.size(); }

	//! add a element set
	void AddElemSet(FSElemSet* pg) { m_ESet.push_back(pg); }

	//! return an element set
	FSElemSet& ElemSet(int n) { return *m_ESet[n]; }

	// number of surfaces
	int Surfaces() const { return (int) m_Surf.size(); }

	// return a surface
	FSSurface& Surface(int n) { return *m_Surf[n]; }

	FSSurface* FindSurface(const std::string& s);

	// Add a surface
	void AddSurface(FSSurface* ps) { m_Surf.push_back(ps); }

	//! number of node sets
	int NodeSets() const { return (int) m_NSet.size(); }

	//! return a node set
	FSNodeSet& NodeSet(int i) { return *m_NSet[i]; }

	//! Add a node set
	void AddNodeSet(FSNodeSet* ps) { m_NSet.push_back(ps); }

	// --- D A T A   U P D A T E ---

	//! update mesh data
	void BuildMesh() override;

protected:
	void UpdateDomains();

	void ClearDomains();
	void ClearElemSets();
	void ClearSurfaces();
	void ClearNodeSets();

protected:
	// --- G E O M E T R Y ---
	std::vector<MeshDomain*>	m_Dom;	// domains

	// user-defined partitions
	std::vector<FSElemSet*>	m_ESet;	// element sets
	std::vector<FSSurface*>	m_Surf;	// surfaces
	std::vector<FSNodeSet*>	m_NSet;	// node sets
};

// find the element and the iso-parametric coordinates of a point inside the mesh
// the x coordinates is assumed to be in reference frame
bool FindElementInReferenceFrame(FSCoreMesh& m, const vec3f& x, int& nelem, double r[3]);

class FEState;

double IntegrateNodes(Post::FEPostMesh& mesh, Post::FEState* ps);
double IntegrateEdges(Post::FEPostMesh& mesh, Post::FEState* ps);

// This function calculates the integral over a surface. Note that if the surface
// is triangular, then we calculate the integral from a degenerate quad.
double IntegrateFaces(Post::FEPostMesh& mesh, Post::FEState* ps);
double IntegrateReferenceFaces(Post::FEPostMesh& mesh, Post::FEState* ps);

// integrates the surface normal scaled by the data field
vec3d IntegrateSurfaceNormal(Post::FEPostMesh& mesh, Post::FEState* ps);

// This function calculates the integral over a volume. Note that if the volume
// is not hexahedral, then we calculate the integral from a degenerate hex.
double IntegrateElems(Post::FEPostMesh& mesh, Post::FEState* ps);
double IntegrateReferenceElems(Post::FEPostMesh& mesh, Post::FEState* ps);

} // namespace Post
