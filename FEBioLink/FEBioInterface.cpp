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
#include "FEBioInterface.h"
#include "FEBioClass.h"
#include "FEBioModule.h"
#include <FEMLib/FEStepComponent.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEMaterial.h>
#include <FEMLib/FEAnalysisStep.h>
#include <FEMLib/FEInterface.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FEConnector.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEDiscreteMaterial.h>
#include <sstream>
using namespace std;

mat3ds qvariant_to_mat3ds(const QVariant& v)
{
	QList<QVariant> val = v.value<QList<QVariant> >();
	mat3ds m;
	m.xx() = val.at(0).toDouble();
	m.yy() = val.at(1).toDouble();
	m.zz() = val.at(2).toDouble();
	m.xy() = val.at(3).toDouble();
	m.yz() = val.at(4).toDouble();
	m.xz() = val.at(5).toDouble();
	return m;
}

void map_parameters(FSObject* po, FEBio::FEBioClass* feb)
{
	// copy the parameters from the FEBioClass to the FSObject
	for (int i = 0; i < feb->Parameters(); ++i)
	{
		FEBio::FEBioParam& param = feb->GetParameter(i);
		QVariant v = param.m_val;

		int type = param.type();

		// TODO: The name needs to be copied in the FSObject class!! 
		//       This is a memory leak!!!!
		const char* szname = strdup(param.name().c_str());
		const char* szlongname = strdup(param.longName().c_str());
		Param* p = nullptr;
		switch (type)
		{
		case FEBio::FEBIO_PARAM_INT   : 
		{
			if (param.m_enums)
			{
				p = po->AddChoiceParam(v.toInt(), szname, szlongname);
				p->CopyEnumNames(param.m_enums);
			}
			else p = po->AddIntParam(v.toInt(), szname, szlongname);
		}
		break;
		case FEBio::FEBIO_PARAM_BOOL          : p = po->AddBoolParam(v.toBool(), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_DOUBLE        : p = po->AddDoubleParam(v.toDouble(), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_VEC3D         : p = po->AddVecParam(qvariant_to_vec3d(v), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_MAT3D         : p = po->AddMat3dParam(qvariant_to_mat3d(v), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_STD_STRING    : p = po->AddStringParam(v.toString().toStdString(), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_DOUBLE_MAPPED : p = po->AddDoubleParam(v.toDouble(), szname, szlongname)->MakeVariable(true); break;
		case FEBio::FEBIO_PARAM_VEC3D_MAPPED  : p = po->AddVecParam(qvariant_to_vec3d(v), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_MAT3D_MAPPED  : p = po->AddMat3dParam(qvariant_to_mat3d(v), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_MAT3DS_MAPPED : p = po->AddMat3dsParam(qvariant_to_mat3ds(v), szname, szlongname); break;
		case FEBio::FEBIO_PARAM_STD_VECTOR_INT:
		{
			std::vector<int> val = v.value<std::vector<int> >();
			p = po->AddVectorIntParam(val, szname, szlongname);
			if (param.m_enums) p->CopyEnumNames(param.m_enums);
		}
		break;
		case FEBio::FEBIO_PARAM_STD_VECTOR_DOUBLE:
		{
			std::vector<double> val = v.value<std::vector<double> >();
			p = po->AddVectorDoubleParam(val, szname, szlongname);
		}
		break;
		default:
			assert(false);
		}
		assert(p);
		if (p)
		{
			p->SetFlags(param.m_flags);
			if (param.m_flags & 0x08) p->SetLoadCurve();
			if (param.m_szunit) p->SetUnit(param.m_szunit);
		}
	}
}

void map_parameters(FEBio::FEBioClass* feb, FSObject* po)
{
	assert(feb->Parameters() == po->Parameters());
	int NP = feb->Parameters();
	for (int i = 0; i < NP; ++i)
	{
		FEBio::FEBioParam& febParam = feb->GetParameter(i);
		Param& fsParam = po->GetParam(i);

		switch (febParam.type())
		{
		case FEBio::FEBIO_PARAM_VEC3D:
		{
			vec3d v = fsParam.GetVec3dValue();
			febParam.m_val = vec3d_to_qvariant(v);
		}
		break;
		case FEBio::FEBIO_PARAM_STD_STRING:
		{
			string s = fsParam.GetStringValue();
			febParam.m_val = QString::fromStdString(s);
		}
		break;
		case FEBio::FEBIO_PARAM_DOUBLE_MAPPED:
		{
			if (fsParam.GetParamType() == Param_FLOAT)
			{
				double v = fsParam.GetFloatValue();
				febParam.m_val = v;
			}
		}
		break;
		}
	}
}

bool FEBio::CreateModelComponent(int classId, FSModelComponent* po)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return false;

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the FEBioClass parameters to the FSObject
	map_parameters(po, feb);

	// map the properties
	if (dynamic_cast<FSStepComponent*>(po))
	{
		FSStepComponent* pc = dynamic_cast<FSStepComponent*>(po);
		for (int i = 0; i < feb->Properties(); ++i)
		{
			// TODO: mapped vectors are added as properties, but they should be parameters instead. 
			//       This is a bit of a hack that I need to clean up.
			FEBio::FEBioProperty& prop = feb->GetProperty(i);
			int maxSize = (prop.m_isArray ? 0 : 1);
			if (prop.m_superClassId == FE_VECTORGENERATOR)
			{
				po->AddVecParam(vec3d(0, 0, 0), prop.m_name.c_str());
			}
		}
	}

	if (dynamic_cast<FEBioMaterial*>(po))
	{
		FEBioMaterial* febMat = dynamic_cast<FEBioMaterial*>(po);
		febMat->SetFEBioMaterial(feb);
	}
	else if (dynamic_cast<FSDomainComponent*>(po))
	{
		FSDomainComponent* pbc = dynamic_cast<FSDomainComponent*>(po);
		if (feb->FindProperty("surface"))
		{
			pbc->SetMeshItemType(FE_FACE_FLAG);
		}
	}
	else delete feb;

	return true;
}

bool FEBio::CreateModelComponent(int superClassId, const std::string& typeStr, FSModelComponent* po)
{
	bool ret = true;

	if (superClassId == FE_MATERIAL)
	{
		FEBioMaterial* pmat = dynamic_cast<FEBioMaterial*>(po); assert(pmat);
		ret = CreateMaterial(typeStr.c_str(), pmat);
	}
	else if (superClassId == FE_MATERIALPROP)
	{
		FEBioMaterial* pmat = dynamic_cast<FEBioMaterial*>(po); assert(pmat);
		CreateMaterialProperty(FE_MATERIALPROP, typeStr.c_str(), pmat);
	}
	else if (superClassId == FE_DISCRETE_MATERIAL)
	{
		FEBioDiscreteMaterial* pmat = dynamic_cast<FEBioDiscreteMaterial*>(po); assert(pmat);
		CreateDiscreteMaterial(FE_DISCRETE_MATERIAL, typeStr.c_str(), pmat);
	}
	else if (superClassId == FE_ANALYSIS)
	{
		FEStep* pstep = dynamic_cast<FEStep*>(po);
		CreateStep(typeStr.c_str(), pstep);
	}
	else
	{
		int classId = FEBio::GetClassId(superClassId, typeStr); assert(classId);
		ret = CreateModelComponent(classId, po);
	}

	return ret;
}

void FEBio::CreateStep(const char* sztype, FEStep* po)
{
	int classId = FEBio::GetClassId(FE_ANALYSIS, sztype); assert(classId);
	CreateStep(classId, po, false);
}

void FEBio::CreateStep(int classId, FEStep* po, bool initDefaultProps)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return;

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the FEBioClass parameters to the FSObject
	map_parameters(po, feb);

	// get the active module
	int modId = FEBio::GetActiveModule();

	// map the properties
	for (int i = 0; i < feb->Properties(); ++i)
	{
		FEBio::FEBioProperty& prop = feb->GetProperty(i);

		FEStepControlProperty* pc = new FEStepControlProperty;

		string name = prop.m_name;
		pc->m_brequired = prop.m_brequired;

		if (initDefaultProps && pc->m_brequired)
		{
			vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllClasses(modId, prop.m_superClassId, -1, ClassSearchFlags::IncludeFECoreClasses);
			if (fci.size() > 0)
			{
				FSStepComponent* psc = new FSStepComponent;
				CreateModelComponent(fci[0].classId, psc);
				pc->m_prop = psc;
			}
		}
		
		pc->SetName(name);
		pc->m_nClassID = prop.m_baseClassId;
		pc->m_nSuperClassId = prop.m_superClassId;
		po->AddControlProperty(pc);
	}

	// don't forget to cleanup
	delete feb;
}

bool FEBio::CreateMaterial(const char* sztype, FEBioMaterial* po)
{
	int classId = FEBio::GetClassId(FE_MATERIAL, sztype); assert(classId >= 0);
	if (classId < 0) return false;
	CreateMaterial(classId, po);
	return true;
}

void FEBio::CreateMaterialProperty(int superClassID, const char* sztype, FEBioMaterial* po)
{
	int classId = FEBio::GetClassId(superClassID, sztype); assert(classId != -1);

	// If we found it, allocate the material
	if (classId >= 0) CreateMaterial(classId, po);
}

bool FEBio::CreateDiscreteMaterial(int superClassID, const char* sztype, FEBioDiscreteMaterial* po)
{
	int classId = FEBio::GetClassId(superClassID, sztype); assert(classId);
	return CreateModelComponent(classId, po);
}

void FEBio::CreateMaterial(int classId, FEBioMaterial* po)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return;

	// check the super class ID
	int superClassID = feb->GetSuperClassID();

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));
	po->SetSuperClassID(superClassID);

	// pass the FEBio object to the FEBio Studio object
	po->SetFEBioMaterial(feb);

	// map the parameters
	map_parameters(po, feb);

	// map the properties
	for (int i = 0; i < feb->Properties(); ++i)
	{
		FEBio::FEBioProperty& prop = feb->GetProperty(i);
		int maxSize = (prop.m_isArray ? 0 : 1);
		FEMaterialProperty* matProp = po->AddProperty(prop.m_name, prop.m_baseClassId + FE_FEBIO_MATERIAL_CLASS, maxSize); assert(matProp);
		matProp->SetSuperClassID(prop.m_superClassId);
		if (prop.m_brequired)
			matProp->SetFlags(matProp->GetFlags() | FEMaterialProperty::REQUIRED);
		matProp->SetDefaultType(prop.m_defType);

		if (prop.m_comp.empty() == false)
		{
			FEBioClass& fbc = prop.m_comp[0];
			FEBioMaterial* pmi = new FEBioMaterial;
			pmi->SetTypeString(strdup(fbc.TypeString().c_str()));
			pmi->SetSuperClassID(fbc.GetSuperClassID());
			pmi->SetFEBioMaterial(&fbc);
			map_parameters(pmi, &fbc);
			matProp->AddMaterial(pmi);
		}
	}
}

void FEBio::UpdateFEBioMaterial(FEBioMaterial* pm)
{
	FEBioClass* febClass = pm->GetFEBioMaterial();

	// first map the parameters to the FEBioClass
	map_parameters(febClass, pm);

	// then write the parameters to the FEBio class
	febClass->UpdateData();
}

void FEBio::UpdateFEBioDiscreteMaterial(FEBioDiscreteMaterial* pm)
{
	FEBioClass* febClass = pm->GetFEBioMaterial();

	// first map the parameters to the FEBioClass
	map_parameters(febClass, pm);

	// then write the parameters to the FEBio class
	febClass->UpdateData();
}

FSMaterial* FEBio::CreateMaterial(const char* sztype, FSModel* fem)
{
	FEBioMaterial* pmat = new FEBioMaterial;
	if (CreateMaterial(sztype, pmat) == false)
	{
		delete pmat;
		return nullptr;
	}
	return pmat;
}

FSBoundaryCondition* FEBio::CreateBoundaryCondition(const char* sztype, FSModel* fem)
{
	FEBioBoundaryCondition* pbc = new FEBioBoundaryCondition(fem);
	if (FEBio::CreateModelComponent(FE_ESSENTIAL_BC, sztype, pbc) == false)
	{
		delete pbc;
		return nullptr;
	}
	return pbc;
}

FENodalLoad* FEBio::CreateNodalLoad(const char* sztype, FSModel* fem)
{
	FEBioNodalLoad* pnl = new FEBioNodalLoad(fem);
	if (FEBio::CreateModelComponent(FE_NODAL_LOAD, sztype, pnl) == false)
	{
		delete pnl;
		return nullptr;
	}
	return pnl;
}

FESurfaceLoad* FEBio::CreateSurfaceLoad(const char* sztype, FSModel* fem)
{
	FEBioSurfaceLoad* psl = new FEBioSurfaceLoad(fem);
	if (FEBio::CreateModelComponent(FE_SURFACE_LOAD, sztype, psl) == false)
	{
		delete psl;
		return nullptr;
	}
	return psl;
}

FEBodyLoad* FEBio::CreateBodyLoad(const char* sztype, FSModel* fem)
{
	FEBioBodyLoad* pbl = new FEBioBodyLoad(fem);
	if (FEBio::CreateModelComponent(FE_BODY_LOAD, sztype, pbl) == false)
	{
		delete pbl;
		return nullptr;
	}
	return pbl;
}

FEPairedInterface* FEBio::CreatePairedInterface(const char* sztype, FSModel* fem)
{
	FEPairedInterface* pci = new FEBioInterface(fem);
	if (CreateModelComponent(FE_INTERFACE, sztype, pci) == false)
	{
		delete pci;
		return nullptr;
	}
	return pci;
}

FEModelConstraint* FEBio::CreateNLConstraint(const char* sztype, FSModel* fem)
{
	FEModelConstraint* pmc = new FEBioNLConstraint(fem);
	if (CreateModelComponent(FE_CONSTRAINT, sztype, pmc) == false)
	{
		delete pmc;
		return nullptr;
	}
	return pmc;
}

FSRigidConstraint* FEBio::CreateRigidConstraint(const char* sztype, FSModel* fem)
{
	FSRigidConstraint* pi = new FEBioRigidConstraint(fem);
	if (FEBio::CreateModelComponent(FE_RIGID_CONSTRAINT, sztype, pi) == false)
	{
		delete pi;
		return nullptr;
	}
	return pi;
}

FSRigidConnector* FEBio::CreateRigidConnector(const char* sztype, FSModel* fem)
{
	FSRigidConnector* pi = new FEBioRigidConnector(fem);
	if (FEBio::CreateModelComponent(FE_RIGID_CONNECTOR, sztype, pi) == false)
	{
		delete pi;
		return nullptr;
	}
	return pi;
}

FEInitialCondition* FEBio::CreateInitialCondition(const char* sztype, FSModel* fem)
{
	FEInitialCondition* pic = new FEBioInitialCondition(fem);
	if (FEBio::CreateModelComponent(FE_INITIAL_CONDITION, sztype, pic) == false)
	{
		delete pic;
		return nullptr;
	}
	return pic;
}
