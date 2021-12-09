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
#include "FEProject.h"
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include "GGroup.h"
#include "GModel.h"
#include <FEBioStudio/WebDefines.h>
#include <FEBioLink/FEBioModule.h>
#include <GeomLib/GObject.h>
#include <string>
//using namespace std;

//=================================================================================================
CLogDataSettings::CLogDataSettings()
{
}

void CLogDataSettings::RemoveLogData(int item)
{
	m_log.erase(m_log.begin() + item);
}

void CLogDataSettings::Save(OArchive& ar)
{
	const int N = (int) m_log.size();
	for (int i = 0; i<N; ++i)
	{
		FELogData& v = m_log[i];
		ar.BeginChunk(CID_PRJ_LOGDATA_ITEM);
		{
			ar.WriteChunk(CID_PRJ_LOGDATA_TYPE, v.type);
			ar.WriteChunk(CID_PRJ_LOGDATA_DATA, v.sdata);
			ar.WriteChunk(CID_PRJ_LOGDATA_MID , v.matID);
			ar.WriteChunk(CID_PRJ_LOGDATA_GID , v.groupID);
            ar.WriteChunk(CID_PRJ_LOGDATA_CID , v.rcID);
			ar.WriteChunk(CID_PRJ_LOGDATA_FILE, v.fileName);
		}
		ar.EndChunk();
	}
}

void CLogDataSettings::Load(IArchive& ar)
{
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		if (ar.GetChunkID() == CID_PRJ_LOGDATA_ITEM)
		{
			string data, file;
			int ntype;
			int mid = -1, gid = -1, cid = -1;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PRJ_LOGDATA_TYPE: ar.read(ntype); break;
				case CID_PRJ_LOGDATA_DATA: ar.read(data); break;
				case CID_PRJ_LOGDATA_MID : ar.read(mid); break;
				case CID_PRJ_LOGDATA_GID : ar.read(gid); break;
                case CID_PRJ_LOGDATA_CID : ar.read(cid); break;
				case CID_PRJ_LOGDATA_FILE: ar.read(file); break;
				}
				ar.CloseChunk();
			}

			FELogData d;
			d.type = ntype;
			d.sdata = data;
			d.matID = mid;
			d.groupID = gid;
            d.rcID = cid;
			d.fileName = file;
			AddLogData(d);
		}
		ar.CloseChunk();
	}
}

//=================================================================================================
//-----------------------------------------------------------------------------
// constructor
FEProject::FEProject(void) : m_plt(*this)
{
	// set the title
	m_title = "untitled";

	// activate all modules
	m_module = -1;

	static bool init = false;
	if (init == false)
	{
		InitModules();
		init = true;
	}
}

//-----------------------------------------------------------------------------
// destructor
FEProject::~FEProject(void)
{
	
}

//-----------------------------------------------------------------------------
void FEProject::Reset()
{
	m_fem.New();
	m_plt.Init();
}

//-----------------------------------------------------------------------------
unsigned int FEProject::GetModule() const
{
	return m_module;
}

//-----------------------------------------------------------------------------
void FEProject::SetModule(unsigned int mod)
{
	FEBio::SetActiveModule(mod);
	m_module = mod;

	if (m_plt.PlotVariables() == 0)
	{
		// add some default variables
		// TODO: Maybe I can pull this info from FEBio somehow
		SetDefaultPlotVariables();
	}
}

//-----------------------------------------------------------------------------
void FEProject::SetTitle(const std::string& title)
{ 
	m_title = title; 
}

//-----------------------------------------------------------------------------
// save project data to archive
void FEProject::Save(OArchive& ar)
{
	// save the title
	ar.WriteChunk(CID_PRJ_TITLE   , m_title);

	// save the modules flag
	string modName(FEBio::GetModuleName(m_module));
	ar.WriteChunk(CID_PRJ_MODULE_NAME, modName);

	// save the model data
	ar.BeginChunk(CID_FEM);
	{
		m_fem.Save(ar);
	}
	ar.EndChunk();

	// save the output data
	int N = m_plt.PlotVariables();
	if (N > 0)
	{
		ar.BeginChunk(CID_PRJ_OUTPUT);
		{
			m_plt.Save(ar);
		}
		ar.EndChunk();
	}

	// save the logfile data
	N = m_log.LogDataSize();
	if (N > 0)
	{
		ar.BeginChunk(CID_PRJ_LOGDATA);
		{
			m_log.Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
int MapOldToNewModules(int oldId)
{
	if (oldId & MODULE_FLUID_FSI  ) return FEBio::GetModuleId("fluid-FSI");
	if (oldId & MODULE_FLUID      ) return FEBio::GetModuleId("fluid");
	if (oldId & MODULE_MULTIPHASIC) return FEBio::GetModuleId("multiphasic");
	if (oldId & MODULE_BIPHASIC   ) return FEBio::GetModuleId("biphasic");
	if (oldId & MODULE_HEAT       ) return FEBio::GetModuleId("heat");
	if (oldId & MODULE_MECH       ) return FEBio::GetModuleId("solid");
	assert(false);
	return -1;
}

//-----------------------------------------------------------------------------
// load project data from archive
void FEProject::Load(IArchive &ar)
{
	TRACE("FEProject::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_PRJ_TITLE  : ar.read(m_title); break;
		case CID_PRJ_MODULES: { int oldModuleId = 0;  ar.read(oldModuleId); m_module = MapOldToNewModules(oldModuleId); } break;
		case CID_PRJ_MODULE_NAME:
		{
			string modName;
			ar.read(modName);
			int moduleId = FEBio::GetModuleId(modName); assert(m_module > 0);
			SetModule(moduleId);
		}
		break;
		case CID_FEM        : m_fem.Load(ar); break;
		case CID_PRJ_OUTPUT : m_plt.Load(ar); break;
		case CID_PRJ_LOGDATA: m_log.Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
// Validate the project, that is try to catch problems that FEBio won't like
// returns 0 on success, non-zero on failure
int FEProject::Validate(std::string &szerr)
{
	szerr = "ERRORS:\n";
	GModel& mdl = m_fem.GetModel();

	// --- collect metrics ---
	int nerrs = 0;

	// number of objects
	int nobj = mdl.Objects();
	if (nobj   ==  0) { szerr += "-No geometry is defined for this model.\n"; nerrs++; }

	// parts that don't have a material assigned
	int npnm = 0; 
	for (int i=0; i<mdl.Parts(); ++i)
	{
		GPart* pg = mdl.Part(i);
		if (pg->GetMaterialID() == -1) npnm++;
	}
	if (npnm   >   0) { szerr += "-Not all parts or objects have a material assigned.\n"; nerrs++; }

	// numer of analysis steps
	int nsteps = m_fem.Steps() - 1;
	if (nsteps <=  0) { szerr += "-At least one analysis step is required to run the model.\n"; nerrs++; }

	// number of objects that have no mesh
	int nnomesh = 0;
	for (int i=0; i<mdl.Objects(); ++i)
	{
		FEMesh* pm = mdl.Object(i)->GetFEMesh();
		if (pm == 0) nnomesh++;
	}
	if (nnomesh > 0) { szerr += "-Not all objects have a mesh.\n"; nerrs++; }

	return nerrs;
}

void FEProject::InitModules()
{
	// register material categories
	FEMaterialFactory::AddCategory("elastic"             , MODULE_MECH     , FE_MAT_ELASTIC);
	FEMaterialFactory::AddCategory("uncoupled elastic"   , MODULE_MECH     , FE_MAT_ELASTIC_UNCOUPLED);
	FEMaterialFactory::AddCategory("bi-/tri-/multiphasic", MODULE_BIPHASIC , FE_MAT_MULTIPHASIC);
	FEMaterialFactory::AddCategory("heat transfer"       , MODULE_HEAT     , FE_MAT_HEAT_TRANSFER);
	FEMaterialFactory::AddCategory("fluid"               , MODULE_FLUID    , FE_MAT_FLUID);
    FEMaterialFactory::AddCategory("fluid-FSI"           , MODULE_FLUID_FSI, FE_MAT_FLUID_FSI);
	FEMaterialFactory::AddCategory("reaction-diffusion"  , MODULE_REACTION_DIFFUSION	, FE_MAT_REACTION_DIFFUSION);
	FEMaterialFactory::AddCategory("other"               , MODULE_MECH				, FE_MAT_RIGID);

	// --- MECH MODULE ---
	REGISTER_FE_CLASS(FSNonLinearMechanics         , MODULE_MECH, FE_ANALYSIS         , FE_STEP_MECHANICS               , "Structural Mechanics");
	REGISTER_FE_CLASS(FSFixedDisplacement          , MODULE_MECH, FE_ESSENTIAL_BC     , FE_FIXED_DISPLACEMENT           , "Fixed displacement", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSFixedShellDisplacement     , MODULE_MECH, FE_ESSENTIAL_BC     , FE_FIXED_SHELL_DISPLACEMENT     , "Fixed shell displacement", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSFixedRotation              , MODULE_MECH, FE_ESSENTIAL_BC     , FE_FIXED_ROTATION               , "Fixed shell rotation", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedDisplacement     , MODULE_MECH, FE_ESSENTIAL_BC     , FE_PRESCRIBED_DISPLACEMENT      , "Prescribed displacement", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedShellDisplacement, MODULE_MECH, FE_ESSENTIAL_BC     , FE_PRESCRIBED_SHELL_DISPLACEMENT, "Prescribed shell displacement", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedRotation         , MODULE_MECH, FE_ESSENTIAL_BC     , FE_PRESCRIBED_ROTATION          , "Prescribed shell rotation", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FEPressureLoad               , MODULE_MECH, FE_SURFACE_LOAD     , FE_PRESSURE_LOAD                , "Pressure", PRESSURE_LOAD_HTML);
	REGISTER_FE_CLASS(FESurfaceTraction            , MODULE_MECH, FE_SURFACE_LOAD     , FE_SURFACE_TRACTION             , "Surface traction", TRACTION_LOAD_HTML);
	REGISTER_FE_CLASS(FENodalVelocities            , MODULE_MECH, FE_INITIAL_CONDITION, FE_NODAL_VELOCITIES             , "Velocity");
	REGISTER_FE_CLASS(FENodalShellVelocities       , MODULE_MECH, FE_INITIAL_CONDITION, FE_NODAL_SHELL_VELOCITIES       , "Shell velocity");
	REGISTER_FE_CLASS(FEInitPrestrain              , MODULE_MECH, FE_INITIAL_CONDITION, FE_INIT_PRESTRAIN               , "Initialize Prestrain");
	REGISTER_FE_CLASS(FERigidInterface             , MODULE_MECH, FE_INTERFACE        , FE_RIGID_INTERFACE              , "Rigid", RIGID_NODES_HTML);
	REGISTER_FE_CLASS(FESlidingWithGapsInterface   , MODULE_MECH, FE_INTERFACE        , FE_SLIDING_WITH_GAPS            , "Sliding node-on-facet", SLIDING_INTERFACES_HTML);
	REGISTER_FE_CLASS(FEFacetOnFacetInterface      , MODULE_MECH, FE_INTERFACE        , FE_FACET_ON_FACET_SLIDING       , "Sliding facet-on-facet", SLIDING_INTERFACES_HTML);
    REGISTER_FE_CLASS(FETensionCompressionInterface, MODULE_MECH, FE_INTERFACE        , FE_TENSCOMP_INTERFACE           , "Sliding elastic", SLIDING_INTERFACES_HTML);
	REGISTER_FE_CLASS(FETiedInterface              , MODULE_MECH, FE_INTERFACE        , FE_TIED_INTERFACE               , "Tied node-on-facet", TIED_INTERFACES_HTML);
	REGISTER_FE_CLASS(FEF2FTiedInterface           , MODULE_MECH, FE_INTERFACE        , FE_FACET_ON_FACET_TIED          , "Tied facet-on-facet", TIED_INTERFACES_HTML);
    REGISTER_FE_CLASS(FETiedElasticInterface       , MODULE_MECH, FE_INTERFACE        , FE_TIED_ELASTIC_INTERFACE       , "Tied elastic", TIED_INTERFACES_HTML);
    REGISTER_FE_CLASS(FEContactPotentialInterface  , MODULE_MECH, FE_INTERFACE        , FE_CONTACTPOTENTIAL_CONTACT     , "contact potential");
	REGISTER_FE_CLASS(FEStickyInterface            , MODULE_MECH, FE_INTERFACE        , FE_STICKY_INTERFACE             , "Sticky", STICKY_INTERFACES_HTML);
	REGISTER_FE_CLASS(FEPeriodicBoundary           , MODULE_MECH, FE_INTERFACE        , FE_PERIODIC_BOUNDARY            , "Periodic boundary");
	REGISTER_FE_CLASS(FERigidWallInterface         , MODULE_MECH, FE_INTERFACE        , FE_RIGID_WALL                   , "Rigid wall", RIGID_WALL_INTERFACES_HTML);
	REGISTER_FE_CLASS(FERigidSphereInterface       , MODULE_MECH, FE_INTERFACE        , FE_RIGID_SPHERE_CONTACT         , "Rigid sphere");
	REGISTER_FE_CLASS(FERigidJoint                 , MODULE_MECH, FE_INTERFACE        , FE_RIGID_JOINT                  , "Rigid joint");
	REGISTER_FE_CLASS(FSConstBodyForce             , MODULE_MECH, FE_BODY_LOAD        , FE_CONST_BODY_FORCE             , "Const body force", CONSTANT_BODY_FORCE_HTML);
	REGISTER_FE_CLASS(FENonConstBodyForce          , MODULE_MECH, FE_BODY_LOAD        , FE_NON_CONST_BODY_FORCE         , "Non-const body force", NON_CONSTANT_BODY_FORCE_HTML);
    REGISTER_FE_CLASS(FECentrifugalBodyForce       , MODULE_MECH, FE_BODY_LOAD        , FE_CENTRIFUGAL_BODY_FORCE       , "Centrifugal body force", CENTRIFUGAL_BODY_FORCE_HTML);
    REGISTER_FE_CLASS(FEMassDamping                , MODULE_MECH, FE_BODY_LOAD        , FE_MASSDAMPING_LOAD             , "Mass damping");

	REGISTER_FE_CLASS(FERigidFixed			, MODULE_MECH, FE_RIGID_CONSTRAINT, FE_RIGID_FIXED				, "Fixed rigid displacement/rotation", PRESCRIBE_RIGID_CONSTRAINT_HTML);
	REGISTER_FE_CLASS(FERigidDisplacement	, MODULE_MECH, FE_RIGID_CONSTRAINT, FE_RIGID_DISPLACEMENT		, "Prescribed rigid displacement/rotation", PRESCRIBE_RIGID_CONSTRAINT_HTML);
	REGISTER_FE_CLASS(FERigidForce			, MODULE_MECH, FE_RIGID_CONSTRAINT, FE_RIGID_FORCE				, "Prescribed rigid force", PRESCRIBE_RIGID_CONSTRAINT_HTML);
	REGISTER_FE_CLASS(FERigidVelocity		, MODULE_MECH, FE_RIGID_CONSTRAINT, FE_RIGID_INIT_VELOCITY		, "Initial rigid velocity");
	REGISTER_FE_CLASS(FERigidAngularVelocity, MODULE_MECH, FE_RIGID_CONSTRAINT, FE_RIGID_INIT_ANG_VELOCITY	, "Initial rigid angular velocity");

	REGISTER_FE_CLASS(FSRigidSphericalJoint		, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_SPHERICAL_JOINT	, "Spherical joint", RIGID_SPHERICAL_JOINT_HTML);
	REGISTER_FE_CLASS(FSRigidRevoluteJoint		, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_REVOLUTE_JOINT		, "Revolute joint", RIGID_REVOLUTE_JOINT_HTML);
	REGISTER_FE_CLASS(FSRigidPrismaticJoint		, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_PRISMATIC_JOINT	, "Prismatic joint", RIGID_PRISMATIC_JOINT_HTML);
	REGISTER_FE_CLASS(FSRigidCylindricalJoint	, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_CYLINDRICAL_JOINT	, "Cylindrical joint", RIGID_CYLINDRICAL_JOINT_HTML);
	REGISTER_FE_CLASS(FSRigidPlanarJoint		, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_PLANAR_JOINT		, "Planar joint", RIGID_PLANAR_JOINT_HTML);
    REGISTER_FE_CLASS(FSRigidLock               , MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_RIGID_LOCK         , "Rigid lock");
	REGISTER_FE_CLASS(FSRigidSpring				, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_SPRING				, "Spring", RIGID_SPRING_HTML);
	REGISTER_FE_CLASS(FSRigidDamper				, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_DAMPER				, "Damper", RIGID_DAMPER_HTML);
	REGISTER_FE_CLASS(FSRigidAngularDamper		, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_ANGULAR_DAMPER		, "Angular damper");
	REGISTER_FE_CLASS(FSRigidContractileForce	, MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_CONTRACTILE_FORCE	, "Contractile force");
	REGISTER_FE_CLASS(FSGenericRigidJoint       , MODULE_MECH, FE_RIGID_CONNECTOR, FE_RC_GENERIC_JOINT      , "Generic joint");

	REGISTER_FE_CLASS(FESymmetryPlane    , MODULE_MECH, FE_CONSTRAINT, FE_SYMMETRY_PLANE   , "symmetry plane", SYMMETRY_PLANE_HTML);
	REGISTER_FE_CLASS(FEVolumeConstraint , MODULE_MECH, FE_CONSTRAINT, FE_VOLUME_CONSTRAINT, "volume constraint");
	REGISTER_FE_CLASS(FEWarpingConstraint, MODULE_MECH, FE_CONSTRAINT, FE_WARP_CONSTRAINT  , "warp-image");
	REGISTER_FE_CLASS(FEPrestrainConstraint, MODULE_MECH, FE_CONSTRAINT, FE_PRESTRAIN_CONSTRAINT, "prestrain");

	// --- HEAT MODULE ---
	REGISTER_FE_CLASS(FSHeatTransfer         , MODULE_HEAT, FE_ANALYSIS         , FE_STEP_HEAT_TRANSFER    , "Heat Transfer");
	REGISTER_FE_CLASS(FSFixedTemperature     , MODULE_HEAT, FE_ESSENTIAL_BC     , FE_FIXED_TEMPERATURE     , "Zero temperature", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedTemperature, MODULE_HEAT, FE_ESSENTIAL_BC     , FE_PRESCRIBED_TEMPERATURE, "Prescribed temperature", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FEHeatFlux             , MODULE_HEAT, FE_SURFACE_LOAD     , FE_HEAT_FLUX             , "Heat flux", HEAT_FLUX_HTML);
	REGISTER_FE_CLASS(FEConvectiveHeatFlux   , MODULE_HEAT, FE_SURFACE_LOAD     , FE_CONV_HEAT_FLUX        , "Convective heat flux", CONVECTIVE_HEAT_FLUX_HTML);
	REGISTER_FE_CLASS(FEInitTemperature      , MODULE_HEAT, FE_INITIAL_CONDITION, FE_INIT_TEMPERATURE      , "Temperature");
	REGISTER_FE_CLASS(FEHeatSource           , MODULE_HEAT, FE_BODY_LOAD        , FE_HEAT_SOURCE           , "Heat source", HEAT_SOURCE_HTML);
	REGISTER_FE_CLASS(FEGapHeatFluxInterface , MODULE_HEAT, FE_INTERFACE        , FE_GAPHEATFLUX_INTERFACE , "Gap heat flux");

	// --- BIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSNonLinearBiphasic      , MODULE_BIPHASIC, FE_ANALYSIS         , FE_STEP_BIPHASIC            , "Biphasic");
	REGISTER_FE_CLASS(FSFixedFluidPressure     , MODULE_BIPHASIC, FE_ESSENTIAL_BC     , FE_FIXED_FLUID_PRESSURE     , "Zero fluid pressure", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedFluidPressure, MODULE_BIPHASIC, FE_ESSENTIAL_BC     , FE_PRESCRIBED_FLUID_PRESSURE, "Prescribed fluid pressure", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FEFluidFlux              , MODULE_BIPHASIC, FE_SURFACE_LOAD     , FE_FLUID_FLUX               , "Fluid flux", FLUID_FLUX_HTML);
	REGISTER_FE_CLASS(FEBPNormalTraction       , MODULE_BIPHASIC, FE_SURFACE_LOAD     , FE_BP_NORMAL_TRACTION       , "Mixture normal traction", FLUID_NORMAL_TRACTION_HTML);
	REGISTER_FE_CLASS(FEInitFluidPressure      , MODULE_BIPHASIC, FE_INITIAL_CONDITION, FE_INIT_FLUID_PRESSURE      , "Fluid pressure");
    REGISTER_FE_CLASS(FEInitShellFluidPressure , MODULE_BIPHASIC, FE_INITIAL_CONDITION, FE_INIT_SHELL_FLUID_PRESSURE, "Shell fluid pressure");
    REGISTER_FE_CLASS(FEPoroContact            , MODULE_BIPHASIC, FE_INTERFACE        , FE_PORO_INTERFACE           , "Biphasic contact", BIPHASIC_CONTACT_HTML);
    REGISTER_FE_CLASS(FETiedBiphasicInterface  , MODULE_BIPHASIC, FE_INTERFACE        , FE_TIEDBIPHASIC_INTERFACE   , "Tied biphasic contact", TIED_BIPHASIC_INTERFACES_HTML);

	// --- SOLUTES MODULE ---
	REGISTER_FE_CLASS(FSFixedConcentration      , MODULE_SOLUTES, FE_ESSENTIAL_BC     , FE_FIXED_CONCENTRATION      , "Zero concentration", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedConcentration , MODULE_SOLUTES, FE_ESSENTIAL_BC     , FE_PRESCRIBED_CONCENTRATION , "Prescribed concentration", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FEInitConcentration       , MODULE_SOLUTES, FE_INITIAL_CONDITION, FE_INIT_CONCENTRATION       , "Concentration");
    REGISTER_FE_CLASS(FEInitShellConcentration  , MODULE_SOLUTES, FE_INITIAL_CONDITION, FE_INIT_SHELL_CONCENTRATION , "Shell concentration");

	// --- MULTIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSBiphasicSolutes         , MODULE_MULTIPHASIC, FE_ANALYSIS         , FE_STEP_BIPHASIC_SOLUTE     , "Biphasic-solute");
	REGISTER_FE_CLASS(FSMultiphasicAnalysis     , MODULE_MULTIPHASIC, FE_ANALYSIS         , FE_STEP_MULTIPHASIC         , "Multiphasic");
	REGISTER_FE_CLASS(FEPoroSoluteContact       , MODULE_MULTIPHASIC, FE_INTERFACE        , FE_PORO_SOLUTE_INTERFACE    , "Biphasic-solute contact", BIPHASIC_SOLUTE_AND_MULTIPHASIC_CONTACT_HTML);
	REGISTER_FE_CLASS(FEMultiphasicContact      , MODULE_MULTIPHASIC, FE_INTERFACE        , FE_MULTIPHASIC_INTERFACE    , "Multiphasic contact", BIPHASIC_SOLUTE_AND_MULTIPHASIC_CONTACT_HTML);
	REGISTER_FE_CLASS(FETiedMultiphasicInterface, MODULE_MULTIPHASIC, FE_INTERFACE        , FE_TIEDMULTIPHASIC_INTERFACE, "Tied multiphasic contact", TIED_MULTIPHASIC_INTERFACES_HTML);
	REGISTER_FE_CLASS(FESoluteFlux              , MODULE_MULTIPHASIC, FE_SURFACE_LOAD     , FE_SOLUTE_FLUX              , "Solute flux", SOLUTE_FLUX_HTML);
    REGISTER_FE_CLASS(FEMatchingOsmoticCoefficient, MODULE_MULTIPHASIC, FE_SURFACE_LOAD   , FE_MATCHING_OSM_COEF        , "Matching osmotic coefficient", FLUID_NORMAL_TRACTION_HTML);

#ifdef _DEBUG
	REGISTER_FE_CLASS(FESBMPointSource, MODULE_MULTIPHASIC, FE_BODY_LOAD, FE_SBM_POINT_SOURCE, "SBM point source");
#endif

	// --- FLUID MODULE ---
	REGISTER_FE_CLASS(FSFluidAnalysis           , MODULE_FLUID, FE_ANALYSIS    , FE_STEP_FLUID                  , "Fluid Mechanics");
    REGISTER_FE_CLASS(FSFixedFluidVelocity      , MODULE_FLUID, FE_ESSENTIAL_BC, FE_FIXED_FLUID_VELOCITY        , "Zero fluid velocity", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
    REGISTER_FE_CLASS(FSPrescribedFluidVelocity , MODULE_FLUID, FE_ESSENTIAL_BC, FE_PRESCRIBED_FLUID_VELOCITY   , "Prescribed fluid velocity", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSFixedFluidDilatation    , MODULE_FLUID, FE_ESSENTIAL_BC, FE_FIXED_DILATATION            , "Zero fluid dilatation", FIXED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FSPrescribedFluidDilatation,MODULE_FLUID, FE_ESSENTIAL_BC, FE_PRESCRIBED_DILATATION       , "Prescribed fluid dilatation", PRESCRIBED_NODAL_DEGREES_OF_FREEDOM_HTML);
	REGISTER_FE_CLASS(FEFluidTraction           , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_TRACTION              , "Fluid viscous traction", FLUID_TRACTION_HTML);
    REGISTER_FE_CLASS(FEFluidPressureLoad       , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_PRESSURE_LOAD         , "Fluid pressure", FLUID_PRESSURE_HTML);
    REGISTER_FE_CLASS(FEFluidVelocity           , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_VELOCITY              , "Fluid velocity", FLUID_VELOCITY_HTML);
    REGISTER_FE_CLASS(FEFluidNormalVelocity     , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_NORMAL_VELOCITY       , "Fluid normal velocity", FLUID_NORMAL_VELOCITY_HTML);
    REGISTER_FE_CLASS(FEFluidRotationalVelocity , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_ROTATIONAL_VELOCITY   , "Fluid rotational velocity", FLUID_ROTATIONAL_VELOCITY_HTML);
    REGISTER_FE_CLASS(FEFluidFlowResistance     , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_FLOW_RESISTANCE       , "Fluid flow resistance", FLUID_RESISTANCE_HTML);
    REGISTER_FE_CLASS(FEFluidFlowRCR            , MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_FLOW_RCR              , "Fluid RCR");
    REGISTER_FE_CLASS(FEFluidBackflowStabilization, MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_BACKFLOW_STABIL     , "Fluid back flow stabilization", FLUID_BACKFLOW_STABILIZATION_HTML);
    REGISTER_FE_CLASS(FEFluidTangentialStabilization, MODULE_FLUID, FE_SURFACE_LOAD, FE_FLUID_TANGENTIAL_STABIL , "Fluid tangential stabilization", FLUID_TANGENTIAL_STABILIZATION_HTML);
    REGISTER_FE_CLASS(FEInitFluidDilatation     , MODULE_FLUID, FE_INITIAL_CONDITION, FE_INIT_FLUID_DILATATION  , "Fluid dilatation");

	REGISTER_FE_CLASS(FENormalFlowSurface       , MODULE_FLUID, FE_CONSTRAINT  , FE_NORMAL_FLUID_FLOW           , "Normal flow constraint", NORMAL_FLUID_VELOCITY_CONSTRAINT_HTML);
    REGISTER_FE_CLASS(FEFrictionlessFluidWall   , MODULE_FLUID, FE_CONSTRAINT  , FE_FRICTIONLESS_FLUID_WALL     , "Frictionless fluid wall");

    // --- FLUID-FSI MODULE ---
    REGISTER_FE_CLASS(FSFluidFSIAnalysis        , MODULE_FLUID_FSI, FE_ANALYSIS    , FE_STEP_FLUID_FSI   , "Fluid-FSI Mechanics");
    REGISTER_FE_CLASS(FEFSITraction             , MODULE_FLUID_FSI, FE_SURFACE_LOAD, FE_FSI_TRACTION     , "FSI Interface Traction", FLUID_FSI_TRACTION_HTML);
    REGISTER_FE_CLASS(FEBFSITraction            , MODULE_FLUID_FSI, FE_SURFACE_LOAD, FE_BFSI_TRACTION     , "Biphasic-FSI Interface Traction", BIPHASIC_FSI_TRACTION_HTML);
    

	// --- REACTION-DIFFUSION MODULE ---
	REGISTER_FE_CLASS(FSReactionDiffusionAnalysis, MODULE_REACTION_DIFFUSION, FE_ANALYSIS, FE_STEP_REACTION_DIFFUSION, "Reaction-Diffusion");
	REGISTER_FE_CLASS(FEConcentrationFlux        , MODULE_REACTION_DIFFUSION, FE_SURFACE_LOAD, FE_CONCENTRATION_FLUX, "Concentration Flux");

	// FEBio interface classes
	REGISTER_FE_CLASS(FEBioBoundaryCondition, 0, FE_ESSENTIAL_BC     , FE_FEBIO_BC               , "[febio]");
	REGISTER_FE_CLASS(FEBioNodalLoad        , 0, FE_NODAL_LOAD       , FE_FEBIO_NODAL_LOAD       , "[febio]");
	REGISTER_FE_CLASS(FEBioSurfaceLoad      , 0, FE_SURFACE_LOAD     , FE_FEBIO_SURFACE_LOAD     , "[febio]");
	REGISTER_FE_CLASS(FEBioBodyLoad         , 0, FE_BODY_LOAD        , FE_FEBIO_BODY_LOAD        , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidLoad        , 0, FE_RIGID_LOAD       , FE_FEBIO_RIGID_LOAD       , "[febio]");
	REGISTER_FE_CLASS(FEBioInterface        , 0, FE_INTERFACE        , FE_FEBIO_INTERFACE        , "[febio]");
	REGISTER_FE_CLASS(FEBioInitialCondition , 0, FE_INITIAL_CONDITION, FE_FEBIO_INITIAL_CONDITION, "[febio]");
	REGISTER_FE_CLASS(FEBioNLConstraint     , 0, FE_CONSTRAINT       , FE_FEBIO_NLCONSTRAINT     , "[febio]");
	REGISTER_FE_CLASS(FEBioAnalysisStep     , 0, FE_ANALYSIS         , FE_STEP_FEBIO_ANALYSIS    , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidConstraint  , 0, FE_RIGID_CONSTRAINT , FE_FEBIO_RIGID_CONSTRAINT , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidConnector   , 0, FE_RIGID_CONNECTOR  , FE_FEBIO_RIGID_CONNECTOR  , "[febio]");
}

//-------------------------------------------------------------------------------------------------
void FEProject::SetDefaultPlotVariables()
{
	const char* szmod = FEBio::GetModuleName(m_module); assert(szmod);
	if (szmod == nullptr) return;
	if (strcmp(szmod, "solid") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
	}
	else if (strcmp(szmod, "biphasic") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
		m_plt.AddPlotVariable("relative volume", true);
		m_plt.AddPlotVariable("solid stress", true);
		m_plt.AddPlotVariable("effective fluid pressure", true);
		m_plt.AddPlotVariable("fluid flux", true);
	}
	else if (strcmp(szmod, "heat") == 0)
	{
		m_plt.AddPlotVariable("temperature", true);
		m_plt.AddPlotVariable("heat flux", true);
	}
	else if ((strcmp(szmod, "multiphasic") == 0) || (strcmp(szmod, "solute") == 0))
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("stress", true);
		m_plt.AddPlotVariable("relative volume", true);
		m_plt.AddPlotVariable("solid stress", true);
		m_plt.AddPlotVariable("fluid flux", true);
		m_plt.AddPlotVariable("effective fluid pressure", true);
		m_plt.AddPlotVariable("effective solute concentration", true);
		m_plt.AddPlotVariable("solute concentration", true);
		m_plt.AddPlotVariable("solute flux", true);
	}
	else if (strcmp(szmod, "fluid") == 0)
	{
		m_plt.AddPlotVariable("fluid pressure", true);
		m_plt.AddPlotVariable("nodal fluid velocity", true);
		m_plt.AddPlotVariable("fluid stress", true);
		m_plt.AddPlotVariable("fluid velocity", true);
		m_plt.AddPlotVariable("fluid acceleration", true);
		m_plt.AddPlotVariable("fluid vorticity", true);
		m_plt.AddPlotVariable("fluid rate of deformation", true);
		m_plt.AddPlotVariable("fluid dilatation", true);
		m_plt.AddPlotVariable("fluid volume ratio", true);
	}
	else if (strcmp(szmod, "fluid-FSI") == 0)
	{
		m_plt.AddPlotVariable("displacement", true);
		m_plt.AddPlotVariable("velocity", true);
		m_plt.AddPlotVariable("acceleration", true);
		m_plt.AddPlotVariable("fluid pressure", true);
		m_plt.AddPlotVariable("fluid stress", true);
		m_plt.AddPlotVariable("fluid velocity", true);
		m_plt.AddPlotVariable("fluid acceleration", true);
		m_plt.AddPlotVariable("fluid vorticity", true);
		m_plt.AddPlotVariable("fluid rate of deformation", true);
		m_plt.AddPlotVariable("fluid dilatation", true);
		m_plt.AddPlotVariable("fluid volume ratio", true);
	}
}
