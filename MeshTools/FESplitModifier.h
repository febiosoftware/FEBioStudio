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
#include "FEModifier.h"
#include <vector>

//-----------------------------------------------------------------------------
// Splits the selected elements
class FEQuadSplitModifier : public FEModifier
{
	struct DATA {
		FSElement*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FEQuadSplitModifier();
	FSMesh* Apply(FSMesh* pm);

protected:
	FSElement* neighbor(FSElement* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FSElement* pe, int i);

	bool can_split(FSElement* pe, int i);

	bool have_to_split(FSElement* pe, int i);

protected:
	FSMesh*				m_pm;
	std::vector<DATA>	m_Data;
};

//-----------------------------------------------------------------------------
// Split tri elements
class FETriSplitModifier : public FEModifier
{
	struct DATA {
		FSElement*	pe;
		int			ncase;
		int			nid;
		int			ntag;
	};

public:
	FETriSplitModifier();
	FSMesh* Apply(FSMesh* pm);

	void SetIterations(int niter) { m_niter = niter; }

protected:
	FSMesh* Split(FSMesh* pm);

	FSElement* neighbor(FSElement* pe, int i) { return (pe->m_nbr[i] >= 0 ? &m_pm->Element(pe->m_nbr[i]) : 0); }

	bool is_split(FSElement* pe, int i);

	bool can_split(FSElement* pe, int i);

	bool have_to_split(FSElement* pe, int i);

protected:
	FSMesh*				m_pm;
	int					m_niter;
};

//-----------------------------------------------------------------------------
// Splits tetrahedral elements
class FETetSplitModifier : public FEModifier
{
public:
	FETetSplitModifier();
	FSMesh* Apply(FSMesh* pm);

protected:
	double	m_tol;	// tolerance for splitting facets
};


//-----------------------------------------------------------------------------
// Splits hexahedral elements
class FEHexSplitModifier : public FEModifier
{
public:
	FEHexSplitModifier();
	FSMesh* Apply(FSMesh* pm);

	void DoSurfaceSmoothing(bool b);

private:
	FSMesh* RefineMesh(FSMesh* pm);
	FSMesh* RefineSelection(FSMesh* pm);

protected:
	double	m_tol;				// tolerance for splitting facets
	bool	m_smoothSurface;	// smooth surface
};


//-----------------------------------------------------------------------------
// Refines meshes of linear solid elements of any combination of types, such as
// hex-dominant meshes. Every element is split in eight: a hex8 into 8 hex8, a
// penta6 into 8 penta6 and a tet4 into 8 tet4 (the octahedron in its middle is
// split along its shortest diagonal), except that a pyra5 is split into 6 pyra5
// and 4 tet4. Every edge gets a node at its midpoint and every quad face a node at
// its center, so the refined mesh is conforming. The element, surface, edge and
// node partitions of the mesh are preserved.
// The surface can optionally be smoothed by a subdivision step that is Catmull-Clark
// on regular quads and Loop on regular triangles (cf. the quad/triangle subdivision
// of Stam and Loop, 2003). The edges between surfaces are kept as creases and the
// corner nodes of the mesh stay in place. Nodes only move normal to the surface (or
// to the crease), so flat surfaces and straight edges do not change. The new nodes
// inside the mesh follow the surface, and nodes move less where their full move
// would invert an element.
// Children can be worse than their parent where the parent's Jacobian is negative or
// small at a corner but fine at the integration points that FEBio checks (e.g. a
// pyra5 whose apex is below the plane of its base at a corner). Elements that come
// out worse than the worst element before the split (and below 0.2) are improved by
// optimizing the nodes around them (with FEMeshUntangler, then moving the nodes of
// the worst element without folding corners); nodes on a surface only slide where it
// is flat, or along straight creases.
class FEMixedSplitModifier : public FEModifier
{
public:
	FEMixedSplitModifier();
	FSMesh* Apply(FSMesh* pm) override;

	void DoSurfaceSmoothing(bool b);

	// improve the elements that come out worse than before the split (on by default)
	void DoElementRepair(bool b) { m_repair = b; }

	// true if the mesh has elements and all of them are hex8, penta6, pyra5 or tet4
	static bool IsSupported(FSMesh& mesh);

	// number of nodes that smoothing moved less (or not at all), so as not to invert elements
	int LimitedNodes() const { return m_limitedNodes; }

	// number of refined elements that were worse than the worst element before the split
	// (and below 0.2), around which the nodes were optimized
	int OptimizedElements() const { return m_optimized; }

	// smallest scaled Jacobian of the elements at FEBio's integration points (1 = ideal element)
	static double WorstElement(FSMesh& mesh);

protected:
	bool	m_smoothSurface;	// smooth surface
	bool	m_repair;			// improve the elements that come out worse than before the split
	int		m_limitedNodes;		// nodes that smoothing moved less, to keep the elements valid
	int		m_optimized;		// elements around which the nodes were optimized
	double	m_worstBefore;		// smallest scaled Jacobian before the split
};

//-----------------------------------------------------------------------------
// Splits hexahedral elements only in-plane
class FEHex2DSplitModifier : public FEModifier
{
public:
	FEHex2DSplitModifier();
	FSMesh* Apply(FSMesh* pm);

	void DoSurfaceSmoothing(bool b);

protected:
	double	m_tol;				// tolerance for splitting facets
	bool	m_smoothSurface;	// smooth surface
};
