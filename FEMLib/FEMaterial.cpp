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

// FSMaterial.cpp: implementation of the FSMaterial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMaterial.h"
#include "FSProject.h"
#include <FECore/units.h>
#include "FEDiscreteMaterial.h"
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioInterface.h>

FSFiberGenerator::FSFiberGenerator(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{
	SetSuperClassID(FEVEC3DVALUATOR_ID);
}

//////////////////////////////////////////////////////////////////////
// FEFiberGeneratorLocal
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFiberGeneratorLocal, MODULE_MECH, FE_FIBER_GENERATOR_LOCAL, FE_MAT_FIBER_GENERATOR, "local", 0);

FSFiberGeneratorLocal::FSFiberGeneratorLocal(FSModel* fem, int n0, int n1) : FSFiberGenerator(FE_FIBER_GENERATOR_LOCAL, fem)
{
	AddVec2iParam(vec2i(n0, n1), "local", "local");
}

vec3d FSFiberGeneratorLocal::GetFiber(FEElementRef& el)
{
	vec2i v = GetVec2iValue(0);

	FSCoreMesh* pm = el.m_pmesh;
	int n[2] = { v.x, v.y };
	if ((n[0] == 0) && (n[1] == 0)) { n[0] = 1; n[1] = 2; }
	vec3d a = pm->Node(el->m_node[n[0] - 1]).r;
	vec3d b = pm->Node(el->m_node[n[1] - 1]).r;

	b -= a;
	b.Normalize();

	return b;
}

//////////////////////////////////////////////////////////////////////
// FSFiberGeneratorVector
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFiberGeneratorVector, MODULE_MECH, FE_FIBER_GENERATOR_VECTOR, FE_MAT_FIBER_GENERATOR, "vector", 0);

FSFiberGeneratorVector::FSFiberGeneratorVector(FSModel* fem, const vec3d& v) : FSFiberGenerator(FE_FIBER_GENERATOR_VECTOR, fem)
{
	AddVecParam(v, "vector", "vector");
}

vec3d FSFiberGeneratorVector::GetFiber(FEElementRef& el)
{
	return GetVecValue(0);
}

//////////////////////////////////////////////////////////////////////
// FSCylindricalVectorGenerator
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCylindricalVectorGenerator, MODULE_MECH, FE_FIBER_GENERATOR_CYLINDRICAL, FE_MAT_FIBER_GENERATOR, "cylindrical", 0);

FSCylindricalVectorGenerator::FSCylindricalVectorGenerator(FSModel* fem) : FSFiberGenerator(FE_FIBER_GENERATOR_CYLINDRICAL, fem)
{
	AddVecParam(vec3d(0, 0, 0), "center", "center");
	AddVecParam(vec3d(0, 0, 1), "axis"  , "axis"  );
	AddVecParam(vec3d(1, 0, 0), "vector", "vector");
}

FSCylindricalVectorGenerator::FSCylindricalVectorGenerator(FSModel* fem, const vec3d& center, const vec3d& axis, const vec3d& vector) : FSFiberGenerator(FE_FIBER_GENERATOR_CYLINDRICAL, fem)
{
	AddVecParam(center, "center", "center");
	AddVecParam(axis  , "axis", "axis");
	AddVecParam(vector, "vector", "vector");
}

vec3d FSCylindricalVectorGenerator::GetFiber(FEElementRef& el)
{
	vec3d r = GetVecValue(0);
	vec3d a = GetVecValue(1);
	vec3d v = GetVecValue(2);

	// we'll use the element center as the reference point
	FSCoreMesh* pm = el.m_pmesh;
	int n = el->Nodes();
	vec3d c(0, 0, 0);
	for (int i = 0; i < n; ++i) c += pm->NodePosition(el->m_node[i]);
	c /= (double)n;

	// find the vector to the axis
	vec3d b = (c - r) - a * (a*(c - r)); b.Normalize();

	// setup the rotation vector
	vec3d x_unit(vec3d(1, 0, 0));
	quatd q(x_unit, b);

	// rotate the reference vector
	v.Normalize();
	q.RotateVector(v);

	return v;
}

//////////////////////////////////////////////////////////////////////
// FSSphericalVectorGenerator
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSSphericalVectorGenerator, MODULE_MECH, FE_FIBER_GENERATOR_SPHERICAL, FE_MAT_FIBER_GENERATOR, "spherical", 0);

FSSphericalVectorGenerator::FSSphericalVectorGenerator(FSModel* fem) : FSFiberGenerator(FE_FIBER_GENERATOR_SPHERICAL, fem)
{
	AddVecParam(vec3d(0, 0, 0), "center", "center");
	AddVecParam(vec3d(1, 0, 0), "vector", "vector");
}

FSSphericalVectorGenerator::FSSphericalVectorGenerator(FSModel* fem, const vec3d& center, const vec3d& vector) : FSFiberGenerator(FE_FIBER_GENERATOR_SPHERICAL, fem)
{
	AddVecParam(center, "center", "center");
	AddVecParam(vector, "vector", "vector");
}

vec3d FSSphericalVectorGenerator::GetFiber(FEElementRef& el)
{
	vec3d o = GetVecValue(0);
	vec3d v = GetVecValue(1);

	FSCoreMesh* pm = el.m_pmesh;
	int n = el->Nodes();
	vec3d c(0, 0, 0);
	for (int i = 0; i < n; ++i) c += pm->NodePosition(el->m_node[i]);
	c /= (double)n;
	c -= o;
	c.Normalize();

	// setup the rotation vector
	vec3d x_unit(vec3d(1, 0, 0));
	quatd q(x_unit, c);

	v.Normalize();
	q.RotateVector(v);

	return v;
}


//////////////////////////////////////////////////////////////////////
// FSSphericalVectorGenerator
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSAnglesVectorGenerator, MODULE_MECH, FE_FIBER_GENERATOR_ANGLES, FE_MAT_FIBER_GENERATOR, "angles", 0);

FSAnglesVectorGenerator::FSAnglesVectorGenerator(FSModel* fem, double theta, double phi) : FSFiberGenerator(FE_FIBER_GENERATOR_ANGLES, fem)
{
	AddScienceParam(theta, UNIT_DEGREE, "theta", "theta")->MakeVariable(true);
	AddScienceParam(phi, UNIT_DEGREE, "phi", "phi")->MakeVariable(true);
}

vec3d FSAnglesVectorGenerator::GetFiber(FEElementRef& el)
{
	double the = GetFloatValue(0) * DEG2RAD;
	double phi = GetFloatValue(1) * DEG2RAD;

	vec3d a;
	a.x = cos(the)*sin(phi);
	a.y = sin(the)*sin(phi);
	a.z = cos(phi);

	return a;
}

void FSAnglesVectorGenerator::GetAngles(double& theta, double& phi)
{
	theta = GetFloatValue(0);
	phi = GetFloatValue(1);
}

void FSAnglesVectorGenerator::SetAngles(double theta, double phi)
{
	SetFloatValue(0, theta);
	SetFloatValue(1, phi);
}

//////////////////////////////////////////////////////////////////////
// FSIsotropicElastic  - isotropic elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSIsotropicElastic, MODULE_MECH, FE_ISOTROPIC_ELASTIC, FE_MAT_ELASTIC, "isotropic elastic", MaterialFlags::TOPLEVEL);

FSIsotropicElastic::FSIsotropicElastic(FSModel* fem) : FSMaterial(FE_ISOTROPIC_ELASTIC, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus")->MakeVariable(true);
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FSOrthoElastic - orthotropic elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOrthoElastic, MODULE_MECH, FE_ORTHO_ELASTIC, FE_MAT_ELASTIC, "orthotropic elastic", MaterialFlags::TOPLEVEL);

FSOrthoElastic::FSOrthoElastic(FSModel* fem) : FSMaterial(FE_ORTHO_ELASTIC, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"    )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E1", "E1 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E2", "E2 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E3", "E3 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G12", "G12 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G23", "G23 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G31", "G31 shear modulus");
	AddScienceParam(0, UNIT_NONE   ,       "v12", "Poisson's ratio v12");
	AddScienceParam(0, UNIT_NONE   ,       "v23", "Poisson's ratio v23");
	AddScienceParam(0, UNIT_NONE   ,       "v31", "Poisson's ratio v31");

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//////////////////////////////////////////////////////////////////////
// FSNeoHookean - neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNeoHookean, MODULE_MECH, FE_NEO_HOOKEAN, FE_MAT_ELASTIC, "neo-Hookean", MaterialFlags::TOPLEVEL);

FSNeoHookean::FSNeoHookean(FSModel* fem) : FSMaterial(FE_NEO_HOOKEAN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus")->MakeVariable(true);
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FENaturalNeoHookean - natural neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNaturalNeoHookean, MODULE_MECH, FE_NATURAL_NEO_HOOKEAN, FE_MAT_ELASTIC, "natural neo-Hookean", MaterialFlags::TOPLEVEL);

FSNaturalNeoHookean::FSNaturalNeoHookean(FSModel* fem) : FSMaterial(FE_NATURAL_NEO_HOOKEAN, fem)
{
    AddScienceParam(1, UNIT_DENSITY , "density", "density"        )->MakeVariable(true)->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE,       "E", "Young's modulus")->MakeVariable(true);
    AddScienceParam(0, UNIT_NONE    ,       "v", "Poisson's ratio")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FETraceFreeNeoHookean - trace-free neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTraceFreeNeoHookean, MODULE_MECH, FE_TRACE_FREE_NEO_HOOKEAN, FE_MAT_ELASTIC, "trace-free neo-Hookean", MaterialFlags::TOPLEVEL);

FSTraceFreeNeoHookean::FSTraceFreeNeoHookean(FSModel* fem) : FSMaterial(FE_TRACE_FREE_NEO_HOOKEAN, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->MakeVariable(true)->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE ,    "mu", "shear modulus")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FEIncompNeoHookean - incompressible neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSIncompNeoHookean, MODULE_MECH, FE_INCOMP_NEO_HOOKEAN, FE_MAT_ELASTIC_UNCOUPLED, "incomp neo-Hookean", MaterialFlags::TOPLEVEL);

FSIncompNeoHookean::FSIncompNeoHookean(FSModel* fem) : FSMaterial(FE_INCOMP_NEO_HOOKEAN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "G", "shear modulus");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSPorousNeoHookean - porous neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPorousNeoHookean, MODULE_MECH, FE_POROUS_NEO_HOOKEAN, FE_MAT_ELASTIC, "porous neo-Hookean", MaterialFlags::TOPLEVEL);

FSPorousNeoHookean::FSPorousNeoHookean(FSModel* fem) : FSMaterial(FE_POROUS_NEO_HOOKEAN, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus");
    AddScienceParam(1, UNIT_NONE   ,    "phi0", "solid volume fraction");
}

//////////////////////////////////////////////////////////////////////
// FSMooneyRivlin - Mooney-Rivlin rubber
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSMooneyRivlin, MODULE_MECH, FE_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSMooneyRivlin::FSMooneyRivlin(FSModel* fem) : FSMaterial(FE_MOONEY_RIVLIN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_PRESSURE , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSVerondaWestmann - Veronda-Westmann elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSVerondaWestmann, MODULE_MECH, FE_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSVerondaWestmann::FSVerondaWestmann(FSModel* fem) : FSMaterial(FE_VERONDA_WESTMANN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_NONE   , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FEPolynomialHyperelastic
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPolynomialHyperelastic, MODULE_MECH, FE_POLYNOMIAL_HYPERELASTIC, FE_MAT_ELASTIC_UNCOUPLED, "polynomial", MaterialFlags::TOPLEVEL);

FSPolynomialHyperelastic::FSPolynomialHyperelastic(FSModel* fem) : FSMaterial(FE_POLYNOMIAL_HYPERELASTIC, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c01", "c01");
	AddScienceParam(0, UNIT_PRESSURE, "c02", "c02");
	AddScienceParam(0, UNIT_PRESSURE, "c10", "c10");
	AddScienceParam(0, UNIT_PRESSURE, "c11", "c11");
	AddScienceParam(0, UNIT_PRESSURE, "c12", "c12");
	AddScienceParam(0, UNIT_PRESSURE, "c20", "c20");
	AddScienceParam(0, UNIT_PRESSURE, "c21", "c21");
	AddScienceParam(0, UNIT_PRESSURE, "c22", "c22");
	AddScienceParam(0, UNIT_PRESSURE, "D1", "D1")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "D2", "D2")->SetPersistent(false);
}


//////////////////////////////////////////////////////////////////////
// FSCoupledMooneyRivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCoupledMooneyRivlin, MODULE_MECH, FE_COUPLED_MOONEY_RIVLIN, FE_MAT_ELASTIC, "coupled Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSCoupledMooneyRivlin::FSCoupledMooneyRivlin(FSModel* fem) : FSMaterial(FE_COUPLED_MOONEY_RIVLIN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FSCoupledVerondaWestmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCoupledVerondaWestmann, MODULE_MECH, FE_COUPLED_VERONDA_WESTMANN, FE_MAT_ELASTIC, "coupled Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSCoupledVerondaWestmann::FSCoupledVerondaWestmann(FSModel* fem) : FSMaterial(FE_COUPLED_VERONDA_WESTMANN, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_NONE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FSHolmesMow -Holmes-Mow elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolmesMow, MODULE_MECH, FE_HOLMES_MOW, FE_MAT_ELASTIC, "Holmes-Mow", MaterialFlags::TOPLEVEL);

FSHolmesMow::FSHolmesMow(FSModel* fem) : FSMaterial(FE_HOLMES_MOW, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddScienceParam(0, UNIT_NONE   , "beta", "power exponent");
}

//////////////////////////////////////////////////////////////////////
// FEHolmesMowUC -uncoupled Holmes-Mow elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolmesMowUC, MODULE_MECH, FE_HOLMES_MOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled Holmes-Mow", MaterialFlags::TOPLEVEL);

FSHolmesMowUC::FSHolmesMowUC(FSModel* fem) : FSMaterial(FE_HOLMES_MOW_UNCOUPLED, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "mu", "shear modulus");
    AddScienceParam(0, UNIT_NONE   , "beta", "power exponent");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FEArrudaBoyce - Arruda-Boyce elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSArrudaBoyce, MODULE_MECH, FE_ARRUDA_BOYCE, FE_MAT_ELASTIC_UNCOUPLED, "Arruda-Boyce", MaterialFlags::TOPLEVEL);

FSArrudaBoyce::FSArrudaBoyce(FSModel* fem) : FSMaterial(FE_ARRUDA_BOYCE, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "mu", "Initial modulus");
	AddScienceParam(0, UNIT_NONE   , "N", "links");
	AddScienceParam(0, UNIT_PRESSURE , "k", "Bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FEArrudaBoyceUC - Arruda-Boyce unconstrained
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSArrudaBoyceUC, MODULE_MECH, FE_ARRUDA_BOYCE_UC, FE_MAT_ELASTIC_UNCOUPLED, "Arruda-Boyce unconstrained", MaterialFlags::TOPLEVEL);

FSArrudaBoyceUC::FSArrudaBoyceUC(FSModel* fem) : FSMaterial(FE_ARRUDA_BOYCE_UC, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
	AddDoubleParam(0.00001, "ksi");
	AddDoubleParam(100.0, "N");
	AddIntParam(30, "n_term");
	AddDoubleParam(0, "kappa");
}

//////////////////////////////////////////////////////////////////////
// FSCarterHayes - Carter-Hayes elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCarterHayes, MODULE_MECH, FE_CARTER_HAYES, FE_MAT_ELASTIC, "Carter-Hayes", MaterialFlags::TOPLEVEL);

FSCarterHayes::FSCarterHayes(FSModel* fem) : FSMaterial(FE_CARTER_HAYES, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "true density");
	AddScienceParam(0, UNIT_PRESSURE , "E0", "E0");
	AddScienceParam(1, UNIT_DENSITY, "rho0", "rho0");
	AddScienceParam(0, UNIT_NONE   , "gamma", "gamma");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddIntParam    (-1, "sbm", "sbm");
}

//////////////////////////////////////////////////////////////////////
// FSNewtonianViscousSolid - Newtonian viscous solid
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNewtonianViscousSolid, MODULE_MECH, FE_NEWTONIAN_VISCOUS_SOLID, FE_MAT_ELASTIC, "Newtonian viscous solid",0);

FSNewtonianViscousSolid::FSNewtonianViscousSolid(FSModel* fem) : FSMaterial(FE_NEWTONIAN_VISCOUS_SOLID, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "true density");
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity");
    AddScienceParam(0, UNIT_VISCOSITY, "kappa", "bulk viscosity");
}

//////////////////////////////////////////////////////////////////////
// FSPRLig - Poission-Ratio Ligament
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPRLig, MODULE_MECH, FE_PRLIG, FE_MAT_ELASTIC, "PRLig", MaterialFlags::TOPLEVEL);

FSPRLig::FSPRLig(FSModel* fem) : FSMaterial(FE_PRLIG, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_NONE   , "c1"     , "c1");
	AddScienceParam(1, UNIT_NONE   , "c2"     , "c2");
	AddScienceParam(0, UNIT_NONE   , "v0"     , "v0");
	AddScienceParam(0, UNIT_NONE   , "m"      , "m" );
	AddScienceParam(0, UNIT_NONE   , "mu"     , "mu");
	AddScienceParam(0, UNIT_NONE   , "k"      , "k")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSOldFiberMaterial - material for fibers
//////////////////////////////////////////////////////////////////////

FSOldFiberMaterial::FSOldFiberMaterial(FSModel* fem) : FSMaterial(0, fem)
{
	AddIntParam(0, "fiber", "fiber")->SetEnumNames("local\0cylindrical\0spherical\0vector\0user\0angles\0polar\0\0");
	AddIntParam(0, "n0", "n0");
	AddIntParam(0, "n1", "n1");
	AddVecParam(vec3d(0, 0, 0), "r");
	AddVecParam(vec3d(0, 0, 1), "a");
	AddVecParam(vec3d(1, 0, 0), "d");
	AddDoubleParam(0, "theta");
	AddDoubleParam(0, "phi");

	m_naopt = FE_FIBER_VECTOR;
	m_nuser = 0;
	m_n[0] = m_n[1] = 0;
	m_r = vec3d(0,0,0);
	m_a = vec3d(1,0,0);
	m_d = vec3d(0,1,0);
	m_theta = 0.0;
	m_phi = 90.0;
	m_d0 = m_d1 = vec3d(0,0,1);
	m_R0 = 0; m_R1 = 1;

	UpdateData(false);
	UpdateData(true);
}

bool FSOldFiberMaterial::UpdateData(bool bsave)
{
	if (bsave)
	{
		int oldopt = m_naopt;
		m_naopt = GetIntValue(0);

		for (int i = 1; i < Parameters(); ++i) GetParam(i).SetState(0);

		switch (m_naopt)
		{
		case FE_FIBER_LOCAL:
			GetParam(1).SetState(Param_ALLFLAGS);
			GetParam(2).SetState(Param_ALLFLAGS);
			m_n[0] = GetIntValue(1);
			m_n[1] = GetIntValue(2);
			break;
		case FE_FIBER_CYLINDRICAL:
			GetParam(3).SetState(Param_ALLFLAGS);
			GetParam(4).SetState(Param_ALLFLAGS);
			GetParam(5).SetState(Param_ALLFLAGS);
			m_r = GetVecValue(3);
			m_a = GetVecValue(4);
			m_d = GetVecValue(5);
			break;
		case FE_FIBER_SPHERICAL:
			GetParam(3).SetState(Param_ALLFLAGS);
			m_r = GetVecValue(3);
			break;
		case FE_FIBER_VECTOR:
			GetParam(4).SetState(Param_ALLFLAGS);
			m_a = GetVecValue(4);
			break;
		case FE_FIBER_USER:
			break;
		case FE_FIBER_ANGLES:
			GetParam(6).SetState(Param_ALLFLAGS);
			GetParam(7).SetState(Param_ALLFLAGS);
			m_theta = GetFloatValue(6);
			m_phi = GetFloatValue(7);
			break;
		case FE_FIBER_POLAR:
			GetParam(3).SetState(Param_ALLFLAGS);
			GetParam(4).SetState(Param_ALLFLAGS);
			m_r = GetVecValue(3);
			m_a = GetVecValue(4);
			break;
		}

		return (m_naopt != oldopt);
	}
	else
	{
		SetIntValue(0, m_naopt);
		SetIntValue(1, m_n[0]);
		SetIntValue(2, m_n[1]);
		SetVecValue(3, m_r);
		SetVecValue(4, m_a);
		SetVecValue(5, m_d);
		SetFloatValue(6, m_theta);
		SetFloatValue(7, m_phi);
	}

	return false;
}

vec3d FSOldFiberMaterial::GetFiberVector(FEElementRef& el)
{
	vec3d v = GetLocalFiberVector(el);
	const FSMaterial* parentMat = GetParentMaterial();
	if (parentMat)
	{
		mat3d Q = parentMat->GetMatAxes(el);
		v = Q * v;
	}
	return v;
}

vec3d FSOldFiberMaterial::GetLocalFiberVector(FEElementRef& el)
{
	switch (m_naopt)
	{
	case FE_FIBER_LOCAL:
	{
		FSCoreMesh* pm = el.m_pmesh;
		int n[2] = { m_n[0], m_n[1] };
		if ((n[0] == 0) && (n[1] == 0)) { n[0] = 1; n[1] = 2; }
		vec3d a = pm->Node(el->m_node[n[0] - 1]).r;
		vec3d b = pm->Node(el->m_node[n[1] - 1]).r;

		b -= a;
		b.Normalize();

		return b;
	}
	break;
	case FE_FIBER_CYLINDRICAL:
	{
		// we'll use the element center as the reference point
		FSCoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i < n; ++i) c += pm->NodePosition(el->m_node[i]);
		c /= (double)n;

		// find the vector to the axis
		vec3d r = m_r;
		vec3d a = m_a;
		vec3d v = m_d;
		vec3d b = (c - r) - a * (a*(c - r)); b.Normalize();

		// setup the rotation vector
		vec3d x_unit(vec3d(1, 0, 0));
		quatd q(x_unit, b);

		// rotate the reference vector
		v.Normalize();
		q.RotateVector(v);

		return v;
	}
	break;
	case FE_FIBER_SPHERICAL:
	{
		FSCoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i < n; ++i) c += pm->NodePosition(el->m_node[i]);
		c /= (double)n;
		c -= m_r;
		c.Normalize();

		// setup the rotation vector
		vec3d x_unit(vec3d(1, 0, 0));
		quatd q(x_unit, c);

		vec3d v = m_d;
		v.Normalize();
		q.RotateVector(v);

		return v;
	}
	break;
	case FE_FIBER_VECTOR:
	{
		return m_a;
	}
	break;
	case FE_FIBER_USER:
	{
		return el->m_fiber;
	}
	break;
	case FE_FIBER_ANGLES:
	{
		// convert from degress to radians
		const double pi = 4 * atan(1.0);
		const double the = m_theta * pi / 180.;
		const double phi = m_phi * pi / 180.;

		// define the first axis (i.e. the fiber vector)
		vec3d a;
		a.x = cos(the)*sin(phi);
		a.y = sin(the)*sin(phi);
		a.z = cos(phi);
		return a;
	}
	break;
	case FE_FIBER_POLAR:
	{
		// we'll use the element center as the reference point
		FSCoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i < n; ++i) c += pm->NodePosition(el->m_node[i]);
		c /= (double)n;

		// find the vector to the axis
		vec3d r = m_r;
		vec3d a = m_a;

		vec3d b = (c - r) - a * (a*(c - r));
		double R = b.Length(); b.Normalize();

		double R0 = m_R0;
		double R1 = m_R1;
		if (R1 == R0) R1 += 1;
		double w = (R - R0) / (R1 - R0);

		vec3d v0 = m_d0; v0.Normalize();
		vec3d v1 = m_d1; v1.Normalize();

		quatd Q0(0, vec3d(0, 0, 1)), Q1(v0, v1);
		quatd Q = quatd::slerp(Q0, Q1, w);
		vec3d v = v0; Q.RotateVector(v);

		// setup the rotation vector
		vec3d x_unit(vec3d(1, 0, 0));
		quatd q(x_unit, b);

		// rotate the reference vector
		v.Normalize();
		q.RotateVector(v);

		return v;
	}
	break;
	}

	assert(false);
	return vec3d(0, 0, 0);
}

FSOldFiberMaterial::FSOldFiberMaterial(const FSOldFiberMaterial& m) : FSMaterial(0, nullptr) {}
FSOldFiberMaterial& FSOldFiberMaterial::operator = (const FSOldFiberMaterial& m) { return (*this); }

void FSOldFiberMaterial::copy(FSOldFiberMaterial* pm)
{
	m_naopt = pm->m_naopt;
	m_nuser = pm->m_nuser;
	m_n[0] = pm->m_n[0];
	m_n[1] = pm->m_n[1];
	m_r = pm->m_r;
	m_a = pm->m_a;
	m_d = pm->m_d;
	m_theta = pm->m_theta;
	m_phi = pm->m_phi;
	m_d0 = pm->m_d0;
	m_d1 = pm->m_d1;
	m_R0 = pm->m_R0;
	m_R1 = pm->m_R1;

//	GetParamBlock() = pm->GetParamBlock();
}

void FSOldFiberMaterial::Save(OArchive &ar)
{
	ar.WriteChunk(MP_AOPT, m_naopt);
	ar.WriteChunk(MP_N, m_n, 2);
	ar.WriteChunk(MP_R, m_r);
	ar.WriteChunk(MP_A, m_a);
	ar.WriteChunk(MP_D, m_d);
	ar.WriteChunk(MP_NUSER, m_nuser);
	ar.WriteChunk(MP_THETA, m_theta);
	ar.WriteChunk(MP_PHI, m_phi);
	ar.WriteChunk(MP_D0, m_d0);
	ar.WriteChunk(MP_D1, m_d1);
	ar.WriteChunk(MP_R0, m_R0);
	ar.WriteChunk(MP_R1, m_R1);
	ar.BeginChunk(MP_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FSOldFiberMaterial::Load(IArchive& ar)
{
	TRACE("FSOldFiberMaterial::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case MP_AOPT: ar.read(m_naopt); break;
		case MP_N: ar.read(m_n, 2); break;
		case MP_R: ar.read(m_r); break;
		case MP_A: ar.read(m_a); break;
		case MP_D: ar.read(m_d); break;
		case MP_NUSER: ar.read(m_nuser); break;
		case MP_THETA: ar.read(m_theta); break;
		case MP_PHI: ar.read(m_phi); break;
		case MP_D0: ar.read(m_d0); break;
		case MP_D1: ar.read(m_d1); break;
		case MP_R0: ar.read(m_R0); break;
		case MP_R1: ar.read(m_R1); break;
		case MP_PARAMS: 
			ParamContainer::Load(ar);
			break;
		}
		ar.CloseChunk();
	}
	UpdateData(false);
	UpdateData(true);
}

//////////////////////////////////////////////////////////////////////
// FSTransverselyIsotropic - base class for transversely isotropic
//////////////////////////////////////////////////////////////////////

FSTransverselyIsotropic::FSTransverselyIsotropic(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{
	m_pfiber = 0;
}

FSOldFiberMaterial* FSTransverselyIsotropic::GetFiberMaterial()
{
	return m_pfiber;
}

const FSOldFiberMaterial* FSTransverselyIsotropic::GetFiberMaterial() const
{
	return m_pfiber;
}

void FSTransverselyIsotropic::SetFiberMaterial(FSOldFiberMaterial* fiber)
{
	fiber->SetParentMaterial(this);
	m_pfiber = fiber;
}

vec3d FSTransverselyIsotropic::GetFiber(FEElementRef& el)
{
	FSOldFiberMaterial& fiber = *m_pfiber;
	return fiber.GetFiberVector(el);
}

void FSTransverselyIsotropic::copy(FSMaterial *pmat)
{
	FSTransverselyIsotropic& m = dynamic_cast<FSTransverselyIsotropic&>(*pmat);
	assert(m.Type() == m_ntype);

	FSMaterial::copy(pmat);
	m_pfiber->copy(m.m_pfiber);
}

void FSTransverselyIsotropic::Save(OArchive &ar)
{
	ar.BeginChunk(MP_MAT);
	{
		FSMaterial::Save(ar);
	}
	ar.EndChunk();

	ar.BeginChunk(MP_FIBERS);
	{
		m_pfiber->Save(ar);
	}
	ar.EndChunk();
}

void FSTransverselyIsotropic::Load(IArchive &ar)
{
	TRACE("FSTransverselyIsotropic::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case MP_MAT: FSMaterial::Load(ar); break;
		case MP_FIBERS: m_pfiber->Load(ar); break;
		default:
			throw ReadError("unknown CID in FSTransverselyIsotropic::Load");
		}

		ar.CloseChunk();
	}
}

//////////////////////////////////////////////////////////////////////
// FSTransMooneyRivlinOld - transversely isotropic mooney-rivlin (obsolete implementation)
//////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSTransMooneyRivlinOld, MODULE_MECH, FE_TRANS_MOONEY_RIVLIN_OLD, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSTransMooneyRivlinOld::Fiber::Fiber(FSModel* fem) : FSOldFiberMaterial(fem)
{
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE  , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE  , "lam_max", "lam_max");
	AddDoubleParam(0, "ca0" , "ca0" )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "beta", "beta")->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "L0"  , "L0"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "Lr"  , "Lr"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "active_contraction", "active_contraction")->SetState(Param_State::Param_READWRITE);
}

FSTransMooneyRivlinOld::FSTransMooneyRivlinOld(FSModel* fem) : FSTransverselyIsotropic(FE_TRANS_MOONEY_RIVLIN_OLD, fem)
{
	// define the fiber class
	SetFiberMaterial(new Fiber(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSTransVerondaWestmann - transversely isotropic veronda-westmann (obsolete)
//////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSTransVerondaWestmannOld, MODULE_MECH, FE_TRANS_VERONDA_WESTMANN_OLD, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSTransVerondaWestmannOld::Fiber::Fiber(FSModel* fem) : FSOldFiberMaterial(fem)
{
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE  , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE  , "lam_max", "lam_max");
	AddDoubleParam(0, "ca0" , "ca0" )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "beta", "beta")->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "L0"  , "L0"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "Lr"  , "Lr"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "active_contraction", "active_contraction")->SetState(Param_State::Param_READWRITE);
}

FSTransVerondaWestmannOld::FSTransVerondaWestmannOld(FSModel* fem) : FSTransverselyIsotropic(FE_TRANS_VERONDA_WESTMANN_OLD, fem)
{
	// define the fiber class
	SetFiberMaterial(new Fiber(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_NONE   , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus")->SetPersistent(false);
}


//////////////////////////////////////////////////////////////////////
// FSActiveContraction - Active contraction material
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSActiveContraction, MODULE_MECH, FE_MAT_ACTIVE_CONTRACTION, FE_MAT_ACTIVE_CONTRACTION_CLASS, "active_contraction", 0);

FSActiveContraction::FSActiveContraction(FSModel* fem) : FSMaterialProp(FE_MAT_ACTIVE_CONTRACTION, fem)
{
	AddDoubleParam(0, "ascl", "scale");
	AddDoubleParam(0, "ca0");
	AddDoubleParam(0, "beta");
	AddDoubleParam(0, "l0");
	AddDoubleParam(0, "refl");
}

//////////////////////////////////////////////////////////////////////
// FEForceVelocityEstrada - Active contraction material
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSForceVelocityEstrada, MODULE_MECH, FE_FORCE_VELOCITY_ESTRADA, FE_MAT_ACTIVE_CONTRACTION_CLASS, "force-velocity-Estrada", 0);

FSForceVelocityEstrada::FSForceVelocityEstrada(FSModel* fem) : FSMaterialProp(FE_FORCE_VELOCITY_ESTRADA, fem)
{
    AddDoubleParam(0, "ascl", "scale");
    AddDoubleParam(0, "ca0");
    AddDoubleParam(0, "camax");
    AddDoubleParam(0, "beta");
    AddDoubleParam(0, "l0");
    AddDoubleParam(0, "refl");
    AddDoubleParam(0, "Tmax");
    AddDoubleParam(0, "alpha1");
    AddDoubleParam(0, "alpha2");
    AddDoubleParam(0, "alpha3");
    AddDoubleParam(0, "A1");
    AddDoubleParam(0, "A2");
    AddDoubleParam(0, "A3");
    AddDoubleParam(0, "a_t");
    AddBoolParam(true, "force_velocity");
}

//////////////////////////////////////////////////////////////////////
// FETransMooneyRivlin - transversely isotropic mooney-rivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTransMooneyRivlin, MODULE_MECH, FE_TRANS_ISO_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSTransMooneyRivlin::FSTransMooneyRivlin(FSModel* fem) : FSTransverselyIsotropic(FE_TRANS_ISO_MOONEY_RIVLIN, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lam_max", "lam_max");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);

	AddProperty("active_contraction", FE_MAT_ACTIVE_CONTRACTION_CLASS);
}

void FSTransMooneyRivlin::Convert(FSTransMooneyRivlinOld* pold)
{
	if (pold == 0) return;

	FSTransMooneyRivlinOld::Fiber* oldFiber = dynamic_cast<FSTransMooneyRivlinOld::Fiber*>(pold->GetFiberMaterial());

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FSTransMooneyRivlinOld::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FSTransMooneyRivlinOld::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FSTransMooneyRivlinOld::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FSTransMooneyRivlinOld::MP_K));
	SetFloatValue(MP_C3     , oldFiber->GetFloatValue(FSTransMooneyRivlinOld::Fiber::MP_C3));
	SetFloatValue(MP_C4     , oldFiber->GetFloatValue(FSTransMooneyRivlinOld::Fiber::MP_C4));
	SetFloatValue(MP_C5     , oldFiber->GetFloatValue(FSTransMooneyRivlinOld::Fiber::MP_C5));
	SetFloatValue(MP_LAM    , oldFiber->GetFloatValue(FSTransMooneyRivlinOld::Fiber::MP_LAM));

	GetFiberMaterial()->copy(oldFiber);
}

//////////////////////////////////////////////////////////////////////
// FSTransVerondaWestmann - transversely isotropic Veronda-Westmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTransVerondaWestmann, MODULE_MECH, FE_TRANS_ISO_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSTransVerondaWestmann::FSTransVerondaWestmann(FSModel* fem) : FSTransverselyIsotropic(FE_TRANS_ISO_VERONDA_WESTMANN, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lam_max", "lam_max");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);

	AddProperty("active_contraction", FE_MAT_ACTIVE_CONTRACTION_CLASS);
}

void FSTransVerondaWestmann::Convert(FSTransVerondaWestmannOld* pold)
{
	if (pold == 0) return;

	FSTransVerondaWestmannOld::Fiber* oldFiber = dynamic_cast<FSTransVerondaWestmannOld::Fiber*>(pold->GetFiberMaterial());

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FSTransVerondaWestmannOld::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FSTransVerondaWestmannOld::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FSTransVerondaWestmannOld::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FSTransVerondaWestmannOld::MP_K));
	SetFloatValue(MP_C3     , oldFiber->GetFloatValue(FSTransVerondaWestmannOld::Fiber::MP_C3));
	SetFloatValue(MP_C4     , oldFiber->GetFloatValue(FSTransVerondaWestmannOld::Fiber::MP_C4));
	SetFloatValue(MP_C5     , oldFiber->GetFloatValue(FSTransVerondaWestmannOld::Fiber::MP_C5));
	SetFloatValue(MP_LAM    , oldFiber->GetFloatValue(FSTransVerondaWestmannOld::Fiber::MP_LAM));

	GetFiberMaterial()->copy(oldFiber);
}

//////////////////////////////////////////////////////////////////////
// FSCoupledTransIsoVerondaWestmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCoupledTransIsoVerondaWestmann, MODULE_MECH, FE_COUPLED_TRANS_ISO_VW, FE_MAT_ELASTIC, "coupled trans-iso Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSCoupledTransIsoVerondaWestmann::FSCoupledTransIsoVerondaWestmann(FSModel* fem) : FSTransverselyIsotropic(FE_COUPLED_TRANS_ISO_VW, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lambda", "lambda");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// Coupled Trans-iso Mooney-Rivlin (Obsolete implementation)
//=============================================================================

//REGISTER_MATERIAL(FSCoupledTransIsoMooneyRivlinOld, MODULE_MECH, FE_COUPLED_TRANS_ISO_MR, FE_MAT_ELASTIC, "coupled trans-iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSCoupledTransIsoMooneyRivlinOld::FSCoupledTransIsoMooneyRivlinOld(FSModel* fem) : FSMaterial(FE_COUPLED_TRANS_ISO_MR, fem)
{
	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "c3", "c3");
	AddScienceParam(0, UNIT_NONE   , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE , "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_NONE   , "lambda", "lambda");
}

//=============================================================================
// Coupled Trans-iso Mooney-Rivlin
//=============================================================================

REGISTER_MATERIAL(FSCoupledTransIsoMooneyRivlin, MODULE_MECH, FE_COUPLED_TRANS_ISO_MR, FE_MAT_ELASTIC, "coupled trans-iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSCoupledTransIsoMooneyRivlin::FSCoupledTransIsoMooneyRivlin(FSModel* fem) : FSTransverselyIsotropic(FE_COUPLED_TRANS_ISO_MR, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddScienceParam(0, UNIT_NONE, "lam_max", "lambda max");
}

void FSCoupledTransIsoMooneyRivlin::Convert(FSCoupledTransIsoMooneyRivlinOld* pold)
{
	if (pold == 0) return;

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_K));
	SetFloatValue(MP_C3     , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_C3));
	SetFloatValue(MP_C4     , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_C4));
	SetFloatValue(MP_C5     , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_C5));
	SetFloatValue(MP_LAMBDA , pold->GetFloatValue(FSCoupledTransIsoMooneyRivlin::MP_LAMBDA));
}

//=============================================================================
// FSMooneyRivlinVonMisesFibers
//=============================================================================

REGISTER_MATERIAL(FSMooneyRivlinVonMisesFibers, MODULE_MECH, FE_MAT_MR_VON_MISES_FIBERS, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin von Mises Fibers", MaterialFlags::TOPLEVEL);

FSMooneyRivlinVonMisesFibers::FSMooneyRivlinVonMisesFibers(FSModel* fem) : FSMaterial(FE_MAT_MR_VON_MISES_FIBERS, fem)
{
	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3");
	AddScienceParam(0, UNIT_NONE, "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_NONE, "lam_max");
	AddDoubleParam(0, "kf");
	AddDoubleParam(0, "vmc");
	AddDoubleParam(0, "var_n");
	AddDoubleParam(0, "tp");
	AddIntParam(0, "gipt");

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// FS2DTransIsoMooneyRivlin
//=============================================================================

REGISTER_MATERIAL(FS2DTransIsoMooneyRivlin, MODULE_MECH, FE_MAT_2D_TRANS_ISO_MR, FE_MAT_ELASTIC_UNCOUPLED, "2D trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FS2DTransIsoMooneyRivlin::FS2DTransIsoMooneyRivlin(FSModel* fem) : FSTransverselyIsotropic(FE_MAT_2D_TRANS_ISO_MR, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3");
	AddScienceParam(0, UNIT_NONE, "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_NONE, "lam_max");
}

//////////////////////////////////////////////////////////////////////
// FSRigidMaterial - rigid body material
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSRigidMaterial, MODULE_MECH, FE_RIGID_MATERIAL, FE_MAT_RIGID, "rigid body", MaterialFlags::TOPLEVEL);

FSRigidMaterial::FSRigidMaterial(FSModel* fem) : FSMaterial(FE_RIGID_MATERIAL, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(1, UNIT_PRESSURE , "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddBoolParam  (false, "auto_com", "Auto-COM");
	AddVecParam   (vec3d(0,0,0), "center_of_mass", "Center of mass");

	m_pid = -1;
}

void FSRigidMaterial::Save(OArchive& ar)
{
	ar.WriteChunk(CID_MAT_RIGID_PID, m_pid);
	ar.BeginChunk(CID_MAT_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FSRigidMaterial::Load(IArchive &ar)
{
	TRACE("FSRigidMaterial::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MAT_RIGID_PID: ar.read(m_pid); break;
		case CID_MAT_PARAMS: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

void FSRigidMaterial::SetAutoCOM(bool b)
{
	SetBoolValue(MP_COM, b);
}

void FSRigidMaterial::SetCenterOfMass(const vec3d& r)
{
	SetVecValue(MP_RC, r);
}

vec3d FSRigidMaterial::GetCenterOfMass() const
{
	return GetVecValue(MP_RC);
}

void FSRigidMaterial::copy(FSMaterial* pmat)
{
	FSMaterial::copy(pmat);
	m_pid = (dynamic_cast<FSRigidMaterial*>(pmat))->m_pid;
}

bool FSRigidMaterial::IsRigid()
{
	return true;
}

////////////////////////////////////////////////////////////////////////
// FSTCNonlinearOrthotropic - Tension-Compression Nonlinear Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTCNonlinearOrthotropic, MODULE_MECH, FE_TCNL_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "TC nonlinear orthotropic", MaterialFlags::TOPLEVEL);

FSTCNonlinearOrthotropic::FSTCNonlinearOrthotropic(FSModel* fem) : FSMaterial(FE_TCNL_ORTHO, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFungOrthotropic, MODULE_MECH, FE_FUNG_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "Fung orthotropic", MaterialFlags::TOPLEVEL);

FSFungOrthotropic::FSFungOrthotropic(FSModel* fem) : FSMaterial(FE_FUNG_ORTHO, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "v12");
	AddScienceParam(0, UNIT_NONE   , "v23", "v23");
	AddScienceParam(0, UNIT_NONE   , "v31", "v31");
	AddScienceParam(0, UNIT_PRESSURE , "c", "c");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFungOrthoCompressible, MODULE_MECH, FE_FUNG_ORTHO_COUPLED, FE_MAT_ELASTIC, "Fung-ortho-compressible", MaterialFlags::TOPLEVEL);

FSFungOrthoCompressible::FSFungOrthoCompressible(FSModel* fem) : FSMaterial(FE_FUNG_ORTHO_COUPLED, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "v12");
	AddScienceParam(0, UNIT_NONE   , "v23", "v23");
	AddScienceParam(0, UNIT_NONE   , "v31", "v31");
	AddScienceParam(0, UNIT_PRESSURE , "c", "c");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSHolzapfelGasserOgden - HGO MODEL
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolzapfelGasserOgden, MODULE_MECH, FE_HOLZAPFEL_GASSER_OGDEN, FE_MAT_ELASTIC_UNCOUPLED, "Holzapfel-Gasser-Ogden", MaterialFlags::TOPLEVEL);

FSHolzapfelGasserOgden::FSHolzapfelGasserOgden(FSModel* fem) : FSMaterial(FE_HOLZAPFEL_GASSER_OGDEN, fem)
{
    AddScienceParam(1, UNIT_DENSITY  , "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "c", "c");
    AddScienceParam(0, UNIT_PRESSURE , "k1", "k1");
    AddScienceParam(0, UNIT_NONE     , "k2", "k2");
    AddScienceParam(0, UNIT_NONE     , "kappa", "kappa");
    AddScienceParam(0, UNIT_DEGREE   , "gamma", "gamma");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);
    
    SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSHolzapfelUnconstrained - HGO MODEL
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolzapfelUnconstrained, MODULE_MECH, FE_HOLZAPFEL_UNCONSTRAINED, FE_MAT_ELASTIC, "HGO unconstrained", MaterialFlags::TOPLEVEL);

FSHolzapfelUnconstrained::FSHolzapfelUnconstrained(FSModel* fem) : FSMaterial(FE_HOLZAPFEL_UNCONSTRAINED, fem)
{
    AddScienceParam(1, UNIT_DENSITY  , "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "c", "c");
    AddScienceParam(0, UNIT_PRESSURE , "k1", "k1");
    AddScienceParam(0, UNIT_NONE     , "k2", "k2");
    AddScienceParam(0, UNIT_NONE     , "kappa", "kappa");
    AddScienceParam(0, UNIT_DEGREE   , "gamma", "gamma");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
    
    SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSLinearOrthotropic - Linear Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSLinearOrthotropic, MODULE_MECH, FE_LINEAR_ORTHO, FE_MAT_ELASTIC, "orthotropic elastic", MaterialFlags::TOPLEVEL);

FSLinearOrthotropic::FSLinearOrthotropic(FSModel* fem) : FSMaterial(FE_LINEAR_ORTHO, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "v12");
	AddScienceParam(0, UNIT_NONE   , "v23", "v23");
	AddScienceParam(0, UNIT_NONE   , "v31", "v31");

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSMuscleMaterial - Silvia Blemker's muscle material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSMuscleMaterial, MODULE_MECH, FE_MUSCLE_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "muscle material", MaterialFlags::TOPLEVEL);

FSMuscleMaterial::FSMuscleMaterial(FSModel* fem) : FSTransverselyIsotropic(FE_MUSCLE_MATERIAL, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "g1");
	AddScienceParam(0, UNIT_PRESSURE , "g2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);
	AddDoubleParam(0, "p1");
	AddDoubleParam(0, "p2");
	AddDoubleParam(0, "Lofl");
	AddDoubleParam(0, "lam_max");
	AddDoubleParam(0, "smax");
	AddDoubleParam(0, "activation");
}

////////////////////////////////////////////////////////////////////////
// FSTendonMaterial - Silvia Blemker's tendon material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTendonMaterial, MODULE_MECH, FE_TENDON_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "tendon material", MaterialFlags::TOPLEVEL);

FSTendonMaterial::FSTendonMaterial(FSModel* fem) : FSTransverselyIsotropic(FE_TENDON_MATERIAL, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "g1");
	AddScienceParam(0, UNIT_PRESSURE , "g2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);
	AddDoubleParam(0, "l1");
	AddDoubleParam(0, "l2");
	AddDoubleParam(0, "lam_max");
}

////////////////////////////////////////////////////////////////////////
// Ogden material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOgdenMaterial, MODULE_MECH, FE_OGDEN_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "Ogden", MaterialFlags::TOPLEVEL);

FSOgdenMaterial::FSOgdenMaterial(FSModel* fem) : FSMaterial(FE_OGDEN_MATERIAL, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE , "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE , "c6", "c6");
	AddScienceParam(1, UNIT_NONE   , "m1", "m1");
	AddScienceParam(1, UNIT_NONE   , "m2", "m2");
	AddScienceParam(1, UNIT_NONE   , "m3", "m3");
	AddScienceParam(1, UNIT_NONE   , "m4", "m4");
	AddScienceParam(1, UNIT_NONE   , "m5", "m5");
	AddScienceParam(1, UNIT_NONE   , "m6", "m6");
}

////////////////////////////////////////////////////////////////////////
// Ogden material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOgdenUnconstrained, MODULE_MECH, FE_OGDEN_UNCONSTRAINED, FE_MAT_ELASTIC, "Ogden unconstrained", MaterialFlags::TOPLEVEL);

FSOgdenUnconstrained::FSOgdenUnconstrained(FSModel* fem) : FSMaterial(FE_OGDEN_UNCONSTRAINED, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "cp", "cp");
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE , "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE , "c6", "c6");
	AddScienceParam(1, UNIT_NONE   , "m1", "m1");
	AddScienceParam(1, UNIT_NONE   , "m2", "m2");
	AddScienceParam(1, UNIT_NONE   , "m3", "m3");
	AddScienceParam(1, UNIT_NONE   , "m4", "m4");
	AddScienceParam(1, UNIT_NONE   , "m5", "m5");
	AddScienceParam(1, UNIT_NONE   , "m6", "m6");
}

//////////////////////////////////////////////////////////////////////
// FSEFDMooneyRivlin - ellipsoidal fiber distribution model with MR base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDMooneyRivlin, MODULE_MECH, FE_EFD_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FSEFDMooneyRivlin::FSEFDMooneyRivlin(FSModel* fem) : FSMaterial(FE_EFD_MOONEY_RIVLIN, fem)
{
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//////////////////////////////////////////////////////////////////////
// FSEFDNeoHookean - ellipsoidal fiber distribution model with MR base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDNeoHookean, MODULE_MECH, FE_EFD_NEO_HOOKEAN, FE_MAT_ELASTIC, "EFD neo-Hookean", MaterialFlags::TOPLEVEL);

FSEFDNeoHookean::FSEFDNeoHookean(FSModel* fem) : FSMaterial(FE_EFD_NEO_HOOKEAN, fem)
{
	AddScienceParam(0, UNIT_PRESSURE, "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE  , "v", "Poisson's ratio");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi"  )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//////////////////////////////////////////////////////////////////////
// FSEFDDonnan - ellipsoidal fiber distribution model with Donnan base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDDonnan, MODULE_MECH, FE_EFD_DONNAN, FE_MAT_ELASTIC, "EFD Donnan equilibrium", MaterialFlags::TOPLEVEL);

FSEFDDonnan::FSEFDDonnan(FSModel* fem) : FSMaterial(FE_EFD_DONNAN, fem)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(1, UNIT_NONE, "Phi", "Phi");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

/////////////////////////////////////////////////////////////////////////////////////////
// FSEFDVerondaWestmann - ellipsoidal fiber distribution model with Veronda Westmann base
/////////////////////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDVerondaWestmann, MODULE_MECH, FE_EFD_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Veronda-Westmann", MaterialFlags::TOPLEVEL);

FSEFDVerondaWestmann::FSEFDVerondaWestmann(FSModel* fem) : FSMaterial(FE_EFD_VERONDA_WESTMANN, fem)
{
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi"  )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSCubicCLE - Conewise Linear Elasticity with cubic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCubicCLE, MODULE_MECH, FE_CLE_CUBIC, FE_MAT_ELASTIC, "cubic CLE", MaterialFlags::TOPLEVEL);

FSCubicCLE::FSCubicCLE(FSModel* fem) : FSMaterial(FE_CLE_CUBIC, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "lp1", "l+1");
    AddScienceParam(0, UNIT_PRESSURE , "lm1", "l-1");
    AddScienceParam(0, UNIT_PRESSURE , "l2" , "l2" );
    AddScienceParam(0, UNIT_PRESSURE , "mu" , "mu"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FSCubicCLE - Conewise Linear Elasticity with orthotropic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOrthotropicCLE, MODULE_MECH, FE_CLE_ORTHOTROPIC, FE_MAT_ELASTIC, "orthotropic CLE", MaterialFlags::TOPLEVEL);

FSOrthotropicCLE::FSOrthotropicCLE(FSModel* fem) : FSMaterial(FE_CLE_ORTHOTROPIC, fem)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "lp11", "l+11");
    AddScienceParam(0, UNIT_PRESSURE , "lp22", "l+22");
    AddScienceParam(0, UNIT_PRESSURE , "lp33", "l+33");
    AddScienceParam(0, UNIT_PRESSURE , "lm11", "l-11");
    AddScienceParam(0, UNIT_PRESSURE , "lm22", "l-22");
    AddScienceParam(0, UNIT_PRESSURE , "lm33", "l-33");
    AddScienceParam(0, UNIT_PRESSURE , "l12" , "l12" );
    AddScienceParam(0, UNIT_PRESSURE , "l23" , "l23" );
    AddScienceParam(0, UNIT_PRESSURE , "l31" , "l31" );
    AddScienceParam(0, UNIT_PRESSURE , "mu1" , "mu1"  );
    AddScienceParam(0, UNIT_PRESSURE , "mu2" , "mu2"  );
    AddScienceParam(0, UNIT_PRESSURE , "mu3" , "mu3"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//////////////////////////////////////////////////////////////////////
// FEHGOCoronary
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEHGOCoronary, MODULE_MECH, FE_HGO_CORONARY, FE_MAT_ELASTIC_UNCOUPLED, "HGO-coronary", MaterialFlags::TOPLEVEL);

FEHGOCoronary::FEHGOCoronary(FSModel* fem) : FSTransverselyIsotropic(FE_HGO_CORONARY, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "rho", "rho");
	AddScienceParam(0, UNIT_PRESSURE, "k1", "k1");
	AddScienceParam(0, UNIT_NONE    , "k2", "k2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}


////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxial - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxial, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI, FE_MAT_ELASTIC, "prescribed uniaxial active contraction", 0, Prescribed_Uniaxial_Active_Contraction);

FSPrescribedActiveContractionUniaxialOld::FSPrescribedActiveContractionUniaxialOld(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_UNI_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIso - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionTransIso, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO, FE_MAT_ELASTIC, "prescribed trans iso active contraction", 0, Prescribed_Transversely_Isotropic_Active_Contraction);

FSPrescribedActiveContractionTransIsoOld::FSPrescribedActiveContractionTransIsoOld(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_TISO_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxial - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxial, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI, FE_MAT_ELASTIC, "prescribed uniaxial active contraction", 0);

FSPrescribedActiveContractionUniaxial::FSPrescribedActiveContractionUniaxial(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_UNI, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0")->MakeVariable(true);
	SetAxisMaterial(new FSAxisMaterial(fem));
}

void FSPrescribedActiveContractionUniaxial::Convert(FSPrescribedActiveContractionUniaxialOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial(GetFSModel()));
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionFiber
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionFiber, MODULE_MECH, FE_ACTIVE_CONTRACT_FIBER, FE_MAT_ELASTIC, "prescribed fiber active contraction", 0);

FSPrescribedActiveContractionFiber::FSPrescribedActiveContractionFiber(FSModel* fem) : FSTransverselyIsotropic(FE_ACTIVE_CONTRACT_FIBER, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));

	AddScienceParam(0, UNIT_PRESSURE, "T0", "T0")->MakeVariable(true);
	SetAxisMaterial(new FSAxisMaterial(fem));
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionTransIso - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionTransIso, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO, FE_MAT_ELASTIC, "prescribed trans iso active contraction", 0);

FSPrescribedActiveContractionTransIso::FSPrescribedActiveContractionTransIso(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_TISO, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
	SetAxisMaterial(new FSAxisMaterial(fem));
}

void FSPrescribedActiveContractionTransIso::Convert(FSPrescribedActiveContractionTransIsoOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial(GetFSModel()));
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionIsotropic - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionIsotropic, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO, FE_MAT_ELASTIC, "prescribed isotropic active contraction", 0);

FSPrescribedActiveContractionIsotropic::FSPrescribedActiveContractionIsotropic(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_ISO, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxialUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxialUC, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed uniaxial active contraction", 0, Uncoupled_Prescribed_Uniaxial_Active_Contraction);

FSPrescribedActiveContractionUniaxialUCOld::FSPrescribedActiveContractionUniaxialUCOld(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_UNI_UC_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIsoUC - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionTransIsoUC, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed trans iso active contraction", 0, Uncoupled_Prescribed_Transversely_Isotropic_Active_Contraction);

FSPrescribedActiveContractionTransIsoUCOld::FSPrescribedActiveContractionTransIsoUCOld(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_TISO_UC_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxialUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxialUC, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed uniaxial active contraction", 0);

FSPrescribedActiveContractionUniaxialUC::FSPrescribedActiveContractionUniaxialUC(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_UNI_UC, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0")->MakeVariable(true);;
	SetAxisMaterial(new FSAxisMaterial(fem));
}

void FSPrescribedActiveContractionUniaxialUC::Convert(FSPrescribedActiveContractionUniaxialUCOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial(GetFSModel()));
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_PH);
}


////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionFiberUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionFiberUC, MODULE_MECH, FE_ACTIVE_CONTRACT_FIBER_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed fiber active contraction", 0);

FSPrescribedActiveContractionFiberUC::FSPrescribedActiveContractionFiberUC(FSModel* fem) : FSTransverselyIsotropic(FE_ACTIVE_CONTRACT_FIBER_UC, fem)
{
	SetFiberMaterial(new FSOldFiberMaterial(fem));
	AddScienceParam(0, UNIT_PRESSURE, "T0", "T0")->MakeVariable(true);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIsoUC - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionTransIsoUC, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed trans iso active contraction", 0);

FSPrescribedActiveContractionTransIsoUC::FSPrescribedActiveContractionTransIsoUC(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_TISO_UC, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
	SetAxisMaterial(new FSAxisMaterial(fem));
}

void FSPrescribedActiveContractionTransIsoUC::Convert(FSPrescribedActiveContractionTransIsoUCOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial(GetFSModel()));
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionIsotropicUC - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionIsotropicUC, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed isotropic active contraction", 0);

FSPrescribedActiveContractionIsotropicUC::FSPrescribedActiveContractionIsotropicUC(FSModel* fem) : FSMaterial(FE_ACTIVE_CONTRACT_ISO_UC, fem)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

//////////////////////////////////////////////////////////////////////
REGISTER_MATERIAL(FSIsotropicLeeSacks, MODULE_MECH, FE_ISOTROPIC_LEE_SACKS, FE_MAT_ELASTIC, "isotropic Lee-Sacks", MaterialFlags::TOPLEVEL);

FSIsotropicLeeSacks::FSIsotropicLeeSacks(FSModel* fem) : FSMaterial(FE_ISOTROPIC_LEE_SACKS, fem)
{
	AddScienceParam(1, UNIT_DENSITY , "density")->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c0");
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_NONE    , "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k");
	AddScienceParam(1, UNIT_NONE    , "tangent_scale");
}

//////////////////////////////////////////////////////////////////////
REGISTER_MATERIAL(FSIsotropicLeeSacksUncoupled, MODULE_MECH, FE_ISOTROPIC_LEE_SACKS_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled isotropic Lee-Sacks", MaterialFlags::TOPLEVEL);

FSIsotropicLeeSacksUncoupled::FSIsotropicLeeSacksUncoupled(FSModel* fem) : FSMaterial(FE_ISOTROPIC_LEE_SACKS_UNCOUPLED, fem)
{
	AddScienceParam(1, UNIT_DENSITY , "density")->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c0");
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_NONE    , "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k");
	AddScienceParam(1, UNIT_NONE    , "tangent_scale");
}

//////////////////////////////////////////////////////////////////////
// FSIsotropicFourier - Isotropic Fourier
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSIsotropicFourier, MODULE_HEAT, FE_ISOTROPIC_FOURIER, FE_MAT_HEAT_TRANSFER, "isotropic Fourier", MaterialFlags::TOPLEVEL);

FSIsotropicFourier::FSIsotropicFourier(FSModel* fem) : FSMaterial(FE_ISOTROPIC_FOURIER, fem)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, "W/L.T", "k"      , "heat conductivity");
	AddScienceParam(0, "E/T"  , "c"      , "heat capacity");
}

//=============================================================================
// Constant Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermConst, MODULE_BIPHASIC, FE_PERM_CONST, FE_MAT_PERMEABILITY, "perm-const-iso", 0);

FSPermConst::FSPermConst(FSModel* fem) : FSMaterialProp(FE_PERM_CONST, fem)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm", "permeability");
}

//=============================================================================
// Holmes-Mow Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermHolmesMow, MODULE_BIPHASIC, FE_PERM_HOLMES_MOW, FE_MAT_PERMEABILITY, "perm-Holmes-Mow", 0);

FSPermHolmesMow::FSPermHolmesMow(FSModel* fem) : FSMaterialProp(FE_PERM_HOLMES_MOW, fem)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm" , "permeability");
	AddScienceParam(0, UNIT_NONE        , "M"    , "M");
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha");
}

//=============================================================================
// Ateshian-Weiss isotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissIso, MODULE_BIPHASIC, FE_PERM_REF_ISO, FE_MAT_PERMEABILITY, "perm-ref-iso", 0);

FSPermAteshianWeissIso::FSPermAteshianWeissIso(FSModel* fem) : FSMaterialProp(FE_PERM_REF_ISO, fem)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0", "perm0");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1", "perm1");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2", "perm2");
	AddScienceParam(0, UNIT_NONE        , "M"    , "M");
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha");
}

//=============================================================================
// Ateshian-Weiss trans-isotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissTransIso, MODULE_BIPHASIC, FE_PERM_REF_TRANS_ISO, FE_MAT_PERMEABILITY, "perm-ref-trans-iso", 0);

FSPermAteshianWeissTransIso::FSPermAteshianWeissTransIso(FSModel* fem) : FSMaterialProp(FE_PERM_REF_TRANS_ISO, fem)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0" , "perm0" );
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1T", "perm1T");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1A", "perm1A");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2T", "perm2T");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2A", "perm2A");
	AddScienceParam(0, UNIT_NONE        , "M0"    , "M0"    );
	AddScienceParam(0, UNIT_NONE        , "MT"    , "MT"    );
	AddScienceParam(0, UNIT_NONE        , "MA"    , "MA"    );
	AddScienceParam(0, UNIT_NONE        , "alpha0", "alpha0");
	AddScienceParam(0, UNIT_NONE        , "alphaA", "alphaA");
	AddScienceParam(0, UNIT_NONE        , "alphaT", "alphaT");

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// Ateshian-Weiss orthotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissOrtho, MODULE_BIPHASIC, FE_PERM_REF_ORTHO, FE_MAT_PERMEABILITY, "perm-ref-ortho", 0);

FSPermAteshianWeissOrtho::FSPermAteshianWeissOrtho(FSModel* fem) : FSMaterialProp(FE_PERM_REF_ORTHO, fem)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0" , "perm0");
	AddVecParam(vec3d(0,0,0), "perm1" , "perm1")->SetUnit(UNIT_PERMEABILITY);
	AddVecParam(vec3d(0,0,0), "perm2" , "perm2")->SetUnit(UNIT_PERMEABILITY);
	AddScienceParam(0, UNIT_NONE        , "M0"    , "M0");
	AddScienceParam(0, UNIT_NONE        , "alpha0", "alpha0");
	AddVecParam(vec3d(0,0,0), "M"     , "M");
	AddVecParam(vec3d(0,0,0), "alpha" , "alpha");

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// Exponential Isotropic Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermExpIso, MODULE_BIPHASIC, FE_PERM_EXP_ISO, FE_MAT_PERMEABILITY, "perm-exp-iso", 0);

FSPermExpIso::FSPermExpIso(FSModel* fem) : FSMaterialProp(FE_PERM_EXP_ISO, fem)
{
    AddScienceParam(0, UNIT_PERMEABILITY, "perm" , "permeability");
    AddScienceParam(0, UNIT_NONE        , "M"    , "M");
}

//=============================================================================
// constant diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffConst, MODULE_BIPHASIC, FE_DIFF_CONST, FE_MAT_DIFFUSIVITY, "diff-const-iso", 0);

FSDiffConst::FSDiffConst(FSModel* fem) : FSMaterialProp(FE_DIFF_CONST, fem)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff"     , "diffusivity");
}

//=============================================================================
// orthotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffOrtho, MODULE_BIPHASIC, FE_DIFF_CONST_ORTHO, FE_MAT_DIFFUSIVITY, "diff-const-ortho", 0);

FSDiffOrtho::FSDiffOrtho(FSModel* fem) : FSMaterialProp(FE_DIFF_CONST_ORTHO, fem)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddVecParam(vec3d(0,0,0), "diff", "diffusivity")->SetUnit(UNIT_DIFFUSIVITY);
}

//=============================================================================
// Ateshian-Weiss isotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffAteshianWeissIso, MODULE_BIPHASIC, FE_DIFF_REF_ISO, FE_MAT_DIFFUSIVITY, "diff-ref-iso", 0);

FSDiffAteshianWeissIso::FSDiffAteshianWeissIso(FSModel* fem) : FSMaterialProp(FE_DIFF_REF_ISO, fem)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff0"    , "diff0");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff1"    , "diff1");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff2"    , "diff2");
	AddScienceParam(0, UNIT_NONE       , "M"        , "M"    );
	AddScienceParam(0, UNIT_NONE       , "alpha"    , "alpha");
}

//=============================================================================
// Albro isotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffAlbroIso, MODULE_BIPHASIC, FE_DIFF_ALBRO_ISO, FE_MAT_DIFFUSIVITY, "diff-Albro-iso", 0);

FSDiffAlbroIso::FSDiffAlbroIso(FSModel* fem) : FSMaterialProp(FE_DIFF_ALBRO_ISO, fem)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_NONE       , "cdinv"    , "cdinv");
	AddScienceParam(0, UNIT_NONE       , "alphad"   , "alphad");
}

//=============================================================================
// constant solubility
//=============================================================================

REGISTER_MATERIAL(FSSolubConst, MODULE_BIPHASIC, FE_SOLUB_CONST, FE_MAT_SOLUBILITY, "solub-const", 0);

FSSolubConst::FSSolubConst(FSModel* fem) : FSMaterialProp(FE_SOLUB_CONST, fem)
{
	AddScienceParam(1, UNIT_NONE, "solub", "solubility");
}

//=============================================================================
// constant osmotic coefficient
//=============================================================================

REGISTER_MATERIAL(FSOsmoConst, MODULE_BIPHASIC, FE_OSMO_CONST, FE_MAT_OSMOTIC_COEFFICIENT, "osm-coef-const", 0);

FSOsmoConst::FSOsmoConst(FSModel* fem) : FSMaterialProp(FE_OSMO_CONST, fem)
{
	AddScienceParam(1, UNIT_NONE, "osmcoef", "osmotic coefficient");
}

//=============================================================================
// Wells-Manning osmotic coefficient
//=============================================================================

REGISTER_MATERIAL(FSOsmoWellsManning, MODULE_BIPHASIC, FE_OSMO_WM, FE_MAT_OSMOTIC_COEFFICIENT, "osm-coef-Manning", 0);

FSOsmoWellsManning::FSOsmoWellsManning(FSModel* fem) : FSMaterialProp(FE_OSMO_WM, fem)
{
    AddScienceParam(1, UNIT_NONE, "ksi", "ksi");
    AddChoiceParam(0, "co_ion", "co-ion")->SetEnumNames("$(solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//=============================================================================
// SFD compressible
//=============================================================================

REGISTER_MATERIAL(FSSFDCoupled, MODULE_MECH, FE_SFD_COUPLED, FE_MAT_ELASTIC, "spherical fiber distribution", 0);

FSSFDCoupled::FSSFDCoupled(FSModel* fem) : FSMaterial(FE_SFD_COUPLED, fem)
{
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE        , "beta", "beta");
	AddScienceParam(0, UNIT_PRESSURE    , "ksi" , "ksi" );
}

//=============================================================================
// SFD SBM
//=============================================================================

REGISTER_MATERIAL(FSSFDSBM, MODULE_MECH, FE_SFD_SBM, FE_MAT_ELASTIC, "spherical fiber distribution sbm", 0);

FSSFDSBM::FSSFDSBM(FSModel* fem) : FSMaterial(FE_SFD_SBM, fem)
{
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha" );
	AddScienceParam(0, UNIT_NONE        , "beta", "beta"   );
	AddScienceParam(0, UNIT_NONE        , "ksi0" , "ksi0"  );
	AddScienceParam(1, UNIT_NONE        , "rho0" , "rho0"  );
	AddScienceParam(0, UNIT_NONE        , "gamma" , "gamma");
	AddIntParam    (-1                   , "sbm"   , "sbm"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// EFD Coupled
//=============================================================================

REGISTER_MATERIAL(FSEFDCoupled, MODULE_MECH, FE_EFD_COUPLED, FE_MAT_ELASTIC, "ellipsoidal fiber distribution", 0);

FSEFDCoupled::FSEFDCoupled(FSModel* fem) : FSMaterial(FE_EFD_COUPLED, fem)
{
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi" , "ksi" )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// EFD Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSEFDUncoupled, MODULE_MECH, FE_EFD_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "EFD uncoupled", 0);

FSEFDUncoupled::FSEFDUncoupled(FSModel* fem) : FSMaterial(FE_EFD_UNCOUPLED, fem)
{
	AddVecParam(vec3d(0,0,0), "beta" , "beta");
	AddVecParam(vec3d(0,0,0), "ksi" , "ksi")->SetUnit(UNIT_PRESSURE);
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial(fem));
}

//=============================================================================
// FSFiberMaterial
//=============================================================================

FSFiberMaterial::FSFiberMaterial(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{
	AddProperty("fiber", FE_MAT_FIBER_GENERATOR);
}

bool FSFiberMaterial::HasFibers() { return true; }

void FSFiberMaterial::SetFiberGenerator(FSFiberGenerator* v)
{
	GetProperty(0).SetComponent(v);
}

FSFiberGenerator* FSFiberMaterial::GetFiberGenerator()
{
	return dynamic_cast<FSFiberGenerator*>(GetProperty(0).GetComponent());
}

void FSFiberMaterial::SetAxisMaterial(FSAxisMaterial* Q)
{
	FSModel* fem = GetFSModel();
	// If the fiber generator was not set we'll create a fiber generator from the mat axes
	FSFiberGenerator* v = dynamic_cast<FSFiberGenerator*>(GetProperty(0).GetComponent());
	if (v == nullptr)
	{
		switch (Q->m_naopt)
		{
		case FE_AXES_LOCAL:
			SetFiberGenerator(new FSFiberGeneratorLocal(fem, Q->m_n[0], Q->m_n[1]));
			break;
		case FE_AXES_VECTOR:
			SetFiberGenerator(new FSFiberGeneratorVector(fem, Q->m_a));
			break;
		case FE_AXES_ANGLES:
			SetFiberGenerator(new FSAnglesVectorGenerator(fem, Q->m_theta, Q->m_phi));
			break;
		case FE_AXES_CYLINDRICAL:
			SetFiberGenerator(new FSCylindricalVectorGenerator(fem, Q->m_center, Q->m_axis, Q->m_vec));
			break;
		case FE_AXES_SPHERICAL:
			SetFiberGenerator(new FSSphericalVectorGenerator(fem, Q->m_center, Q->m_vec));
			break;
		default:
			assert(false);
		}
		delete Q;
	}
	else FSMaterial::SetAxisMaterial(Q);
}

vec3d FSFiberMaterial::GetFiber(FEElementRef& el)
{
	FSFiberGenerator* fiber = dynamic_cast<FSFiberGenerator*>(GetProperty(0).GetComponent());
	vec3d v(1, 0, 0);
	if (fiber) v = fiber->GetFiber(el);
	if (m_axes)
	{
		mat3d Q = m_axes->GetMatAxes(el);
		v = Q * v;
	}
	const FSMaterial* parentMat = GetParentMaterial();
	if (parentMat && parentMat->m_axes)
	{
		mat3d Q = parentMat->GetMatAxes(el);
		v = Q * v;
	}

	return v;
}

//=============================================================================
// Fiber-Exp-Pow
//=============================================================================

//REGISTER_MATERIAL(FSFiberExpPow, MODULE_MECH, FE_FIBEREXPPOW_COUPLED, FE_MAT_ELASTIC, "fiber-exp-pow", 0, Fiber_with_Exponential_Power_Law);

FSFiberExpPowOld::FSFiberExpPowOld(FSModel* fem) : FSMaterial(FE_FIBEREXPPOW_COUPLED_OLD, fem)
{
	AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

REGISTER_MATERIAL(FSFiberExpPow, MODULE_MECH, FE_FIBEREXPPOW_COUPLED, FE_MAT_ELASTIC, "fiber-exp-pow", 0);

FSFiberExpPow::FSFiberExpPow(FSModel* fem) : FSFiberMaterial(FE_FIBEREXPPOW_COUPLED, fem)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
}

void FSFiberExpPow::Convert(FSFiberExpPowOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_ALPHA, pold->GetFloatValue(FSFiberExpPowOld::MP_ALPHA));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberExpPowOld::MP_BETA ));
    SetFloatValue(MP_KSI  , pold->GetFloatValue(FSFiberExpPowOld::MP_KSI  ));

	double the = pold->GetFloatValue(FSFiberExpPowOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberExpPowOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(GetFSModel(), the, phi));
}

//=============================================================================
// Fiber-Exp-Linear
//=============================================================================

REGISTER_MATERIAL(FSFiberExpLinear, MODULE_MECH, FE_FIBEREXPLIN_COUPLED, FE_MAT_ELASTIC, "fiber-exp-linear", 0);

FSFiberExpLinear::FSFiberExpLinear(FSModel* fem) : FSFiberMaterial(FE_FIBEREXPLIN_COUPLED, fem)
{
	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "lambda");
}

//=============================================================================
// Fiber-Exp-Linear uncoupled
//=============================================================================

REGISTER_MATERIAL(FSFiberExpLinearUncoupled, MODULE_MECH, FE_FIBEREXPLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled fiber-exp-linear", 0);

FSFiberExpLinearUncoupled::FSFiberExpLinearUncoupled(FSModel* fem) : FSFiberMaterial(FE_FIBEREXPLIN_UNCOUPLED, fem)
{
	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "lambda");
}

//=============================================================================
// Fiber-Exp-Pow-Linear
//=============================================================================

REGISTER_MATERIAL(FSFiberExpPowLin, MODULE_MECH, FE_FIBER_EXP_POW_LIN, FE_MAT_ELASTIC, "fiber-exp-pow-linear", 0);

FSFiberExpPowLin::FSFiberExpPowLin(FSModel* fem) : FSFiberMaterial(FE_FIBER_EXP_POW_LIN, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E"  , "E"  );
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
}

//=============================================================================
// Fiber-Neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSFiberNeoHookean, MODULE_MECH, FE_FIBER_NEO_HOOKEAN, FE_MAT_ELASTIC, "fiber-NH", 0);

FSFiberNeoHookean::FSFiberNeoHookean(FSModel* fem) : FSFiberMaterial(FE_FIBER_NEO_HOOKEAN, fem)
{
    AddDoubleParam(0.0, "mu", "mu");
}

//=============================================================================
// Fiber-Natural-Neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSFiberNaturalNH, MODULE_MECH, FE_FIBER_NATURAL_NH, FE_MAT_ELASTIC, "fiber-natural-NH", 0);

FSFiberNaturalNH::FSFiberNaturalNH(FSModel* fem) : FSFiberMaterial(FE_FIBER_NATURAL_NH, fem)
{
    AddDoubleParam(0.0, "ksi", "ksi");
    AddDoubleParam(1.0, "lam0", "lam0");
}

//=============================================================================
// damage fiber power
//=============================================================================

REGISTER_MATERIAL(FSFiberDamagePower, MODULE_MECH, FE_FIBER_DAMAGE_POWER, FE_MAT_ELASTIC, "damage fiber power", 0);

FSFiberDamagePower::FSFiberDamagePower(FSModel* fem) : FSFiberMaterial(FE_FIBER_DAMAGE_POWER, fem)
{
	AddDoubleParam(0.0, "a1", "a1");
	AddDoubleParam(0.0, "a2", "a2");
	AddDoubleParam(0.0, "kappa", "kappa");
	AddDoubleParam(0.0, "t0", "t0");
	AddDoubleParam(0.0, "Dmax", "Dmax");
	AddDoubleParam(0.0, "beta_s", "beta_s");
	AddDoubleParam(0.0, "gamma_max", "gamma_max");
}

//=============================================================================
// damage fiber exponential
//=============================================================================

REGISTER_MATERIAL(FSFiberDamageExponential, MODULE_MECH, FE_FIBER_DAMAGE_EXP, FE_MAT_ELASTIC, "damage fiber exponential", 0);

FSFiberDamageExponential::FSFiberDamageExponential(FSModel* fem) : FSFiberMaterial(FE_FIBER_DAMAGE_EXP, fem)
{
	AddDoubleParam(0.0, "k1", "k1");
	AddDoubleParam(0.0, "k2", "k2");
	AddDoubleParam(0.0, "kappa", "kappa");
	AddDoubleParam(0.0, "t0", "t0");
	AddDoubleParam(0.0, "Dmax", "Dmax");
	AddDoubleParam(0.0, "beta_s", "beta_s");
	AddDoubleParam(0.0, "gamma_max", "gamma_max");
}

//=============================================================================
// damage fiber exp-linear
//=============================================================================

REGISTER_MATERIAL(FSFiberDamageExpLinear, MODULE_MECH, FE_FIBER_DAMAGE_EXPLINEAR, FE_MAT_ELASTIC, "damage fiber exp-linear", 0);

FSFiberDamageExpLinear::FSFiberDamageExpLinear(FSModel* fem) : FSFiberMaterial(FE_FIBER_DAMAGE_EXPLINEAR, fem)
{
	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "lambda");
	AddDoubleParam(0.0, "t0", "t0");
	AddDoubleParam(0.0, "Dmax", "Dmax");
	AddDoubleParam(0.0, "beta_s", "beta_s");
	AddDoubleParam(0.0, "gamma_max", "gamma_max");

	AddDoubleParam(0.0, "D2_a");
	AddDoubleParam(0.0, "D2_b");
	AddDoubleParam(0.0, "D2_c");
	AddDoubleParam(0.0, "D2_d");
	AddDoubleParam(0.0, "D3_inf");
	AddDoubleParam(0.0, "D3_g0");
	AddDoubleParam(1.0, "D3_rg");
}

//=============================================================================
// Fiber-Kiousis-Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSFiberKiousisUncoupled, MODULE_MECH, FE_FIBER_KIOUSIS_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-Kiousis-uncoupled", 0);

FSFiberKiousisUncoupled::FSFiberKiousisUncoupled(FSModel* fem) : FSFiberMaterial(FE_FIBER_KIOUSIS_UNCOUPLED, fem)
{
    AddDoubleParam(0.0, "d1", "d1");
    AddDoubleParam(1.0, "d2", "d2");
    AddDoubleParam(2.0, "n", "n");
}

//=============================================================================
// Fiber-Exp-Pow Uncoupled
//=============================================================================

//REGISTER_MATERIAL(FSFiberExpPowUncoupled, MODULE_MECH, FE_FIBEREXPPOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-exp-pow-uncoupled", 0, Fiber_with_Exponential_Power_Law_Uncoupled_Formulation);

FSFiberExpPowUncoupledOld::FSFiberExpPowUncoupledOld(FSModel* fem) : FSMaterial(FE_FIBEREXPPOW_UNCOUPLED_OLD, fem)
{
	AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

REGISTER_MATERIAL(FSFiberExpPowUncoupled, MODULE_MECH, FE_FIBEREXPPOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-exp-pow-uncoupled", 0);

FSFiberExpPowUncoupled::FSFiberExpPowUncoupled(FSModel* fem) : FSFiberMaterial(FE_FIBEREXPPOW_UNCOUPLED, fem)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

void FSFiberExpPowUncoupled::Convert(FSFiberExpPowUncoupledOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_ALPHA, pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_ALPHA));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_BETA ));
    SetFloatValue(MP_KSI  , pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_KSI  ));
    SetFloatValue(MP_K    , pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_K    ));

	double the = pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberExpPowUncoupledOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(GetFSModel(), the, phi));
}

//=============================================================================
// Fiber-Pow-Linear
//=============================================================================

//REGISTER_MATERIAL(FSFiberPowLin, MODULE_MECH, FE_FIBERPOWLIN_COUPLED, FE_MAT_ELASTIC, "fiber-pow-linear", 0, Fiber_with_Toe_Linear_Response);

FSFiberPowLinOld::FSFiberPowLinOld(FSModel* fem) : FSMaterial(FE_FIBERPOWLIN_COUPLED_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "beta");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

REGISTER_MATERIAL(FSFiberPowLin, MODULE_MECH, FE_FIBERPOWLIN_COUPLED, FE_MAT_ELASTIC, "fiber-pow-linear", 0);

FSFiberPowLin::FSFiberPowLin(FSModel* fem) : FSFiberMaterial(FE_FIBERPOWLIN_COUPLED, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "fiber modulus E");
    AddScienceParam(2, UNIT_NONE, "beta" , "toe power exponent beta");
    AddScienceParam(1, UNIT_NONE, "lam0" , "toe stretch ratio lam0");
}

void FSFiberPowLin::Convert(FSFiberPowLinOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_E    , pold->GetFloatValue(FSFiberPowLinOld::MP_E   ));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberPowLinOld::MP_BETA));
    SetFloatValue(MP_LAM0 , pold->GetFloatValue(FSFiberPowLinOld::MP_LAM0));

	double the = pold->GetFloatValue(FSFiberPowLinOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberPowLinOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(GetFSModel(), the, phi));
}

//=============================================================================
// Fiber-Pow-Linear Uncoupled
//=============================================================================

//REGISTER_MATERIAL(FSFiberPowLinUncoupled, MODULE_MECH, FE_FIBERPOWLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-pow-linear-uncoupled", 0, Fiber_with_Toe_Linear_Response_Uncoupled_Formulation);

FSFiberPowLinUncoupledOld::FSFiberPowLinUncoupledOld(FSModel* fem) : FSMaterial(FE_FIBERPOWLIN_UNCOUPLED_OLD, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "beta");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial(fem));
}

REGISTER_MATERIAL(FSFiberPowLinUncoupled, MODULE_MECH, FE_FIBERPOWLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-pow-linear-uncoupled", 0);

FSFiberPowLinUncoupled::FSFiberPowLinUncoupled(FSModel* fem) : FSFiberMaterial(FE_FIBERPOWLIN_UNCOUPLED, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "fiber modulus E");
    AddScienceParam(2, UNIT_NONE, "beta" , "toe power exponent");
    AddScienceParam(1, UNIT_NONE, "lam0" , "toe stretch ratio");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

void FSFiberPowLinUncoupled::Convert(FSFiberPowLinUncoupledOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_E    , pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_E   ));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_BETA));
    SetFloatValue(MP_LAM0 , pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_LAM0));
    SetFloatValue(MP_K    , pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_K   ));

	double the = pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberPowLinUncoupledOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(GetFSModel(), the, phi));
}

//=============================================================================
// Donnan swelling
//=============================================================================

REGISTER_MATERIAL(FSDonnanSwelling, MODULE_MECH, FE_DONNAN_SWELLING, FE_MAT_ELASTIC, "Donnan equilibrium", 0);

FSDonnanSwelling::FSDonnanSwelling(FSModel* fem) : FSMaterial(FE_DONNAN_SWELLING, fem)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(0, UNIT_NONE, "Phi", "Phi");
}

//=============================================================================
// Perfect Osmometer
//=============================================================================

REGISTER_MATERIAL(FSPerfectOsmometer, MODULE_MECH, FE_PERFECT_OSMOMETER, FE_MAT_ELASTIC, "perfect osmometer", 0);

FSPerfectOsmometer::FSPerfectOsmometer(FSModel* fem) : FSMaterial(FE_PERFECT_OSMOMETER, fem)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "iosm", "iosm");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
}

//=============================================================================
// Cell Growth
//=============================================================================

REGISTER_MATERIAL(FSCellGrowth, MODULE_MECH, FE_CELL_GROWTH, FE_MAT_ELASTIC, "cell growth", 0);

FSCellGrowth::FSCellGrowth(FSModel* fem) : FSMaterial(FE_CELL_GROWTH, fem)
{
	AddScienceParam(0, UNIT_NONE, "phir", "phir");
	AddScienceParam(0, UNIT_CONCENTRATION, "cr", "cr");
	AddScienceParam(0, UNIT_CONCENTRATION, "ce", "ce");
}

//=============================================================================
// Osmotic pressure using virial coefficients
//=============================================================================

REGISTER_MATERIAL(FSOsmoticVirial, MODULE_MECH, FE_OSMOTIC_VIRIAL, FE_MAT_ELASTIC, "osmotic virial expansion", 0);

FSOsmoticVirial::FSOsmoticVirial(FSModel* fem) : FSMaterial(FE_OSMOTIC_VIRIAL, fem)
{
    AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
    AddScienceParam(0, UNIT_CONCENTRATION, "cr", "cr");
    AddDoubleParam(0, "c1", "c1");
    AddDoubleParam(0, "c2", "c2");
    AddDoubleParam(0, "c3", "c3");
}

//=============================================================================
// Constant reaction rate
//=============================================================================

REGISTER_MATERIAL(FSReactionRateConst, MODULE_REACTIONS, FE_REACTION_RATE_CONST, FE_MAT_REACTION_RATE, "constant reaction rate", 0);

FSReactionRateConst::FSReactionRateConst(FSModel* fem) : FSMaterialProp(FE_REACTION_RATE_CONST, fem)
{
	AddDoubleParam(0, "k", "k");
}

double FSReactionRateConst::GetRateConstant() { return GetParam(0).GetFloatValue(); }

void FSReactionRateConst::SetRateConstant(double K) { SetFloatValue(0, K); }

//=============================================================================
// Huiskes reaction rate
//=============================================================================

REGISTER_MATERIAL(FSReactionRateHuiskes, MODULE_REACTIONS, FE_REACTION_RATE_HUISKES, FE_MAT_REACTION_RATE, "Huiskes reaction rate", 0);

FSReactionRateHuiskes::FSReactionRateHuiskes(FSModel* fem) : FSMaterialProp(FE_REACTION_RATE_HUISKES, fem)
{
	AddDoubleParam(0, "B", "B");
	AddDoubleParam(0, "psi0", "psi0");
}

//=============================================================================
REGISTER_MATERIAL(FEBioReactionRate, MODULE_REACTIONS, FE_REACTION_RATE_FEBIO, FE_MAT_REACTION_RATE, "", 0);
FEBioReactionRate::FEBioReactionRate(FSModel* fem) : FSMaterialProp(FE_REACTION_RATE_FEBIO, fem)
{

}
void FEBioReactionRate::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSMaterial::Save(ar);
	}
	ar.EndChunk();
}

void FEBioReactionRate::Load(IArchive& ar)
{
	TRACE("FEBioReactionRate::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSMaterial::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
}

//=============================================================================
// Membrane constant reaction rate
//=============================================================================

REGISTER_MATERIAL(FSMembraneReactionRateConst, MODULE_REACTIONS, FE_MREACTION_RATE_CONST, FE_MAT_MREACTION_RATE, "membrane constant reaction rate", 0);

FSMembraneReactionRateConst::FSMembraneReactionRateConst(FSModel* fem) : FSMaterialProp(FE_MREACTION_RATE_CONST, fem)
{
    AddDoubleParam(0, "k", "k");
}

double FSMembraneReactionRateConst::GetRateConstant() { return GetParam(0).GetFloatValue(); }

void FSMembraneReactionRateConst::SetRateConstant(double K) { SetFloatValue(0, K); }

//=============================================================================
// Membrane ion channel reaction rate
//=============================================================================

REGISTER_MATERIAL(FSMembraneReactionRateIonChannel, MODULE_REACTIONS, FE_MREACTION_RATE_ION_CHNL, FE_MAT_MREACTION_RATE, "membrane ion channel reaction rate", 0);

FSMembraneReactionRateIonChannel::FSMembraneReactionRateIonChannel(FSModel* fem) : FSMaterialProp(FE_MREACTION_RATE_ION_CHNL, fem)
{
    AddScienceParam(0, UNIT_CURRENT_CONDUCTIVITY, "g", "g");
    AddIntParam(0,"sol","sol");
}

double FSMembraneReactionRateIonChannel::GetConductivity() { return GetParam(0).GetFloatValue(); }

void FSMembraneReactionRateIonChannel::SetConductivity(double g) { SetFloatValue(0, g); }

int FSMembraneReactionRateIonChannel::GetSolute()  { return GetParam(1).GetIntValue(); }
void FSMembraneReactionRateIonChannel::SetSolute(int isol) { SetIntValue(1, isol); }

//=============================================================================
// Membrane voltage-gated channel reaction rate
//=============================================================================

REGISTER_MATERIAL(FSMembraneReactionRateVoltageGated, MODULE_REACTIONS, FE_MREACTION_RATE_VOLT_GTD, FE_MAT_MREACTION_RATE, "membrane voltage-gated reaction rate", 0);

FSMembraneReactionRateVoltageGated::FSMembraneReactionRateVoltageGated(FSModel* fem) : FSMaterialProp(FE_MREACTION_RATE_VOLT_GTD, fem)
{
    AddDoubleParam(0, "a", "a");
    AddDoubleParam(0, "b", "b");
    AddDoubleParam(0, "c", "c");
    AddDoubleParam(0, "d", "d");
    AddIntParam(0,"sol","sol");
}

double FSMembraneReactionRateVoltageGated::GetConstant(int i) { return GetParam(i).GetFloatValue(); }

void FSMembraneReactionRateVoltageGated::SetConstant(int i, double c) { SetFloatValue(i, c); }

int FSMembraneReactionRateVoltageGated::GetSolute()  { return GetParam(4).GetIntValue(); }
void FSMembraneReactionRateVoltageGated::SetSolute(int isol) { SetIntValue(1, isol); }

//=============================================================================
// CFD Fiber-Exponential-Power-Law
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberExpPow, MODULE_MECH, FE_FIBER_EXP_POW, FE_MAT_CFD_FIBER, "fiber-exp-pow", 0);

FSCFDFiberExpPow::FSCFDFiberExpPow(FSModel* fem) : FSMaterial(FE_FIBER_EXP_POW, fem)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
}

//=============================================================================
// CFD Fiber-neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberNH, MODULE_MECH, FE_FIBER_NH, FE_MAT_CFD_FIBER, "fiber-NH", 0);

FSCFDFiberNH::FSCFDFiberNH(FSModel* fem) : FSMaterial(FE_FIBER_NH, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu");
}

//=============================================================================
// CFD Fiber-Power-Linear
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberPowLinear, MODULE_MECH, FE_FIBER_POW_LIN, FE_MAT_CFD_FIBER, "fiber-pow-linear", 0);

FSCFDFiberPowLinear::FSCFDFiberPowLinear(FSModel* fem) : FSMaterial(FE_FIBER_POW_LIN, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E"   , "fiber modulus");
    AddScienceParam(2, UNIT_NONE    , "beta", "toe power exponent");
    AddScienceParam(1, UNIT_NONE    , "lam0", "toe stretch ratio");
}

//=============================================================================
// CFD Fiber-Exponential-Power-Law uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberExpPowUC, MODULE_MECH, FE_FIBER_EXP_POW_UC, FE_MAT_CFD_FIBER_UC, "fiber-exp-pow-uncoupled", 0);

FSCFDFiberExpPowUC::FSCFDFiberExpPowUC(FSModel* fem) : FSMaterial(FE_FIBER_EXP_POW_UC, fem)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// CFD Fiber-neo-Hookean uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberNHUC, MODULE_MECH, FE_FIBER_NH_UC, FE_MAT_CFD_FIBER_UC, "fiber-NH-uncoupled", 0);

FSCFDFiberNHUC::FSCFDFiberNHUC(FSModel* fem) : FSMaterial(FE_FIBER_NH_UC, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// CFD Fiber-Power-Linear uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberPowLinearUC, MODULE_MECH, FE_FIBER_POW_LIN_UC, FE_MAT_CFD_FIBER_UC, "fiber-pow-linear-uncoupled", 0);

FSCFDFiberPowLinearUC::FSCFDFiberPowLinearUC(FSModel* fem) : FSMaterial(FE_FIBER_POW_LIN_UC, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "E"   , "fiber modulus");
    AddScienceParam(2, UNIT_NONE    , "beta", "toe power exponent");
    AddScienceParam(1, UNIT_NONE    , "lam0", "toe stretch ratio");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// FDD Spherical
//=============================================================================

REGISTER_MATERIAL(FSFDDSpherical, MODULE_MECH, FE_DSTRB_SFD, FE_MAT_CFD_DIST, "spherical", 0);

FSFDDSpherical::FSFDDSpherical(FSModel* fem) : FSMaterialProp(FE_DSTRB_SFD, fem)
{
}

//=============================================================================
// FDD Ellipsoidal
//=============================================================================

REGISTER_MATERIAL(FSFDDEllipsoidal, MODULE_MECH, FE_DSTRB_EFD, FE_MAT_CFD_DIST, "ellipsoidal", 0);

FSFDDEllipsoidal::FSFDDEllipsoidal(FSModel* fem) : FSMaterialProp(FE_DSTRB_EFD, fem)
{
    AddVecParam(vec3d(1,1,1), "spa" , "spa");
}

//=============================================================================
// FDD von Mises 3d
//=============================================================================

REGISTER_MATERIAL(FSFDDvonMises3d, MODULE_MECH, FE_DSTRB_VM3, FE_MAT_CFD_DIST, "von-Mises-3d", 0);

FSFDDvonMises3d::FSFDDvonMises3d(FSModel* fem) : FSMaterialProp(FE_DSTRB_VM3, fem)
{
    AddDoubleParam(0, "b"   , "concentration");
}

//=============================================================================
// FDD Circular
//=============================================================================

REGISTER_MATERIAL(FSFDDCircular, MODULE_MECH, FE_DSTRB_CFD, FE_MAT_CFD_DIST, "circular", 0);

FSFDDCircular::FSFDDCircular(FSModel* fem) : FSMaterialProp(FE_DSTRB_CFD, fem)
{
}

//=============================================================================
// FDD Elliptical
//=============================================================================

REGISTER_MATERIAL(FSFDDElliptical, MODULE_MECH, FE_DSTRB_PFD, FE_MAT_CFD_DIST, "elliptical", 0);

FSFDDElliptical::FSFDDElliptical(FSModel* fem) : FSMaterialProp(FE_DSTRB_PFD, fem)
{
    AddScienceParam(0, UNIT_NONE, "spa1"   , "spa1");
    AddScienceParam(0, UNIT_NONE, "spa2"   , "spa2");
}

//=============================================================================
// FDD von Mises 2d
//=============================================================================

REGISTER_MATERIAL(FSFDDvonMises2d, MODULE_MECH, FE_DSTRB_VM2, FE_MAT_CFD_DIST, "von-Mises-2d", 0);

FSFDDvonMises2d::FSFDDvonMises2d(FSModel* fem) : FSMaterialProp(FE_DSTRB_VM2, fem)
{
    AddScienceParam(0, UNIT_NONE, "b"   , "concentration");
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal
//=============================================================================

REGISTER_MATERIAL(FSSchemeGKT, MODULE_MECH, FE_SCHM_GKT, FE_MAT_CFD_SCHEME, "fibers-3d-gkt", 0);

FSSchemeGKT::FSSchemeGKT(FSModel* fem) : FSMaterialProp(FE_SCHM_GKT, fem)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");// enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration
//=============================================================================

REGISTER_MATERIAL(FSSchemeFEI, MODULE_MECH, FE_SCHM_FEI, FE_MAT_CFD_SCHEME, "fibers-3d-fei", 0);

FSSchemeFEI::FSSchemeFEI(FSModel* fem) : FSMaterialProp(FE_SCHM_FEI, fem)
{
    AddIntParam(1796, "resolution"   , "resolution");// choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d
//=============================================================================

REGISTER_MATERIAL(FSSchemeT2d, MODULE_MECH, FE_SCHM_T2D, FE_MAT_CFD_SCHEME, "fibers-2d-trapezoidal", 0);

FSSchemeT2d::FSSchemeT2d(FSModel* fem) : FSMaterialProp(FE_SCHM_T2D, fem)
{
    AddIntParam(31, "nth"   , "nth");// odd and >= 3
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeGKTUC, MODULE_MECH, FE_SCHM_GKT_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-gkt-uncoupled", 0);

FSSchemeGKTUC::FSSchemeGKTUC(FSModel* fem) : FSMaterialProp(FE_SCHM_GKT_UC, fem)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");//  enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeFEIUC, MODULE_MECH, FE_SCHM_FEI_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-fei-uncoupled", 0);

FSSchemeFEIUC::FSSchemeFEIUC(FSModel* fem) : FSMaterialProp(FE_SCHM_FEI_UC, fem)
{
    AddIntParam(11, "resolution"   , "resolution"); // choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeT2dUC, MODULE_MECH, FE_SCHM_T2D_UC, FE_MAT_CFD_SCHEME_UC, "fibers-2d-trapezoidal-uncoupled", 0);

FSSchemeT2dUC::FSSchemeT2dUC(FSModel* fem) : FSMaterialProp(FE_SCHM_T2D_UC, fem)
{
    AddIntParam(31, "nth"   , "nth"); // nth (odd and >= 3)
}

//=============================================================================
// CDF Simo
//=============================================================================

REGISTER_MATERIAL(FSCDFSimo, MODULE_MECH, FE_CDF_SIMO, FE_MAT_DAMAGE, "CDF Simo", 0);

FSCDFSimo::FSCDFSimo(FSModel* fem) : FSMaterialProp(FE_CDF_SIMO, fem)
{
	AddDoubleParam(0, "a", "a");
    AddScienceParam(0, UNIT_NONE, "b" , "b");
}

//=============================================================================
// CDF Log Normal
//=============================================================================

REGISTER_MATERIAL(FSCDFLogNormal, MODULE_MECH, FE_CDF_LOG_NORMAL, FE_MAT_DAMAGE, "CDF log-normal", 0);

FSCDFLogNormal::FSCDFLogNormal(FSModel* fem) : FSMaterialProp(FE_CDF_LOG_NORMAL, fem)
{
	AddDoubleParam(0, "mu" , "mu");
	AddScienceParam(0, UNIT_NONE, "sigma" , "sigma");
	AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax");
}

//=============================================================================
// CDF Weibull
//=============================================================================

REGISTER_MATERIAL(FSCDFWeibull, MODULE_MECH, FE_CDF_WEIBULL, FE_MAT_DAMAGE, "CDF Weibull", 0);

FSCDFWeibull::FSCDFWeibull(FSModel* fem) : FSMaterialProp(FE_CDF_WEIBULL, fem)
{
	AddDoubleParam(0, "mu" , "mu");
	AddScienceParam(0, UNIT_NONE, "alpha" , "alpha");
	AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax");
}

//=============================================================================
// CDF Step
//=============================================================================

REGISTER_MATERIAL(FSCDFStep, MODULE_MECH, FE_CDF_STEP, FE_MAT_DAMAGE, "CDF step", 0);

FSCDFStep::FSCDFStep(FSModel* fem) : FSMaterialProp(FE_CDF_STEP, fem)
{
    AddDoubleParam(0, "mu" , "mu" ); //  mu must be > 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 <= Dmax <= 1)
}

//=============================================================================
// CDF Quintic
//=============================================================================

REGISTER_MATERIAL(FSCDFQuintic, MODULE_MECH, FE_CDF_QUINTIC, FE_MAT_DAMAGE, "CDF quintic", 0);

FSCDFQuintic::FSCDFQuintic(FSModel* fem) : FSMaterialProp(FE_CDF_QUINTIC, fem)
{
    AddDoubleParam(0, "mumin" , "mumin"); // mumin must be > 0
    AddDoubleParam(0, "mumax" , "mumax"); // mumax must be > mumin
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax" ); // Maximum allowable damage (0 <= Dmax <= 1)
}

//=============================================================================
// CDF Power
//=============================================================================

REGISTER_MATERIAL(FSCDFPower, MODULE_MECH, FE_CDF_POWER, FE_MAT_DAMAGE, "CDF power", 0);

FSCDFPower::FSCDFPower(FSModel* fem) : FSMaterialProp(FE_CDF_POWER, fem)
{
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent alpha");
    AddScienceParam(1, UNIT_NONE, "mu0"   , "constant mu0");
    AddScienceParam(0, UNIT_NONE, "mu1"   , "power coefficient mu1");
    AddScienceParam(1, UNIT_NONE, "scale" , "scale factor for argument");
}

//=============================================================================
// DC Simo
//=============================================================================

REGISTER_MATERIAL(FSDCSimo, MODULE_MECH, FE_DC_SIMO, FE_MAT_DAMAGE_CRITERION, "DC Simo", 0);

FSDCSimo::FSDCSimo(FSModel* fem) : FSMaterialProp(FE_DC_SIMO, fem)
{
}

//=============================================================================
// DC Strain Energy Density
//=============================================================================

REGISTER_MATERIAL(FSDCStrainEnergyDensity, MODULE_MECH, FE_DC_SED, FE_MAT_DAMAGE_CRITERION, "DC strain energy density", 0);

FSDCStrainEnergyDensity::FSDCStrainEnergyDensity(FSModel* fem) : FSMaterialProp(FE_DC_SED, fem)
{
}

//=============================================================================
// DC Specific Strain Energy
//=============================================================================

REGISTER_MATERIAL(FSDCSpecificStrainEnergy, MODULE_MECH, FE_DC_SSE, FE_MAT_DAMAGE_CRITERION, "DC specific strain energy", 0);

FSDCSpecificStrainEnergy::FSDCSpecificStrainEnergy(FSModel* fem) : FSMaterialProp(FE_DC_SSE, fem)
{
}

//=============================================================================
// DC von Mises Stress
//=============================================================================

REGISTER_MATERIAL(FSDCvonMisesStress, MODULE_MECH, FE_DC_VMS, FE_MAT_DAMAGE_CRITERION, "DC von Mises stress", 0);

FSDCvonMisesStress::FSDCvonMisesStress(FSModel* fem) : FSMaterialProp(FE_DC_VMS, fem)
{
}

//=============================================================================
// DC Drucker Shear Stress
//=============================================================================

REGISTER_MATERIAL(FSDCDruckerShearStress, MODULE_MECH, FE_DC_DRUCKER, FE_MAT_DAMAGE_CRITERION, "DC Drucker shear stress", 0);

FSDCDruckerShearStress::FSDCDruckerShearStress(FSModel* fem) : FSMaterialProp(FE_DC_DRUCKER, fem)
{
    AddScienceParam(1, UNIT_NONE, "c" , "c" ); // Maximum allowable damage (0 <= Dmax <= 1)
}

//=============================================================================
// DC Maximum Shear Stress
//=============================================================================

REGISTER_MATERIAL(FSDCMaxShearStress, MODULE_MECH, FE_DC_MSS, FE_MAT_DAMAGE_CRITERION, "DC max shear stress", 0);

FSDCMaxShearStress::FSDCMaxShearStress(FSModel* fem) : FSMaterialProp(FE_DC_MSS, fem)
{
}

//=============================================================================
// DC Maximum Normal Stress
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalStress, MODULE_MECH, FE_DC_MNS, FE_MAT_DAMAGE_CRITERION, "DC max normal stress", 0);

FSDCMaxNormalStress::FSDCMaxNormalStress(FSModel* fem) : FSMaterialProp(FE_DC_MNS, fem)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalLagrangeStrain, MODULE_MECH, FE_DC_MNLE, FE_MAT_DAMAGE_CRITERION, "DC max normal Lagrange strain", 0);

FSDCMaxNormalLagrangeStrain::FSDCMaxNormalLagrangeStrain(FSModel* fem) : FSMaterialProp(FE_DC_MNLE, fem)
{
}

//=============================================================================
// DC Octahedral Shear Strain
//=============================================================================

REGISTER_MATERIAL(FSDCOctahedralShearStrain, MODULE_MECH, FE_DC_OSS, FE_MAT_DAMAGE_CRITERION, "DC octahedral shear strain", 0);

FSDCOctahedralShearStrain::FSDCOctahedralShearStrain(FSModel* fem) : FSMaterialProp(FE_DC_OSS, fem)
{
}

//=============================================================================
// DC Simo Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCSimoUC, MODULE_MECH, FE_DC_SIMO_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC Simo uncoupled", 0);

FSDCSimoUC::FSDCSimoUC(FSModel* fem) : FSMaterialProp(FE_DC_SIMO_UC, fem)
{
}

//=============================================================================
// DC Strain Energy Density Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCStrainEnergyDensityUC, MODULE_MECH, FE_DC_SED_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC strain energy density uncoupled", 0);

FSDCStrainEnergyDensityUC::FSDCStrainEnergyDensityUC(FSModel* fem) : FSMaterialProp(FE_DC_SED_UC, fem)
{
}

//=============================================================================
// DC Specific Strain Energy Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCSpecificStrainEnergyUC, MODULE_MECH, FE_DC_SSE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC specific strain energy uncoupled", 0);

FSDCSpecificStrainEnergyUC::FSDCSpecificStrainEnergyUC(FSModel* fem) : FSMaterialProp(FE_DC_SSE_UC, fem)
{
}

//=============================================================================
// DC von Mises Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCvonMisesStressUC, MODULE_MECH, FE_DC_VMS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC von Mises stress uncoupled", 0);

FSDCvonMisesStressUC::FSDCvonMisesStressUC(FSModel* fem) : FSMaterialProp(FE_DC_VMS_UC, fem)
{
}

//=============================================================================
// DC Maximum Shear Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxShearStressUC, MODULE_MECH, FE_DC_MSS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max shear stress uncoupled", 0);

FSDCMaxShearStressUC::FSDCMaxShearStressUC(FSModel* fem) : FSMaterialProp(FE_DC_MSS_UC, fem)
{
}

//=============================================================================
// DC Maximum Normal Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalStressUC, MODULE_MECH, FE_DC_MNS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal stress uncoupled", 0);

FSDCMaxNormalStressUC::FSDCMaxNormalStressUC(FSModel* fem) : FSMaterialProp(FE_DC_MNS_UC, fem)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalLagrangeStrainUC, MODULE_MECH, FE_DC_MNLE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal Lagrange strain uncoupled", 0);

FSDCMaxNormalLagrangeStrainUC::FSDCMaxNormalLagrangeStrainUC(FSModel* fem) : FSMaterialProp(FE_DC_MNLE_UC, fem)
{
}

//=============================================================================
// Relaxation Exponential with Continuous Spectrum
//=============================================================================

REGISTER_MATERIAL(FSRelaxCSExp, MODULE_MECH, FE_RELAX_CSEXP, FE_MAT_RV_RELAX, "relaxation-CSexp", 0);

FSRelaxCSExp::FSRelaxCSExp(FSModel* fem) : FSMaterialProp(FE_RELAX_CSEXP, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "exponential spectrum constant"); // characteristic relaxation time
}

//=============================================================================
// Relaxation Exponential with Continuous Spectrum
//=============================================================================

REGISTER_MATERIAL(FSRelaxCSExpDistUser, MODULE_MECH, FE_RELAX_CSEXP_DIST_USER, FE_MAT_RV_RELAX, "relaxation-CSexp-dist-user", 0);

FSRelaxCSExpDistUser::FSRelaxCSExpDistUser(FSModel* fem) : FSMaterialProp(FE_RELAX_CSEXP_DIST_USER, fem)
{
	AddProperty("tau", FE_MAT_1DFUNC);
}

//=============================================================================
// Relaxation Exponential
//=============================================================================

REGISTER_MATERIAL(FSRelaxExp, MODULE_MECH, FE_RELAX_EXP, FE_MAT_RV_RELAX, "relaxation-exponential", 0);

FSRelaxExp::FSRelaxExp(FSModel* fem) : FSMaterialProp(FE_RELAX_EXP, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "time constant"); // characteristic relaxation time
}

//=============================================================================
// Relaxation Exponential Distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxExpDistortion, MODULE_MECH, FE_RELAX_EXP_DIST, FE_MAT_RV_RELAX, "relaxation-exp-distortion", 0);

FSRelaxExpDistortion::FSRelaxExpDistortion(FSModel* fem) : FSMaterialProp(FE_RELAX_EXP_DIST, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient" );
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent");
}

//=============================================================================
// Relaxation Exponential
//=============================================================================

REGISTER_MATERIAL(FSRelaxExpDistUser, MODULE_MECH, FE_RELAX_EXP_DIST_USER, FE_MAT_RV_RELAX, "relaxation-exp-dist-user", 0);

FSRelaxExpDistUser::FSRelaxExpDistUser(FSModel* fem) : FSMaterialProp(FE_RELAX_EXP_DIST_USER, fem)
{
	AddProperty("tau", FE_MAT_1DFUNC);
}

//=============================================================================
// Relaxation Fung
//=============================================================================

REGISTER_MATERIAL(FSRelaxFung, MODULE_MECH, FE_RELAX_FUNG, FE_MAT_RV_RELAX, "relaxation-Fung", 0);

FSRelaxFung::FSRelaxFung(FSModel* fem) : FSMaterialProp(FE_RELAX_FUNG, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau1"   , "min. relaxation time"); //  minimum characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau2"   , "max. relaxation time"); // maximum characteristic relaxation time
}

//=============================================================================
// Relaxation Malkin
//=============================================================================

REGISTER_MATERIAL(FSRelaxMalkin, MODULE_MECH, FE_RELAX_MALKIN, FE_MAT_RV_RELAX, "relaxation-Malkin", 0);

FSRelaxMalkin::FSRelaxMalkin(FSModel* fem) : FSMaterialProp(FE_RELAX_MALKIN, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau1"   , "min. relaxation time"); //  minimum characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau2"   , "max. relaxation time"); // maximum characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"   , "power exponent"); // exponent
}

//=============================================================================
// Relaxation Malkin Distributin User
//=============================================================================

REGISTER_MATERIAL(FSRelaxMalkinDistUser, MODULE_MECH, FE_RELAX_MALKIN_DIST_USER, FE_MAT_RV_RELAX, "relaxation-Malkin-dist-user", 0);

FSRelaxMalkinDistUser::FSRelaxMalkinDistUser(FSModel* fem) : FSMaterialProp(FE_RELAX_MALKIN_DIST_USER, fem)
{
	AddProperty("tau1", FE_MAT_1DFUNC);
	AddProperty("tau2", FE_MAT_1DFUNC);
	AddProperty("beta", FE_MAT_1DFUNC);
}

//=============================================================================
// Relaxation Malkin Distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxMalkinDistortion, MODULE_MECH, FE_RELAX_MALKIN_DIST, FE_MAT_RV_RELAX, "relaxation-Malkin-distortion", 0);

FSRelaxMalkinDistortion::FSRelaxMalkinDistortion(FSModel* fem) : FSMaterialProp(FE_RELAX_MALKIN_DIST, fem)
{
    AddScienceParam(0, UNIT_TIME, "t1c0"   , "constant for tau1");
    AddScienceParam(0, UNIT_TIME, "t1c1"   , "coefficient for tau1");
    AddScienceParam(0, UNIT_TIME, "t1s0"   , "strain for tau1");
    AddScienceParam(0, UNIT_TIME, "t2c0"   , "constant for tau2");
    AddScienceParam(0, UNIT_TIME, "t2c1"   , "coefficient for tau2");
    AddScienceParam(0, UNIT_TIME, "t2s0"   , "strain for tau2");
    AddScienceParam(0, UNIT_NONE, "beta"   , "power exponent beta"); // exponent
}

//=============================================================================
// Relaxation Park
//=============================================================================

REGISTER_MATERIAL(FSRelaxPark, MODULE_MECH, FE_RELAX_PARK, FE_MAT_RV_RELAX, "relaxation-Park", 0);

FSRelaxPark::FSRelaxPark(FSModel* fem) : FSMaterialProp(FE_RELAX_PARK, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "tau" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "beta"); // exponent
}

//=============================================================================
// Relaxation Park Distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxParkDistortion, MODULE_MECH, FE_RELAX_PARK_DIST, FE_MAT_RV_RELAX, "relaxation-Park-distortion", 0);

FSRelaxParkDistortion::FSRelaxParkDistortion(FSModel* fem) : FSMaterialProp(FE_RELAX_PARK_DIST, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient tau0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient tau1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "constant coefficient"); // exponent
    AddScienceParam(0, UNIT_NONE, "beta1" , "power coefficient beta1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent alpha");
}

//=============================================================================
// Relaxation power
//=============================================================================

REGISTER_MATERIAL(FSRelaxPow, MODULE_MECH, FE_RELAX_POW, FE_MAT_RV_RELAX, "relaxation-power", 0);

FSRelaxPow::FSRelaxPow(FSModel* fem) : FSMaterialProp(FE_RELAX_POW, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "characteristic time constant" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "power exponent"); // exponent
}

//=============================================================================
// Relaxation power distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxPowDistortion, MODULE_MECH, FE_RELAX_POW_DIST, FE_MAT_RV_RELAX, "relaxation-power-distortion", 0);

FSRelaxPowDistortion::FSRelaxPowDistortion(FSModel* fem) : FSMaterialProp(FE_RELAX_POW_DIST, fem)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient tau0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient tau1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "constant coefficient beta0"); // exponent
    AddScienceParam(0, UNIT_NONE, "beta1" , "power coefficient beta1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent alpha");
}

//=============================================================================
// Relaxation power distortion user
//=============================================================================

REGISTER_MATERIAL(FSRelaxPowDistUser, MODULE_MECH, FE_RELAX_POW_DIST_USER, FE_MAT_RV_RELAX, "relaxation-power-dist-user", 0);

FSRelaxPowDistUser::FSRelaxPowDistUser(FSModel* fem) : FSMaterialProp(FE_RELAX_POW_DIST_USER, fem)
{
	AddProperty("tau", FE_MAT_1DFUNC);
	AddProperty("beta", FE_MAT_1DFUNC);
}

//=============================================================================
// Relaxation Prony
//=============================================================================

REGISTER_MATERIAL(FSRelaxProny, MODULE_MECH, FE_RELAX_PRONY, FE_MAT_RV_RELAX, "relaxation-Prony", 0);

FSRelaxProny::FSRelaxProny(FSModel* fem) : FSMaterialProp(FE_RELAX_PRONY, fem)
{
    AddScienceParam(0, UNIT_NONE, "g1", "coeffient g1");
    AddScienceParam(0, UNIT_NONE, "g2", "coeffient g2");
    AddScienceParam(0, UNIT_NONE, "g3", "coeffient g3");
    AddScienceParam(0, UNIT_NONE, "g4", "coeffient g4");
    AddScienceParam(0, UNIT_NONE, "g5", "coeffient g5");
    AddScienceParam(0, UNIT_NONE, "g6", "coeffient g6");
    
    AddScienceParam(1, UNIT_TIME, "t1", "relaxation time t1");
    AddScienceParam(1, UNIT_TIME, "t2", "relaxation time t2");
    AddScienceParam(1, UNIT_TIME, "t3", "relaxation time t3");
    AddScienceParam(1, UNIT_TIME, "t4", "relaxation time t4");
    AddScienceParam(1, UNIT_TIME, "t5", "relaxation time t5");
    AddScienceParam(1, UNIT_TIME, "t6", "relaxation time t6");
}

//=============================================================================
// Elastic pressure for ideal gas
//=============================================================================

REGISTER_MATERIAL(FSEPIdealGas, MODULE_FLUID, FE_EP_IDEAL_GAS, FE_MAT_FLUID_ELASTIC, "ideal gas", 0);

FSEPIdealGas::FSEPIdealGas(FSModel* fem) : FSMaterialProp(FE_EP_IDEAL_GAS, fem)
{
    AddScienceParam(0, UNIT_MOLAR_MASS, "molar_mass"  , "molar_mass");
}

//=============================================================================
// Elastic pressure for ideal fluid
//=============================================================================

REGISTER_MATERIAL(FSEPIdealFluid, MODULE_FLUID, FE_EP_IDEAL_FLUID, FE_MAT_FLUID_ELASTIC, "ideal fluid", 0);

FSEPIdealFluid::FSEPIdealFluid(FSModel* fem) : FSMaterialProp(FE_EP_IDEAL_FLUID, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Elastic pressure for neo-Hookean fluid
//=============================================================================

REGISTER_MATERIAL(FSEPNeoHookeanFluid, MODULE_FLUID, FE_EP_NEOHOOKEAN_FLUID, FE_MAT_FLUID_ELASTIC, "neo-Hookean fluid", 0);

FSEPNeoHookeanFluid::FSEPNeoHookeanFluid(FSModel* fem) : FSMaterialProp(FE_EP_NEOHOOKEAN_FLUID, fem)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Viscous Newtonian fluid
//=============================================================================

REGISTER_MATERIAL(FSVFNewtonian, MODULE_FLUID, FE_VF_NEWTONIAN, FE_MAT_FLUID_VISCOSITY, "Newtonian fluid", 0);

FSVFNewtonian::FSVFNewtonian(FSModel* fem) : FSMaterialProp(FE_VF_NEWTONIAN, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity");
    AddScienceParam(0, UNIT_VISCOSITY, "kappa", "bulk viscosity");
}

//=============================================================================
// Viscous Bingham fluid
//=============================================================================

REGISTER_MATERIAL(FSVFBingham, MODULE_FLUID, FE_VF_BINGHAM, FE_MAT_FLUID_VISCOSITY, "Bingham", 0);

FSVFBingham::FSVFBingham(FSModel* fem) : FSMaterialProp(FE_VF_BINGHAM, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity"); // viscosity at infinite shear rate
    AddScienceParam(0, UNIT_PRESSURE , "tauy", "yield stress"   );
    AddScienceParam(0, UNIT_NONE     , "n"   , "exponent"       );
}

//=============================================================================
// Viscous Carreau fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCarreau, MODULE_FLUID, FE_VF_CARREAU, FE_MAT_FLUID_VISCOSITY, "Carreau", 0);

FSVFCarreau::FSVFCarreau(FSModel* fem) : FSMaterialProp(FE_VF_CARREAU, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity"); // viscosity at zero shear rate
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity"); // viscosity at infinite shear rate
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index n"  );
}

//=============================================================================
// Viscous Carreau-Yasuda fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCarreauYasuda, MODULE_FLUID, FE_VF_CARREAU_YASUDA, FE_MAT_FLUID_VISCOSITY, "Carreau-Yasuda", 0);

FSVFCarreauYasuda::FSVFCarreauYasuda(FSModel* fem) : FSMaterialProp(FE_VF_CARREAU_YASUDA, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index n"  );
    AddScienceParam(0, UNIT_NONE, "a" , "power denominator a"  );
}

//=============================================================================
// Viscous Powell-Eyring fluid
//=============================================================================

REGISTER_MATERIAL(FSVFPowellEyring, MODULE_FLUID, FE_VF_POWELL_EYRING, FE_MAT_FLUID_VISCOSITY, "Powell-Eyring", 0);

FSVFPowellEyring::FSVFPowellEyring(FSModel* fem) : FSMaterialProp(FE_VF_POWELL_EYRING, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
}

//=============================================================================
// Viscous Cross fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCross, MODULE_FLUID, FE_VF_CROSS, FE_MAT_FLUID_VISCOSITY, "Cross", 0);

FSVFCross::FSVFCross(FSModel* fem) : FSMaterialProp(FE_VF_CROSS, fem)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "m" , "power"  );
}

//=============================================================================
// Linear Polar Viscous fluid
//=============================================================================

REGISTER_MATERIAL(FSVPFLinear, MODULE_POLAR_FLUID, FE_PVF_LINEAR, FE_MAT_POLAR_FLUID_VISCOSITY, "polar linear", 0);

FSVPFLinear::FSVPFLinear(FSModel* fem) : FSMaterialProp(FE_PVF_LINEAR, fem)
{
    AddScienceParam(0, UNIT_ROTATIONAL_VISCOSITY, "tau"  , "polar rotational viscosity");
    AddScienceParam(0, UNIT_COUPLE_VISCOSITY, "alpha", "couple bulk viscosity");
    AddScienceParam(0, UNIT_COUPLE_VISCOSITY, "beta" , "couple viscosity-sym");
    AddScienceParam(0, UNIT_COUPLE_VISCOSITY, "gamma", "couple viscosity-skw");
}

//=============================================================================
// Starling solvent supply
//=============================================================================

REGISTER_MATERIAL(FSStarlingSupply, MODULE_MULTIPHASIC, FE_STARLING_SUPPLY, FE_MAT_SOLVENT_SUPPLY, "Starling", 0);

FSStarlingSupply::FSStarlingSupply(FSModel* fem) : FSMaterialProp(FE_STARLING_SUPPLY, fem)
{
	AddScienceParam(0, UNIT_FILTRATION, "kp", "filtration coefficient");
	AddScienceParam(0, UNIT_PRESSURE, "pv", "external pressure");
}

//=============================================================================
// const prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FSPrestrainConstGradient, MODULE_MECH, FE_PRESTRAIN_CONST_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "prestrain gradient", 0);

FSPrestrainConstGradient::FSPrestrainConstGradient(FSModel* fem) : FSMaterialProp(FE_PRESTRAIN_CONST_GRADIENT, fem)
{
	mat3d F0; F0.unit();
	AddMat3dParam(F0, "F0", "prestrain gradient")->MakeVariable(true);
}

//=============================================================================
// in-situ stretch prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FSPrestrainInSituGradient, MODULE_MECH, FE_PRESTRAIN_INSITU_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "in-situ stretch", 0);

FSPrestrainInSituGradient::FSPrestrainInSituGradient(FSModel* fem) : FSMaterialProp(FE_PRESTRAIN_INSITU_GRADIENT, fem)
{
	AddScienceParam(1.0, UNIT_NONE, "stretch", "fiber stretch")->MakeVariable(true);
	AddBoolParam(false, "isochoric", "isochoric prestrain");
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurvePaper, MODULE_MECH, FE_MAT_PLASTIC_FLOW_PAPER, FE_MAT_PLASTIC_FLOW_RULE, "PFC paper", 0);

FSPlasticFlowCurvePaper::FSPlasticFlowCurvePaper(FSModel* fem) : FSMaterialProp(FE_MAT_PLASTIC_FLOW_PAPER, fem)
{
	AddDoubleParam(0, "Y0")->MakeVariable(true);
	AddDoubleParam(0, "Ymax");
	AddDoubleParam(1, "w0");
	AddDoubleParam(0, "we");
	AddIntParam(1, "nf");
	AddDoubleParam(0.9, "r");
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurveUser, MODULE_MECH, FE_MAT_PLASTIC_FLOW_USER, FE_MAT_PLASTIC_FLOW_RULE, "PFC user", 0);

FSPlasticFlowCurveUser::FSPlasticFlowCurveUser(FSModel* fem) : FSMaterialProp(FE_MAT_PLASTIC_FLOW_USER, fem)
{
	AddProperty("plastic_response", FE_MAT_1DFUNC);
	AddProperty(0, new FS1DPointFunction(fem));
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurveMath, MODULE_MECH, FE_MAT_PLASTIC_FLOW_MATH, FE_MAT_PLASTIC_FLOW_RULE, "PFC math", 0);

FSPlasticFlowCurveMath::FSPlasticFlowCurveMath(FSModel* fem) : FSMaterialProp(FE_MAT_PLASTIC_FLOW_MATH, fem)
{
	AddIntParam(1, "nf");
	AddDoubleParam(0, "e0");
	AddDoubleParam(1, "emax");
	AddStringParam("", "plastic_response", "plastic flow curve");
}


//=============================================================================
REGISTER_MATERIAL(FEBioMaterial, MODULE_MECH, FE_FEBIO_MATERIAL, FE_MAT_ELASTIC, "[febio]", 0);

FEBioMaterial::FEBioMaterial(FSModel* fem) : FSMaterial(FE_FEBIO_MATERIAL, fem)
{
}

FEBioMaterial::~FEBioMaterial()
{
//	delete m_febClass;
}

bool FEBioMaterial::IsRigid()
{
	return (strcmp(GetTypeString(), "rigid body") == 0);
}

bool FEBioMaterial::HasFibers()
{
	if (FindProperty("fiber"))
	{
		return true;
	}
	else return false;
}

// return a string for the material type
const char* FEBioMaterial::GetTypeString() const
{
	return FSObject::GetTypeString();
}

void FEBioMaterial::SetTypeString(const std::string& s)
{
	FSObject::SetTypeString(s);
}

vec3d FEBioMaterial::GetFiber(FEElementRef& el)
{
	vec3d v(0, 0, 0);

	FSProperty* pm = FindProperty("fiber");
	FSVec3dValuator* fiber = dynamic_cast<FSVec3dValuator*>(pm->GetComponent());
	if (fiber)
	{
		// evaluate the fiber direction
		v = fiber->GetFiberVector(el);
	}
	return v;
}

bool FEBioMaterial::HasMaterialAxes() const
{
	if (FindProperty("mat_axis"))
	{
		return true;
	}
	else return false;
}

mat3d FEBioMaterial::GetMatAxes(FEElementRef& el) const
{
	mat3d Q = mat3d::identity();

	const FSProperty* pm = FindProperty("mat_axis");
	if (pm)
	{
		const FSMat3dValuator* matAxis = dynamic_cast<const FSMat3dValuator*>(pm->GetComponent());
		if (matAxis)
		{
			// evaluate the fiber direction
			Q = matAxis->GetMatAxis(el);
		}
	}

	return Q;
}

void FEBioMaterial::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSMaterial::Save(ar);
	}
	ar.EndChunk();
}

void FEBioMaterial::Load(IArchive& ar)
{
	TRACE("FEBioMaterial::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
		case CID_FEBIO_BASE_DATA: FSMaterial::Load(ar); break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}
	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

bool FEBioMaterial::UpdateData(bool bsave)
{
	if (bsave) FEBio::UpdateFEBioClass(this);
	return false;
}

FSMaterial* FEBioMaterial::Clone()
{
	return dynamic_cast<FSMaterial*>(FEBio::CloneModelComponent(this, GetFSModel()));
}
