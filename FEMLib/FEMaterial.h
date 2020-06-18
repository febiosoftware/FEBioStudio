#pragma once
#include <FSCore/LoadCurve.h>
#include "FECoreMaterial.h"
#include "FEMaterialFactory.h"

// --- Material Classes ---
// These values are not stored in the prv file, so they can be changed
// Top level classes.
// Note that these are one bit values!
#define FE_MAT_ELASTIC				0x0001
#define FE_MAT_ELASTIC_UNCOUPLED	0x0002
#define FE_MAT_MULTIPHASIC			0x0004
#define FE_MAT_HEAT_TRANSFER		0x0008
#define FE_MAT_FLUID				0x0010
#define FE_MAT_REACTION_DIFFUSION	0x0020
#define FE_MAT_RIGID				0x0040
#define FE_MAT_FLUID_FSI            0x0080
#define FE_MAT_DISCRETE				0x0100
#define FE_MAT_1DFUNC				0x0200

// component classes
// These values must not contain the top level class values in the lower bits!
#define FE_MAT_PERMEABILITY				0x00010000
#define FE_MAT_DIFFUSIVITY				0x00020000
#define FE_MAT_SOLUBILITY				0x00030000
#define FE_MAT_OSMOTIC_COEFFICIENT		0x00040000
#define FE_MAT_SOLUTE					0x00050000
#define FE_MAT_SBM						0x00060000
#define FE_MAT_REACTION					0x00070000
#define FE_MAT_DAMAGE					0x00080000
#define FE_MAT_DAMAGE_CRITERION			0x00090000
#define FE_MAT_DAMAGE_CRITERION_UC		0x000A0000
#define FE_MAT_FLUID_ELASTIC			0x000B0000
#define FE_MAT_FLUID_VISCOSITY			0x000C0000
#define FE_MAT_CFD_FIBER				0x000D0000
#define FE_MAT_CFD_DIST					0x000E0000
#define FE_MAT_CFD_SCHEME				0x000F0000
#define FE_MAT_CFD_FIBER_UC				0x00100000
#define FE_MAT_CFD_SCHEME_UC			0x00110000	// This is obsolete. Only retained for backward compatibility
#define FE_MAT_RV_RELAX					0x00120000
#define FE_MAT_SOLVENT_SUPPLY			0x00130000
#define FE_MAT_REACTION_RATE			0x00140000
#define FE_MAT_REACTION_REACTANTS		0x00150000
#define FE_MAT_REACTION_PRODUCTS		0x00160000
#define FE_MAT_SPECIES					0x00170000
#define FE_MAT_SOLID_SPECIES			0x00180000
#define FE_MAT_ACTIVE_CONTRACTION_CLASS 0x00190000
#define FE_MAT_GENERATION				0x001A0000
#define FE_MAT_PRESTRAIN_GRADIENT		0x001B0000
#define FE_MAT_FIBER_GENERATOR			0x001C0000

// --- Material Types ---
// These values are stored in the prv file so don't change!
//
#define FE_NEO_HOOKEAN					1
#define FE_MOONEY_RIVLIN				2
#define FE_VERONDA_WESTMANN				3
#define FE_TRANS_MOONEY_RIVLIN_OLD		4	// ---> obsolete (2.0)
#define FE_TRANS_VERONDA_WESTMANN_OLD	5	// --->obsolete (2.0)
#define FE_RIGID_BODY					6	//---> obsolete (1.4?)
#define FE_ISOTROPIC_ELASTIC			7
//#define ST_VENANT_KIRCHHOFF			8
#define FE_PORO_ELASTIC_OLD				9	//---> obsolete (1.5)
#define FE_TCNL_ORTHO					10
#define FE_MUSCLE_MATERIAL				11
#define FE_TENDON_MATERIAL				12
#define FE_VISCO_ELASTIC_OLD			13	//---> obsolete (1.5)
#define FE_OGDEN_MATERIAL				14
//#define FE_NIKE_MATERIAL				15	//---> obsolete (1.5)
#define FE_EFD_MOONEY_RIVLIN			16
#define FE_EFD_NEO_HOOKEAN				17
#define FE_PORO_HOLMES_MOW_OLD			18	//---> obsolete (1.5)
#define FE_HOLMES_MOW					19
#define FE_ARRUDA_BOYCE					20
#define FE_RIGID_MATERIAL				21
#define FE_FUNG_ORTHO					22
#define FE_LINEAR_ORTHO					23
#define FE_EFD_DONNAN					24
#define FE_ISOTROPIC_FOURIER			25
#define FE_EFD_VERONDA_WESTMANN			26
#define FE_ORTHO_ELASTIC				27
#define FE_OGDEN_UNCONSTRAINED			28
#define FE_EFD_COUPLED					29
#define FE_EFD_UNCOUPLED				30
#define FE_FIBEREXPPOW_COUPLED_OLD      31
#define FE_FIBEREXPPOW_UNCOUPLED_OLD    32
#define FE_FIBERPOWLIN_COUPLED_OLD      33
#define FE_FIBERPOWLIN_UNCOUPLED_OLD    34
#define FE_DONNAN_SWELLING				35
#define FE_PERFECT_OSMOMETER			36
#define FE_CELL_GROWTH					37
#define FE_SFD_COUPLED					38
#define FE_CARTER_HAYES					39
#define FE_SFD_SBM						40
#define FE_FUNG_ORTHO_COUPLED			41
#define FE_PRLIG						42
#define FE_OSMOTIC_VIRIAL				43
#define FE_CLE_CUBIC					44
#define FE_CLE_ORTHOTROPIC				45
#define FE_ACTIVE_CONTRACT_UNI_OLD      46
#define FE_ACTIVE_CONTRACT_TISO_OLD     47
#define FE_ACTIVE_CONTRACT_ISO          48
#define FE_ACTIVE_CONTRACT_UNI_UC_OLD   49
#define FE_ACTIVE_CONTRACT_TISO_UC_OLD  50
#define FE_ACTIVE_CONTRACT_ISO_UC       51
#define FE_FIBEREXPLIN_COUPLED			52
#define FE_COUPLED_TRANS_ISO_MR_OLD		53	//---> obsolete in 2.1
#define FE_TRANS_ISO_MOONEY_RIVLIN		54	// new in 2.0 (replaces old implementation)
#define FE_TRANS_ISO_VERONDA_WESTMANN	55	// new in 2.0 (replaces old implementation)
#define FE_MAT_ACTIVE_CONTRACTION		56	// new in 2.0
#define FE_MAT_MR_VON_MISES_FIBERS		57	// new in 2.0
#define FE_MAT_2D_TRANS_ISO_MR			58	// new in 2.0
#define FE_COUPLED_TRANS_ISO_MR			59	// new in 2.1 (replaces old implementation)
#define FE_INCOMP_NEO_HOOKEAN			60	// new in 2.2
#define FE_HOLZAPFEL_UC					61
#define FE_POROUS_NEO_HOOKEAN           62
#define FE_NATURAL_NEO_HOOKEAN          63
#define FE_PRESTRAIN_CONST_GRADIENT		64
#define FE_PRESTRAIN_INSITU_GRADIENT	65
#define FE_COUPLED_MOONEY_RIVLIN		66	// added in FS 1.0
#define FE_COUPLED_VERONDA_WESTMANN		67  // added in FS 1.0
#define FE_FIBEREXPLIN_UNCOUPLED		68  // added in FS 1.0
#define FE_COUPLED_TRANS_ISO_VW			69  // added in FS 1.0
#define FE_ACTIVE_CONTRACT_UNI          70
#define FE_ACTIVE_CONTRACT_TISO         71
#define FE_ACTIVE_CONTRACT_UNI_UC       72
#define FE_ACTIVE_CONTRACT_TISO_UC      73
#define FE_FIBEREXPPOW_COUPLED          74
#define FE_FIBEREXPPOW_UNCOUPLED        75
#define FE_FIBERPOWLIN_COUPLED          76
#define FE_FIBERPOWLIN_UNCOUPLED        77
#define FE_USER_MATERIAL				1000

// multi-materials (new from 1.5)
#define FE_HYPER_ELASTIC			100
#define FE_VISCO_ELASTIC			101
#define FE_BIPHASIC_MATERIAL		102
#define FE_BIPHASIC_SOLUTE			103
#define FE_SOLID_MIXTURE			104
#define FE_UNCOUPLED_SOLID_MIXTURE	105
#define FE_TRIPHASIC_MATERIAL		106
#define FE_SOLUTE_MATERIAL			107
#define FE_MULTIPHASIC_MATERIAL		108
#define FE_UNCOUPLED_VISCO_ELASTIC	109
#define FE_SBM_MATERIAL             110
#define FE_RV_MATERIAL              111
#define FE_RV_MATERIAL_UC           112
#define FE_FLUID_MATERIAL           120
#define FE_REACTION_DIFFUSION_MATERIAL	121
#define FE_SPECIES_MATERIAL			122
#define FE_SOLID_SPECIES_MATERIAL	123
#define FE_FLUID_FSI_MATERIAL       124
#define FE_GENERATION               125
#define FE_MULTI_GENERATION         126
#define FE_PRESTRAIN_MATERIAL		127
#define FE_UNCOUPLED_PRESTRAIN_MATERIAL		128

// permeability materials
#define FE_PERM_CONST				200
#define FE_PERM_HOLMES_MOW			201
#define FE_PERM_REF_ISO				202
#define FE_PERM_REF_TRANS_ISO		203
#define FE_PERM_REF_ORTHO			204
#define FE_PERM_EXP_ISO             205

// diffusivity materials
#define FE_DIFF_CONST				300
#define FE_DIFF_CONST_ORTHO			301
#define FE_DIFF_REF_ISO				302
#define FE_DIFF_ALBRO_ISO			303

// solubility materials
#define FE_SOLUB_CONST				400

// osmotic coefficient
#define FE_OSMO_CONST				500
#define FE_OSMO_WM                  501

// chemical reactions
#define FE_REACTANT_MATERIAL        600
#define FE_PRODUCT_MATERIAL         601
#define FE_MASS_ACTION_FORWARD      602
#define FE_MASS_ACTION_REVERSIBLE   603
#define FE_MICHAELIS_MENTEN         604
#define FE_REACTION_RATE_CONST      605
#define FE_REACTION_RATE_HUISKES    606

// fiber generators (old mechanism)
#define FE_FIBER_LOCAL			0
#define FE_FIBER_CYLINDRICAL	1
#define FE_FIBER_SPHERICAL		2
#define FE_FIBER_VECTOR			3
#define FE_FIBER_USER			4
#define FE_FIBER_ANGLES			5
#define FE_FIBER_POLAR			6

// continuous fiber distributions
#define FE_CFD_MATERIAL             700
#define FE_FIBER_EXP_POW            701
#define FE_FIBER_NH                 702
#define FE_CFD_MATERIAL_UC          710
#define FE_FIBER_EXP_POW_UC         711
#define FE_FIBER_NH_UC              712
#define FE_DSTRB_SFD                730
#define FE_DSTRB_EFD                731
#define FE_DSTRB_VM3                732
#define FE_DSTRB_CFD                733
#define FE_DSTRB_PFD                734
#define FE_DSTRB_VM2                735
#define FE_SCHM_GKT                 760
#define FE_SCHM_FEI                 761
#define FE_SCHM_T2D                 762
#define FE_SCHM_GKT_UC              770
#define FE_SCHM_FEI_UC              771
#define FE_SCHM_T2D_UC              772

// reduced relaxation functions
#define FE_RELAX_EXP                800
#define FE_RELAX_EXP_DIST           801
#define FE_RELAX_FUNG               802
#define FE_RELAX_PARK               803
#define FE_RELAX_PARK_DIST          804
#define FE_RELAX_POW                805
#define FE_RELAX_POW_DIST           806

// elastic damage materials
#define FE_DMG_MATERIAL             900
#define FE_DMG_MATERIAL_UC          910
#define FE_CDF_SIMO                 920
#define FE_CDF_LOG_NORMAL           921
#define FE_CDF_WEIBULL              922
#define FE_CDF_STEP                 923
#define FE_CDF_QUINTIC              924
#define FE_DC_SIMO                  940
#define FE_DC_SED                   941
#define FE_DC_SSE                   942
#define FE_DC_VMS                   943
#define FE_DC_MSS                   944
#define FE_DC_MNS                   945
#define FE_DC_MNLE                  946
#define FE_DC_SIMO_UC               960
#define FE_DC_SED_UC                961
#define FE_DC_SSE_UC                962
#define FE_DC_VMS_UC                963
#define FE_DC_MSS_UC                964
#define FE_DC_MNS_UC                965
#define FE_DC_MNLE_UC               966

// elastic pressure materials (fluid)
#define FE_EP_IDEAL_GAS             1001
#define FE_EP_IDEAL_FLUID           1002
#define FE_EP_NEOHOOKEAN_FLUID      1003

// viscous materials (fluid)
#define FE_VF_NEWTONIAN             1100
#define FE_VF_CARREAU               1101
#define FE_VF_CARREAU_YASUDA        1102
#define FE_VF_POWELL_EYRING         1103
#define FE_VF_CROSS                 1104

// solvent supplies
#define FE_STARLING_SUPPLY			1200

// fiber generators
#define FE_FIBER_GENERATOR_LOCAL		1301
#define FE_FIBER_GENERATOR_VECTOR		1302
#define FE_FIBER_GENERATOR_SPHERICAL	1303
#define FE_FIBER_GENERATOR_CYLINDRICAL	1304

// discrete materials
#define FE_DISCRETE_LINEAR_SPRING		1401
#define FE_DISCRETE_NONLINEAR_SPRING	1402
#define FE_DISCRETE_HILL				1403

// 1D functions
#define FE_FNC1D_POINT		1501

//-----------------------------------------------------------------------------
class FEFiberGenerator : public FEMaterial
{
public:
	FEFiberGenerator(int ntype) : FEMaterial(ntype) {}
};

//-----------------------------------------------------------------------------
class FEFiberGeneratorLocal : public FEFiberGenerator
{
public:
	FEFiberGeneratorLocal();
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FEFiberGeneratorLocal);
};

//-----------------------------------------------------------------------------
class FEFiberGeneratorVector : public FEFiberGenerator
{
public:
	FEFiberGeneratorVector();
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FEFiberGeneratorVector);
};

//-----------------------------------------------------------------------------
class FECylindricalVectorGenerator : public FEFiberGenerator
{
public:
	FECylindricalVectorGenerator();
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FECylindricalVectorGenerator);
};

//-----------------------------------------------------------------------------
class FESphericalVectorGenerator : public FEFiberGenerator
{
public:
	FESphericalVectorGenerator();
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FESphericalVectorGenerator);
};

//-----------------------------------------------------------------------------
// Isotropic Elastic
//
class FEIsotropicElastic : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_E, MP_v };

public:
	FEIsotropicElastic();

	DECLARE_REGISTERED(FEIsotropicElastic);
};

//-----------------------------------------------------------------------------
// Orthotropic elastic
class FEOrthoElastic : public FEMaterial
{
public:
	enum { 
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_v12, MP_v23, MP_v31
	};

public:
	FEOrthoElastic();

	DECLARE_REGISTERED(FEOrthoElastic);
};

//-----------------------------------------------------------------------------
// Neo-Hookean
//
class FENeoHookean : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_E, MP_v };

public:
	FENeoHookean();

	DECLARE_REGISTERED(FENeoHookean);
};

//-----------------------------------------------------------------------------
// Natural Neo-Hookean
//
class FENaturalNeoHookean : public FEMaterial
{
public:
    enum { MP_DENSITY, MP_G, MP_K };
    
public:
    FENaturalNeoHookean();
    
    DECLARE_REGISTERED(FENaturalNeoHookean);
};

//-----------------------------------------------------------------------------
// incompressible neo-Hookean
class FEIncompNeoHookean : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_G, MP_K };

public:
	FEIncompNeoHookean();
	DECLARE_REGISTERED(FEIncompNeoHookean);
};

//-----------------------------------------------------------------------------
// Porous neo-Hookean
//
class FEPorousNeoHookean : public FEMaterial
{
public:
    enum { MP_DENSITY, MP_E, MP_PHI0 };
    
public:
    FEPorousNeoHookean();
    
    DECLARE_REGISTERED(FEPorousNeoHookean);
};

//-----------------------------------------------------------------------------
// Mooney-Rivlin
//
class FEMooneyRivlin : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };
	
public:
	FEMooneyRivlin();

	DECLARE_REGISTERED(FEMooneyRivlin);
};

//-----------------------------------------------------------------------------
// Veronda-Westmann
//
class FEVerondaWestmann : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };
	
public:
	FEVerondaWestmann();

	DECLARE_REGISTERED(FEVerondaWestmann);
};

//-----------------------------------------------------------------------------
// coupled Mooney-Rivlin
//
class FECoupledMooneyRivlin : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };

public:
	FECoupledMooneyRivlin();

	DECLARE_REGISTERED(FECoupledMooneyRivlin);
};

//-----------------------------------------------------------------------------
// coupled Veronda-Westmann
//
class FECoupledVerondaWestmann : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };

public:
	FECoupledVerondaWestmann();

	DECLARE_REGISTERED(FECoupledVerondaWestmann);
};

//-----------------------------------------------------------------------------
// Holmes-Mow
//
class FEHolmesMow : public FEMaterial
{
public:
	enum { MP_DENS, MP_E, MP_V, MP_BETA };

public:
	FEHolmesMow();

	DECLARE_REGISTERED(FEHolmesMow);
};

//-----------------------------------------------------------------------------
// Arruda-Boyce
//
class FEArrudaBoyce: public FEMaterial
{
public:
	enum { MP_DENS, MP_MU, MP_N, MP_K };

public:
	FEArrudaBoyce();

	DECLARE_REGISTERED(FEArrudaBoyce);
};

//-----------------------------------------------------------------------------
// Carter-Hayes
//
class FECarterHayes : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_E0, MP_RHO0, MP_GA, MP_v , MP_SBM };
    
public:
	FECarterHayes();
    
	DECLARE_REGISTERED(FECarterHayes);
};

//-----------------------------------------------------------------------------
// PRLig
//
class FEPRLig : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_C1, MP_C2, MP_V0, MP_M, MP_MU, MP_K };

public:
	FEPRLig();

	DECLARE_REGISTERED(FEPRLig);
};

class FEOldFiberMaterial : public FEMaterial
{
private:
	enum {
		MP_AOPT, MP_N, 
		MP_R, MP_A,	MP_D, MP_PARAMS, MP_NUSER, MP_THETA, MP_PHI, MP_D0, MP_D1, MP_R0, MP_R1 };

public:
	int		m_naopt;	// fiber option
	int		m_n[2];		// node numbers for local option
	int		m_nuser;	// user data variable ID for user option
	vec3d	m_r;
	vec3d	m_a;
	vec3d	m_d;
	double	m_theta, m_phi;	// spherical angles

	// used by POLAR method
	vec3d	m_d0, m_d1;
	double	m_R0, m_R1;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void copy(FEOldFiberMaterial* pm);

public:
	FEOldFiberMaterial();

	vec3d GetFiberVector(FEElementRef& el);

	bool UpdateData(bool bsave) override;

private:
	FEOldFiberMaterial(const FEOldFiberMaterial& m);
	FEOldFiberMaterial& operator = (const FEOldFiberMaterial& m);
};

//-----------------------------------------------------------------------------
// base class for transversely isotropic materials
//
class FETransverselyIsotropic : public FEMaterial
{
public:
	enum { MP_MAT, MP_FIBERS };

public:
	FETransverselyIsotropic(int ntype);
	vec3d GetFiber(FEElementRef& el);
	bool HasFibers() { return true; }

	FEOldFiberMaterial* GetFiberMaterial();

	void copy(FEMaterial* pmat);
	void Load(IArchive& ar);
	void Save(OArchive& ar);

protected:
	void SetFiberMaterial(FEOldFiberMaterial* fiber);

private:
	FEOldFiberMaterial*	m_pfiber;
};

//-----------------------------------------------------------------------------
// Transversely Isotropic Mooney Rivlin (obsolete implementation)
//
class FETransMooneyRivlinOld : public FETransverselyIsotropic
{
public:
	// the fiber class for this material
	class Fiber : public FEOldFiberMaterial
	{
	public:
		enum {
			MP_C3, MP_C4, MP_C5, MP_LAM, 
			MP_CA0, MP_BETA, MP_L0, MP_LREF,
			MP_AC
		};

		Fiber();
	};

public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_K };

public:
	FETransMooneyRivlinOld();

//	DECLARE_REGISTERED(FETransMooneyRivlinOld);
};

//-----------------------------------------------------------------------------
// Transversely Isotropic Veronda-Westmann
//
class FETransVerondaWestmannOld : public FETransverselyIsotropic
{
public:
	// the fiber class for this material
	class Fiber : public FEOldFiberMaterial
	{
	public:
		enum {
			MP_C3, MP_C4, MP_C5, MP_LAM, 
			MP_CA0, MP_BETA, MP_L0, MP_LREF,
			MP_AC
		};

		Fiber();
	};

public:
	enum { MP_DENSITY, MP_C1, MP_C2, MP_K };

public:
	FETransVerondaWestmannOld();

//	DECLARE_REGISTERED(FETransVerondaWestmannOld);
};

//-----------------------------------------------------------------------------
// Active contraction material for new transverely-isotropic materials

class FEActiveContraction : public FEMaterial
{
public:
	enum { MP_ASCL, MP_CA0, MP_BETA, MP_L0, MP_REFL };

public:
	FEActiveContraction();

	DECLARE_REGISTERED(FEActiveContraction);
};

//-----------------------------------------------------------------------------
class FETransMooneyRivlin : public FETransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FETransMooneyRivlin();

	// convert from the old to the new format
	void Convert(FETransMooneyRivlinOld* mat);

	DECLARE_REGISTERED(FETransMooneyRivlin);
};

//-----------------------------------------------------------------------------
class FETransVerondaWestmann : public FETransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FETransVerondaWestmann();

	// convert from the old to the new format
	void Convert(FETransVerondaWestmannOld* mat);

	DECLARE_REGISTERED(FETransVerondaWestmann);
};

//-----------------------------------------------------------------------------
class FECoupledTransIsoVerondaWestmann : public FETransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FECoupledTransIsoVerondaWestmann();

	DECLARE_REGISTERED(FECoupledTransIsoVerondaWestmann);
};

//-----------------------------------------------------------------------------
// Coupled Transversely Isotropic Mooney Rivlin
//
class FECoupledTransIsoMooneyRivlinOld : public FEMaterial
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAMBDA, MP_K };

public:
	FECoupledTransIsoMooneyRivlinOld();

//	DECLARE_REGISTERED(FECoupledTransIsoMooneyRivlinOld);
};

//-----------------------------------------------------------------------------
// Coupled Transversely Isotropic Mooney Rivlin
//
class FECoupledTransIsoMooneyRivlin : public FETransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAMBDA, MP_K };

public:
	FECoupledTransIsoMooneyRivlin();

	void Convert(FECoupledTransIsoMooneyRivlinOld* mat);

	DECLARE_REGISTERED(FECoupledTransIsoMooneyRivlin);
};

//-----------------------------------------------------------------------------
class FEMooneyRivlinVonMisesFibers : public FEMaterial
{
public:
	FEMooneyRivlinVonMisesFibers();
	DECLARE_REGISTERED(FEMooneyRivlinVonMisesFibers);
};

//-----------------------------------------------------------------------------
class FE2DTransIsoMooneyRivlin : public FETransverselyIsotropic
{
public:
	FE2DTransIsoMooneyRivlin();
	DECLARE_REGISTERED(FE2DTransIsoMooneyRivlin);
};

//-----------------------------------------------------------------------------
// rigid body material
//
class FERigidMaterial : public FEMaterial
{
public:
	enum { 
		MP_DENSITY, MP_E, MP_V, 
		MP_COM, MP_RC,
	};

public:
	FERigidMaterial();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void copy(FEMaterial* pmat);

	void SetAutoCOM(bool b);
	void SetCenterOfMass(const vec3d& r);

public:
	int	m_pid;	// parent ID

	DECLARE_REGISTERED(FERigidMaterial);
};

//-----------------------------------------------------------------------------
// TC nonlinear orthotropic
//
class FETCNonlinearOrthotropic : public FEMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_C1, MP_C2, MP_K, 
		MP_BETA, MP_KSI,
		MP_A, MP_D };

public:
	FETCNonlinearOrthotropic();

	DECLARE_REGISTERED(FETCNonlinearOrthotropic);
};

//-----------------------------------------------------------------------------
// Fung Orthotropic
//
class FEFungOrthotropic : public FEMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31,
		MP_C, MP_K };

public:
	FEFungOrthotropic();

	DECLARE_REGISTERED(FEFungOrthotropic);
};

//-----------------------------------------------------------------------------
// Fung Orthotropic
//
class FEFungOrthoCompressible : public FEMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31,
		MP_C, MP_K };
    
public:
	FEFungOrthoCompressible();
    
	DECLARE_REGISTERED(FEFungOrthoCompressible);
};

//-----------------------------------------------------------------------------
// Linear Orthotropic
//
class FELinearOrthotropic : public FEMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31 };

public:
	FELinearOrthotropic();

	DECLARE_REGISTERED(FELinearOrthotropic);
};

//-----------------------------------------------------------------------------
// Muscle Material
//
class FEMuscleMaterial : public FETransverselyIsotropic
{
public:
	enum { MP_DENSITY, MP_G1, MP_G2, MP_K, MP_P1, MP_P2, MP_LOFL, MP_LAM, MP_SMAX };

public:
	FEMuscleMaterial();

	DECLARE_REGISTERED(FEMuscleMaterial);
};

//-----------------------------------------------------------------------------
// Tendon Material
//
class FETendonMaterial : public FETransverselyIsotropic
{
public:
	enum { MP_DENSITY, MP_G1, MP_G2, MP_K, MP_L1, MP_L2, MP_LAM };

	FETendonMaterial();

	DECLARE_REGISTERED(FETendonMaterial);
};

//-----------------------------------------------------------------------------
// Ogden Material
//
class FEOgdenMaterial : public FEMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_K,
		MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_C6,
		MP_M1, MP_M2, MP_M3, MP_M4, MP_M5, MP_M6};

public:
	FEOgdenMaterial();

	DECLARE_REGISTERED(FEOgdenMaterial);
};

//-----------------------------------------------------------------------------
// Ogden unconstrained Material
//
class FEOgdenUnconstrained: public FEMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_CP,
		MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_C6,
		MP_M1, MP_M2, MP_M3, MP_M4, MP_M5, MP_M6};

public:
	FEOgdenUnconstrained();

	DECLARE_REGISTERED(FEOgdenUnconstrained);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - Mooney-Rivlin
//
class FEEFDMooneyRivlin : public FEMaterial
{
public:
	enum { MP_C1, MP_C2, MP_K, MP_BETA, MP_KSI };

public:
	FEEFDMooneyRivlin();

	DECLARE_REGISTERED(FEEFDMooneyRivlin);

};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - neo-Hookean
//
class FEEFDNeoHookean : public FEMaterial
{
public:
	enum { MP_E, MP_v, MP_BETA, MP_KSI };

public:
	FEEFDNeoHookean();

	DECLARE_REGISTERED(FEEFDNeoHookean);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - Donnan Equilibrium Swelling
//
class FEEFDDonnan : public FEMaterial
{
public:
	enum
	{
		MP_PHIW0, MP_CF0, MP_BOSM, MP_PHI,
		MP_BETA, MP_KSI,
	};

public:
	FEEFDDonnan();

	DECLARE_REGISTERED(FEEFDDonnan);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution with Veronda-Westmann matrix
class FEEFDVerondaWestmann : public FEMaterial
{
public:
	enum
	{
		MP_C1, MP_C2, MP_K,
		MP_BETA, MP_KSI
	};

public:
	FEEFDVerondaWestmann();

	DECLARE_REGISTERED(FEEFDVerondaWestmann);
};

//-----------------------------------------------------------------------------
// Isotropic Fourier
class FEIsotropicFourier : public FEMaterial
{
public:
	enum { MP_DENSITY, MP_K, MP_C };

public:
	FEIsotropicFourier();
	DECLARE_REGISTERED(FEIsotropicFourier);
};

//-----------------------------------------------------------------------------
// Constant permeability
class FEPermConst : public FEMaterial
{
public:
	enum { MP_PERM };
public:
	FEPermConst();
	DECLARE_REGISTERED(FEPermConst);
};

//-----------------------------------------------------------------------------
// Holmes-Mow permeability
class FEPermHolmesMow : public FEMaterial
{
public:
	enum { MP_PERM, MP_M, MP_ALPHA };
public:
	FEPermHolmesMow();
	DECLARE_REGISTERED(FEPermHolmesMow);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss isotropic permeability
class FEPermAteshianWeissIso : public FEMaterial
{
public:
	enum { MP_PERM0, MP_PERM1, MP_PERM2, MP_M, MP_ALPHA };
public:
	FEPermAteshianWeissIso();
	DECLARE_REGISTERED(FEPermAteshianWeissIso);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss trans-isotropic permeability
class FEPermAteshianWeissTransIso : public FEMaterial
{
public:
	enum { MP_PERM0, MP_PERM1T, MP_PERM1A, MP_PERM2T, MP_PERM2A, MP_M0, MP_MT, MP_MA, MP_ALPHA0, MP_ALPHAT, MP_ALPHAA };
public:
	FEPermAteshianWeissTransIso();
	DECLARE_REGISTERED(FEPermAteshianWeissTransIso);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss orthotropic permeability
class FEPermAteshianWeissOrtho : public FEMaterial
{
public:
	enum { MP_PERM0, MP_PERM1, MP_PERM2, MP_M0, MP_ALPHA0, MP_M, MP_ALPHA };

public:
	FEPermAteshianWeissOrtho();
	DECLARE_REGISTERED(FEPermAteshianWeissOrtho);
};

//-----------------------------------------------------------------------------
// exponential isotropic permeability
class FEPermExpIso : public FEMaterial
{
public:
    enum { MP_PERM, MP_M };
public:
    FEPermExpIso();
    DECLARE_REGISTERED(FEPermExpIso);
};

//-----------------------------------------------------------------------------
// Constant diffusivity
class FEDiffConst : public FEMaterial
{
public:
	enum { MP_DIFF_FREE, MP_DIFF};

public:
	FEDiffConst();
	DECLARE_REGISTERED(FEDiffConst);
};

//-----------------------------------------------------------------------------
// orthotropic diffusivity
class FEDiffOrtho : public FEMaterial
{
public:
	enum { MP_DIFF_FREE, MP_DIFF};

public:
	FEDiffOrtho();
	DECLARE_REGISTERED(FEDiffOrtho);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss isotropic diffusivity
class FEDiffAteshianWeissIso : public FEMaterial
{
public:
	enum { MP_DIFF_FREE, MP_DIFF0, MP_DIFF1, MP_DIFF2, MP_M, MP_ALPHA };

public:
	FEDiffAteshianWeissIso();
	DECLARE_REGISTERED(FEDiffAteshianWeissIso);
};

//-----------------------------------------------------------------------------
// Albro isotropic diffusivity
class FEDiffAlbroIso : public FEMaterial
{
public:
	enum { MP_DIFF_FREE, MP_CDINV, MP_ALPHAD };
    
public:
	FEDiffAlbroIso();
	DECLARE_REGISTERED(FEDiffAlbroIso);
};

//-----------------------------------------------------------------------------
// Constant solubility
class FESolubConst : public FEMaterial
{
public:
	enum { MP_SOLUB };

public:
	FESolubConst();
	DECLARE_REGISTERED(FESolubConst);
};

//-----------------------------------------------------------------------------
// Constant osmotic coefficient
class FEOsmoConst : public FEMaterial
{
public:
	enum { MP_OSMO };

public:
	FEOsmoConst();
	DECLARE_REGISTERED(FEOsmoConst);
};

//-----------------------------------------------------------------------------
// Wells-Manning osmotic coefficient
class FEOsmoWellsManning : public FEMaterial
{
public:
    enum { MP_KSI, MP_COION };

public:
    FEOsmoWellsManning();

    // get/set co-ion index
    void SetCoIonIndex(int i) { SetIntValue(MP_COION, i); }
    int GetCoIonIndex() { return GetIntValue(MP_COION); }
    
    DECLARE_REGISTERED(FEOsmoWellsManning);
};

//-----------------------------------------------------------------------------
// SFD compressible
class FESFDCoupled : public FEMaterial
{
public:
	enum {MP_ALPHA, MP_BETA, MP_KSI};
public:
	FESFDCoupled();
	DECLARE_REGISTERED(FESFDCoupled);
};

//-----------------------------------------------------------------------------
// SFD SBM
class FESFDSBM : public FEMaterial
{
public:
	enum {MP_ALPHA, MP_BETA, MP_KSI0, MP_RHO0, MP_GAMMA, MP_SBM};
public:
	FESFDSBM();
	DECLARE_REGISTERED(FESFDSBM);
};

//-----------------------------------------------------------------------------
// EFD compressible
class FEEFDCoupled : public FEMaterial
{
public:
	enum {MP_BETA, MP_KSI};
public:
	FEEFDCoupled();
	DECLARE_REGISTERED(FEEFDCoupled);
};

//-----------------------------------------------------------------------------
// EFD uncoupled
class FEEFDUncoupled : public FEMaterial
{
public:
	enum {MP_BETA, MP_KSI, MP_K};
public:
	FEEFDUncoupled();
	DECLARE_REGISTERED(FEEFDUncoupled);
};

//-----------------------------------------------------------------------------
class FEFiberExpPowOld : public FEMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FEFiberExpPowOld();
//	DECLARE_REGISTERED(FEFiberExpPow);
};

//-----------------------------------------------------------------------------
class FEFiberExpPowUncoupledOld : public FEMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_K, MP_THETA, MP_PHI };
public:
	FEFiberExpPowUncoupledOld();
//	DECLARE_REGISTERED(FEFiberExpPowUncoupled);
};

//-----------------------------------------------------------------------------
class FEFiberPowLinOld : public FEMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_THETA, MP_PHI };
public:
    FEFiberPowLinOld();
//    DECLARE_REGISTERED(FEFiberPowLin);
};

//-----------------------------------------------------------------------------
class FEFiberPowLinUncoupledOld : public FEMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_K, MP_THETA, MP_PHI };
public:
    FEFiberPowLinUncoupledOld();
//    DECLARE_REGISTERED(FEFiberPowLinUncoupled);
};

//-----------------------------------------------------------------------------
class FEFiberMaterial : public FEMaterial
{
public:
	FEFiberMaterial(int ntype);

	bool HasFibers() override;

	vec3d GetFiber(FEElementRef& el) override;
};

//-----------------------------------------------------------------------------
class FEFiberExpPow : public FEMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI };

public:
    FEFiberExpPow();
    
    // convert from the old to the new format
    void Convert(FEFiberExpPowOld* mat);
    
    DECLARE_REGISTERED(FEFiberExpPow);
};

//-----------------------------------------------------------------------------
class FEFiberExpPowUncoupled : public FEMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_K };
public:
    FEFiberExpPowUncoupled();
    
    // convert from the old to the new format
    void Convert(FEFiberExpPowUncoupledOld* mat);
    
    DECLARE_REGISTERED(FEFiberExpPowUncoupled);
};

//-----------------------------------------------------------------------------
class FEFiberPowLin : public FEMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0 };
public:
    FEFiberPowLin();
    
    // convert from the old to the new format
    void Convert(FEFiberPowLinOld* mat);
    
    DECLARE_REGISTERED(FEFiberPowLin);
};

//-----------------------------------------------------------------------------
class FEFiberPowLinUncoupled : public FEMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_K };
public:
    FEFiberPowLinUncoupled();
    
    // convert from the old to the new format
    void Convert(FEFiberPowLinUncoupledOld* mat);
    
    DECLARE_REGISTERED(FEFiberPowLinUncoupled);
};

//-----------------------------------------------------------------------------
class FEFiberExpLinear : public FEMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FEFiberExpLinear();
	DECLARE_REGISTERED(FEFiberExpLinear);
};

//-----------------------------------------------------------------------------
class FEFiberExpLinearUncoupled : public FEMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FEFiberExpLinearUncoupled();
	DECLARE_REGISTERED(FEFiberExpLinearUncoupled);
};

//-----------------------------------------------------------------------------
// CLE cubic
//
class FECubicCLE : public FEMaterial
{
public:
    enum { MP_DENSITY, MP_LP1, MP_LM1, MP_L2, MP_MU };
    
public:
    FECubicCLE();
    
    DECLARE_REGISTERED(FECubicCLE);
};

//-----------------------------------------------------------------------------
// CLE orthotropic
//
class FEOrthotropicCLE : public FEMaterial
{
public:
    enum { MP_DENSITY,
        MP_LP11, MP_LP22, MP_LP33,
        MP_LM11, MP_LM22, MP_LM33,
        MP_L12, MP_L23, MP_L31,
        MP_MU1, MP_MU2, MP_MU3
    };
    
public:
    FEOrthotropicCLE();
    
    DECLARE_REGISTERED(FEOrthotropicCLE);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction
//
class FEPrescribedActiveContractionUniaxialOld : public FEMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FEPrescribedActiveContractionUniaxialOld();
    
//    DECLARE_REGISTERED(FEPrescribedActiveContractionUniaxial);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction
//
class FEPrescribedActiveContractionTransIsoOld : public FEMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FEPrescribedActiveContractionTransIsoOld();
    
//    DECLARE_REGISTERED(FEPrescribedActiveContractionTransIso);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction
//
class FEPrescribedActiveContractionUniaxial : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionUniaxial();
    
    // convert from the old to the new format
    void Convert(FEPrescribedActiveContractionUniaxialOld* mat);

    DECLARE_REGISTERED(FEPrescribedActiveContractionUniaxial);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction
//
class FEPrescribedActiveContractionTransIso : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionTransIso();
    
    // convert from the old to the new format
    void Convert(FEPrescribedActiveContractionTransIsoOld* mat);

    DECLARE_REGISTERED(FEPrescribedActiveContractionTransIso);
};

//-----------------------------------------------------------------------------
// Prescribed isotropic active contraction
//
class FEPrescribedActiveContractionIsotropic : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionIsotropic();
    
    DECLARE_REGISTERED(FEPrescribedActiveContractionIsotropic);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction uncoupled
//
class FEPrescribedActiveContractionUniaxialUCOld : public FEMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FEPrescribedActiveContractionUniaxialUCOld();
    
//    DECLARE_REGISTERED(FEPrescribedActiveContractionUniaxialUC);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction uncoupled
//
class FEPrescribedActiveContractionTransIsoUCOld : public FEMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FEPrescribedActiveContractionTransIsoUCOld();
    
//    DECLARE_REGISTERED(FEPrescribedActiveContractionTransIsoUC);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction uncoupled
//
class FEPrescribedActiveContractionUniaxialUC : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionUniaxialUC();
    
    // convert from the old to the new format
    void Convert(FEPrescribedActiveContractionUniaxialUCOld* mat);

    DECLARE_REGISTERED(FEPrescribedActiveContractionUniaxialUC);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction uncoupled
//
class FEPrescribedActiveContractionTransIsoUC : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionTransIsoUC();
    
    // convert from the old to the new format
    void Convert(FEPrescribedActiveContractionTransIsoUCOld* mat);

    DECLARE_REGISTERED(FEPrescribedActiveContractionTransIsoUC);
};

//-----------------------------------------------------------------------------
// Prescribed isotropic active contraction uncoupled
//
class FEPrescribedActiveContractionIsotropicUC : public FEMaterial
{
public:
    enum { MP_T0 };
    
public:
    FEPrescribedActiveContractionIsotropicUC();
    
    DECLARE_REGISTERED(FEPrescribedActiveContractionIsotropicUC);
};

//-----------------------------------------------------------------------------
class FEDonnanSwelling : public FEMaterial
{
public:
	enum { MP_PHIW0, MP_CF0, MP_BOSM, MP_PHI };
public:
	FEDonnanSwelling();
	DECLARE_REGISTERED(FEDonnanSwelling);
};

//-----------------------------------------------------------------------------
class FEPerfectOsmometer : public FEMaterial
{
public:
	enum { MP_PHIW0, MP_IOSM, MP_BOSM };
public:
	FEPerfectOsmometer();
	DECLARE_REGISTERED(FEPerfectOsmometer);
};

//-----------------------------------------------------------------------------
class FECellGrowth : public FEMaterial
{
public:
	enum { MP_PHIR, MP_CR, MP_CE };
public:
	FECellGrowth();
	DECLARE_REGISTERED(FECellGrowth);
};

//-----------------------------------------------------------------------------
class FEOsmoticVirial : public FEMaterial
{
public:
    enum { MP_PHIW0, MP_CR, MP_C1, MP_C2, MP_c3 };
public:
    FEOsmoticVirial();
    DECLARE_REGISTERED(FEOsmoticVirial);
};

//-----------------------------------------------------------------------------
class FEReactionRateConst : public FEMaterial
{
public:
	enum { MP_K };

	double GetRateConstant();
	void SetRateConstant(double K);

public:
	FEReactionRateConst();
	DECLARE_REGISTERED(FEReactionRateConst);
};

//-----------------------------------------------------------------------------
class FEReactionRateHuiskes : public FEMaterial
{
public:
	enum { MP_B, MP_PSI0 };
public:
	FEReactionRateHuiskes();
	DECLARE_REGISTERED(FEReactionRateHuiskes);
};

//-----------------------------------------------------------------------------
class FECFDFiberExpPow : public FEMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_MU };
public:
    FECFDFiberExpPow();
    DECLARE_REGISTERED(FECFDFiberExpPow);
};

//-----------------------------------------------------------------------------
class FECFDFiberNH : public FEMaterial
{
public:
    enum { MP_MU };
public:
    FECFDFiberNH();
    DECLARE_REGISTERED(FECFDFiberNH);
};

//-----------------------------------------------------------------------------
class FECFDFiberExpPowUC : public FEMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_MU, MP_K };
public:
    FECFDFiberExpPowUC();
    DECLARE_REGISTERED(FECFDFiberExpPowUC);
};

//-----------------------------------------------------------------------------
class FECFDFiberNHUC : public FEMaterial
{
public:
    enum { MP_MU, MP_K };
public:
    FECFDFiberNHUC();
    DECLARE_REGISTERED(FECFDFiberNHUC);
};

//-----------------------------------------------------------------------------
class FEFDDSpherical : public FEMaterial
{
public:
    FEFDDSpherical();
    DECLARE_REGISTERED(FEFDDSpherical);
};

//-----------------------------------------------------------------------------
class FEFDDEllipsoidal : public FEMaterial
{
public:
    enum { MP_SPA };
public:
    FEFDDEllipsoidal();
    DECLARE_REGISTERED(FEFDDEllipsoidal);
};

//-----------------------------------------------------------------------------
class FEFDDvonMises3d : public FEMaterial
{
public:
    enum { MP_B };
public:
    FEFDDvonMises3d();
    DECLARE_REGISTERED(FEFDDvonMises3d);
};

//-----------------------------------------------------------------------------
class FEFDDCircular : public FEMaterial
{
public:
    FEFDDCircular();
    DECLARE_REGISTERED(FEFDDCircular);
};

//-----------------------------------------------------------------------------
class FEFDDElliptical : public FEMaterial
{
public:
    enum { MP_SPA1, M_SPA2 };
public:
    FEFDDElliptical();
    DECLARE_REGISTERED(FEFDDElliptical);
};

//-----------------------------------------------------------------------------
class FEFDDvonMises2d : public FEMaterial
{
public:
    enum { MP_B };
public:
    FEFDDvonMises2d();
    DECLARE_REGISTERED(FEFDDvonMises2d);
};

//-----------------------------------------------------------------------------
class FESchemeGKT : public FEMaterial
{
public:
    enum { MP_NPH, M_NTH };
public:
    FESchemeGKT();
    DECLARE_REGISTERED(FESchemeGKT);
};

//-----------------------------------------------------------------------------
class FESchemeFEI : public FEMaterial
{
public:
    enum { MP_RES };
public:
    FESchemeFEI();
    DECLARE_REGISTERED(FESchemeFEI);
};

//-----------------------------------------------------------------------------
class FESchemeT2d : public FEMaterial
{
public:
    enum { M_NTH };
public:
    FESchemeT2d();
    DECLARE_REGISTERED(FESchemeT2d);
};

//-----------------------------------------------------------------------------
class FESchemeGKTUC : public FEMaterial
{
public:
    enum { MP_NPH, M_NTH };
public:
    FESchemeGKTUC();
    DECLARE_REGISTERED(FESchemeGKTUC);
};

//-----------------------------------------------------------------------------
class FESchemeFEIUC : public FEMaterial
{
public:
    enum { MP_RES };
public:
    FESchemeFEIUC();
    DECLARE_REGISTERED(FESchemeFEIUC);
};

//-----------------------------------------------------------------------------
class FESchemeT2dUC : public FEMaterial
{
public:
    enum { M_NTH };
public:
    FESchemeT2dUC();
    DECLARE_REGISTERED(FESchemeT2dUC);
};

//-----------------------------------------------------------------------------
class FECDFSimo : public FEMaterial
{
public:
    enum { MP_A, MP_B };
public:
    FECDFSimo();
    DECLARE_REGISTERED(FECDFSimo);
};

//-----------------------------------------------------------------------------
class FECDFLogNormal : public FEMaterial
{
public:
    enum { MP_MU, MP_SIGMA, MP_DMAX };
public:
    FECDFLogNormal();
    DECLARE_REGISTERED(FECDFLogNormal);
};

//-----------------------------------------------------------------------------
class FECDFWeibull : public FEMaterial
{
public:
    enum { MP_ALPHA, MP_MU, MP_DMAX };
public:
    FECDFWeibull();
    DECLARE_REGISTERED(FECDFWeibull);
};

//-----------------------------------------------------------------------------
class FECDFStep : public FEMaterial
{
public:
    enum { MP_MU, MP_DMAX };
public:
    FECDFStep();
    DECLARE_REGISTERED(FECDFStep);
};

//-----------------------------------------------------------------------------
class FECDFQuintic : public FEMaterial
{
public:
    enum { MP_MUMIN, MP_MUMAX, MP_DMAX };
public:
    FECDFQuintic();
    DECLARE_REGISTERED(FECDFQuintic);
};

//-----------------------------------------------------------------------------
class FEDCSimo : public FEMaterial
{
public:
    FEDCSimo();
    DECLARE_REGISTERED(FEDCSimo);
};

//-----------------------------------------------------------------------------
class FEDCStrainEnergyDensity : public FEMaterial
{
public:
    FEDCStrainEnergyDensity();
    DECLARE_REGISTERED(FEDCStrainEnergyDensity);
};

//-----------------------------------------------------------------------------
class FEDCSpecificStrainEnergy : public FEMaterial
{
public:
    FEDCSpecificStrainEnergy();
    DECLARE_REGISTERED(FEDCSpecificStrainEnergy);
};

//-----------------------------------------------------------------------------
class FEDCvonMisesStress : public FEMaterial
{
public:
    FEDCvonMisesStress();
    DECLARE_REGISTERED(FEDCvonMisesStress);
};

//-----------------------------------------------------------------------------
class FEDCMaxShearStress : public FEMaterial
{
public:
    FEDCMaxShearStress();
    DECLARE_REGISTERED(FEDCMaxShearStress);
};

//-----------------------------------------------------------------------------
class FEDCMaxNormalStress : public FEMaterial
{
public:
    FEDCMaxNormalStress();
    DECLARE_REGISTERED(FEDCMaxNormalStress);
};

//-----------------------------------------------------------------------------
class FEDCMaxNormalLagrangeStrain : public FEMaterial
{
public:
    FEDCMaxNormalLagrangeStrain();
    DECLARE_REGISTERED(FEDCMaxNormalLagrangeStrain);
};

//-----------------------------------------------------------------------------
class FEDCSimoUC : public FEMaterial
{
public:
    FEDCSimoUC();
    DECLARE_REGISTERED(FEDCSimoUC);
};

//-----------------------------------------------------------------------------
class FEDCStrainEnergyDensityUC : public FEMaterial
{
public:
    FEDCStrainEnergyDensityUC();
    DECLARE_REGISTERED(FEDCStrainEnergyDensityUC);
};

//-----------------------------------------------------------------------------
class FEDCSpecificStrainEnergyUC : public FEMaterial
{
public:
    FEDCSpecificStrainEnergyUC();
    DECLARE_REGISTERED(FEDCSpecificStrainEnergyUC);
};

//-----------------------------------------------------------------------------
class FEDCvonMisesStressUC : public FEMaterial
{
public:
    FEDCvonMisesStressUC();
    DECLARE_REGISTERED(FEDCvonMisesStressUC);
};

//-----------------------------------------------------------------------------
class FEDCMaxShearStressUC : public FEMaterial
{
public:
    FEDCMaxShearStressUC();
    DECLARE_REGISTERED(FEDCMaxShearStressUC);
};

//-----------------------------------------------------------------------------
class FEDCMaxNormalStressUC : public FEMaterial
{
public:
    FEDCMaxNormalStressUC();
    DECLARE_REGISTERED(FEDCMaxNormalStressUC);
};

//-----------------------------------------------------------------------------
class FEDCMaxNormalLagrangeStrainUC : public FEMaterial
{
public:
    FEDCMaxNormalLagrangeStrainUC();
    DECLARE_REGISTERED(FEDCMaxNormalLagrangeStrainUC);
};

//-----------------------------------------------------------------------------
class FERelaxExp : public FEMaterial
{
public:
    enum { MP_TAU };
public:
    FERelaxExp();
    DECLARE_REGISTERED(FERelaxExp);
};

//-----------------------------------------------------------------------------
class FERelaxExpDistortion : public FEMaterial
{
public:
    enum { MP_TAU, M_TAU1, M_ALPHA };
public:
    FERelaxExpDistortion();
    DECLARE_REGISTERED(FERelaxExpDistortion);
};

//-----------------------------------------------------------------------------
class FERelaxFung : public FEMaterial
{
public:
    enum { MP_TAU1, M_TAU2 };
public:
    FERelaxFung();
    DECLARE_REGISTERED(FERelaxFung);
};

//-----------------------------------------------------------------------------
class FERelaxPark : public FEMaterial
{
public:
    enum { MP_TAU, M_BETA };
public:
    FERelaxPark();
    DECLARE_REGISTERED(FERelaxPark);
};

//-----------------------------------------------------------------------------
class FERelaxParkDistortion : public FEMaterial
{
public:
    enum { MP_TAU, M_TAU1, M_BETA, M_BETA1, M_ALPHA };
public:
    FERelaxParkDistortion();
    DECLARE_REGISTERED(FERelaxParkDistortion);
};

//-----------------------------------------------------------------------------
class FERelaxPow : public FEMaterial
{
public:
    enum { MP_TAU, M_BETA };
public:
    FERelaxPow();
    DECLARE_REGISTERED(FERelaxPow);
};

//-----------------------------------------------------------------------------
class FERelaxPowDistortion : public FEMaterial
{
public:
    enum { MP_TAU, M_TAU1, M_BETA, M_BETA1, M_ALPHA };
public:
    FERelaxPowDistortion();
    DECLARE_REGISTERED(FERelaxPowDistortion);
};

//-----------------------------------------------------------------------------
// Elastic pressure for ideal gas
class FEEPIdealGas : public FEMaterial
{
public:
    enum { MP_M };
public:
    FEEPIdealGas();
    DECLARE_REGISTERED(FEEPIdealGas);
};

//-----------------------------------------------------------------------------
// Elastic pressure for ideal fluid
class FEEPIdealFluid : public FEMaterial
{
public:
    enum { MP_K };
public:
    FEEPIdealFluid();
    DECLARE_REGISTERED(FEEPIdealFluid);
};

//-----------------------------------------------------------------------------
// Elastic pressure for neo-Hookean fluid
class FEEPNeoHookeanFluid : public FEMaterial
{
public:
    enum { MP_K };
public:
    FEEPNeoHookeanFluid();
    DECLARE_REGISTERED(FEEPNeoHookeanFluid);
};

//-----------------------------------------------------------------------------
// Viscous Newtonian fluid
class FEVFNewtonian : public FEMaterial
{
public:
    enum { MP_MU, MP_K };
public:
    FEVFNewtonian();
    DECLARE_REGISTERED(FEVFNewtonian);
};

//-----------------------------------------------------------------------------
// Viscous Carreau fluid
class FEVFCarreau : public FEMaterial
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_N };
public:
    FEVFCarreau();
    DECLARE_REGISTERED(FEVFCarreau);
};

//-----------------------------------------------------------------------------
// Viscous Carreau-Yasuda fluid
class FEVFCarreauYasuda : public FEMaterial
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_N, MP_A };
public:
    FEVFCarreauYasuda();
    DECLARE_REGISTERED(FEVFCarreauYasuda);
};

//-----------------------------------------------------------------------------
// Viscous Powell-Eyring fluid
class FEVFPowellEyring : public FEMaterial
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM };
public:
    FEVFPowellEyring();
    DECLARE_REGISTERED(FEVFPowellEyring);
};

//-----------------------------------------------------------------------------
// Viscous Cross fluid
class FEVFCross : public FEMaterial
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_M };
public:
    FEVFCross();
    DECLARE_REGISTERED(FEVFCross);
};

//-----------------------------------------------------------------------------
class FEStarlingSupply : public FEMaterial
{
public:
	enum { MP_KP, MP_PV };
public:
	FEStarlingSupply();
	DECLARE_REGISTERED(FEStarlingSupply);
};

//-----------------------------------------------------------------------------
class FEPrestrainConstGradient : public FEMaterial
{
public:
	enum { MP_F0 };
public:
	FEPrestrainConstGradient();
	DECLARE_REGISTERED(FEPrestrainConstGradient);
};

//-----------------------------------------------------------------------------
class FEPrestrainInSituGradient : public FEMaterial
{
public:
	enum { MP_LAM, MP_ISO };
public:
	FEPrestrainInSituGradient();
	DECLARE_REGISTERED(FEPrestrainInSituGradient);
};
