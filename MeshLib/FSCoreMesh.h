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
#include "FSNode.h"
#include "FSElement.h"
#include "FSMeshBase.h"
#include <vector>
#include <functional>

//-----------------------------------------------------------------------------
//! This class defines a simple mesh structure that provides basic container
//! services for storing mesh data. It only stores nodes, edges, faces. It implements 
//! an interface for accessing element data, but derived classes need to implement this. 
class FSCoreMesh : public FSMeshBase
{
public:
	//! constructor
	FSCoreMesh();

	//! destructor
	virtual ~FSCoreMesh();

	//! allocate space for mesh
	virtual void Create(int nodes, int elems, int faces = 0, int edges = 0) = 0;

	//! check the type of the mesh
	bool IsType(int ntype) const;

	//! get the mesh type (returns -1 for mixed meshes)
	int GetMeshType() const;

public: // interface for accessing elements

	//! total number of elements
	virtual int Elements() const = 0;

	//! return reference to element
	virtual FSElement_& ElementRef(int n) = 0;
	virtual const FSElement_& ElementRef(int n) const = 0;

	//! return pointer to element
	FSElement_* ElementPtr(int n = 0);
	const FSElement_* ElementPtr(int n = 0) const;

	// tag all elements
	void TagAllElements(int ntag);

	// select a list of elements
	void SelectElements(const std::vector<int>& elem);

public:
	void ShowElements(std::vector<int>& elem, bool show = true);
	void UpdateItemVisibility();
	void ShowAllElements();

public:
	vec3d ElementCenter(const FSElement_& el) const;

	// element volume
	double ElementVolume(int iel);
	double ElementVolume(const FSElement_& el);
	double HexVolume(const FSElement_& el);
	double PentaVolume(const FSElement_& el);
	double TetVolume(const FSElement_& el);
	double PyramidVolume(const FSElement_& el);
	double QuadVolume(const FSElement_& el);

	//! see if this is a shell mesh
	bool IsShell() const;

	//! see if this is a solid mesh
	bool IsSolid() const;

	//! return nr of shell elements
	int ShellElements();

	//! return nr of solid elements
	int SolidElements();

	//! return nr of beam elements
	int BeamElements();

	//! Is an element exterior or not
	bool IsExterior(FSElement_* pe) const;

	// find a face of an element
	int FindFace(FSElement_* pe, FSFace& f, FSFace& fe);

	// Find and label all exterior nodes
	void MarkExteriorNodes();

	// returns a list of node indices that belong to a part with part ID gid
	void FindNodesFromPart(int gid, std::vector<int>& node);

	// find a node from its GID
	FSNode* FindNodeFromID(int gid);

	int CountNodePartitions() const;
	int CountEdgePartitions() const;
	int CountFacePartitions() const;
	int CountElementPartitions() const;
};

inline FSElement_* FSCoreMesh::ElementPtr(int n) { return ((n >= 0) && (n<Elements()) ? &ElementRef(n) : 0); }
inline const FSElement_* FSCoreMesh::ElementPtr(int n) const { return ((n >= 0) && (n<Elements()) ? &ElementRef(n) : 0); }

// --- I N T E G R A T E ---
double IntegrateQuad(vec3d* r, float* v);
float IntegrateQuad(vec3f* r, float* v);
float IntegrateHex(vec3f* r, float* v);

double hex8_volume(vec3d* r, bool bJ = false);
double hex20_volume(vec3d* r, bool bJ = false);
double hex27_volume(vec3d* r, bool bJ = false);
double tet4_volume(vec3d* r, bool bJ = false);
double tet5_volume(vec3d* r, bool bJ = false);
double tet10_volume(vec3d* r, bool bJ = false);
double tet15_volume(vec3d* r, bool bJ = false);
double tet20_volume(vec3d* r, bool bJ = false);
double penta6_volume(vec3d* r, bool bJ = false);
double penta15_volume(vec3d* r, bool bJ = false);
double pyra5_volume(vec3d* r, bool bJ = false);
double pyra13_volume(vec3d* r, bool bJ = false);
double tri3_volume(vec3d* r, vec3d* D, bool bJ = false);
double quad4_volume(vec3d* r, vec3d* D, bool bJ = false);

// helper functions
void ForAllElements(FSCoreMesh& mesh, std::function<void(FSElement_& el)> f);
void ForAllSelectedElements(FSCoreMesh& mesh, std::function<void(FSElement_& el)> f);
void ForAllSelectedNodes(FSCoreMesh& mesh, std::function<void(FSNode& node)> f);
void ForAllTaggedNodes(FSCoreMesh& mesh, int tag, std::function<void(FSNode& node)> f);
