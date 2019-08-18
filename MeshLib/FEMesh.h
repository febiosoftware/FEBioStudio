// FEMesh.h: interface for the FEMesh class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEMESH_H__CB7EC714_E3FD_46A4_A397_FEFB23429520__INCLUDED_)
#define AFX_FEMESH_H__CB7EC714_E3FD_46A4_A397_FEFB23429520__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MeshLib/FECoreMesh.h"
#include "MeshTools/FEGroup.h"
#include "MeshTools/FENodeData.h"
#include "MeshTools/FESurfaceData.h"
#include "MeshTools/FEElementData.h"
#include <vector>
#include <set>
#include <string>
using namespace std;

//-----------------------------------------------------------------------------
class FEMesh;
class FESurfaceMesh;

//-----------------------------------------------------------------------------
// Forward declaration of GObject class. The GObject class will own and manage
// the FEMesh. 
class GObject;

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

	//! see if this is a shell mesh
	bool IsShell() const;

	//! see if this is a solid mesh
	bool IsSolid() const;

	//! clear this mesh
	void Clear();

public: // --- E L E M E N T   A C C E S S ---

	//! return number of elements
	int Elements() const { return (int)m_Elem.size(); }

	//! return element
	FEElement& Element(int n) { return m_Elem[n]; }
	const FEElement& Element(int n) const { return m_Elem[n]; }

	//! return reference to element
	FEElement_& ElementRef(int n) { return m_Elem[n]; }
	const FEElement_& ElementRef(int n) const { return m_Elem[n]; }

	//! return pointer to element
	FEElement* ElementPtr(int n=0) { return ((n>=0) && (n<(int)m_Elem.size())? &m_Elem[n] : 0); }

public:
	// --- S U B M E S H ---

	// find a face of an element
	int FindFace(FEElement* pe, FEFace& f, FEFace& fe);

	// returns a list of node indices that belong to a part with part ID gid
	void FindNodesFromPart(int gid, vector<int>& node);

	FEMesh* ExtractFaces(bool selectedOnly);

public: // --- S E R I A L I Z A T I O N ---
	void Save(OArchive& ar);
	void Load(IArchive& ar);

public: // --- E V A L U A T I O N ---

	// get the current element value
	double GetElementValue(int n) const { return m_data[n]; }

	// get the data tag
	int GetElementDataTag(int n) const { return m_idata[n]; }

	// set the element value
	void SetElementValue(int n, double v) { m_data[n] = v; }

	// set the data tag
	void SetElementDataTag(int n, int tag) { m_idata[n] = tag; }

	// update the range of values
	void UpdateValueRange();

	// get the value range
	void GetValueRange(double& vmin, double& vmax) const;

public:
	// --- U P D A T E ---
	void Update();
	void UpdateElementNeighbors();
	void UpdateFaceNeighbors();
	void UpdateEdgeNeighbors();
	void UpdateFaceElementTable();

	// Find and label all exterior nodes
	void MarkExteriorNodes();

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

public:
	void ShowElements(vector<int>& elem, bool show = true);
	void UpdateItemVisibility();
	void ShowAllElements();

	void SelectElements(const vector<int>& elem);

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

	void ClearFaceSelection();

	// detach the selected elements and create a new mesh
	FEMesh* DetachSelectedMesh();

	// remove duplicate edges
	void RemoveDuplicateEdges();

	// remove duplicate edges
	void RemoveDuplicateFaces();

public: 
	// get the local positions of an element
	void ElementNodeLocalPositions(const FEElement& e, vec3d* r) const;

public:
	// tag all elements
	void TagAllElements(int ntag);

	// resize arrays
	void ResizeNodes(int newSize);
	void ResizeEdges(int newSize);
	void ResizeFaces(int newSize);
	void ResizeElems(int newSize);

	// return tag counts
	int CountTaggedNodes(int tag) const;

public:
	int NodeDataFields() const { return (int)m_nodeData.size(); }
	FENodeData* AddNodeDataField(const string& name, double v = 0.0);
	FENodeData& GetNodeDataField(int i) { return m_nodeData[i]; }
	FENodeData* FindNodeDataField(const string& sz);
	void RemoveNodeDataField(int i);

	int SurfaceDataFields() const { return (int)m_surfData.size(); }
	FESurfaceData& GetSurfaceDataField(int i) { return *m_surfData[i]; }
	FESurfaceData* AddSurfaceDataField(const string& name, FESurface* surface, FEMeshData::DATA_TYPE dataType);
	FESurfaceData* FindSurfaceDataField(const string& sz);
	void RemoveSurfaceDataField(int i);

	int ElementDataFields() const { return (int) m_elemData.size(); }
	FEElementData* AddElementDataField(const string& name, double v = 0.0);
	FEElementData& GetElementDataField(int i) { return m_elemData[i]; }
	FEElementData* FindElementDataField(const string& sz);
	void RemoveElementDataField(int i);

	// simpler interface for accessing data fields
	int DataFields() const;
	FEMeshData* GetMeshData(int i);
	void RemoveMeshData(int i);

public:
	void BuildSurfaceNodeNodeTable(vector< set<int> >& NNT);

public:
	int CountNodePartitions() const;
	int CountEdgePartitions() const;
	int CountFacePartitions() const;
	int CountElementPartitions() const;
	int CountSmoothingGroups() const;

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

	// element data (maybe create class for this)
	std::vector<double>		m_data;		//!< element values
	std::vector<int>		m_idata;	//!< used as tags to see if elements have valid data
	double	m_min, m_max;				//!< value range of element data

	// data fields
	vector<FENodeData>		m_nodeData;
	vector<FESurfaceData*>	m_surfData;
	vector<FEElementData>	m_elemData;
};

double bias(double b, double x);
double gain(double g, double x);

FEMesh* ConvertSurfaceToMesh(FESurfaceMesh* surfaceMesh);

#endif // !defined(AFX_FEMESH_H__CB7EC714_E3FD_46A4_A397_FEFB23429520__INCLUDED_)
