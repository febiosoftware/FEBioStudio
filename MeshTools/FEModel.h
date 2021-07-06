/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include "MeshLib/FEMesh.h"
#include "FEMLib/FEInterface.h"
#include "FEMLib/FEConnector.h"
#include "FEMLib/FECoreModel.h"
#include "FEMLib/FEBoundaryCondition.h"
#include "FEMLib/FEAnalysisStep.h"
#include "GMaterial.h"
#include "FEDataVariable.h"
#include "FESoluteData.h"
#include "FEMLib/FEDOF.h"
#include "FEMLib/FEInitialCondition.h"
#include <FSCore/FSObjectList.h>

//-----------------------------------------------------------------------------
// For convenience, we define the following variable IDs
// TODO: It looks like only FE_VAR_CONCENTRATION is used. Maybe see if this can be eliminated.
#define FE_VAR_DISPLACEMENT		0
#define FE_VAR_ROTATION			1
#define FE_VAR_FLUID_PRESSURE	2
#define FE_VAR_TEMPERATURE		3
#define FE_VAR_CONCENTRATION	4
#define FE_VAR_VELOCITY			5
#define FE_VAR_FLUID_DILATAION	6

//-----------------------------------------------------------------------------
class GModel;
class FEReactionMaterial;

//-----------------------------------------------------------------------------
//! The FE model stores all FE data.
//
//! It stores the geometry, the material list, and the analysis data.
class FEModel : public FECoreModel 
{
public:
	// constructor/destructor
	FEModel();
	virtual ~FEModel();

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
	void DeleteAllRigidConstraints();
	void DeleteAllRigidConnectors();
	void DeleteAllSteps();

	// clear the selections of all the bc, loads, etc.
	void ClearSelections();

	// reset model data
	void New();

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
    FERigidConnector* GetRigidConnectorFromID(int id);

	// --- serialization ---

	// load FE data from the archive
	void Load(IArchive& ar);

	// save the FE data to the archive
	void Save(OArchive& ar);

	// --- Analysis steps ---
	int Steps();
	FEStep* GetStep(int i);
	FEStep* FindStep(int nid);
	int GetStepIndex(FEStep* ps);

	void AddStep(FEStep* ps);
	int DeleteStep(FEStep* ps);
	void InsertStep(int n, FEStep* ps);
	void SwapSteps(FEStep* ps0, FEStep* ps1);

	void AssignComponentToStep(FEStepComponent* pc, FEStep* ps);

	// --- data variables ---
	int DataVariables();
	FEDataVariable* DataVariable(int i);
	FEDataVariable* FindDataVariable(int nid);
	void AddDataVariable(FEDataVariable* pv);
	// Update model data
	void UpdateData();

	// --- miscellaneous ---
	FESoluteData& GetSoluteData(int i);
	int Solutes();
	int FindSolute(const char* sz);
	void AddSolute(const std::string& name, int z, double M, double d);
	void RemoveSolute(int n);
	void ClearSolutes();

	void GetSoluteNames(char* szbuf);
	void GetSBMNames(char* szbuf);
	void GetRigidMaterialNames(char* szbuf);
	void GetDOFNames(FEDOFVariable& var, char* szbuf);
	void GetVariableNames(const char* szvar, char* szbuf);
	
	const char* GetVariableName(const char* szvar, int n);
	int GetVariableIntValue(const char* szvar, int n);
	const char* GetEnumValue(const char* szenum, int n);
	int GetEnumIntValue(Param& param);

	FESoluteData& GetSBMData(int i);
	int SBMs();
	int FindSBM(const char* sz);
	void AddSBM(const std::string& name, int z, double M, double d);
	void RemoveSBM(int n);
	void ClearSBMs();
    
    int Reactions();
    FEReactionMaterial* GetReaction(int id);

	// find (and assign) the group's parent
	bool FindGroupParent(FEGroup* pg);

public:
	int DataMaps() const;
	void AddDataMap(FEDataMapGenerator* map);
	int RemoveMap(FEDataMapGenerator* map);
	FEDataMapGenerator* GetDataMap(int i);

public:
	int Variables() const { return (int)m_DOF.size(); }
	FEDOFVariable& Variable(int i) { return m_DOF[i]; }
	FEDOFVariable* AddVariable(const char* szvar);
	int GetVariableIndex(const char* sz);
	FEDOFVariable& GetVariable(const char* sz);
    int GetDOFIndex(const char* sz);

public:
	int CountBCs(int type);
	int CountLoads(int type);
	int CountICs(int type);
	int CountInterfaces(int type);
	int CountRigidConstraints(int type);
	int CountRigidConnectors(int type);

protected:
	// I/O helper functions
	void LoadData      (IArchive& ar);
	void LoadSoluteData(IArchive& ar);
	void LoadSBMData   (IArchive& ar);
	void LoadMaterials (IArchive& ar);
	void LoadSteps     (IArchive& ar);

protected:
	GModel*					m_pModel;	//!< Model geometry
	vector<FEDOFVariable>	m_DOF;		//!< degree of freedom list

	FSObjectList<GMaterial>			m_pMat;		//!< Material list
	FSObjectList<FEStep>			m_pStep;	//!< Analysis data
	FSObjectList<FEDataVariable>	m_Var;		//!< data variables
	FSObjectList<FESoluteData>		m_Sol;		//!< solute data variables
	FSObjectList<FESoluteData>		m_SBM;		//!< solid-bound molecule data variables
	FSObjectList<FEDataMapGenerator>	m_Map;		//!< data maps
};

//-----------------------------------------------------------------------------
// helper function for identifying the number of interfaces of a specific type that have been defined.
template <class T> int CountInterfaces(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
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
template <class T> int CountConstraints(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
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
template <class T> int CountBCs(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->BCs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->BC(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
// helper function for identifying the number of BCs of a specific type that have been defined.
template <class T> int CountICs(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
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
template <class T> int CountLoads(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->Loads(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->Load(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

//-----------------------------------------------------------------------------
// helper function for identifying the number of BCs of a specific type that have been defined.
template <class T> int CountConnectors(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
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
template <class T> int CountRigidConstraints(FEModel& fem)
{
	int nc = 0;
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->RigidConstraints(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RigidConstraint(j));
			if (pbc) nc++;
		}
	}
	return nc;
}

// helper function for creating a valid name from a string.
std::string Namify(const char* sz);

// functions for creating default names
std::string defaultBCName(FEModel* fem, FEBoundaryCondition* pbc);
std::string defaultICName(FEModel* fem, FEInitialCondition* pic);
std::string defaultLoadName(FEModel* fem, FELoad* pbc);
std::string defaultInterfaceName(FEModel* fem, FEInterface* pi);
std::string defaultConstraintName(FEModel* fem, FEModelConstraint* pi);
std::string defaultRigidConnectorName(FEModel* fem, FERigidConnector* pc);
std::string defaultRigidConstraintName(FEModel* fem, FERigidConstraint* pc);
std::string defaultStepName(FEModel* fem, FEStep* ps);
