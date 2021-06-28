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
#include "stdafx.h"
#include "FEBioInterface.h"
#include "FEBioClass.h"
#include <FEMLib/FEStepComponent.h>
#include <FEMLib/FEMaterial.h>
#include <FEMLib/FEAnalysisStep.h>
#include <sstream>
using namespace std;

vec3d qvariant_to_vec3d(const QVariant& v)
{
	QList<QVariant> val = v.value<QList<QVariant> >();
	vec3d w;
	w.x = val.at(0).toDouble();
	w.y = val.at(1).toDouble();
	w.z = val.at(2).toDouble();
	return w;
}

mat3d qvariant_to_mat3d(const QVariant& v)
{
	QList<QVariant> val = v.value<QList<QVariant> >();
	mat3d w;
	int n = 0;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j) w[i][j] = val.at(n++).toDouble();
	return w;
}

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

		// TODO: The name needs to copied in the FSObject class!! 
		const char* szname = strdup(param.name().c_str());

		switch (type)
		{
		case FEBio::FEBIO_PARAM_INT   : 
		{
			Param* p = po->AddIntParam(v.toInt(), szname);
			if (param.m_enums)
			{
				p->CopyEnumNames(param.m_enums);
			}
		}
		break;
		case FEBio::FEBIO_PARAM_BOOL  : po->AddBoolParam(v.toBool(), szname); break;
		case FEBio::FEBIO_PARAM_DOUBLE: po->AddDoubleParam(v.toDouble(), szname); break;
		case FEBio::FEBIO_PARAM_VEC3D : po->AddVecParam(qvariant_to_vec3d(v), szname); break;
		case FEBio::FEBIO_PARAM_MAT3D : po->AddMat3dParam(qvariant_to_mat3d(v), szname); break;
		case FEBio::FEBIO_PARAM_STD_STRING: po->AddStringParam(v.toString().toStdString(), szname); break;
		case FEBio::FEBIO_PARAM_DOUBLE_MAPPED: po->AddDoubleParam(v.toDouble(), szname)->MakeVariable(true); break;
		case FEBio::FEBIO_PARAM_VEC3D_MAPPED : po->AddVecParam(qvariant_to_vec3d(v), szname); break;
		case FEBio::FEBIO_PARAM_MAT3D_MAPPED : po->AddMat3dParam(qvariant_to_mat3d(v), szname); break;
		case FEBio::FEBIO_PARAM_MAT3DS_MAPPED : po->AddMat3dsParam(qvariant_to_mat3ds(v), szname); break;
		default:
			assert(false);
		}
	}
}

void FEBio::CreateStepComponent(int classId, FEStepComponent* po)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return;

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the FEBioClass parameters to the FSObject
	map_parameters(po, feb);

	// don't forget to cleanup
	delete feb;
}

void FEBio::CreateStep(int moduleId, int classId, FEStep* po)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return;

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the FEBioClass parameters to the FSObject
	map_parameters(po, feb);

	// map the properties
	for (int i = 0; i < feb->Properties(); ++i)
	{
		FEBio::FEBioProperty& prop = feb->GetProperty(i);

		FEControlProperty* pc = new FEControlProperty;

		string name = prop.m_name;

		vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllClasses(moduleId, prop.m_superClassId, -1);
		if (fci.size() > 0)
		{
			FEStepComponent* psc = new FEStepComponent;
			CreateStepComponent(fci[0].classId, psc);
			pc->m_prop = psc;

			stringstream ss;
			ss << name << " [" << fci[0].sztype << "]";
			name = ss.str();
		}
		
		pc->SetName(name);
		pc->m_nClassID = prop.m_baseClassId;
		po->AddControlProperty(pc);
	}

	// don't forget to cleanup
	delete feb;
}

void FEBio::CreateMaterial(int classId, FEMaterial* po)
{
	// create the FEBioClass object
	FEBioClass* feb = FEBio::CreateFEBioClass(classId);
	if (feb == nullptr) return;

	// set the type string
	string typeStr = feb->TypeString();
	po->SetTypeString(strdup(typeStr.c_str()));

	// map the parameters
	map_parameters(po, feb);

	// map the properties
	for (int i = 0; i < feb->Properties(); ++i)
	{
		FEBio::FEBioProperty& prop = feb->GetProperty(i);
		po->AddProperty(prop.m_name, prop.m_baseClassId + FE_FEBIO_MATERIAL_CLASS, 1);
	}

	delete feb;
}
