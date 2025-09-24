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
#include "stdafx.h"
#include "FSModel.h"
#include "FERigidConstraint.h"
#include "FEMultiMaterial.h"
#include "FEUserMaterial.h"
#include "FESurfaceLoad.h"
#include "FEBodyLoad.h"
#include "FERigidLoad.h"
#include "FEModelConstraint.h"
#include <GeomLib/GObject.h>
#include <FECore/units.h>
#include <FSCore/ParamBlock.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioModule.h>
#include "FEMKernel.h"
#include <GeomLib/GGroup.h>
#include <GeomLib/GModel.h>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <cstring>
using namespace std;

#ifndef WIN32
#define _stricmp strcasecmp
#endif

std::string Namify(const char* sz)
{
	char s[128] = { 0 };
	int n = 0;
	const char* c = sz;
	bool cap = true;
	while (*c)
	{
		if (isspace(*c) || (*c=='-')) cap = true;
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

std::string defaultBCName(FSModel* fem, FSBoundaryCondition* pbc)
{
	const char* ch = pbc->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountBCs(pbc->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultICName(FSModel* fem, FSInitialCondition* pic)
{
	const char* ch = pic->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountICs(pic->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultLoadName(FSModel* fem, FSLoad* pbc)
{
	const char* ch = pbc->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountLoads(pbc->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultInterfaceName(FSModel* fem, FSInterface* pi)
{
	const char* ch = pi->GetTypeString();
	string type = Namify(ch);

	int n = fem->CountInterfaces(pi->Type());

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultConstraintName(FSModel* fem, FSModelConstraint* pi)
{
	const char* ch = pi->GetTypeString();
	string type = Namify(ch);

	int n = CountConstraints<FSModelConstraint>(*fem);

	stringstream ss;
	ss << type << n + 1;
	return ss.str();
}

std::string defaultRigidConnectorName(FSModel* fem, FSRigidConnector* pi)
{
	int nrc = CountConnectors<FSRigidConnector>(*fem);
	stringstream ss;
	ss << "RigidConnector" << nrc + 1;
	return  ss.str();
}

std::string defaultRigidBCName(FSModel* fem, FSRigidBC* pc)
{
	int nrc = CountRigidBCs<FSRigidBC>(*fem);
	stringstream ss;
	ss << "RigidConstraint" << nrc + 1;
	return  ss.str();
}

std::string defaultRigidICName(FSModel* fem, FSRigidIC* pc)
{
	int nrc = CountRigidICs<FSRigidIC>(*fem);
	stringstream ss;
	ss << "RigidIC" << nrc + 1;
	return  ss.str();
}

std::string defaultRigidLoadName(FSModel* fem, FSRigidLoad* pc)
{
	int nrc = CountRigidLoads<FSRigidLoad>(*fem);
	stringstream ss;
	ss << "RigidLoad" << nrc + 1;
	return  ss.str();
}

std::string defaultMeshAdaptorName(FSModel* fem, FSMeshAdaptor* pc)
{
	int nrc = CountMeshAdaptors<FSMeshAdaptor>(*fem);
	stringstream ss;
	ss << "MeshAdaptor" << nrc + 1;
	return ss.str();
}

std::string defaultStepName(FSModel* fem, FSStep* ps)
{
	int nsteps = fem->Steps();
	stringstream ss;
	ss << "Step" << nsteps;
	return ss.str();
}

//-----------------------------------------------------------------------------
FSModel::FSModel() : m_skipGeometry(false)
{
	m_pModel = new GModel(this);
	New();

	// define degrees of freedom
	m_DOF.clear();

	FEDOFVariable* varDisp = AddVariable("displacement");
	varDisp->AddDOF("X-displacement", "x");
	varDisp->AddDOF("Y-displacement", "y");
	varDisp->AddDOF("Z-displacement", "z");

	FEDOFVariable* varRot = AddVariable("shell rotation");
	varRot->AddDOF("X-rotation", "u");
	varRot->AddDOF("Y-rotation", "v");
	varRot->AddDOF("Z-rotation", "w");

	FEDOFVariable* varPressure = AddVariable("fluid pressure");
	varPressure->AddDOF("pressure", "p");

	FEDOFVariable* varTemperature = AddVariable("temperature");
	varTemperature->AddDOF("temperature", "T");

    FEDOFVariable* varSolute = AddVariable("concentration");
    // (start with an empty solute variable)
    
    FEDOFVariable* varVel = AddVariable("relative fluid velocity");
    varVel->AddDOF("X-fluid velocity", "wx");
    varVel->AddDOF("Y-fluid velocity", "wy");
    varVel->AddDOF("Z-fluid velocity", "wz");
    
	FEDOFVariable* varDil = AddVariable("fluid dilatation");
	varDil->AddDOF("dilatation", "ef");

	FEDOFVariable* varSDisp = AddVariable("shell displacement");
	varSDisp->AddDOF("Shell X-displacement", "sx");
	varSDisp->AddDOF("Shell Y-displacement", "sy");
	varSDisp->AddDOF("Shell Z-displacement", "sz");

    FEDOFVariable* varAVel = AddVariable("fluid angular velocity");
    varAVel->AddDOF("X-fluid angular velocity", "gx");
    varAVel->AddDOF("Y-fluid angular velocity", "gy");
    varAVel->AddDOF("Z-fluid angular velocity", "gz");
    
	// define model parameters
	AddScienceParam(0, UNIT_TEMPERATURE, "T", "Referential absolute temperature");
    AddScienceParam(0, UNIT_PRESSURE, "P", "Referential absolute pressure");
	AddScienceParam(0, UNIT_GAS_CONSTANT, "R", "Gas constant");
	AddScienceParam(0, UNIT_FARADAY_CONSTANT, "Fc", "Faraday's constant");

	m_MLT_offset = 0;
}

//-----------------------------------------------------------------------------
void FSModel::ClearVariables()
{
	m_DOF.clear();
}

//-----------------------------------------------------------------------------
FEDOFVariable* FSModel::AddVariable(const std::string& varName)
{
	FEDOFVariable var(varName);
	m_DOF.push_back(var);
	return &m_DOF[m_DOF.size() - 1];
}

//-----------------------------------------------------------------------------
int FSModel::GetVariableIndex(const char* sz)
{
	for (int i=0; i<(int)m_DOF.size(); ++i)
	{
		if (strcmp(m_DOF[i].name(), sz) == 0) return i;
	}
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
int FSModel::GetDOFIndex(const char* sz)
{
    int idx = -1;
    for (int i=0; i<(int)m_DOF.size(); ++i)
    {
        for (int j=0; j<m_DOF[i].DOFs(); ++j) {
            ++idx;
            if (strcmp(m_DOF[i].GetDOF(j).symbol(), sz) == 0) return idx;
        }
    }
    assert(false);
    return -1;
}

//-----------------------------------------------------------------------------
FEDOFVariable& FSModel::GetVariable(const char* sz)
{
	int nvar = GetVariableIndex(sz);
	return m_DOF[nvar];
}

//-----------------------------------------------------------------------------
const char* FSModel::GetDOFSymbol(int n) const
{
	if (n < 0) { assert(false); return nullptr; }
	for (int i = 0, m = 0; i < m_DOF.size(); ++i)
	{
		const FEDOFVariable& var = m_DOF[i];
		for (int j = 0; j < var.DOFs(); ++j, ++m)
		{
			if (m == n)
			{
				const FEDOF& dof = var.GetDOF(j);
				return dof.symbol();
			}
		}
	}
	assert(false);
	return nullptr;
}

//-----------------------------------------------------------------------------
const char* FSModel::GetDOFName(int n) const
{
	if (n < 0) { return "(invalid)"; }
	for (int i = 0, m = 0; i < m_DOF.size(); ++i)
	{
		const FEDOFVariable& var = m_DOF[i];
		for (int j = 0; j < var.DOFs(); ++j, ++m)
		{
			if (m == n)
			{
				const FEDOF& dof = var.GetDOF(j);
				return dof.name();
			}
		}
	}
	assert(false);
	return nullptr;
}

//-----------------------------------------------------------------------------
FSModel::~FSModel()
{
	Clear();
	delete m_pModel;
}

//-----------------------------------------------------------------------------
void FSModel::ClearSolutes()
{
	if (m_Sol.IsEmpty() == false)
	{
		m_Sol.Clear(); 
		FEDOFVariable& var = GetVariable("concentration");
		var.Clear();
	}
}

//-----------------------------------------------------------------------------
void FSModel::AddSolute(const std::string& name, int z, double M, double d)
{
	SoluteData* s = new SoluteData;
	s->SetName(name);
	s->SetChargeNumber(z);
	s->SetMolarMass(M);
	s->SetDensity(d);
	m_Sol.Add(s);

	// Also add a degree of freedom for this
	FEDOFVariable& var = GetVariable("concentration");

	char sz[12] = {0};
	sprintf(sz, "c%d", (int)m_Sol.Size());
	var.AddDOF(name, sz);
}

//-----------------------------------------------------------------------------
void FSModel::GetRigidMaterialNames(char* szbuf)
{
	char* ch = szbuf;
	for (int i = 0; i<Materials(); ++i)
	{
		GMaterial* pm = GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			const char* szi = pm->GetName().c_str();
			strcat(ch, szi);
			ch += strlen(szi);
			*ch++ = '\0';
		}
	}
}

//-----------------------------------------------------------------------------
void FSModel::GetVariableNames(const char* szvar, char* szbuf)
{
	char var[512] = {0};
	const char* chl = strchr(szvar, '('); assert(chl);
	const char* chr = strchr(szvar, ')'); assert(chr);
	strncpy(var, chl+1, chr-chl-1);

	if      (strcmp(var, "solutes"        ) == 0) { GetSoluteNames(szbuf); return; }
	else if (strcmp(var, "sbms"           ) == 0) { GetSBMNames(szbuf); return; }
	else if (strcmp(var, "species"        ) == 0) { GetSpeciesNames(szbuf); return; }
	else if (strcmp(var, "rigid_materials") == 0) 
	{
		GetRigidMaterialNames(szbuf); return;
	}
	else
	{
		const char* szvar = var;
		if (strncmp(var, "dof_list", 8) == 0)
		{
			if (szvar[8] == 0)
			{
				char* ch = szbuf;
				int NVAR = Variables();
				for (int i = 0; i < NVAR; ++i)
				{
					FEDOFVariable& var = Variable(i);
					for (int j = 0; j < var.DOFs(); ++j)
					{
						const char* szi = var.GetDOF(j).name();
						strcat(ch, szi);
						ch += strlen(szi);
						*ch++ = '\0';
					}
				}
				return;
			}
			else if (szvar[8] == ':')
			{
				szvar = var + 9;
				int NVAR = Variables();
				for (int i = 0; i < NVAR; ++i)
				{
					FEDOFVariable& v = Variable(i);
					if (strcmp(v.name(), szvar) == 0) { GetDOFNames(v, szbuf); return; }
				}
			}
			else assert(false);
		}
	}
	assert(false);
}


//-----------------------------------------------------------------------------
const char* FSModel::GetVariableName(const char* szvar, int n, bool longName)
{
	if (szvar[0] != '$') return nullptr;

	char var[256] = { 0 };
	const char* chl = strchr(szvar, '('); assert(chl);
	const char* chr = strchr(szvar, ')'); assert(chr);
	strncpy(var, chl + 1, chr - chl - 1);

	if (strcmp(var, "solutes") == 0)
	{
		if ((n >= 0) && (n < m_Sol.Size()))
			return m_Sol[n]->GetName().c_str();
		else
			return "(invalid)";
	}
	else if (strcmp(var, "sbms") == 0)
	{
		if ((n >= 0) && (n < m_SBM.Size()))
			return m_SBM[n]->GetName().c_str();
		else
			return "(invalid)";
	}
	else if (strcmp(var, "species") == 0)
	{
		if ((n >= 0) && (n < m_Sol.Size()))
			return m_Sol[n]->GetName().c_str();
		else
		{
			n -= m_Sol.Size();
			if ((n >= 0) && (n < m_SBM.Size()))
				return m_SBM[n]->GetName().c_str();
			else
				return "(invalid)";
		}
	}
	else if (strncmp(var, "dof_list", 8) == 0)
	{
		if (var[8] == 0)
		{
			const char* szdof = (longName ? GetDOFName(n) : GetDOFSymbol(n)); assert(szdof);
			return (szdof == "nullptr" ? "(error)" : szdof);
		}
		else if (var[8] == ':')
		{
			szvar = var + 9;
			int NVAR = Variables();
			for (int i = 0; i < NVAR; ++i)
			{
				FEDOFVariable& v = Variable(i);
				if (strcmp(v.name(), szvar) == 0)
				{
					if ((n >= 0) && (n < v.DOFs()))
					{
						if (longName)
							return v.GetDOF(n).name();
						else
							return v.GetDOF(n).symbol();
					}
					else return nullptr;
				}
			}
		}
	}
	else if (strcmp(var, "rigid_materials") == 0)
	{
		GMaterial* mat = GetMaterialFromID(n);
		if (mat)
		{
			FSMaterial* femat = mat->GetMaterialProperties();
			if (femat && femat->IsRigid())
			{
				return mat->GetName().c_str();
			}
			else assert(femat);
		}
		else return nullptr;
	}
	assert(false);
	return nullptr;
}

//-----------------------------------------------------------------------------
const char* FSModel::GetEnumKey(const Param& param, bool longName)
{
	const char* szenum = param.GetEnumNames();

	if (szenum == nullptr) return nullptr;

	if (szenum[0] == '$')
	{
		return GetVariableName(szenum, param.GetIntValue(), longName);
	}
	
	int n = param.GetIntValue();
	const char* ch = szenum;
	int i = 0;
	while (*ch && (i < n))
	{
		ch = ch + strlen(ch) + 1;
		i++;
	}

	return ch;
}

//-----------------------------------------------------------------------------
int FSModel::GetEnumValue(const Param& param)
{
	const char* szenum = param.GetEnumNames(); assert(szenum);
	if ((szenum == nullptr) || (szenum[0] != '$')) return param.GetIntValue();

	const char* szvar = param.GetEnumNames();
	int n = param.GetIntValue();

	if (strcmp(szvar, "$(solutes)") == 0)
	{
		return n + 1;
	}
	else if (strcmp(szvar, "$(sbms)") == 0)
	{
		return n + 1;
	}
	else if (strcmp(szvar, "$(species)") == 0)
	{
		return n + 1;
	}
	else if (strcmp(szvar, "$(rigid_materials)") == 0)
	{
		for (int i = 0; i < Materials(); ++i)
		{
			GMaterial* mat = GetMaterial(i);
			FSMaterial* femat = mat->GetMaterialProperties();
			if (femat && femat->IsRigid())
			{
				if (mat->GetID() == n) return i + 1;
			}
		}
//		assert(false);
	}

	return param.GetIntValue();
}

//-----------------------------------------------------------------------------
int FSModel::GetEnumIndex(const Param& param)
{
	const char* szenum = param.GetEnumNames();
	int n = param.GetIntValue();
	if (szenum == nullptr) return n;
	if (szenum[0] == '$')
	{
		if (strcmp(szenum, "$(solutes)") == 0)
		{
			if ((n >= 0) && (n < m_Sol.Size()))
				return n;
			else
				return -1;
		}
		else if (strcmp(szenum, "$(sbms)") == 0)
		{
			if ((n >= 0) && (n < m_SBM.Size()))
				return n;
			else
				return -1;
		}
		else if (strcmp(szenum, "$(species)") == 0)
		{
			if ((n >= 0) && (n < m_Sol.Size())) return n;
			else
			{
				n -= m_Sol.Size();
				if ((n >= 0) && (n < m_SBM.Size())) return n + m_Sol.Size();
			}
		}
		else if (strcmp(szenum, "$(rigid_materials)") == 0)
		{
			int m = 0;
			for (int i = 0; i < Materials(); ++i)
			{
				GMaterial* mat = GetMaterial(i);
				FSMaterial* femat = mat->GetMaterialProperties();
				if (femat && femat->IsRigid())
				{
					if (mat->GetID() == n) return m;
					m++;
				}
			}
			return -1;
		}
	}
	
	return n;
}

//-----------------------------------------------------------------------------
void FSModel::GetDOFNames(FEDOFVariable& var, char* szbuf)
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
void FSModel::GetDOFNames(FEDOFVariable& var, vector<string>& dofList)
{
	dofList.clear();
	for (int i = 0; i < var.DOFs(); ++i)
	{
		const char* szi = var.GetDOF(i).name();
		dofList.push_back(szi);
	}
}

//-----------------------------------------------------------------------------
void FSModel::GetDOFSymbols(FEDOFVariable& var, vector<string>& dofList)
{
	dofList.clear();

	// TODO: little hack for concentration variables.
	//       The problem is that the concentration variable is empty and does not get updated.
	//       when solutes are added. So, we do it this way.
	if (strcmp(var.name(), "concentration") == 0)
	{
		int nsol = Solutes();
		for (int i = 0; i < nsol; ++i)
		{
			char sz[16] = { 0 };
			sprintf(sz, "c%d", i + 1);
			dofList.push_back(sz);
		}
	}

	for (int i = 0; i < var.DOFs(); ++i)
	{
		const char* szi = var.GetDOF(i).symbol();
		dofList.push_back(szi);
	}
}

//-----------------------------------------------------------------------------
bool FSModel::GetEnumValues(char* szbuf, std::vector<int>& l, const char* szenum)
{
	assert(szbuf);
	if (szbuf == nullptr) return false;
	if (szenum == nullptr) return false;
	if (szenum[0] == '$')
	{
		char var[256] = { 0 };
		const char* chl = strchr(szenum, '('); assert(chl);
		const char* chr = strchr(szenum, ')'); assert(chr);
		strncpy(var, chl + 1, chr - chl - 1);

		if (strncmp(var, "dof_list", 8) == 0)
		{
			if (var[8] == 0)
			{
				char* sz = szbuf;
				for (int i = 0; i < l.size(); ++i)
				{
					const char* dofname = GetDOFSymbol(l[i]); assert(dofname);
					strcat(sz, dofname);
					int n = strlen(sz);
					if (i != l.size() - 1) sz[n] = ',';
					sz += n + 1;
				}
			}
			else
			{
				const char* szvar = var + 9;
				FEDOFVariable& var = GetVariable(szvar);

				vector<string> dofList;
				GetDOFSymbols(var, dofList);

				char* sz = szbuf;
				for (int i = 0; i < l.size(); ++i)
				{
					strcat(sz, dofList[l[i]].c_str());
					int n = strlen(sz);
					if (i != l.size() - 1) sz[n] = ',';
					sz += n + 1;
				}
			}

			return true;
		}
	}
	
	return false;
}

//-----------------------------------------------------------------------------
bool FSModel::SetEnumIndex(Param& param, int index)
{
	const char* szvar = param.GetEnumNames();
	if ((szvar == nullptr) || (szvar[0] != '$')) param.SetIntValue(index);

	int val = index;
	if (strcmp(szvar, "$(solutes)") == 0)
	{
		val = index;
	}
	else if (strcmp(szvar, "$(sbms)") == 0)
	{
		val = index;
	}
	else if (strcmp(szvar, "$(species)") == 0)
	{
		val = index;
	}
	else if (strcmp(szvar, "$(rigid_materials)") == 0)
	{
		int m = 0;
		for (int i = 0; i < Materials(); ++i)
		{
			GMaterial* mat = GetMaterial(i);
			FSMaterial* femat = mat->GetMaterialProperties();
			if (femat && femat->IsRigid())
			{
				if (m == index)
				{
					val = mat->GetID();
					break;
				}
				m++;
			}
		}
	}
	param.SetIntValue(val);

	return true;
}

//-----------------------------------------------------------------------------
bool FSModel::SetEnumValue(Param& param, int nvalue)
{
	const char* szvar = param.GetEnumNames();
	if (strcmp(szvar, "$(solutes)") == 0)
	{
		if ((nvalue > 0) && (nvalue <= m_Sol.Size()))
			param.SetIntValue(nvalue - 1);
		else
			param.SetIntValue(-1);
		return true;
	}
	else if (strcmp(szvar, "$(sbms)") == 0)
	{
		if ((nvalue > 0) && (nvalue <= m_SBM.Size()))
			param.SetIntValue(nvalue - 1);
		else
			param.SetIntValue(-1);
		return true;
	}
	else if (strcmp(szvar, "$(species)") == 0)
	{
		if ((nvalue > 0) && (nvalue <= m_Sol.Size() + m_SBM.Size()))
			param.SetIntValue(nvalue - 1);
		else
			param.SetIntValue(-1);
		return true;
	}
	else if (strcmp(szvar, "$(rigid_materials)") == 0)
	{
		if ((nvalue > 0) && (nvalue <= Materials()))
		{
			GMaterial* mat = GetMaterial(nvalue - 1);
			FSMaterial* femat = mat->GetMaterialProperties();
			if (femat && femat->IsRigid())
			{
				param.SetIntValue(mat->GetID());
				return true;
			}
		}
		assert(false);
	}

	param.SetIntValue(nvalue);

	return true;
}

//-----------------------------------------------------------------------------
bool FSModel::SetEnumKey(Param& param, const std::string& key)
{
	const char* szvar = param.GetEnumNames();
	if (szvar == nullptr) return false;

	if (strcmp(szvar, "$(solutes)") == 0)
	{
		for (int i = 0; i < Solutes(); ++i)
		{
			string si = GetSoluteData(i).GetName();
			if (si == key)
			{
				param.SetIntValue(i);
				return true;
			}
		}
	}
	else if (strcmp(szvar, "$(sbms)") == 0)
	{
		for (int i = 0; i < SBMs(); ++i)
		{
			string si = GetSBMData(i).GetName();
			if (si == key)
			{
				param.SetIntValue(i);
				return true;
			}
		}
	}
	else if (strcmp(szvar, "$(species)") == 0)
	{
		for (int i = 0; i < Solutes(); ++i)
		{
			string si = GetSoluteData(i).GetName();
			if (si == key)
			{
				param.SetIntValue(i);
				return true;
			}
		}
		for (int i = 0; i < SBMs(); ++i)
		{
			string si = GetSBMData(i).GetName();
			if (si == key)
			{
				param.SetIntValue(i + Solutes());
				return true;
			}
		}
	}
	else if (strcmp(szvar, "$(rigid_materials)") == 0)
	{
		for (int i=0; i<Materials(); ++i)
		{
			GMaterial* mat = GetMaterial(i);
			FSMaterial* femat = mat->GetMaterialProperties();
			if (femat && femat->IsRigid() && (mat->GetName() == key))
			{
				param.SetIntValue(mat->GetID());
				return true;
			}
		}
	}
	else if (strcmp(szvar, "$(dof_list)") == 0)
	{
		int n = 0;
		int NVAR = Variables();
		for (int i = 0; i < NVAR; ++i)
		{
			FEDOFVariable& var = Variable(i);
			for (int j = 0; j < var.DOFs(); ++j, ++n)
			{
				const char* szi = var.GetDOF(j).symbol();
				if (key == szi)
				{
					param.SetIntValue(n);
					return true;
				}
			}
		}
	}
	else if (strncmp(szvar, "$(dof_list:", 11) == 0)
	{
		const char* szvarname = szvar + 11;
		char tmp[256] = { 0 };
		strcpy(tmp, szvar + 11);
		int l = strlen(tmp);
		tmp[l - 1] = 0;

		int i = GetVariableIndex(tmp);
		FEDOFVariable& var = Variable(i);
		for (int j = 0; j < var.DOFs(); ++j)
		{
			const char* szi = var.GetDOF(j).symbol();
			if (key == szi)
			{
				param.SetIntValue(j);
				return true;
			}
		}
	}
	else if (szvar[0] != '$')
	{
		// see if the value string matches an enum string
		int n = 0;
		const char* sz = nullptr;
		while (sz = param.GetEnumName(n))
		{
			if (_stricmp(key.c_str(), sz) == 0)
			{
				param.SetIntValue(n - param.GetOffset());
				return true;
			}
			n++;
		}
	}

	assert(false);
	return false;
}

//-----------------------------------------------------------------------------
void FSModel::GetSoluteNames(char* szbuf)
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
void FSModel::GetSBMNames(char* szbuf)
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
void FSModel::GetSpeciesNames(char* szbuf)
{
	// get the solute names
	GetSoluteNames(szbuf);

	// wind the buffer forward
	while (szbuf[0])
	{
		szbuf = szbuf + strlen(szbuf) + 1;
	}

	// add the SBM names
	GetSBMNames(szbuf);
}

//-----------------------------------------------------------------------------
int FSModel::FindSolute(const char* sz)
{
	string sol(sz);
	for (int i=0; i<m_Sol.Size(); ++i)
	{
		if (m_Sol[i]->GetName() == sol) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
SoluteData& FSModel::GetSoluteData(int i)
{ 
	return *m_Sol[i]; 
}

//-----------------------------------------------------------------------------
int FSModel::Solutes()
{ 
	return (int)m_Sol.Size(); 
}

//-----------------------------------------------------------------------------
void FSModel::RemoveSolute(int n)
{
	delete m_Sol[n];

	// Also remove degree of freedom for this
	FEDOFVariable& var = GetVariable("concentration");
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
int FSModel::FindSBM(const char* sz)
{
	string sbm(sz);
	for (int i = 0; i<m_SBM.Size(); ++i)
	{
		if (m_SBM[i]->GetName() == sbm) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
SoluteData& FSModel::GetSBMData(int i)
{ 
	return *m_SBM[i]; 
}

//-----------------------------------------------------------------------------
int FSModel::SBMs()
{ 
	return (int)m_SBM.Size(); 
}

//-----------------------------------------------------------------------------
void FSModel::AddSBM(const std::string& name, int z, double M, double d)
{
	SoluteData* s = new SoluteData;
	s->SetName(name);
	s->SetChargeNumber(z);
	s->SetMolarMass(M);
	s->SetDensity(d);
	m_SBM.Add(s);
}

//-----------------------------------------------------------------------------
void FSModel::ClearSBMs()
{
	m_SBM.Clear();
}

//-----------------------------------------------------------------------------
void FSModel::RemoveSBM(int n)
{
	delete m_SBM[n];
}

//-----------------------------------------------------------------------------
int FSModel::Reactions()
{
    int n = 0;
	for (int i=0; i<(int) m_pMat.Size(); ++i)
    {
        FSMaterial* pmat = m_pMat[i]->GetMaterialProperties();
        FSMultiphasicMaterial* pmp = dynamic_cast<FSMultiphasicMaterial*>(pmat);
        if (pmp) n += pmp->Reactions();
    }
    return n;
}

//-----------------------------------------------------------------------------
FSReactionMaterial* FSModel::GetReaction(int id)
{
    int n = -1;
	for (int i=0; i<(int) m_pMat.Size(); ++i)
    {
        FSMaterial* pmat = m_pMat[i]->GetMaterialProperties();
        FSMultiphasicMaterial* pmp = dynamic_cast<FSMultiphasicMaterial*>(pmat);
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
void FSModel::ReplaceMaterial(GMaterial *pold, GMaterial *pnew)
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
		po->UpdateFEElementMatIDs();
	}
	ClearMLT();
}

bool FSModel::CanDeleteMaterial(GMaterial* pmat)
{
	for (int i=0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		for (int j=0; j<po->Parts(); ++j)
		{
			GPart* pp = po->Part(j);
			GMaterial* pm = GetMaterialFromID(pp->GetMaterialID());
			if (pm == pmat) return false;
		}
	}

	return true;
}

GMaterial* FSModel::GetMaterial(int n)
{
	return (n<0 || n >= (int)m_pMat.Size() ? 0 : m_pMat[n]);
}

void FSModel::AddMaterial(GMaterial* pmat)
{
	m_pMat.Add(pmat); pmat->SetModel(this);
	ClearMLT();
}

void FSModel::InsertMaterial(int n, GMaterial* pm)
{ 
	m_pMat.Insert(n, pm); 
	ClearMLT();
}

int FSModel::Materials()
{ 
	return (int)m_pMat.Size(); 
}

int FSModel::DeleteMaterial(GMaterial* pmat)
{
	// first, we see if this material being used by a mesh
	for (int i = 0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		bool needsUpdate = false;
		for (int j = 0; j<po->Parts(); ++j)
		{
			GPart* pp = po->Part(j);
			GMaterial* pm = GetMaterialFromID(pp->GetMaterialID());
			if (pm == pmat)
			{
				pp->SetMaterialID(-1);
				needsUpdate = true;
			}
		}
		if (needsUpdate) po->UpdateFEElementMatIDs();
	}
	ClearMLT();
	return m_pMat.Remove(pmat);
}

void FSModel::ClearMLT()
{
	m_MLT_offset = 0;
	m_MLT.clear();
}

void FSModel::BuildMLT()
{
	if (Materials() == 0)
	{
		ClearMLT();
		return;
	}

	int minId = m_pMat[0]->GetID();
	int maxId = minId;
	for (int i = 0; i < Materials(); ++i)
	{
		int mid = m_pMat[i]->GetID();
		if (mid < minId) minId = mid;
		if (mid > maxId) maxId = mid;
	}
	int mats = maxId - minId + 1;
	m_MLT.assign(mats, nullptr);
	m_MLT_offset = minId;
	for (int i = 0; i < Materials(); ++i)
	{
		int mid = m_pMat[i]->GetID();
		m_MLT[mid - m_MLT_offset] = m_pMat[i];
	}
}

GMaterial* FSModel::GetMaterialFromID(int id)
{
	// don't bother looking of ID is invalid
	if (id < 0) return nullptr;

	if (m_MLT.empty()) BuildMLT();

	id -= m_MLT_offset;
	if ((id < 0) || (id >= m_MLT.size())) return nullptr;

	return m_MLT[id];
}

//-----------------------------------------------------------------------------
// find a material from its name
GMaterial* FSModel::FindMaterial(const string& name)
{
	for (int i=0; i<Materials(); ++i)
	{
		if (m_pMat[i]->GetName() == name) return m_pMat[i];
	}

	return nullptr;
}

void FSModel::AssignMaterial(GObject* po, GMaterial* mat)
{
	int matID = (mat ? mat->GetID() : -1);
	for (int i = 0; i < po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		pg->SetMaterialID(matID);
	}
	po->UpdateFEElementMatIDs();
	UpdateMaterialPositions();
}

void FSModel::AssignMaterial(GPart* pg, GMaterial* mat)
{
	int matID = (mat ? mat->GetID() : -1);
	pg->SetMaterialID(matID);
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	po->UpdateFEElementMatIDs();
	UpdateMaterialPositions();
}

void FSModel::AssignMaterial(const std::vector<GPart*>& partList, GMaterial* mat)
{
	int matID = (mat ? mat->GetID() : -1);
	std::set<GObject*> obj;
	for (auto pg : partList)
	{
		pg->SetMaterialID(matID);
		obj.insert(dynamic_cast<GObject*>(pg->Object()));
	}
	for (auto po : obj) po->UpdateFEElementMatIDs();
	UpdateMaterialPositions();
}

void FSModel::AssignMaterial(const std::vector<GPart*>& partList, const std::vector<GMaterial*>& matList)
{
	if (partList.size() != matList.size()) { assert(false); return; }

	std::set<GObject*> obj;
	for (int i=0; i<partList.size(); ++i)
	{
		GPart* pg = partList[i];
		GMaterial* mat = matList[i];

		int matID = (mat ? mat->GetID() : -1);
		pg->SetMaterialID(matID);
		obj.insert(dynamic_cast<GObject*>(pg->Object()));
	}
	for (auto po : obj) po->UpdateFEElementMatIDs();
	UpdateMaterialPositions();
}

std::vector<GPart*> FSModel::GetMaterialPartList(GMaterial* mat)
{
	std::vector<GPart*> partList;
	if (mat)
	{
		GModel& gm = GetModel();
		int matID = mat->GetID();
		for (int i = 0; i < gm.Objects(); ++i)
		{
			GObject* po = gm.Object(i);
			for (int j = 0; j < po->Parts(); ++j)
			{
				GPart* pg = po->Part(j); assert(pg);
				if (pg && (pg->GetMaterialID() == matID)) partList.push_back(pg);
			}
		}
	}

	return partList;
}

void FSModel::UpdateMaterialPositions()
{
	int NMAT = Materials();
	if (NMAT == 0) return;

	std::map<GMaterial*, int> tag;
	for (int i = 0; i < NMAT; ++i)
	{
		GMaterial* mat = GetMaterial(i);
		mat->m_pos = vec3d(0, 0, 0);
		tag[mat] = 0;
	}

	GModel& gm = GetModel();
	for (int i = 0; i < gm.Objects(); ++i)
	{
		GObject* po = gm.Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pg = po->Part(j);
			if (pg)
			{
				GMaterial* mat = GetMaterialFromID(pg->GetMaterialID());
				if (mat)
				{
					mat->m_pos += pg->GetGlobalBox().Center();
					tag[mat]++;
				}
			}
		}
	}

	for (int i = 0; i < NMAT; ++i)
	{
		GMaterial* mat = GetMaterial(i);
		int count = tag[mat];
		if (count != 0) mat->m_pos /= count;
	}
}

//-----------------------------------------------------------------------------
FSRigidConnector* FSModel::GetRigidConnectorFromID(int id)
{
    // don't bother looking of ID is invalid
    if (id < 0) return 0;

    int lid = -1;
    for (int n = 0; n<Steps(); ++n)
    {
        FSStep& s = *GetStep(n);
        for (int i = 0; i<s.RigidConnectors(); ++i)
            if (++lid == id) return s.RigidConnector(i);
    }

    return 0;
}

//-----------------------------------------------------------------------------

void FSModel::Clear()
{
	// clear all data variables
	m_Var.Clear();

	// remove all materials
	m_pMat.Clear();
	ClearMLT();

	// remove all steps
	m_pStep.Clear();

	// remove all meshes
	m_pModel->Clear();

	// clear all solutes and SBMS
	ClearSolutes();
	ClearSBMs();
}

//-----------------------------------------------------------------------------
void FSModel::New()
{
	// clear FE data
	Clear();

	// define the initial step
	m_pStep.Add(new FSInitialStep(this));
}

//-----------------------------------------------------------------------------
int FSModel::CountMeshDataFields()
{
	// count the mesh data fields on the meshes
	GModel& mdl = GetModel();
	int total = 0;
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		FSMesh* mesh = po->GetFEMesh();
		if (mesh) total += mesh->MeshDataFields();
	}

	// add the data generators
	total += MeshDataGenerators();

	return total;
}

//-----------------------------------------------------------------------------
void FSModel::Save(OArchive& ar)
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
				SoluteData& sd = *m_Sol[i];
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
				SoluteData& sd = *m_SBM[i];
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
				FSMaterial* mat = pm->GetMaterialProperties();
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

	// save mesh data generators
	if (MeshDataGenerators() > 0)
	{
		ar.BeginChunk(CID_MESHDATA_LIST);
		{
			for (int i = 0; i < MeshDataGenerators(); ++i)
			{
				FSMeshDataGenerator* pmd = GetMeshDataGenerator(i);
				int ntype = pmd->Type();
				ar.BeginChunk(ntype);
				{
					pmd->Save(ar);
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save load controllers
	if (LoadControllers() > 0)
	{
		ar.BeginChunk(CID_LOAD_CONTROLLER_LIST);
		{
			for (int i = 0; i < LoadControllers(); ++i)
			{
				FSLoadController* plc = GetLoadController(i);
				int ntype = plc->Type();
				ar.BeginChunk(ntype);
				{
					plc->Save(ar);
				}
				ar.EndChunk();
			}
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
				FSStep* ps = GetStep(i);
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

void FSModel::Load(IArchive& ar)
{
	TRACE("FSModel::Load");

	// clear the model
	Clear();

    m_pModel->SetLoadOnlyDiscreteFlag(m_skipGeometry);

	// read the model data
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEM_DATA            : LoadData(ar); break;
		case CID_FEM_SOLUTE_DATA     : LoadSoluteData(ar); break;
        case CID_FEM_SBM_DATA        : LoadSBMData(ar); break;
		case CID_MATERIAL_SECTION    : LoadMaterials(ar); break;
		case CID_GEOMETRY_SECTION    : m_pModel->Load(ar); break;
		case CID_STEP_SECTION        : LoadSteps(ar); break;
		case CID_LOAD_CONTROLLER_LIST: LoadLoadControllers(ar); break;
		case CID_MESHDATA_LIST       : LoadMeshDataGenerators(ar); break;
		}
		ar.CloseChunk();
	}
	UpdateMaterialPositions();
}

//-----------------------------------------------------------------------------
// reads the model data
void FSModel::LoadData(IArchive& ar)
{
	TRACE("FSModel::LoadData");

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
void FSModel::LoadSoluteData(IArchive& ar)
{
	int n = 0;
	m_Sol.Clear();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if (ntype == CID_FEM_SOLUTE)
		{
			if (n > 1) throw ReadError("error parsing CID_FEM_SOLUTE (FSModel::LoadSoluteData)");
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
		else throw ReadError("error in FSModel::LoadSoluteData");
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// Load solid-bound molecule data
void FSModel::LoadSBMData(IArchive& ar)
{
	int n = 0;
	m_SBM.Clear();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		if (ntype == CID_FEM_SBM)
		{
			if (n > 1) throw ReadError("error parsing CID_FEM_SBM (FSModel::LoadSBMData)");
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
		else throw ReadError("error in FSModel::LoadSBMData");
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// reads the steps from the input file
void FSModel::LoadSteps(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		FSStep* ps = 0;
		switch (ntype)
		{
		case FE_STEP_INITIAL            : ps = new FSInitialStep        (this); break;
		case FE_STEP_MECHANICS          : ps = new FSNonLinearMechanics (this); break;
		case FE_STEP_NL_DYNAMIC         : ps = new FSNonLinearMechanics (this); break;	// obsolete (remains for backward compatibility)
		case FE_STEP_HEAT_TRANSFER      : ps = new FSHeatTransfer       (this); break;
		case FE_STEP_BIPHASIC           : ps = new FSNonLinearBiphasic  (this); break;
		case FE_STEP_BIPHASIC_SOLUTE    : ps = new FSBiphasicSolutes    (this); break;
		case FE_STEP_MULTIPHASIC		: ps = new FSMultiphasicAnalysis(this); break;
        case FE_STEP_FLUID              : ps = new FSFluidAnalysis      (this); break;
        case FE_STEP_FLUID_FSI          : ps = new FSFluidFSIAnalysis   (this); break;
		case FE_STEP_REACTION_DIFFUSION : ps = new FSReactionDiffusionAnalysis(this); break;
        case FE_STEP_POLAR_FLUID        : ps = new FSPolarFluidAnalysis (this); break;
		case FE_STEP_FEBIO_ANALYSIS     : ps = new FEBioAnalysisStep(this); break;
        case FE_STEP_FLUID_SOLUTES      : ps = new FSFluidSolutesAnalysis(this); break;
        case FE_STEP_THERMO_FLUID       : ps = new FSThermoFluidAnalysis(this); break;
		default:
			throw ReadError("unknown CID in FSModel::LoadSteps");
		}

		// load the step data
		ps->Load(ar);

		// add step to model
		AddStep(ps);

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
void FSModel::LoadMeshDataGenerators(IArchive& ar)
{
	assert(MeshDataGenerators() == 0);
	FEMKernel& kernel = *FEMKernel::Instance();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		FSMeshDataGenerator* pmd = nullptr;
		switch (ntype)
		{
		case FE_FEBIO_NODEDATA_GENERATOR: pmd = new FEBioNodeDataGenerator(this); break;
		case FE_FEBIO_EDGEDATA_GENERATOR: pmd = new FEBioEdgeDataGenerator(this); break;
		case FE_FEBIO_FACEDATA_GENERATOR: pmd = new FEBioFaceDataGenerator(this); break;
		case FE_FEBIO_ELEMDATA_GENERATOR: pmd = new FEBioElemDataGenerator(this); break;
		case FE_CONST_FACEDATA_GENERATOR: pmd = new FSConstFaceDataGenerator(this); break;
		default:
			assert(false);
		}
		
		if (pmd == nullptr) throw ReadError("unknown CID in FSModel::LoadMeshDataGenerators");
		AddMeshDataGenerator(pmd);
		pmd->Load(ar);
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
void FSModel::LoadLoadControllers(IArchive& ar)
{
	assert(LoadControllers() == 0);
	FEMKernel& kernel = *FEMKernel::Instance();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();
		FSLoadController* plc = dynamic_cast<FSLoadController*>(kernel.Create(this, FELOADCONTROLLER_ID, ntype));
		if (plc == nullptr) throw ReadError("unknown CID in FSModel::LoadLoadControllers");
		AddLoadController(plc);
		plc->Load(ar);
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// reads materials from archive
void FSModel::LoadMaterials(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int ntype = ar.GetChunkID();

		FSMaterial* pmat = 0;
		// allocate the material
		if (ntype == FE_USER_MATERIAL) pmat = new FSUserMaterial(FE_USER_MATERIAL, this);
		else if (ntype == FE_TRANS_MOONEY_RIVLIN_OLD    ) pmat = new FSTransMooneyRivlinOld(this);
		else if (ntype == FE_TRANS_VERONDA_WESTMANN_OLD ) pmat = new FSTransVerondaWestmannOld(this);
		else if (ntype == FE_COUPLED_TRANS_ISO_MR_OLD   ) pmat = new FSCoupledTransIsoMooneyRivlinOld(this);
        else if (ntype == FE_ACTIVE_CONTRACT_UNI_OLD    ) pmat = new FSPrescribedActiveContractionUniaxialOld(this);
        else if (ntype == FE_ACTIVE_CONTRACT_TISO_OLD   ) pmat = new FSPrescribedActiveContractionTransIsoOld(this);
        else if (ntype == FE_ACTIVE_CONTRACT_UNI_UC_OLD ) pmat = new FSPrescribedActiveContractionUniaxialUCOld(this);
        else if (ntype == FE_ACTIVE_CONTRACT_TISO_UC_OLD) pmat = new FSPrescribedActiveContractionTransIsoUCOld(this);
        else if (ntype == FE_FIBEREXPPOW_COUPLED_OLD    ) pmat = new FSFiberExpPowOld(this);
        else if (ntype == FE_FIBEREXPPOW_UNCOUPLED_OLD  ) pmat = new FSFiberExpPowUncoupledOld(this);
        else if (ntype == FE_FIBERPOWLIN_COUPLED_OLD    ) pmat = new FSFiberPowLinOld(this);
        else if (ntype == FE_FIBERPOWLIN_UNCOUPLED_OLD  ) pmat = new FSFiberPowLinUncoupledOld(this);
		else pmat = FEMaterialFactory::Create(this, ntype);

		GMaterial* pgm = new GMaterial(pmat);

		// load material data
		pgm->Load(ar);

		if (ntype == FE_TRANS_MOONEY_RIVLIN_OLD)
		{
			FSTransMooneyRivlin* pnewMat = new FSTransMooneyRivlin(this);
			pnewMat->Convert(dynamic_cast<FSTransMooneyRivlinOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}
		else if (ntype == FE_TRANS_VERONDA_WESTMANN_OLD)
		{
			FSTransVerondaWestmann* pnewMat = new FSTransVerondaWestmann(this);
			pnewMat->Convert(dynamic_cast<FSTransVerondaWestmannOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}
		else if (ntype == FE_COUPLED_TRANS_ISO_MR_OLD)
		{
			FSCoupledTransIsoMooneyRivlin* pnewMat = new FSCoupledTransIsoMooneyRivlin(this);
			pnewMat->Convert(dynamic_cast<FSCoupledTransIsoMooneyRivlinOld*>(pmat));
			pgm->SetMaterialProperties(pnewMat);
		}
        else if (ntype == FE_ACTIVE_CONTRACT_UNI_OLD)
        {
            FSPrescribedActiveContractionUniaxial* pnewMat = new FSPrescribedActiveContractionUniaxial(this);
            pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionUniaxialOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_ACTIVE_CONTRACT_TISO_OLD)
        {
            FSPrescribedActiveContractionTransIso* pnewMat = new FSPrescribedActiveContractionTransIso(this);
            pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionTransIsoOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_ACTIVE_CONTRACT_UNI_UC_OLD)
        {
            FSPrescribedActiveContractionUniaxialUC* pnewMat = new FSPrescribedActiveContractionUniaxialUC(this);
            pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionUniaxialUCOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_ACTIVE_CONTRACT_TISO_UC_OLD)
        {
            FSPrescribedActiveContractionTransIsoUC* pnewMat = new FSPrescribedActiveContractionTransIsoUC(this);
            pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionTransIsoUCOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_FIBEREXPPOW_COUPLED_OLD)
        {
            FSFiberExpPow* pnewMat = new FSFiberExpPow(this);
            pnewMat->Convert(dynamic_cast<FSFiberExpPowOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_FIBEREXPPOW_UNCOUPLED_OLD)
        {
            FSFiberExpPowUncoupled* pnewMat = new FSFiberExpPowUncoupled(this);
            pnewMat->Convert(dynamic_cast<FSFiberExpPowUncoupledOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_FIBERPOWLIN_COUPLED_OLD)
        {
            FSFiberPowLin* pnewMat = new FSFiberPowLin(this);
            pnewMat->Convert(dynamic_cast<FSFiberPowLinOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }
        else if (ntype == FE_FIBERPOWLIN_UNCOUPLED_OLD)
        {
            FSFiberPowLinUncoupled* pnewMat = new FSFiberPowLinUncoupled(this);
            pnewMat->Convert(dynamic_cast<FSFiberPowLinUncoupledOld*>(pmat));
            pgm->SetMaterialProperties(pnewMat);
        }

		// add the material to the model
		AddMaterial(pgm);

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
int FSModel::Steps()
{ 
	return (int)m_pStep.Size(); 
}

//-----------------------------------------------------------------------------
FSStep* FSModel::GetStep(int i)
{ 
	return m_pStep[i]; 
}

//-----------------------------------------------------------------------------
void FSModel::AddStep(FSStep* ps)
{ 
	m_pStep.Add(ps); 
}

//-----------------------------------------------------------------------------
void FSModel::InsertStep(int n, FSStep* ps)
{ 
	m_pStep.Insert(n, ps); 
}

//-----------------------------------------------------------------------------
void FSModel::SwapSteps(FSStep* ps0, FSStep* ps1)
{
	int n0 = GetStepIndex(ps0);
	assert(n0 >= 0);

	int n1 = GetStepIndex(ps1);
	assert(n1 >= 0);

	if ((n0 >= 0) && (n1 >= 0))
	{
		FSStep* tmp = m_pStep[n0];
		m_pStep.Set(n0, m_pStep[n1]);
		m_pStep.Set(n1, tmp);
	}
}

//-----------------------------------------------------------------------------
FSStep* FSModel::ReplaceStep(int i, FSStep* newStep)
{
	return m_pStep.Replace(i, newStep);
}

//-----------------------------------------------------------------------------
FSStep* FSModel::FindStep(int nid)
{
	for (int i=0; i<(int) m_pStep.Size(); ++i)
	{
		if (m_pStep[i]->GetID() == nid) return m_pStep[i];
	}
	assert(false);
	return 0;
}

//-----------------------------------------------------------------------------
int FSModel::GetStepIndex(FSStep* ps)
{
	for (int i = 0; i < (int)m_pStep.Size(); ++i)
	{
		if (m_pStep[i] == ps) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------

int FSModel::DeleteStep(FSStep* ps)
{
	return m_pStep.Remove(ps);
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllMaterials()
{
	// reset all objects materials
	for (int i = 0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		AssignMaterial(po, nullptr);
	}

	// delete all materials
	m_pMat.Clear();
	ClearMLT();
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllBC()
{
	for (int i=0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllBCs();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllLoads()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllLoads();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllIC()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllICs();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllContact()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllInterfaces();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllConstraints()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllConstraints();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllRigidLoads()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllRigidLoads();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllLoadControllers()
{
	m_LC.Clear();
}

//-----------------------------------------------------------------------------
void FSModel::RemoveUnusedLoadControllers()
{
	int NLC = m_LC.Size();
	int n = 0;
	for (int i = 0; i < NLC; ++i)
	{
		FSLoadController* plc = m_LC[n];
		if (plc->GetReferenceCount() == 0) {
			m_LC.Remove(plc); delete plc;
		}
		else n++;
	}
}

void FSModel::RemoveUnusedMaterials()
{
	const int NMAT = Materials();
	std::map<int, int> tag;

	GModel& mdl = GetModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pg = po->Part(j);
			int matid = pg->GetMaterialID();
			tag[matid] = 1;
		}
	}

	std::vector<GMaterial*> deleteMats;
	for (int i = 0; i < Materials(); ++i)
	{
		GMaterial* mat = GetMaterial(i);
		if (tag.find(mat->GetID()) == tag.end())
			deleteMats.push_back(mat);
	}

	if (!deleteMats.empty())
	{
		for (GMaterial* pm : deleteMats)
			delete pm;
	}

	ClearMLT();
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllMeshDataGenerators()
{
	m_MD.Clear();
}

void FSModel::DeleteAllMeshData()
{
	GModel& m = GetModel();
	for (int i = 0; i < m.Objects(); ++i)
	{
		GObject* po = m.Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm) pm->ClearMeshData();
	}
}

void FSModel::DeleteAllMeshAdaptors()
{
	for (int i = 0; i < Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllMeshAdaptors();
	}
}

void FSModel::DeleteAllRigidBCs()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllRigidBCs();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllRigidICs()
{
	for (int i = 0; i < Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllRigidICs();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllRigidConnectors()
{
	for (int i = 0; i<Steps(); ++i)
	{
		FSStep* pstep = GetStep(i);
		pstep->RemoveAllRigidConnectors();
	}
}

//-----------------------------------------------------------------------------
void FSModel::DeleteAllSteps()
{
	// remove all steps except the initial step
	int N = Steps();
	for (int i = 1; i<N; ++i) 
	{
		delete GetStep(1);
	}
}

//-----------------------------------------------------------------------------
void FSModel::Purge()
{
	m_pModel->RemoveMeshData();

	// clear all groups
	m_pModel->RemoveNamedSelections();

	// remove discrete objects
	m_pModel->ClearDiscrete();

	// remove all steps
	m_pStep.Clear();

	// remove all materials
	DeleteAllMaterials();

	// remove all load controllers
	m_LC.Clear();

	// add an initial step
	m_pStep.Add(new FSInitialStep(this));
}

//-----------------------------------------------------------------------------
// clear the selections of all the bc, loads, etc.
void FSModel::ClearSelections()
{
	for (int i=0; i<Steps(); ++i)
	{
		FSStep* step = GetStep(i);

		for (int i=0; i<step->BCs(); ++i)
		{
			FSBoundaryCondition* pbc = step->BC(i);
			pbc->SetItemList(nullptr);
		}

		for (int i=0; i<step->Loads(); ++i)
		{
			FSLoad* pl = step->Load(i);
			pl->SetItemList(nullptr);
		}

		for (int i = 0; i<step->ICs(); ++i)
		{
			FSInitialCondition* pic = step->IC(i);
			FSItemListBuilder* pi = pic->GetItemList();
			pic->SetItemList(nullptr);
		}

		for (int i=0; i<step->Interfaces(); ++i)
		{
			FSInterface* pi = step->Interface(i);

			if (dynamic_cast<FSSoloInterface*>(pi))
			{
				FSSoloInterface* pc = dynamic_cast<FSSoloInterface*>(pi);
				pc->SetItemList(nullptr);
			}
			else if (dynamic_cast<FSPairedInterface*>(pi))
			{
				FSPairedInterface* pc = dynamic_cast<FSPairedInterface*>(pi);
				pc->SetPrimarySurface(nullptr);
				pc->SetSecondarySurface(nullptr);
			}
		}

		for (int i = 0; i < step->Constraints(); ++i)
		{
			FSModelConstraint* mc = step->Constraint(i);
			mc->SetItemList(nullptr);
		}
	}

	GetModel().RemoveNamedSelections();
}

void FSModel::RemoveUnusedItems()
{
	GModel& mdl = GetModel();
	mdl.RemoveUnusedSelections();
	RemoveUnusedMaterials();
	RemoveUnusedLoadControllers();
}

//-----------------------------------------------------------------------------
int FSModel::DataVariables()
{ 
	return (int)m_Var.Size(); 
}

//-----------------------------------------------------------------------------
FEDataVariable* FSModel::DataVariable(int i)
{ 
	return m_Var[i]; 
}

//-----------------------------------------------------------------------------
void FSModel::AddDataVariable(FEDataVariable* pv)
{ 
	m_Var.Add(pv); 
}

//-----------------------------------------------------------------------------
FEDataVariable* FSModel::FindDataVariable(int nid)
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
void FSModel::UpdateData()
{
	// update material fiber pointer for trans-iso materials
	for (int i=0; i<m_pModel->Objects(); ++i)
	{
		GObject* po = m_pModel->Object(i);
		FSMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int NP = po->Parts();
			for (int j=0; j<NP; ++j)
			{
				GPart& p = *po->Part(j);
				GMaterial* pgm = GetMaterialFromID(p.GetMaterialID());
				FSTransverselyIsotropic* pmat = (pgm?dynamic_cast<FSTransverselyIsotropic*>(pgm->GetMaterialProperties()):0);
				if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
				{
					FEDataVariable* pv = FindDataVariable(pmat->GetFiberMaterial()->m_nuser);
					int NE = pm->Elements();
					if (pv)
					{
						vec3d r;
						for (int n=0; n<NE; ++n)
						{
							FSElement& e = pm->Element(n);
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
void FSModel::AssignComponentToStep(FSStepComponent* pc, FSStep* ps)
{
	FSStep* po = FindStep(pc->GetStep());
	assert(po);
	if (po == 0) return;

	if (po != ps)
	{
		po->RemoveComponent(pc);
		ps->AddComponent(pc);
	}
}

//-----------------------------------------------------------------------------
// This function is used when reading FSGroup's that are not managed by an FSMesh.
// The FSGroup class reads the mesh ID and then the owner of the FSGroup calls
// this function to find the parent object (and mesh).
bool FSModel::FindGroupParent(FSGroup* pg)
{
	int obj_id = pg->GetMeshID();
	if (obj_id == -1) return false;
	else
	{
		GObject* po = GetModel().FindObject(obj_id);
		if ((po == nullptr) || (po->GetFEMesh() == nullptr)) return false;
		pg->SetMesh(po->GetFEMesh());
		return true;
	}
}

//-----------------------------------------------------------------------------
int FSModel::CountBCs(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i=0; i<NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NBC = step->BCs();
		for (int j=0; j<NBC; ++j)
		{
			FSBoundaryCondition* pbc = step->BC(j);
			if (pbc->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FSModel::CountLoads(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NL = step->Loads();
		for (int j = 0; j<NL; ++j)
		{
			FSLoad* pl = step->Load(j);
			if (pl->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FSModel::CountICs(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NL = step->ICs();
		for (int j = 0; j<NL; ++j)
		{
			FSInitialCondition* pbc = step->IC(j);
			if (pbc->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FSModel::CountInterfaces(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NC = step->Interfaces();
		for (int j = 0; j<NC; ++j)
		{
			FSInterface* pi = step->Interface(j);
			if (pi->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FSModel::CountRigidConstraints(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i<NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NRC = step->RigidConstraints();
		for (int j = 0; j<NRC; ++j)
		{
			FSRigidConstraint* prc = step->RigidConstraint(j);
			if (prc->Type() == type) n++;
		}
	}
	return n;
}

//-----------------------------------------------------------------------------
int FSModel::CountRigidConnectors(int type)
{
	int n = 0;
	int NSTEPS = Steps();
	for (int i = 0; i < NSTEPS; ++i)
	{
		FSStep* step = GetStep(i);

		int NRC = step->RigidConnectors();
		for (int j = 0; j < NRC; ++j)
		{
			FSRigidConnector* prc = step->RigidConnector(j);
			if (prc->Type() == type) n++;
		}
	}
	return n;
}

void FSModel::SetSkipGeometry(bool skip)
{
    m_skipGeometry = skip;
}

int FSModel::LoadControllers() const
{
	return (int)m_LC.Size();
}

FSLoadController* FSModel::GetLoadController(int i)
{
	return m_LC[i];
}

FSLoadController* FSModel::GetLoadControllerFromID(int lc)
{
	if (lc < 0) return nullptr;
	for (int i = 0; i < m_LC.Size(); ++i)
	{
		FSLoadController* plc = m_LC[i];
		if (plc->GetID() == lc) return plc;
	}
	return nullptr;
}

void FSModel::AddLoadController(FSLoadController* plc)
{
	m_LC.Add(plc);
}

int FSModel::RemoveLoadController(FSLoadController* plc)
{
	return (int) m_LC.Remove(plc);
}

// helper function for creating load curves (returns UID of load controller)
FSLoadController* FSModel::AddLoadCurve(LoadCurve& lc)
{
	// allocate load curve
	FSLoadController* plc = FEBio::CreateLoadController("loadcurve", this);

	// set default name
	std::string name;
	const char* szname = lc.GetName();
	if ((szname == nullptr) || (szname[0] == 0)) 
	{
		std::stringstream ss;
		ss << "LC" << LoadControllers() + 1;
		name = ss.str();
	}
	else name = szname;
	plc->SetName(name);

	// set parameters
	plc->SetParamInt("interpolate", lc.GetInterpolator());
	plc->SetParamInt("extend", lc.GetExtendMode());

	// copy point data
	std::vector<vec2d> pt;
	for (int i = 0; i < lc.Points(); ++i)
	{
		vec2d pi = lc.Point(i);
		pt.push_back(pi);
	}
	Param& points = *plc->GetParam("points");
	points.val<std::vector<vec2d> >() = pt;

	// add it to the pile
	AddLoadController(plc);

	return plc;
}

//----------------------------------------------------------------------------------------
void UpdateLCRefsCount(FSModelComponent* pmc, std::map<int, int>& LCT)
{
	if (pmc == nullptr) return;

	for (int n = 0; n < pmc->Parameters(); ++n)
	{
		Param& p = pmc->GetParam(n);
		if (p.GetLoadCurveID() > 0)
		{
			LCT[p.GetLoadCurveID()]++;
		}
	}

	for (int m = 0; m < pmc->Properties(); ++m)
	{
		FSProperty& prop = pmc->GetProperty(m);
		for (int k = 0; k < prop.Size(); ++k)
		{
			FSModelComponent* pmk = dynamic_cast<FSModelComponent*>(prop.GetComponent(k));
			if (pmk) UpdateLCRefsCount(pmk, LCT);
		}
	}
}

//----------------------------------------------------------------------------------------
void FSModel::UpdateLoadControllerReferenceCounts()
{
	// clear all reference counters
	int NLC = LoadControllers();
	std::map<int, int> LCT;
	for (int i = 0; i < NLC; ++i)
	{
		FSLoadController* plc = GetLoadController(i);
		plc->SetReferenceCount(0);
		LCT[plc->GetID()] = 0;
	}

	// process materials
	for (int i = 0; i < Materials(); ++i)
	{
		GMaterial* mat = GetMaterial(i);
		FSMaterial* pm = mat->GetMaterialProperties();
		if (pm) UpdateLCRefsCount(pm, LCT);
	}

	// process discrete
	GModel& gm = GetModel();
	for (int i=0; i<gm.DiscreteObjects(); ++i)
	{ 
		GDiscreteSpringSet* po = dynamic_cast<GDiscreteSpringSet*>(gm.DiscreteObject(i));
		if (po && po->GetMaterial()) UpdateLCRefsCount(po->GetMaterial(), LCT);
	}

	// process Steps
	for (int n = 0; n < Steps(); ++n)
	{
		FSStep* step = GetStep(n);
		UpdateLCRefsCount(step, LCT);

		// process BCs
		for (int i = 0; i < step->BCs(); ++i)
		{
			FSBoundaryCondition* pbc = step->BC(i);
			UpdateLCRefsCount(pbc, LCT);
		}

		// process Loads
		for (int i = 0; i < step->Loads(); ++i)
		{
			FSLoad* pload = step->Load(i);
			UpdateLCRefsCount(pload, LCT);
		}

		// process contact
		for (int i = 0; i < step->Interfaces(); ++i)
		{
			FSInterface* pi = step->Interface(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// nonlinear constraints
		for (int i = 0; i < step->Constraints(); ++i)
		{
			FSModelConstraint* pi = step->Constraint(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// rigid BC
		for (int i = 0; i < step->RigidBCs(); ++i)
		{
			FSRigidBC* pi = step->RigidBC(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// rigid load
		for (int i = 0; i < step->RigidLoads(); ++i)
		{
			FSRigidLoad* pi = step->RigidLoad(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// rigid constraints
		for (int i = 0; i < step->RigidConstraints(); ++i)
		{
			FSRigidConstraint* pi = step->RigidConstraint(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// rigid connector
		for (int i = 0; i < step->RigidConnectors(); ++i)
		{
			FSRigidConnector* pi = step->RigidConnector(i);
			UpdateLCRefsCount(pi, LCT);
		}

		// mesh adaptor
		for (int i = 0; i < step->MeshAdaptors(); ++i)
		{
			FSMeshAdaptor* pi = step->MeshAdaptor(i);
			UpdateLCRefsCount(pi, LCT);
		}
	}

	// update reference counts
	for (int i = 0; i < NLC; ++i)
	{
		FSLoadController* plc = GetLoadController(i);
		plc->SetReferenceCount(LCT[plc->GetID()]);
	}
}


//----------------------------------------------------------------------------------------
int FSModel::MeshDataGenerators() const
{
	return m_MD.Size();
}

FSMeshDataGenerator* FSModel::GetMeshDataGenerator(int i)
{
	return m_MD[i];
}

void FSModel::AddMeshDataGenerator(FSMeshDataGenerator* pmd)
{
	m_MD.Add(pmd);
}

int FSModel::RemoveMeshDataGenerator(FSMeshDataGenerator* pmd)
{
	return (int)m_MD.Remove(pmd);
}

void GetAllocatorIDsRecursive(FSCoreBase* obj, std::unordered_set<int>& allocatorIDs)
{
    if ((obj == nullptr) || (obj->GetClassID() == -1)) return;

    FEBio::FEBioClassInfo ci = FEBio::GetClassInfo(obj->GetClassID());
    allocatorIDs.insert(ci.allocId);

    // check properties
    for (int i = 0; i < obj->Properties(); ++i)
    {
        FSProperty& prop = obj->GetProperty(i);
        for (int j = 0; j < prop.Size(); ++j)
        {
            GetAllocatorIDsRecursive(prop.GetComponent(j), allocatorIDs);
        }
    }
}

void FSModel::GetActivePluginIDs(std::unordered_set<int>& allocatorIDs)
{
    // Materials
    for(int index = 0; index < m_pMat.Size(); index++)
    {
        GMaterial* pgm = m_pMat[index];
        FSMaterial* pmat = pgm->GetMaterialProperties();
        if (pmat)
        {
            GetAllocatorIDsRecursive(pmat, allocatorIDs);
        }
    }

    // Steps
    for (int index = 0; index < Steps(); ++index)
    {
        FSStep* ps = GetStep(index);
        // GetAllocatorIDsRecursive(ps, allocatorIDs);

        // Boundary Conditions
        for (int j = 0; j < ps->BCs(); ++j)
        {
            FSBoundaryCondition* pbc = ps->BC(j);
            GetAllocatorIDsRecursive(pbc, allocatorIDs);
        }

        // Loads
        for (int j = 0; j < ps->Loads(); ++j)
        {
            FSLoad* pl = ps->Load(j);
            GetAllocatorIDsRecursive(pl, allocatorIDs);
        }

        // Initial Conditions
        for (int j = 0; j < ps->ICs(); ++j)
        {
            FSInitialCondition* pic = ps->IC(j);
            GetAllocatorIDsRecursive(pic, allocatorIDs);
        }

        // Contact Interfaces
        for (int j = 0; j < ps->Interfaces(); ++j)
        {
            FSInterface* pi = ps->Interface(j);
            GetAllocatorIDsRecursive(pi, allocatorIDs);
        }

        // Non-linear Constraints
        for (int j = 0; j < ps->Constraints(); ++j)
        {
            FSModelConstraint* pmc = ps->Constraint(j);
            GetAllocatorIDsRecursive(pmc, allocatorIDs);
        }

        // Rigid Constraints
        for (int j = 0; j < ps->RigidConstraints(); ++j)
        {
            FSRigidConstraint* prc = ps->RigidConstraint(j);
            GetAllocatorIDsRecursive(prc, allocatorIDs);
        }

        // Rigid Loads
        for (int j = 0; j < ps->RigidLoads(); ++j)
        {
            FSRigidLoad* prl = ps->RigidLoad(j);
            GetAllocatorIDsRecursive(prl, allocatorIDs);
        }

        // Rigid Boundary Conditions
        for (int j = 0; j < ps->RigidBCs(); ++j)
        {
            FSRigidBC* prb = ps->RigidBC(j);
            GetAllocatorIDsRecursive(prb, allocatorIDs);
        }   

        // Rigid Initial Conditions
        for (int j = 0; j < ps->RigidICs(); ++j)
        {
            FSRigidIC* pic = ps->RigidIC(j);
            GetAllocatorIDsRecursive(pic, allocatorIDs);
        }

        // Rigid Connectors
        for (int j = 0; j < ps->RigidConnectors(); ++j)
        {
            FSRigidConnector* prc = ps->RigidConnector(j);
            GetAllocatorIDsRecursive(prc, allocatorIDs);
        }

        // Mesh Adaptors
        for (int j = 0; j < ps->MeshAdaptors(); ++j)
        {
            FSMeshAdaptor* pma = ps->MeshAdaptor(j);
            GetAllocatorIDsRecursive(pma, allocatorIDs);
        }
    }

    // FSLoadControllers
    for (int index = 0; index < LoadControllers(); ++index)
    {
        FSLoadController* plc = GetLoadController(index);
        GetAllocatorIDsRecursive(plc, allocatorIDs);
    }

    // FSMeshDataGenerators
    for (int index = 0; index < MeshDataGenerators(); ++index)
    {
        FSMeshDataGenerator* pmd = GetMeshDataGenerator(index);
        GetAllocatorIDsRecursive(pmd, allocatorIDs);
    }
}

//----------------------------------------------------------------------------------------
int CountBCsByTypeString(const std::string& typeStr, FSModel& fem)
{
	int nc = 0;
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FSStep* ps = fem.GetStep(i);
		for (int j = 0; j < ps->BCs(); ++j)
		{
			FSBoundaryCondition* pbc = ps->BC(j);
			string t = pbc->GetTypeString();
			if (t == typeStr) nc++;
		}
	}
	return nc;
}
