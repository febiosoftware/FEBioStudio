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

#include <MeshLib/FSMesh.h>
#include "Material.h"
#include "FEState.h"
#include "FEDataManager.h"
#include "GLObject.h"
#include <FSCore/box.h>
#include <vector>
#include <memory>

namespace Post {

//! Class for storing metadata information about the model
class MetaData
{
public:
	string	author;		//!< author of model file
	string	software;	//!< software that generated the model file
};

//! Base class for derived classes that need to be informed when the model changes
class FEModelDependant
{
public:
	//! Default constructor
	FEModelDependant() {}
	//! Virtual destructor
	virtual ~FEModelDependant() {}

	//! This function is called whenever the model changes
	//! When the model is deleted it will call this function with 0
	virtual void Update(FEPostModel* pfem) = 0;
};

//! Class that describes an FEPostModel. A model consists of a mesh (in the future
//! there can be multiple meshes to support remeshing), a list of materials
//! and a list of states. The states contain the data associated with the model
class FEPostModel  
{
public:
	//! Base class for plot objects that can be rendered in the scene
	class PlotObject : public CGLObject
	{
	public:
		//! Default constructor
		PlotObject();

		//! Get the color of the plot object
		GLColor	Color();

		//! Set the color of the plot object
		void SetColor(const GLColor& c);

		//! Get the scale factor for the plot object
		double Scale() const;

		//! Find the index of object data with the given name
		int FindObjectDataIndex(const std::string& name) const;

	public:
		//! Unique identifier for the plot object
		int				m_id;
		//! Tag for the plot object
		int				m_tag;
		//! Position of the plot object
		vec3d			m_pos;
		//! Rotation of the plot object
		quatd			m_rot;

		//! Vector of data associated with the plot object
		std::vector<PlotObjectData*>	m_data;
	};

	//! Point object for rendering single points
	class PointObject : public PlotObject
	{
	public:
		//! Default constructor
		PointObject() {}

	public:
		//! Position vector for the point
		vec3d	m_rt;
	};

	//! Line object for rendering line segments
	class LineObject : public PlotObject
	{
	public:
		//! Default constructor
		LineObject() {}

	public:
		//! First endpoint of the line
		vec3d	m_r1;
		//! Second endpoint of the line
		vec3d	m_r2;

		//! Original first endpoint of the line
		vec3d	m_r01;
		//! Original second endpoint of the line
		vec3d	m_r02;
	};

public:
	//! Default constructor
	FEPostModel();
	//! Virtual destructor
	virtual ~FEPostModel();

	// This class is not copyable
	FEPostModel(const FEPostModel&) = delete;
	FEPostModel& operator=(const FEPostModel&) = delete;

	//! Clear all model data
	void Clear();

	//! Delete all meshes from the model
	void DeleteMeshes();

	//! Set the name of the model
	void SetName(const std::string& name);
	//! Get the name of the model
	const string& GetName() const;

	//! Set the title of the model
	void SetTitle(const string& title);
	//! Get the title of the model
	const string& GetTitle() const;

	//! Get the current time value
	float CurrentTime() const { return m_fTime; }
	//! Get the current time index
	int CurrentTimeIndex() const { return m_nTime; }

	//! Get the current state
	FEState* CurrentState();

	//! Set the current time index
	void SetCurrentTimeIndex(int n);

	//! Set the active state closest to time t
	void SetTimeValue(float ftime);

	//! Get the time value of state n
	float GetTimeValue(int ntime);

	//! Get the state closest to time t
	int GetClosestTime(double t);

	//! Get the data manager
	FEDataManager* GetDataManager() { return m_DM.get(); }

	// --- M A T E R I A L S ---

	//! Return number of materials
	int Materials() { return (int) m_Mat.size();  }

	//! Get a particular material
	Material* GetMaterial(int i) { return &m_Mat[i]; }

	//! Clear all materials
	void ClearMaterials() { m_Mat.clear(); }

	//! Add a material to the model
	void AddMaterial(Material& mat);

    //! Enable or disable a material
    void EnableMaterial(int i, bool enable);

	// --- S T A T E   M A N A G M E N T ---
	//! Add a state to the mesh
	void AddState(FEState* pFEState);

	//! Add a new state at time
	void AddState(float ftime, int nstatus = 0, bool interpolateData = false);

	//! Insert a state at a particular time
	void InsertState(FEState* ps, float f);

	//! Remove a state from the mesh
	void DeleteState(int n);

	//! Get the nr of states
	int GetStates() const { return (int) m_State.size(); }

	//! Retrieve pointer to a state
	FEState* GetState(int nstate);

	//! Add a new data field
	void AddDataField(ModelDataField* pd, const std::string& name = "");

	//! Add a new data field constrained to a set
	void AddDataField(ModelDataField* pd, std::vector<int>& L);

	//! Delete a data field
	void DeleteDataField(ModelDataField* pd);

	//! Copy a data field
	ModelDataField* CopyDataField(ModelDataField* pd, const char* sznewname = 0);

	//! Create a cached copy of a data field
	ModelDataField* CreateCachedCopy(ModelDataField* pd, const char* sznewname);

	//! Get the field variable name
	std::string getDataString(int nfield, Data_Tensor_Type ntype);

	//! Interpolate data between its neighbors
	void InterpolateStateData(FEState* ps);

public:
	//! Get the bounding box
	BOX GetBoundingBox() { return m_bbox; }

	//! Update the bounding box
	void UpdateBoundingBox();

	//! Clear all states
	void ClearStates();

	// --- E V A L U A T I O N ---
	//! Evaluate a field at a given time
	bool Evaluate(int nfield, int ntime, bool breset = false);

	//! Update the nodal positions based on the displacement field
	bool EvaluateNodalPosition(int nfield, int ntime);

	//! Get the nodal coordinates of an element at time (returns nr of nodes of element)
	int GetElementCoords(int iel, int ntime, vec3f* r);

	//! Project a point onto the mesh
	int ProjectToMesh(int nstate, const vec3f& r0, vec3d& rt, bool bfollow);

	//! Evaluate scalar functions at a node
	void EvaluateNode   (int n, int ntime, int nfield, NODEDATA& d);
	//! Evaluate scalar functions at an edge
	bool EvaluateEdge   (int n, int ntime, int nfield, EDGEDATA& d);
	//! Evaluate scalar functions at a face
	bool EvaluateFace   (int n, int ntime, int nfield, float* data, float& val);
	//! Evaluate scalar functions at an element
	bool EvaluateElement(int n, int ntime, int nfield, float* data, float& val);

	//! Evaluate based on point
	void EvaluateNode(const vec3f& r, int ntime, int nfield, NODEDATA& d);

	//! Evaluate vector functions at a node
	vec3f EvaluateNodeVector(int n, int ntime, int nvec);
	//! Evaluate vector functions at a face
	bool EvaluateFaceVector(int n, int ntime, int nvec, vec3f& r);
	//! Evaluate vector functions at an element
	vec3f EvaluateElemVector(int n, int ntime, int nvec);

	//! Evaluate tensor functions at a node
	mat3f EvaluateNodeTensor(int n, int ntime, int nten, int ntype = -1);
	//! Evaluate tensor functions at a face
	mat3f EvaluateFaceTensor(int n, int ntime, int nten, int ntype = -1);
	//! Evaluate tensor functions at an element
	mat3f EvaluateElemTensor(int n, int ntime, int nten, int ntype = -1);

	//! Set the displacement field
	void SetDisplacementField(int ndisp) { m_ndisp = ndisp; }
	//! Get the displacement field
	int GetDisplacementField() { return m_ndisp; }
	//! Get the position of a node at a given time
	vec3f NodePosition(int n, int ntime);
	//! Get the normal of a face at a given time
	vec3f FaceNormal(FSFace& f, int ntime);

	//! Get the position of a node based on a reference point at a given time
	vec3f NodePosition(const vec3f& r, int ntime);

	//! Check if the field code is valid for the given state
	bool IsValidFieldCode(int nfield, int nstate);

public:
	//! Add a dependant object that will be notified of model changes
	void AddDependant(FEModelDependant* pc);
	//! Update all dependant objects
	void UpdateDependants();
	//! Remove a dependant object
	void RemoveDependant(FEModelDependant* pc);
	//! Clear all dependant objects
	void ClearDependants();

public:
	//! Add a mesh to the model
	void AddMesh(FSMesh* mesh);
	//! Get the number of meshes in the model
	int Meshes() const;
	//! Get a mesh by index
	FSMesh* GetFEMesh(int i);

    //! Enable or disable mesh items based on material's state
    void UpdateMeshState();
    //! Update mesh state at a specific time
    void UpdateMeshState(int ntime);

    //! Reset all the states so any update will force the state to be evaluated
	void ResetAllStates();

public:
	//! Merge another FEPostModel into this one
	bool Merge(FEPostModel* fem);

public:
	//! Set the active model
	static void SetActiveModel(FEPostModel* fem);

	//! Get the active model
	static FEPostModel* GetActiveModel();

	//! Get the metadata
	MetaData& GetMetaData() { return m_meta; }

public:
	//! Get the number of plot objects
	int PlotObjects() const;
	//! Get a plot object by index
	PlotObject* GetPlotObject(int n);

	//! Get the number of point objects
	int PointObjects() const;
	//! Add a point object
	void AddPointObject(PointObject* ob);
	//! Get a point object by index
	PointObject* GetPointObject(int i);

	//! Get the number of line objects
	int LineObjects() const;
	//! Add a line object
	void AddLineObject(LineObject* ob);
	//! Get a line object by index
	LineObject* GetLineObject(int i);

	//! Clear all plot objects
	void ClearObjects();

protected:
	//! Helper function for evaluating node fields
	void EvalNodeField(int ntime, int nfield);
	//! Helper function for evaluating edge fields
	void EvalEdgeField(int ntime, int nfield);
	//! Helper function for evaluating face fields
	void EvalFaceField(int ntime, int nfield);
	//! Helper function for evaluating element fields
	void EvalElemField(int ntime, int nfield);
	
protected:
	//! Name (as displayed in model viewer)
	string	m_name;		// name (as displayed in model viewer)
	//! Title of project
	string	m_title;	// title of project

	//! Current time value
	float	m_fTime;		// current time value
	//! Active time step
	int		m_nTime;		// active time step

	// --- M E T A   D A T A ---
	//! Metadata for the model
	MetaData	m_meta;

	// --- M E S H ---
	//! Reference state for meshes
	std::vector< std::unique_ptr<FERefState>>	m_RefState;	// reference state for meshes
	//! The list of meshes
	std::vector<std::unique_ptr<FSMesh>>	m_mesh;		// the list of meshes
	//! Bounding box of mesh
	BOX						m_bbox;		// bounding box of mesh

	// --- M A T E R I A L S ---
	//! Array of materials
	std::vector<Material>	m_Mat;		// array of materials

	// --- O B J E C T S ---
	//! Vector of point objects
	std::vector< std::unique_ptr<PointObject> >	m_Points;
	//! Vector of line objects
	std::vector<std::unique_ptr<LineObject> >	m_Lines;

	// --- S T A T E ---
	//! Array of pointers to FE-state structures
	std::vector<std::unique_ptr<FEState>>	m_State;	// array of pointers to FE-state structures

	//! The Data Manager
	std::unique_ptr<FEDataManager>	m_DM;		// the Data Manager
	//! Vector field defining the displacement
	int					m_ndisp;	// vector field defining the displacement

	//! Vector of dependant objects
	std::vector<FEModelDependant*>	m_Dependants;

	//! Only one model can be active at a time
	static FEPostModel*	m_activeModel;
};
} // namespace Post