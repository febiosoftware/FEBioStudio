// FEModel.h: interface for the FEModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEMODEL_H__8CCBD4A4_79CC_44DB_9DE9_8ED16E0EEAA8__INCLUDED_)
#define AFX_FEMODEL_H__8CCBD4A4_79CC_44DB_9DE9_8ED16E0EEAA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MeshLib/FEMesh.h"
#include "FEMLib/FEInterface.h"
#include "FEMLib/FEConnector.h"
#include "LoadCurve.h"
#include "Serializable.h"
#include "FEMLib/FECoreModel.h"
#include "GModel.h"
#include "FEMLib/FEBoundaryCondition.h"
#include "FEMLib/FEAnalysisStep.h"
#include "GMaterial.h"
#include "FEDataVariable.h"
#include "FESoluteData.h"
#include "FEMLib/FEDOF.h"
#include "FEMLib/FEInitialCondition.h"

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
	void DeleteAllConnectors();
	void DeleteAllSteps();

	// clear the selections of all the bc, loads, etc.
	void ClearSelections();

	// reset model data
	void New();

	// return the model geometry
	GModel& GetModel() { return *m_pModel; }

	// --- material functions ---

	// return the material
	GMaterial* GetMaterial(int n) { return (n<0||n>=(int) m_pMat.size()?0:m_pMat[n]); }

	// add a material to the model
	void AddMaterial(GMaterial* pmat) { m_pMat.push_back(pmat); pmat->SetModel(this); } 

	// replace a material in the model
	void ReplaceMaterial(GMaterial* pold, GMaterial* pnew);

	// delete a material from the model
	bool CanDeleteMaterial(GMaterial* pm);
	int DeleteMaterial(GMaterial* pm);
	void InsertMaterial(int n, GMaterial* pm) {m_pMat.insert(m_pMat.begin()+n, pm);}

	// return materials
	int Materials() { return (int)m_pMat.size(); }

	// find a material from its ID
	GMaterial* GetMaterialFromID(int id);

	// find a material from its name
	GMaterial* FindMaterial(const char* szname);

	// --- serialization ---

	// load FE data from the archive
	void Load(IArchive& ar);

	// save the FE data to the archive
	void Save(OArchive& ar);

	// --- Analysis steps ---
	int Steps() { return (int)m_pStep.size(); }
	FEStep* GetStep(int i) { return m_pStep[i]; }
	FEStep* FindStep(int nid);

	void AddStep(FEStep* ps) { m_pStep.push_back(ps); }
	int DeleteStep(FEStep* ps);
	void InsertStep(int n, FEStep* ps) {m_pStep.insert(m_pStep.begin()+n, ps);}

	void AssignBCToStep(FEBoundaryCondition* pbc, FEStep* ps);
	void AssignLoadToStep(FEBoundaryCondition* pbc, FEStep* ps);
	void AssignICToStep(FEInitialCondition* pic, FEStep* ps);
	void AssignInterfaceToStep(FEInterface* pi, FEStep* ps);
	void AssignConstraintToStep(FERigidConstraint* pc, FEStep* ps);
    void AssignConnectorToStep(FEConnector* pi, FEStep* ps);

	void AssignComponentToStep(FEStepComponent* pc, FEStep* ps);

	// --- data variables ---
	int DataVariables() { return (int) m_Var.size(); }
	FEDataVariable* DataVariable(int i) { return m_Var[i]; }
	FEDataVariable* FindDataVariable(int nid);
	void AddDataVariable(FEDataVariable* pv) { m_Var.push_back(pv); }
	// Update model data
	void UpdateData();

	// --- miscellaneous ---
	FESoluteData& GetSoluteData(int i) { return *m_Sol[i]; }
	int Solutes() { return (int) m_Sol.size(); }
	int FindSolute(const char* sz);
	void AddSolute(const std::string& name, int z, double M, double d);
	void RemoveSolute(int n);
	void ClearSolutes();

	void GetSoluteNames(char* szbuf);
	void GetSBMNames(char* szbuf);
	void GetRigidMaterialNames(char* szbuf);
	void GetDOFNames(FEDOFVariable& var, char* szbuf);
	void GetVariableNames(const char* szvar, char* szbuf);

	FESoluteData& GetSBMData(int i) { return *m_SBM[i]; }
	int SBMs() { return (int) m_SBM.size(); }
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
	void AddDataMap(FEDataMap* map);
	bool RemoveMap(FEDataMap* map);
	FEDataMap* GetDataMap(int i);

public:
	int Variables() const { return (int)m_DOF.size(); }
	FEDOFVariable& Variable(int i) { return m_DOF[i]; }
	FEDOFVariable* AddVariable(const char* szvar);
	int GetVariableIndex(const char* sz);
	FEDOFVariable& GetVariable(const char* sz);

public:
	int CountBCs(int type);
	int CountLoads(int type);
	int CountICs(int type);
	int CountInterfaces(int type);
	int CountConstraints(int type);
	int CountConnectors(int type);

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
	vector<GMaterial*>		m_pMat;		//!< Material list
	vector<FEStep*>			m_pStep;	//!< Analysis data
	vector<FEDataVariable*>	m_Var;		//!< data variables
	vector<FESoluteData*>	m_Sol;		//!< solute data variables
	vector<FESoluteData*>	m_SBM;		//!< solid-bound molecule data variables

	vector<FEDataMap*>		m_Map;		//!< data maps
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
		for (int j = 0; j<ps->Connectors(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->Connector(j));
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
		for (int j = 0; j<ps->RCs(); ++j)
		{
			T* pbc = dynamic_cast<T*>(ps->RC(j));
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
std::string defaultLoadName(FEModel* fem, FEBoundaryCondition* pbc);
std::string defaultInterfaceName(FEModel* fem, FEInterface* pi);
std::string defaultConnectorName(FEModel* fem, FEConnector* pc);
std::string defaultConstraintName(FEModel* fem, FERigidConstraint* pc);
std::string defaultStepName(FEModel* fem, FEAnalysisStep* ps);

#endif // !defined(AFX_FEMODEL_H__8CCBD4A4_79CC_44DB_9DE9_8ED16E0EEAA8__INCLUDED_)
