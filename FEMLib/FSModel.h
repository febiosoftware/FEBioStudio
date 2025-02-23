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

//-----------------------------------------------------------------------------
class GModel;
class FSReactionMaterial;

//-----------------------------------------------------------------------------
//! The FE model stores all FE data.
//
//! It stores the geometry, the material list, and the analysis data.
class FSModel : public FSObject
{
public:
	// constructor/destructor
	FSModel();
	virtual ~FSModel();

	// clear the model
	void Clear();

	// purge the model
	void Purge(int ops);

	// functions to delete all components
	void DeleteAllMaterials();
	void DeleteAllBC();
	void DeleteAllLoads();
	void DeleteAllIC();
	void DeleteAllContact();
	void DeleteAllConstraints();
	void DeleteAllRigidBCs();
	void DeleteAllRigidICs();
	void DeleteAllRigidLoads();
	void DeleteAllRigidConnectors();
	void DeleteAllSteps();
	void DeleteAllLoadControllers();
	void DeleteAllMeshDataGenerators();
	void DeleteAllMeshData();
	void DeleteAllMeshAdaptors();

	void RemoveUnusedLoadControllers();

	// clear the selections of all the bc, loads, etc.
	void ClearSelections();

	// reset model data
	void New();

	// count the mesh data fields (includes mesh data fields stored on the mesh 
	// and the mesh data generators)
	int CountMeshDataFields();

	// return the model geometry
	GModel& GetModel() { return *m_pModel; }

	// --- material functions ---

	// return the material
	GMaterial* GetMaterial(int n);

	// add a material to the model
	void AddMaterial(GMaterial* pmat);

	// replace a material in the model
	void ReplaceMaterial(GMaterial* pold, GMaterial* pnew);

	// delete a material from the model
	bool CanDeleteMaterial(GMaterial* pm);
	int DeleteMaterial(GMaterial* pm);
	void InsertMaterial(int n, GMaterial* pm);

	// return materials
	int Materials();

	// find a material from its ID
	GMaterial* GetMaterialFromID(int id);

	// find a material from its name
	GMaterial* FindMaterial(const char* szname);

	// find a rigid connector from its ID
	FSRigidConnector* GetRigidConnectorFromID(int id);

	// assign material to object
	void AssignMaterial(GObject* po, GMaterial* mat);

	// assign material to part
	void AssignMaterial(GPart* pg, GMaterial* mat);

	// update materials' part list (using parts' material IDs)
	void UpdateMaterialSelections();

	// update part mat IDs from the material selections
	void UpdateMaterialAssignments();

	// --- serialization ---

	// load FE data from the archive
	void Load(IArchive& ar);

	// save the FE data to the archive
	void Save(OArchive& ar);

	// --- Analysis steps ---
	int Steps();
	FSStep* GetStep(int i);
	FSStep* FindStep(int nid);
	int GetStepIndex(FSStep* ps);

	void AddStep(FSStep* ps);
	int DeleteStep(FSStep* ps);
	void InsertStep(int n, FSStep* ps);
	void SwapSteps(FSStep* ps0, FSStep* ps1);

	//! replaces step i with newStep. 
	//! Returns pointer to old step
	FSStep* ReplaceStep(int i, FSStep* newStep);

	void AssignComponentToStep(FSStepComponent* pc, FSStep* ps);

	// --- data variables ---
	int DataVariables();
	FEDataVariable* DataVariable(int i);
	FEDataVariable* FindDataVariable(int nid);
	void AddDataVariable(FEDataVariable* pv);
	// Update model data
	void UpdateData();

	// --- miscellaneous ---
	SoluteData& GetSoluteData(int i);
	int Solutes();
	int FindSolute(const char* sz);
	void AddSolute(const std::string& name, int z, double M, double d);
	void RemoveSolute(int n);
	void ClearSolutes();

	void GetSoluteNames(char* szbuf);
	void GetSBMNames(char* szbuf);
	void GetSpeciesNames(char* szbuf);
	void GetRigidMaterialNames(char* szbuf);
	void GetDOFNames(FEDOFVariable& var, char* szbuf);
	void GetDOFNames(FEDOFVariable& var, std::vector<std::string>& dofList);
	void GetDOFSymbols(FEDOFVariable& var, std::vector<std::string>& dofList);
	void GetVariableNames(const char* szvar, char* szbuf);
	
	const char* GetVariableName(const char* szvar, int n, bool longName = true);

public:
	// These functions deal with enums
	// Enums are identified via:
	// - Key  : The string name of an option (there is a long and short key)
	// - Value: The numeric value of the option
	// - Index: The 0-based index into the array of options
	const char* GetEnumKey(const Param& param, bool longName = true);
	int GetEnumValue(const Param& param);
	int GetEnumIndex(const Param& param);
	bool GetEnumValues(char* szbuf, std::vector<int>& l, const char* szenum);
	bool SetEnumIndex(Param& param, int index);
	bool SetEnumValue(Param& param, int nvalue);
	bool SetEnumKey(Param& param, const std::string& key);

public:
	SoluteData& GetSBMData(int i);
	int SBMs();
	int FindSBM(const char* sz);
	void AddSBM(const std::string& name, int z, double M, double d);
	void RemoveSBM(int n);
	void ClearSBMs();
    
    int Reactions();
    FSReactionMaterial* GetReaction(int id);

	// find (and assign) the group's parent
	bool FindGroupParent(FSGroup* pg);

public:
	int Variables() const { return (int)m_DOF.size(); }
	FEDOFVariable& Variable(int i) { return m_DOF[i]; }
	FEDOFVariable* AddVariable(const std::string& varName);
	int GetVariableIndex(const char* sz);
	FEDOFVariable& GetVariable(const char* sz);
    int GetDOFIndex(const char* sz);
	void ClearVariables();
	const char* GetDOFSymbol(int n) const;
	const char* GetDOFName(int n) const;

public:
	int LoadControllers() const;
	FSLoadController* GetLoadController(int i);
	void AddLoadController(FSLoadController* plc);
	int RemoveLoadController(FSLoadController* plc);

	// helper function for creating load curves
	FSLoadController* AddLoadCurve(LoadCurve& lc);

	FSLoadController* GetLoadControllerFromID(int lc);

	void UpdateLoadControllerReferenceCounts();

public:
	int MeshDataGenerators() const;
	FSMeshDataGenerator* GetMeshDataGenerator(int i);
	void AddMeshDataGenerator(FSMeshDataGenerator* plc);
	int RemoveMeshDataGenerator(FSMeshDataGenerator* plc);

public:
	int CountBCs(int type);
	int CountLoads(int type);
	int CountICs(int type);
	int CountInterfaces(int type);
	int CountRigidConstraints(int type);
	int CountRigidConnectors(int type);

    void SetSkipGeometry(bool skip);

protected:
	// I/O helper functions
	void LoadData              (IArchive& ar);
	void LoadSoluteData        (IArchive& ar);
	void LoadSBMData           (IArchive& ar);
	void LoadMaterials         (IArchive& ar);
	void LoadSteps             (IArchive& ar);
	void LoadLoadControllers   (IArchive& ar);
	void LoadMeshDataGenerators(IArchive& ar);

protected:
	void BuildMLT();
	void ClearMLT();

protected:
	GModel*					m_pModel;	//!< Model geometry
	std::vector<FEDOFVariable>	m_DOF;		//!< degree of freedom list

	FSObjectList<GMaterial>			m_pMat;		//!< Material list
	FSObjectList<FSStep>			m_pStep;	//!< Analysis data
	FSObjectList<FEDataVariable>	m_Var;		//!< data variables
	FSObjectList<SoluteData>		m_Sol;		//!< solute data variables
	FSObjectList<SoluteData>		m_SBM;		//!< solid-bound molecule data variables
	FSObjectList<FSLoadController>		m_LC;		//!< load controllers
	FSObjectList<FSMeshDataGenerator>	m_MD;		//!< mesh data generators

	std::vector<GMaterial*>	m_MLT;	// material look-up table
	int m_MLT_offset;

	bool m_skipGeometry; //!< Skip geometry section when loading file
};

//-----------------------------------------------------------------------------
// helper function for identifying the number of interfaces of a specific type that have been defined.
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
// helper function for identifying the number of constraints of a specific type that have been defined.
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
// helper function for identifying the number of BCs of a specific type that have been defined.
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
int CountBCsByTypeString(const std::string& typeStr, FSModel& fem);

//-----------------------------------------------------------------------------
// helper function for identifying the number of BCs of a specific type that have been defined.
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
// helper function for identifying the number of BCs of a specific type that have been defined.
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
// helper function for identifying the number of rigid constraints of a specific type that have been defined.
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
// helper function for identifying the number of BCs of a specific type that have been defined.
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
// helper function for identifying the number of rigid constraints of a specific type that have been defined.
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
// helper function for identifying the number of rigid constraints of a specific type that have been defined.
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
// helper function for identifying the number of rigid constraints of a specific type that have been defined.
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
// helper function for identifying the number of mesh adaptors.
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

// helper function for creating a valid name from a string.
std::string Namify(const char* sz);

// functions for creating default names
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
