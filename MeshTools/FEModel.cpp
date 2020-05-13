// FEModel.cpp: implementation of the FEModel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEModel.h"
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEUserMaterial.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <GeomLib/GObject.h>
#include <FSCore/paramunit.h>
#include <FSCore/ParamBlock.h>
#include "GGroup.h"
#include "GModel.h"
#include <vector>
#include <sstream>
#include <algorithm>
using namespace std;

std::string Namify(const char* sz)
{
	char s[128] = { 0 };
	int n = 0;
	const char* c = sz;
	bool cap = true;
	while (*c)
	{
		if (isspace(*c)) cap = true;
		else
		{
			if (cap)
			{
				if (islower(*c)) s[n++] = toupper(*c);
				else s[n++] = *c;
				cap = false;
			}
			else s[n++] = *c;
		}
		++c;
	}

	return s;
}

std::string defaultBCName(FEModel* fem, FEBoundaryCondition* pbc)
{
	const char* ch = pbc->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountBCs(pbc->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultICName(FEModel* fem, FEInitialCondition* pic)
{
	const char* ch = pic->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountICs(pic->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultLoadName(FEModel* fem, FELoad* pbc)
{
	const char* ch = pbc->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountLoads(pbc->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultInterfaceName(FEModel* fem, FEInterface* pi)
{
	const char* ch = pi->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountInterfaces(pi->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultConstraintName(FEModel* fem, FEModelConstraint* pi)
{
	const char* ch = pi->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountConstraints(pi->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultRigidConnectorName(FEModel* fem, FERigidConnector* pi)
{
	int nrc = CountConnectors<FERigidConnector>(*fem);
	stringstream ss;
	ss << "RigidConnector" << nrc + 1;
	return  ss.str();
}

std::string defaultRigidConstraintName(FEModel* fem, FERigidConstraint* pc)
{
	int nrc = CountRigidConstraints<FERigidConstraint>(*fem);
	stringstream ss;
	ss << "RigidConstraint" << nrc + 1;
	return  ss.str();
}

std::string defaultStepName(FEModel* fem, FEAnalysisStep* ps)
{
	int nsteps = fem->Steps();
	stringstream ss;
	ss << "Step" << nsteps;
	return ss.str();
}

//-----------------------------------------------------------------------------
FEModel::FEModel()
{
	m_pModel = new GModel(this);
	New();

	// define degrees of freedom
	m_DOF.clear();

	FEDOFVariable* varDisp = AddVariable("Displacement");
	varDisp->AddDOF("X-displacement", "x");
	varDisp->AddDOF("Y-displacement", "y");
	varDisp->AddDOF("Z-displacement", "z");

	FEDOFVariable* varRot = AddVariable("Rotation");
	varRot->AddDOF("X-rotation", "u");
	varRot->AddDOF("Y-rotation", "v");
	varRot->AddDOF("Z-rotation", "w");

	FEDOFVariable* varPressure = AddVariable("Effective Fluid Pressure");
	varPressure->AddDOF("pressure", "p");

	FEDOFVariable* varTemperature = AddVariable("Temperature");
	varTemperature->AddDOF("temperature", "T");

	FEDOFVariable* varSolute = AddVariable("Effective Solute Concentration");
	// (start with an empty solute variable)

    FEDOFVariable* varVel = AddVariable("Fluid Velocity");
    varVel->AddDOF("X-fluid velocity", "wx");
    varVel->AddDOF("Y-fluid velocity", "wy");
    varVel->AddDOF("Z-fluid velocity", "wz");
    
	FEDOFVariable* varDil = AddVariable("Fluid Dilatation");
	varDil->AddDOF("dilatation", "ef");

	FEDOFVariable* varSDisp = AddVariable("Shell Displacement");
	varSDisp->AddDOF("Shell X-displacement", "sx");
	varSDisp->AddDOF("Shell Y-displacement", "sy");
	varSDisp->AddDOF("Shell Z-displacement", "sz");

	// define model parameters
	AddScienceParam(0, UNIT_TEMPERATURE, "T", "Absolute temperature");
	AddScienceParam(0, UNIT_GAS_CONSTANT, "R", "Gas constant");
	AddScienceParam(0, UNIT_FARADAY_CONSTANT, "Fc", "Faraday's constant");
}

//-----------------------------------------------------------------------------
FEDOFVariable* FEModel::AddVariable(const char* szname)
{
	FEDOFVariable var(szname);
	m_DOF.push_back(var);
	return &m_DOF[m_DOF.size() - 1];
}

//-----------------------------------------------------------------------------
int FEModel::GetVariableIndex(const char* sz)
{
	for (int i=0; i<(int)m_DOF.size(); ++i)
	{
		if (strcmp(m_DOF[i].name(), sz) == 0) return i;
	}
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
FEDOFVariable& FEModel::GetVariable(const char* sz)
{
	int nvar = GetVariableIndex(sz);
	return m_DOF[nvar];
}

//-----------------------------------------------------------------------------
FEModel::~FEModel()
{
	Clear();
	delete m_pModel;
}

//-----------------------------------------------------------------------------
void FEModel::ClearSolutes()
{
	if (m_Sol.IsEmpty() == false)
	{
		m_Sol.Clear(); 
		FEDOFVariable& var = Variable(FE_VAR_CONCENTRATION);
		var.Clear();
	}
}

//-----------------------------------------------------------------------------
void FEModel::AddSolute(const std::string& name, int z, double M, double d)
{
	FESoluteData* s = new FESoluteData;
	s->SetName(name);
	s->SetChargeNumber(z);
	s->SetMolarMass(M);
	s->SetDensity(d);
	m_Sol.Add(s);

	// Also add a degree of freedom for this
	FEDOFVariable& var = Variable(FE_VAR_CONCENTRATION);

	char sz[12] = {0};
	sprintf(sz, "c%d", (int)m_Sol.Size());
	var.AddDOF(name, sz);
}

//-----------------------------------------------------------------------------
void FEModel::GetRigidMaterialNames(char* szbuf)
{
	char* ch = szbuf;
	for (int i = 0; i<Materials(); ++i)
	{
		GMaterial* pm = GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
		{
			const char* szi = pm->GetName().c_str();
			strcat(ch, szi);
			ch += strlen(szi);
			*ch++ = '\0';
		}
	}
}

//-----------------------------------------------------------------------------
void FEModel::GetVariableNames(const char* szvar, char* szbuf)
{
	char var[256] = {0};
	const char* chl = strchr(szvar, '('); assert(chl);
	const char* chr = strchr(szvar, ')'); assert(chr);
	strncpy(var, chl+1, chr-chl-1);

	if (strcmp(var, "Solutes") == 0) { GetSoluteNames(szbuf); return; }
	else if (strcmp(var, "SBMs") == 0) { GetSBMNames(szbuf); return; }
	else
	{
		int NVAR = Variables();
		for (int i=0; i<NVAR; ++i)
		{
			FEDOFVariable& v = Variable(i);
			if (strcmp(v.name(), var) == 0) { GetDOFNames(v, szbuf); return; }
		}
	}
	assert(false);
}

//-----------------------------------------------------------------------------
void FEModel::GetDOFNames(FEDOFVariable& var, char* szbuf)
{
	char* ch = szbuf;
	for (int i = 0; i<var.DOFs(); ++i)
	{
		const char* szi = var.GetDOF(i).name();
		strcat(ch, szi);
		ch += strlen(szi);
		*ch++ = '\0';
	}
}

//-----------------------------------------------------------------------------
void FEModel::GetSoluteNames(char* szbuf)
{
	char* ch = szbuf;
	for (int i=0; i<(int)m_Sol.Size(); ++i)
	{
		const char* szi = m_Sol[i]->GetName().c_str();
		strcat(ch, szi);
		ch += strlen(szi);
		*ch++ = '\0';
	}
}

//-----------------------------------------------------------------------------
void FEModel::GetSBMNames(char* szbuf)
{
	char* ch = szbuf;
	for (int i = 0; i<(int)m_SBM.Size(); ++i)
	{
		const char* szi = m_SBM[i]->GetName().c_str();
		strcat(ch, szi);
		ch += strlen(szi);
		*ch++ = '\0';
	}
}

//-----------------------------------------------------------------------------
int FEModel::FindSolute(const char* sz)
{
	string sol(sz);
	for (int i=0; i<m_Sol.Size(); ++i)
	{
		if (m_Sol[i]->GetName() == sol) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
FESoluteData& FEModel::GetSoluteData(int i)
{ 
	return *m_Sol[i]; 
}

//-----------------------------------------------------------------------------
int FEModel::Solutes()
{ 
	return (int)m_Sol.Size(); 
}

//-----------------------------------------------------------------------------
void FEModel::RemoveSolute(int n)
{
	delete m_Sol[n];

	// Also remove degree of freedom for this
	FEDOFVariable& var = Variable(FE_VAR_CONCENTRATION);
	var.RemoveDOF(n);

	// redefine the symbols
	char sz[12] = {0};
	for (int i=0; i<var.DOFs(); ++i)
	{
		sprintf(sz, "c%d", i + 1);
		var.GetDOF(i).SetSymbol(sz);
	}
}

//-----------------------------------------------------------------------------
int FEModel::FindSBM(const char* sz)
{
	string sbm(sz);
	for (int i = 0; i<m_SBM.Size(); ++i)
	{
		if (m_SBM[i]->GetName() == sbm) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
FESoluteData& FEModel::GetSBMData(int i)
{ 
	return *m_SBM[i]; 
}

//-----------------------------------------------------------------------------
int FEModel::SBMs()
{ 
	return (int)m_SBM.Size(); 
}

//-----------------------------------------------------------------------------
void FEModel::AddSBM(const std::string& name, int z, double M, double d)
{
	FESoluteData* s = new FESoluteData;
	s->SetName(name);
	s->SetChargeNumber(z);
	s->SetMolarMass(M);
	s->SetDensity(d);
	m_SBM.Add(s);
}

//-----------------------------------------------------------------------------
void FEModel::ClearSBMs()
{
	m_SBM.Clear();
}

//-----------------------------------------------------------------------------
void FEModel::RemoveSBM(int n)
{
	delete m_SBM[n];
}

//-----------------------------------------------------------------------------
int FEModel::Reactions()
{
    int n = 0;
	for (int i=0; i<(int) m_pMat.Size(); ++i)
    {
        FEMaterial* pmat = m_pMat[i]->GetMaterialProperties();
        FEMultiphasicMaterial* pmp = dynamic_cast<FEMultiphasicMaterial*>(pmat);
        if (pmp) n += pmp->Reactions();
    }
    return n;
}

//-----------------------------------------------------------------------------
FEReactionMaterial* FEModel::GetReaction(int id)
{
    int n = -1;
	for (int i=0; i<(int) m_pMat.Size(); ++i)
    {
        FEMaterial* pmat = m_pMat[i]->GetMaterialProperties();
        FEMultiphasicMaterial* pmp = dynamic_cast<FEMultiphasicMaterial*>(pmat);
        if (pmp) {
            for (int j=0; j<pmp->Reactions(); ++j) {
                n++;
                if (n == id) {
                    return pmp->GetReaction(j);
                }
            }
        }
    }
    return NULL;
}

//-----------------------------------------------------------------------------
void FEModel::ReplaceMaterial(GMaterial *pold, GMaterial *pnew)
{
	// find the old material
	for (int i=0; i<(int) m_pMat.Size(); ++i)
	{
		if (m_pMat[i] == pold)
		{
			size_t n = m_pMat.Remove(pold);
			m_pMat.Insert(n, pnew);
			delete pold;
			break;
		}
	}

	// replace all occurences of this material
	for (int i=0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		for (int j=0; j<po->Parts(); ++j)
		{
			GPart* pp = po->Part(j);
			GMaterial* pmat = GetMaterialFromID(pp->GetMaterialID());
			if (pmat == pold) pp->SetMaterialID(pnew->GetID());
		}
	}
}

//-----------------------------------------------------------------------------

bool FEModel::CanDeleteMaterial(GMaterial* pmat)
{
	int i, j;

	// first, we see if this material being used by a mesh
	for (i=0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		for (j=0; j<po->Parts(); ++j)
		{
			GPart* pp = po->Part(j);
			GMaterial* pm = GetMaterialFromID(pp->GetMaterialID());
			if (pm == pmat) return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
GMaterial* FEModel::GetMaterial(int n)
{
	return (n<0 || n >= (int)m_pMat.Size() ? 0 : m_pMat[n]);
}

//-----------------------------------------------------------------------------
void FEModel::AddMaterial(GMaterial* pmat)
{
	m_pMat.Add(pmat); pmat->SetModel(this);
}

//-----------------------------------------------------------------------------
void FEModel::InsertMaterial(int n, GMaterial* pm)
{ 
	m_pMat.Insert(n, pm); 
}

//-----------------------------------------------------------------------------
int FEModel::Materials()
{ 
	return (int)m_pMat.Size(); 
}

//-----------------------------------------------------------------------------
int FEModel::DeleteMaterial(GMaterial* pmat)
{
	// first, we see if this material being used by a mesh
	for (int i = 0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		for (int j = 0; j<po->Parts(); ++j)
		{
			GPart* pp = po->Part(j);
			GMaterial* pm = GetMaterialFromID(pp->GetMaterialID());
			if (pm == pmat) pp->SetMaterialID(-1);
		}
	}

	return m_pMat.Remove(pmat);
}

//-----------------------------------------------------------------------------
GMaterial* FEModel::GetMaterialFromID(int id)
{
	// don't bother looking of ID is invalid
	if (id < 0) return 0;

	for (int i=0; i<Materials(); ++i)
		if (m_pMat[i]->GetID() == id) return m_pMat[i];

	return 0;
}

//-----------------------------------------------------------------------------
// find a material from its name
GMaterial* FEModel::FindMaterial(const char* szname)
{
	if (szname == 0) return 0;
	string sname = szname;

	for (int i=0; i<Materials(); ++i)
	{
		if (m_pMat[i]->GetName() == sname) return m_pMat[i];
	}

	return 0;
}

//-----------------------------------------------------------------------------

void FEModel::Clear()
{
	// clear all data variables
	m_Var.Clear();

	// remove all meshes
	m_pModel->Clear();

	// remove all materials
	m_pMat.Clear();

	// remove all steps
	m_pStep.Clear();

	// clear all solutes and SBMS
	ClearSolutes();
	ClearSBMs();
}

//-----------------------------------------------------------------------------
void FEModel::New()
{
	// clear FE data
	Clear();

	// define the initial step
	m_pStep.Add(new FEInitialStep(this));
}

//-----------------------------------------------------------------------------
void FEModel::Save(OArchive& ar)
{
	// save model data
	int nvar = Parameters();
	if (nvar > 0)
	{
		ar.BeginChunk(CID_FEM_DATA);
		{
			for (int i=0; i<nvar; ++i)
			{
				Param& p = GetParam(i);
				double v = p.GetFloatValue();
				ar.BeginChunk(CID_FEM_CONSTANT);
				{
					ar.WriteChunk(CID_FEM_CONST_NAME , (char*) p.GetShortName ());
					ar.WriteChunk(CID_FEM_CONST_VALUE, v);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save solute data
	if (m_Sol.IsEmpty() == false)
	{
		ar.BeginChunk(CID_FEM_SOLUTE_DATA);
		{
			int NS = Solutes();
			for (int i=0; i<NS; ++i)
			{
				FESoluteData& sd = *m_Sol[i];
				ar.BeginChunk(CID_FEM_SOLUTE);
				{
					ar.WriteChunk(CID_FEM_SOLUTE_NAME      , sd.GetName());
					ar.WriteChunk(CID_FEM_SOLUTE_CHARGE    , sd.GetChargeNumber());
					ar.WriteChunk(CID_FEM_SOLUTE_MOLAR_MASS, sd.GetMolarMass());
					ar.WriteChunk(CID_FEM_SOLUTE_DENSITY   , sd.GetDensity());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save solid-bound molecule data
	if (m_SBM.IsEmpty() == false)
	{
		ar.BeginChunk(CID_FEM_SBM_DATA);
		{
			int NS = SBMs();
			for (int i=0; i<NS; ++i)
			{
				FESoluteData& sd = *m_SBM[i];
				ar.BeginChunk(CID_FEM_SBM);
				{
					ar.WriteChunk(CID_FEM_SBM_NAME      , sd.GetName());
					ar.WriteChunk(CID_FEM_SBM_CHARGE    , sd.GetChargeNumber());
					ar.WriteChunk(CID_FEM_SBM_MOLAR_MASS, sd.GetMolarMass());
					ar.WriteChunk(CID_FEM_SBM_DENSITY   , sd.GetDensity());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
    
	// save the materials
	int nmats = Materials();
	if (nmats > 0)
	{
		ar.BeginChunk(CID_MATERIAL_SECTION);
		{
			for (int i=0; i<nmats; ++i)
			{
				GMaterial* pm = GetMaterial(i);

				int ntype = 0;
				FEMaterial* mat = pm->GetMaterialProperties();
				if (mat) ntype = mat->Type();
				ar.BeginChunk(ntype);
				{
					pm->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the geometry
	int nobjs = m_pModel->Objects() + m_pModel->DiscreteObjects();
	if (nobjs > 0)
	{
		ar.BeginChunk(CID_GEOMETRY_SECTION);
		{
			m_pModel->Save(ar);
		}
		ar.EndChunk();
	}

	// save the steps
	int nsteps = Steps();
	if (nsteps > 0)
	{
		ar.BeginChunk(CID_STEP_SECTION);
		{
			for (int i=0; i<nsteps; ++i)
			{
				FEStep* ps = GetStep(i);
				int ntype = ps->GetType();
				ar.BeginChunk(ntype);
				{
					ps->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------

void FEModel::Load(IArchive& ar)
{
	TRACE("FEModel::Load");

	// clear the model
	Clear();

	GPartList::SetModel(this);

	// read the model data
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEM_DATA         : LoadData(ar); break;
		case CID_FEM_SOLUTE_DATA  : LoadSoluteData(ar); break;
        case CID_FEM_SBM_DATA     : LoadSBMData(ar); break;
		case CID_MATERIAL_SECTION : LoadMaterials(ar); break;
		case CID_GEOMETRY_SECTION : m_pModel->Load(ar); break;
		case CID_STEP_SECTION     : LoadSteps(ar); break;
		}
		ar.CloseChunk();
	}

	GPartList::SetModel(nullptr);
}

//-----------------------------------------------------------------------------
// reads the model data
void FEModel::LoadData(IArchive& ar)
{
	TRACE("FEModel::LoadData");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if (ntype == CID_FEM_CONSTANT)
		{
			char sz[256] = {0};
			double val;

			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();
				if      (ntype == CID_FEM_CONST_NAME ) ar.read(sz);
				else if (ntype == CID_FEM_CONST_VALUE) ar.read(val);
				else ReadError("unknown CID");
				ar.CloseChunk();
			}

			Param* p = GetParam(sz);
			if (p) p->SetFloatValue(val);
		}
		else throw ReadError("unknown CID");
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// Load solute data
void FEModel::LoadSoluteData(IArchive& ar)
{
	int n = 0;
	m_Sol.Clear();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if (ntype == CID_FEM_SOLUTE)
		{
			if (n > 1) throw ReadError("error parsing CID_FEM_SOLUTE (FEModel::LoadSoluteData)");
			std::string name;
			int z; double M, d;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();
				if      (ntype == CID_FEM_SOLUTE_NAME      ) ar.read(name);
				else if (ntype == CID_FEM_SOLUTE_CHARGE    ) ar.read(z);
				else if (ntype == CID_FEM_SOLUTE_MOLAR_MASS) ar.read(M);
				else if (ntype == CID_FEM_SOLUTE_DENSITY   ) ar.read(d);
				ar.CloseChunk();
			}
			AddSolute(name, z, M, d);
		}
		else throw ReadError("error in FEModel::LoadSoluteData");
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// Load solid-bound molecule data
void FEModel::LoadSBMData(IArchive& ar)
{
	int n = 0;
	m_SBM.Clear();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if (ntype == CID_FEM_SBM)
		{
			if (n > 1) throw ReadError("error parsing CID_FEM_SBM (FEModel::LoadSBMData)");
			std::string name;
			int z; double M, d;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int ntype = ar.GetChunkID();
				if      (ntype == CID_FEM_SBM_NAME      ) ar.read(name);
				else if (ntype == CID_FEM_SBM_CHARGE    ) ar.read(z);
				else if (ntype == CID_FEM_SBM_MOLAR_MASS) ar.read(M);
				else if (ntype == CID_FEM_SBM_DENSITY   ) ar.read(d);
				ar.CloseChunk();
			}
			AddSBM(name, z, M, d);
		}
		else throw ReadError("error in FEModel::LoadSBMData");
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// reads the steps from the input file
void FEModel::LoadSteps(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		FEStep* ps = 0;
		switch (ntype)
		{
		case FE_STEP_INITIAL            : ps = new FEInitialStep        (this); break;
		case FE_STEP_MECHANICS          : ps = new FENonLinearMechanics (this); break;
		case FE_STEP_NL_DYNAMIC         : ps = new FENonLinearMechanics (this); break;	// obsolete (remains for backward compatibility)
		case FE_STEP_HEAT_TRANSFER      : ps = new FEHeatTransfer       (this); break;
		case FE_STEP_BIPHASIC           : ps = new FENonLinearBiphasic  (this); break;
		case FE_STEP_BIPHASIC_SOLUTE   : ps = new FEBiphasicSolutes    (this); break;
		case FE_STEP_MULTIPHASIC		: ps = new FEMultiphasicAnalysis(this); break;
        case FE_STEP_FLUID              : ps = new FEFluidAnalysis      (this); break;
        case FE_STEP_FLUID_FSI          : ps = new FEFluidFSIAnalysis   (this); break;
		case FE_STEP_REACTION_DIFFUSION : ps = new FEReactionDiffusionAnalysis(this); break;
		default:
			throw ReadError("unknown CID in FEModel::LoadSteps");
		}

		// load the step data
		ps->Load(ar);

		// add step to model
		AddStep(ps);

		ar.CloseChunk();
	}
}


//-----------------------------------------------------------------------------
// reads materials from archive
void FEModel::LoadMaterials(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();

		FEMaterial* pmat = 0;
		// allocate the material
		if (ntype == FE_USER_MATERIAL) pmat = new FEUserMaterial(FE_USER_MATERIAL);
		else if (ntype == FE_TRANS_MOONEY_RIVLIN_OLD) pmat = new FETransMooneyRivlinOld;
		else if (ntype == FE_TRANS_VERONDA_WESTMANN_OLD) pmat = new FETransVerondaWestmannOld;
		else if (ntype == FE_COUPLED_TRANS_ISO_MR_OLD) pmat = new FECoupledTransIsoMooneyRivlinOld;
		else pmat = FEMaterialFactory::Create(ntype);

		GMaterial* pgm = new GMaterial(pmat);

		// load material data
		pgm->Load(ar);

		if (ntype == FE_TRANS_MOONEY_RIVLIN_OLD)
		{
			FETransMooneyRivlin* pnewMat = new FETransMooneyRivlin;
			pnewMat->Convert(dynamic_cast<FETransMooneyRivlinOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}
		else if (ntype == FE_TRANS_VERONDA_WESTMANN_OLD)
		{
			FETransVerondaWestmann* pnewMat = new FETransVerondaWestmann;
			pnewMat->Convert(dynamic_cast<FETransVerondaWestmannOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}
		else if (ntype == FE_COUPLED_TRANS_ISO_MR_OLD)
		{
			FECoupledTransIsoMooneyRivlin* pnewMat = new FECoupledTransIsoMooneyRivlin;
			pnewMat->Convert(dynamic_cast<FECoupledTransIsoMooneyRivlinOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}

		// add the material to the model
		AddMaterial(pgm);

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
int FEModel::Steps()
{ 
	return (int)m_pStep.Size(); 
}

//-----------------------------------------------------------------------------
FEStep* FEModel::GetStep(int i)
{ 
	return m_pStep[i]; 
}

//-----------------------------------------------------------------------------
void FEModel::AddStep(FEStep* ps)
{ 
	m_pStep.Add(ps); 
}

//-----------------------------------------------------------------------------
void FEModel::InsertStep(int n, FEStep* ps)
{ 
	m_pStep.Insert(n, ps); 
}

//-----------------------------------------------------------------------------
FEStep* FEModel::FindStep(int nid)
{
	for (int i=0; i<(int) m_pStep.Size(); ++i)
	{
		if (m_pStep[i]->GetID() == nid) return m_pStep[i];
	}
	assert(false);
	return 0;
}

//-----------------------------------------------------------------------------

int FEModel::DeleteStep(FEStep* ps)
{
	return m_pStep.Remove(ps);
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllMaterials()
{
	// reset all objects materials
	for (int i = 0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		for (int j = 0; j<po->Parts(); ++j) po->AssignMaterial(po->Part(j)->GetID(), 0);
	}

	// delete all materials
	m_pMat.Clear();
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllBC()
{
	for (int i=0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllBCs();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllLoads()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllLoads();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllIC()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllICs();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllContact()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllInterfaces();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllConstraints()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllConstraints();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllRigidConstraints()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllRigidConstraints();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllRigidConnectors()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FEStep* pstep = GetStep(i);
		pstep->RemoveAllRigidConnectors();
	}
}

//-----------------------------------------------------------------------------
void FEModel::DeleteAllSteps()
{
	// remove all steps except the initial step
	int N = Steps();
	for (int i = 1; i<N; ++i) 
	{
		delete GetStep(1);
	}
}

//-----------------------------------------------------------------------------
void FEModel::Purge(int ops)
{
	if (ops == 0)
	{
		// clear all groups
		m_pModel->ClearGroups();

		// remove discrete objects
		m_pModel->ClearDiscrete();

		// remove all steps
		m_pStep.Clear();

		// remove all materials
		DeleteAllMaterials();

		// add an initial step
		m_pStep.Add(new FEInitialStep(this));
	}
	else
	{
		ClearSelections();
	}
}

//-----------------------------------------------------------------------------
// clear the selections of all the bc, loads, etc.
void FEModel::ClearSelections()
{
	for (int i=0; i<Steps(); ++i)
	{
		FEStep* step = GetStep(i);

		for (int i=0; i<step->BCs(); ++i)
		{
			FEBoundaryCondition* pbc = step->BC(i);
			delete pbc->GetItemList(); pbc->SetItemList(0);
		}

		for (int i=0; i<step->Loads(); ++i)
		{
			FELoad* pl = step->Load(i);
			delete pl->GetItemList(); pl->SetItemList(0);
		}

		for (int i = 0; i<step->ICs(); ++i)
		{
			FEInitialCondition* pic = step->IC(i);
			delete pic->GetItemList(); pic->SetItemList(0);
		}

		for (int i=0; i<step->Interfaces(); ++i)
		{
			FEInterface* pi = step->Interface(i);

			if (dynamic_cast<FESoloInterface*>(pi))
			{
				FESoloInterface* pc = dynamic_cast<FESoloInterface*>(pi);
				delete pc->GetItemList(); pc->SetItemList(0);
			}
			else if (dynamic_cast<FEPairedInterface*>(pi))
			{
				FEPairedInterface* pc = dynamic_cast<FEPairedInterface*>(pi);
				delete pc->GetMasterSurfaceList(); pc->SetMaster(0);
				delete pc->GetSlaveSurfaceList(); pc->SetSlave(0);
			}
		}

		for (int i = 0; i < step->Constraints(); ++i)
		{
			FEModelConstraint* mc = step->Constraint(i);
			delete mc->GetItemList(); mc->SetItemList(0);
		}
	}
}

//-----------------------------------------------------------------------------
int FEModel::DataVariables()
{ 
	return (int)m_Var.Size(); 
}

//-----------------------------------------------------------------------------
FEDataVariable* FEModel::DataVariable(int i)
{ 
	return m_Var[i]; 
}

//-----------------------------------------------------------------------------
void FEModel::AddDataVariable(FEDataVariable* pv)
{ 
	m_Var.Add(pv); 
}

//-----------------------------------------------------------------------------
FEDataVariable* FEModel::FindDataVariable(int nid)
{
	for (int i=0; i<(int)m_Var.Size(); ++i)
	{
		FEDataVariable* pv = m_Var[i];
		if (pv->GetID() == nid) return pv;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// Update model data
void FEModel::UpdateData()
{
	// update material fiber pointer for trans-iso materials
	for (int i=0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int NP = po->Parts();
			for (int j=0; j<NP; ++j)
			{
				GPart& p = *po->Part(j);
				GMaterial* pgm = GetMaterialFromID(p.GetMaterialID());
				FETransverselyIsotropic* pmat = (pgm?dynamic_cast<FETransverselyIsotropic*>(pgm->GetMaterialProperties()):0);
				if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
				{
					FEDataVariable* pv = FindDataVariable(pmat->GetFiberMaterial()->m_nuser);
					int NE = pm->Elements();
					if (pv)
					{
						vec3d r;
						for (int n=0; n<NE; ++n)
						{
							FEElement& e = pm->Element(n);
							int ne = e.Nodes();
							r = vec3d(0,0,0);
							for (int m = 0; m<ne; ++m) r += po->GetTransform().LocalToGlobal(pm->Node(e.m_node[m]).r);
							r /= (double) ne;
							e.m_fiber = pv->Value(r);
							e.m_fiber.Normalize();
						}
					}
					else
					{
						// NOTE: Don't zero it since this will overwrite the values
						//       that are read from the FEBio input file.
//						for (int n=0; n<NE; ++n) pm->Element(n).m_fiber = vec3d(0,0,0);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEModel::AssignComponentToStep(FEStepComponent* pc, FEStep* ps)
{
	FEStep* po = FindStep(pc->GetStep());
	assert(po);
	if (po == 0) return;

	if (po != ps)
	{
		po->RemoveComponent(pc);
		ps->AddComponent(pc);
	}
}

//-----------------------------------------------------------------------------
// This function is used when reading FEGroup's that are not managed by an FEMesh.
// The FEGroup class reads the mesh ID and then the owner of the FEGroup calls
// this function to find the parent object (and mesh).
bool FEModel::FindGroupParent(FEGroup* pg)
{
	int obj_id = pg->GetObjectID();
	if (obj_id == -1) return false;
	else
	{
		GObject* po = GetModel().FindObject(obj_id);
		if (po == 0) return false;
		pg->SetGObject(po);
	}
	return true;
}

//-----------------------------------------------------------------------------
int FEModel::DataMaps() const
{
	return (int)m_Map.Size();
}

//-----------------------------------------------------------------------------
void FEModel::AddDataMap(FEDataMap* map)
{
	m_Map.Add(map);
}

//-----------------------------------------------------------------------------
int FEModel::RemoveMap(FEDataMap* map)
{
	return m_Map.Remove(map);
}

//-----------------------------------------------------------------------------
FEDataMap* FEModel::GetDataMap(int i)
{
	return m_Map[i];
}

//-----------------------------------------------------------------------------
int FEModel::CountBCs(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i=0; i<NSTEPS; ++i)
	{
		FEStep* step = GetStep(i);

		int NBC = step->BCs();
		for (int j=0; j<NBC; ++j)
		{
			FEBoundaryCondition* pbc = step->BC(j);
			if (pbc->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FEModel::CountLoads(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FEStep* step = GetStep(i);

		int NL = step->Loads();
		for (int j = 0; j<NL; ++j)
		{
			FELoad* pl = step->Load(j);
			if (pl->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FEModel::CountICs(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FEStep* step = GetStep(i);

		int NL = step->ICs();
		for (int j = 0; j<NL; ++j)
		{
			FEInitialCondition* pbc = step->IC(j);
			if (pbc->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FEModel::CountInterfaces(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FEStep* step = GetStep(i);

		int NC = step->Interfaces();
		for (int j = 0; j<NC; ++j)
		{
			FEInterface* pi = step->Interface(j);
			if (pi->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FEModel::CountConstraints(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FEStep* step = GetStep(i);

		int NRC = step->RigidConstraints();
		for (int j = 0; j<NRC; ++j)
		{
			FERigidConstraint* prc = step->RigidConstraint(j);
			if (prc->Type() == type) n++;
		}
	}
	return n;
}
