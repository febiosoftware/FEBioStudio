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
#include "MeshLib/FSMesh.h"
#include "FEInterface.h"
#include "FEConnector.h"
#include <FSCore/FSObject.h>
#include "FEBoundaryCondition.h"
#include "FEAnalysisStep.h"
#include "FEMeshAdaptor.h"
#include "FELoadController.h"
#include "FEMeshDataGenerator.h"
#include "GMaterial.h"
#include "FEDataVariable.h"
#include "FESoluteData.h"
#include "FEDOF.h"
#include "FEInitialCondition.h"
#include <FSCore/FSObjectList.h>
#include <unordered_set>
#include <memory>

class GModel;
class FSReactionMaterial;

//! The FE model stores all FE data.
//
//! It stores the geometry, the material list, and the analysis data.
class FSModel : public FSObject
{
public:
	//! Constructor
	FSModel();
	//! Destructor
	virtual ~FSModel();

	FSModel(const FSModel&) = delete;
	FSModel& operator = (const FSModel&) = delete;

public:
	//! Clear the model
	void Clear();

	// purge the model
	void Purge();

	//! Delete all materials from the model
	void DeleteAllMaterials();
	//! Delete all boundary conditions from the model
	void DeleteAllBC();
	//! Delete all loads from the model
	void DeleteAllLoads();
	//! Delete all initial conditions from the model
	void DeleteAllIC();
	//! Delete all contact interfaces from the model
	void DeleteAllContact();
	//! Delete all constraints from the model
	void DeleteAllConstraints();
	//! Delete all rigid boundary conditions from the model
	void DeleteAllRigidBCs();
	//! Delete all rigid initial conditions from the model
	void DeleteAllRigidICs();
	//! Delete all rigid loads from the model
	void DeleteAllRigidLoads();
	//! Delete all rigid connectors from the model
	void DeleteAllRigidConnectors();
	//! Delete all analysis steps from the model
	void DeleteAllSteps();
	//! Delete all load controllers from the model
	void DeleteAllLoadControllers();
	//! Delete all mesh data generators from the model
	void DeleteAllMeshDataGenerators();
	//! Delete all mesh data from the model
	void DeleteAllMeshData();
	//! Delete all mesh adaptors from the model
	void DeleteAllMeshAdaptors();

	//! Remove unused load controllers from the model
	void RemoveUnusedLoadControllers();

	//! removed unused materials
	void RemoveUnusedMaterials();

	//! Clear the selections of all the bc, loads, etc.
	void ClearSelections();

	//! remove all unused items
	void RemoveUnusedItems();

	//! Reset model data
	void Reset();

	//! Count the mesh data fields (includes mesh data fields stored on the mesh and the mesh data generators)
	int CountMeshDataFields();

	//! Return the model geometry
	GModel& GetModel() { return *(m_GMdl.get()); }

	// --- material functions ---

	//! Return the material at the specified index
	GMaterial* GetMaterial(int n);

	//! Add a material to the model
	void AddMaterial(GMaterial* pmat);

	//! Replace a material in the model
	void ReplaceMaterial(GMaterial* pold, GMaterial* pnew);

	//! Check if a material can be deleted from the model
	bool CanDeleteMaterial(GMaterial* pm);
	//! Delete a material from the model
	int DeleteMaterial(GMaterial* pm);
	//! Insert a material at the specified position
	void InsertMaterial(int n, GMaterial* pm);

	//! Return the number of materials
	int Materials();

	//! Find a material from its ID
	GMaterial* GetMaterialFromID(int id);

	// find a material from its name
	GMaterial* FindMaterial(const std::string& name);

	//! Assign material to object
	void AssignMaterial(GObject* po, GMaterial* mat);

	//! Assign material to part
	void AssignMaterial(GPart* pg, GMaterial* mat);
	//! Assign material to multiple parts
	void AssignMaterial(const std::vector<GPart*>& partList, GMaterial* mat);
	//! Assign materials to parts using lists
	void AssignMaterial(const std::vector<GPart*>& partList, const std::vector<GMaterial*>& matList);

	//! Get list of parts associated with a material
	std::vector<GPart*> GetMaterialPartList(GMaterial* mat);

	//! Update material positions
	void UpdateMaterialPositions();

	// --- rigid connectors ---
	//! Find a rigid connector from its ID
	FSRigidConnector* GetRigidConnectorFromID(int id);

	// --- serialization ---

	//! Load FE data from the archive
	void Load(IArchive& ar);

	//! Save the FE data to the archive
	void Save(OArchive& ar);

	// --- Analysis steps ---
	//! Return the number of analysis steps
	int Steps();
	//! Get the analysis step at the specified index
	FSStep* GetStep(int i);
	//! Find an analysis step by ID
	FSStep* FindStep(int nid);
	//! Get the index of the specified analysis step
	int GetStepIndex(FSStep* ps);

	//! Add an analysis step to the model
	void AddStep(FSStep* ps);
	//! Delete an analysis step from the model
	int DeleteStep(FSStep* ps);
	//! Insert an analysis step at the specified position
	void InsertStep(int n, FSStep* ps);
	//! Swap two analysis steps
	void SwapSteps(FSStep* ps0, FSStep* ps1);

	//! Replace step i with newStep. Returns pointer to old step
	FSStep* ReplaceStep(int i, FSStep* newStep);

	//! Assign a component to an analysis step
	void AssignComponentToStep(FSStepComponent* pc, FSStep* ps);

	// --- data variables ---
	//! Return the number of data variables
	int DataVariables();
	//! Get the data variable at the specified index
	FEDataVariable* DataVariable(int i);
	//! Find a data variable by ID
	FEDataVariable* FindDataVariable(int nid);
	//! Add a data variable to the model
	void AddDataVariable(FEDataVariable* pv);
	//! Update model data
	void UpdateData();

	// --- miscellaneous ---
	//! Get solute data at the specified index
	SoluteData& GetSoluteData(int i);
	//! Return the number of solutes
	int Solutes();
	//! Find a solute by name
	int FindSolute(const char* sz);
	//! Add a solute to the model
	void AddSolute(const std::string& name, int z, double M, double d);
	//! Remove a solute from the model
	void RemoveSolute(int n);
	//! Clear all solutes from the model
	void ClearSolutes();

	//! Get solute names as a string buffer
	void GetSoluteNames(char* szbuf);
	//! Get solid-bound molecule names as a string buffer
	void GetSBMNames(char* szbuf);
	//! Get species names as a string buffer
	void GetSpeciesNames(char* szbuf);
	//! Get rigid material names as a string buffer
	void GetRigidMaterialNames(char* szbuf);
	//! Get DOF names for a variable as a string buffer
	void GetDOFNames(FEDOFVariable& var, char* szbuf);
	//! Get DOF names for a variable as a vector of strings
	void GetDOFNames(FEDOFVariable& var, std::vector<std::string>& dofList);
	//! Get DOF symbols for a variable as a vector of strings
	void GetDOFSymbols(FEDOFVariable& var, std::vector<std::string>& dofList);
	//! Get variable names as a string buffer
	void GetVariableNames(const char* szvar, char* szbuf);
	
	//! Get the name of a variable
	const char* GetVariableName(const char* szvar, int n, bool longName = true);

public:
	// These functions deal with enums
	// Enums are identified via:
	// - Key  : The string name of an option (there is a long and short key)
	// - Value: The numeric value of the option
	// - Index: The 0-based index into the array of options
	//! Get the key for an enum parameter
	const char* GetEnumKey(const Param& param, bool longName = true);
	//! Get the value for an enum parameter
	int GetEnumValue(const Param& param);
	//! Get the index for an enum parameter
	int GetEnumIndex(const Param& param);
	//! Get enum values as a string buffer and list
	bool GetEnumValues(char* szbuf, std::vector<int>& l, const char* szenum);
	//! Set enum parameter by index
	bool SetEnumIndex(Param& param, int index);
	//! Set enum parameter by value
	bool SetEnumValue(Param& param, int nvalue);
	//! Set enum parameter by key
	bool SetEnumKey(Param& param, const std::string& key);

public:
	//! Get solid-bound molecule data at the specified index
	SoluteData& GetSBMData(int i);
	//! Return the number of solid-bound molecules
	int SBMs();
	//! Find a solid-bound molecule by name
	int FindSBM(const char* sz);
	//! Add a solid-bound molecule to the model
	void AddSBM(const std::string& name, int z, double M, double d);
	//! Remove a solid-bound molecule from the model
	void RemoveSBM(int n);
	//! Clear all solid-bound molecules from the model
	void ClearSBMs();
    
    //! Return the number of reactions
    int Reactions();
    //! Get a reaction by ID
    FSReactionMaterial* GetReaction(int id);

	//! Find (and assign) the group's parent
	bool FindGroupParent(FSGroup* pg);

public:
	//! Return the number of variables
	int Variables() const { return (int)m_DOF.size(); }
	//! Get the variable at the specified index
	FEDOFVariable& Variable(int i) { return m_DOF[i]; }
	//! Add a variable to the model
	FEDOFVariable* AddVariable(const std::string& varName);
	//! Get the index of a variable by name
	int GetVariableIndex(const char* sz);
	//! Get a variable by name
	FEDOFVariable& GetVariable(const char* sz);
    //! Get the DOF index by name
	int GetDOFIndex(const char* sz);
	//! Clear all variables from the model
	void ClearVariables();
	//! Get the DOF symbol at the specified index
	const char* GetDOFSymbol(int n) const;
	//! Get the DOF name at the specified index
	const char* GetDOFName(int n) const;

public:
	//! Return the number of load controllers
	int LoadControllers() const;
	//! Get the load controller at the specified index
	FSLoadController* GetLoadController(int i);
	//! Add a load controller to the model
	void AddLoadController(FSLoadController* plc);
	//! Remove a load controller from the model
	int RemoveLoadController(FSLoadController* plc);

	//! Helper function for creating load curves
	FSLoadController* AddLoadCurve(LoadCurve& lc);

	//! Get a load controller by ID
	FSLoadController* GetLoadControllerFromID(int lc);

	//! Update load controller reference counts
	void UpdateLoadControllerReferenceCounts();

public:
	//! Return the number of mesh data generators
	int MeshDataGenerators() const;
	//! Get the mesh data generator at the specified index
	FSMeshDataGenerator* GetMeshDataGenerator(int i);
	//! Add a mesh data generator to the model
	void AddMeshDataGenerator(FSMeshDataGenerator* plc);
	//! Remove a mesh data generator from the model
	int RemoveMeshDataGenerator(FSMeshDataGenerator* plc);

public:
	//! Count boundary conditions of a specific type
	int CountBCs(int type);
	//! Count loads of a specific type
	int CountLoads(int type);
	//! Count initial conditions of a specific type
	int CountICs(int type);
	//! Count interfaces of a specific type
	int CountInterfaces(int type);
	//! Count rigid constraints of a specific type
	int CountRigidConstraints(int type);
	//! Count rigid connectors of a specific type
	int CountRigidConnectors(int type);

    //! Set whether to skip geometry during loading
    void SetSkipGeometry(bool skip);

    //! Get allocator IDs of all in-use plugins
    void GetActivePluginIDs(std::unordered_set<int>& allocatorIDs);

protected:
	// I/O helper functions
	//! Load data from archive
	void LoadData              (IArchive& ar);
	//! Load solute data from archive
	void LoadSoluteData        (IArchive& ar);
	//! Load solid-bound molecule data from archive
	void LoadSBMData           (IArchive& ar);
	//! Load materials from archive
	void LoadMaterials         (IArchive& ar);
	//! Load analysis steps from archive
	void LoadSteps             (IArchive& ar);
	//! Load load controllers from archive
	void LoadLoadControllers   (IArchive& ar);
	//! Load mesh data generators from archive
	void LoadMeshDataGenerators(IArchive& ar);

protected:
	//! Build material lookup table
	void BuildMLT();
	//! Clear material lookup table
	void ClearMLT();

protected:
	//! Model geometry
	std::unique_ptr<GModel>	m_GMdl;

	//! Degree of freedom list
	std::vector<FEDOFVariable>	m_DOF;

	//! Material list
	FSObjectList<GMaterial>			m_pMat;
	//! Analysis data
	FSObjectList<FSStep>			m_pStep;
	//! Data variables
	FSObjectList<FEDataVariable>	m_Var;
	//! Solute data variables
	FSObjectList<SoluteData>		m_Sol;
	//! Solid-bound molecule data variables
	FSObjectList<SoluteData>		m_SBM;
	//! Load controllers
	FSObjectList<FSLoadController>		m_LC;
	//! Mesh data generators
	FSObjectList<FSMeshDataGenerator>	m_MD;

	//! Material look-up table
	std::vector<GMaterial*>	m_MLT;
	//! Material lookup table offset
	int m_MLT_offset;

	//! Skip geometry section when loading file
	bool m_skipGeometry;
};

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of interfaces of a specific type that have been defined.
template <class T> int CountInterfaces(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->Interfaces(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->Interface(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of constraints of a specific type that have been defined.
template <class T> int CountConstraints(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->Constraints(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->Constraint(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of BCs of a specific type that have been defined.
template <class T> int CountBCs(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->BCs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->BC(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Count boundary conditions by type string
int CountBCsByTypeString(const std::string& typeStr, FSModel& fem);

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of ICs of a specific type that have been defined.
template <class T> int CountICs(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->ICs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->IC(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of loads of a specific type that have been defined.
template <class T> int CountLoads(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->Loads(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->Load(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of rigid constraints of a specific type that have been defined.
template <class T> int CountRigidConstraints(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j < ps->RigidConstraints(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidConstraint(j));
			if (pbc) nc++;
		}
	}
	return nc;
}


//-----------------------------------------------------------------------------
//! Helper function for identifying the number of connectors of a specific type that have been defined.
template <class T> int CountConnectors(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->RigidConnectors(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidConnector(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of rigid boundary conditions of a specific type that have been defined.
template <class T> int CountRigidBCs(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->RigidBCs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidBC(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of rigid initial conditions of a specific type that have been defined.
template <class T> int CountRigidICs(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j < ps->RigidICs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidIC(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of rigid loads of a specific type that have been defined.
template <class T> int CountRigidLoads(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j < ps->RigidLoads(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidLoad(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
//! Helper function for identifying the number of mesh adaptors of a specific type that have been defined.
template <class T> int CountMeshAdaptors(FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j < ps->MeshAdaptors(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->MeshAdaptor(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//! Helper function for creating a valid name from a string.
std::string Namify(const char* sz);

//! functions for creating default names
std::string defaultBCName(FSModel* fem, FSBoundaryCondition* pbc);
std::string defaultICName(FSModel* fem, FSInitialCondition* pic);
std::string defaultLoadName(FSModel* fem, FSLoad* pbc);
std::string defaultInterfaceName(FSModel* fem, FSInterface* pi);
std::string defaultConstraintName(FSModel* fem, FSModelConstraint* pi);
std::string defaultRigidConnectorName(FSModel* fem, FSRigidConnector* pc);
std::string defaultRigidBCName(FSModel* fem, FSRigidBC* pc);
std::string defaultRigidICName(FSModel* fem, FSRigidIC* pc);
std::string defaultRigidLoadName(FSModel* fem, FSRigidLoad* pc);
std::string defaultMeshAdaptorName(FSModel* fem, FSMeshAdaptor* pc);
std::string defaultStepName(FSModel* fem, FSStep* ps);