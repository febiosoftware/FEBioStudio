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
FEDiscreteMaterial::FEDiscreteMaterial(int ntype) : FEMaterial(ntype)
{

}

//===================================================================
REGISTER_MATERIAL(FELinearSpringMaterial, MODULE_MECH, FE_DISCRETE_LINEAR_SPRING, FE_MAT_DISCRETE, "linear spring", 0);

FELinearSpringMaterial::FELinearSpringMaterial() : FEDiscreteMaterial(FE_DISCRETE_LINEAR_SPRING)
{
	AddScienceParam(1, UNIT_STIFFNESS, "E", "spring constant");
}

void FELinearSpringMaterial::SetSpringConstant(double E)
{
	SetFloatValue(0, E);
}

//===================================================================

REGISTER_MATERIAL(FENonLinearSpringMaterial, MODULE_MECH, FE_DISCRETE_NONLINEAR_SPRING, FE_MAT_DISCRETE, "nonlinear spring", 0);

FENonLinearSpringMaterial::FENonLinearSpringMaterial() : FEDiscreteMaterial(FE_DISCRETE_NONLINEAR_SPRING)
{
	AddScienceParam(1, UNIT_FORCE, "force", "spring force")->SetLoadCurve();

	// create an initial linear ramp
	LOADPOINT p0(0, 0), p1(1, 1);
	GetParamLC(0)->Clear();
	GetParamLC(0)->Add(p0);
	GetParamLC(0)->Add(p1);
}

//===================================================================

REGISTER_MATERIAL(FEHillContractileMaterial, MODULE_MECH, FE_DISCRETE_HILL, FE_MAT_DISCRETE, "Hill", 0);

FEHillContractileMaterial::FEHillContractileMaterial() : FEDiscreteMaterial(FE_DISCRETE_HILL)
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

	AddProperty(0, new FE1DPointFunction);
	AddProperty(1, new FE1DPointFunction);
	AddProperty(2, new FE1DPointFunction);
}

//===================================================================

REGISTER_MATERIAL(FE1DPointFunction, MODULE_MECH, FE_FNC1D_POINT, FE_MAT_1DFUNC, "point", 0);

FE1DPointFunction::FE1DPointFunction() : FE1DFunction(FE_FNC1D_POINT) 
{
	// dummy parameter so we can use FEMaterial's serialization for the load curve
	AddDoubleParam(0, "points", "points")->SetLoadCurve();

	// constant value
	GetParamLC(0)->Clear();
	GetParamLC(0)->Add(0, 1);
	GetParamLC(0)->Add(1, 1);
}

FELoadCurve* FE1DPointFunction::GetPointCurve()
{
	return GetParamLC(0);
}

void FE1DPointFunction::SetPointCurve(FELoadCurve& lc)
{
	GetParam(0).SetLoadCurve(lc);
}

//===================================================================
FEBioDiscreteMaterial::FEBioDiscreteMaterial() : FEDiscreteMaterial(FE_DISCRETE_FEBIO_MATERIAL)
{

}

FEBioDiscreteMaterial::~FEBioDiscreteMaterial()
{

}

void FEBioDiscreteMaterial::SetTypeString(const char* sz)
{
	m_stype = sz;
}

const char* FEBioDiscreteMaterial::GetTypeString()
{
	return m_stype.c_str();
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
		FEDiscreteMaterial::Save(ar);
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
		case CID_FEBIO_BASE_DATA: FEDiscreteMaterial::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

void FEBioDiscreteMaterial::SetFEBioMaterial(FEBio::FEBioClass* febClass)
{
	m_febClass = febClass;
}

FEBio::FEBioClass* FEBioDiscreteMaterial::GetFEBioMaterial()
{
	return m_febClass;
}

bool FEBioDiscreteMaterial::UpdateData(bool bsave)
{
	if (m_febClass)
	{
		if (bsave) FEBio::UpdateFEBioDiscreteMaterial(this);
	}
	return false;
}
