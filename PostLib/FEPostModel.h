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

#include "FEPostMesh.h"
#include "Material.h"
#include "FEState.h"
#include "FEDataManager.h"
#include "GLObject.h"
#include <FSCore/box.h>
#include <vector>
//using namespace std;

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
	virtual void Update(FEPostModel* pfem) = 0;
};

//-----------------------------------------------------------------------------
// Class that describes an FEPostModel. A model consists of a mesh (in the future
// there can be multiple meshes to support remeshing), a list of materials
// and a list of states. The states contain the data associated with the model
class FEPostModel  
{
public:
	class PlotObject : public CGLObject
	{
	public:
		PlotObject();

		GLColor	Color();

		void SetColor(const GLColor& c);

		double Scale() const;

		int FindObjectDataIndex(const std::string& name) const;

	public:
		int				m_id;
		int				m_tag;
		vec3d			m_pos;
		quatd			m_rot;

		std::vector<PlotObjectData*>	m_data;
	};

	class PointObject : public PlotObject
	{
	public:
		PointObject() {}

	public:
		vec3d	m_rt;
	};

	class LineObject : public PlotObject
	{
	public:
		LineObject() {}

	public:
		vec3d	m_r1;
		vec3d	m_r2;

		vec3d	m_r01;
		vec3d	m_r02;
	};

public:
	// con-/destructor
	FEPostModel();
	virtual ~FEPostModel();

	// clear all model data
	void Clear();

	void DeleteMeshes();

	// get/set name
	void SetName(const std::string& name);
	const string& GetName() const;

	// get/set title of model
	void SetTitle(const string& title);
	const string& GetTitle() const;

	float CurrentTime() const { return m_fTime; }
	int CurrentTimeIndex() const { return m_nTime; }

	FEState* CurrentState();

	void SetCurrentTimeIndex(int n);

	// set the active state closest to time t
	void SetTimeValue(float ftime);

	// get the time value of state n
	float GetTimeValue(int ntime);

	// get the state closest to time t
	int GetClosestTime(double t);

	//! get the data manager
	FEDataManager* GetDataManager() { return m_pDM; }

	// --- M A T E R I A L S ---

	// return number of materials
	int Materials() { return (int) m_Mat.size();  }

	// get a particular material
	Material* GetMaterial(int i) { return &m_Mat[i]; }

	// clear all materials
	void ClearMaterials() { m_Mat.clear(); }

	// add a material to the model
	void AddMaterial(Material& mat);

	// --- S T A T E   M A N A G M E N T ---
	//! add a state to the mesh
	void AddState(FEState* pFEState);

	//! Add a new state at time
	void AddState(float ftime, int nstatus = 0, bool interpolateData = false);

	//! insert a state at a particular time
	void InsertState(FEState* ps, float f);

	//! remove a state from the mesh
	void DeleteState(int n);

	//! get the nr of states
	int GetStates() { return (int) m_State.size(); }

	//! retrieve pointer to a state
	FEState* GetState(int nstate) { return m_State[nstate]; }

	//! Add a new data field
	void AddDataField(ModelDataField* pd, const std::string& name = "");

	//! add a new data field constrained to a set
	void AddDataField(ModelDataField* pd, std::vector<int>& L);

	//! delete a data field
	void DeleteDataField(ModelDataField* pd);

	//! Copy a data field
	ModelDataField* CopyDataField(ModelDataField* pd, const char* sznewname = 0);

	//! Create a cached copy of a data field
	ModelDataField* CreateCachedCopy(ModelDataField* pd, const char* sznewname);

	// Get the field variable name
	std::string getDataString(int nfield, Data_Tensor_Type ntype);

	// interpolate data between its neighbors
	void InterpolateStateData(FEState* ps);

public:
	//! get the bounding box
	BOX GetBoundingBox() { return m_bbox; }

	//! Update the bounding box
	void UpdateBoundingBox();

	// Clear all states
	void ClearStates();

	// --- E V A L U A T I O N ---
	bool Evaluate(int nfield, int ntime, bool breset = false);

	// get the nodal coordinates of an element at time
	void GetElementCoords(int iel, int ntime, vec3f* r);

	// project a point onto the mesh
	int ProjectToMesh(int nstate, const vec3f& r0, vec3d& rt, bool bfollow);

	// evaluate scalar functions
	void EvaluateNode   (int n, int ntime, int nfield, NODEDATA& d);
	bool EvaluateEdge   (int n, int ntime, int nfield, EDGEDATA& d);
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
	vec3f FaceNormal(FSFace& f, int ntime);

	vec3f NodePosition(const vec3f& r, int ntime);

	// checks if the field code is valid for the given state
	bool IsValidFieldCode(int nfield, int nstate);

public:
	void AddDependant(FEModelDependant* pc);
	void UpdateDependants();
	void RemoveDependant(FEModelDependant* pc);
	void ClearDependants();

public:
	void AddMesh(FEPostMesh* mesh);
	int Meshes() const;
	FEPostMesh* GetFEMesh(int i);

	void UpdateMeshState(int ntime);

public:
	bool Merge(FEPostModel* fem);

public:
	static void SetInstance(FEPostModel* fem);
	static FEPostModel* GetInstance();

	MetaData& GetMetaData() { return m_meta; }

public:
	int PlotObjects() const;
	PlotObject* GetPlotObject(int n);

	int PointObjects() const;
	void AddPointObject(PointObject* ob);
	PointObject* GetPointObject(int i);

	int LineObjects() const;
	void AddLineObject(LineObject* ob);
	LineObject* GetLineObject(int i);

	void ClearObjects();

protected:
	// Helper functions for data evaluation
	void EvalNodeField(int ntime, int nfield);
	void EvalEdgeField(int ntime, int nfield);
	void EvalFaceField(int ntime, int nfield);
	void EvalElemField(int ntime, int nfield);
	
protected:
	string	m_name;		// name (as displayed in model viewer)
	string	m_title;	// title of project

	float	m_fTime;		// current time value
	int		m_nTime;		// active time step

	// --- M E T A   D A T A ---
	MetaData	m_meta;

	// --- M E S H ---
	std::vector<FERefState*>		m_RefState;	// reference state for meshes
	std::vector<FEPostMesh*>		m_mesh;		// the list of meshes
	BOX						m_bbox;		// bounding box of mesh

	// --- M A T E R I A L S ---
	std::vector<Material>	m_Mat;		// array of materials

	// --- O B J E C T S ---
	std::vector<PointObject*>	m_Points;
	std::vector<LineObject *>	m_Lines;

	// --- S T A T E ---
	std::vector<FEState*>	m_State;	// array of pointers to FE-state structures
	FEDataManager*		m_pDM;		// the Data Manager
	int					m_ndisp;	// vector field defining the displacement

	// dependants
	std::vector<FEModelDependant*>	m_Dependants;

	static FEPostModel*	m_pThis;
};
} // namespace Post
