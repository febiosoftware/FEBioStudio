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
#include <FEMLib/FEDiscreteMaterial.h>
#include <FEMLib/FEAnalysisStep.h>
#include <FEMLib/FEInterface.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FEConnector.h>
#include <FEMLib/FERigidLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FEDiscreteMaterial.h>
#include <MeshTools/FEModel.h>
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

void map_parameters(FSModelComponent* po, FEBio::FEBioClass* feb)
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
		case FEBio::FEBIO_PARAM_STD_VECTOR_VEC2D:
		{
			std::vector<vec2d> val = v.value<std::vector<vec2d> >();
			p = po->AddVectorVec2dParam(val, szname, szlongname);
		}
		break;
		default:
			assert(false);
		}
		assert(p);
		if (p)
		{
			p->SetFlags(param.m_flags);
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

bool BuildModelComponent(FEBio::FEBioClass* feb, FSModelComponent* po)
{
	assert(po->GetSuperClassID() == feb->GetSuperClassID());

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the FEBioClass parameters to the FSObject
	map_parameters(po, feb);

	// map the properties
	for (int i = 0; i < feb->Properties(); ++i)
	{
		// TODO: mapped vectors are added as properties, but they should be parameters instead. 
		//       This is a bit of a hack that I need to clean up.
		FEBio::FEBioProperty& prop = feb->GetProperty(i);
		int maxSize = (prop.m_isArray ? 0 : 1);
		FSProperty* pci = po->AddProperty(prop.m_name, prop.m_baseClassId, maxSize); assert(pci);
		pci->SetSuperClassID(prop.m_superClassId);
		if (prop.m_brequired)
			pci->SetFlags(pci->GetFlags() | FSProperty::REQUIRED);
		pci->SetDefaultType(prop.m_defType);

		// NOTE: is this ever true?
		if (prop.m_comp.empty() == false)
		{
			FEBio::FEBioClass& fbc = prop.m_comp[0];
			FSCoreBase* pmi = FEBio::CreateClass(fbc.GetSuperClassID(), fbc.TypeString().c_str(), nullptr);
			pci->AddComponent(pmi);
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

bool BuildModelComponent(FSModelComponent* po)
{
	// create the FEBioClass object
	int classId = po->GetClassID();
	FEBio::FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return false;
	return BuildModelComponent(feb, po);
}

// Call this to initialize default properties
bool FEBio::InitDefaultProps(FSModelComponent* pc)
{
	for (int i = 0; i < pc->Properties(); ++i)
	{
		FSProperty& prop = pc->GetProperty(i);
		if (prop.IsRequired() && (prop.GetComponent() == nullptr))
		{
			vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllActiveClasses(prop.GetSuperClassID(), -1, FEBio::ClassSearchFlags::IncludeFECoreClasses);
			if (fci.size() > 0)
			{
				FSModel* fem = pc->GetFSModel();
				FSModelComponent* psc = FEBio::CreateClass(prop.GetSuperClassID(), prop.GetDefaultType().c_str(), fem);
				assert(psc);
				if (psc)
				{
					prop.AddComponent(psc);
					bool b = InitDefaultProps(psc);
					if (b == false) { assert(false); return false; }
				}
				else return false;
			}
		}
	}
	return true;
}

bool BuildModelComponent(int superClassId, const std::string& typeStr, FSModelComponent* po)
{
	int classId = FEBio::GetClassId(superClassId, typeStr); assert(classId > 0);
	po->SetSuperClassID(superClassId);
	po->SetClassID(classId);
	po->SetTypeString(typeStr);
	bool ret = BuildModelComponent(po);
	return ret;
}

bool FEBio::BuildModelComponent(FSModelComponent* pc, const std::string& typeStr)
{
	return BuildModelComponent(pc->GetSuperClassID(), typeStr, pc);
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

template <class T> T* CreateModelComponent(int superClassID, const std::string& typeStr, FSModel* fem)
{
	T* mc = new T(fem);
	if (BuildModelComponent(superClassID, typeStr, mc) == false)
	{
		assert(false);
		delete mc;
		return nullptr;
	}
	return mc;
}

template <class T> T* CreateModelComponent(int superClassID, const std::string& typeStr)
{
	T* mc = new T;
	if (BuildModelComponent(superClassID, typeStr, mc) == false)
	{
		assert(false);
		delete mc;
		return nullptr;
	}
	return mc;
}


FSStep* FEBio::CreateStep(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioAnalysisStep>(FEANALYSIS_ID, typeStr, fem);
}

FSMaterial* FEBio::CreateMaterial(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioMaterial>(FEMATERIAL_ID, typeStr);
}

FSMaterialProperty* FEBio::CreateMaterialProperty(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioMaterialProperty>(FEMATERIALPROP_ID, typeStr, fem);
}

FSDiscreteMaterial* FEBio::CreateDiscreteMaterial(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioDiscreteMaterial>(FEDISCRETEMATERIAL_ID, typeStr);
}

FSBoundaryCondition* FEBio::CreateBoundaryCondition(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioBoundaryCondition>(FEBC_ID, typeStr, fem);
}

FSNodalLoad* FEBio::CreateNodalLoad(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioNodalLoad>(FENODALLOAD_ID, typeStr, fem);
}

FSSurfaceLoad* FEBio::CreateSurfaceLoad(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioSurfaceLoad>(FESURFACELOAD_ID, typeStr, fem);
}

FSBodyLoad* FEBio::CreateBodyLoad(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioBodyLoad>(FEBODYLOAD_ID, typeStr, fem);
}

FSPairedInterface* FEBio::CreatePairedInterface(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioInterface>(FESURFACEINTERFACE_ID, typeStr, fem);
}

FSModelConstraint* FEBio::CreateNLConstraint(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioNLConstraint>(FENLCONSTRAINT_ID, typeStr, fem);
}

FSRigidConstraint* FEBio::CreateRigidConstraint(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioRigidConstraint>(FERIGIDBC_ID, typeStr, fem);
}

FSRigidConnector* FEBio::CreateRigidConnector(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioRigidConnector>(FERIGIDCONNECTOR_ID, typeStr, fem);
}

FSRigidLoad* FEBio::CreateRigidLoad(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioRigidLoad>(FERIGIDLOAD_ID, typeStr, fem);
}

FSInitialCondition* FEBio::CreateInitialCondition(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioInitialCondition>(FEIC_ID, typeStr, fem);
}

FSLoadController* FEBio::CreateLoadController(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioLoadController>(FELOADCONTROLLER_ID, typeStr, fem);
}

FSFunction1D* FEBio::CreateFunction1D(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FEBioFunction1D>(FEFUNCTION1D_ID, typeStr, fem);
}

FSGenericClass* FEBio::CreateGenericClass(const std::string& typeStr, FSModel* fem)
{
	return CreateModelComponent<FSGenericClass>(FECLASS_ID, typeStr);
}

FSModelComponent* FEBio::CreateClass(int superClassID, const std::string& typeStr, FSModel* fem)
{
	switch (superClassID)
	{
	case FEMATERIAL_ID        : return CreateMaterial        (typeStr, fem); break;
	case FEMATERIALPROP_ID    : return CreateMaterialProperty(typeStr, fem); break;
	case FEDISCRETEMATERIAL_ID: return CreateDiscreteMaterial(typeStr, fem); break;
	case FECLASS_ID           : return CreateGenericClass    (typeStr, fem); break;
	case FELOADCONTROLLER_ID  : return CreateLoadController  (typeStr, fem); break;
	case FEFUNCTION1D_ID      : return CreateFunction1D      (typeStr, fem); break;
	case FESOLVER_ID          :
	case FENEWTONSTRATEGY_ID  :
	case FETIMECONTROLLER_ID  :
	case FEVEC3DGENERATOR_ID  :
	case FEMAT3DGENERATOR_ID  :
	case FEMAT3DSGENERATOR_ID :
	{
		FSGenericClass* pc = new FSGenericClass;
		BuildModelComponent(superClassID, typeStr, pc);
		return pc;
	}
	break;
	default:
		assert(false);
	}
	return nullptr;
}

FSModelComponent* FEBio::CreateClass(int classId, FSModel* fem)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return nullptr;

	// allocate correct FBS class. 
	FSModelComponent* pc = nullptr;
	switch (feb->GetSuperClassID())
	{
	case FEANALYSIS_ID            : pc = new FEBioAnalysisStep(fem); break;
	case FEFUNCTION1D_ID          : pc = new FEBioFunction1D(fem); break;
	case FEMATERIAL_ID            : pc = new FEBioMaterial(); break;
	case FEBC_ID                  : pc = new FEBioBoundaryCondition(fem); break;
	case FEIC_ID                  : pc = new FEBioInitialCondition(fem); break;
	case FESURFACEINTERFACE_ID    : pc = new FEBioInterface(fem); break;
	case FENODALLOAD_ID           : pc = new FEBioNodalLoad(fem); break;
	case FESURFACELOAD_ID         : pc = new FEBioSurfaceLoad(fem); break;
	case FEBODYLOAD_ID            : pc = new FEBioBodyLoad(fem); break;
	case FEMATERIALPROP_ID        : pc = new FEBioMaterialProperty(fem); break;
	case FELOADCONTROLLER_ID      : pc = new FEBioLoadController(fem); break;
	case FEMESHADAPTOR_ID         : pc = new FEBioMeshAdaptor(fem); break;
	case FENLCONSTRAINT_ID        : pc = new FEBioNLConstraint(fem); break;
	case FESOLVER_ID              : pc = new FSGenericClass; break;
	case FEMESHADAPTORCRITERION_ID: pc = new FSGenericClass; break;
	case FENEWTONSTRATEGY_ID      : pc = new FSGenericClass; break;
	case FECLASS_ID               : pc = new FSGenericClass; break;
	case FETIMECONTROLLER_ID      : pc = new FSGenericClass; break;
	break;
	default:
		assert(false);
	}

	pc->SetClassID(classId);
	pc->SetSuperClassID(feb->GetSuperClassID());
	pc->SetTypeString(feb->TypeString());
	BuildModelComponent(feb, pc);

	return pc;
}

