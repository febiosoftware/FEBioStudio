#pragma once
#include "FECoreMesh.h"
#include <MeshTools/FEGroup.h>
#include <MeshTools/FEMeshData.h>
#include <vector>
#include <set>
#include <string>
using namespace std;

//-----------------------------------------------------------------------------
class FESurfaceMesh;
class FEMesh;
class FEMeshData;
class FENodeData;
class FESurfaceData;
class FEElementData;

//-----------------------------------------------------------------------------
class Mesh_Data
{
	struct DATA
	{
		double	val[FEElement::MAX_NODES];	// nodal values for element
		int		nval;						// number of nodal values (should equal nr of nodes for corresponding element)
		int		tag;
	};

public:
	Mesh_Data();
	Mesh_Data(const Mesh_Data& d);
	void operator = (const Mesh_Data& d);

	void Clear();

	void Init(FEMesh* mesh, double initVal, int initTag);

	bool IsValid() const { return (m_data.empty() == false); }

	DATA& operator[](size_t n) { return m_data[n]; }

	// get the current element value
	double GetElementValue(int elem, int node) const { return m_data[elem].val[node]; }

	// get the average element value
	double GetElementAverageValue(int elem);

	// get the data tag
	int GetElementDataTag(int n) const { return m_data[n].tag; }

	// set the element's node value
	void SetElementValue(int elem, int node, double v) { m_data[elem].val[node] = v; }

	// set the element (average) value
	void SetElementValue(int elem, double v);

	// set the data tag
	void SetElementDataTag(int n, int tag) { m_data[n].tag = tag; }

	// update the range of values
	void UpdateValueRange();

	// get the value range
	void GetValueRange(double& vmin, double& vmax) const;

public:
	std::vector<DATA>		m_data;		//!< element values
	double	m_min, m_max;				//!< value range of element data
};

//-----------------------------------------------------------------------------
class FEMeshBuilder;

//-----------------------------------------------------------------------------
// This class describes a finite element mesh. Every FEMesh must be owned by a
// GObject class. 
class FEMesh : public FECoreMesh
{
public:
	// --- C O N S T R U C T I O N ---
	FEMesh();
	FEMesh(FEMesh& m);
	virtual ~FEMesh();

	// allocate space for mesh
	void Create(int nodes, int elems, int faces = 0, int edges = 0) override;

	// copy part of the mesh
	void ShallowCopy(FEMesh* pm);

	//! clear this mesh
	void Clear();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public: // from FECoreMesh

	//! return number of elements
	int Elements() const override { return (int)m_Elem.size(); }

	//! return element
	FEElement& Element(int n) { return m_Elem[n]; }
	const FEElement& Element(int n) const { return m_Elem[n]; }

	//! return reference to element
	FEElement_& ElementRef(int n) override { return m_Elem[n]; }
	const FEElement_& ElementRef(int n) const override { return m_Elem[n]; }

public:
	// Build all mesh data structures
	// This assumes that all mesh items are created and partitioned!
	void BuildMesh() override;

	// reconstruct the mesh
	void RebuildMesh(double smoothingAngle = 60.0, bool partitionMesh = false);

protected: // Helper functions for updating mesh data structures
	void RebuildElementData();
	void RebuildFaceData();
	void RebuildEdgeData();
	void RebuildNodeData();

	void UpdateElementNeighbors();
	void UpdateFaceNeighbors();
	void UpdateEdgeNeighbors();
	void UpdateFaceElementTable();

	void UpdateNodePartitions();
	void UpdateEdgePartitions();
	void UpdateFacePartitions();
	void UpdateElementPartitions();
	void UpdateSmoothingGroups();

	void RemoveElements(int ntag);

	void MarkExteriorElements();
	void MarkExteriorFaces();
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
	FEMesh* ExtractFaces(bool selectedOnly);

public:
	int MeshDataFields() const;
	FEMeshData* GetMeshDataField(int i);
	FEMeshData* FindMeshDataField(const string& sz);
	void RemoveMeshDataField(int i);
	int GetMeshDataIndex(FEMeshData* data);
	void InsertMeshData(int i, FEMeshData* data);
	void AddMeshDataField(FEMeshData* data);

	FENodeData* AddNodeDataField(const string& name, double v = 0.0);
	FESurfaceData* AddSurfaceDataField(const string& name, FESurface* surface, FEMeshData::DATA_TYPE dataType);
	FEElementData* AddElementDataField(const string& name, FEPart* part, FEMeshData::DATA_TYPE dataType);
	void ClearMeshData();

	Mesh_Data& GetMeshData();

public: // --- M E S H   Q U E R I E S ---
	void BuildSurfaceNodeNodeTable(vector< set<int> >& NNT);

	void FindDuplicateFaces(vector<int>& l);
	void FindDuplicateEdges(vector<int>& l);

	// select elements based on face selection
	vector<int> GetElementsFromSelectedFaces();

protected:
	// elements
	std::vector<FEElement>	m_Elem;	//!< FE elements

	// mesh data (used for data evaluation)
	Mesh_Data	m_data;

	// data fields
	vector<FEMeshData*>		m_meshData;

	friend class FEMeshBuilder;
};

double bias(double b, double x);
double gain(double g, double x);

FEMesh* ConvertSurfaceToMesh(FESurfaceMesh* surfaceMesh);
