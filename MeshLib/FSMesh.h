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
//! This class describes a finite element mesh. Every FSMesh must be owned by a GObject class.
class FSMesh : public FSCoreMesh
{
public:
	// --- C O N S T R U C T I O N ---
	//! Default constructor
	FSMesh();
	//! Copy constructor
	FSMesh(FSMesh& m);
	//! Constructor from surface mesh
	FSMesh(FSSurfaceMesh& m);
	//! Virtual destructor
	virtual ~FSMesh();

	//! Allocate space for mesh
	void Create(int nodes, int elems, int faces = 0, int edges = 0) override;

	//! Copy part of the mesh
	void ShallowCopy(FSMesh* pm);

	//! Clear this mesh
	void Clear();

	//! Clear all selections
	void ClearSelections();

	//! Save mesh to archive
	void Save(OArchive& ar);
	//! Load mesh from archive
	void Load(IArchive& ar);

public: // from FSCoreMesh

	//! Return number of elements
	int Elements() const override { return (int)m_Elem.size(); }

	//! Return element reference
	FSElement& Element(int n) { return m_Elem[n]; }
	//! Return const element reference
	const FSElement& Element(int n) const { return m_Elem[n]; }

	//! Return reference to element
	FSElement_& ElementRef(int n) override { return m_Elem[n]; }
	//! Return const reference to element
	const FSElement_& ElementRef(int n) const override { return m_Elem[n]; }

	//! Set uniform shell thickness
	void SetUniformShellThickness(double h);

public:
	//! Build all mesh data structures
	void BuildMesh() override;

	//! Update the mesh
	void UpdateMesh() override;

	//! Reconstruct the mesh
	void RebuildMesh(double smoothingAngle = 60.0, bool partitionMesh = false);

	//! Count selected elements
	int CountSelectedElements() const;

	//! Return node index from its nodal ID
	int NodeIndexFromID(int nid);

	//! Re-generate nodal IDs (startID must be larger than 0!)
	int GenerateNodalIDs(int startID = 1);

	//! Build node lookup table
	void BuildNLT();
	//! Clear node lookup table
	void ClearNLT();

	//! Return element index from its element ID
	int ElementIndexFromID(int eid);

	//! (Re-)generate element IDs (startID must be larger than 0!)
	int GenerateElementIDs(int startID = 1);

	//! Build element lookup table
	void BuildELT();
	//! Clear element lookup table
	void ClearELT();

public: // Helper functions for updating mesh data structures
	//! Rebuild element data
	void RebuildElementData();
	//! Rebuild face data
	void RebuildFaceData();
	//! Rebuild edge data
	void RebuildEdgeData();
	//! Rebuild node data
	void RebuildNodeData();

	//! Update element neighbors
	void UpdateElementNeighbors();
	//! Update face neighbors
	void UpdateFaceNeighbors();
	//! Update edge neighbors
	void UpdateEdgeNeighbors();
	//! Update face element table
	void UpdateFaceElementTable();
	//! Update edge element table
	void UpdateEdgeElementTable();

	//! Update node partitions
	void UpdateNodePartitions();
	//! Update edge partitions
	void UpdateEdgePartitions();
	//! Update face partitions
	void UpdateFacePartitions();
	//! Update element partitions
	void UpdateElementPartitions();
	//! Update smoothing groups
	void UpdateSmoothingGroups();

	//! Remove elements with specified tag
	int RemoveElements(int ntag);

	//! Mark exterior edges
	void MarkExteriorEdges();

	//! Validate elements
	bool ValidateElements() const;
	//! Validate faces
	bool ValidateFaces() const;
	//! Validate edges
	bool ValidateEdges() const;

	//! The following functions may leave the mesh in an invalid state.
public:
	//! Resize node array
	void ResizeNodes(int newSize);
	//! Resize edge array
	void ResizeEdges(int newSize);
	//! Resize face array
	void ResizeFaces(int newSize);
	//! Resize element array
	void ResizeElems(int newSize);

	//! Extract faces and return as new mesh
	FSMesh* ExtractFaces(bool selectedOnly);
	//! Extract faces and return as surface mesh
    FSSurfaceMesh* ExtractFacesAsSurface(bool selectedOnly);

public:
	//! Get number of mesh data fields
	int MeshDataFields() const;
	//! Get mesh data field by index
	FSMeshData* GetMeshDataField(int i);
	//! Find mesh data field by name
	FSMeshData* FindMeshDataField(const std::string& sz);
	//! Remove mesh data field by index
	void RemoveMeshDataField(int i);
	//! Remove mesh data field by pointer
	void RemoveMeshDataField(FSMeshData* data);
	//! Get mesh data index
	int GetMeshDataIndex(FSMeshData* data);
	//! Insert mesh data at specified index
	void InsertMeshData(int i, FSMeshData* data);
	//! Add mesh data field
	void AddMeshDataField(FSMeshData* data);

	//! Add node data field
	FSNodeData*    AddNodeDataField   (const std::string& name, FSNodeSet* nodeset, DATA_TYPE dataType);
	//! Add surface data field
	FSSurfaceData* AddSurfaceDataField(const std::string& name, FSSurface* surface, DATA_TYPE dataType);
	//! Add element data field
	FSElementData* AddElementDataField(const std::string& name, FSElemSet* part, DATA_TYPE dataType);
	//! Add part data field
	FSPartData*    AddPartDataField   (const std::string& name, FSPartSet* part, DATA_TYPE dataType);
	//! Clear all mesh data
	void ClearMeshData();

	//! Find part data field by name
	FSPartData* FindPartDataField(const std::string& name);

	//! Get mesh data reference
	Mesh_Data& GetMeshData();

public:
	//! Get number of FE part sets
	int FEPartSets() const;
	//! Get number of FE element sets
	int FEElemSets() const;
	//! Get number of FE surfaces
	int FESurfaces() const;
	//! Get number of FE edge sets
	int FEEdgeSets() const;
	//! Get number of FE node sets
	int FENodeSets() const;

	//! Get FE element set by index
	FSElemSet* GetFEElemSet(int n);
	//! Get FE surface by index
	FSSurface* GetFESurface(int n);
	//! Get FE edge set by index
	FSEdgeSet* GetFEEdgeSet(int n);
	//! Get FE node set by index
	FSNodeSet* GetFENodeSet(int n);
	//! Get FE part set by index
	FSPartSet* GetFEPartSet(int n);

	//! Add FE element set
	void AddFEElemSet(FSElemSet* pg);
	//! Add FE surface
	void AddFESurface(FSSurface* pg);
	//! Add FE edge set
	void AddFEEdgeSet(FSEdgeSet* pg);
	//! Add FE node set
	void AddFENodeSet(FSNodeSet* pg);
	//! Add FE part set
	void AddFEPartSet(FSPartSet* pg);

	//! Remove FE element set
	int RemoveFEElemSet(FSElemSet* pg);
	//! Remove FE surface
	int RemoveFESurface(FSSurface* pg);
	//! Remove FE edge set
	int RemoveFEEdgeSet(FSEdgeSet* pg);
	//! Remove FE node set
	int RemoveFENodeSet(FSNodeSet* pg);
	//! Remove FE part set
	int RemoveFEPartSet(FSPartSet* pg);

	//! Insert FE element set at specified index
	void InsertFEElemSet(int n, FSElemSet* pg);
	//! Insert FE surface at specified index
	void InsertFESurface(int n, FSSurface* pg);
	//! Insert FE edge set at specified index
	void InsertFEEdgeSet(int n, FSEdgeSet* pg);
	//! Insert FE node set at specified index
	void InsertFENodeSet(int n, FSNodeSet* pg);
	//! Insert FE part set at specified index
	void InsertFEPartSet(int n, FSPartSet* pg);

	//! Find FE group by ID
	FSGroup* FindFEGroup(int nid);
	//! Find FE part set by name
	FSPartSet* FindFEPartSet(const std::string& name);
	//! Find FE surface by name
	FSSurface* FindFESurface(const std::string& szname);
	//! Find FE edge set by name
	FSEdgeSet* FindFEEdgeSet(const std::string& szname);
	//! Find FE node set by name
	FSNodeSet* FindFENodeSet(const std::string& szname);

	//! Clear all FE groups
	void ClearFEGroups();
	//! Remove empty FE groups
	void RemoveEmptyFEGroups();
	//! Remove unused FE groups
	void RemoveUnusedFEGroups();

public:
	//! Take item lists from another mesh
	void TakeItemLists(FSMesh* pm);
	//! Take mesh data from another mesh
	void TakeMeshData(FSMesh* pm);

public:
	//! Get number of mesh partitions
	int MeshPartitions() const { return (int)m_Dom.Size(); }
	//! Clear mesh partitions
	void ClearMeshPartitions();

	//! Get mesh partition by index
	FSMeshPartition& MeshPartition(int i) { return *m_Dom[i]; }

	//! Update mesh partitions
	void UpdateMeshPartitions();

public: // --- M E S H   Q U E R I E S ---

	//! Find duplicate faces
	void FindDuplicateFaces(std::vector<int>& l);
	//! Find duplicate edges
	void FindDuplicateEdges(std::vector<int>& l);

	//! Select elements based on face selection
	std::vector<int> GetElementsFromSelectedFaces();

	//! Get node element list
	FSNodeElementList& NodeElementList();

	//! Find face index
	int FindFaceIndex(FSFace& face);

protected:
	//! Clear mesh topology
	void ClearMeshTopo();

public:
	//! Map FE node sets from another mesh
	void MapFENodeSets(FSMesh* pm);
	//! Map FE element sets from another mesh
	void MapFEElemSets(FSMesh* pm);
	//! Map FE surfaces from another mesh
	void MapFESurfaces(FSMesh* pm);

protected:
	//! FE elements
	std::vector<FSElement>	m_Elem;

	//! Mesh data (used for data evaluation)
	Mesh_Data	m_data;

	//! Data fields
	FSObjectList<FSMeshData>	m_meshData;

	//! Named element sets
	FSObjectList<FSElemSet>		m_pFEElemSet;
	//! Named surfaces
	FSObjectList<FSSurface>		m_pFESurface;
	//! Named edge sets
	FSObjectList<FSEdgeSet>		m_pFEEdgeSet;
	//! Named node sets
	FSObjectList<FSNodeSet>		m_pFENodeSet;
	//! Named part sets
	FSObjectList<FSPartSet>		m_pFEPartSet;

	//! Domains
	FSObjectList<FSMeshPartition>	m_Dom;

protected:
	//! Node ID lookup table
	std::vector<int> m_NLT;
	//! The minimum node ID
	int m_nltmin;

	//! Element ID lookup table
	std::vector<int> m_ELT;
	//! The minimum element ID
	int m_eltmin;

	//! Node element list
	FSNodeElementList m_NEL;

	friend class FSMeshBuilder;
};

//! Bias function
double bias(double b, double x);
//! Gain function
double gain(double g, double x);

//! Mesh tools namespace
namespace MeshTools {

	//! Convert surface mesh to volume mesh
	FSMesh* ConvertSurfaceToMesh(FSSurfaceMesh* surfaceMesh);

	//! Get connected elements
	std::vector<int> GetConnectedElements(FSMesh* pm, int startIndex, double fconn, bool bpart, bool exteriorOnly, bool bmax);
}