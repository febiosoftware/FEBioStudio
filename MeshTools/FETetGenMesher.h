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
#include "FEMesher.h"
#include <MeshLib/FSSurfaceMesh.h>
#include <GeomLib/GItem.h>
#include <vector>

class GObject;
class GSurfaceMeshObject;

#ifdef TETLIBRARY
class tetgenio;
#endif

//-----------------------------------------------------------------------------
// The PLC class describes a piece-wise linear complex and is used as an
// intermediate structure beteen the GPLCObject and the TetGen data-structures
//
class PLC
{
public:
	class NODE
	{
	public:
		vec3d	r;
		int		nid;
	};

	class EDGE
	{
	public:
		EDGE() { ndiv = 0; fixedDiv = false; }
		EDGE(const EDGE& e) { node = e.node; nid = e.nid; ndiv = e.ndiv; fixedDiv = e.fixedDiv; }
		void operator = (const EDGE& e) { node = e.node; nid = e.nid; ndiv = e.ndiv; fixedDiv = e.fixedDiv; }

		void AddNode(int n) { node.push_back(n); }
		int FindNode(int nid);

		int Nodes() { return (int) node.size(); }

	public:
		std::vector<int>	node;	// node list
		int			nid;	// edge ID
		int			ndiv;	// number of divisions
		bool		fixedDiv;	// ndiv is fixed and cannot be changed.
	};

	class FACE
	{
	public:
		FACE() {}
		FACE(const FACE& f) { node = f.node; nid = f.nid; }
		void operator = (const FACE& f) { node = f.node; nid = f.nid; }

		int Nodes() { return (int) node.size(); }

	public:
		std::vector<int>	node;
		int					nid;
	};

public:
	PLC();
	bool Build(GObject* po, double h);

	int AddNode(vec3d r, int nid = -1);
	int FindNode(int nid);

	int Nodes() { return (int) m_Node.size(); }
	NODE& Node(int i) { return m_Node[i]; }

	int Faces() { return (int) m_Face.size(); }
	FACE& Face(int i) { return m_Face[i]; }

	int Edges() { return (int) m_Edge.size(); }
	EDGE& Edge(int i) { return m_Edge[i]; }

protected:
	bool ProcessSizing();
	bool BuildNodes();
	bool BuildEdges();
	bool BuildFaces();

protected:
	bool BuildFaceQuad   (GFace& fs);
	bool BuildFacePolygon(GFace& fs);
	bool BuildFaceExtrude(GFace& fs);
	bool BuildFaceRevolve(GFace& fs);
	bool BuildFaceRevolveWedge(GFace& fs);

protected:
	std::vector<NODE>	m_Node;
	std::vector<EDGE>	m_Edge;
	std::vector<FACE>	m_Face;

	GObject*	m_po;	//!< object we are building a PLC for
	double		m_h;	//!< element size
};

//-----------------------------------------------------------------------------
//! This class implements an interface to the TetGen mesher
class FETetGenMesher : public FEMesher
{
public:
	enum { ELSIZE, QUALITY, ELTYPE, SPLIT_FACES, HOLE, HOLE_COORD };

public:
	FETetGenMesher();

	// build the mesh
	FSMesh*	BuildMesh(GObject* po) override;

	double ElementSize();

	bool SplitFaces();

	int ElementType();

	double ElementQuality();

public:
	// Generate a volume mesh from a surface mesh
	FSMesh* CreateMesh(GSurfaceMeshObject* surfaceObj);

private:
	void UpdateElementPartitioning(GObject* po, FSMesh* pm);

protected:
	GObject*	m_po;	// TODO: move this to base class

#ifdef TETLIBRARY
public:
	bool build_plc(FSSurfaceMesh* pm, tetgenio& in);
protected:
	FSMesh* BuildPLCMesh();
	bool build_tetgen_in(tetgenio& in);
	bool build_tetgen_in_remesh(tetgenio& io);
	FSMesh* build_tet_mesh(tetgenio& out);
	FSMesh* build_tet10_mesh(FSMesh* pm);
	FSMesh* build_tet15_mesh(FSMesh* pm);
#endif
};

//-----------------------------------------------------------------------------
class FEConvexHullMesher : public FEMesher
{
public:
	FEConvexHullMesher();
	FSMesh* Create(const std::vector<vec3d>& pointCloud);

protected:
	FSMesh* BuildMesh(GObject* po) override { return nullptr; }
};
