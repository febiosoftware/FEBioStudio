#pragma once
#include <vector>
#include <set>
#include <FSCore/box.h>
#include "FENode.h"
#include "FEEdge.h"
#include "FEFace.h"
#include "FELineMesh.h"

//-------------------------------------------------------------------
// Base class for mesh classes.
// Essentially manages the nodes, edges, and faces
class FEMeshBase : public FELineMesh
{
public:
	FEMeshBase();
	virtual ~FEMeshBase();

	BOX GetBoundingBox() const { return m_box; }

public:
	// get the local positions of a face
	void FaceNodeLocalPositions(const FEFace& f, vec3d* r) const;

public:
	// calculate smoothing IDs based on face normals.
	void AutoSmooth(double angleDegrees);

	// assign smoothing IDs based on surface partition
	void SmoothByPartition();

	// update the normals
	void UpdateNormals();

	// update item visibility
	virtual void UpdateItemVisibility() {}

	void UpdateMeshData() override;

	vec3d FaceCenter(FEFace& f) const;

	vec3d EdgeCenter(FEEdge& e) const;

	// face area
	double FaceArea(FEFace& f);
	double FaceArea(const vector<vec3d>& f, int faceType);

	// --- F A C E   D A T A ---
	void FaceNodePosition(const FEFace& f, vec3d* r) const;
	void FaceNodeNormals(FEFace& f, vec3f* n);
	void FaceNodeTexCoords(FEFace& f, float* t);

	void ClearFaceSelection();

public: // interface for accessing mesh items
	int Faces() const { return (int)m_Face.size(); }
	FEFace& Face(int n) { return m_Face[n]; }
	const FEFace& Face(int n) const { return m_Face[n]; }
	FEFace* FacePtr(int n = 0) { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }
	const FEFace* FacePtr(int n = 0) const { return ((n >= 0) && (n<(int)m_Face.size()) ? &m_Face[n] : 0); }

	void DeleteFaces() { if (!m_Face.empty()) m_Face.clear(); }
	void DeleteEdges() { if (!m_Edge.empty()) m_Edge.clear(); }
	void DeleteNodes() { if (!m_Node.empty()) m_Node.clear(); }

public:
	void TagAllFaces(int ntag);

	int CountSelectedFaces() const;

	// see if the nodes form an edge
	bool IsEdge(int n0, int n1);

	// find an edge if it exists (or null otherwise)
	FEEdge* FindEdge(int n0, int n1);

	bool IsCreaseEdge(int n0, int n1);

	void UpdateBox();

public:
	void BuildNodeFaceTable(vector< vector<int> >& NFT);
	void BuildNodeEdgeTable(vector< vector<int> >& NET);

protected:
	void RemoveEdges(int ntag);
	void RemoveFaces(int ntag);

protected:
	std::vector<FEFace>		m_Face;	//!< FE faces
	BOX	m_box;		// bounding box
};

//-------------------------------------------------------------------
namespace MeshTools {
	// get a list of face indices, connected to a face 
	// INPUT:
	// pm               : the mesh
	// nface            : the index of the start face
	// tolAngleDeg      : angle of selection tolerance (degrees). Set to zero to turn off.
	// respectPartitions: do not cross surface partitions if true
	std::vector<int> GetConnectedFaces(FEMeshBase* pm, int nface, double tolAngleDeg, bool respectPartitions);
}
