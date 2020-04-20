#pragma once
#include "FECoreMesh.h"
#include "MeshTools/FEGroup.h"
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
// Forward declaration of GObject class. The GObject class will own and manage
// the FEMesh. 
class GObject;

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

	void ClearElements();

	void CreateElements(int elems);

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
	// --- S U B M E S H ---

	FEMesh* ExtractFaces(bool selectedOnly);

public: // --- S E R I A L I Z A T I O N ---
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	// --- U P D A T E ---
	void Update();
	void UpdateElementNeighbors();
	void UpdateFaceNeighbors();
	void UpdateEdgeNeighbors();
	void UpdateFaceElementTable();

	void BuildFaces();
	void BuildEdges();
	void UpdateFaces();

	// reconstruct the mesh
	void RebuildMesh(double smoothingAngle = 60.0, bool autoSurface = true, bool partitionMesh = false);

public:
	void AutoPartitionElements();
	void AutoPartitionSurface();
	void AutoPartitionSurfaceQuick();
	void AutoPartitionEdges();
	void AutoPartitionNodes();

	void PartitionFaceSelection();
	void PartitionEdgeSelection();
	void PartitionElementSelection();

	void AssignElementsToPartition(int lid);

	void PartitionNodeSet(FENodeSet* pg);

	void Repartition();

	void AutoPartition(double smoothingAngle);

	void PartitionNode(int node);

public: // --- M E S H   M A N I P U L A T I O N ---

	void RemoveIsolatedNodes();
	void Attach(FEMesh& fem);
	void AttachAndWeld(FEMesh& mesh, double tol);

	void AddNode(const vec3d& r);

	void AddNode(FENode& n) { m_Node.push_back(n); }
	void AddEdge(FEEdge& e) { m_Edge.push_back(e); }
	void AddFace(FEFace& f) { m_Face.push_back(f); }
	void AddElement(FEElement& e) { m_Elem.push_back(e); }

	void DeleteTaggedElements(int tag);
	void DeleteTaggedFaces   (int tag);
	void DeleteTaggedEdges   (int tag);
	void DeleteTaggedNodes   (int tag);

	void FindDuplicateFaces(vector<int>& l);
	void FindDuplicateEdges(vector<int>& l);

	void DeleteSelectedElements();
	void DeleteSelectedFaces();
	void DeleteSelectedNodes();

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

	// select elements based on face selection
	vector<int> GetElementsFromSelectedFaces();

public:
	// resize arrays
	void ResizeNodes(int newSize);
	void ResizeEdges(int newSize);
	void ResizeFaces(int newSize);
	void ResizeElems(int newSize);

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

public:
	void BuildSurfaceNodeNodeTable(vector< set<int> >& NNT);

public: // The following functions may leave the mesh in an invalid state or try to restore (part) of the mesh' state.
		 // As a result, these functions should be used with care. 
	void UpdateNodePartitions();
	void UpdateEdgePartitions();
	void UpdateFacePartitions();
	void UpdateSmoothingGroups();
	void UpdateElementPartitions();
	void RemoveElements(int ntag);
	void SplitEdgePartition(int edgeID);
	void SplitFacePartition(int faceID);

protected:
	// elements
	std::vector<FEElement>	m_Elem;	//!< FE elements

	// mesh data (used for data evaluation)
	Mesh_Data	m_data;

	// data fields
	vector<FEMeshData*>		m_meshData;
};

double bias(double b, double x);
double gain(double g, double x);

FEMesh* ConvertSurfaceToMesh(FESurfaceMesh* surfaceMesh);
