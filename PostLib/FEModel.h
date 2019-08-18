#pragma once

#include "FEMesh.h"
#include "FEMaterial.h"
#include "FEState.h"
#include "FEDataManager.h"
#include "bbox.h"
#include <vector>
using namespace std;

namespace Post {

//-----------------------------------------------------------------------------
class MetaData
{
public:
	string	author;		// author of model file
	string	software;	// software that generated the model file
};

//-----------------------------------------------------------------------------
// Base class for derived classes that need to be informed when the model changes
class FEModelDependant
{
public:
	FEModelDependant() {}
	virtual ~FEModelDependant() {}

	// This function is called whenever the model changes
	// When the model is deleted it will call this function with 0
	virtual void Update(FEModel* pfem) = 0;
};

//-----------------------------------------------------------------------------
// Class that describes an FEModel. A model consists of a mesh (in the future
// there can be multiple meshes to support remeshing), a list of materials
// and a list of states. The states contain the data associated with the model
class FEModel  
{
public:
	// con-/destructor
	FEModel();
	virtual ~FEModel();

	// clear all model data
	void Clear();

	void DeleteMeshes();

	// get/set name
	void SetName(const std::string& name);
	const string& GetName() const;

	// get/set title of model
	void SetTitle(const string& title);
	const string& GetTitle() const;

	// get the current active state
	int currentTime() { return m_ntime; }

	//! get the data manager
	FEDataManager* GetDataManager() { return m_pDM; }

	// --- M A T E R I A L S ---

	// return number of materials
	int Materials() { return (int) m_Mat.size();  }

	// get a particular material
	FEMaterial* GetMaterial(int i) { return &m_Mat[i]; }

	// clear all materials
	void ClearMaterials() { m_Mat.clear(); }

	// add a material to the model
	void AddMaterial(FEMaterial& mat);

	// --- S T A T E   M A N A G M E N T ---
	//! add a state to the mesh
	void AddState(FEState* pFEState);

	//! Add a new state at time
	void AddState(float ftime);

	//! insert a state at a particular time
	void InsertState(FEState* ps, float f);

	//! remove a state from the mesh
	void DeleteState(int n);

	//! get the nr of states
	int GetStates() { return (int) m_State.size(); }

	//! retrieve pointer to a state
	FEState* GetState(int nstate) { return m_State[nstate]; }

	//! retrieve pointer to active state
	FEState* GetActiveState() { return m_State[m_ntime]; }

	//! Add a new data field
	void AddDataField(FEDataField* pd);

	//! add a new data field constrained to a set
	void AddDataField(FEDataField* pd, vector<int>& L);

	//! delete a data field
	void DeleteDataField(FEDataField* pd);

	//! Copy a data field
	FEDataField* CopyDataField(FEDataField* pd, const char* sznewname = 0);

	//! Create a cached copy of a data field
	FEDataField* CreateCachedCopy(FEDataField* pd, const char* sznewname);

	// Get the field variable name
	std::string getDataString(int nfield, Data_Tensor_Type ntype);

public:
	//! get the bounding box
	BOUNDINGBOX GetBoundingBox() { return m_bbox; }

	//! Update the bounding box
	void UpdateBoundingBox();

	// Clear all states
	void ClearStates();

	// --- E V A L U A T I O N ---
	bool Evaluate(int nfield, int ntime, bool breset = false);

	// get the nodal coordinates of an element at time
	void GetElementCoords(int iel, int ntime, vec3f* r);

	// evaluate scalar functions
	void EvaluateNode   (int n, int ntime, int nfield, NODEDATA& d);
	void EvaluateEdge   (int n, int ntime, int nfield, EDGEDATA& d);
	bool EvaluateFace   (int n, int ntime, int nfield, float* data, float& val);
	bool EvaluateElement(int n, int ntime, int nfield, float* data, float& val);

	// evaluate based on point
	void EvaluateNode(const vec3f& r, int ntime, int nfield, NODEDATA& d);

	// evaluate vector functions
	vec3f EvaluateNodeVector(int n, int ntime, int nvec);
	bool EvaluateFaceVector(int n, int ntime, int nvec, vec3f& r);
	vec3f EvaluateElemVector(int n, int ntime, int nvec);

	// evaluate tensor functions
	mat3f EvaluateNodeTensor(int n, int ntime, int nten, int ntype = -1);
	mat3f EvaluateFaceTensor(int n, int ntime, int nten, int ntype = -1);
	mat3f EvaluateElemTensor(int n, int ntime, int nten, int ntype = -1);

	// displacement field
	void SetDisplacementField(int ndisp) { m_ndisp = ndisp; }
	int GetDisplacementField() { return m_ndisp; }
	vec3f NodePosition(int n, int ntime);
	vec3f FaceNormal(FEFace& f, int ntime);

	vec3f NodePosition(const vec3f& r, int ntime);

	// checks if the field code is valid for the given state
	bool IsValidFieldCode(int nfield, int nstate);

public:
	void AddDependant(FEModelDependant* pc);
	void UpdateDependants();
	void RemoveDependant(FEModelDependant* pc);
	void ClearDependants();

public:
	void AddMesh(FEMeshBase* mesh);
	int Meshes() const;
	FEMeshBase* GetFEMesh(int i);

	void UpdateMeshState(int ntime);

public:
	static void SetInstance(FEModel* fem);
	static FEModel* GetInstance();

	MetaData& GetMetaData() { return m_meta; }

protected:
	// Helper functions for data evaluation
	void EvalNodeField(int ntime, int nfield);
	void EvalFaceField(int ntime, int nfield);
	void EvalElemField(int ntime, int nfield);
	
protected:
	string	m_name;		// name (as displayed in model viewer)
	string	m_title;	// title of project
	int		m_ntime;	// time step that is being evaluated

	// --- M E T A   D A T A ---
	MetaData	m_meta;

	// --- M E S H ---
	vector<FEMeshBase*>		m_mesh;		// the list of meshes
	BOUNDINGBOX		m_bbox;				// bounding box of mesh

	// --- M A T E R I A L S ---
	vector<FEMaterial>	m_Mat;		// array of materials

	// --- S T A T E ---
	vector<FEState*>	m_State;	// array of pointers to FE-state structures
	FEDataManager*		m_pDM;		// the Data Manager
	int					m_ndisp;	// vector field defining the displacement

	// dependants
	vector<FEModelDependant*>	m_Dependants;

	static FEModel*	m_pThis;
};
} // namespace Post
