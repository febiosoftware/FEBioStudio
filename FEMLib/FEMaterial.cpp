// FEMaterial.cpp: implementation of the FEMaterial class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEMaterial.h"
#include <MeshTools/FEProject.h>
#include <FSCore/paramunit.h>
#include <FEBioStudio/WebDefines.h>

//////////////////////////////////////////////////////////////////////
// FEFiberGeneratorLocal
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEFiberGeneratorLocal, MODULE_MECH, FE_FIBER_GENERATOR_LOCAL, FE_MAT_FIBER_GENERATOR, "local", 0);

FEFiberGeneratorLocal::FEFiberGeneratorLocal() : FEFiberGenerator(FE_FIBER_GENERATOR_LOCAL)
{
	AddVec2iParam(vec2i(), "local", "local");
}

//////////////////////////////////////////////////////////////////////
// FEFiberGeneratorVector
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEFiberGeneratorVector, MODULE_MECH, FE_FIBER_GENERATOR_VECTOR, FE_MAT_FIBER_GENERATOR, "vector", 0);

FEFiberGeneratorVector::FEFiberGeneratorVector() : FEFiberGenerator(FE_FIBER_GENERATOR_VECTOR)
{
	AddVecParam(vec3d(1, 0, 0), "vector", "vector");
}

//////////////////////////////////////////////////////////////////////
// FEIsotropicElastic  - isotropic elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEIsotropicElastic, MODULE_MECH, FE_ISOTROPIC_ELASTIC, FE_MAT_ELASTIC, "isotropic elastic", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Isotropic_Elastic);

FEIsotropicElastic::FEIsotropicElastic() : FEMaterial(FE_ISOTROPIC_ELASTIC)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        );
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio");
}

//////////////////////////////////////////////////////////////////////
// FEOrthoElastic - orthotropic elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEOrthoElastic, MODULE_MECH, FE_ORTHO_ELASTIC, FE_MAT_ELASTIC, "orthotropic elastic", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Orthotropic_Elastic);

FEOrthoElastic::FEOrthoElastic() : FEMaterial(FE_ORTHO_ELASTIC)
{
	m_hasMatAxes = true;

	AddScienceParam(1, UNIT_DENSITY, "density", "density"    );
	AddScienceParam(0, UNIT_PRESSURE ,       "E1", "E1 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E2", "E2 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "E3", "E3 modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G12", "G12 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G23", "G23 shear modulus");
	AddScienceParam(0, UNIT_PRESSURE ,       "G31", "G31 shear modulus");
	AddScienceParam(0, UNIT_NONE   ,       "v12", "Poisson's ratio v12");
	AddScienceParam(0, UNIT_NONE   ,       "v23", "Poisson's ratio v23");
	AddScienceParam(0, UNIT_NONE   ,       "v31", "Poisson's ratio v31");
}

//////////////////////////////////////////////////////////////////////
// FENeoHookean - neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FENeoHookean, MODULE_MECH, FE_NEO_HOOKEAN, FE_MAT_ELASTIC, "neo-Hookean", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Neo_Hookean);

FENeoHookean::FENeoHookean() : FEMaterial(FE_NEO_HOOKEAN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->MakeVariable(true)->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus")->MakeVariable(true);
	AddScienceParam(0, UNIT_NONE   ,       "v", "Poisson's ratio")->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FENaturalNeoHookean - natural neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FENaturalNeoHookean, MODULE_MECH, FE_NATURAL_NEO_HOOKEAN, FE_MAT_ELASTIC, "natural neo-Hookean", MaterialFlags::TOPLEVEL);

FENaturalNeoHookean::FENaturalNeoHookean() : FEMaterial(FE_NATURAL_NEO_HOOKEAN)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"        )->MakeVariable(true);
    AddScienceParam(0, UNIT_PRESSURE ,       "G", "shear modulus"  )->MakeVariable(true);
    AddScienceParam(0, UNIT_PRESSURE ,       "k", "bulk modulus"   )->MakeVariable(true);
}

//////////////////////////////////////////////////////////////////////
// FEIncompNeoHookean - incompressible neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEIncompNeoHookean, MODULE_MECH, FE_INCOMP_NEO_HOOKEAN, FE_MAT_ELASTIC_UNCOUPLED, "incomp neo-Hookean", MaterialFlags::TOPLEVEL);

FEIncompNeoHookean::FEIncompNeoHookean() : FEMaterial(FE_INCOMP_NEO_HOOKEAN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE, "G", "shear modulus");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FEPorousNeoHookean - porous neo-hookean elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPorousNeoHookean, MODULE_MECH, FE_POROUS_NEO_HOOKEAN, FE_MAT_ELASTIC, "porous neo-Hookean", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Porous_Neo_Hookean);

FEPorousNeoHookean::FEPorousNeoHookean() : FEMaterial(FE_POROUS_NEO_HOOKEAN)
{
    AddScienceParam(1, UNIT_DENSITY, "density", "density"        );
    AddScienceParam(0, UNIT_PRESSURE ,       "E", "Young's modulus");
    AddScienceParam(0, UNIT_NONE   ,    "phi0", "solid volume fraction");
}

//////////////////////////////////////////////////////////////////////
// FEMooneyRivlin - Mooney-Rivlin rubber
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEMooneyRivlin, MODULE_MECH, FE_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Mooney_Rivlin);

FEMooneyRivlin::FEMooneyRivlin() : FEMaterial(FE_MOONEY_RIVLIN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     )->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_PRESSURE , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus")->SetPersistent(false);
}

//////////////////////////////////////////////////////////////////////
// FEVerondaWestmann - Veronda-Westmann elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEVerondaWestmann, MODULE_MECH, FE_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "Veronda-Westmann", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Veronda_Westmann);

FEVerondaWestmann::FEVerondaWestmann() : FEMaterial(FE_VERONDA_WESTMANN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density"     );
	AddScienceParam(0, UNIT_PRESSURE , "c1"     , "c1"          );
	AddScienceParam(0, UNIT_NONE   , "c2"     , "c2"          );
	AddScienceParam(0, UNIT_PRESSURE , "k"      , "bulk modulus");
}


//////////////////////////////////////////////////////////////////////
// FECoupledMooneyRivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FECoupledMooneyRivlin, MODULE_MECH, FE_COUPLED_MOONEY_RIVLIN, FE_MAT_ELASTIC, "coupled Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Coupled_Mooney_Rivlin);

FECoupledMooneyRivlin::FECoupledMooneyRivlin() : FEMaterial(FE_COUPLED_MOONEY_RIVLIN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FECoupledVerondaWestmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FECoupledVerondaWestmann, MODULE_MECH, FE_COUPLED_VERONDA_WESTMANN, FE_MAT_ELASTIC, "coupled Veronda-Westmann", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Coupled_Veronda_Westmann);

FECoupledVerondaWestmann::FECoupledVerondaWestmann() : FEMaterial(FE_COUPLED_VERONDA_WESTMANN)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_NONE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FEHolmesMow -Holmes-Mow elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEHolmesMow, MODULE_MECH, FE_HOLMES_MOW, FE_MAT_ELASTIC, "Holmes-Mow", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Holmes_Mow);

FEHolmesMow::FEHolmesMow() : FEMaterial(FE_HOLMES_MOW)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density");
	AddScienceParam(0, UNIT_PRESSURE , "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddScienceParam(0, UNIT_NONE   , "beta", "beta"        );
}

//////////////////////////////////////////////////////////////////////
// FEArrudaBoyce - Arruda-Boyce elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEArrudaBoyce, MODULE_MECH, FE_ARRUDA_BOYCE, FE_MAT_ELASTIC_UNCOUPLED, "Arruda-Boyce", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Arruda_Boyce);

FEArrudaBoyce::FEArrudaBoyce() : FEMaterial(FE_ARRUDA_BOYCE)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "Material density");
	AddScienceParam(0, UNIT_PRESSURE , "mu", "Initial modulus");
	AddScienceParam(0, UNIT_NONE   , "N", "links");
	AddScienceParam(0, UNIT_PRESSURE , "k", "Bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FECarterHayes - Carter-Hayes elasticity
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FECarterHayes, MODULE_MECH, FE_CARTER_HAYES, FE_MAT_ELASTIC, "Carter-Hayes", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Carter_Hayes);

FECarterHayes::FECarterHayes() : FEMaterial(FE_CARTER_HAYES)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "true density");
	AddScienceParam(0, UNIT_PRESSURE , "E0", "E0");
	AddScienceParam(1, UNIT_DENSITY, "rho0", "rho0");
	AddScienceParam(0, UNIT_NONE   , "gamma", "gamma");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddIntParam    (-1, "sbm", "sbm");
}

//////////////////////////////////////////////////////////////////////
// FEPRLig - Poission-Ratio Ligament
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPRLig, MODULE_MECH, FE_PRLIG, FE_MAT_ELASTIC_UNCOUPLED, "PRLig", MaterialFlags::TOPLEVEL);

FEPRLig::FEPRLig() : FEMaterial(FE_PRLIG)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_NONE   , "c1"     , "c1");
	AddScienceParam(1, UNIT_NONE   , "c2"     , "c2");
	AddScienceParam(0, UNIT_NONE   , "v0"     , "v0");
	AddScienceParam(0, UNIT_NONE   , "m"      , "m" );
	AddScienceParam(0, UNIT_NONE   , "mu"     , "mu");
	AddScienceParam(0, UNIT_NONE   , "k"      , "k" );
}

//////////////////////////////////////////////////////////////////////
// FEOldFiberMaterial - material for fibers
//////////////////////////////////////////////////////////////////////

FEFiberGeneratorMaterial::FEFiberGeneratorMaterial() : FEMaterial(0)
{
	AddIntParam(0, "fiber", "fiber")->SetEnumNames("local\0cylindrical\0spherical\0vector\0user\0angles\0polar\0\0");
	AddIntParam(0, "n0", "n0");
	AddIntParam(0, "n1", "n1");
	AddVecParam(vec3d(0, 0, 0), "r");
	AddVecParam(vec3d(0, 0, 1), "a");
	AddVecParam(vec3d(1, 0, 0), "d");
	AddDoubleParam(0, "theta");
	AddDoubleParam(0, "phi");
}

bool FEFiberGeneratorMaterial::UpdateData(bool bsave)
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

vec3d FEFiberGeneratorMaterial::GetFiberVector(FEElementRef& el)
{
	switch (m_naopt)
	{
	case FE_FIBER_LOCAL:
	{
		FECoreMesh* pm = el.m_pmesh;
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
		FECoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i<n; ++i) c += pm->NodePosition(el->m_node[i]);
		c /= (double)n;

		// find the vector to the axis
		vec3d r = m_r;
		vec3d a = m_a;
		vec3d v = m_d;
		vec3d b = (c - r) - a*(a*(c - r)); b.Normalize();

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
		FECoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i<n; ++i) c += pm->NodePosition(el->m_node[i]);
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
		const double the = m_theta*pi / 180.;
		const double phi = m_phi*pi / 180.;

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
		FECoreMesh* pm = el.m_pmesh;
		int n = el->Nodes();
		vec3d c(0, 0, 0);
		for (int i = 0; i<n; ++i) c += pm->NodePosition(el->m_node[i]);
		c /= (double)n;

		// find the vector to the axis
		vec3d r = m_r;
		vec3d a = m_a;

		vec3d b = (c - r) - a*(a*(c - r));
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

FEOldFiberMaterial::FEOldFiberMaterial()
{
	m_fiber.m_naopt = FE_FIBER_LOCAL;
	m_fiber.m_nuser = 0;
	m_fiber.m_n[0] = m_fiber.m_n[1] = 0;
	m_fiber.m_r = vec3d(0,0,0);
	m_fiber.m_a = vec3d(0,0,1);
	m_fiber.m_d = vec3d(1,0,0);
	m_fiber.m_theta = 0.0;
	m_fiber.m_phi = 90.0;
	m_fiber.m_d0 = m_fiber.m_d1 = vec3d(0,0,1);
	m_fiber.m_R0 = 0; m_fiber.m_R1 = 1;
}

FEOldFiberMaterial::FEOldFiberMaterial(const FEOldFiberMaterial& m) {}
FEOldFiberMaterial& FEOldFiberMaterial::operator = (const FEOldFiberMaterial& m) { return (*this); }

void FEOldFiberMaterial::copy(FEOldFiberMaterial* pm)
{
	m_fiber.m_naopt = pm->m_fiber.m_naopt;
	m_fiber.m_nuser = pm->m_fiber.m_nuser;
	m_fiber.m_n[0] = pm->m_fiber.m_n[0];
	m_fiber.m_n[1] = pm->m_fiber.m_n[1];
	m_fiber.m_r = pm->m_fiber.m_r;
	m_fiber.m_a = pm->m_fiber.m_a;
	m_fiber.m_d = pm->m_fiber.m_d;
	m_fiber.m_theta = pm->m_fiber.m_theta;
	m_fiber.m_phi = pm->m_fiber.m_phi;
	m_fiber.m_d0 = pm->m_fiber.m_d0;
	m_fiber.m_d1 = pm->m_fiber.m_d1;
	m_fiber.m_R0 = pm->m_fiber.m_R0;
	m_fiber.m_R1 = pm->m_fiber.m_R1;

//	GetParamBlock() = pm->GetParamBlock();
}

void FEOldFiberMaterial::Save(OArchive &ar)
{
	ar.WriteChunk(MP_AOPT, m_fiber.m_naopt);
	ar.WriteChunk(MP_N, m_fiber.m_n, 2);
	ar.WriteChunk(MP_R, m_fiber.m_r);
	ar.WriteChunk(MP_A, m_fiber.m_a);
	ar.WriteChunk(MP_D, m_fiber.m_d);
	ar.WriteChunk(MP_NUSER, m_fiber.m_nuser);
	ar.WriteChunk(MP_THETA, m_fiber.m_theta);
	ar.WriteChunk(MP_PHI, m_fiber.m_phi);
	ar.WriteChunk(MP_D0, m_fiber.m_d0);
	ar.WriteChunk(MP_D1, m_fiber.m_d1);
	ar.WriteChunk(MP_R0, m_fiber.m_R0);
	ar.WriteChunk(MP_R1, m_fiber.m_R1);
	ar.BeginChunk(MP_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FEOldFiberMaterial::Load(IArchive& ar)
{
	TRACE("FEOldFiberMaterial::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case MP_AOPT: ar.read(m_fiber.m_naopt); break;
		case MP_N: ar.read(m_fiber.m_n, 2); break;
		case MP_R: ar.read(m_fiber.m_r); break;
		case MP_A: ar.read(m_fiber.m_a); break;
		case MP_D: ar.read(m_fiber.m_d); break;
		case MP_NUSER: ar.read(m_fiber.m_nuser); break;
		case MP_THETA: ar.read(m_fiber.m_theta); break;
		case MP_PHI: ar.read(m_fiber.m_phi); break;
		case MP_D0: ar.read(m_fiber.m_d0); break;
		case MP_D1: ar.read(m_fiber.m_d1); break;
		case MP_R0: ar.read(m_fiber.m_R0); break;
		case MP_R1: ar.read(m_fiber.m_R1); break;
		case MP_PARAMS: 
			ParamContainer::Load(ar);
			break;
		}
		ar.CloseChunk();
	}
	m_fiber.UpdateData(false);
	m_fiber.UpdateData(true);
}

//////////////////////////////////////////////////////////////////////
// FETransverselyIsotropic - base class for transversely isotropic
//////////////////////////////////////////////////////////////////////

FETransverselyIsotropic::FETransverselyIsotropic(int ntype) : FEMaterial(ntype)
{
	m_pfiber = 0;
}

FEOldFiberMaterial* FETransverselyIsotropic::GetFiberMaterial()
{
	return m_pfiber;
}

void FETransverselyIsotropic::SetFiberMaterial(FEOldFiberMaterial* fiber)
{
	m_pfiber = fiber;
}

vec3d FETransverselyIsotropic::GetFiber(FEElementRef& el)
{
	FEOldFiberMaterial& fiber = *m_pfiber;
	return fiber.m_fiber.GetFiberVector(el);
}

void FETransverselyIsotropic::copy(FEMaterial *pmat)
{
	FETransverselyIsotropic& m = dynamic_cast<FETransverselyIsotropic&>(*pmat);
	assert(m.Type() == m_ntype);

	FEMaterial::copy(pmat);
	m_pfiber->copy(m.m_pfiber);
}

void FETransverselyIsotropic::Save(OArchive &ar)
{
	ar.BeginChunk(MP_MAT);
	{
		FEMaterial::Save(ar);
	}
	ar.EndChunk();

	ar.BeginChunk(MP_FIBERS);
	{
		m_pfiber->Save(ar);
	}
	ar.EndChunk();
}

void FETransverselyIsotropic::Load(IArchive &ar)
{
	TRACE("FETransverselyIsotropic::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case MP_MAT: FEMaterial::Load(ar); break;
		case MP_FIBERS: m_pfiber->Load(ar); break;
		default:
			throw ReadError("unknown CID in FETransverselyIsotropic::Load");
		}

		ar.CloseChunk();
	}
}

//////////////////////////////////////////////////////////////////////
// FETransMooneyRivlinOld - transversely isotropic mooney-rivlin (obsolete implementation)
//////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FETransMooneyRivlinOld, MODULE_MECH, FE_TRANS_MOONEY_RIVLIN_OLD, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FETransMooneyRivlinOld::Fiber::Fiber()
{
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE  , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE  , "lam_max", "lam_max");
	AddDoubleParam(0, "ca0" , "ca0" )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "beta", "beta")->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "L0"  , "L0"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "Lr"  , "Lr"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "active_contraction", "active_contraction")->SetState(Param_State::Param_READWRITE)->SetLoadCurve();
}

FETransMooneyRivlinOld::FETransMooneyRivlinOld() : FETransverselyIsotropic(FE_TRANS_MOONEY_RIVLIN_OLD)
{
	// define the fiber class
	SetFiberMaterial(new Fiber);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus");
}

//////////////////////////////////////////////////////////////////////
// FETransVerondaWestmann - transversely isotropic veronda-westmann (obsolete)
//////////////////////////////////////////////////////////////////////

//REGISTER_MATERIAL(FETransVerondaWestmannOld, MODULE_MECH, FE_TRANS_VERONDA_WESTMANN_OLD, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Veronda-Westmann", MaterialFlags::TOPLEVEL);

FETransVerondaWestmannOld::Fiber::Fiber()
{
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE  , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE  , "lam_max", "lam_max");
	AddDoubleParam(0, "ca0" , "ca0" )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "beta", "beta")->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "L0"  , "L0"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "Lr"  , "Lr"  )->SetState(Param_State::Param_READWRITE);
	AddDoubleParam(0, "active_contraction", "active_contraction")->SetState(Param_State::Param_READWRITE)->SetLoadCurve();
}

FETransVerondaWestmannOld::FETransVerondaWestmannOld() : FETransverselyIsotropic(FE_TRANS_VERONDA_WESTMANN_OLD)
{
	// define the fiber class
	SetFiberMaterial(new Fiber);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_NONE   , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus");
}


//////////////////////////////////////////////////////////////////////
// FEActiveContraction - Active contraction material
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEActiveContraction, MODULE_MECH, FE_MAT_ACTIVE_CONTRACTION, FE_MAT_ACTIVE_CONTRACTION_CLASS, "active_contraction", 0);

FEActiveContraction::FEActiveContraction() : FEMaterial(FE_MAT_ACTIVE_CONTRACTION)
{
	AddDoubleParam(0, "ascl", "scale")->SetLoadCurve();
	AddDoubleParam(0, "ca0");
	AddDoubleParam(0, "beta");
	AddDoubleParam(0, "l0");
	AddDoubleParam(0, "refl");
}

//////////////////////////////////////////////////////////////////////
// FETransMooneyRivlin - transversely isotropic mooney-rivlin
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FETransMooneyRivlin, MODULE_MECH, FE_TRANS_ISO_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Transversely_Isotropic_Mooney_Rivlin);

FETransMooneyRivlin::FETransMooneyRivlin() : FETransverselyIsotropic(FE_TRANS_ISO_MOONEY_RIVLIN)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lam_max", "lam_max");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");

	AddProperty("active_contraction", FE_MAT_ACTIVE_CONTRACTION_CLASS);
}

void FETransMooneyRivlin::Convert(FETransMooneyRivlinOld* pold)
{
	if (pold == 0) return;

	FETransMooneyRivlinOld::Fiber* oldFiber = dynamic_cast<FETransMooneyRivlinOld::Fiber*>(pold->GetFiberMaterial());

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FETransMooneyRivlinOld::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FETransMooneyRivlinOld::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FETransMooneyRivlinOld::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FETransMooneyRivlinOld::MP_K));
	SetFloatValue(MP_C3     , oldFiber->GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C3));
	SetFloatValue(MP_C4     , oldFiber->GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C4));
	SetFloatValue(MP_C5     , oldFiber->GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C5));
	SetFloatValue(MP_LAM    , oldFiber->GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_LAM));

	GetFiberMaterial()->copy(oldFiber);
}

//////////////////////////////////////////////////////////////////////
// FETransVerondaWestmann - transversely isotropic Veronda-Westmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FETransVerondaWestmann, MODULE_MECH, FE_TRANS_ISO_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "trans iso Veronda-Westmann", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Transversely_Isotropic_Veronda_Westmann);

FETransVerondaWestmann::FETransVerondaWestmann() : FETransverselyIsotropic(FE_TRANS_ISO_VERONDA_WESTMANN)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lam_max", "lam_max");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");

	AddProperty("active_contraction", FE_MAT_ACTIVE_CONTRACTION_CLASS);
}

void FETransVerondaWestmann::Convert(FETransVerondaWestmannOld* pold)
{
	if (pold == 0) return;

	FETransVerondaWestmannOld::Fiber* oldFiber = dynamic_cast<FETransVerondaWestmannOld::Fiber*>(pold->GetFiberMaterial());

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FETransVerondaWestmannOld::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FETransVerondaWestmannOld::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FETransVerondaWestmannOld::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FETransVerondaWestmannOld::MP_K));
	SetFloatValue(MP_C3     , oldFiber->GetFloatValue(FETransVerondaWestmannOld::Fiber::MP_C3));
	SetFloatValue(MP_C4     , oldFiber->GetFloatValue(FETransVerondaWestmannOld::Fiber::MP_C4));
	SetFloatValue(MP_C5     , oldFiber->GetFloatValue(FETransVerondaWestmannOld::Fiber::MP_C5));
	SetFloatValue(MP_LAM    , oldFiber->GetFloatValue(FETransVerondaWestmannOld::Fiber::MP_LAM));

	GetFiberMaterial()->copy(oldFiber);
}

//////////////////////////////////////////////////////////////////////
// FECoupledTransIsoVerondaWestmann
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FECoupledTransIsoVerondaWestmann, MODULE_MECH, FE_COUPLED_TRANS_ISO_VW, FE_MAT_ELASTIC, "coupled trans-iso Veronda-Westmann", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Coupled_Transversely_Isotropic_Veronda_Westmann);

FECoupledTransIsoVerondaWestmann::FECoupledTransIsoVerondaWestmann() : FETransverselyIsotropic(FE_COUPLED_TRANS_ISO_VW)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density")->SetPersistent(false);
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_PRESSURE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_NONE, "lambda", "lambda");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//=============================================================================
// Coupled Trans-iso Mooney-Rivlin (Obsolete implementation)
//=============================================================================

//REGISTER_MATERIAL(FECoupledTransIsoMooneyRivlinOld, MODULE_MECH, FE_COUPLED_TRANS_ISO_MR, FE_MAT_ELASTIC, "coupled trans-iso Mooney-Rivlin", MaterialFlags::TOPLEVEL);

FECoupledTransIsoMooneyRivlinOld::FECoupledTransIsoMooneyRivlinOld() : FEMaterial(FE_COUPLED_TRANS_ISO_MR)
{
	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE , "c3", "c3");
	AddScienceParam(0, UNIT_NONE   , "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE , "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE , "k" , "bulk modulus");
	AddScienceParam(0, UNIT_NONE   , "lambda", "lambda");
}

//=============================================================================
// Coupled Trans-iso Mooney-Rivlin
//=============================================================================

REGISTER_MATERIAL(FECoupledTransIsoMooneyRivlin, MODULE_MECH, FE_COUPLED_TRANS_ISO_MR, FE_MAT_ELASTIC, "coupled trans-iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Coupled_Transversely_Isotropic_Mooney_Rivlin);

FECoupledTransIsoMooneyRivlin::FECoupledTransIsoMooneyRivlin() : FETransverselyIsotropic(FE_COUPLED_TRANS_ISO_MR)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3", "c3");
	AddScienceParam(0, UNIT_NONE, "c4", "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5", "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddScienceParam(0, UNIT_NONE, "lambda", "lambda");
}

void FECoupledTransIsoMooneyRivlin::Convert(FECoupledTransIsoMooneyRivlinOld* pold)
{
	if (pold == 0) return;

	SetFloatValue(MP_DENSITY, pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_DENSITY));
	SetFloatValue(MP_C1     , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_C1));
	SetFloatValue(MP_C2     , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_C2));
	SetFloatValue(MP_K      , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_K));
	SetFloatValue(MP_C3     , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_C3));
	SetFloatValue(MP_C4     , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_C4));
	SetFloatValue(MP_C5     , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_C5));
	SetFloatValue(MP_LAMBDA , pold->GetFloatValue(FECoupledTransIsoMooneyRivlin::MP_LAMBDA));
}

//=============================================================================
// FEMooneyRivlinVonMisesFibers
//=============================================================================

REGISTER_MATERIAL(FEMooneyRivlinVonMisesFibers, MODULE_MECH, FE_MAT_MR_VON_MISES_FIBERS, FE_MAT_ELASTIC_UNCOUPLED, "Mooney-Rivlin von Mises Fibers", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Mooney_Rivlin_Von_Mises_Distributed_Fibers);

FEMooneyRivlinVonMisesFibers::FEMooneyRivlinVonMisesFibers() : FEMaterial(FE_MAT_MR_VON_MISES_FIBERS)
{
	m_hasMatAxes = true;

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density");
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3");
	AddScienceParam(0, UNIT_NONE, "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddScienceParam(0, UNIT_NONE, "lam_max");
	AddDoubleParam(0, "kf");
	AddDoubleParam(0, "vmc");
	AddDoubleParam(0, "var_n");
	AddDoubleParam(0, "tp");
	AddIntParam(0, "gipt");
}

//=============================================================================
// FE2DTransIsoMooneyRivlin
//=============================================================================

REGISTER_MATERIAL(FE2DTransIsoMooneyRivlin, MODULE_MECH, FE_MAT_2D_TRANS_ISO_MR, FE_MAT_ELASTIC_UNCOUPLED, "2D trans iso Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Transversely_Isotropic_Mooney_Rivlin);

FE2DTransIsoMooneyRivlin::FE2DTransIsoMooneyRivlin() : FETransverselyIsotropic(FE_MAT_2D_TRANS_ISO_MR)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	// define material parameters
	AddScienceParam(1, UNIT_DENSITY, "density");
	AddScienceParam(0, UNIT_PRESSURE, "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2");
	AddScienceParam(0, UNIT_PRESSURE, "c3");
	AddScienceParam(0, UNIT_NONE, "c4");
	AddScienceParam(0, UNIT_PRESSURE, "c5");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddScienceParam(0, UNIT_NONE, "lam_max");
}

//////////////////////////////////////////////////////////////////////
// FERigidMaterial - rigid body material
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FERigidMaterial, MODULE_MECH, FE_RIGID_MATERIAL, FE_MAT_RIGID, "Rigid body", MaterialFlags::TOPLEVEL, Materials_Rigid_Body);

FERigidMaterial::FERigidMaterial() : FEMaterial(FE_RIGID_MATERIAL)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE   , "v", "Poisson's ratio");
	AddBoolParam  (false, "Auto-COM", "Auto-COM");
	AddVecParam   (vec3d(0,0,0), "rc", "Center of mass");

	m_pid = -1;
}

void FERigidMaterial::Save(OArchive& ar)
{
	ar.WriteChunk(CID_MAT_RIGID_PID, m_pid);
	ar.BeginChunk(CID_MAT_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FERigidMaterial::Load(IArchive &ar)
{
	TRACE("FERigidMaterial::Load");

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

void FERigidMaterial::SetAutoCOM(bool b)
{
	SetBoolValue(MP_COM, b);
}

void FERigidMaterial::SetCenterOfMass(const vec3d& r)
{
	SetVecValue(MP_RC, r);
}

void FERigidMaterial::copy(FEMaterial* pmat)
{
	FEMaterial::copy(pmat);
	m_pid = (dynamic_cast<FERigidMaterial*>(pmat))->m_pid;
}

////////////////////////////////////////////////////////////////////////
// FETCNonlinearOrthotropic - Tension-Compression Nonlinear Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FETCNonlinearOrthotropic, MODULE_MECH, FE_TCNL_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "TC nonlinear orthotropic", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Tension_Compression_Nonlinear_Orthotropic);

FETCNonlinearOrthotropic::FETCNonlinearOrthotropic() : FEMaterial(FE_TCNL_ORTHO)
{
	m_hasMatAxes = true;

	AddScienceParam(1, UNIT_DENSITY, "density");
	AddScienceParam(0, UNIT_PRESSURE , "c1");
	AddScienceParam(0, UNIT_PRESSURE , "c2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");

	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);
}

////////////////////////////////////////////////////////////////////////
// FEFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEFungOrthotropic, MODULE_MECH, FE_FUNG_ORTHO, FE_MAT_ELASTIC_UNCOUPLED, "Fung orthotropic", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Fung_Orthotropic);

FEFungOrthotropic::FEFungOrthotropic() : FEMaterial(FE_FUNG_ORTHO)
{
	m_hasMatAxes = true;

	AddScienceParam(1, UNIT_DENSITY, "density", "density");
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
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
}

////////////////////////////////////////////////////////////////////////
// FEFungOrthotropic - Fung Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEFungOrthoCompressible, MODULE_MECH, FE_FUNG_ORTHO_COUPLED, FE_MAT_ELASTIC_UNCOUPLED, "Fung-ortho-compressible", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Fung_Orthotropic_Compressible);

FEFungOrthoCompressible::FEFungOrthoCompressible() : FEMaterial(FE_FUNG_ORTHO_COUPLED)
{
	m_hasMatAxes = true;

	AddScienceParam(1, UNIT_DENSITY, "density", "density");
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
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
}

////////////////////////////////////////////////////////////////////////
// FELinearOrthotropic - Linear Orthotropic
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FELinearOrthotropic, MODULE_MECH, FE_LINEAR_ORTHO, FE_MAT_ELASTIC, "linear orthotropic", MaterialFlags::TOPLEVEL);

FELinearOrthotropic::FELinearOrthotropic() : FEMaterial(FE_LINEAR_ORTHO)
{
	m_hasMatAxes = true;

	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "E1", "E1");
	AddScienceParam(0, UNIT_PRESSURE , "E2", "E2");
	AddScienceParam(0, UNIT_PRESSURE , "E3", "E3");
	AddScienceParam(0, UNIT_PRESSURE , "G12", "G12");
	AddScienceParam(0, UNIT_PRESSURE , "G23", "G23");
	AddScienceParam(0, UNIT_PRESSURE , "G31", "G31");
	AddScienceParam(0, UNIT_NONE   , "v12", "v12");
	AddScienceParam(0, UNIT_NONE   , "v23", "v23");
	AddScienceParam(0, UNIT_NONE   , "v31", "v31");
}

////////////////////////////////////////////////////////////////////////
// FEMuscleMaterial - Silvia Blemker's muscle material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEMuscleMaterial, MODULE_MECH, FE_MUSCLE_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "muscle material", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Muscle_Material);

FEMuscleMaterial::FEMuscleMaterial() : FETransverselyIsotropic(FE_MUSCLE_MATERIAL)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	AddScienceParam(1, UNIT_DENSITY, "density");
	AddScienceParam(0, UNIT_PRESSURE , "g1");
	AddScienceParam(0, UNIT_PRESSURE , "g2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
	AddDoubleParam(0, "p1");
	AddDoubleParam(0, "p2");
	AddDoubleParam(0, "Lofl");
	AddDoubleParam(0, "lam_max");
	AddDoubleParam(0, "smax");
}

////////////////////////////////////////////////////////////////////////
// FETendonMaterial - Silvia Blemker's tendon material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FETendonMaterial, MODULE_MECH, FE_TENDON_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "tendon material", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Tendon_Material);

FETendonMaterial::FETendonMaterial() : FETransverselyIsotropic(FE_TENDON_MATERIAL)
{
	SetFiberMaterial(new FEOldFiberMaterial);

	AddScienceParam(1, UNIT_DENSITY, "density");
	AddScienceParam(0, UNIT_PRESSURE , "g1");
	AddScienceParam(0, UNIT_PRESSURE , "g2");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
	AddDoubleParam(0, "l1");
	AddDoubleParam(0, "l2");
	AddDoubleParam(0, "lam_max");
}

////////////////////////////////////////////////////////////////////////
// Ogden material
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEOgdenMaterial, MODULE_MECH, FE_OGDEN_MATERIAL, FE_MAT_ELASTIC_UNCOUPLED, "Ogden", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Ogden);

FEOgdenMaterial::FEOgdenMaterial() : FEMaterial(FE_OGDEN_MATERIAL)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, UNIT_PRESSURE , "k", "bulk modulus");
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

REGISTER_MATERIAL(FEOgdenUnconstrained, MODULE_MECH, FE_OGDEN_UNCONSTRAINED, FE_MAT_ELASTIC, "Ogden unconstrained", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Ogden_Unconstrained);

FEOgdenUnconstrained::FEOgdenUnconstrained() : FEMaterial(FE_OGDEN_UNCONSTRAINED)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
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
// FEEFDMooneyRivlin - ellipsoidal fiber distribution model with MR base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEEFDMooneyRivlin, MODULE_MECH, FE_EFD_MOONEY_RIVLIN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Mooney-Rivlin", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Ellipsoidal_Fiber_Distribution_Mooney_Rivlin);

FEEFDMooneyRivlin::FEEFDMooneyRivlin() : FEMaterial(FE_EFD_MOONEY_RIVLIN)
{
	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);
}

//////////////////////////////////////////////////////////////////////
// FEEFDNeoHookean - ellipsoidal fiber distribution model with MR base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEEFDNeoHookean, MODULE_MECH, FE_EFD_NEO_HOOKEAN, FE_MAT_ELASTIC, "EFD neo-Hookean", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Ellipsoidal_Fiber_Distribution_Neo_Hookean);

FEEFDNeoHookean::FEEFDNeoHookean() : FEMaterial(FE_EFD_NEO_HOOKEAN)
{
	AddScienceParam(0, UNIT_PRESSURE, "E", "Young's modulus");
	AddScienceParam(0, UNIT_NONE  , "v", "Poisson's ratio");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi"  )->SetUnit(UNIT_PRESSURE);
}

//////////////////////////////////////////////////////////////////////
// FEEFDDonnan - ellipsoidal fiber distribution model with Donnan base
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEEFDDonnan, MODULE_MECH, FE_EFD_DONNAN, FE_MAT_ELASTIC, "EFD Donnan equilibrium", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Ellipsoidal_Fiber_Distribution_with_Donnan_Equilibrium_Swelling);

FEEFDDonnan::FEEFDDonnan() : FEMaterial(FE_EFD_DONNAN)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(1, UNIT_NONE, "Phi", "Phi");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi")->SetUnit(UNIT_PRESSURE);
}

/////////////////////////////////////////////////////////////////////////////////////////
// FEEFDVerondaWestmann - ellipsoidal fiber distribution model with Veronda Westmann base
/////////////////////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEEFDVerondaWestmann, MODULE_MECH, FE_EFD_VERONDA_WESTMANN, FE_MAT_ELASTIC_UNCOUPLED, "EFD Veronda-Westmann", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Uncoupled_Materials_Ellipsoidal_Fiber_Distribution_Veronda_Westmann);

FEEFDVerondaWestmann::FEEFDVerondaWestmann() : FEMaterial(FE_EFD_VERONDA_WESTMANN)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_PRESSURE, "c1", "c1");
	AddScienceParam(0, UNIT_PRESSURE, "c2", "c2");
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi", "ksi"  )->SetUnit(UNIT_PRESSURE);
}

////////////////////////////////////////////////////////////////////////
// FECubicCLE - Conewise Linear Elasticity with cubic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FECubicCLE, MODULE_MECH, FE_CLE_CUBIC, FE_MAT_ELASTIC, "cubic CLE", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Cubic_CLE);

FECubicCLE::FECubicCLE() : FEMaterial(FE_CLE_CUBIC)
{
	m_hasMatAxes = true;

    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddScienceParam(0, UNIT_PRESSURE , "lp1", "lambda_+1");
    AddScienceParam(0, UNIT_PRESSURE , "lm1", "lambda_-1");
    AddScienceParam(0, UNIT_PRESSURE , "l2" , "lambda_2" );
    AddScienceParam(0, UNIT_PRESSURE , "mu" , "mu"       );
}

////////////////////////////////////////////////////////////////////////
// FECubicCLE - Conewise Linear Elasticity with orthotropic symmetry
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEOrthotropicCLE, MODULE_MECH, FE_CLE_ORTHOTROPIC, FE_MAT_ELASTIC, "orthotropic CLE", MaterialFlags::TOPLEVEL, Materials_Elastic_Solids_Compressible_Materials_Orthotropic_CLE);

FEOrthotropicCLE::FEOrthotropicCLE() : FEMaterial(FE_CLE_ORTHOTROPIC)
{
	m_hasMatAxes = true;

    AddScienceParam(1, UNIT_DENSITY, "density", "density");
    AddScienceParam(0, UNIT_PRESSURE , "lp11", "lambda_+11");
    AddScienceParam(0, UNIT_PRESSURE , "lp22", "lambda_+22");
    AddScienceParam(0, UNIT_PRESSURE , "lp33", "lambda_+33");
    AddScienceParam(0, UNIT_PRESSURE , "lm11", "lambda_-11");
    AddScienceParam(0, UNIT_PRESSURE , "lm22", "lambda_-22");
    AddScienceParam(0, UNIT_PRESSURE , "lm33", "lambda_-33");
    AddScienceParam(0, UNIT_PRESSURE , "l12" , "lambda_12" );
    AddScienceParam(0, UNIT_PRESSURE , "l23" , "lambda_23" );
    AddScienceParam(0, UNIT_PRESSURE , "l31" , "lambda_31" );
    AddScienceParam(0, UNIT_PRESSURE , "mu1" , "mu1"       );
    AddScienceParam(0, UNIT_PRESSURE , "mu2" , "mu2"       );
    AddScienceParam(0, UNIT_PRESSURE , "mu3" , "mu3"       );
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionUniaxial - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionUniaxial, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI, FE_MAT_ELASTIC, "prescribed uniaxial active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Compressible_Materials_Prescribed_Uniaxial_Active_Contraction);

FEPrescribedActiveContractionUniaxial::FEPrescribedActiveContractionUniaxial() : FEMaterial(FE_ACTIVE_CONTRACT_UNI)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionTransIso - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionTransIso, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO, FE_MAT_ELASTIC, "prescribed trans iso active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Compressible_Materials_Prescribed_Transversely_Isotropic_Active_Contraction);

FEPrescribedActiveContractionTransIso::FEPrescribedActiveContractionTransIso() : FEMaterial(FE_ACTIVE_CONTRACT_TISO)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionIsotropic - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionIsotropic, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO, FE_MAT_ELASTIC, "prescribed isotropic active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Compressible_Materials_Prescribed_Isotropic_Active_Contraction);

FEPrescribedActiveContractionIsotropic::FEPrescribedActiveContractionIsotropic() : FEMaterial(FE_ACTIVE_CONTRACT_ISO)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionUniaxialUC - Prescribed uniaxial active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionUniaxialUC, MODULE_MECH, FE_ACTIVE_CONTRACT_UNI_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed uniaxial active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Uncoupled_Materials_Uncoupled_Prescribed_Uniaxial_Active_Contraction);

FEPrescribedActiveContractionUniaxialUC::FEPrescribedActiveContractionUniaxialUC() : FEMaterial(FE_ACTIVE_CONTRACT_UNI_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionTransIsoUC - Prescribed trans iso active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionTransIsoUC, MODULE_MECH, FE_ACTIVE_CONTRACT_TISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed trans iso active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Uncoupled_Materials_Uncoupled_Prescribed_Transversely_Isotropic_Active_Contraction);

FEPrescribedActiveContractionTransIsoUC::FEPrescribedActiveContractionTransIsoUC() : FEMaterial(FE_ACTIVE_CONTRACT_TISO_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi"  , "phi" );
}

////////////////////////////////////////////////////////////////////////
// FEPrescribedActiveContractionIsotropicUC - Prescribed isotropic active contraction
////////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEPrescribedActiveContractionIsotropicUC, MODULE_MECH, FE_ACTIVE_CONTRACT_ISO_UC, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled prescribed isotropic active contraction", 0, Materials_Active_Contraction_Contraction_in_Mixtures_of_Uncoupled_Materials_Uncoupled_Prescribed_Isotropic_Active_Contraction);

FEPrescribedActiveContractionIsotropicUC::FEPrescribedActiveContractionIsotropicUC() : FEMaterial(FE_ACTIVE_CONTRACT_ISO_UC)
{
    AddScienceParam(0, UNIT_PRESSURE , "T0", "T0");
}

//////////////////////////////////////////////////////////////////////
// FEIsotropicFourier - Isotropic Fourier
//////////////////////////////////////////////////////////////////////

REGISTER_MATERIAL(FEIsotropicFourier, MODULE_HEAT, FE_ISOTROPIC_FOURIER, FE_MAT_HEAT_TRANSFER, "isotropic Fourier", MaterialFlags::TOPLEVEL);

FEIsotropicFourier::FEIsotropicFourier() : FEMaterial(FE_ISOTROPIC_FOURIER)
{
	AddScienceParam(1, UNIT_DENSITY, "density", "density");
	AddScienceParam(0, "W/L.T", "k"      , "heat conductivity");
	AddScienceParam(0, "E/T"  , "c"      , "heat capacity");
}

//=============================================================================
// Constant Permeability
//=============================================================================

REGISTER_MATERIAL(FEPermConst, MODULE_BIPHASIC, FE_PERM_CONST, FE_MAT_PERMEABILITY, "perm-const-iso", 0, Materials_Biphasic_Materials_Permeability_Materials_Constant_Isotropic_Permeability);

FEPermConst::FEPermConst() : FEMaterial(FE_PERM_CONST)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm", "permeability");
}

//=============================================================================
// Holmes-Mow Permeability
//=============================================================================

REGISTER_MATERIAL(FEPermHolmesMow, MODULE_BIPHASIC, FE_PERM_HOLMES_MOW, FE_MAT_PERMEABILITY, "perm-Holmes-Mow", 0, Materials_Biphasic_Materials_Permeability_Materials_Holmes_Mow);

FEPermHolmesMow::FEPermHolmesMow() : FEMaterial(FE_PERM_HOLMES_MOW)
{
	AddScienceParam(0, UNIT_PERMEABILITY, "perm" , "permeability");
	AddScienceParam(0, UNIT_NONE        , "M"    , "M");
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha");
}

//=============================================================================
// Ateshian-Weiss isotropic permeability
//=============================================================================

REGISTER_MATERIAL(FEPermAteshianWeissIso, MODULE_BIPHASIC, FE_PERM_REF_ISO, FE_MAT_PERMEABILITY, "perm-ref-iso", 0, Materials_Biphasic_Materials_Permeability_Materials_Referentially_Isotropic_Permeability);

FEPermAteshianWeissIso::FEPermAteshianWeissIso() : FEMaterial(FE_PERM_REF_ISO)
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

REGISTER_MATERIAL(FEPermAteshianWeissTransIso, MODULE_BIPHASIC, FE_PERM_REF_TRANS_ISO, FE_MAT_PERMEABILITY, "perm-ref-trans-iso", 0, Materials_Biphasic_Materials_Permeability_Materials_Referentially_Transversely_Isotropic_Permeability);

FEPermAteshianWeissTransIso::FEPermAteshianWeissTransIso() : FEMaterial(FE_PERM_REF_TRANS_ISO)
{
	m_hasMatAxes = true;

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
}

//=============================================================================
// Ateshian-Weiss orthotropic permeability
//=============================================================================

REGISTER_MATERIAL(FEPermAteshianWeissOrtho, MODULE_BIPHASIC, FE_PERM_REF_ORTHO, FE_MAT_PERMEABILITY, "perm-ref-ortho", 0, Materials_Biphasic_Materials_Permeability_Materials_Referentially_Orthotropic_Permeability);

FEPermAteshianWeissOrtho::FEPermAteshianWeissOrtho() : FEMaterial(FE_PERM_REF_ORTHO)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_PERMEABILITY, "perm0" , "perm0");
	AddVecParam(vec3d(0,0,0), "perm1" , "perm1")->SetUnit(UNIT_PERMEABILITY);
	AddVecParam(vec3d(0,0,0), "perm2" , "perm2")->SetUnit(UNIT_PERMEABILITY);
	AddScienceParam(0, UNIT_NONE        , "M0"    , "M0");
	AddScienceParam(0, UNIT_NONE        , "alpha0", "alpha0");
	AddVecParam(vec3d(0,0,0), "M"     , "M");
	AddVecParam(vec3d(0,0,0), "alpha" , "alpha");
}

//=============================================================================
// constant diffusivity
//=============================================================================

REGISTER_MATERIAL(FEDiffConst, MODULE_BIPHASIC, FE_DIFF_CONST, FE_MAT_DIFFUSIVITY, "diff-const-iso", 0, Materials_Biphasic_Solute_Materials_Diffusivity_Materials_Constant_Isotropic_Diffusivity);

FEDiffConst::FEDiffConst() : FEMaterial(FE_DIFF_CONST)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_DIFFUSIVITY, "diff"     , "diffusivity");
}

//=============================================================================
// orthotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FEDiffOrtho, MODULE_BIPHASIC, FE_DIFF_CONST_ORTHO, FE_MAT_DIFFUSIVITY, "diff-const-ortho", 0, Materials_Biphasic_Solute_Materials_Diffusivity_Materials_Constant_Orthotropic_Diffusivity);

FEDiffOrtho::FEDiffOrtho() : FEMaterial(FE_DIFF_CONST_ORTHO)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddVecParam(vec3d(0,0,0), "diff", "diffusivity")->SetUnit(UNIT_DIFFUSIVITY);
}

//=============================================================================
// Ateshian-Weiss isotropic diffusivity
//=============================================================================

REGISTER_MATERIAL(FEDiffAteshianWeissIso, MODULE_BIPHASIC, FE_DIFF_REF_ISO, FE_MAT_DIFFUSIVITY, "diff-ref-iso", 0, Materials_Biphasic_Solute_Materials_Diffusivity_Materials_Referentially_Isotropic_Diffusivity);

FEDiffAteshianWeissIso::FEDiffAteshianWeissIso() : FEMaterial(FE_DIFF_REF_ISO)
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

REGISTER_MATERIAL(FEDiffAlbroIso, MODULE_BIPHASIC, FE_DIFF_ALBRO_ISO, FE_MAT_DIFFUSIVITY, "diff-Albro-iso", 0, Materials_Biphasic_Solute_Materials_Diffusivity_Materials_Albro_Isotropic_Diffusivity);

FEDiffAlbroIso::FEDiffAlbroIso() : FEMaterial(FE_DIFF_ALBRO_ISO)
{
	AddScienceParam(0, UNIT_DIFFUSIVITY, "free_diff", "free diffusivity");
	AddScienceParam(0, UNIT_NONE       , "cdinv"    , "cdinv");
	AddScienceParam(0, UNIT_NONE       , "alphad"   , "alphad");
}

//=============================================================================
// constant solubility
//=============================================================================

REGISTER_MATERIAL(FESolubConst, MODULE_BIPHASIC, FE_SOLUB_CONST, FE_MAT_SOLUBILITY, "solub-const", 0, Materials_Biphasic_Solute_Materials_Solubility_Materials_Constant_Solubility);

FESolubConst::FESolubConst() : FEMaterial(FE_SOLUB_CONST)
{
	AddScienceParam(1, UNIT_NONE, "solub", "solubility");
}

//=============================================================================
// constant osmotic coefficient
//=============================================================================

REGISTER_MATERIAL(FEOsmoConst, MODULE_BIPHASIC, FE_OSMO_CONST, FE_MAT_OSMOTIC_COEFFICIENT, "osm-coef-const", 0, Materials_Biphasic_Solute_Materials_Osmotic_Coefficient_Materials_Constant_Osmotic_Coefficient);

FEOsmoConst::FEOsmoConst() : FEMaterial(FE_OSMO_CONST)
{
	AddScienceParam(1, UNIT_NONE, "osmcoef", "osmotic coefficient");
}

//=============================================================================
// SFD compressible
//=============================================================================

REGISTER_MATERIAL(FESFDCoupled, MODULE_MECH, FE_SFD_COUPLED, FE_MAT_ELASTIC, "spherical fiber distribution", 0, Materials_Elastic_Solids_Compressible_Materials_Spherical_Fiber_Distribution);

FESFDCoupled::FESFDCoupled() : FEMaterial(FE_SFD_COUPLED)
{
	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE        , "beta", "beta");
	AddScienceParam(0, UNIT_PRESSURE    , "ksi" , "ksi" );
}

//=============================================================================
// SFD SBM
//=============================================================================

REGISTER_MATERIAL(FESFDSBM, MODULE_MECH, FE_SFD_SBM, FE_MAT_ELASTIC, "spherical fiber distribution sbm", 0, Materials_Elastic_Solids_Compressible_Materials_Spherical_Fiber_Distribution_from_Solid_Bound_Molecule);

FESFDSBM::FESFDSBM() : FEMaterial(FE_SFD_SBM)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_NONE        , "alpha", "alpha" );
	AddScienceParam(0, UNIT_NONE        , "beta", "beta"   );
	AddScienceParam(0, UNIT_NONE        , "ksi0" , "ksi0"  );
	AddScienceParam(1, UNIT_NONE        , "rho0" , "rho0"  );
	AddScienceParam(0, UNIT_NONE        , "gamma" , "gamma");
	AddIntParam    (-1                   , "sbm"   , "sbm"  );
}

//=============================================================================
// EFD Coupled
//=============================================================================

REGISTER_MATERIAL(FEEFDCoupled, MODULE_MECH, FE_EFD_COUPLED, FE_MAT_ELASTIC, "ellipsoidal fiber distribution", 0, Materials_Elastic_Solids_Compressible_Materials_Ellipsoidal_Fiber_Distribution);

FEEFDCoupled::FEEFDCoupled() : FEMaterial(FE_EFD_COUPLED)
{
	m_hasMatAxes = true;

	AddVecParam(vec3d(0,0,0), "beta", "beta");
	AddVecParam(vec3d(0,0,0), "ksi" , "ksi" )->SetUnit(UNIT_PRESSURE);
}

//=============================================================================
// EFD Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEEFDUncoupled, MODULE_MECH, FE_EFD_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "EFD uncoupled", 0, Materials_Elastic_Solids_Uncoupled_Materials_Ellipsoidal_Fiber_Distribution);

FEEFDUncoupled::FEEFDUncoupled() : FEMaterial(FE_EFD_UNCOUPLED)
{
	m_hasMatAxes = true;

	AddVecParam(vec3d(0,0,0), "beta" , "beta");
	AddVecParam(vec3d(0,0,0), "ksi" , "ksi")->SetUnit(UNIT_PRESSURE);
	AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//=============================================================================
// Fiber-Exp-Pow
//=============================================================================

REGISTER_MATERIAL(FEFiberExpPow, MODULE_MECH, FE_FIBEREXPPOW_COUPLED, FE_MAT_ELASTIC, "fiber-exp-pow", 0, Materials_Elastic_Solids_Compressible_Materials_Fiber_with_Exponential_Power_Law);

FEFiberExpPow::FEFiberExpPow() : FEMaterial(FE_FIBEREXPPOW_COUPLED)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );

//	AddProperty("fiber", FE_MAT_FIBER_GENERATOR);
//	GetProperty(0).SetMaterial(new FEFiberGeneratorLocal);
}

//=============================================================================
// Fiber-Exp-Linear
//=============================================================================

REGISTER_MATERIAL(FEFiberExpLinear, MODULE_MECH, FE_FIBEREXPLIN_COUPLED, FE_MAT_ELASTIC, "fiber-exp-linear", 0);

FEFiberExpLinear::FEFiberExpLinear() : FEMaterial(FE_FIBEREXPLIN_COUPLED)
{
	m_hasMatAxes = true;

	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "lambda");
}

//=============================================================================
// Fiber-Exp-Linear uncoupled
//=============================================================================

REGISTER_MATERIAL(FEFiberExpLinearUncoupled, MODULE_MECH, FE_FIBEREXPLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "uncoupled fiber-exp-linear", 0);

FEFiberExpLinearUncoupled::FEFiberExpLinearUncoupled() : FEMaterial(FE_FIBEREXPLIN_UNCOUPLED)
{
	m_hasMatAxes = true;

	AddDoubleParam(0.0, "c3", "c3");
	AddDoubleParam(0.0, "c4", "c4");
	AddDoubleParam(0.0, "c5", "c5");
	AddDoubleParam(0.0, "lambda", "lambda");
}

//=============================================================================
// Fiber-Exp-Pow Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEFiberExpPowUncoupled, MODULE_MECH, FE_FIBEREXPPOW_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-exp-pow-uncoupled", 0, Materials_Elastic_Solids_Uncoupled_Materials_Fiber_with_Exponential_Power_Law_Uncoupled_Formulation);

FEFiberExpPowUncoupled::FEFiberExpPowUncoupled() : FEMaterial(FE_FIBEREXPPOW_UNCOUPLED)
{
	m_hasMatAxes = true;

	AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
	AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
	AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
	AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
	AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );
}

//=============================================================================
// Fiber-Pow-Linear
//=============================================================================

REGISTER_MATERIAL(FEFiberPowLin, MODULE_MECH, FE_FIBERPOWLIN_COUPLED, FE_MAT_ELASTIC, "fiber-pow-linear", 0, Materials_Elastic_Solids_Compressible_Materials_Fiber_with_Toe_Linear_Response);

FEFiberPowLin::FEFiberPowLin() : FEMaterial(FE_FIBERPOWLIN_COUPLED)
{
	m_hasMatAxes = true;

    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "beta");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );
}

//=============================================================================
// Fiber-Pow-Linear Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEFiberPowLinUncoupled, MODULE_MECH, FE_FIBERPOWLIN_UNCOUPLED, FE_MAT_ELASTIC_UNCOUPLED, "fiber-pow-linear-uncoupled", 0, Materials_Elastic_Solids_Uncoupled_Materials_Fiber_with_Toe_Linear_Response_Uncoupled_Formulation);

FEFiberPowLinUncoupled::FEFiberPowLinUncoupled() : FEMaterial(FE_FIBERPOWLIN_UNCOUPLED)
{
	m_hasMatAxes = true;

    AddScienceParam(0, UNIT_PRESSURE, "E", "E");
    AddScienceParam(2, UNIT_NONE, "beta" , "beta");
    AddScienceParam(1, UNIT_NONE, "lam0"  , "lam0");
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus")->SetPersistent(false);
    AddScienceParam(0, UNIT_DEGREE, "theta", "theta");
    AddScienceParam(0, UNIT_DEGREE, "phi"  , "phi"  );
}

//=============================================================================
// Donnan swelling
//=============================================================================

REGISTER_MATERIAL(FEDonnanSwelling, MODULE_MECH, FE_DONNAN_SWELLING, FE_MAT_ELASTIC, "Donnan equilibrium", 0, Materials_Elastic_Solids_Compressible_Materials_Donnan_Equilibrium_Swelling);

FEDonnanSwelling::FEDonnanSwelling() : FEMaterial(FE_DONNAN_SWELLING)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "cF0", "cF0");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
    AddScienceParam(0, UNIT_NONE, "Phi", "Phi");
}

//=============================================================================
// Perfect Osmometer
//=============================================================================

REGISTER_MATERIAL(FEPerfectOsmometer, MODULE_MECH, FE_PERFECT_OSMOMETER, FE_MAT_ELASTIC, "perfect osmometer", 0, Materials_Elastic_Solids_Compressible_Materials_Perfect_Osmometer_Equilibrium_Osmotic_Pressure);

FEPerfectOsmometer::FEPerfectOsmometer() : FEMaterial(FE_PERFECT_OSMOMETER)
{
	AddScienceParam(0, UNIT_NONE, "phiw0", "phiw0");
	AddScienceParam(0, UNIT_CONCENTRATION, "iosm", "iosm");
	AddScienceParam(0, UNIT_CONCENTRATION, "bosm", "bosm");
}

//=============================================================================
// Cell Growth
//=============================================================================

REGISTER_MATERIAL(FECellGrowth, MODULE_MECH, FE_CELL_GROWTH, FE_MAT_ELASTIC, "cell growth", 0, Materials_Elastic_Solids_Compressible_Materials_Cell_Growth);

FECellGrowth::FECellGrowth() : FEMaterial(FE_CELL_GROWTH)
{
	AddScienceParam(0, UNIT_NONE, "phir", "phir");
	AddScienceParam(0, UNIT_CONCENTRATION, "cr", "cr");
	AddScienceParam(0, UNIT_CONCENTRATION, "ce", "ce");
}

//=============================================================================
// Osmotic pressure using virial coefficients
//=============================================================================

REGISTER_MATERIAL(FEOsmoticVirial, MODULE_MECH, FE_OSMOTIC_VIRIAL, FE_MAT_ELASTIC, "osmotic virial expansion", 0, Materials_Elastic_Solids_Compressible_Materials_Osmotic_Pressure_from_Virial_Expansion);

FEOsmoticVirial::FEOsmoticVirial() : FEMaterial(FE_OSMOTIC_VIRIAL)
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

REGISTER_MATERIAL(FEReactionRateConst, MODULE_REACTIONS, FE_REACTION_RATE_CONST, FE_MAT_REACTION_RATE, "constant reaction rate", 0, Materials_Chemical_Reactions_Specific_Reaction_Rate_Materials_Constant_Reaction_Rate);

FEReactionRateConst::FEReactionRateConst() : FEMaterial(FE_REACTION_RATE_CONST)
{
	AddDoubleParam(0, "k", "k");
}

double FEReactionRateConst::GetRateConstant() { return GetParam(0).GetFloatValue(); }

void FEReactionRateConst::SetRateConstant(double K) { SetFloatValue(0, K); }

//=============================================================================
// Huiskes reaction rate
//=============================================================================

REGISTER_MATERIAL(FEReactionRateHuiskes, MODULE_REACTIONS, FE_REACTION_RATE_HUISKES, FE_MAT_REACTION_RATE, "Huiskes reaction rate", 0, Materials_Chemical_Reactions_Specific_Reaction_Rate_Materials_Huiskes_Reaction_Rate);

FEReactionRateHuiskes::FEReactionRateHuiskes() : FEMaterial(FE_REACTION_RATE_HUISKES)
{
	AddDoubleParam(0, "B", "B");
	AddDoubleParam(0, "psi0", "psi0");
}

//=============================================================================
// CFD Fiber-Exponential-Power-Law
//=============================================================================

REGISTER_MATERIAL(FECFDFiberExpPow, MODULE_MECH, FE_FIBER_EXP_POW, FE_MAT_CFD_FIBER, "fiber-exponential-power-law", 0, Materials_Continuous_Fiber_Distribution_Fibers_Fiber_with_Exponential_Power_Law);

FECFDFiberExpPow::FECFDFiberExpPow() : FEMaterial(FE_FIBER_EXP_POW)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
}

//=============================================================================
// CFD Fiber-neo-Hookean
//=============================================================================

REGISTER_MATERIAL(FECFDFiberNH, MODULE_MECH, FE_FIBER_NH, FE_MAT_CFD_FIBER, "fiber-NH", 0, Materials_Continuous_Fiber_Distribution_Fibers_Fiber_with_Neo_Hookean_Law);

FECFDFiberNH::FECFDFiberNH() : FEMaterial(FE_FIBER_NH)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu");
}

//=============================================================================
// CFD Fiber-Exponential-Power-Law uncoupled
//=============================================================================

REGISTER_MATERIAL(FECFDFiberExpPowUC, MODULE_MECH, FE_FIBER_EXP_POW_UC, FE_MAT_CFD_FIBER_UC, "fiber-exponential-power-law-uncoupled", 0, Materials_Continuous_Fiber_Distribution_Fibers_Fiber_with_Exponential_Power_Law_Uncoupled);

FECFDFiberExpPowUC::FECFDFiberExpPowUC() : FEMaterial(FE_FIBER_EXP_POW_UC)
{
    AddScienceParam(0, UNIT_NONE, "alpha", "alpha");
    AddScienceParam(0, UNIT_NONE, "beta" , "beta" );
    AddScienceParam(0, UNIT_PRESSURE, "ksi"  , "ksi"  );
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//=============================================================================
// CFD Fiber-neo-Hookean uncoupled
//=============================================================================

REGISTER_MATERIAL(FECFDFiberNHUC, MODULE_MECH, FE_FIBER_NH_UC, FE_MAT_CFD_FIBER_UC, "fiber-NH-uncoupled", 0, Materials_Continuous_Fiber_Distribution_Fibers_Fiber_with_Neo_Hookean_Law_Uncoupled);

FECFDFiberNHUC::FECFDFiberNHUC() : FEMaterial(FE_FIBER_NH_UC)
{
    AddScienceParam(0, UNIT_PRESSURE, "mu"   , "mu"   );
    AddScienceParam(0, UNIT_PRESSURE, "k", "bulk modulus");
}

//=============================================================================
// FDD Spherical
//=============================================================================

REGISTER_MATERIAL(FEFDDSpherical, MODULE_MECH, FE_DSTRB_SFD, FE_MAT_CFD_DIST, "spherical", 0, Materials_Continuous_Fiber_Distribution_Distribution_Spherical);

FEFDDSpherical::FEFDDSpherical() : FEMaterial(FE_DSTRB_SFD)
{
}

//=============================================================================
// FDD Ellipsoidal
//=============================================================================

REGISTER_MATERIAL(FEFDDEllipsoidal, MODULE_MECH, FE_DSTRB_EFD, FE_MAT_CFD_DIST, "ellipsoidal", 0, Materials_Continuous_Fiber_Distribution_Distribution_Ellipsoidal);

FEFDDEllipsoidal::FEFDDEllipsoidal() : FEMaterial(FE_DSTRB_EFD)
{
    AddVecParam(vec3d(1,1,1), "spa" , "spa");
}

//=============================================================================
// FDD von Mises 3d
//=============================================================================

REGISTER_MATERIAL(FEFDDvonMises3d, MODULE_MECH, FE_DSTRB_VM3, FE_MAT_CFD_DIST, "von-Mises-3d", 0);

FEFDDvonMises3d::FEFDDvonMises3d() : FEMaterial(FE_DSTRB_VM3)
{
    AddDoubleParam(0, "b"   , "b");
}

//=============================================================================
// FDD Circular
//=============================================================================

REGISTER_MATERIAL(FEFDDCircular, MODULE_MECH, FE_DSTRB_CFD, FE_MAT_CFD_DIST, "circular", 0, Materials_Continuous_Fiber_Distribution_Distribution_Circular);

FEFDDCircular::FEFDDCircular() : FEMaterial(FE_DSTRB_CFD)
{
}

//=============================================================================
// FDD Elliptical
//=============================================================================

REGISTER_MATERIAL(FEFDDElliptical, MODULE_MECH, FE_DSTRB_PFD, FE_MAT_CFD_DIST, "elliptical", 0, Materials_Continuous_Fiber_Distribution_Distribution_Elliptical);

FEFDDElliptical::FEFDDElliptical() : FEMaterial(FE_DSTRB_PFD)
{
    AddScienceParam(0, UNIT_NONE, "spa1"   , "spa1");
    AddScienceParam(0, UNIT_NONE, "spa2"   , "spa2");
}

//=============================================================================
// FDD von Mises 2d
//=============================================================================

REGISTER_MATERIAL(FEFDDvonMises2d, MODULE_MECH, FE_DSTRB_VM2, FE_MAT_CFD_DIST, "von-Mises-2d", 0, Materials_Continuous_Fiber_Distribution_Distribution_von_Mises_Distribution);

FEFDDvonMises2d::FEFDDvonMises2d() : FEMaterial(FE_DSTRB_VM2)
{
    AddScienceParam(0, UNIT_NONE, "b"   , "b");
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal
//=============================================================================

REGISTER_MATERIAL(FESchemeGKT, MODULE_MECH, FE_SCHM_GKT, FE_MAT_CFD_SCHEME, "fibers-3d-gkt", 0, Materials_Continuous_Fiber_Distribution_Scheme_Gauss_Kronrod_Trapezoidal_Rule);

FESchemeGKT::FESchemeGKT() : FEMaterial(FE_SCHM_GKT)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");// enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration
//=============================================================================

REGISTER_MATERIAL(FESchemeFEI, MODULE_MECH, FE_SCHM_FEI, FE_MAT_CFD_SCHEME, "fibers-3d-fei", 0, Materials_Continuous_Fiber_Distribution_Scheme_Finite_Element_Integration_Rule);

FESchemeFEI::FESchemeFEI() : FEMaterial(FE_SCHM_FEI)
{
    AddIntParam(1796, "resolution"   , "resolution");// choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d
//=============================================================================

REGISTER_MATERIAL(FESchemeT2d, MODULE_MECH, FE_SCHM_T2D, FE_MAT_CFD_SCHEME, "fibers-2d-trapezoidal", 0, Materials_Continuous_Fiber_Distribution_Scheme_Trapezoidal_Rule);

FESchemeT2d::FESchemeT2d() : FEMaterial(FE_SCHM_T2D)
{
    AddIntParam(31, "nth"   , "nth");// odd and >= 3
}

//=============================================================================
// Scheme Gauss-Kronrod Trapezoidal uncoupled
//=============================================================================

REGISTER_MATERIAL(FESchemeGKTUC, MODULE_MECH, FE_SCHM_GKT_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-gkt-uncoupled", 0, Materials_Continuous_Fiber_Distribution_Scheme_Gauss_Kronrod_Trapezoidal_Rule);

FESchemeGKTUC::FESchemeGKTUC() : FEMaterial(FE_SCHM_GKT_UC)
{
    AddIntParam(11, "nph"   , "nph");// choose from 7, 11, 15, 19, 23, or 27
    AddIntParam(31, "nth"   , "nth");//  enter odd value >= 3
}

//=============================================================================
// Scheme Finite Element Integration uncoupled
//=============================================================================

REGISTER_MATERIAL(FESchemeFEIUC, MODULE_MECH, FE_SCHM_FEI_UC, FE_MAT_CFD_SCHEME_UC, "fibers-3d-fei-uncoupled", 0, Materials_Continuous_Fiber_Distribution_Scheme_Finite_Element_Integration_Rule);

FESchemeFEIUC::FESchemeFEIUC() : FEMaterial(FE_SCHM_FEI_UC)
{
    AddIntParam(11, "resolution"   , "resolution"); // choose from 20, 34, 60, 74, 196, 210, 396, 410, ..., 1596, 1610, 1796
}

//=============================================================================
// Scheme Trapezoidal 2d uncoupled
//=============================================================================

REGISTER_MATERIAL(FESchemeT2dUC, MODULE_MECH, FE_SCHM_T2D_UC, FE_MAT_CFD_SCHEME_UC, "fibers-2d-trapezoidal-uncoupled", 0, Materials_Continuous_Fiber_Distribution_Scheme_Trapezoidal_Rule);

FESchemeT2dUC::FESchemeT2dUC() : FEMaterial(FE_SCHM_T2D_UC)
{
    AddIntParam(31, "nth"   , "nth"); // nth (odd and >= 3)
}

//=============================================================================
// CDF Simo
//=============================================================================

REGISTER_MATERIAL(FECDFSimo, MODULE_MECH, FE_CDF_SIMO, FE_MAT_DAMAGE, "CDF Simo", 0, Materials_Reactive_Damage_Mechanics_Cumulative_Distribution_Functions_Simo);

FECDFSimo::FECDFSimo() : FEMaterial(FE_CDF_SIMO)
{
    AddDoubleParam(0, "a" , "a"); // a must be ≥ 0
    AddScienceParam(0, UNIT_NONE, "b" , "b");
}

//=============================================================================
// CDF Log Normal
//=============================================================================

REGISTER_MATERIAL(FECDFLogNormal, MODULE_MECH, FE_CDF_LOG_NORMAL, FE_MAT_DAMAGE, "CDF log-normal", 0, Materials_Reactive_Damage_Mechanics_Cumulative_Distribution_Functions_Log_Normal);

FECDFLogNormal::FECDFLogNormal() : FEMaterial(FE_CDF_LOG_NORMAL)
{
    AddDoubleParam(0, "mu" , "mu"); // mu must be > 0
    AddScienceParam(0, UNIT_NONE, "sigma" , "sigma"); // sigma must be > 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Weibull
//=============================================================================

REGISTER_MATERIAL(FECDFWeibull, MODULE_MECH, FE_CDF_WEIBULL, FE_MAT_DAMAGE, "CDF Weibull", 0, Materials_Reactive_Damage_Mechanics_Cumulative_Distribution_Functions_Weibull);

FECDFWeibull::FECDFWeibull() : FEMaterial(FE_CDF_WEIBULL)
{
    AddDoubleParam(0, "mu" , "mu"); // mu must be > 0
    AddScienceParam(0, UNIT_NONE, "alpha" , "alpha"); // alpha must be ≥ 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Step
//=============================================================================

REGISTER_MATERIAL(FECDFStep, MODULE_MECH, FE_CDF_STEP, FE_MAT_DAMAGE, "CDF step", 0, Materials_Reactive_Damage_Mechanics_Cumulative_Distribution_Functions_Step);

FECDFStep::FECDFStep() : FEMaterial(FE_CDF_STEP)
{
    AddDoubleParam(0, "mu" , "mu" ); //  mu must be > 0
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax"); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// CDF Quintic
//=============================================================================

REGISTER_MATERIAL(FECDFQuintic, MODULE_MECH, FE_CDF_QUINTIC, FE_MAT_DAMAGE, "CDF quintic", 0, Materials_Reactive_Damage_Mechanics_Cumulative_Distribution_Functions_Quintic_Polynomial);

FECDFQuintic::FECDFQuintic() : FEMaterial(FE_CDF_QUINTIC)
{
    AddDoubleParam(0, "mumin" , "mumin"); // mumin must be > 0
    AddDoubleParam(0, "mumax" , "mumax"); // mumax must be > mumin
    AddScienceParam(1, UNIT_NONE, "Dmax" , "Dmax" ); // Maximum allowable damage (0 ≤ Dmax ≤ 1)
}

//=============================================================================
// DC Simo
//=============================================================================

REGISTER_MATERIAL(FEDCSimo, MODULE_MECH, FE_DC_SIMO, FE_MAT_DAMAGE_CRITERION, "DC Simo", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Simo);

FEDCSimo::FEDCSimo() : FEMaterial(FE_DC_SIMO)
{
}

//=============================================================================
// DC Strain Energy Density
//=============================================================================

REGISTER_MATERIAL(FEDCStrainEnergyDensity, MODULE_MECH, FE_DC_SED, FE_MAT_DAMAGE_CRITERION, "DC strain energy density", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Strain_Energy_Density);

FEDCStrainEnergyDensity::FEDCStrainEnergyDensity() : FEMaterial(FE_DC_SED)
{
}

//=============================================================================
// DC Specific Strain Energy
//=============================================================================

REGISTER_MATERIAL(FEDCSpecificStrainEnergy, MODULE_MECH, FE_DC_SSE, FE_MAT_DAMAGE_CRITERION, "DC specific strain energy", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Specific_Strain_Energy);

FEDCSpecificStrainEnergy::FEDCSpecificStrainEnergy() : FEMaterial(FE_DC_SSE)
{
}

//=============================================================================
// DC von Mises Stress
//=============================================================================

REGISTER_MATERIAL(FEDCvonMisesStress, MODULE_MECH, FE_DC_VMS, FE_MAT_DAMAGE_CRITERION, "DC von Mises stress", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Von_Mises_Stress);

FEDCvonMisesStress::FEDCvonMisesStress() : FEMaterial(FE_DC_VMS)
{
}

//=============================================================================
// DC Maximum Shear Stress
//=============================================================================

REGISTER_MATERIAL(FEDCMaxShearStress, MODULE_MECH, FE_DC_MSS, FE_MAT_DAMAGE_CRITERION, "DC max shear stress", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Maximum_Shear_Stress);

FEDCMaxShearStress::FEDCMaxShearStress() : FEMaterial(FE_DC_MSS)
{
}

//=============================================================================
// DC Maximum Normal Stress
//=============================================================================

REGISTER_MATERIAL(FEDCMaxNormalStress, MODULE_MECH, FE_DC_MNS, FE_MAT_DAMAGE_CRITERION, "DC max normal stress", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Maximum_Normal_Stress);

FEDCMaxNormalStress::FEDCMaxNormalStress() : FEMaterial(FE_DC_MNS)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain
//=============================================================================

REGISTER_MATERIAL(FEDCMaxNormalLagrangeStrain, MODULE_MECH, FE_DC_MNLE, FE_MAT_DAMAGE_CRITERION, "DC max normal Lagrange strain", 0, Materials_Reactive_Damage_Mechanics_Damage_Criterion_Maximum_Normal_Lagrange_Strain);

FEDCMaxNormalLagrangeStrain::FEDCMaxNormalLagrangeStrain() : FEMaterial(FE_DC_MNLE)
{
}

//=============================================================================
// DC Simo Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCSimoUC, MODULE_MECH, FE_DC_SIMO_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC Simo uncoupled", 0);

FEDCSimoUC::FEDCSimoUC() : FEMaterial(FE_DC_SIMO_UC)
{
}

//=============================================================================
// DC Strain Energy Density Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCStrainEnergyDensityUC, MODULE_MECH, FE_DC_SED_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC strain energy density uncoupled", 0);

FEDCStrainEnergyDensityUC::FEDCStrainEnergyDensityUC() : FEMaterial(FE_DC_SED_UC)
{
}

//=============================================================================
// DC Specific Strain Energy Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCSpecificStrainEnergyUC, MODULE_MECH, FE_DC_SSE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC specific strain energy uncoupled", 0);

FEDCSpecificStrainEnergyUC::FEDCSpecificStrainEnergyUC() : FEMaterial(FE_DC_SSE_UC)
{
}

//=============================================================================
// DC von Mises Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCvonMisesStressUC, MODULE_MECH, FE_DC_VMS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC von Mises stress uncoupled", 0);

FEDCvonMisesStressUC::FEDCvonMisesStressUC() : FEMaterial(FE_DC_VMS_UC)
{
}

//=============================================================================
// DC Maximum Shear Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCMaxShearStressUC, MODULE_MECH, FE_DC_MSS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max shear stress uncoupled", 0);

FEDCMaxShearStressUC::FEDCMaxShearStressUC() : FEMaterial(FE_DC_MSS_UC)
{
}

//=============================================================================
// DC Maximum Normal Stress Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCMaxNormalStressUC, MODULE_MECH, FE_DC_MNS_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal stress uncoupled", 0);

FEDCMaxNormalStressUC::FEDCMaxNormalStressUC() : FEMaterial(FE_DC_MNS_UC)
{
}

//=============================================================================
// DC Maximum Normal Lagrange Strain Uncoupled
//=============================================================================

REGISTER_MATERIAL(FEDCMaxNormalLagrangeStrainUC, MODULE_MECH, FE_DC_MNLE_UC, FE_MAT_DAMAGE_CRITERION_UC, "DC max normal Lagrange strain uncoupled", 0);

FEDCMaxNormalLagrangeStrainUC::FEDCMaxNormalLagrangeStrainUC() : FEMaterial(FE_DC_MNLE_UC)
{
}

//=============================================================================
// Relaxation Exponential
//=============================================================================

REGISTER_MATERIAL(FERelaxExp, MODULE_MECH, FE_RELAX_EXP, FE_MAT_RV_RELAX, "relaxation-exponential", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Exponential);

FERelaxExp::FERelaxExp() : FEMaterial(FE_RELAX_EXP)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "tau"); // characteristic relaxation time
}

//=============================================================================
// Relaxation Exponential Distortion
//=============================================================================

REGISTER_MATERIAL(FERelaxExpDistortion, MODULE_MECH, FE_RELAX_EXP_DIST, FE_MAT_RV_RELAX, "relaxation-exp-distortion", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Exponential_Distortional);

FERelaxExpDistortion::FERelaxExpDistortion() : FEMaterial(FE_RELAX_EXP_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "tau0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "tau1" );
    AddScienceParam(0, UNIT_NONE, "alpha" , "alpha");
}

//=============================================================================
// Relaxation Fung
//=============================================================================

REGISTER_MATERIAL(FERelaxFung, MODULE_MECH, FE_RELAX_FUNG, FE_MAT_RV_RELAX, "relaxation-Fung", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Fung);

FERelaxFung::FERelaxFung() : FEMaterial(FE_RELAX_FUNG)
{
    AddScienceParam(0, UNIT_TIME, "tau1"   , "tau1"); //  minimum characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau2"   , "tau2"); // maximum characteristic relaxation time
}

//=============================================================================
// Relaxation Park
//=============================================================================

REGISTER_MATERIAL(FERelaxPark, MODULE_MECH, FE_RELAX_PARK, FE_MAT_RV_RELAX, "relaxation-Park", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Park);

FERelaxPark::FERelaxPark() : FEMaterial(FE_RELAX_PARK)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "tau" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "beta"); // exponent
}

//=============================================================================
// Relaxation Park Distortion
//=============================================================================

REGISTER_MATERIAL(FERelaxParkDistortion, MODULE_MECH, FE_RELAX_PARK_DIST, FE_MAT_RV_RELAX, "relaxation-Park-distortion", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Park_Distortional);

FERelaxParkDistortion::FERelaxParkDistortion() : FEMaterial(FE_RELAX_PARK_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "tau0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "tau1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "beta0"); // exponent
    AddScienceParam(0, UNIT_NONE, "beta1" , "beta1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "alpha");
}

//=============================================================================
// Relaxation power
//=============================================================================

REGISTER_MATERIAL(FERelaxPow, MODULE_MECH, FE_RELAX_POW, FE_MAT_RV_RELAX, "relaxation-power", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Power);

FERelaxPow::FERelaxPow() : FEMaterial(FE_RELAX_POW)
{
    AddScienceParam(0, UNIT_TIME, "tau"   , "tau" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_NONE, "beta"  , "beta"); // exponent
}

//=============================================================================
// Relaxation power distortion
//=============================================================================

REGISTER_MATERIAL(FERelaxPowDistortion, MODULE_MECH, FE_RELAX_POW_DIST, FE_MAT_RV_RELAX, "relaxation-power-distortion", 0, Materials_Reactive_Viscoelastic_Solid_Relaxation_Functions_Power_Distortional);

FERelaxPowDistortion::FERelaxPowDistortion() : FEMaterial(FE_RELAX_POW_DIST)
{
    AddScienceParam(0, UNIT_TIME, "tau0"  , "tau0" ); // characteristic relaxation time
    AddScienceParam(0, UNIT_TIME, "tau1"  , "tau1" );
    AddScienceParam(0, UNIT_NONE, "beta0" , "beta0");
    AddScienceParam(0, UNIT_NONE, "beta1" , "beta1");
    AddScienceParam(0, UNIT_NONE, "alpha" , "alpha");
}

//=============================================================================
// Elastic pressure for ideal gas
//=============================================================================

REGISTER_MATERIAL(FEEPIdealGas, MODULE_FLUID, FE_EP_IDEAL_GAS, FE_MAT_FLUID_ELASTIC, "ideal gas", 0);

FEEPIdealGas::FEEPIdealGas() : FEMaterial(FE_EP_IDEAL_GAS)
{
    AddScienceParam(0, UNIT_MOLAR_MASS, "molar_mass"  , "molar_mass");
}

//=============================================================================
// Elastic pressure for ideal fluid
//=============================================================================

REGISTER_MATERIAL(FEEPIdealFluid, MODULE_FLUID, FE_EP_IDEAL_FLUID, FE_MAT_FLUID_ELASTIC, "ideal fluid", 0);

FEEPIdealFluid::FEEPIdealFluid() : FEMaterial(FE_EP_IDEAL_FLUID)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Elastic pressure for neo-Hookean fluid
//=============================================================================

REGISTER_MATERIAL(FEEPNeoHookeanFluid, MODULE_FLUID, FE_EP_NEOHOOKEAN_FLUID, FE_MAT_FLUID_ELASTIC, "neo-Hookean fluid", 0);

FEEPNeoHookeanFluid::FEEPNeoHookeanFluid() : FEMaterial(FE_EP_NEOHOOKEAN_FLUID)
{
    AddScienceParam(0, UNIT_PRESSURE, "k"  , "Bulk modulus");
}

//=============================================================================
// Viscous Newtonian fluid
//=============================================================================

REGISTER_MATERIAL(FEVFNewtonian, MODULE_FLUID, FE_VF_NEWTONIAN, FE_MAT_FLUID_VISCOSITY, "Newtonian fluid", 0, Materials_Viscous_Fluids_Viscous_Fluid_Materials_Newtonian_Fluid);

FEVFNewtonian::FEVFNewtonian() : FEMaterial(FE_VF_NEWTONIAN)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu"  , "shear viscosity");
    AddScienceParam(0, UNIT_VISCOSITY, "kappa", "bulk viscosity");
}

//=============================================================================
// Viscous Carreau fluid
//=============================================================================

REGISTER_MATERIAL(FEVFCarreau, MODULE_FLUID, FE_VF_CARREAU, FE_MAT_FLUID_VISCOSITY, "Carreau", 0, Materials_Viscous_Fluids_Viscous_Fluid_Materials_Carreau_Model);

FEVFCarreau::FEVFCarreau() : FEMaterial(FE_VF_CARREAU)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "mu0"); // viscosity at zero shear rate
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "mui"); // viscosity at infinite shear rate
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index"  );
}

//=============================================================================
// Viscous Carreau-Yasuda fluid
//=============================================================================

REGISTER_MATERIAL(FEVFCarreauYasuda, MODULE_FLUID, FE_VF_CARREAU_YASUDA, FE_MAT_FLUID_VISCOSITY, "Carreau-Yasuda", 0, Materials_Viscous_Fluids_Viscous_Fluid_Materials_Carreau_Yasuda_Model);

FEVFCarreauYasuda::FEVFCarreauYasuda() : FEMaterial(FE_VF_CARREAU_YASUDA)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "viscosity at zero shear rate"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "viscosity at infinite shear rate"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "n" , "power index"  );
    AddScienceParam(0, UNIT_NONE, "a" , "power denominator"  );
}

//=============================================================================
// Viscous Powell-Eyring fluid
//=============================================================================

REGISTER_MATERIAL(FEVFPowellEyring, MODULE_FLUID, FE_VF_POWELL_EYRING, FE_MAT_FLUID_VISCOSITY, "Powell-Eyring", 0, Materials_Viscous_Fluids_Viscous_Fluid_Materials_Powell_Eyring_Model);

FEVFPowellEyring::FEVFPowellEyring() : FEMaterial(FE_VF_POWELL_EYRING)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "viscosity at zero shear rate"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "viscosity at infinite shear rate"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
}

//=============================================================================
// Viscous Cross fluid
//=============================================================================

REGISTER_MATERIAL(FEVFCross, MODULE_FLUID, FE_VF_CROSS, FE_MAT_FLUID_VISCOSITY, "Cross", 0, Materials_Viscous_Fluids_Viscous_Fluid_Materials_Cross_Model);

FEVFCross::FEVFCross() : FEMaterial(FE_VF_CROSS)
{
    AddScienceParam(0, UNIT_VISCOSITY, "mu0" , "viscosity at zero shear rate"  );
    AddScienceParam(0, UNIT_VISCOSITY, "mui" , "viscosity at infinite shear rate"  );
    AddScienceParam(0, UNIT_TIME, "lambda" , "relaxation time"  );
    AddScienceParam(0, UNIT_NONE, "m" , "power"  );
}

//=============================================================================
// Starling solvent supply
//=============================================================================

REGISTER_MATERIAL(FEStarlingSupply, MODULE_MULTIPHASIC, FE_STARLING_SUPPLY, FE_MAT_SOLVENT_SUPPLY, "Starling", 0, Materials_Triphasic_and_Multiphasic_Materials_Solvent_Supply_Materials_Starling_Equation);

FEStarlingSupply::FEStarlingSupply() : FEMaterial(FE_STARLING_SUPPLY)
{
	AddScienceParam(0, UNIT_FILTRATION, "kp", "filtration coefficient");
	AddScienceParam(0, UNIT_PRESSURE, "pv", "external pressure");
}

//=============================================================================
// const prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FEPrestrainConstGradient, MODULE_MECH, FE_PRESTRAIN_CONST_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "prestrain gradient", 0);

FEPrestrainConstGradient::FEPrestrainConstGradient() : FEMaterial(FE_PRESTRAIN_CONST_GRADIENT)
{
	mat3d F0; F0.unit();
	AddMat3dParam(F0, "F0", "prestrain gradient");
}

//=============================================================================
// in-situ stretch prestrain gradient
//=============================================================================

REGISTER_MATERIAL(FEPrestrainInSituGradient, MODULE_MECH, FE_PRESTRAIN_INSITU_GRADIENT, FE_MAT_PRESTRAIN_GRADIENT, "in-situ stretch", 0);

FEPrestrainInSituGradient::FEPrestrainInSituGradient() : FEMaterial(FE_PRESTRAIN_INSITU_GRADIENT)
{
	AddScienceParam(1.0, UNIT_NONE, "stretch", "fiber stretch");
	AddBoolParam(false, "isochoric", "isochoric prestrain");
}
