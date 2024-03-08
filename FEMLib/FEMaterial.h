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
#define FE_MAT_POLAR_FLUID          0x0300
#define FE_MAT_FLUID_SOLUTES        0x0400
#define FE_MAT_THERMO_FLUID         0x0500

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
#define FE_MAT_MREACTION                0x001D0000
#define FE_MAT_MREACTION_RATE           0x001E0000
#define FE_MAT_MREACTION_IREACTANTS     0x001F0000
#define FE_MAT_MREACTION_IPRODUCTS      0x00200000
#define FE_MAT_MREACTION_EREACTANTS     0x00210000
#define FE_MAT_MREACTION_EPRODUCTS      0x00220000
#define FE_MAT_PLASTIC_FLOW_RULE		0x00230000
#define FE_MAT_POLAR_FLUID_VISCOSITY    0x00240000


// Classes with IDs above this refer to FEBio base classes.
#define FE_FEBIO_MATERIAL_CLASS			0x00FF0000

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
#define FE_HOLZAPFEL_GASSER_OGDEN		61
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
#define FE_FIBER_DAMAGE_POWER			78	// added in FS 1.1
#define FE_FIBER_DAMAGE_EXP				79	// added in FS 1.1
#define FE_FIBER_DAMAGE_EXPLINEAR		80	// added in FS 1.2
#define FE_HOLZAPFEL_UNCONSTRAINED      81
#define FE_FIBER_KIOUSIS_UNCOUPLED      82
#define FE_NEWTONIAN_VISCOUS_SOLID      83
#define FE_ISOTROPIC_LEE_SACKS			84
#define FE_ISOTROPIC_LEE_SACKS_UNCOUPLED 85
#define FE_FIBER_NEO_HOOKEAN            86
#define FE_FIBER_NATURAL_NH             87
#define FE_HOLMES_MOW_UNCOUPLED         88
#define FE_TRACE_FREE_NEO_HOOKEAN       89
#define FE_POLYNOMIAL_HYPERELASTIC      90
#define FE_FORCE_VELOCITY_ESTRADA       91
#define FE_FIBER_EXP_POW_LIN            92
#define FE_HGO_CORONARY                 93
#define FE_ACTIVE_CONTRACT_FIBER        94
#define FE_ACTIVE_CONTRACT_FIBER_UC     95
#define FE_ARRUDA_BOYCE_UC              96
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
#define FE_REACTIVE_PLASTICITY      129
#define FE_REACTIVE_PLASTIC_DAMAGE  130
#define FE_BIPHASIC_FSI_MATERIAL    131
#define FE_RV_DAMAGE_MATERIAL       132
#define FE_REACTIVE_FATIGUE         133
#define FE_UNCOUPLED_REACTIVE_FATIGUE   134

#define FE_FEBIO_MATERIAL			135
#define FE_POLAR_FLUID_MATERIAL     136

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
#define FE_REACTION_RATE_FEBIO		607
#define FE_MASS_ACTION_REACTION		608

// membrane reactions
#define FE_INT_REACTANT_MATERIAL    650
#define FE_INT_PRODUCT_MATERIAL     651
#define FE_EXT_REACTANT_MATERIAL    652
#define FE_EXT_PRODUCT_MATERIAL     653
#define FE_MMASS_ACTION_FORWARD     654
#define FE_MMASS_ACTION_REVERSIBLE  655
#define FE_MREACTION_RATE_CONST     656
#define FE_MREACTION_RATE_ION_CHNL  657
#define FE_MREACTION_RATE_VOLT_GTD  658

// fiber generators (old mechanism)
#define FE_FIBER_LOCAL			0
#define FE_FIBER_CYLINDRICAL	1
#define FE_FIBER_SPHERICAL		2
#define FE_FIBER_VECTOR			3
#define FE_FIBER_USER			4
#define FE_FIBER_ANGLES			5
#define FE_FIBER_POLAR			6
#define FE_FIBER_MAP			7

// continuous fiber distributions
#define FE_CFD_MATERIAL             700
#define FE_FIBER_EXP_POW            701
#define FE_FIBER_NH                 702
#define FE_CFD_MATERIAL_UC          710
#define FE_FIBER_EXP_POW_UC         711
#define FE_FIBER_NH_UC              712
#define FE_FIBER_POW_LIN            713
#define FE_FIBER_POW_LIN_UC         714
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
#define FE_RELAX_PRONY              807
#define FE_RELAX_MALKIN             808
#define FE_RELAX_CSEXP              809
#define FE_RELAX_MALKIN_DIST        810
#define FE_RELAX_MALKIN_DIST_USER   811
#define FE_RELAX_CSEXP_DIST_USER    812
#define FE_RELAX_EXP_DIST_USER      813
#define FE_RELAX_POW_DIST_USER      814

// elastic damage materials
#define FE_DMG_MATERIAL             900
#define FE_DMG_MATERIAL_UC          910
#define FE_CDF_SIMO                 920
#define FE_CDF_LOG_NORMAL           921
#define FE_CDF_WEIBULL              922
#define FE_CDF_STEP                 923
#define FE_CDF_QUINTIC              924
#define FE_CDF_POWER                925
#define FE_DC_SIMO                  940
#define FE_DC_SED                   941
#define FE_DC_SSE                   942
#define FE_DC_VMS                   943
#define FE_DC_MSS                   944
#define FE_DC_MNS                   945
#define FE_DC_MNLE                  946
#define FE_DC_OSS                   947
#define FE_DC_SIMO_UC               960
#define FE_DC_SED_UC                961
#define FE_DC_SSE_UC                962
#define FE_DC_VMS_UC                963
#define FE_DC_MSS_UC                964
#define FE_DC_MNS_UC                965
#define FE_DC_MNLE_UC               966
#define FE_DC_DRUCKER               967

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
#define FE_VF_BINGHAM               1105

// polar viscous fluid material
#define FE_PVF_LINEAR               1150

// solvent supplies
#define FE_STARLING_SUPPLY			1200

// fiber generators
#define FE_FIBER_GENERATOR_LOCAL		1301
#define FE_FIBER_GENERATOR_VECTOR		1302
#define FE_FIBER_GENERATOR_SPHERICAL	1303
#define FE_FIBER_GENERATOR_CYLINDRICAL	1304
#define FE_FIBER_GENERATOR_ANGLES		1305

// discrete materials
#define FE_DISCRETE_LINEAR_SPRING		1401
#define FE_DISCRETE_NONLINEAR_SPRING	1402
#define FE_DISCRETE_HILL				1403
#define FE_DISCRETE_FEBIO_MATERIAL		1404

// 1D functions
#define FE_FNC1D_POINT		1501
#define FE_FNC1D_MATH		1502

// plastic flow rules
#define FE_MAT_PLASTIC_FLOW_PAPER		1601
#define FE_MAT_PLASTIC_FLOW_USER		1602
#define FE_MAT_PLASTIC_FLOW_MATH		1603

//-----------------------------------------------------------------------------
class FSFiberGenerator : public FSMaterial
{
public:
    FSFiberGenerator(int ntype, FSModel* fem);
};

//-----------------------------------------------------------------------------
class FSFiberGeneratorLocal : public FSFiberGenerator
{
public:
	FSFiberGeneratorLocal(FSModel* fem, int n0 = 0, int n1 = 0);
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FSFiberGeneratorLocal);
};

//-----------------------------------------------------------------------------
class FSFiberGeneratorVector : public FSFiberGenerator
{
public:
	FSFiberGeneratorVector(FSModel* fem, const vec3d& v = vec3d(1,0,0));
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FSFiberGeneratorVector);
};

//-----------------------------------------------------------------------------
class FSCylindricalVectorGenerator : public FSFiberGenerator
{
public:
	FSCylindricalVectorGenerator(FSModel* fem);
	FSCylindricalVectorGenerator(FSModel* fem, const vec3d& center, const vec3d& axis, const vec3d& vector);
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FSCylindricalVectorGenerator);
};

//-----------------------------------------------------------------------------
class FSSphericalVectorGenerator : public FSFiberGenerator
{
public:
	FSSphericalVectorGenerator(FSModel* fem);
	FSSphericalVectorGenerator(FSModel* fem, const vec3d& center, const vec3d& vector);
	vec3d GetFiber(FEElementRef& el) override;
	DECLARE_REGISTERED(FSSphericalVectorGenerator);
};

//-----------------------------------------------------------------------------
class FSAnglesVectorGenerator : public FSFiberGenerator
{
public:
	FSAnglesVectorGenerator(FSModel* fem, double theta = 0.0, double phi = 90.0);
	vec3d GetFiber(FEElementRef& el) override;
	void GetAngles(double& theta, double& phi);
	void SetAngles(double theta, double phi);
	DECLARE_REGISTERED(FSAnglesVectorGenerator);
};

//-----------------------------------------------------------------------------
// Isotropic Elastic
//
class FSIsotropicElastic : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_E, MP_v };

public:
	FSIsotropicElastic(FSModel* fem);

	DECLARE_REGISTERED(FSIsotropicElastic);
};

//-----------------------------------------------------------------------------
// Orthotropic elastic
class FSOrthoElastic : public FSMaterial
{
public:
	enum { 
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_v12, MP_v23, MP_v31
	};

public:
	FSOrthoElastic(FSModel* fem);

	DECLARE_REGISTERED(FSOrthoElastic);
};

//-----------------------------------------------------------------------------
// Neo-Hookean
//
class FSNeoHookean : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_E, MP_v };

public:
	FSNeoHookean(FSModel* fem);

	DECLARE_REGISTERED(FSNeoHookean);
};

//-----------------------------------------------------------------------------
// Natural Neo-Hookean
//
class FSNaturalNeoHookean : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_G, MP_K };
    
public:
    FSNaturalNeoHookean(FSModel* fem);
    
    DECLARE_REGISTERED(FSNaturalNeoHookean);
};

//-----------------------------------------------------------------------------
// trace-free Neo-Hookean
//
class FSTraceFreeNeoHookean : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_MU };
    
public:
    FSTraceFreeNeoHookean(FSModel* fem);
    
    DECLARE_REGISTERED(FSTraceFreeNeoHookean);
};

//-----------------------------------------------------------------------------
// incompressible neo-Hookean
class FSIncompNeoHookean : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_G, MP_K };

public:
	FSIncompNeoHookean(FSModel* fem);
	DECLARE_REGISTERED(FSIncompNeoHookean);
};

//-----------------------------------------------------------------------------
// Porous neo-Hookean
//
class FSPorousNeoHookean : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_E, MP_PHI0 };
    
public:
    FSPorousNeoHookean(FSModel* fem);
    
    DECLARE_REGISTERED(FSPorousNeoHookean);
};

//-----------------------------------------------------------------------------
// Mooney-Rivlin
//
class FSMooneyRivlin : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };
	
public:
	FSMooneyRivlin(FSModel* fem);

	DECLARE_REGISTERED(FSMooneyRivlin);
};

//-----------------------------------------------------------------------------
// Veronda-Westmann
//
class FSVerondaWestmann : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };
	
public:
	FSVerondaWestmann(FSModel* fem);

	DECLARE_REGISTERED(FSVerondaWestmann);
};

//-----------------------------------------------------------------------------
// polynomial hyper-elastic
//
class FSPolynomialHyperelastic : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_C01, MP_C02, MP_C10, MP_C11, MP_C12, MP_C20, MP_C21, MP_C22, MP_D1, MP_D2 };

public:
    FSPolynomialHyperelastic(FSModel* fem);

    DECLARE_REGISTERED(FSPolynomialHyperelastic);
};


//-----------------------------------------------------------------------------
// coupled Mooney-Rivlin
//
class FSCoupledMooneyRivlin : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };

public:
	FSCoupledMooneyRivlin(FSModel* fem);

	DECLARE_REGISTERED(FSCoupledMooneyRivlin);
};

//-----------------------------------------------------------------------------
// coupled Veronda-Westmann
//
class FSCoupledVerondaWestmann : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_A, MP_B, MP_K };

public:
	FSCoupledVerondaWestmann(FSModel* fem);

	DECLARE_REGISTERED(FSCoupledVerondaWestmann);
};

//-----------------------------------------------------------------------------
// Holmes-Mow
//
class FSHolmesMow : public FSMaterial
{
public:
	enum { MP_DENS, MP_E, MP_V, MP_BETA };

public:
	FSHolmesMow(FSModel* fem);

	DECLARE_REGISTERED(FSHolmesMow);
};

//-----------------------------------------------------------------------------
// Holmes-Mow uncoupled
//
class FSHolmesMowUC : public FSMaterial
{
public:
    enum { MP_DENS, MP_MU, MP_BETA, MP_K };
    
public:
    FSHolmesMowUC(FSModel* fem);
    
    DECLARE_REGISTERED(FSHolmesMowUC);
};

//-----------------------------------------------------------------------------
// Arruda-Boyce
//
class FSArrudaBoyce: public FSMaterial
{
public:
	enum { MP_DENS, MP_MU, MP_N, MP_K };

public:
	FSArrudaBoyce(FSModel* fem);

	DECLARE_REGISTERED(FSArrudaBoyce);
};

//-----------------------------------------------------------------------------
// Arruda-Boyce unconstrained
//
class FSArrudaBoyceUC : public FSMaterial
{
public:
	enum { MP_DENS, MP_MU, MP_N, MP_K };

public:
	FSArrudaBoyceUC(FSModel* fem);

	DECLARE_REGISTERED(FSArrudaBoyceUC);
};


//-----------------------------------------------------------------------------
// Carter-Hayes
//
class FSCarterHayes : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_E0, MP_RHO0, MP_GA, MP_v , MP_SBM };
    
public:
	FSCarterHayes(FSModel* fem);
    
	DECLARE_REGISTERED(FSCarterHayes);
};

//-----------------------------------------------------------------------------
// Newtonian viscous solid
//
class FSNewtonianViscousSolid : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_MU, MP_K };
    
public:
    FSNewtonianViscousSolid(FSModel* fem);
    
    DECLARE_REGISTERED(FSNewtonianViscousSolid);
};

//-----------------------------------------------------------------------------
// PRLig
//
class FSPRLig : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_C1, MP_C2, MP_V0, MP_M, MP_MU, MP_K };

public:
	FSPRLig(FSModel* fem);

	DECLARE_REGISTERED(FSPRLig);
};

class FSOldFiberMaterial : public FSMaterial
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

	// used by map
	string	m_map;

public:
	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	void copy(FSOldFiberMaterial* pm);

public:
	FSOldFiberMaterial(FSModel* fem);

	vec3d GetFiberVector(FEElementRef& el);

	bool UpdateData(bool bsave) override;

private:
	FSOldFiberMaterial(const FSOldFiberMaterial& m);
	FSOldFiberMaterial& operator = (const FSOldFiberMaterial& m);

    vec3d GetLocalFiberVector(FEElementRef& el);
};

//-----------------------------------------------------------------------------
// base class for transversely isotropic materials
//
class FSTransverselyIsotropic : public FSMaterial
{
public:
	enum { MP_MAT, MP_FIBERS };

public:
	FSTransverselyIsotropic(int ntype, FSModel* fem);
	vec3d GetFiber(FEElementRef& el);
	bool HasFibers() { return true; }

	FSOldFiberMaterial* GetFiberMaterial();
	const FSOldFiberMaterial* GetFiberMaterial() const;

	void copy(FSMaterial* pmat);
	void Load(IArchive& ar);
	void Save(OArchive& ar);

protected:
	void SetFiberMaterial(FSOldFiberMaterial* fiber);

private:
	FSOldFiberMaterial*	m_pfiber;
};

//-----------------------------------------------------------------------------
// Transversely Isotropic Mooney Rivlin (obsolete implementation)
//
class FSTransMooneyRivlinOld : public FSTransverselyIsotropic
{
public:
	// the fiber class for this material
	class Fiber : public FSOldFiberMaterial
	{
	public:
		enum {
			MP_C3, MP_C4, MP_C5, MP_LAM, 
			MP_CA0, MP_BETA, MP_L0, MP_LREF,
			MP_AC
		};

		Fiber(FSModel* fem);
	};

public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_K };

public:
	FSTransMooneyRivlinOld(FSModel* fem);

//	DECLARE_REGISTERED(FSTransMooneyRivlinOld);
};

//-----------------------------------------------------------------------------
// Transversely Isotropic Veronda-Westmann
//
class FSTransVerondaWestmannOld : public FSTransverselyIsotropic
{
public:
	// the fiber class for this material
	class Fiber : public FSOldFiberMaterial
	{
	public:
		enum {
			MP_C3, MP_C4, MP_C5, MP_LAM, 
			MP_CA0, MP_BETA, MP_L0, MP_LREF,
			MP_AC
		};

		Fiber(FSModel* fem);
	};

public:
	enum { MP_DENSITY, MP_C1, MP_C2, MP_K };

public:
	FSTransVerondaWestmannOld(FSModel* fem);

//	DECLARE_REGISTERED(FSTransVerondaWestmannOld);
};

//-----------------------------------------------------------------------------
// Active contraction material for new transverely-isotropic materials

class FSActiveContraction : public FSMaterialProp
{
public:
	enum { MP_ASCL, MP_CA0, MP_BETA, MP_L0, MP_REFL };

public:
	FSActiveContraction(FSModel* fem);

	DECLARE_REGISTERED(FSActiveContraction);
};

//-----------------------------------------------------------------------------
// force-velocity Estrada contraction material for transverely-isotropic materials

class FSForceVelocityEstrada : public FSMaterialProp
{
public:
    enum { MP_ASCL, MP_CA0, MP_CAM, MP_BETA, MP_L0, MP_REFL, M_TMAX, M_AL1, M_AL2, M_AL3, M_A1, M_A2, M_A3, M_AT, M_FV };
    
public:
    FSForceVelocityEstrada(FSModel* fem);
    
    DECLARE_REGISTERED(FSForceVelocityEstrada);
};

//-----------------------------------------------------------------------------
class FSTransMooneyRivlin : public FSTransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FSTransMooneyRivlin(FSModel* fem);

	// convert from the old to the new format
	void Convert(FSTransMooneyRivlinOld* mat);

	DECLARE_REGISTERED(FSTransMooneyRivlin);
};

//-----------------------------------------------------------------------------
class FSTransVerondaWestmann : public FSTransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FSTransVerondaWestmann(FSModel* fem);

	// convert from the old to the new format
	void Convert(FSTransVerondaWestmannOld* mat);

	DECLARE_REGISTERED(FSTransVerondaWestmann);
};

//-----------------------------------------------------------------------------
class FSCoupledTransIsoVerondaWestmann : public FSTransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAM, MP_K };

public:
	FSCoupledTransIsoVerondaWestmann(FSModel* fem);

	DECLARE_REGISTERED(FSCoupledTransIsoVerondaWestmann);
};

//-----------------------------------------------------------------------------
// Coupled Transversely Isotropic Mooney Rivlin
//
class FSCoupledTransIsoMooneyRivlinOld : public FSMaterial
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_LAMBDA, MP_K };

public:
	FSCoupledTransIsoMooneyRivlinOld(FSModel* fem);

//	DECLARE_REGISTERED(FSCoupledTransIsoMooneyRivlinOld);
};

//-----------------------------------------------------------------------------
// Coupled Transversely Isotropic Mooney Rivlin
//
class FSCoupledTransIsoMooneyRivlin : public FSTransverselyIsotropic
{
public:
	// this material's material parameters
	enum { MP_DENSITY, MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_K, MP_LAMBDA };

public:
	FSCoupledTransIsoMooneyRivlin(FSModel* fem);

	void Convert(FSCoupledTransIsoMooneyRivlinOld* mat);

	DECLARE_REGISTERED(FSCoupledTransIsoMooneyRivlin);
};

//-----------------------------------------------------------------------------
class FSMooneyRivlinVonMisesFibers : public FSMaterial
{
public:
	FSMooneyRivlinVonMisesFibers(FSModel* fem);
	DECLARE_REGISTERED(FSMooneyRivlinVonMisesFibers);
};

//-----------------------------------------------------------------------------
class FS2DTransIsoMooneyRivlin : public FSTransverselyIsotropic
{
public:
	FS2DTransIsoMooneyRivlin(FSModel* fem);
	DECLARE_REGISTERED(FS2DTransIsoMooneyRivlin);
};

//-----------------------------------------------------------------------------
// rigid body material
//
class FSRigidMaterial : public FSMaterial
{
public:
	enum { 
		MP_DENSITY, MP_E, MP_V, 
		MP_COM, MP_RC,
	};

public:
	FSRigidMaterial(FSModel* fem);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void copy(FSMaterial* pmat);

	void SetAutoCOM(bool b);
	void SetCenterOfMass(const vec3d& r);

	vec3d GetCenterOfMass() const;

	bool IsRigid() override;

public:
	int	m_pid;	// parent ID

	DECLARE_REGISTERED(FSRigidMaterial);
};

//-----------------------------------------------------------------------------
// TC nonlinear orthotropic
//
class FSTCNonlinearOrthotropic : public FSMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_C1, MP_C2, MP_K, 
		MP_BETA, MP_KSI,
		MP_A, MP_D };

public:
	FSTCNonlinearOrthotropic(FSModel* fem);

	DECLARE_REGISTERED(FSTCNonlinearOrthotropic);
};

//-----------------------------------------------------------------------------
// Fung Orthotropic
//
class FSFungOrthotropic : public FSMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31,
		MP_C, MP_K };

public:
	FSFungOrthotropic(FSModel* fem);

	DECLARE_REGISTERED(FSFungOrthotropic);
};

//-----------------------------------------------------------------------------
// Fung Orthotropic
//
class FSFungOrthoCompressible : public FSMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31,
		MP_C, MP_K };
    
public:
	FSFungOrthoCompressible(FSModel* fem);
    
	DECLARE_REGISTERED(FSFungOrthoCompressible);
};

//-----------------------------------------------------------------------------
// Holzapfel-Gasser-Ogden
//
class FSHolzapfelGasserOgden : public FSMaterial
{
public:
    enum {
        MP_DENSITY,
        MP_C, MP_K1,
        MP_K2,MP_KAPPA,
        MP_G,
        MP_K };
    
public:
    FSHolzapfelGasserOgden(FSModel* fem);
    
    DECLARE_REGISTERED(FSHolzapfelGasserOgden);
};

//-----------------------------------------------------------------------------
// Holzapfel-Gasser-Ogden unconstrained
//
class FSHolzapfelUnconstrained : public FSMaterial
{
public:
    enum {
        MP_DENSITY,
        MP_C, MP_K1,
        MP_K2,MP_KAPPA,
        MP_G,
        MP_K };
    
public:
    FSHolzapfelUnconstrained(FSModel* fem);
    
    DECLARE_REGISTERED(FSHolzapfelUnconstrained);
};

//-----------------------------------------------------------------------------
// Linear Orthotropic
//
class FSLinearOrthotropic : public FSMaterial
{
public:
	enum {
		MP_DENSITY,
		MP_E1, MP_E2, MP_E3,
		MP_G12, MP_G23, MP_G31,
		MP_V12, MP_V23, MP_V31 };

public:
	FSLinearOrthotropic(FSModel* fem);

	DECLARE_REGISTERED(FSLinearOrthotropic);
};

//-----------------------------------------------------------------------------
// Muscle Material
//
class FSMuscleMaterial : public FSTransverselyIsotropic
{
public:
	enum { MP_DENSITY, MP_G1, MP_G2, MP_K, MP_P1, MP_P2, MP_LOFL, MP_LAM, MP_SMAX };

public:
	FSMuscleMaterial(FSModel* fem);

	DECLARE_REGISTERED(FSMuscleMaterial);
};

//-----------------------------------------------------------------------------
// Tendon Material
//
class FSTendonMaterial : public FSTransverselyIsotropic
{
public:
	enum { MP_DENSITY, MP_G1, MP_G2, MP_K, MP_L1, MP_L2, MP_LAM };

	FSTendonMaterial(FSModel* fem);

	DECLARE_REGISTERED(FSTendonMaterial);
};

//-----------------------------------------------------------------------------
// Ogden Material
//
class FSOgdenMaterial : public FSMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_K,
		MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_C6,
		MP_M1, MP_M2, MP_M3, MP_M4, MP_M5, MP_M6};

public:
	FSOgdenMaterial(FSModel* fem);

	DECLARE_REGISTERED(FSOgdenMaterial);
};

//-----------------------------------------------------------------------------
// Ogden unconstrained Material
//
class FSOgdenUnconstrained: public FSMaterial
{
public:
	enum { 
		MP_DENSITY,
		MP_CP,
		MP_C1, MP_C2, MP_C3, MP_C4, MP_C5, MP_C6,
		MP_M1, MP_M2, MP_M3, MP_M4, MP_M5, MP_M6};

public:
	FSOgdenUnconstrained(FSModel* fem);

	DECLARE_REGISTERED(FSOgdenUnconstrained);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - Mooney-Rivlin
//
class FSEFDMooneyRivlin : public FSMaterial
{
public:
	enum { MP_C1, MP_C2, MP_K, MP_BETA, MP_KSI };

public:
	FSEFDMooneyRivlin(FSModel* fem);

	DECLARE_REGISTERED(FSEFDMooneyRivlin);

};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - neo-Hookean
//
class FSEFDNeoHookean : public FSMaterial
{
public:
	enum { MP_E, MP_v, MP_BETA, MP_KSI };

public:
	FSEFDNeoHookean(FSModel* fem);

	DECLARE_REGISTERED(FSEFDNeoHookean);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution - Donnan Equilibrium Swelling
//
class FSEFDDonnan : public FSMaterial
{
public:
	enum
	{
		MP_PHIW0, MP_CF0, MP_BOSM, MP_PHI,
		MP_BETA, MP_KSI,
	};

public:
	FSEFDDonnan(FSModel* fem);

	DECLARE_REGISTERED(FSEFDDonnan);
};

//-----------------------------------------------------------------------------
// ellipsoidal fiber distribution with Veronda-Westmann matrix
class FSEFDVerondaWestmann : public FSMaterial
{
public:
	enum
	{
		MP_C1, MP_C2, MP_K,
		MP_BETA, MP_KSI
	};

public:
	FSEFDVerondaWestmann(FSModel* fem);

	DECLARE_REGISTERED(FSEFDVerondaWestmann);
};

//-----------------------------------------------------------------------------
class FSIsotropicLeeSacks : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_C0, MP_C1, MP_C2, MP_K, MP_TANGENT_SCALE };

public:
    FSIsotropicLeeSacks(FSModel* fem);

	DECLARE_REGISTERED(FSIsotropicLeeSacks);
};

//-----------------------------------------------------------------------------
class FSIsotropicLeeSacksUncoupled : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_C0, MP_C1, MP_C2, MP_K, MP_TANGENT_SCALE };

public:
    FSIsotropicLeeSacksUncoupled(FSModel* fem);

	DECLARE_REGISTERED(FSIsotropicLeeSacksUncoupled);
};

//-----------------------------------------------------------------------------
// Isotropic Fourier
class FSIsotropicFourier : public FSMaterial
{
public:
	enum { MP_DENSITY, MP_K, MP_C };

public:
	FSIsotropicFourier(FSModel* fem);
	DECLARE_REGISTERED(FSIsotropicFourier);
};

//-----------------------------------------------------------------------------
// Constant permeability
class FSPermConst : public FSMaterialProp
{
public:
	enum { MP_PERM };
public:
	FSPermConst(FSModel* fem);
	DECLARE_REGISTERED(FSPermConst);
};

//-----------------------------------------------------------------------------
// Holmes-Mow permeability
class FSPermHolmesMow : public FSMaterialProp
{
public:
	enum { MP_PERM, MP_M, MP_ALPHA };
public:
	FSPermHolmesMow(FSModel* fem);
	DECLARE_REGISTERED(FSPermHolmesMow);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss isotropic permeability
class FSPermAteshianWeissIso : public FSMaterialProp
{
public:
	enum { MP_PERM0, MP_PERM1, MP_PERM2, MP_M, MP_ALPHA };
public:
	FSPermAteshianWeissIso(FSModel* fem);
	DECLARE_REGISTERED(FSPermAteshianWeissIso);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss trans-isotropic permeability
class FSPermAteshianWeissTransIso : public FSMaterialProp
{
public:
	enum { MP_PERM0, MP_PERM1T, MP_PERM1A, MP_PERM2T, MP_PERM2A, MP_M0, MP_MT, MP_MA, MP_ALPHA0, MP_ALPHAT, MP_ALPHAA };
public:
	FSPermAteshianWeissTransIso(FSModel* fem);
	DECLARE_REGISTERED(FSPermAteshianWeissTransIso);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss orthotropic permeability
class FSPermAteshianWeissOrtho : public FSMaterialProp
{
public:
	enum { MP_PERM0, MP_PERM1, MP_PERM2, MP_M0, MP_ALPHA0, MP_M, MP_ALPHA };

public:
	FSPermAteshianWeissOrtho(FSModel* fem);
	DECLARE_REGISTERED(FSPermAteshianWeissOrtho);
};

//-----------------------------------------------------------------------------
// exponential isotropic permeability
class FSPermExpIso : public FSMaterialProp
{
public:
    enum { MP_PERM, MP_M };
public:
    FSPermExpIso(FSModel* fem);
    DECLARE_REGISTERED(FSPermExpIso);
};

//-----------------------------------------------------------------------------
// Constant diffusivity
class FSDiffConst : public FSMaterialProp
{
public:
	enum { MP_DIFF_FREE, MP_DIFF};

public:
	FSDiffConst(FSModel* fem);
	DECLARE_REGISTERED(FSDiffConst);
};

//-----------------------------------------------------------------------------
// orthotropic diffusivity
class FSDiffOrtho : public FSMaterialProp
{
public:
	enum { MP_DIFF_FREE, MP_DIFF};

public:
	FSDiffOrtho(FSModel* fem);
	DECLARE_REGISTERED(FSDiffOrtho);
};

//-----------------------------------------------------------------------------
// Ateshian-Weiss isotropic diffusivity
class FSDiffAteshianWeissIso : public FSMaterialProp
{
public:
	enum { MP_DIFF_FREE, MP_DIFF0, MP_DIFF1, MP_DIFF2, MP_M, MP_ALPHA };

public:
	FSDiffAteshianWeissIso(FSModel* fem);
	DECLARE_REGISTERED(FSDiffAteshianWeissIso);
};

//-----------------------------------------------------------------------------
// Albro isotropic diffusivity
class FSDiffAlbroIso : public FSMaterialProp
{
public:
	enum { MP_DIFF_FREE, MP_CDINV, MP_ALPHAD };
    
public:
	FSDiffAlbroIso(FSModel* fem);
	DECLARE_REGISTERED(FSDiffAlbroIso);
};

//-----------------------------------------------------------------------------
// Constant solubility
class FSSolubConst : public FSMaterialProp
{
public:
	enum { MP_SOLUB };

public:
	FSSolubConst(FSModel* fem);
	DECLARE_REGISTERED(FSSolubConst);
};

//-----------------------------------------------------------------------------
// Constant osmotic coefficient
class FSOsmoConst : public FSMaterialProp
{
public:
	enum { MP_OSMO };

public:
	FSOsmoConst(FSModel* fem);
	DECLARE_REGISTERED(FSOsmoConst);
};

//-----------------------------------------------------------------------------
// Wells-Manning osmotic coefficient
class FSOsmoWellsManning : public FSMaterialProp
{
public:
    enum { MP_KSI, MP_COION };

public:
    FSOsmoWellsManning(FSModel* fem);

    // get/set co-ion index
    void SetCoIonIndex(int i) { SetIntValue(MP_COION, i); }
    int GetCoIonIndex() { return GetIntValue(MP_COION); }
    
    DECLARE_REGISTERED(FSOsmoWellsManning);
};

//-----------------------------------------------------------------------------
// SFD compressible
class FSSFDCoupled : public FSMaterial
{
public:
	enum {MP_ALPHA, MP_BETA, MP_KSI};
public:
	FSSFDCoupled(FSModel* fem);
	DECLARE_REGISTERED(FSSFDCoupled);
};

//-----------------------------------------------------------------------------
// SFD SBM
class FSSFDSBM : public FSMaterial
{
public:
	enum {MP_ALPHA, MP_BETA, MP_KSI0, MP_RHO0, MP_GAMMA, MP_SBM};
public:
	FSSFDSBM(FSModel* fem);
	DECLARE_REGISTERED(FSSFDSBM);
};

//-----------------------------------------------------------------------------
// EFD compressible
class FSEFDCoupled : public FSMaterial
{
public:
	enum {MP_BETA, MP_KSI};
public:
	FSEFDCoupled(FSModel* fem);
	DECLARE_REGISTERED(FSEFDCoupled);
};

//-----------------------------------------------------------------------------
// EFD uncoupled
class FSEFDUncoupled : public FSMaterial
{
public:
	enum {MP_BETA, MP_KSI, MP_K};
public:
	FSEFDUncoupled(FSModel* fem);
	DECLARE_REGISTERED(FSEFDUncoupled);
};

//=============================================================================
// Obsolete Fiber materials. Retained for backward compatibility.
//=============================================================================

//-----------------------------------------------------------------------------
class FSFiberExpPowOld : public FSMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FSFiberExpPowOld(FSModel* fem);
//	DECLARE_REGISTERED(FSFiberExpPow);
};

//-----------------------------------------------------------------------------
class FSFiberExpPowUncoupledOld : public FSMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_K, MP_THETA, MP_PHI };
public:
	FSFiberExpPowUncoupledOld(FSModel* fem);
//	DECLARE_REGISTERED(FSFiberExpPowUncoupled);
};

//-----------------------------------------------------------------------------
class FSFiberPowLinOld : public FSMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_THETA, MP_PHI };
public:
    FSFiberPowLinOld(FSModel* fem);
//    DECLARE_REGISTERED(FSFiberPowLin);
};

//-----------------------------------------------------------------------------
class FSFiberPowLinUncoupledOld : public FSMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_K, MP_THETA, MP_PHI };
public:
    FSFiberPowLinUncoupledOld(FSModel* fem);
//    DECLARE_REGISTERED(FSFiberPowLinUncoupled);
};

//=============================================================================
// Fiber materials
//=============================================================================

//-----------------------------------------------------------------------------
// Base class manages the fiber generator
class FSFiberMaterial : public FSMaterial
{
public:
	FSFiberMaterial(int ntype, FSModel* fem);

	bool HasFibers() override;

	vec3d GetFiber(FEElementRef& el) override;

	void SetFiberGenerator(FSFiberGenerator* v);

	FSFiberGenerator* GetFiberGenerator();

	void SetAxisMaterial(FSAxisMaterial* Q) override;
};

//-----------------------------------------------------------------------------
class FSFiberExpPow : public FSFiberMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, M_L0 };

public:
    FSFiberExpPow(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSFiberExpPowOld* mat);
    
    DECLARE_REGISTERED(FSFiberExpPow);
};

//-----------------------------------------------------------------------------
class FSFiberExpPowUncoupled : public FSFiberMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_K };
public:
    FSFiberExpPowUncoupled(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSFiberExpPowUncoupledOld* mat);
    
    DECLARE_REGISTERED(FSFiberExpPowUncoupled);
};

//-----------------------------------------------------------------------------
class FSFiberPowLin : public FSFiberMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0 };
public:
    FSFiberPowLin(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSFiberPowLinOld* mat);
    
    DECLARE_REGISTERED(FSFiberPowLin);
};

//-----------------------------------------------------------------------------
class FSFiberPowLinUncoupled : public FSFiberMaterial
{
public:
    enum { MP_E, MP_BETA, MP_LAM0, MP_K };
public:
    FSFiberPowLinUncoupled(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSFiberPowLinUncoupledOld* mat);
    
    DECLARE_REGISTERED(FSFiberPowLinUncoupled);
};

//-----------------------------------------------------------------------------
class FSFiberExpLinear : public FSFiberMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FSFiberExpLinear(FSModel* fem);
	DECLARE_REGISTERED(FSFiberExpLinear);
};

//-----------------------------------------------------------------------------
class FSFiberExpLinearUncoupled : public FSFiberMaterial
{
public:
	enum { MP_ALPHA, MP_BETA, MP_KSI, MP_THETA, MP_PHI };
public:
	FSFiberExpLinearUncoupled(FSModel* fem);
	DECLARE_REGISTERED(FSFiberExpLinearUncoupled);
};

//-----------------------------------------------------------------------------
class FSFiberExpPowLin : public FSFiberMaterial
{
public:
    enum { MP_E, MP_ALPHA, MP_BETA, MP_LAM0 };
public:
    FSFiberExpPowLin(FSModel* fem);
    
    DECLARE_REGISTERED(FSFiberExpPowLin);
};

//-----------------------------------------------------------------------------
class FSFiberNeoHookean : public FSFiberMaterial
{
public:
    enum { MP_MU };
public:
    FSFiberNeoHookean(FSModel* fem);
    
    DECLARE_REGISTERED(FSFiberNeoHookean);
};

//-----------------------------------------------------------------------------
class FSFiberNaturalNH : public FSFiberMaterial
{
public:
    enum { MP_KSI, MP_LAM0 };
public:
    FSFiberNaturalNH(FSModel* fem);
    
    DECLARE_REGISTERED(FSFiberNaturalNH);
};

//-----------------------------------------------------------------------------
class FSFiberDamagePower : public FSFiberMaterial
{
public:
	enum { MP_ALPHA1, MP_ALPHA2, MP_KAPPA, MP_T0, MP_DMAX, MP_BETA_S, MP_GAMMA_MAX };
public:
	FSFiberDamagePower(FSModel* fem);
	DECLARE_REGISTERED(FSFiberDamagePower);
};

//-----------------------------------------------------------------------------
class FSFiberDamageExponential : public FSFiberMaterial
{
public:
	enum { MP_K1, MP_K2, MP_KAPPA, MP_T0, MP_DMAX, MP_BETA_S, MP_GAMMA_MAX };
public:
	FSFiberDamageExponential(FSModel* fem);
	DECLARE_REGISTERED(FSFiberDamageExponential);
};

//-----------------------------------------------------------------------------
class FSFiberDamageExpLinear : public FSFiberMaterial
{
public:
	enum { MP_C3, MP_C4, MP_C5, MP_LAMBDA, MP_T0, MP_DMAX, MP_BETA_S, MP_GAMMA_MAX };
public:
	FSFiberDamageExpLinear(FSModel* fem);
	DECLARE_REGISTERED(FSFiberDamageExpLinear);
};

//-----------------------------------------------------------------------------
class FSFiberKiousisUncoupled : public FSFiberMaterial
{
public:
    enum { MP_D1, MP_D2, M_N };
public:
    FSFiberKiousisUncoupled(FSModel* fem);
    
    DECLARE_REGISTERED(FSFiberKiousisUncoupled);
};

//=============================================================================

//-----------------------------------------------------------------------------
// CLE cubic
//
class FSCubicCLE : public FSMaterial
{
public:
    enum { MP_DENSITY, MP_LP1, MP_LM1, MP_L2, MP_MU };
    
public:
    FSCubicCLE(FSModel* fem);
    
    DECLARE_REGISTERED(FSCubicCLE);
};

//-----------------------------------------------------------------------------
// CLE orthotropic
//
class FSOrthotropicCLE : public FSMaterial
{
public:
    enum { MP_DENSITY,
        MP_LP11, MP_LP22, MP_LP33,
        MP_LM11, MP_LM22, MP_LM33,
        MP_L12, MP_L23, MP_L31,
        MP_MU1, MP_MU2, MP_MU3
    };
    
public:
    FSOrthotropicCLE(FSModel* fem);
    
    DECLARE_REGISTERED(FSOrthotropicCLE);
};

//-----------------------------------------------------------------------------
class FEHGOCoronary : public FSTransverselyIsotropic
{
public:
    FEHGOCoronary(FSModel* fem);
    DECLARE_REGISTERED(FEHGOCoronary);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction
//
class FSPrescribedActiveContractionUniaxialOld : public FSMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FSPrescribedActiveContractionUniaxialOld(FSModel* fem);
    
//    DECLARE_REGISTERED(FSPrescribedActiveContractionUniaxial);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction
//
class FSPrescribedActiveContractionTransIsoOld : public FSMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FSPrescribedActiveContractionTransIsoOld(FSModel* fem);
    
//    DECLARE_REGISTERED(FSPrescribedActiveContractionTransIso);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction
//
class FSPrescribedActiveContractionUniaxial : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionUniaxial(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSPrescribedActiveContractionUniaxialOld* mat);

    DECLARE_REGISTERED(FSPrescribedActiveContractionUniaxial);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction
//
class FSPrescribedActiveContractionTransIso : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionTransIso(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSPrescribedActiveContractionTransIsoOld* mat);

    DECLARE_REGISTERED(FSPrescribedActiveContractionTransIso);
};

//-----------------------------------------------------------------------------
// Prescribed isotropic active contraction
//
class FSPrescribedActiveContractionIsotropic : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionIsotropic(FSModel* fem);
    
    DECLARE_REGISTERED(FSPrescribedActiveContractionIsotropic);
};

//-----------------------------------------------------------------------------
// Prescribed fiber active contraction
//
class FSPrescribedActiveContractionFiber : public FSTransverselyIsotropic
{
public:
    enum { MP_T0 };

public:
    FSPrescribedActiveContractionFiber(FSModel* fem);
    DECLARE_REGISTERED(FSPrescribedActiveContractionFiber);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction uncoupled
//
class FSPrescribedActiveContractionUniaxialUCOld : public FSMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FSPrescribedActiveContractionUniaxialUCOld(FSModel* fem);
    
//    DECLARE_REGISTERED(FSPrescribedActiveContractionUniaxialUC);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction uncoupled
//
class FSPrescribedActiveContractionTransIsoUCOld : public FSMaterial
{
public:
    enum { MP_T0, MP_TH, MP_PH };
    
public:
    FSPrescribedActiveContractionTransIsoUCOld(FSModel* fem);
    
//    DECLARE_REGISTERED(FSPrescribedActiveContractionTransIsoUC);
};

//-----------------------------------------------------------------------------
// Prescribed uniaxial active contraction uncoupled
//
class FSPrescribedActiveContractionUniaxialUC : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionUniaxialUC(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSPrescribedActiveContractionUniaxialUCOld* mat);

    DECLARE_REGISTERED(FSPrescribedActiveContractionUniaxialUC);
};

//-----------------------------------------------------------------------------
// Prescribed fiber active contraction
//
class FSPrescribedActiveContractionFiberUC : public FSTransverselyIsotropic
{
public:
    enum { MP_T0 };

public:
    FSPrescribedActiveContractionFiberUC(FSModel* fem);
    DECLARE_REGISTERED(FSPrescribedActiveContractionFiberUC);
};

//-----------------------------------------------------------------------------
// Prescribed transversely isotropic active contraction uncoupled
//
class FSPrescribedActiveContractionTransIsoUC : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionTransIsoUC(FSModel* fem);
    
    // convert from the old to the new format
    void Convert(FSPrescribedActiveContractionTransIsoUCOld* mat);

    DECLARE_REGISTERED(FSPrescribedActiveContractionTransIsoUC);
};

//-----------------------------------------------------------------------------
// Prescribed isotropic active contraction uncoupled
//
class FSPrescribedActiveContractionIsotropicUC : public FSMaterial
{
public:
    enum { MP_T0 };
    
public:
    FSPrescribedActiveContractionIsotropicUC(FSModel* fem);
    
    DECLARE_REGISTERED(FSPrescribedActiveContractionIsotropicUC);
};

//-----------------------------------------------------------------------------
class FSDonnanSwelling : public FSMaterial
{
public:
	enum { MP_PHIW0, MP_CF0, MP_BOSM, MP_PHI };
public:
	FSDonnanSwelling(FSModel* fem);
	DECLARE_REGISTERED(FSDonnanSwelling);
};

//-----------------------------------------------------------------------------
class FSPerfectOsmometer : public FSMaterial
{
public:
	enum { MP_PHIW0, MP_IOSM, MP_BOSM };
public:
	FSPerfectOsmometer(FSModel* fem);
	DECLARE_REGISTERED(FSPerfectOsmometer);
};

//-----------------------------------------------------------------------------
class FSCellGrowth : public FSMaterial
{
public:
	enum { MP_PHIR, MP_CR, MP_CE };
public:
	FSCellGrowth(FSModel* fem);
	DECLARE_REGISTERED(FSCellGrowth);
};

//-----------------------------------------------------------------------------
class FSOsmoticVirial : public FSMaterial
{
public:
    enum { MP_PHIW0, MP_CR, MP_C1, MP_C2, MP_c3 };
public:
    FSOsmoticVirial(FSModel* fem);
    DECLARE_REGISTERED(FSOsmoticVirial);
};

//-----------------------------------------------------------------------------
class FSReactionRateConst : public FSMaterialProp
{
public:
	enum { MP_K };

	double GetRateConstant();
	void SetRateConstant(double K);

public:
	FSReactionRateConst(FSModel* fem);
	DECLARE_REGISTERED(FSReactionRateConst);
};

//-----------------------------------------------------------------------------
class FSReactionRateHuiskes : public FSMaterialProp
{
public:
	enum { MP_B, MP_PSI0 };
public:
	FSReactionRateHuiskes(FSModel* fem);
	DECLARE_REGISTERED(FSReactionRateHuiskes);
};

//-----------------------------------------------------------------------------
class FEBioReactionRate : public FSMaterialProp
{
public:
	FEBioReactionRate(FSModel* fem);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
	DECLARE_REGISTERED(FEBioReactionRate);
};

//-----------------------------------------------------------------------------
class FSMembraneReactionRateConst : public FSMaterialProp
{
public:
    enum { MP_K };
    
    double GetRateConstant();
    void SetRateConstant(double K);
    
public:
    FSMembraneReactionRateConst(FSModel* fem);
    DECLARE_REGISTERED(FSMembraneReactionRateConst);
};

//-----------------------------------------------------------------------------
class FSMembraneReactionRateIonChannel : public FSMaterialProp
{
public:
    enum { MP_G, MP_SOL };
    
    double GetConductivity();
    void SetConductivity(double g);
    int GetSolute();
    void SetSolute(int isol);
    
public:
    FSMembraneReactionRateIonChannel(FSModel* fem);
    DECLARE_REGISTERED(FSMembraneReactionRateIonChannel);
};

//-----------------------------------------------------------------------------
class FSMembraneReactionRateVoltageGated : public FSMaterialProp
{
public:
    enum { MP_A, MP_B, MP_C, MP_D, MP_SOL };
    
    double GetConstant(int i);
    void SetConstant(int i, double c);
    int GetSolute();
    void SetSolute(int isol);
    
public:
    FSMembraneReactionRateVoltageGated(FSModel* fem);
    DECLARE_REGISTERED(FSMembraneReactionRateVoltageGated);
};

//-----------------------------------------------------------------------------
class FSCFDFiberExpPow : public FSMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_MU };
public:
    FSCFDFiberExpPow(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberExpPow);
};

//-----------------------------------------------------------------------------
class FSCFDFiberNH : public FSMaterial
{
public:
    enum { MP_MU };
public:
    FSCFDFiberNH(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberNH);
};

//-----------------------------------------------------------------------------
class FSCFDFiberPowLinear : public FSMaterial
{
public:
    enum { MP_E, MP_B, MP_L0 };
public:
    FSCFDFiberPowLinear(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberPowLinear);
};

//-----------------------------------------------------------------------------
class FSCFDFiberExpPowUC : public FSMaterial
{
public:
    enum { MP_ALPHA, MP_BETA, MP_KSI, MP_MU, MP_K };
public:
    FSCFDFiberExpPowUC(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberExpPowUC);
};

//-----------------------------------------------------------------------------
class FSCFDFiberNHUC : public FSMaterial
{
public:
    enum { MP_MU, MP_K };
public:
    FSCFDFiberNHUC(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberNHUC);
};

//-----------------------------------------------------------------------------
class FSCFDFiberPowLinearUC : public FSMaterial
{
public:
    enum { MP_E, MP_B, MP_L0, MP_K };
public:
    FSCFDFiberPowLinearUC(FSModel* fem);
    DECLARE_REGISTERED(FSCFDFiberPowLinearUC);
};

//-----------------------------------------------------------------------------
class FSFDDSpherical : public FSMaterialProp
{
public:
    FSFDDSpherical(FSModel* fem);
    DECLARE_REGISTERED(FSFDDSpherical);
};

//-----------------------------------------------------------------------------
class FSFDDEllipsoidal : public FSMaterialProp
{
public:
    enum { MP_SPA };
public:
    FSFDDEllipsoidal(FSModel* fem);
    DECLARE_REGISTERED(FSFDDEllipsoidal);
};

//-----------------------------------------------------------------------------
class FSFDDvonMises3d : public FSMaterialProp
{
public:
    enum { MP_B };
public:
    FSFDDvonMises3d(FSModel* fem);
    DECLARE_REGISTERED(FSFDDvonMises3d);
};

//-----------------------------------------------------------------------------
class FSFDDCircular : public FSMaterialProp
{
public:
    FSFDDCircular(FSModel* fem);
    DECLARE_REGISTERED(FSFDDCircular);
};

//-----------------------------------------------------------------------------
class FSFDDElliptical : public FSMaterialProp
{
public:
    enum { MP_SPA1, M_SPA2 };
public:
    FSFDDElliptical(FSModel* fem);
    DECLARE_REGISTERED(FSFDDElliptical);
};

//-----------------------------------------------------------------------------
class FSFDDvonMises2d : public FSMaterialProp
{
public:
    enum { MP_B };
public:
    FSFDDvonMises2d(FSModel* fem);
    DECLARE_REGISTERED(FSFDDvonMises2d);
};

//-----------------------------------------------------------------------------
class FSSchemeGKT : public FSMaterialProp
{
public:
    enum { MP_NPH, M_NTH };
public:
    FSSchemeGKT(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeGKT);
};

//-----------------------------------------------------------------------------
class FSSchemeFEI : public FSMaterialProp
{
public:
    enum { MP_RES };
public:
    FSSchemeFEI(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeFEI);
};

//-----------------------------------------------------------------------------
class FSSchemeT2d : public FSMaterialProp
{
public:
    enum { M_NTH };
public:
    FSSchemeT2d(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeT2d);
};

//-----------------------------------------------------------------------------
class FSSchemeGKTUC : public FSMaterialProp
{
public:
    enum { MP_NPH, M_NTH };
public:
    FSSchemeGKTUC(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeGKTUC);
};

//-----------------------------------------------------------------------------
class FSSchemeFEIUC : public FSMaterialProp
{
public:
    enum { MP_RES };
public:
    FSSchemeFEIUC(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeFEIUC);
};

//-----------------------------------------------------------------------------
class FSSchemeT2dUC : public FSMaterialProp
{
public:
    enum { M_NTH };
public:
    FSSchemeT2dUC(FSModel* fem);
    DECLARE_REGISTERED(FSSchemeT2dUC);
};

//-----------------------------------------------------------------------------
class FSCDFSimo : public FSMaterialProp
{
public:
    enum { MP_A, MP_B };
public:
    FSCDFSimo(FSModel* fem);
    DECLARE_REGISTERED(FSCDFSimo);
};

//-----------------------------------------------------------------------------
class FSCDFLogNormal : public FSMaterialProp
{
public:
    enum { MP_MU, MP_SIGMA, MP_DMAX };
public:
    FSCDFLogNormal(FSModel* fem);
    DECLARE_REGISTERED(FSCDFLogNormal);
};

//-----------------------------------------------------------------------------
class FSCDFWeibull : public FSMaterialProp
{
public:
    enum { MP_ALPHA, MP_MU, MP_DMAX };
public:
    FSCDFWeibull(FSModel* fem);
    DECLARE_REGISTERED(FSCDFWeibull);
};

//-----------------------------------------------------------------------------
class FSCDFStep : public FSMaterialProp
{
public:
    enum { MP_MU, MP_DMAX };
public:
    FSCDFStep(FSModel* fem);
    DECLARE_REGISTERED(FSCDFStep);
};

//-----------------------------------------------------------------------------
class FSCDFQuintic : public FSMaterialProp
{
public:
    enum { MP_MUMIN, MP_MUMAX, MP_DMAX };
public:
    FSCDFQuintic(FSModel* fem);
    DECLARE_REGISTERED(FSCDFQuintic);
};

//-----------------------------------------------------------------------------
class FSCDFPower : public FSMaterialProp
{
public:
    enum { MP_ALPHA, MP_MU0, MP_MU1 };
public:
    FSCDFPower(FSModel* fem);
    DECLARE_REGISTERED(FSCDFPower);
};

//-----------------------------------------------------------------------------
class FSDCSimo : public FSMaterialProp
{
public:
    FSDCSimo(FSModel* fem);
    DECLARE_REGISTERED(FSDCSimo);
};

//-----------------------------------------------------------------------------
class FSDCStrainEnergyDensity : public FSMaterialProp
{
public:
    FSDCStrainEnergyDensity(FSModel* fem);
    DECLARE_REGISTERED(FSDCStrainEnergyDensity);
};

//-----------------------------------------------------------------------------
class FSDCSpecificStrainEnergy : public FSMaterialProp
{
public:
    FSDCSpecificStrainEnergy(FSModel* fem);
    DECLARE_REGISTERED(FSDCSpecificStrainEnergy);
};

//-----------------------------------------------------------------------------
class FSDCvonMisesStress : public FSMaterialProp
{
public:
    FSDCvonMisesStress(FSModel* fem);
    DECLARE_REGISTERED(FSDCvonMisesStress);
};

//-----------------------------------------------------------------------------
class FSDCDruckerShearStress : public FSMaterialProp
{
public:
    enum { MP_C };
public:
    FSDCDruckerShearStress(FSModel* fem);
    DECLARE_REGISTERED(FSDCDruckerShearStress);
};

//-----------------------------------------------------------------------------
class FSDCMaxShearStress : public FSMaterialProp
{
public:
    FSDCMaxShearStress(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxShearStress);
};

//-----------------------------------------------------------------------------
class FSDCMaxNormalStress : public FSMaterialProp
{
public:
    FSDCMaxNormalStress(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxNormalStress);
};

//-----------------------------------------------------------------------------
class FSDCMaxNormalLagrangeStrain : public FSMaterialProp
{
public:
    FSDCMaxNormalLagrangeStrain(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxNormalLagrangeStrain);
};

//-----------------------------------------------------------------------------
class FSDCOctahedralShearStrain : public FSMaterialProp
{
public:
    FSDCOctahedralShearStrain(FSModel* fem);
    DECLARE_REGISTERED(FSDCOctahedralShearStrain);
};

//-----------------------------------------------------------------------------
class FSDCSimoUC : public FSMaterialProp
{
public:
    FSDCSimoUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCSimoUC);
};

//-----------------------------------------------------------------------------
class FSDCStrainEnergyDensityUC : public FSMaterialProp
{
public:
    FSDCStrainEnergyDensityUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCStrainEnergyDensityUC);
};

//-----------------------------------------------------------------------------
class FSDCSpecificStrainEnergyUC : public FSMaterialProp
{
public:
    FSDCSpecificStrainEnergyUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCSpecificStrainEnergyUC);
};

//-----------------------------------------------------------------------------
class FSDCvonMisesStressUC : public FSMaterialProp
{
public:
    FSDCvonMisesStressUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCvonMisesStressUC);
};

//-----------------------------------------------------------------------------
class FSDCMaxShearStressUC : public FSMaterialProp
{
public:
    FSDCMaxShearStressUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxShearStressUC);
};

//-----------------------------------------------------------------------------
class FSDCMaxNormalStressUC : public FSMaterialProp
{
public:
    FSDCMaxNormalStressUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxNormalStressUC);
};

//-----------------------------------------------------------------------------
class FSDCMaxNormalLagrangeStrainUC : public FSMaterialProp
{
public:
    FSDCMaxNormalLagrangeStrainUC(FSModel* fem);
    DECLARE_REGISTERED(FSDCMaxNormalLagrangeStrainUC);
};

//-----------------------------------------------------------------------------
class FSRelaxCSExp : public FSMaterialProp
{
public:
    enum { MP_TAU };
public:
    FSRelaxCSExp(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxCSExp);
};

//-----------------------------------------------------------------------------
class FSRelaxCSExpDistUser : public FSMaterialProp
{
public:
	FSRelaxCSExpDistUser(FSModel* fem);
	DECLARE_REGISTERED(FSRelaxCSExpDistUser);
};


//-----------------------------------------------------------------------------
class FSRelaxExp : public FSMaterialProp
{
public:
    enum { MP_TAU };
public:
    FSRelaxExp(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxExp);
};

//-----------------------------------------------------------------------------
class FSRelaxExpDistortion : public FSMaterialProp
{
public:
    enum { MP_TAU, M_TAU1, M_ALPHA };
public:
    FSRelaxExpDistortion(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxExpDistortion);
};

//-----------------------------------------------------------------------------
class FSRelaxExpDistUser : public FSMaterialProp
{
public:
	FSRelaxExpDistUser(FSModel* fem);
	DECLARE_REGISTERED(FSRelaxExpDistUser);
};

//-----------------------------------------------------------------------------
class FSRelaxFung : public FSMaterialProp
{
public:
    enum { MP_TAU1, M_TAU2 };
public:
    FSRelaxFung(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxFung);
};

//-----------------------------------------------------------------------------
class FSRelaxMalkin : public FSMaterialProp
{
public:
    enum { MP_TAU1, M_TAU2, M_BETA };
public:
    FSRelaxMalkin(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxMalkin);
};

//-----------------------------------------------------------------------------
class FSRelaxMalkinDistortion : public FSMaterialProp
{
public:
    enum { MP_T1C0, MP_T1C1, MP_T1S0, MP_T2C0, MP_T2C1, MP_T2S0, M_BETA };
public:
    FSRelaxMalkinDistortion(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxMalkinDistortion);
};

//-----------------------------------------------------------------------------
class FSRelaxPark : public FSMaterialProp
{
public:
    enum { MP_TAU, M_BETA };
public:
    FSRelaxPark(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxPark);
};

//-----------------------------------------------------------------------------
class FSRelaxParkDistortion : public FSMaterialProp
{
public:
    enum { MP_TAU, M_TAU1, M_BETA, M_BETA1, M_ALPHA };
public:
    FSRelaxParkDistortion(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxParkDistortion);
};

//-----------------------------------------------------------------------------
class FSRelaxPow : public FSMaterialProp
{
public:
    enum { MP_TAU, M_BETA };
public:
    FSRelaxPow(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxPow);
};

//-----------------------------------------------------------------------------
class FSRelaxPowDistortion : public FSMaterialProp
{
public:
    enum { MP_TAU, M_TAU1, M_BETA, M_BETA1, M_ALPHA };
public:
    FSRelaxPowDistortion(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxPowDistortion);
};

//-----------------------------------------------------------------------------
class FSRelaxPowDistUser : public FSMaterialProp
{
public:
	enum { MP_TAU, M_BETA };

public:
	FSRelaxPowDistUser(FSModel* fem);
	DECLARE_REGISTERED(FSRelaxPowDistUser);
};

//-----------------------------------------------------------------------------
class FSRelaxProny : public FSMaterialProp
{
public:
    // max nr of Prony terms
    enum { MAX_TERMS = 6 };
    
    // material parameters
    enum {
        MP_G1, MP_G2, MP_G3, MP_G4, MP_G5, MP_G6,
        MP_T1, MP_T2, MP_T3, MP_T4, MP_T5, MP_T6
    };
public:
    FSRelaxProny(FSModel* fem);
    DECLARE_REGISTERED(FSRelaxProny);
};

//-----------------------------------------------------------------------------
class FSRelaxMalkinDistUser: public FSMaterialProp
{
public:
	FSRelaxMalkinDistUser(FSModel* fem);
	DECLARE_REGISTERED(FSRelaxMalkinDistUser);
};

//-----------------------------------------------------------------------------
// Elastic pressure for ideal gas
class FSEPIdealGas : public FSMaterialProp
{
public:
    enum { MP_M };
public:
    FSEPIdealGas(FSModel* fem);
    DECLARE_REGISTERED(FSEPIdealGas);
};

//-----------------------------------------------------------------------------
// Elastic pressure for ideal fluid
class FSEPIdealFluid : public FSMaterialProp
{
public:
    enum { MP_K };
public:
    FSEPIdealFluid(FSModel* fem);
    DECLARE_REGISTERED(FSEPIdealFluid);
};

//-----------------------------------------------------------------------------
// Elastic pressure for neo-Hookean fluid
class FSEPNeoHookeanFluid : public FSMaterialProp
{
public:
    enum { MP_K };
public:
    FSEPNeoHookeanFluid(FSModel* fem);
    DECLARE_REGISTERED(FSEPNeoHookeanFluid);
};

//-----------------------------------------------------------------------------
// Viscous Newtonian fluid
class FSVFNewtonian : public FSMaterialProp
{
public:
    enum { MP_MU, MP_K };
public:
    FSVFNewtonian(FSModel* fem);
    DECLARE_REGISTERED(FSVFNewtonian);
};

//-----------------------------------------------------------------------------
// Viscous Bingham fluid
class FSVFBingham : public FSMaterialProp
{
public:
    enum { MP_MU, MP_TAUY, MP_N };
public:
    FSVFBingham(FSModel* fem);
    DECLARE_REGISTERED(FSVFBingham);
};

//-----------------------------------------------------------------------------
// Viscous Carreau fluid
class FSVFCarreau : public FSMaterialProp
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_N };
public:
    FSVFCarreau(FSModel* fem);
    DECLARE_REGISTERED(FSVFCarreau);
};

//-----------------------------------------------------------------------------
// Viscous Carreau-Yasuda fluid
class FSVFCarreauYasuda : public FSMaterialProp
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_N, MP_A };
public:
    FSVFCarreauYasuda(FSModel* fem);
    DECLARE_REGISTERED(FSVFCarreauYasuda);
};

//-----------------------------------------------------------------------------
// Viscous Powell-Eyring fluid
class FSVFPowellEyring : public FSMaterialProp
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM };
public:
    FSVFPowellEyring(FSModel* fem);
    DECLARE_REGISTERED(FSVFPowellEyring);
};

//-----------------------------------------------------------------------------
// Viscous Cross fluid
class FSVFCross : public FSMaterialProp
{
public:
    enum { MP_MU0, MP_MUI, MP_LAM, MP_M };
public:
    FSVFCross(FSModel* fem);
    DECLARE_REGISTERED(FSVFCross);
};

//-----------------------------------------------------------------------------
// Linear Polar Viscous fluid
class FSVPFLinear : public FSMaterialProp
{
public:
    enum { MP_TAU, MP_ALPHA, MP_BETA, MP_GAMMA };
public:
    FSVPFLinear(FSModel* fem);
    DECLARE_REGISTERED(FSVPFLinear);
};

//-----------------------------------------------------------------------------
class FSStarlingSupply : public FSMaterialProp
{
public:
	enum { MP_KP, MP_PV };
public:
	FSStarlingSupply(FSModel* fem);
	DECLARE_REGISTERED(FSStarlingSupply);
};

//-----------------------------------------------------------------------------
class FSPrestrainConstGradient : public FSMaterialProp
{
public:
	enum { MP_F0 };
public:
	FSPrestrainConstGradient(FSModel* fem);
	DECLARE_REGISTERED(FSPrestrainConstGradient);
};

//-----------------------------------------------------------------------------
class FSPrestrainInSituGradient : public FSMaterialProp
{
public:
	enum { MP_LAM, MP_ISO };
public:
	FSPrestrainInSituGradient(FSModel* fem);
	DECLARE_REGISTERED(FSPrestrainInSituGradient);
};

//-----------------------------------------------------------------------------
class FSPlasticFlowCurvePaper : public FSMaterialProp
{
public:
    enum { MP_Y0, MP_YM, MP_W0, MP_WE, MP_NF, MP_R };
public:
	FSPlasticFlowCurvePaper(FSModel* fem);
	DECLARE_REGISTERED(FSPlasticFlowCurvePaper);
};

//-----------------------------------------------------------------------------
class FSPlasticFlowCurveUser : public FSMaterialProp
{
public:
	FSPlasticFlowCurveUser(FSModel* fem);
	DECLARE_REGISTERED(FSPlasticFlowCurveUser);
};

//-----------------------------------------------------------------------------
class FSPlasticFlowCurveMath : public FSMaterialProp
{
public:
    enum { MP_NF, MP_E0, MP_EM, M_PR };
public:
	FSPlasticFlowCurveMath(FSModel* fem);
	DECLARE_REGISTERED(FSPlasticFlowCurveMath);
};

//-----------------------------------------------------------------------------
class FEBioMaterial : public FSMaterial
{
public:
	FEBioMaterial(FSModel* fem);
	~FEBioMaterial();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	bool IsRigid() override;

	bool HasFibers() override;

	vec3d GetFiber(FEElementRef& el) override;

	FSMaterial* Clone() override;

	// local material axes
	bool HasMaterialAxes() const override;
	mat3d GetMatAxes(FEElementRef& el) const override;

	bool UpdateData(bool bsave) override;

	// return a string for the material type
	const char* GetTypeString() const override;
	void SetTypeString(const std::string& s) override;

	DECLARE_REGISTERED(FEBioMaterial);
};
