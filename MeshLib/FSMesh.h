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
#include "FSCoreMesh.h"
#include <GeomLib/FSGroup.h>
#include "FSMeshPartition.h"
#include "FSMeshData.h"
#include "Mesh_Data.h"
#include "FSNodeElementList.h"
#include <FSCore/FSObjectList.h>
#include <vector>
#include <set>
#include <string>

//-----------------------------------------------------------------------------
class FSSurfaceMesh;
class FSMesh;
class FSMeshData;
class FSNodeData;
class FSSurfaceData;
class FSElementData;
class FSPartData;

//-----------------------------------------------------------------------------
class FSMeshBuilder;
class FSSurfaceMesh;

//-----------------------------------------------------------------------------
// This class describes a finite element mesh. Every FSMesh must be owned by a
// GObject class. 
class FSMesh : public FSCoreMesh
{
public:
	// --- C O N S T R U C T I O N ---
	FSMesh();
	FSMesh(FSMesh& m);
	FSMesh(FSSurfaceMesh& m);
	virtual ~FSMesh();

	// allocate space for mesh
	void Create(int nodes, int elems, int faces = 0, int edges = 0) override;

	// copy part of the mesh
	void ShallowCopy(FSMesh* pm);

	//! clear this mesh
	void Clear();

	// clear all selections
	void ClearSelections();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public: // from FSCoreMesh

	//! return number of elements
	int Elements() const override { return (int)m_Elem.size(); }

	//! return element
	FSElement& Element(int n) { return m_Elem[n]; }
	const FSElement& Element(int n) const { return m_Elem[n]; }

	//! return reference to element
	FSElement_& ElementRef(int n) override { return m_Elem[n]; }
	const FSElement_& ElementRef(int n) const override { return m_Elem[n]; }

	void SetUniformShellThickness(double h);

public:
	// Build all mesh data structures
	// This assumes that all mesh items are created and partitioned!
	void BuildMesh() override;

	// update the mesh
	void UpdateMesh() override;

	// reconstruct the mesh
	void RebuildMesh(double smoothingAngle = 60.0, bool partitionMesh = false);

	int CountSelectedElements() const;

	// return node index from its nodal ID
	int NodeIndexFromID(int nid);

	// re-generate nodal IDs (startID must be larger than 0!)
	// returns one larger than the largets ID that was assigned
	int GenerateNodalIDs(int startID = 1);

	void BuildNLT();
	void ClearNLT();

	// return element index from its element ID
	int ElementIndexFromID(int eid);

	// (re-)generate element IDs (startID must be larger than 0!)
	// returns one larger than the largets ID that was assigned
	int GenerateElementIDs(int startID = 1);

	void BuildELT();
	void ClearELT();

public: // Helper functions for updating mesh data structures
	void RebuildElementData();
	void RebuildFaceData();
	void RebuildEdgeData();
	void RebuildNodeData();

	void UpdateElementNeighbors();
	void UpdateFaceNeighbors();
	void UpdateEdgeNeighbors();
	void UpdateFaceElementTable();
	void UpdateEdgeElementTable();

	void UpdateNodePartitions();
	void UpdateEdgePartitions();
	void UpdateFacePartitions();
	void UpdateElementPartitions();
	void UpdateSmoothingGroups();

	int RemoveElements(int ntag);

	void MarkExteriorElements();
	void MarkExteriorEdges();

	// mesh validation
	bool ValidateElements() const;
	bool ValidateFaces() const;
	bool ValidateEdges() const;

	// The following functions may leave the mesh in an invalid state.
	// As a result, these functions should be used with care. 
public:
	// resize arrays
	void ResizeNodes(int newSize);
	void ResizeEdges(int newSize);
	void ResizeFaces(int newSize);
	void ResizeElems(int newSize);

	// extract faces and return as new mesh
	FSMesh* ExtractFaces(bool selectedOnly);
    FSSurfaceMesh* ExtractFacesAsSurface(bool selectedOnly);

public:
	int MeshDataFields() const;
	FSMeshData* GetMeshDataField(int i);
	FSMeshData* FindMeshDataField(const std::string& sz);
	void RemoveMeshDataField(int i);
	void RemoveMeshDataField(FSMeshData* data);
	int GetMeshDataIndex(FSMeshData* data);
	void InsertMeshData(int i, FSMeshData* data);
	void AddMeshDataField(FSMeshData* data);

	FSNodeData*    AddNodeDataField   (const std::string& name, FSNodeSet* nodeset, DATA_TYPE dataType);
	FSSurfaceData* AddSurfaceDataField(const std::string& name, FSSurface* surface, DATA_TYPE dataType);
	FSElementData* AddElementDataField(const std::string& name, FSElemSet* part, DATA_TYPE dataType);
	FSPartData*    AddPartDataField   (const std::string& name, FSPartSet* part, DATA_TYPE dataType);
	void ClearMeshData();

	FSPartData* FindPartDataField(const std::string& name);

	Mesh_Data& GetMeshData();

public:
	int FEPartSets() const;
	int FEElemSets() const;
	int FESurfaces() const;
	int FEEdgeSets() const;
	int FENodeSets() const;

	FSElemSet* GetFEElemSet(int n);
	FSSurface* GetFESurface(int n);
	FSEdgeSet* GetFEEdgeSet(int n);
	FSNodeSet* GetFENodeSet(int n);
	FSPartSet* GetFEPartSet(int n);

	void AddFEElemSet(FSElemSet* pg);
	void AddFESurface(FSSurface* pg);
	void AddFEEdgeSet(FSEdgeSet* pg);
	void AddFENodeSet(FSNodeSet* pg);
	void AddFEPartSet(FSPartSet* pg);

	int RemoveFEElemSet(FSElemSet* pg);
	int RemoveFESurface(FSSurface* pg);
	int RemoveFEEdgeSet(FSEdgeSet* pg);
	int RemoveFENodeSet(FSNodeSet* pg);
	int RemoveFEPartSet(FSPartSet* pg);

	void InsertFEElemSet(int n, FSElemSet* pg);
	void InsertFESurface(int n, FSSurface* pg);
	void InsertFEEdgeSet(int n, FSEdgeSet* pg);
	void InsertFENodeSet(int n, FSNodeSet* pg);
	void InsertFEPartSet(int n, FSPartSet* pg);

	FSGroup* FindFEGroup(int nid);
	FSPartSet* FindFEPartSet(const std::string& name);
	FSSurface* FindFESurface(const std::string& szname);
	FSEdgeSet* FindFEEdgeSet(const std::string& szname);
	FSNodeSet* FindFENodeSet(const std::string& szname);

	void ClearFEGroups();
	void RemoveEmptyFEGroups();
	void RemoveUnusedFEGroups();

public:
	int MeshPartitions() const { return (int)m_Dom.Size(); }
	void ClearMeshPartitions();

	FSMeshPartition& MeshPartition(int i) { return *m_Dom[i]; }

	void UpdateMeshPartitions();

public: // --- M E S H   Q U E R I E S ---
	void BuildSurfaceNodeNodeTable(std::vector< std::set<int> >& NNT);

	void FindDuplicateFaces(std::vector<int>& l);
	void FindDuplicateEdges(std::vector<int>& l);

	// select elements based on face selection
	std::vector<int> GetElementsFromSelectedFaces();

	FSNodeElementList& NodeElementList();

	int FindFaceIndex(FSFace& face);

public:
	// these functions attempt to map user selections from pm onto this mesh
	void MapFENodeSets(FSMesh* pm);
	void MapFEElemSets(FSMesh* pm);
	void MapFESurfaces(FSMesh* pm);

	void CopyFENodeSets(FSMesh* pm);
	void CopyFEElemSets(FSMesh* pm);
	void CopyFESurfaces(FSMesh* pm);

protected:
	// elements
	std::vector<FSElement>	m_Elem;	//!< FE elements

	// mesh data (used for data evaluation)
	Mesh_Data	m_data;

	// data fields
	std::vector<FSMeshData*>		m_meshData;

	FSObjectList<FSElemSet>		m_pFEElemSet;
	FSObjectList<FSSurface>		m_pFESurface;
	FSObjectList<FSEdgeSet>		m_pFEEdgeSet;
	FSObjectList<FSNodeSet>		m_pFENodeSet;
	FSObjectList<FSPartSet>		m_pFEPartSet;

	FSObjectList<FSMeshPartition>	m_Dom;	// domains

protected:
	// Node index look up table
	std::vector<int> m_NLT;	// node ID lookup table
	int m_nltmin;			// the min ID

	// Element index look up table
	std::vector<int> m_ELT;	// Element ID lookup table
	int m_eltmin;			// the min ID

	FSNodeElementList m_NEL;

	friend class FSMeshBuilder;
};

double bias(double b, double x);
double gain(double g, double x);

namespace MeshTools {

	FSMesh* ConvertSurfaceToMesh(FSSurfaceMesh* surfaceMesh);

	std::vector<int> GetConnectedElements(FSMesh* pm, int startIndex, double fconn, bool bpart, bool exteriorOnly, bool bmax);
}
