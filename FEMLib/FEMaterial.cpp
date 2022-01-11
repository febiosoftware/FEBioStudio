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
#include <MeshTools/FEProject.h>
#include <FECore/units.h>
#include <FEBioStudio/WebDefines.h>
#include "FEDiscreteMaterial.h"
#include <FEBioLink/FEBioClass.h>
#include <FEBioLink/FEBioInterface.h>

//////////////////////////////////////////////////////////////////////
// FEFiberGeneratorLocal
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFiberGeneratorLocal, MODULE_MECH, FE_FIBER_GENERATOR_LOCAL, FE_MAT_FIBER_GENERATOR, "local", 0);

FSFiberGeneratorLocal::FSFiberGeneratorLocal(int n0, int n1) : FSFiberGenerator(FE_FIBER_GENERATOR_LOCAL)
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

FSFiberGeneratorVector::FSFiberGeneratorVector(const vec3d& v) : FSFiberGenerator(FE_FIBER_GENERATOR_VECTOR)
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

FSCylindricalVectorGenerator::FSCylindricalVectorGenerator() : FSFiberGenerator(FE_FIBER_GENERATOR_CYLINDRICAL)
{
	AddVecParam(vec3d(0, 0, 0), "center", "center");
	AddVecParam(vec3d(0, 0, 1), "axis"  , "axis"  );
	AddVecParam(vec3d(1, 0, 0), "vector", "vector");
}

FSCylindricalVectorGenerator::FSCylindricalVectorGenerator(const vec3d& center, const vec3d& axis, const vec3d& vector) : FSFiberGenerator(FE_FIBER_GENERATOR_CYLINDRICAL)
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

FSSphericalVectorGenerator::FSSphericalVectorGenerator() : FSFiberGenerator(FE_FIBER_GENERATOR_SPHERICAL)
{
	AddVecParam(vec3d(0, 0, 0), "center", "center");
	AddVecParam(vec3d(1, 0, 0), "vector", "vector");
}

FSSphericalVectorGenerator::FSSphericalVectorGenerator(const vec3d& center, const vec3d& vector) : FSFiberGenerator(FE_FIBER_GENERATOR_SPHERICAL)
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

FSAnglesVectorGenerator::FSAnglesVectorGenerator(double theta, double phi) : FSFiberGenerator(FE_FIBER_GENERATOR_ANGLES)
{
	AddScienceParam(theta, UNIT_DEGREE, "theta", "θ");
	AddScienceParam(phi, UNIT_DEGREE, "phi", "φ");
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

REGISTER_MATERIAL(FSIsotropicElastic, MODULE_MECH, FE_ISOTROPIC_ELASTIC, FE_MAT_ELASTIC, "isotropic elastic", MaterialFlags::TOPLEVEL, ISOTROPIC_ELASTIC_HTML);

FSIsotropicElastic::FSIsotropicElastic() : FSMaterial(FE_ISOTROPIC_ELASTIC)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus E");
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio ν");
}

//////////////////////////////////////////////////////////////////////
// FSOrthoElastic - orthotropic elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOrthoElastic, MODULE_MECH, FE_ORTHO_ELASTIC, FE_MAT_ELASTIC, "orthotropic elastic", MaterialFlags::TOPLEVEL, ORTHOTROPIC_ELASTIC_HTML);

FSOrthoElastic::FSOrthoElastic() : FSMaterial(FE_ORTHO_ELASTIC)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"    )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E1", "E1 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E2", "E2 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E3", "E3 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G12", "G12 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G23", "G23 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G31", "G31 shear modulus");
	AddScienceParam(0, UNIT_NONE   ,       "v12", "Poisson's ratio ν12");
	AddScienceParam(0, UNIT_NONE   ,       "v23", "Poisson's ratio ν23");
	AddScienceParam(0, UNIT_NONE   ,       "v31", "Poisson's ratio ν31");

	SetAxisMaterial(new FSAxisMaterial);
}

//////////////////////////////////////////////////////////////////////
// FSNeoHookean - neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNeoHookean, MODULE_MECH, FE_NEO_HOOKEAN, FE_MAT_ELASTIC, "neo-Hookean", MaterialFlags::TOPLEVEL, NEO_HOOKEAN_HTML);

FSNeoHookean::FSNeoHookean() : FSMaterial(FE_NEO_HOOKEAN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus E")->MakeVariable(true);
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio ν")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FENaturalNeoHookean - natural neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNaturalNeoHookean, MODULE_MECH, FE_NATURAL_NEO_HOOKEAN, FE_MAT_ELASTIC, "natural neo-Hookean", MaterialFlags::TOPLEVEL);

FSNaturalNeoHookean::FSNaturalNeoHookean() : FSMaterial(FE_NATURAL_NEO_HOOKEAN)
{
    AddScienceParam(1, UNIT_DENSITY , "density", "density"        )->MakeVariable(true)->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE,       "E", "Young's modulus E")->MakeVariable(true);
    AddScienceParam(0, UNIT_NONE    ,       "v", "Poisson's ratio ν")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FSIncompNeoHookean - incompressible neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSIncompNeoHookean, MODULE_MECH, FE_INCOMP_NEO_HOOKEAN, FE_MAT_ELASTIC_UNCOUPLED, "incomp neo-Hookean", MaterialFlags::TOPLEVEL);

FSIncompNeoHookean::FSIncompNeoHookean() : FSMaterial(FE_INCOMP_NEO_HOOKEAN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "G", "shear modulus");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSPorousNeoHookean - porous neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPorousNeoHookean, MODULE_MECH, FE_POROUS_NEO_HOOKEAN, FE_MAT_ELASTIC, "porous neo-Hookean", MaterialFlags::TOPLEVEL, POROUS_NEO_HOOKEAN_HTML);

FSPorousNeoHookean::FSPorousNeoHookean() : FSMaterial(FE_POROUS_NEO_HOOKEAN)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus");
    AddScienceParam(1, UNIT_NONE   ,    "phi0", "solid volume fraction");
}

//////////////////////////////////////////////////////////////////////
// FSMooneyRivlin - Mooney-Rivlin rubber
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSMooneyRivlin, MODULE_MECH, FE_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin", MaterialFlags::TOPLEVEL, MOONEY_RIVLIN_HTML);

FSMooneyRivlin::FSMooneyRivlin() : FSMaterial(FE_MOONEY_RIVLIN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_PRESSURE , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSVerondaWestmann - Veronda-Westmann elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSVerondaWestmann, MODULE_MECH, FE_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "Veronda-Westmann", MaterialFlags::TOPLEVEL, VERONDA_WESTMANN_HTML);

FSVerondaWestmann::FSVerondaWestmann() : FSMaterial(FE_VERONDA_WESTMANN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_NONE   , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
}


//////////////////////////////////////////////////////////////////////
// FSCoupledMooneyRivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCoupledMooneyRivlin, MODULE_MECH, FE_COUPLED_MOONEY_RIVLIN, FE_MAT_ELASTIC, "coupled Mooney-Rivlin", MaterialFlags::TOPLEVEL, COUPLED_MOONEY_RIVLIN_HTML);

FSCoupledMooneyRivlin::FSCoupledMooneyRivlin() : FSMaterial(FE_COUPLED_MOONEY_RIVLIN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FSCoupledVerondaWestmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCoupledVerondaWestmann, MODULE_MECH, FE_COUPLED_VERONDA_WESTMANN, FE_MAT_ELASTIC, "coupled Veronda-Westmann", MaterialFlags::TOPLEVEL, COUPLED_VERONDA_WESTMANN_HTML);

FSCoupledVerondaWestmann::FSCoupledVerondaWestmann() : FSMaterial(FE_COUPLED_VERONDA_WESTMANN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_NONE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FSHolmesMow -Holmes-Mow elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolmesMow, MODULE_MECH, FE_HOLMES_MOW, FE_MAT_ELASTIC, "Holmes-Mow", MaterialFlags::TOPLEVEL, HOLMES_MOW_HTML);

FSHolmesMow::FSHolmesMow() : FSMaterial(FE_HOLMES_MOW)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E", "Young's modulus E");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio ν");
	AddScienceParam(0, UNIT_NONE   , "beta", "power exponent β");
}

//////////////////////////////////////////////////////////////////////
// FSArrudaBoyce - Arruda-Boyce elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSArrudaBoyce, MODULE_MECH, FE_ARRUDA_BOYCE, FE_MAT_ELASTIC_UNCOUPLED, "Arruda-Boyce", MaterialFlags::TOPLEVEL, ARRUDA_BOYCE_HTML);

FSArrudaBoyce::FSArrudaBoyce() : FSMaterial(FE_ARRUDA_BOYCE)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "mu", "Initial modulus");
	AddScienceParam(0, UNIT_NONE   , "N", "links");
	AddScienceParam(0, UNIT_PRESSURE , "k", "Bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSCarterHayes - Carter-Hayes elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCarterHayes, MODULE_MECH, FE_CARTER_HAYES, FE_MAT_ELASTIC, "Carter-Hayes", MaterialFlags::TOPLEVEL, CARTER_HAYES_HTML);

FSCarterHayes::FSCarterHayes() : FSMaterial(FE_CARTER_HAYES)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "true density");
	AddScienceParam(0, UNIT_PRESSURE , "E0", "E0");
	AddScienceParam(1, UNIT_DENSITY, "rho0", "ρ0");
	AddScienceParam(0, UNIT_NONE   , "gamma", "γ");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio ν");
	AddIntParam    (-1, "sbm", "sbm");
}

//////////////////////////////////////////////////////////////////////
// FSNewtonianViscousSolid - Newtonian viscous solid
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSNewtonianViscousSolid, MODULE_MECH, FE_NEWTONIAN_VISCOUS_SOLID, FE_MAT_ELASTIC, "Newtonian viscous solid",0);

FSNewtonianViscousSolid::FSNewtonianViscousSolid() : FSMaterial(FE_NEWTONIAN_VISCOUS_SOLID)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "true density");
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity μ");
    AddScienceParam(0, UNIT_VISCOSITY, "kappa", "bulk viscosity κ");
}

//////////////////////////////////////////////////////////////////////
// FSPRLig - Poission-Ratio Ligament
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPRLig, MODULE_MECH, FE_PRLIG, FE_MAT_ELASTIC, "PRLig", MaterialFlags::TOPLEVEL);

FSPRLig::FSPRLig() : FSMaterial(FE_PRLIG)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_NONE   , "c1"     , "c1");
	AddScienceParam(1, UNIT_NONE   , "c2"     , "c2");
	AddScienceParam(0, UNIT_NONE   , "v0"     , "v0");
	AddScienceParam(0, UNIT_NONE   , "m"      , "m" );
	AddScienceParam(0, UNIT_NONE   , "mu"     , "μ");
	AddScienceParam(0, UNIT_NONE   , "k"      , "k" )->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FSOldFiberMaterial - material for fibers
//////////////////////////////////////////////////////////////////////

FSOldFiberMaterial::FSOldFiberMaterial() : FSMaterial(0)
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

FSOldFiberMaterial::FSOldFiberMaterial(const FSOldFiberMaterial& m) : FSMaterial(0) {}
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

FSTransverselyIsotropic::FSTransverselyIsotropic(int ntype) : FSMaterial(ntype)
{
	m_pfiber = 0;
}

FSOldFiberMaterial* FSTransverselyIsotropic::GetFiberMaterial()
{
	return m_pfiber;
}

void FSTransverselyIsotropic::SetFiberMaterial(FSOldFiberMaterial* fiber)
{
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

FSTransMooneyRivlinOld::Fiber::Fiber()
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

FSTransMooneyRivlinOld::FSTransMooneyRivlinOld() : FSTransverselyIsotropic(FE_TRANS_MOONEY_RIVLIN_OLD)
{
	// define the fiber class
	SetFiberMaterial(new Fiber);

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

FSTransVerondaWestmannOld::Fiber::Fiber()
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

FSTransVerondaWestmannOld::FSTransVerondaWestmannOld() : FSTransverselyIsotropic(FE_TRANS_VERONDA_WESTMANN_OLD)
{
	// define the fiber class
	SetFiberMaterial(new Fiber);

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

FSActiveContraction::FSActiveContraction() : FSMaterial(FE_MAT_ACTIVE_CONTRACTION)
{
	AddDoubleParam(0, "ascl", "scale");
	AddDoubleParam(0, "ca0");
	AddDoubleParam(0, "beta");
	AddDoubleParam(0, "l0");
	AddDoubleParam(0, "refl");
}

//////////////////////////////////////////////////////////////////////
// FSTransMooneyRivlin - transversely isotropic mooney-rivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSTransMooneyRivlin, MODULE_MECH, FE_TRANS_ISO_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML);

FSTransMooneyRivlin::FSTransMooneyRivlin() : FSTransverselyIsotropic(FE_TRANS_ISO_MOONEY_RIVLIN)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSTransVerondaWestmann, MODULE_MECH, FE_TRANS_ISO_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Veronda-Westmann", MaterialFlags::TOPLEVEL, TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML);

FSTransVerondaWestmann::FSTransVerondaWestmann() : FSTransverselyIsotropic(FE_TRANS_ISO_VERONDA_WESTMANN)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSCoupledTransIsoVerondaWestmann, MODULE_MECH, FE_COUPLED_TRANS_ISO_VW, FE_MAT_ELASTIC, "coupled trans-iso Veronda-Westmann", MaterialFlags::TOPLEVEL, COUPLED_TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML);

FSCoupledTransIsoVerondaWestmann::FSCoupledTransIsoVerondaWestmann() : FSTransverselyIsotropic(FE_COUPLED_TRANS_ISO_VW)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

FSCoupledTransIsoMooneyRivlinOld::FSCoupledTransIsoMooneyRivlinOld() : FSMaterial(FE_COUPLED_TRANS_ISO_MR)
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

REGISTER_MATERIAL(FSCoupledTransIsoMooneyRivlin, MODULE_MECH, FE_COUPLED_TRANS_ISO_MR, FE_MAT_ELASTIC, "coupled trans-iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, COUPLED_TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML);

FSCoupledTransIsoMooneyRivlin::FSCoupledTransIsoMooneyRivlin() : FSTransverselyIsotropic(FE_COUPLED_TRANS_ISO_MR)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSMooneyRivlinVonMisesFibers, MODULE_MECH, FE_MAT_MR_VON_MISES_FIBERS, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin von Mises Fibers", MaterialFlags::TOPLEVEL, MOONEY_RIVLIN_VON_MISES_DISTRIBUTED_FIBERS_HTML);

FSMooneyRivlinVonMisesFibers::FSMooneyRivlinVonMisesFibers() : FSMaterial(FE_MAT_MR_VON_MISES_FIBERS)
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

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// FS2DTransIsoMooneyRivlin
//=============================================================================

REGISTER_MATERIAL(FS2DTransIsoMooneyRivlin, MODULE_MECH, FE_MAT_2D_TRANS_ISO_MR, FE_MAT_ELASTIC_UNCOUPLED, "2D trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML);

FS2DTransIsoMooneyRivlin::FS2DTransIsoMooneyRivlin() : FSTransverselyIsotropic(FE_MAT_2D_TRANS_ISO_MR)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSRigidMaterial, MODULE_MECH, FE_RIGID_MATERIAL, FE_MAT_RIGID, "Rigid body", MaterialFlags::TOPLEVEL, RIGID_BODY_HTML);

FSRigidMaterial::FSRigidMaterial() : FSMaterial(FE_RIGID_MATERIAL)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "E", "Young's modulus E");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio ν");
	AddBoolParam  (false, "Auto-COM", "Auto-COM");
	AddVecParam   (vec3d(0,0,0), "rc", "Center of mass");

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

REGISTER_MATERIAL(FSTCNonlinearOrthotropic, MODULE_MECH, FE_TCNL_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "TC nonlinear orthotropic", MaterialFlags::TOPLEVEL, TENSION_COMPRESSION_NONLINEAR_ORTHOTROPIC_HTML);

FSTCNonlinearOrthotropic::FSTCNonlinearOrthotropic() : FSMaterial(FE_TCNL_ORTHO)
{
	AddScienceParam(1, UNIT_DENSITY, "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi", "ξ")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFungOrthotropic, MODULE_MECH, FE_FUNG_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "Fung orthotropic", MaterialFlags::TOPLEVEL, FUNG_ORTHOTROPIC_HTML);

FSFungOrthotropic::FSFungOrthotropic() : FSMaterial(FE_FUNG_ORTHO)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "ν12");
	AddScienceParam(0, UNIT_NONE   , "v23", "ν23");
	AddScienceParam(0, UNIT_NONE   , "v31", "ν31");
	AddScienceParam(0, UNIT_PRESSURE , "c", "c");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSFungOrthoCompressible, MODULE_MECH, FE_FUNG_ORTHO_COUPLED, FE_MAT_ELASTIC, "Fung-ortho-compressible", MaterialFlags::TOPLEVEL, FUNG_ORTHOTROPIC_COMPRESSIBLE_HTML);

FSFungOrthoCompressible::FSFungOrthoCompressible() : FSMaterial(FE_FUNG_ORTHO_COUPLED)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "ν12");
	AddScienceParam(0, UNIT_NONE   , "v23", "ν23");
	AddScienceParam(0, UNIT_NONE   , "v31", "ν31");
	AddScienceParam(0, UNIT_PRESSURE , "c", "c");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSHolzapfelGasserOgden - HGO MODEL
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolzapfelGasserOgden, MODULE_MECH, FE_HOLZAPFEL_GASSER_OGDEN, FE_MAT_ELASTIC_UNCOUPLED, "Holzapfel-Gasser-Ogden", MaterialFlags::TOPLEVEL);

FSHolzapfelGasserOgden::FSHolzapfelGasserOgden() : FSMaterial(FE_HOLZAPFEL_GASSER_OGDEN)
{
    AddScienceParam(1, UNIT_DENSITY  , "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "c", "c");
    AddScienceParam(0, UNIT_PRESSURE , "k1", "k1");
    AddScienceParam(0, UNIT_NONE     , "k2", "k2");
    AddScienceParam(0, UNIT_NONE     , "kappa", "κ");
    AddScienceParam(0, UNIT_DEGREE   , "gamma", "γ");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus")->SetPersistent(false);
    
    SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSHolzapfelUnconstrained - HGO MODEL
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSHolzapfelUnconstrained, MODULE_MECH, FE_HOLZAPFEL_UNCONSTRAINED, FE_MAT_ELASTIC, "HGO unconstrained", MaterialFlags::TOPLEVEL);

FSHolzapfelUnconstrained::FSHolzapfelUnconstrained() : FSMaterial(FE_HOLZAPFEL_UNCONSTRAINED)
{
    AddScienceParam(1, UNIT_DENSITY  , "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "c", "c");
    AddScienceParam(0, UNIT_PRESSURE , "k1", "k1");
    AddScienceParam(0, UNIT_NONE     , "k2", "k2");
    AddScienceParam(0, UNIT_NONE     , "kappa", "κ");
    AddScienceParam(0, UNIT_DEGREE   , "gamma", "γ");
    AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
    
    SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSLinearOrthotropic - Linear Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSLinearOrthotropic, MODULE_MECH, FE_LINEAR_ORTHO, FE_MAT_ELASTIC, "orthotropic elastic", MaterialFlags::TOPLEVEL);

FSLinearOrthotropic::FSLinearOrthotropic() : FSMaterial(FE_LINEAR_ORTHO)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "ν12");
	AddScienceParam(0, UNIT_NONE   , "v23", "ν23");
	AddScienceParam(0, UNIT_NONE   , "v31", "ν31");

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSMuscleMaterial - Silvia Blemker's muscle material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSMuscleMaterial, MODULE_MECH, FE_MUSCLE_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "muscle material", MaterialFlags::TOPLEVEL, MUSCLE_MATERIAL_HTML);

FSMuscleMaterial::FSMuscleMaterial() : FSTransverselyIsotropic(FE_MUSCLE_MATERIAL)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSTendonMaterial, MODULE_MECH, FE_TENDON_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "tendon material", MaterialFlags::TOPLEVEL, TENDON_MATERIAL_HTML);

FSTendonMaterial::FSTendonMaterial() : FSTransverselyIsotropic(FE_TENDON_MATERIAL)
{
	SetFiberMaterial(new FSOldFiberMaterial);

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

REGISTER_MATERIAL(FSOgdenMaterial, MODULE_MECH, FE_OGDEN_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "Ogden", MaterialFlags::TOPLEVEL, OGDEN_HTML);

FSOgdenMaterial::FSOgdenMaterial() : FSMaterial(FE_OGDEN_MATERIAL)
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

REGISTER_MATERIAL(FSOgdenUnconstrained, MODULE_MECH, FE_OGDEN_UNCONSTRAINED, FE_MAT_ELASTIC, "Ogden unconstrained", MaterialFlags::TOPLEVEL, OGDEN_UNCONSTRAINED_HTML);

FSOgdenUnconstrained::FSOgdenUnconstrained() : FSMaterial(FE_OGDEN_UNCONSTRAINED)
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

REGISTER_MATERIAL(FSEFDMooneyRivlin, MODULE_MECH, FE_EFD_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Mooney-Rivlin", MaterialFlags::TOPLEVEL, ELLIPSOIDAL_FIBER_DISTRIBUTION_MOONEY_RIVLIN_HTML);

FSEFDMooneyRivlin::FSEFDMooneyRivlin() : FSMaterial(FE_EFD_MOONEY_RIVLIN)
{
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi", "ξ")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

//////////////////////////////////////////////////////////////////////
// FSEFDNeoHookean - ellipsoidal fiber distribution model with MR base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDNeoHookean, MODULE_MECH, FE_EFD_NEO_HOOKEAN, FE_MAT_ELASTIC, "EFD neo-Hookean", MaterialFlags::TOPLEVEL, ELLIPSOIDAL_FIBER_DISTRIBUTION_NEO_HOOKEAN_HTML);

FSEFDNeoHookean::FSEFDNeoHookean() : FSMaterial(FE_EFD_NEO_HOOKEAN)
{
	AddScienceParam(0, UNIT_PRESSURE, "E", "Young's modulus E");
	AddScienceParam(0, UNIT_NONE  , "v", "Poisson's ratio ν");
	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi", "ξ"  )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

//////////////////////////////////////////////////////////////////////
// FSEFDDonnan - ellipsoidal fiber distribution model with Donnan base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDDonnan, MODULE_MECH, FE_EFD_DONNAN, FE_MAT_ELASTIC, "EFD Donnan equilibrium", MaterialFlags::TOPLEVEL, ELLIPSOIDAL_FIBER_DISTRIBUTION_WITH_DONNAN_EQUILIBRIUM_SWELLING_HTML);

FSEFDDonnan::FSEFDDonnan() : FSMaterial(FE_EFD_DONNAN)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(1, UNIT_NONE, "Phi", "Phi");
	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi", "ξ")->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

/////////////////////////////////////////////////////////////////////////////////////////
// FSEFDVerondaWestmann - ellipsoidal fiber distribution model with Veronda Westmann base
/////////////////////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSEFDVerondaWestmann, MODULE_MECH, FE_EFD_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Veronda-Westmann", MaterialFlags::TOPLEVEL, ELLIPSOIDAL_FIBER_DISTRIBUTION_VERONDA_WESTMANN_HTML);

FSEFDVerondaWestmann::FSEFDVerondaWestmann() : FSMaterial(FE_EFD_VERONDA_WESTMANN)
{
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi", "ξ"  )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSCubicCLE - Conewise Linear Elasticity with cubic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSCubicCLE, MODULE_MECH, FE_CLE_CUBIC, FE_MAT_ELASTIC, "cubic CLE", MaterialFlags::TOPLEVEL, CUBIC_CLE_HTML);

FSCubicCLE::FSCubicCLE() : FSMaterial(FE_CLE_CUBIC)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "lp1", "λ+1");
    AddScienceParam(0, UNIT_PRESSURE , "lm1", "λ-1");
    AddScienceParam(0, UNIT_PRESSURE , "l2" , "λ2" );
    AddScienceParam(0, UNIT_PRESSURE , "mu" , "μ"  );

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSCubicCLE - Conewise Linear Elasticity with orthotropic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSOrthotropicCLE, MODULE_MECH, FE_CLE_ORTHOTROPIC, FE_MAT_ELASTIC, "orthotropic CLE", MaterialFlags::TOPLEVEL, ORTHOTROPIC_CLE_HTML);

FSOrthotropicCLE::FSOrthotropicCLE() : FSMaterial(FE_CLE_ORTHOTROPIC)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
    AddScienceParam(0, UNIT_PRESSURE , "lp11", "λ+11");
    AddScienceParam(0, UNIT_PRESSURE , "lp22", "λ+22");
    AddScienceParam(0, UNIT_PRESSURE , "lp33", "λ+33");
    AddScienceParam(0, UNIT_PRESSURE , "lm11", "λ-11");
    AddScienceParam(0, UNIT_PRESSURE , "lm22", "λ-22");
    AddScienceParam(0, UNIT_PRESSURE , "lm33", "λ-33");
    AddScienceParam(0, UNIT_PRESSURE , "l12" , "λ12" );
    AddScienceParam(0, UNIT_PRESSURE , "l23" , "λ23" );
    AddScienceParam(0, UNIT_PRESSURE , "l31" , "λ31" );
    AddScienceParam(0, UNIT_PRESSURE , "mu1" , "μ1"  );
    AddScienceParam(0, UNIT_PRESSURE , "mu2" , "μ2"  );
    AddScienceParam(0, UNIT_PRESSURE , "mu3" , "μ3"  );

	SetAxisMaterial(new FSAxisMaterial);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxial - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxial, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI, FE_MAT_ELASTIC, "prescribed uniaxial active contraction", 0, Prescribed_Uniaxial_Active_Contraction);

FSPrescribedActiveContractionUniaxialOld::FSPrescribedActiveContractionUniaxialOld() : FSMaterial(FE_ACTIVE_CONTRACT_UNI_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIso - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionTransIso, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO, FE_MAT_ELASTIC, "prescribed trans iso active contraction", 0, Prescribed_Transversely_Isotropic_Active_Contraction);

FSPrescribedActiveContractionTransIsoOld::FSPrescribedActiveContractionTransIsoOld() : FSMaterial(FE_ACTIVE_CONTRACT_TISO_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxial - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxial, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI, FE_MAT_ELASTIC, "prescribed uniaxial active contraction", 0, PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionUniaxial::FSPrescribedActiveContractionUniaxial() : FSMaterial(FE_ACTIVE_CONTRACT_UNI)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0")->MakeVariable(true);
	SetAxisMaterial(new FSAxisMaterial);
}

void FSPrescribedActiveContractionUniaxial::Convert(FSPrescribedActiveContractionUniaxialOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial);
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIso - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionTransIso, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO, FE_MAT_ELASTIC, "prescribed trans iso active contraction", 0, PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionTransIso::FSPrescribedActiveContractionTransIso() : FSMaterial(FE_ACTIVE_CONTRACT_TISO)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
	SetAxisMaterial(new FSAxisMaterial);
}

void FSPrescribedActiveContractionTransIso::Convert(FSPrescribedActiveContractionTransIsoOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial);
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionIsotropic - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionIsotropic, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO, FE_MAT_ELASTIC, "prescribed isotropic active contraction", 0, PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionIsotropic::FSPrescribedActiveContractionIsotropic() : FSMaterial(FE_ACTIVE_CONTRACT_ISO)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxialUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxialUC, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed uniaxial active contraction", 0, Uncoupled_Prescribed_Uniaxial_Active_Contraction);

FSPrescribedActiveContractionUniaxialUCOld::FSPrescribedActiveContractionUniaxialUCOld() : FSMaterial(FE_ACTIVE_CONTRACT_UNI_UC_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIsoUC - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FSPrescribedActiveContractionTransIsoUC, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed trans iso active contraction", 0, Uncoupled_Prescribed_Transversely_Isotropic_Active_Contraction);

FSPrescribedActiveContractionTransIsoUCOld::FSPrescribedActiveContractionTransIsoUCOld() : FSMaterial(FE_ACTIVE_CONTRACT_TISO_UC_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionUniaxialUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionUniaxialUC, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed uniaxial active contraction", 0, UNCOUPLED_PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionUniaxialUC::FSPrescribedActiveContractionUniaxialUC() : FSMaterial(FE_ACTIVE_CONTRACT_UNI_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
	SetAxisMaterial(new FSAxisMaterial);
}

void FSPrescribedActiveContractionUniaxialUC::Convert(FSPrescribedActiveContractionUniaxialUCOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial);
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionUniaxialUCOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionTransIsoUC - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionTransIsoUC, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed trans iso active contraction", 0, UNCOUPLED_PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionTransIsoUC::FSPrescribedActiveContractionTransIsoUC() : FSMaterial(FE_ACTIVE_CONTRACT_TISO_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
	SetAxisMaterial(new FSAxisMaterial);
}

void FSPrescribedActiveContractionTransIsoUC::Convert(FSPrescribedActiveContractionTransIsoUCOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_T0, pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_T0));
    
	SetAxisMaterial(new FSAxisMaterial);
	m_axes->m_naopt = FE_AXES_ANGLES;
    m_axes->m_theta = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_TH);
    m_axes->m_phi = pold->GetFloatValue(FSPrescribedActiveContractionTransIsoUCOld::MP_PH);
}

////////////////////////////////////////////////////////////////////////
// FSPrescribedActiveContractionIsotropicUC - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FSPrescribedActiveContractionIsotropicUC, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed isotropic active contraction", 0, UNCOUPLED_PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML);

FSPrescribedActiveContractionIsotropicUC::FSPrescribedActiveContractionIsotropicUC() : FSMaterial(FE_ACTIVE_CONTRACT_ISO_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

//////////////////////////////////////////////////////////////////////
REGISTER_MATERIAL(FSKamensky, MODULE_MECH, FE_KAMENSKY, FE_MAT_ELASTIC, "Kamensky", MaterialFlags::TOPLEVEL);

FSKamensky::FSKamensky() : FSMaterial(FE_KAMENSKY)
{
	AddScienceParam(1, UNIT_DENSITY , "density")->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c0");
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_NONE    , "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k");
	AddScienceParam(1, UNIT_NONE    , "tangent_scale");
}

//////////////////////////////////////////////////////////////////////
REGISTER_MATERIAL(FSKamenskyUncoupled, MODULE_MECH, FE_KAMENSKY_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "Kamensky uncoupled", MaterialFlags::TOPLEVEL);

FSKamenskyUncoupled::FSKamenskyUncoupled() : FSMaterial(FE_KAMENSKY_UNCOUPLED)
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

FSIsotropicFourier::FSIsotropicFourier() : FSMaterial(FE_ISOTROPIC_FOURIER)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, "W/L.T", "k"      , "heat conductivity");
	AddScienceParam(0, "E/T"  , "c"      , "heat capacity");
}

//=============================================================================
// Constant Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermConst, MODULE_BIPHASIC, FE_PERM_CONST, FE_MAT_PERMEABILITY, "perm-const-iso", 0, CONSTANT_ISOTROPIC_PERMEABILITY_HTML);

FSPermConst::FSPermConst() : FSMaterial(FE_PERM_CONST)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm", "permeability");
}

//=============================================================================
// Holmes-Mow Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermHolmesMow, MODULE_BIPHASIC, FE_PERM_HOLMES_MOW, FE_MAT_PERMEABILITY, "perm-Holmes-Mow", 0, HOLMES_MOW_HTML);

FSPermHolmesMow::FSPermHolmesMow() : FSMaterial(FE_PERM_HOLMES_MOW)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm" , "permeability");
	AddScienceParam(0, UNIT_NONE        , "M"    , "M");
	AddScienceParam(0, UNIT_NONE        , "alpha", "α");
}

//=============================================================================
// Ateshian-Weiss isotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissIso, MODULE_BIPHASIC, FE_PERM_REF_ISO, FE_MAT_PERMEABILITY, "perm-ref-iso", 0, REFERENTIALLY_ISOTROPIC_PERMEABILITY_HTML);

FSPermAteshianWeissIso::FSPermAteshianWeissIso() : FSMaterial(FE_PERM_REF_ISO)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0", "perm0");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1", "perm1");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2", "perm2");
	AddScienceParam(0, UNIT_NONE        , "M"    , "M");
	AddScienceParam(0, UNIT_NONE        , "alpha", "α");
}

//=============================================================================
// Ateshian-Weiss trans-isotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissTransIso, MODULE_BIPHASIC, FE_PERM_REF_TRANS_ISO, FE_MAT_PERMEABILITY, "perm-ref-trans-iso", 0, REFERENTIALLY_TRANSVERSELY_ISOTROPIC_PERMEABILITY_HTML);

FSPermAteshianWeissTransIso::FSPermAteshianWeissTransIso() : FSMaterial(FE_PERM_REF_TRANS_ISO)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0" , "perm0" );
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1T", "perm1T");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm1A", "perm1A");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2T", "perm2T");
	AddScienceParam(0, UNIT_PERMEABILITY, "perm2A", "perm2A");
	AddScienceParam(0, UNIT_NONE        , "M0"    , "M0"    );
	AddScienceParam(0, UNIT_NONE        , "MT"    , "MT"    );
	AddScienceParam(0, UNIT_NONE        , "MA"    , "MA"    );
	AddScienceParam(0, UNIT_NONE        , "alpha0", "α0");
	AddScienceParam(0, UNIT_NONE        , "alphaA", "αA");
	AddScienceParam(0, UNIT_NONE        , "alphaT", "αT");

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// Ateshian-Weiss orthotropic permeability
//=============================================================================

REGISTER_MATERIAL(FSPermAteshianWeissOrtho, MODULE_BIPHASIC, FE_PERM_REF_ORTHO, FE_MAT_PERMEABILITY, "perm-ref-ortho", 0, REFERENTIALLY_ORTHOTROPIC_PERMEABILITY_HTML);

FSPermAteshianWeissOrtho::FSPermAteshianWeissOrtho() : FSMaterial(FE_PERM_REF_ORTHO)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm0" , "perm0");
	AddVecParam(vec3d(0,0,0), "perm1" , "perm1")->SetUnit(UNIT_PERMEABILITY);
	AddVecParam(vec3d(0,0,0), "perm2" , "perm2")->SetUnit(UNIT_PERMEABILITY);
	AddScienceParam(0, UNIT_NONE        , "M0"    , "M0");
	AddScienceParam(0, UNIT_NONE        , "alpha0", "α0");
	AddVecParam(vec3d(0,0,0), "M"     , "M");
	AddVecParam(vec3d(0,0,0), "alpha" , "α");

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// Exponential Isotropic Permeability
//=============================================================================

REGISTER_MATERIAL(FSPermExpIso, MODULE_BIPHASIC, FE_PERM_EXP_ISO, FE_MAT_PERMEABILITY, "perm-exp-iso", 0, EXPONENTIAL_ISOTROPIC_PERMEABILITY_HTML);

FSPermExpIso::FSPermExpIso() : FSMaterial(FE_PERM_EXP_ISO)
{
    AddScienceParam(0, UNIT_PERMEABILITY, "perm" , "permeability");
    AddScienceParam(0, UNIT_NONE        , "M"    , "M");
}

//=============================================================================
// constant diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffConst, MODULE_BIPHASIC, FE_DIFF_CONST, FE_MAT_DIFFUSIVITY, "diff-const-iso", 0, CONSTANT_ISOTROPIC_DIFFUSIVITY_HTML);

FSDiffConst::FSDiffConst() : FSMaterial(FE_DIFF_CONST)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff"     , "diffusivity");
}

//=============================================================================
// orthotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffOrtho, MODULE_BIPHASIC, FE_DIFF_CONST_ORTHO, FE_MAT_DIFFUSIVITY, "diff-const-ortho", 0, CONSTANT_ORTHOTROPIC_DIFFUSIVITY_HTML);

FSDiffOrtho::FSDiffOrtho() : FSMaterial(FE_DIFF_CONST_ORTHO)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddVecParam(vec3d(0,0,0), "diff", "diffusivity")->SetUnit(UNIT_DIFFUSIVITY);
}

//=============================================================================
// Ateshian-Weiss isotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffAteshianWeissIso, MODULE_BIPHASIC, FE_DIFF_REF_ISO, FE_MAT_DIFFUSIVITY, "diff-ref-iso", 0, REFERENTIALLY_ISOTROPIC_DIFFUSIVITY_HTML);

FSDiffAteshianWeissIso::FSDiffAteshianWeissIso() : FSMaterial(FE_DIFF_REF_ISO)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff0"    , "diff0");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff1"    , "diff1");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff2"    , "diff2");
	AddScienceParam(0, UNIT_NONE       , "M"        , "M"    );
	AddScienceParam(0, UNIT_NONE       , "alpha"    , "α");
}

//=============================================================================
// Albro isotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FSDiffAlbroIso, MODULE_BIPHASIC, FE_DIFF_ALBRO_ISO, FE_MAT_DIFFUSIVITY, "diff-Albro-iso", 0, ALBRO_ISOTROPIC_DIFFUSIVITY_HTML);

FSDiffAlbroIso::FSDiffAlbroIso() : FSMaterial(FE_DIFF_ALBRO_ISO)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_NONE       , "cdinv"    , "cdinv");
	AddScienceParam(0, UNIT_NONE       , "alphad"   , "alphad");
}

//=============================================================================
// constant solubility
//=============================================================================

REGISTER_MATERIAL(FSSolubConst, MODULE_BIPHASIC, FE_SOLUB_CONST, FE_MAT_SOLUBILITY, "solub-const", 0, CONSTANT_SOLUBILITY_HTML);

FSSolubConst::FSSolubConst() : FSMaterial(FE_SOLUB_CONST)
{
	AddScienceParam(1, UNIT_NONE, "solub", "solubility");
}

//=============================================================================
// constant osmotic coefficient
//=============================================================================

REGISTER_MATERIAL(FSOsmoConst, MODULE_BIPHASIC, FE_OSMO_CONST, FE_MAT_OSMOTIC_COEFFICIENT, "osm-coef-const", 0, CONSTANT_OSMOTIC_COEFFICIENT_HTML);

FSOsmoConst::FSOsmoConst() : FSMaterial(FE_OSMO_CONST)
{
	AddScienceParam(1, UNIT_NONE, "osmcoef", "osmotic coefficient");
}

//=============================================================================
// Wells-Manning osmotic coefficient
//=============================================================================

REGISTER_MATERIAL(FSOsmoWellsManning, MODULE_BIPHASIC, FE_OSMO_WM, FE_MAT_OSMOTIC_COEFFICIENT, "osm-coef-Manning", 0);

FSOsmoWellsManning::FSOsmoWellsManning() : FSMaterial(FE_OSMO_WM)
{
    AddScienceParam(1, UNIT_NONE, "ksi", "ξ");
    AddChoiceParam(0, "co_ion", "co-ion")->SetEnumNames("$(Solutes)")->SetState(Param_EDITABLE | Param_PERSISTENT);
}

//=============================================================================
// SFD compressible
//=============================================================================

REGISTER_MATERIAL(FSSFDCoupled, MODULE_MECH, FE_SFD_COUPLED, FE_MAT_ELASTIC, "spherical fiber distribution", 0, SPHERICAL_FIBER_DISTRIBUTION_HTML);

FSSFDCoupled::FSSFDCoupled() : FSMaterial(FE_SFD_COUPLED)
{
	AddScienceParam(0, UNIT_NONE        , "alpha", "α");
	AddScienceParam(0, UNIT_NONE        , "beta", "β");
	AddScienceParam(0, UNIT_PRESSURE    , "ksi" , "ξ" );
}

//=============================================================================
// SFD SBM
//=============================================================================

REGISTER_MATERIAL(FSSFDSBM, MODULE_MECH, FE_SFD_SBM, FE_MAT_ELASTIC, "spherical fiber distribution sbm", 0, SPHERICAL_FIBER_DISTRIBUTION_FROM_SOLID_BOUND_MOLECULE_HTML);

FSSFDSBM::FSSFDSBM() : FSMaterial(FE_SFD_SBM)
{
	AddScienceParam(0, UNIT_NONE        , "alpha", "α" );
	AddScienceParam(0, UNIT_NONE        , "beta", "β"   );
	AddScienceParam(0, UNIT_NONE        , "ksi0" , "ξ0"  );
	AddScienceParam(1, UNIT_NONE        , "rho0" , "ρ0"  );
	AddScienceParam(0, UNIT_NONE        , "gamma" , "γ");
	AddIntParam    (-1                   , "sbm"   , "sbm"  );

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// EFD Coupled
//=============================================================================

REGISTER_MATERIAL(FSEFDCoupled, MODULE_MECH, FE_EFD_COUPLED, FE_MAT_ELASTIC, "ellipsoidal fiber distribution", 0, ELLIPSOIDAL_FIBER_DISTRIBUTION_HTML);

FSEFDCoupled::FSEFDCoupled() : FSMaterial(FE_EFD_COUPLED)
{
	AddVecParam(vec3d(0,0,0), "beta", "β");
	AddVecParam(vec3d(0,0,0), "ksi" , "ξ" )->SetUnit(UNIT_PRESSURE);

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// EFD Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSEFDUncoupled, MODULE_MECH, FE_EFD_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "EFD uncoupled", 0, ELLIPSOIDAL_FIBER_DISTRIBUTION_HTML);

FSEFDUncoupled::FSEFDUncoupled() : FSMaterial(FE_EFD_UNCOUPLED)
{
	AddVecParam(vec3d(0,0,0), "beta" , "β");
	AddVecParam(vec3d(0,0,0), "ksi" , "ξ")->SetUnit(UNIT_PRESSURE);
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);

	SetAxisMaterial(new FSAxisMaterial);
}

//=============================================================================
// FSFiberMaterial
//=============================================================================

FSFiberMaterial::FSFiberMaterial(int ntype) : FSMaterial(ntype)
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
	// If the fiber generator was not set we'll create a fiber generator from the mat axes
	FSFiberGenerator* v = dynamic_cast<FSFiberGenerator*>(GetProperty(0).GetComponent());
	if (v == nullptr)
	{
		switch (Q->m_naopt)
		{
		case FE_AXES_LOCAL:
			SetFiberGenerator(new FSFiberGeneratorLocal(Q->m_n[0], Q->m_n[1]));
			break;
		case FE_AXES_VECTOR:
			SetFiberGenerator(new FSFiberGeneratorVector(Q->m_a));
			break;
		case FE_AXES_ANGLES:
			SetFiberGenerator(new FSAnglesVectorGenerator(Q->m_theta, Q->m_phi));
			break;
		case FE_AXES_CYLINDRICAL:
			SetFiberGenerator(new FSCylindricalVectorGenerator(Q->m_center, Q->m_axis, Q->m_vec));
			break;
		case FE_AXES_SPHERICAL:
			SetFiberGenerator(new FSSphericalVectorGenerator(Q->m_center, Q->m_vec));
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
		mat3d Q = parentMat->m_axes->GetMatAxes(el);
		v = Q * v;
	}

	return v;
}

//=============================================================================
// Fiber-Exp-Pow
//=============================================================================

//REGISTER_MATERIAL(FSFiberExpPow, MODULE_MECH, FE_FIBEREXPPOW_COUPLED, FE_MAT_ELASTIC, "fiber-exp-pow", 0, Fiber_with_Exponential_Power_Law);

FSFiberExpPowOld::FSFiberExpPowOld() : FSMaterial(FE_FIBEREXPPOW_COUPLED_OLD)
{
	AddScienceParam(0, UNIT_NONE, "alpha", "α");
	AddScienceParam(0, UNIT_NONE, "beta" , "β" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial);
}

REGISTER_MATERIAL(FSFiberExpPow, MODULE_MECH, FE_FIBEREXPPOW_COUPLED, FE_MAT_ELASTIC, "fiber-exp-pow", 0, FIBER_WITH_EXPONENTIAL_POWER_LAW_HTML);

FSFiberExpPow::FSFiberExpPow() : FSFiberMaterial(FE_FIBEREXPPOW_COUPLED)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "α");
    AddScienceParam(0, UNIT_NONE, "beta" , "β" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
    AddScienceParam(1, UNIT_NONE, "lam0"  , "λ0");
}

void FSFiberExpPow::Convert(FSFiberExpPowOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_ALPHA, pold->GetFloatValue(FSFiberExpPowOld::MP_ALPHA));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberExpPowOld::MP_BETA ));
    SetFloatValue(MP_KSI  , pold->GetFloatValue(FSFiberExpPowOld::MP_KSI  ));

	double the = pold->GetFloatValue(FSFiberExpPowOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberExpPowOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(the, phi));
}

//=============================================================================
// Fiber-Exp-Linear
//=============================================================================

REGISTER_MATERIAL(FSFiberExpLinear, MODULE_MECH, FE_FIBEREXPLIN_COUPLED, FE_MAT_ELASTIC, "fiber-exp-linear", 0);

FSFiberExpLinear::FSFiberExpLinear() : FSFiberMaterial(FE_FIBEREXPLIN_COUPLED)
{
	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "λ");
}

//=============================================================================
// Fiber-Exp-Linear uncoupled
//=============================================================================

REGISTER_MATERIAL(FSFiberExpLinearUncoupled, MODULE_MECH, FE_FIBEREXPLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled fiber-exp-linear", 0);

FSFiberExpLinearUncoupled::FSFiberExpLinearUncoupled() : FSFiberMaterial(FE_FIBEREXPLIN_UNCOUPLED)
{
	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "λ");
}

//=============================================================================
// Fiber-Neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSFiberNeoHookean, MODULE_MECH, FE_FIBER_NEO_HOOKEAN, FE_MAT_ELASTIC, "fiber-NH", 0);

FSFiberNeoHookean::FSFiberNeoHookean() : FSFiberMaterial(FE_FIBER_NEO_HOOKEAN)
{
    AddDoubleParam(0.0, "mu", "μ");
}

//=============================================================================
// Fiber-Natural-Neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSFiberNaturalNH, MODULE_MECH, FE_FIBER_NATURAL_NH, FE_MAT_ELASTIC, "fiber-natural-NH", 0);

FSFiberNaturalNH::FSFiberNaturalNH() : FSFiberMaterial(FE_FIBER_NATURAL_NH)
{
    AddDoubleParam(0.0, "ksi", "ξ");
    AddDoubleParam(1.0, "lam0", "λ0");
}

//=============================================================================
// damage fiber power
//=============================================================================

REGISTER_MATERIAL(FSFiberDamagePower, MODULE_MECH, FE_FIBER_DAMAGE_POWER, FE_MAT_ELASTIC, "damage fiber power", 0);

FSFiberDamagePower::FSFiberDamagePower() : FSFiberMaterial(FE_FIBER_DAMAGE_POWER)
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

FSFiberDamageExponential::FSFiberDamageExponential() : FSFiberMaterial(FE_FIBER_DAMAGE_EXP)
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

FSFiberDamageExpLinear::FSFiberDamageExpLinear() : FSFiberMaterial(FE_FIBER_DAMAGE_EXPLINEAR)
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

FSFiberKiousisUncoupled::FSFiberKiousisUncoupled() : FSFiberMaterial(FE_FIBER_KIOUSIS_UNCOUPLED)
{
    AddDoubleParam(0.0, "d1", "d1");
    AddDoubleParam(1.0, "d2", "d2");
    AddDoubleParam(2.0, "n", "n");
}

//=============================================================================
// Fiber-Exp-Pow Uncoupled
//=============================================================================

//REGISTER_MATERIAL(FSFiberExpPowUncoupled, MODULE_MECH, FE_FIBEREXPPOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-exp-pow-uncoupled", 0, Fiber_with_Exponential_Power_Law_Uncoupled_Formulation);

FSFiberExpPowUncoupledOld::FSFiberExpPowUncoupledOld() : FSMaterial(FE_FIBEREXPPOW_UNCOUPLED_OLD)
{
	AddScienceParam(0, UNIT_NONE, "alpha", "α");
	AddScienceParam(0, UNIT_NONE, "beta" , "β" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial);
}

REGISTER_MATERIAL(FSFiberExpPowUncoupled, MODULE_MECH, FE_FIBEREXPPOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-exp-pow-uncoupled", 0, FIBER_WITH_EXPONENTIAL_POWER_LAW_UNCOUPLED_FORMULATION_HTML);

FSFiberExpPowUncoupled::FSFiberExpPowUncoupled() : FSFiberMaterial(FE_FIBEREXPPOW_UNCOUPLED)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "α");
    AddScienceParam(0, UNIT_NONE, "beta" , "β" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
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
	SetFiberGenerator(new FSAnglesVectorGenerator(the, phi));
}

//=============================================================================
// Fiber-Pow-Linear
//=============================================================================

//REGISTER_MATERIAL(FSFiberPowLin, MODULE_MECH, FE_FIBERPOWLIN_COUPLED, FE_MAT_ELASTIC, "fiber-pow-linear", 0, Fiber_with_Toe_Linear_Response);

FSFiberPowLinOld::FSFiberPowLinOld() : FSMaterial(FE_FIBERPOWLIN_COUPLED_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "β");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "λ0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial);
}

REGISTER_MATERIAL(FSFiberPowLin, MODULE_MECH, FE_FIBERPOWLIN_COUPLED, FE_MAT_ELASTIC, "fiber-pow-linear", 0, FIBER_WITH_TOE_LINEAR_RESPONSE_HTML);

FSFiberPowLin::FSFiberPowLin() : FSFiberMaterial(FE_FIBERPOWLIN_COUPLED)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "fiber modulus E");
    AddScienceParam(2, UNIT_NONE, "beta" , "toe power exponent β");
    AddScienceParam(1, UNIT_NONE, "lam0" , "toe stretch ratio λ0");
}

void FSFiberPowLin::Convert(FSFiberPowLinOld* pold)
{
    if (pold == 0) return;

    SetFloatValue(MP_E    , pold->GetFloatValue(FSFiberPowLinOld::MP_E   ));
    SetFloatValue(MP_BETA , pold->GetFloatValue(FSFiberPowLinOld::MP_BETA));
    SetFloatValue(MP_LAM0 , pold->GetFloatValue(FSFiberPowLinOld::MP_LAM0));

	double the = pold->GetFloatValue(FSFiberPowLinOld::MP_THETA);
	double phi = pold->GetFloatValue(FSFiberPowLinOld::MP_PHI);
	SetFiberGenerator(new FSAnglesVectorGenerator(the, phi));
}

//=============================================================================
// Fiber-Pow-Linear Uncoupled
//=============================================================================

//REGISTER_MATERIAL(FSFiberPowLinUncoupled, MODULE_MECH, FE_FIBERPOWLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-pow-linear-uncoupled", 0, Fiber_with_Toe_Linear_Response_Uncoupled_Formulation);

FSFiberPowLinUncoupledOld::FSFiberPowLinUncoupledOld() : FSMaterial(FE_FIBERPOWLIN_UNCOUPLED_OLD)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "β");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "λ0");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

	SetAxisMaterial(new FSAxisMaterial);
}

REGISTER_MATERIAL(FSFiberPowLinUncoupled, MODULE_MECH, FE_FIBERPOWLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-pow-linear-uncoupled", 0, FIBER_WITH_TOE_LINEAR_RESPONSE_UNCOUPLED_FORMULATION_HTML);

FSFiberPowLinUncoupled::FSFiberPowLinUncoupled() : FSFiberMaterial(FE_FIBERPOWLIN_UNCOUPLED)
{
    AddScienceParam(0, UNIT_PRESSURE, "E", "fiber modulus E");
    AddScienceParam(2, UNIT_NONE, "beta" , "toe power exponent β");
    AddScienceParam(1, UNIT_NONE, "lam0" , "toe stretch ratio λ0");
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
	SetFiberGenerator(new FSAnglesVectorGenerator(the, phi));
}

//=============================================================================
// Donnan swelling
//=============================================================================

REGISTER_MATERIAL(FSDonnanSwelling, MODULE_MECH, FE_DONNAN_SWELLING, FE_MAT_ELASTIC, "Donnan equilibrium", 0, DONNAN_EQUILIBRIUM_SWELLING_HTML);

FSDonnanSwelling::FSDonnanSwelling() : FSMaterial(FE_DONNAN_SWELLING)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(0, UNIT_NONE, "Phi", "Phi");
}

//=============================================================================
// Perfect Osmometer
//=============================================================================

REGISTER_MATERIAL(FSPerfectOsmometer, MODULE_MECH, FE_PERFECT_OSMOMETER, FE_MAT_ELASTIC, "perfect osmometer", 0, PERFECT_OSMOMETER_EQUILIBRIUM_OSMOTIC_PRESSURE_HTML);

FSPerfectOsmometer::FSPerfectOsmometer() : FSMaterial(FE_PERFECT_OSMOMETER)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "iosm", "iosm");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
}

//=============================================================================
// Cell Growth
//=============================================================================

REGISTER_MATERIAL(FSCellGrowth, MODULE_MECH, FE_CELL_GROWTH, FE_MAT_ELASTIC, "cell growth", 0, CELL_GROWTH_HTML);

FSCellGrowth::FSCellGrowth() : FSMaterial(FE_CELL_GROWTH)
{
	AddScienceParam(0, UNIT_NONE, "phir", "phir");
	AddScienceParam(0, UNIT_CONCENTRATION, "cr", "cr");
	AddScienceParam(0, UNIT_CONCENTRATION, "ce", "ce");
}

//=============================================================================
// Osmotic pressure using virial coefficients
//=============================================================================

REGISTER_MATERIAL(FSOsmoticVirial, MODULE_MECH, FE_OSMOTIC_VIRIAL, FE_MAT_ELASTIC, "osmotic virial expansion", 0, OSMOTIC_PRESSURE_FROM_VIRIAL_EXPANSION_HTML);

FSOsmoticVirial::FSOsmoticVirial() : FSMaterial(FE_OSMOTIC_VIRIAL)
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

REGISTER_MATERIAL(FSReactionRateConst, MODULE_REACTIONS, FE_REACTION_RATE_CONST, FE_MAT_REACTION_RATE, "constant reaction rate", 0, CONSTANT_REACTION_RATE_HTML);

FSReactionRateConst::FSReactionRateConst() : FSMaterial(FE_REACTION_RATE_CONST)
{
	AddDoubleParam(0, "k", "k");
}

double FSReactionRateConst::GetRateConstant() { return GetParam(0).GetFloatValue(); }

void FSReactionRateConst::SetRateConstant(double K) { SetFloatValue(0, K); }

//=============================================================================
// Huiskes reaction rate
//=============================================================================

REGISTER_MATERIAL(FSReactionRateHuiskes, MODULE_REACTIONS, FE_REACTION_RATE_HUISKES, FE_MAT_REACTION_RATE, "Huiskes reaction rate", 0, HUISKES_REACTION_RATE_HTML);

FSReactionRateHuiskes::FSReactionRateHuiskes() : FSMaterial(FE_REACTION_RATE_HUISKES)
{
	AddDoubleParam(0, "B", "B");
	AddDoubleParam(0, "psi0", "psi0");
}

//=============================================================================
REGISTER_MATERIAL(FEBioReactionRate, MODULE_REACTIONS, FE_REACTION_RATE_FEBIO, FE_MAT_REACTION_RATE, "", 0);
FEBioReactionRate::FEBioReactionRate() : FSMaterial(FE_REACTION_RATE_FEBIO)
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

REGISTER_MATERIAL(FSMembraneReactionRateConst, MODULE_REACTIONS, FE_MREACTION_RATE_CONST, FE_MAT_MREACTION_RATE, "membrane constant reaction rate", 0, CONSTANT_REACTION_RATE_HTML);

FSMembraneReactionRateConst::FSMembraneReactionRateConst() : FSMaterial(FE_MREACTION_RATE_CONST)
{
    AddDoubleParam(0, "k", "k");
}

double FSMembraneReactionRateConst::GetRateConstant() { return GetParam(0).GetFloatValue(); }

void FSMembraneReactionRateConst::SetRateConstant(double K) { SetFloatValue(0, K); }

//=============================================================================
// Membrane ion channel reaction rate
//=============================================================================

REGISTER_MATERIAL(FSMembraneReactionRateIonChannel, MODULE_REACTIONS, FE_MREACTION_RATE_ION_CHNL, FE_MAT_MREACTION_RATE, "membrane ion channel reaction rate", 0, CONSTANT_REACTION_RATE_HTML);

FSMembraneReactionRateIonChannel::FSMembraneReactionRateIonChannel() : FSMaterial(FE_MREACTION_RATE_ION_CHNL)
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

REGISTER_MATERIAL(FSMembraneReactionRateVoltageGated, MODULE_REACTIONS, FE_MREACTION_RATE_VOLT_GTD, FE_MAT_MREACTION_RATE, "membrane voltage-gated reaction rate", 0, CONSTANT_REACTION_RATE_HTML);

FSMembraneReactionRateVoltageGated::FSMembraneReactionRateVoltageGated() : FSMaterial(FE_MREACTION_RATE_VOLT_GTD)
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

REGISTER_MATERIAL(FSCFDFiberExpPow, MODULE_MECH, FE_FIBER_EXP_POW, FE_MAT_CFD_FIBER, "fiber-exp-pow", 0, FIBER_WITH_EXPONENTIAL_POWER_LAW_HTML);

FSCFDFiberExpPow::FSCFDFiberExpPow() : FSMaterial(FE_FIBER_EXP_POW)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "α");
    AddScienceParam(0, UNIT_NONE, "beta" , "β" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "μ"   );
}

//=============================================================================
// CFD Fiber-neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberNH, MODULE_MECH, FE_FIBER_NH, FE_MAT_CFD_FIBER, "fiber-NH", 0, FIBER_WITH_NEO_HOOKEAN_LAW_HTML);

FSCFDFiberNH::FSCFDFiberNH() : FSMaterial(FE_FIBER_NH)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "μ");
}

//=============================================================================
// CFD Fiber-Power-Linear
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberPowLinear, MODULE_MECH, FE_FIBER_POW_LIN, FE_MAT_CFD_FIBER, "fiber-pow-linear", 0);

FSCFDFiberPowLinear::FSCFDFiberPowLinear() : FSMaterial(FE_FIBER_POW_LIN)
{
    AddScienceParam(0, UNIT_PRESSURE, "E"   , "fiber modulus E");
    AddScienceParam(2, UNIT_NONE    , "beta", "toe power exponent β");
    AddScienceParam(1, UNIT_NONE    , "lam0", "toe stretch ratio λ0");
}

//=============================================================================
// CFD Fiber-Exponential-Power-Law uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberExpPowUC, MODULE_MECH, FE_FIBER_EXP_POW_UC, FE_MAT_CFD_FIBER_UC, "fiber-exp-pow-uncoupled", 0);

FSCFDFiberExpPowUC::FSCFDFiberExpPowUC() : FSMaterial(FE_FIBER_EXP_POW_UC)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "α");
    AddScienceParam(0, UNIT_NONE, "beta" , "β" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ξ"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "μ"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// CFD Fiber-neo-Hookean uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberNHUC, MODULE_MECH, FE_FIBER_NH_UC, FE_MAT_CFD_FIBER_UC, "fiber-NH-uncoupled", 0, FIBER_WITH_NEO_HOOKEAN_LAW_UNCOUPLED_HTML);

FSCFDFiberNHUC::FSCFDFiberNHUC() : FSMaterial(FE_FIBER_NH_UC)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "μ"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// CFD Fiber-Power-Linear uncoupled
//=============================================================================

REGISTER_MATERIAL(FSCFDFiberPowLinearUC, MODULE_MECH, FE_FIBER_POW_LIN_UC, FE_MAT_CFD_FIBER_UC, "fiber-pow-linear-uncoupled", 0);

FSCFDFiberPowLinearUC::FSCFDFiberPowLinearUC() : FSMaterial(FE_FIBER_POW_LIN_UC)
{
    AddScienceParam(0, UNIT_PRESSURE, "E"   , "fiber modulus E");
    AddScienceParam(2, UNIT_NONE    , "beta", "toe power exponent β");
    AddScienceParam(1, UNIT_NONE    , "lam0", "toe stretch ratio λ0");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
}

//=============================================================================
// FDD Spherical
//=============================================================================

REGISTER_MATERIAL(FSFDDSpherical, MODULE_MECH, FE_DSTRB_SFD, FE_MAT_CFD_DIST, "spherical", 0, SPHERICAL_HTML);

FSFDDSpherical::FSFDDSpherical() : FSMaterial(FE_DSTRB_SFD)
{
}

//=============================================================================
// FDD Ellipsoidal
//=============================================================================

REGISTER_MATERIAL(FSFDDEllipsoidal, MODULE_MECH, FE_DSTRB_EFD, FE_MAT_CFD_DIST, "ellipsoidal", 0, ELLIPSOIDAL_HTML);

FSFDDEllipsoidal::FSFDDEllipsoidal() : FSMaterial(FE_DSTRB_EFD)
{
    AddVecParam(vec3d(1,1,1), "spa" , "spa");
}

//=============================================================================
// FDD von Mises 3d
//=============================================================================

REGISTER_MATERIAL(FSFDDvonMises3d, MODULE_MECH, FE_DSTRB_VM3, FE_MAT_CFD_DIST, "von-Mises-3d", 0);

FSFDDvonMises3d::FSFDDvonMises3d() : FSMaterial(FE_DSTRB_VM3)
{
    AddDoubleParam(0, "b"   , "concentration");
}

//=============================================================================
// FDD Circular
//=============================================================================

REGISTER_MATERIAL(FSFDDCircular, MODULE_MECH, FE_DSTRB_CFD, FE_MAT_CFD_DIST, "circular", 0, CIRCULAR_HTML);

FSFDDCircular::FSFDDCircular() : FSMaterial(FE_DSTRB_CFD)
{
}

//=============================================================================
// FDD Elliptical
//=============================================================================

REGISTER_MATERIAL(FSFDDElliptical, MODULE_MECH, FE_DSTRB_PFD, FE_MAT_CFD_DIST, "elliptical", 0, ELLIPTICAL_HTML);

FSFDDElliptical::FSFDDElliptical() : FSMaterial(FE_DSTRB_PFD)
{
    AddScienceParam(0, UNIT_NONE, "spa1"   , "spa1");
    AddScienceParam(0, UNIT_NONE, "spa2"   , "spa2");
}

//=============================================================================
// FDD von Mises 2d
//=============================================================================

REGISTER_MATERIAL(FSFDDvonMises2d, MODULE_MECH, FE_DSTRB_VM2, FE_MAT_CFD_DIST, "von-Mises-2d", 0, VON_MISES_DISTRIBUTION_HTML);

FSFDDvonMises2d::FSFDDvonMises2d() : FSMaterial(FE_DSTRB_VM2)
{
    AddScienceParam(0, UNIT_NONE, "b"   , "concentration");
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal
//=============================================================================

REGISTER_MATERIAL(FSSchemeGKT, MODULE_MECH, FE_SCHM_GKT, FE_MAT_CFD_SCHEME, "fibers-3d-gkt", 0, GAUSS_KRONROD_TRAPEZOIDAL_RULE_HTML);

FSSchemeGKT::FSSchemeGKT() : FSMaterial(FE_SCHM_GKT)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");// enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration
//=============================================================================

REGISTER_MATERIAL(FSSchemeFEI, MODULE_MECH, FE_SCHM_FEI, FE_MAT_CFD_SCHEME, "fibers-3d-fei", 0, FINITE_ELEMENT_INTEGRATION_RULE_HTML);

FSSchemeFEI::FSSchemeFEI() : FSMaterial(FE_SCHM_FEI)
{
    AddIntParam(1796, "resolution"   , "resolution");// choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d
//=============================================================================

REGISTER_MATERIAL(FSSchemeT2d, MODULE_MECH, FE_SCHM_T2D, FE_MAT_CFD_SCHEME, "fibers-2d-trapezoidal", 0, TRAPEZOIDAL_RULE_HTML);

FSSchemeT2d::FSSchemeT2d() : FSMaterial(FE_SCHM_T2D)
{
    AddIntParam(31, "nth"   , "nth");// odd and >= 3
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeGKTUC, MODULE_MECH, FE_SCHM_GKT_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-gkt-uncoupled", 0, GAUSS_KRONROD_TRAPEZOIDAL_RULE_HTML);

FSSchemeGKTUC::FSSchemeGKTUC() : FSMaterial(FE_SCHM_GKT_UC)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");//  enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeFEIUC, MODULE_MECH, FE_SCHM_FEI_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-fei-uncoupled", 0, FINITE_ELEMENT_INTEGRATION_RULE_HTML);

FSSchemeFEIUC::FSSchemeFEIUC() : FSMaterial(FE_SCHM_FEI_UC)
{
    AddIntParam(11, "resolution"   , "resolution"); // choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d uncoupled
//=============================================================================

REGISTER_MATERIAL(FSSchemeT2dUC, MODULE_MECH, FE_SCHM_T2D_UC, FE_MAT_CFD_SCHEME_UC, "fibers-2d-trapezoidal-uncoupled", 0, TRAPEZOIDAL_RULE_HTML);

FSSchemeT2dUC::FSSchemeT2dUC() : FSMaterial(FE_SCHM_T2D_UC)
{
    AddIntParam(31, "nth"   , "nth"); // nth (odd and >= 3)
}

//=============================================================================
// CDF Simo
//=============================================================================

REGISTER_MATERIAL(FSCDFSimo, MODULE_MECH, FE_CDF_SIMO, FE_MAT_DAMAGE, "CDF Simo", 0, SIMO_HTML);

FSCDFSimo::FSCDFSimo() : FSMaterial(FE_CDF_SIMO)
{
    AddDoubleParam(0, "a" , "a"); // a must be ≥ 0
    AddScienceParam(0, UNIT_NONE, "b" , "b");
}

//=============================================================================
// CDF Log Normal
//=============================================================================

REGISTER_MATERIAL(FSCDFLogNormal, MODULE_MECH, FE_CDF_LOG_NORMAL, FE_MAT_DAMAGE, "CDF log-normal", 0, LOG_NORMAL_HTML);

FSCDFLogNormal::FSCDFLogNormal() : FSMaterial(FE_CDF_LOG_NORMAL)
{
    AddDoubleParam(0, "mu" , "μ"); // mu must be > 0
    AddScienceParam(0, UNIT_NONE, "sigma" , "σ"); // sigma must be > 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Weibull
//=============================================================================

REGISTER_MATERIAL(FSCDFWeibull, MODULE_MECH, FE_CDF_WEIBULL, FE_MAT_DAMAGE, "CDF Weibull", 0, WEIBULL_HTML);

FSCDFWeibull::FSCDFWeibull() : FSMaterial(FE_CDF_WEIBULL)
{
    AddDoubleParam(0, "mu" , "μ"); // mu must be > 0
    AddScienceParam(0, UNIT_NONE, "alpha" , "α"); // alpha must be ≥ 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Step
//=============================================================================

REGISTER_MATERIAL(FSCDFStep, MODULE_MECH, FE_CDF_STEP, FE_MAT_DAMAGE, "CDF step", 0, STEP_HTML);

FSCDFStep::FSCDFStep() : FSMaterial(FE_CDF_STEP)
{
    AddDoubleParam(0, "mu" , "μ" ); //  mu must be > 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Quintic
//=============================================================================

REGISTER_MATERIAL(FSCDFQuintic, MODULE_MECH, FE_CDF_QUINTIC, FE_MAT_DAMAGE, "CDF quintic", 0, QUINTIC_POLYNOMIAL_HTML);

FSCDFQuintic::FSCDFQuintic() : FSMaterial(FE_CDF_QUINTIC)
{
    AddDoubleParam(0, "mumin" , "μmin"); // mumin must be > 0
    AddDoubleParam(0, "mumax" , "μmax"); // mumax must be > mumin
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax" ); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// DC Simo
//=============================================================================

REGISTER_MATERIAL(FSDCSimo, MODULE_MECH, FE_DC_SIMO, FE_MAT_DAMAGE_CRITERION, "DC Simo", 0, DAMAGE_CRITERION_SIMO_HTML);

FSDCSimo::FSDCSimo() : FSMaterial(FE_DC_SIMO)
{
}

//=============================================================================
// DC Strain Energy Density
//=============================================================================

REGISTER_MATERIAL(FSDCStrainEnergyDensity, MODULE_MECH, FE_DC_SED, FE_MAT_DAMAGE_CRITERION, "DC strain energy density", 0, STRAIN_ENERGY_DENSITY_HTML);

FSDCStrainEnergyDensity::FSDCStrainEnergyDensity() : FSMaterial(FE_DC_SED)
{
}

//=============================================================================
// DC Specific Strain Energy
//=============================================================================

REGISTER_MATERIAL(FSDCSpecificStrainEnergy, MODULE_MECH, FE_DC_SSE, FE_MAT_DAMAGE_CRITERION, "DC specific strain energy", 0, SPECIFIC_STRAIN_ENERGY_HTML);

FSDCSpecificStrainEnergy::FSDCSpecificStrainEnergy() : FSMaterial(FE_DC_SSE)
{
}

//=============================================================================
// DC von Mises Stress
//=============================================================================

REGISTER_MATERIAL(FSDCvonMisesStress, MODULE_MECH, FE_DC_VMS, FE_MAT_DAMAGE_CRITERION, "DC von Mises stress", 0, VON_MISES_STRESS_HTML);

FSDCvonMisesStress::FSDCvonMisesStress() : FSMaterial(FE_DC_VMS)
{
}

//=============================================================================
// DC Drucker Shear Stress
//=============================================================================

REGISTER_MATERIAL(FSDCDruckerShearStress, MODULE_MECH, FE_DC_DRUCKER, FE_MAT_DAMAGE_CRITERION, "DC Drucker shear stress", 0, VON_MISES_STRESS_HTML);

FSDCDruckerShearStress::FSDCDruckerShearStress() : FSMaterial(FE_DC_DRUCKER)
{
    AddScienceParam(1, UNIT_NONE, "c" , "c" ); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// DC Maximum Shear Stress
//=============================================================================

REGISTER_MATERIAL(FSDCMaxShearStress, MODULE_MECH, FE_DC_MSS, FE_MAT_DAMAGE_CRITERION, "DC max shear stress", 0, MAXIMUM_SHEAR_STRESS_HTML);

FSDCMaxShearStress::FSDCMaxShearStress() : FSMaterial(FE_DC_MSS)
{
}

//=============================================================================
// DC Maximum Normal Stress
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalStress, MODULE_MECH, FE_DC_MNS, FE_MAT_DAMAGE_CRITERION, "DC max normal stress", 0, MAXIMUM_NORMAL_STRESS_HTML);

FSDCMaxNormalStress::FSDCMaxNormalStress() : FSMaterial(FE_DC_MNS)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalLagrangeStrain, MODULE_MECH, FE_DC_MNLE, FE_MAT_DAMAGE_CRITERION, "DC max normal Lagrange strain", 0, MAXIMUM_NORMAL_LAGRANGE_STRAIN_HTML);

FSDCMaxNormalLagrangeStrain::FSDCMaxNormalLagrangeStrain() : FSMaterial(FE_DC_MNLE)
{
}

//=============================================================================
// DC Octahedral Shear Strain
//=============================================================================

REGISTER_MATERIAL(FSDCOctahedralShearStrain, MODULE_MECH, FE_DC_OSS, FE_MAT_DAMAGE_CRITERION, "DC octahedral shear strain", 0);

FSDCOctahedralShearStrain::FSDCOctahedralShearStrain() : FSMaterial(FE_DC_OSS)
{
}

//=============================================================================
// DC Simo Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCSimoUC, MODULE_MECH, FE_DC_SIMO_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC Simo uncoupled", 0);

FSDCSimoUC::FSDCSimoUC() : FSMaterial(FE_DC_SIMO_UC)
{
}

//=============================================================================
// DC Strain Energy Density Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCStrainEnergyDensityUC, MODULE_MECH, FE_DC_SED_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC strain energy density uncoupled", 0);

FSDCStrainEnergyDensityUC::FSDCStrainEnergyDensityUC() : FSMaterial(FE_DC_SED_UC)
{
}

//=============================================================================
// DC Specific Strain Energy Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCSpecificStrainEnergyUC, MODULE_MECH, FE_DC_SSE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC specific strain energy uncoupled", 0);

FSDCSpecificStrainEnergyUC::FSDCSpecificStrainEnergyUC() : FSMaterial(FE_DC_SSE_UC)
{
}

//=============================================================================
// DC von Mises Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCvonMisesStressUC, MODULE_MECH, FE_DC_VMS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC von Mises stress uncoupled", 0);

FSDCvonMisesStressUC::FSDCvonMisesStressUC() : FSMaterial(FE_DC_VMS_UC)
{
}

//=============================================================================
// DC Maximum Shear Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxShearStressUC, MODULE_MECH, FE_DC_MSS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max shear stress uncoupled", 0);

FSDCMaxShearStressUC::FSDCMaxShearStressUC() : FSMaterial(FE_DC_MSS_UC)
{
}

//=============================================================================
// DC Maximum Normal Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalStressUC, MODULE_MECH, FE_DC_MNS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal stress uncoupled", 0);

FSDCMaxNormalStressUC::FSDCMaxNormalStressUC() : FSMaterial(FE_DC_MNS_UC)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain Uncoupled
//=============================================================================

REGISTER_MATERIAL(FSDCMaxNormalLagrangeStrainUC, MODULE_MECH, FE_DC_MNLE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal Lagrange strain uncoupled", 0);

FSDCMaxNormalLagrangeStrainUC::FSDCMaxNormalLagrangeStrainUC() : FSMaterial(FE_DC_MNLE_UC)
{
}

//=============================================================================
// Relaxation Exponential with Continuous Spectrum
//=============================================================================

REGISTER_MATERIAL(FSRelaxCSExp, MODULE_MECH, FE_RELAX_CSEXP, FE_MAT_RV_RELAX, "relaxation-CSexp", 0, CSEXP_HTML);

FSRelaxCSExp::FSRelaxCSExp() : FSMaterial(FE_RELAX_CSEXP)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "exponential spectrum constant τ"); // characteristic relaxation time
}

//=============================================================================
// Relaxation Exponential
//=============================================================================

REGISTER_MATERIAL(FSRelaxExp, MODULE_MECH, FE_RELAX_EXP, FE_MAT_RV_RELAX, "relaxation-exponential", 0, EXPONENTIAL_HTML);

FSRelaxExp::FSRelaxExp() : FSMaterial(FE_RELAX_EXP)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "time constant τ"); // characteristic relaxation time
}

//=============================================================================
// Relaxation Exponential Distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxExpDistortion, MODULE_MECH, FE_RELAX_EXP_DIST, FE_MAT_RV_RELAX, "relaxation-exp-distortion", 0, EXPONENTIAL_DISTORTIONAL_HTML);

FSRelaxExpDistortion::FSRelaxExpDistortion() : FSMaterial(FE_RELAX_EXP_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient τ0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient τ1" );
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent α");
}

//=============================================================================
// Relaxation Fung
//=============================================================================

REGISTER_MATERIAL(FSRelaxFung, MODULE_MECH, FE_RELAX_FUNG, FE_MAT_RV_RELAX, "relaxation-Fung", 0, FUNG_HTML);

FSRelaxFung::FSRelaxFung() : FSMaterial(FE_RELAX_FUNG)
{
    AddScienceParam(0, UNIT_TIME, "tau1"   , "min. relaxation time τ1"); //  minimum characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau2"   , "max. relaxation time τ2"); // maximum characteristic relaxation time
}

//=============================================================================
// Relaxation Malkin
//=============================================================================

REGISTER_MATERIAL(FERelaxMalkin, MODULE_MECH, FE_RELAX_MALKIN, FE_MAT_RV_RELAX, "relaxation-Malkin", 0, MALKIN_HTML);

FERelaxMalkin::FERelaxMalkin() : FSMaterial(FE_RELAX_MALKIN)
{
    AddScienceParam(0, UNIT_TIME, "tau1"   , "min. relaxation time τ1"); //  minimum characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau2"   , "max. relaxation time τ2"); // maximum characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"   , "power exponent β"); // exponent
}

//=============================================================================
// Relaxation Park
//=============================================================================

REGISTER_MATERIAL(FSRelaxPark, MODULE_MECH, FE_RELAX_PARK, FE_MAT_RV_RELAX, "relaxation-Park", 0, PARK_HTML);

FSRelaxPark::FSRelaxPark() : FSMaterial(FE_RELAX_PARK)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "τ" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "β"); // exponent
}

//=============================================================================
// Relaxation Park Distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxParkDistortion, MODULE_MECH, FE_RELAX_PARK_DIST, FE_MAT_RV_RELAX, "relaxation-Park-distortion", 0, PARK_DISTORTIONAL_HTML);

FSRelaxParkDistortion::FSRelaxParkDistortion() : FSMaterial(FE_RELAX_PARK_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient τ0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient τ1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "constant coefficient β0"); // exponent
    AddScienceParam(0, UNIT_NONE, "beta1" , "power coefficient β1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent α");
}

//=============================================================================
// Relaxation power
//=============================================================================

REGISTER_MATERIAL(FSRelaxPow, MODULE_MECH, FE_RELAX_POW, FE_MAT_RV_RELAX, "relaxation-power", 0, POWER_HTML);

FSRelaxPow::FSRelaxPow() : FSMaterial(FE_RELAX_POW)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "characteristic time constant τ" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "power exponent β"); // exponent
}

//=============================================================================
// Relaxation power distortion
//=============================================================================

REGISTER_MATERIAL(FSRelaxPowDistortion, MODULE_MECH, FE_RELAX_POW_DIST, FE_MAT_RV_RELAX, "relaxation-power-distortion", 0, POWER_DISTORTIONAL_HTML);

FSRelaxPowDistortion::FSRelaxPowDistortion() : FSMaterial(FE_RELAX_POW_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "constant coefficient τ0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "power coefficient τ1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "constant coefficient β0"); // exponent
    AddScienceParam(0, UNIT_NONE, "beta1" , "power coefficient β1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "power exponent α");
}

//=============================================================================
// Relaxation Prony
//=============================================================================

REGISTER_MATERIAL(FSRelaxProny, MODULE_MECH, FE_RELAX_PRONY, FE_MAT_RV_RELAX, "relaxation-Prony", 0, PRONY_HTML);

FSRelaxProny::FSRelaxProny() : FSMaterial(FE_RELAX_PRONY)
{
    AddScienceParam(0, UNIT_NONE, "g1", "coeffient γ1");
    AddScienceParam(0, UNIT_NONE, "g2", "coeffient γ2");
    AddScienceParam(0, UNIT_NONE, "g3", "coeffient γ3");
    AddScienceParam(0, UNIT_NONE, "g4", "coeffient γ4");
    AddScienceParam(0, UNIT_NONE, "g5", "coeffient γ5");
    AddScienceParam(0, UNIT_NONE, "g6", "coeffient γ6");
    
    AddScienceParam(1, UNIT_TIME, "t1", "relaxation time τ1");
    AddScienceParam(1, UNIT_TIME, "t2", "relaxation time τ2");
    AddScienceParam(1, UNIT_TIME, "t3", "relaxation time τ3");
    AddScienceParam(1, UNIT_TIME, "t4", "relaxation time τ4");
    AddScienceParam(1, UNIT_TIME, "t5", "relaxation time τ5");
    AddScienceParam(1, UNIT_TIME, "t6", "relaxation time τ6");
}

//=============================================================================
// Elastic pressure for ideal gas
//=============================================================================

REGISTER_MATERIAL(FSEPIdealGas, MODULE_FLUID, FE_EP_IDEAL_GAS, FE_MAT_FLUID_ELASTIC, "ideal gas", 0);

FSEPIdealGas::FSEPIdealGas() : FSMaterial(FE_EP_IDEAL_GAS)
{
    AddScienceParam(0, UNIT_MOLAR_MASS, "molar_mass"  , "molar_mass");
}

//=============================================================================
// Elastic pressure for ideal fluid
//=============================================================================

REGISTER_MATERIAL(FSEPIdealFluid, MODULE_FLUID, FE_EP_IDEAL_FLUID, FE_MAT_FLUID_ELASTIC, "ideal fluid", 0);

FSEPIdealFluid::FSEPIdealFluid() : FSMaterial(FE_EP_IDEAL_FLUID)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Elastic pressure for neo-Hookean fluid
//=============================================================================

REGISTER_MATERIAL(FSEPNeoHookeanFluid, MODULE_FLUID, FE_EP_NEOHOOKEAN_FLUID, FE_MAT_FLUID_ELASTIC, "neo-Hookean fluid", 0);

FSEPNeoHookeanFluid::FSEPNeoHookeanFluid() : FSMaterial(FE_EP_NEOHOOKEAN_FLUID)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Viscous Newtonian fluid
//=============================================================================

REGISTER_MATERIAL(FSVFNewtonian, MODULE_FLUID, FE_VF_NEWTONIAN, FE_MAT_FLUID_VISCOSITY, "Newtonian fluid", 0, NEWTONIAN_FLUID_HTML);

FSVFNewtonian::FSVFNewtonian() : FSMaterial(FE_VF_NEWTONIAN)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity μ");
    AddScienceParam(0, UNIT_VISCOSITY, "kappa", "bulk viscosity");
}

//=============================================================================
// Viscous Bingham fluid
//=============================================================================

REGISTER_MATERIAL(FSVFBingham, MODULE_FLUID, FE_VF_BINGHAM, FE_MAT_FLUID_VISCOSITY, "Bingham", 0);

FSVFBingham::FSVFBingham() : FSMaterial(FE_VF_BINGHAM)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity μ"); // viscosity at infinite shear rate
    AddScienceParam(0, UNIT_PRESSURE , "tauy", "yield stress"   );
    AddScienceParam(0, UNIT_NONE     , "n"   , "exponent"       );
}

//=============================================================================
// Viscous Carreau fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCarreau, MODULE_FLUID, FE_VF_CARREAU, FE_MAT_FLUID_VISCOSITY, "Carreau", 0, CARREAU_MODEL_HTML);

FSVFCarreau::FSVFCarreau() : FSMaterial(FE_VF_CARREAU)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity μ0"); // viscosity at zero shear rate
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity μ∞"); // viscosity at infinite shear rate
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time λ"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index n"  );
}

//=============================================================================
// Viscous Carreau-Yasuda fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCarreauYasuda, MODULE_FLUID, FE_VF_CARREAU_YASUDA, FE_MAT_FLUID_VISCOSITY, "Carreau-Yasuda", 0, CARREAU_YASUDA_MODEL_HTML);

FSVFCarreauYasuda::FSVFCarreauYasuda() : FSMaterial(FE_VF_CARREAU_YASUDA)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity μ0"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity μ∞"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time λ"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index n"  );
    AddScienceParam(0, UNIT_NONE, "a" , "power denominator a"  );
}

//=============================================================================
// Viscous Powell-Eyring fluid
//=============================================================================

REGISTER_MATERIAL(FSVFPowellEyring, MODULE_FLUID, FE_VF_POWELL_EYRING, FE_MAT_FLUID_VISCOSITY, "Powell-Eyring", 0, POWELL_EYRING_MODEL_HTML);

FSVFPowellEyring::FSVFPowellEyring() : FSMaterial(FE_VF_POWELL_EYRING)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity μ0"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity μ∞"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time λ"  );
}

//=============================================================================
// Viscous Cross fluid
//=============================================================================

REGISTER_MATERIAL(FSVFCross, MODULE_FLUID, FE_VF_CROSS, FE_MAT_FLUID_VISCOSITY, "Cross", 0, CROSS_MODEL_HTML);

FSVFCross::FSVFCross() : FSMaterial(FE_VF_CROSS)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "zero shear rate viscosity μ0"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "infinite shear rate viscosity μ∞"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time λ"  );
    AddScienceParam(0, UNIT_NONE, "m" , "power"  );
}

//=============================================================================
// Starling solvent supply
//=============================================================================

REGISTER_MATERIAL(FSStarlingSupply, MODULE_MULTIPHASIC, FE_STARLING_SUPPLY, FE_MAT_SOLVENT_SUPPLY, "Starling", 0, STARLING_EQUATION_HTML);

FSStarlingSupply::FSStarlingSupply() : FSMaterial(FE_STARLING_SUPPLY)
{
	AddScienceParam(0, UNIT_FILTRATION, "kp", "filtration coefficient");
	AddScienceParam(0, UNIT_PRESSURE, "pv", "external pressure");
}

//=============================================================================
// const prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FSPrestrainConstGradient, MODULE_MECH, FE_PRESTRAIN_CONST_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "prestrain gradient", 0);

FSPrestrainConstGradient::FSPrestrainConstGradient() : FSMaterial(FE_PRESTRAIN_CONST_GRADIENT)
{
	mat3d F0; F0.unit();
	AddMat3dParam(F0, "F0", "prestrain gradient");
}

//=============================================================================
// in-situ stretch prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FSPrestrainInSituGradient, MODULE_MECH, FE_PRESTRAIN_INSITU_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "in-situ stretch", 0);

FSPrestrainInSituGradient::FSPrestrainInSituGradient() : FSMaterial(FE_PRESTRAIN_INSITU_GRADIENT)
{
	AddScienceParam(1.0, UNIT_NONE, "stretch", "fiber stretch");
	AddBoolParam(false, "isochoric", "isochoric prestrain");
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurvePaper, MODULE_MECH, FE_MAT_PLASTIC_FLOW_PAPER, FE_MAT_PLASTIC_FLOW_RULE, "PFC paper", 0);

FSPlasticFlowCurvePaper::FSPlasticFlowCurvePaper() : FSMaterial(FE_MAT_PLASTIC_FLOW_PAPER)
{
	AddDoubleParam(0, "Y0");
	AddDoubleParam(0, "Ymax");
	AddDoubleParam(1, "w0");
	AddDoubleParam(0, "we");
	AddIntParam(1, "nf");
	AddDoubleParam(0.9, "r");
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurveUser, MODULE_MECH, FE_MAT_PLASTIC_FLOW_USER, FE_MAT_PLASTIC_FLOW_RULE, "PFC user", 0);

FSPlasticFlowCurveUser::FSPlasticFlowCurveUser() : FSMaterial(FE_MAT_PLASTIC_FLOW_USER)
{
	AddProperty("plastic_response", FE_MAT_1DFUNC);
	AddProperty(0, new FS1DPointFunction);
}

//=============================================================================
REGISTER_MATERIAL(FSPlasticFlowCurveMath, MODULE_MECH, FE_MAT_PLASTIC_FLOW_MATH, FE_MAT_PLASTIC_FLOW_RULE, "PFC math", 0);

FSPlasticFlowCurveMath::FSPlasticFlowCurveMath() : FSMaterial(FE_MAT_PLASTIC_FLOW_MATH)
{
	AddIntParam(1, "nf");
	AddDoubleParam(0, "e0");
	AddDoubleParam(1, "emax");
	AddMathParam("<enter math formula>", "plastic_response", "plastic flow curve")->MakeVariable(true);
}


//=============================================================================
REGISTER_MATERIAL(FEBioMaterial, MODULE_MECH, FE_FEBIO_MATERIAL, FE_MAT_ELASTIC, "[febio]", 0);

FEBioMaterial::FEBioMaterial() : FSMaterial(FE_FEBIO_MATERIAL)
{
}

FEBioMaterial::~FEBioMaterial()
{
//	delete m_febClass;
}

void FEBioMaterial::SetTypeString(const std::string& sz)
{
	m_stype = sz;
}

const char* FEBioMaterial::GetTypeString() const
{
	return m_stype.c_str();
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

vec3d FEBioMaterial::GetFiber(FEElementRef& el)
{
	FSProperty* pm = FindProperty("fiber");
	FEBioMaterial* fiber = dynamic_cast<FEBioMaterial*>(pm->GetComponent());

	// evaluate the element's center
	vec3d p = el.center();

	// evaluate the fiber direction
	vec3d v = FEBio::GetMaterialFiber(fiber->m_febClass->GetFEBioClass(), p);
	return v;
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

void FEBioMaterial::SetFEBioMaterial(FEBio::FEBioClass* febClass)
{
	m_febClass = febClass;
}

FEBio::FEBioClass* FEBioMaterial::GetFEBioMaterial()
{
	return m_febClass;
}

bool FEBioMaterial::UpdateData(bool bsave)
{
	if (m_febClass)
	{
		if (bsave) FEBio::UpdateFEBioMaterial(this);
	}
	return false;
}
