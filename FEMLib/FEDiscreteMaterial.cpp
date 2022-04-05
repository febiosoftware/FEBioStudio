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
#include "FEDiscreteMaterial.h"
#include <MeshTools/FEProject.h>
#include <FSCore/ParamBlock.h>
#include <FECore/units.h>
#include <FEBioLink/FEBioInterface.h>

//===================================================================
FSDiscreteMaterial::FSDiscreteMaterial(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{

}

//===================================================================
REGISTER_MATERIAL(FSLinearSpringMaterial, MODULE_MECH, FE_DISCRETE_LINEAR_SPRING, FE_MAT_DISCRETE, "linear spring", 0);

FSLinearSpringMaterial::FSLinearSpringMaterial(FSModel* fem) : FSDiscreteMaterial(FE_DISCRETE_LINEAR_SPRING, fem)
{
	AddScienceParam(1, UNIT_STIFFNESS, "E", "spring constant");
}

void FSLinearSpringMaterial::SetSpringConstant(double E)
{
	SetFloatValue(0, E);
}

//===================================================================

REGISTER_MATERIAL(FSNonLinearSpringMaterial, MODULE_MECH, FE_DISCRETE_NONLINEAR_SPRING, FE_MAT_DISCRETE, "nonlinear spring", 0);

FSNonLinearSpringMaterial::FSNonLinearSpringMaterial(FSModel* fem) : FSDiscreteMaterial(FE_DISCRETE_NONLINEAR_SPRING, fem)
{
	AddScienceParam(1, UNIT_FORCE, "force", "spring force");
	AddDoubleParam(1, "scale", "scale");
	AddChoiceParam(0, "measure", "deformation measure")->SetEnumNames("elongation\0strain\0stretch\0");
}

//===================================================================

REGISTER_MATERIAL(FSHillContractileMaterial, MODULE_MECH, FE_DISCRETE_HILL, FE_MAT_DISCRETE, "Hill", 0);

FSHillContractileMaterial::FSHillContractileMaterial(FSModel* fem) : FSDiscreteMaterial(FE_DISCRETE_HILL, fem)
{
	AddScienceParam(0, UNIT_FORCE, "Fmax", "Max force");
	AddScienceParam(1, UNIT_LENGTH, "Lmax", "Max length");
	AddScienceParam(1, UNIT_LENGTH, "L0"  , "Initial length");
	AddDoubleParam(1, "Ksh" , "Shape parameter");
	AddDoubleParam(0, "ac"  , "Activation");
//	AddDoubleParam(1, "Vmax", "Max velocity scale");

	AddProperty("Sv" , FE_MAT_1DFUNC);
	AddProperty("Ftl", FE_MAT_1DFUNC);
	AddProperty("Fvl", FE_MAT_1DFUNC);

	AddProperty(0, new FS1DPointFunction(fem));
	AddProperty(1, new FS1DPointFunction(fem));
	AddProperty(2, new FS1DPointFunction(fem));
}

//===================================================================

REGISTER_MATERIAL(FS1DPointFunction, MODULE_MECH, FE_FNC1D_POINT, FE_MAT_1DFUNC, "point", 0);

FS1DPointFunction::FS1DPointFunction(FSModel* fem) : FS1DFunction(FE_FNC1D_POINT, fem)
{
	// constant value
	m_lc.Clear();
	m_lc.Add(0, 1);
	m_lc.Add(1, 1);
}

LoadCurve* FS1DPointFunction::GetPointCurve()
{
	return &m_lc;
}

void FS1DPointFunction::SetPointCurve(LoadCurve& lc)
{
	m_lc = lc;
}

//===================================================================
FEBioDiscreteMaterial::FEBioDiscreteMaterial(FSModel* fem) : FSDiscreteMaterial(FE_DISCRETE_FEBIO_MATERIAL, fem)
{

}

FEBioDiscreteMaterial::~FEBioDiscreteMaterial()
{

}

void FEBioDiscreteMaterial::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSDiscreteMaterial::Save(ar);
	}
	ar.EndChunk();
}

void FEBioDiscreteMaterial::Load(IArchive& ar)
{
	TRACE("FEBioDiscreteMaterial::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSDiscreteMaterial::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

bool FEBioDiscreteMaterial::UpdateData(bool bsave)
{
//	if (m_febClass)
	{
		if (bsave) FEBio::UpdateFEBioDiscreteMaterial(this);
	}
	return false;
}

// return a string for the material type
const char* FEBioDiscreteMaterial::GetTypeString() const
{
	return FSObject::GetTypeString();
}

void FEBioDiscreteMaterial::SetTypeString(const std::string& s)
{
	FSObject::SetTypeString(s);
}
