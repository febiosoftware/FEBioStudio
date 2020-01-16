#pragma once
#include "FECoreMesh.h"
#include "MeshTools/FEGroup.h"
#include "MeshTools/FENodeData.h"
#include "MeshTools/FESurfaceData.h"
#include "MeshTools/FEElementData.h"
#include <vector>
#include <set>
#include <string>
using namespace std;

//-----------------------------------------------------------------------------
class FESurfaceMesh;

//-----------------------------------------------------------------------------
// Forward declaration of GObject class. The GObject class will own and manage
// the FEMesh. 
class GObject;

//-----------------------------------------------------------------------------
class Mesh_Data
{
	struct DATA
	{
		double	val;
		int		tag;
	};

public:
	Mesh_Data();
	Mesh_Data(const Mesh_Data& d);
	void operator = (const Mesh_Data& d);

	void Clear();

	void Resize(size_t size);

	void Init(double data, int idata);

	DATA& operator[](size_t n) { return m_data[n]; }

	// get the current element value
	double GetElementValue(int n) const { return m_data[n].val; }

	// get the data tag
	int GetElementDataTag(int n) const { return m_data[n].tag; }

	// set the element value
	void SetElementValue(int n, double v) { m_data[n].val = v; }

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
	void Create(int nodes, int elems, int faces = 0, int edges = 0);

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
	void RebuildMesh(double smoothingAngle = 60.0, bool autoSurface = true);

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
	int NodeDataFields() const { return (int)m_nodeData.size(); }
	FENodeData* AddNodeDataField(const string& name, double v = 0.0);
	FENodeData* GetNodeDataField(int i) { return m_nodeData[i]; }
	FENodeData* FindNodeDataField(const string& sz);
	void RemoveNodeDataField(int i);

	int SurfaceDataFields() const { return (int)m_surfData.size(); }
	FESurfaceData* GetSurfaceDataField(int i) { return m_surfData[i]; }
	FESurfaceData* AddSurfaceDataField(const string& name, FESurface* surface, FEMeshData::DATA_TYPE dataType);
	FESurfaceData* FindSurfaceDataField(const string& sz);
	void RemoveSurfaceDataField(int i);

	int ElementDataFields() const { return (int) m_elemData.size(); }
	FEElementData* AddElementDataField(const string& name, FEPart* part, FEMeshData::DATA_TYPE dataType);
	FEElementData* GetElementDataField(int i) { return m_elemData[i]; }
	FEElementData* FindElementDataField(const string& sz);
	void RemoveElementDataField(int i);

	// simpler interface for accessing data fields
	int DataFields() const;
	FEMeshData* GetMeshData(int i);
	void RemoveMeshData(int i);

	Mesh_Data& GetMeshData() { return m_data; }

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
	vector<FENodeData*>		m_nodeData;
	vector<FESurfaceData*>	m_surfData;
	vector<FEElementData*>	m_elemData;
};

double bias(double b, double x);
double gain(double g, double x);

FEMesh* ConvertSurfaceToMesh(FESurfaceMesh* surfaceMesh);
