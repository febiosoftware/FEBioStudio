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
#include <FEMLib/FELoadController.h>
#include <FEMLib/FEMeshDataGenerator.h>
#include <FEMLib/FEElementFormulation.h>
#include "GGroup.h"
#include "GModel.h"
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
FSProject::FSProject(void) : m_plt(*this)
{
	// set the title
	m_title = "untitled";

	static bool init = false;
	if (init == false)
	{
		InitModules();
		init = true;
	}
}

//-----------------------------------------------------------------------------
// destructor
FSProject::~FSProject(void)
{
	
}

//-----------------------------------------------------------------------------
void FSProject::Reset()
{
	m_fem.New();
	m_plt.Init();
}

//-----------------------------------------------------------------------------
unsigned int FSProject::GetModule() const
{
	return FEBio::GetActiveModule();
}

//-----------------------------------------------------------------------------
void FSProject::SetModule(unsigned int mod)
{
	FEBio::SetActiveModule(mod);

	// get the list of variables
	FSModel& fem = GetFSModel();
	FEBio::InitFSModel(fem);

	if (m_plt.PlotVariables() == 0)
	{
		// add some default variables
		// TODO: Maybe I can pull this info from FEBio somehow
		SetDefaultPlotVariables();
	}
}

//-----------------------------------------------------------------------------
void FSProject::SetTitle(const std::string& title)
{ 
	m_title = title; 
}

//-----------------------------------------------------------------------------
// save project data to archive
void FSProject::Save(OArchive& ar)
{
	// save the title
	ar.WriteChunk(CID_PRJ_TITLE   , m_title);

	// save the modules flag
	int module = FEBio::GetActiveModule();
	string modName(FEBio::GetModuleName(module));
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
void FSProject::Load(IArchive &ar)
{
	TRACE("FSProject::Load");

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_PRJ_TITLE  : ar.read(m_title); break;
		case CID_PRJ_MODULES: { 
			int oldModuleId = 0;  
			ar.read(oldModuleId); 
			int moduleId = MapOldToNewModules(oldModuleId);
			assert(moduleId != -1);
			SetModule(moduleId);
		} 
		break;
		case CID_PRJ_MODULE_NAME:
		{
			string modName;
			ar.read(modName);
			int moduleId = FEBio::GetModuleId(modName); assert(moduleId > 0);
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
int FSProject::Validate(std::string &szerr)
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
		FSMesh* pm = mdl.Object(i)->GetFEMesh();
		if (pm == 0) nnomesh++;
	}
	if (nnomesh > 0) { szerr += "-Not all objects have a mesh.\n"; nerrs++; }

	return nerrs;
}

void FSProject::InitModules()
{
	// register material categories
	FEMaterialFactory::AddCategory("elastic"             , MODULE_MECH               , FE_MAT_ELASTIC);
	FEMaterialFactory::AddCategory("uncoupled elastic"   , MODULE_MECH               , FE_MAT_ELASTIC_UNCOUPLED);
	FEMaterialFactory::AddCategory("bi-/tri-/multiphasic", MODULE_BIPHASIC           , FE_MAT_MULTIPHASIC);
	FEMaterialFactory::AddCategory("heat transfer"       , MODULE_HEAT               , FE_MAT_HEAT_TRANSFER);
	FEMaterialFactory::AddCategory("fluid"               , MODULE_FLUID              , FE_MAT_FLUID);
    FEMaterialFactory::AddCategory("fluid-FSI"           , MODULE_FLUID_FSI          , FE_MAT_FLUID_FSI);
	FEMaterialFactory::AddCategory("reaction-diffusion"  , MODULE_REACTION_DIFFUSION , FE_MAT_REACTION_DIFFUSION);
	FEMaterialFactory::AddCategory("other"               , MODULE_MECH				 , FE_MAT_RIGID);

	// --- MECH MODULE ---
	REGISTER_FE_CLASS(FSNonLinearMechanics         , MODULE_MECH, FEANALYSIS_ID         , FE_STEP_MECHANICS               , "Structural Mechanics");
	REGISTER_FE_CLASS(FSFixedDisplacement          , MODULE_MECH, FEBC_ID			    , FE_FIXED_DISPLACEMENT           , "Fixed displacement");
	REGISTER_FE_CLASS(FSFixedShellDisplacement     , MODULE_MECH, FEBC_ID			    , FE_FIXED_SHELL_DISPLACEMENT     , "Fixed shell displacement");
	REGISTER_FE_CLASS(FSFixedRotation              , MODULE_MECH, FEBC_ID			    , FE_FIXED_ROTATION               , "Fixed shell rotation");
	REGISTER_FE_CLASS(FSPrescribedDisplacement     , MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_DISPLACEMENT      , "Prescribed displacement");
	REGISTER_FE_CLASS(FSPrescribedShellDisplacement, MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_SHELL_DISPLACEMENT, "Prescribed shell displacement");
	REGISTER_FE_CLASS(FSPrescribedRotation         , MODULE_MECH, FEBC_ID			    , FE_PRESCRIBED_ROTATION          , "Prescribed shell rotation");
	REGISTER_FE_CLASS(FSPressureLoad               , MODULE_MECH, FELOAD_ID             , FE_PRESSURE_LOAD                , "Pressure");
	REGISTER_FE_CLASS(FSSurfaceTraction            , MODULE_MECH, FELOAD_ID             , FE_SURFACE_TRACTION             , "Surface traction");
	REGISTER_FE_CLASS(FSNodalVelocities            , MODULE_MECH, FEIC_ID               , FE_NODAL_VELOCITIES             , "Velocity");
	REGISTER_FE_CLASS(FSNodalShellVelocities       , MODULE_MECH, FEIC_ID               , FE_NODAL_SHELL_VELOCITIES       , "Shell velocity");
	REGISTER_FE_CLASS(FSInitPrestrain              , MODULE_MECH, FEIC_ID               , FE_INIT_PRESTRAIN               , "Initialize Prestrain");
	REGISTER_FE_CLASS(FSRigidInterface             , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_INTERFACE              , "Rigid");
	REGISTER_FE_CLASS(FSSlidingWithGapsInterface   , MODULE_MECH, FESURFACEINTERFACE_ID , FE_SLIDING_WITH_GAPS            , "Sliding node-on-facet");
	REGISTER_FE_CLASS(FSFacetOnFacetInterface      , MODULE_MECH, FESURFACEINTERFACE_ID , FE_FACET_ON_FACET_SLIDING       , "Sliding facet-on-facet");
    REGISTER_FE_CLASS(FSTensionCompressionInterface, MODULE_MECH, FESURFACEINTERFACE_ID , FE_TENSCOMP_INTERFACE           , "Sliding elastic");
	REGISTER_FE_CLASS(FSTiedInterface              , MODULE_MECH, FESURFACEINTERFACE_ID , FE_TIED_INTERFACE               , "Tied node-on-facet");
	REGISTER_FE_CLASS(FSF2FTiedInterface           , MODULE_MECH, FESURFACEINTERFACE_ID , FE_FACET_ON_FACET_TIED          , "Tied facet-on-facet");
    REGISTER_FE_CLASS(FSTiedElasticInterface       , MODULE_MECH, FESURFACEINTERFACE_ID , FE_TIED_ELASTIC_INTERFACE       , "Tied elastic");
    REGISTER_FE_CLASS(FSContactPotentialInterface  , MODULE_MECH, FESURFACEINTERFACE_ID , FE_CONTACTPOTENTIAL_CONTACT     , "contact potential");
	REGISTER_FE_CLASS(FSStickyInterface            , MODULE_MECH, FESURFACEINTERFACE_ID , FE_STICKY_INTERFACE             , "Sticky");
	REGISTER_FE_CLASS(FSPeriodicBoundary           , MODULE_MECH, FESURFACEINTERFACE_ID , FE_PERIODIC_BOUNDARY            , "Periodic boundary");
	REGISTER_FE_CLASS(FSRigidWallInterface         , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_WALL                   , "Rigid wall");
	REGISTER_FE_CLASS(FSRigidSphereInterface       , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_SPHERE_CONTACT         , "Rigid sphere");
	REGISTER_FE_CLASS(FSRigidJoint                 , MODULE_MECH, FESURFACEINTERFACE_ID , FE_RIGID_JOINT                  , "Rigid joint");
	REGISTER_FE_CLASS(FSConstBodyForce             , MODULE_MECH, FELOAD_ID        , FE_CONST_BODY_FORCE             , "Const body force");
	REGISTER_FE_CLASS(FSNonConstBodyForce          , MODULE_MECH, FELOAD_ID        , FE_NON_CONST_BODY_FORCE         , "Non-const body force");
    REGISTER_FE_CLASS(FSCentrifugalBodyForce       , MODULE_MECH, FELOAD_ID        , FE_CENTRIFUGAL_BODY_FORCE       , "Centrifugal body force");
    REGISTER_FE_CLASS(FSMassDamping                , MODULE_MECH, FELOAD_ID        , FE_MASSDAMPING_LOAD             , "Mass damping");

	REGISTER_FE_CLASS(FSRigidFixed			, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_FIXED				, "Fixed rigid displacement/rotation");
	REGISTER_FE_CLASS(FSRigidDisplacement	, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_DISPLACEMENT		, "Prescribed rigid displacement/rotation");
	REGISTER_FE_CLASS(FSRigidForce			, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_FORCE				, "Prescribed rigid force");
	REGISTER_FE_CLASS(FSRigidVelocity		, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_INIT_VELOCITY		, "Initial rigid velocity");
	REGISTER_FE_CLASS(FSRigidAngularVelocity, MODULE_MECH, FERIGIDBC_ID, FE_RIGID_INIT_ANG_VELOCITY	, "Initial rigid angular velocity");

	REGISTER_FE_CLASS(FSRigidSphericalJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_SPHERICAL_JOINT	, "Spherical joint");
	REGISTER_FE_CLASS(FSRigidRevoluteJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_REVOLUTE_JOINT		, "Revolute joint");
	REGISTER_FE_CLASS(FSRigidPrismaticJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_PRISMATIC_JOINT	, "Prismatic joint");
	REGISTER_FE_CLASS(FSRigidCylindricalJoint	, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_CYLINDRICAL_JOINT	, "Cylindrical joint");
	REGISTER_FE_CLASS(FSRigidPlanarJoint		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_PLANAR_JOINT		, "Planar joint");
    REGISTER_FE_CLASS(FSRigidLock               , MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_RIGID_LOCK         , "Rigid lock");
	REGISTER_FE_CLASS(FSRigidSpring				, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_SPRING				, "Spring");
	REGISTER_FE_CLASS(FSRigidDamper				, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_DAMPER				, "Damper");
	REGISTER_FE_CLASS(FSRigidAngularDamper		, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_ANGULAR_DAMPER		, "Angular damper");
	REGISTER_FE_CLASS(FSRigidContractileForce	, MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_CONTRACTILE_FORCE	, "Contractile force");
	REGISTER_FE_CLASS(FSGenericRigidJoint       , MODULE_MECH, FENLCONSTRAINT_ID, FE_RC_GENERIC_JOINT      , "Generic joint");

	REGISTER_FE_CLASS(FSSymmetryPlane      , MODULE_MECH, FENLCONSTRAINT_ID, FE_SYMMETRY_PLANE      , "symmetry plane");
	REGISTER_FE_CLASS(FSVolumeConstraint   , MODULE_MECH, FENLCONSTRAINT_ID, FE_VOLUME_CONSTRAINT   , "volume constraint");
	REGISTER_FE_CLASS(FSWarpingConstraint  , MODULE_MECH, FENLCONSTRAINT_ID, FE_WARP_CONSTRAINT     , "warp-image");
	REGISTER_FE_CLASS(FSPrestrainConstraint, MODULE_MECH, FENLCONSTRAINT_ID, FE_PRESTRAIN_CONSTRAINT, "prestrain");

	// --- HEAT MODULE ---
	REGISTER_FE_CLASS(FSHeatTransfer         , MODULE_HEAT, FEANALYSIS_ID        , FE_STEP_HEAT_TRANSFER    , "Heat Transfer");
	REGISTER_FE_CLASS(FSFixedTemperature     , MODULE_HEAT, FEBC_ID              , FE_FIXED_TEMPERATURE     , "Zero temperature");
	REGISTER_FE_CLASS(FSPrescribedTemperature, MODULE_HEAT, FEBC_ID              , FE_PRESCRIBED_TEMPERATURE, "Prescribed temperature");
	REGISTER_FE_CLASS(FSHeatFlux             , MODULE_HEAT, FELOAD_ID       , FE_HEAT_FLUX             , "Heat flux");
	REGISTER_FE_CLASS(FSConvectiveHeatFlux   , MODULE_HEAT, FELOAD_ID       , FE_CONV_HEAT_FLUX        , "Convective heat flux");
	REGISTER_FE_CLASS(FSInitTemperature      , MODULE_HEAT, FEIC_ID              , FE_INIT_TEMPERATURE      , "Temperature");
	REGISTER_FE_CLASS(FSHeatSource           , MODULE_HEAT, FELOAD_ID       , FE_HEAT_SOURCE           , "Heat source");
	REGISTER_FE_CLASS(FSGapHeatFluxInterface , MODULE_HEAT, FESURFACEINTERFACE_ID, FE_GAPHEATFLUX_INTERFACE , "Gap heat flux");

	// --- BIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSNonLinearBiphasic      , MODULE_BIPHASIC, FEANALYSIS_ID        , FE_STEP_BIPHASIC            , "Biphasic");
	REGISTER_FE_CLASS(FSFixedFluidPressure     , MODULE_BIPHASIC, FEBC_ID              , FE_FIXED_FLUID_PRESSURE     , "Zero fluid pressure");
	REGISTER_FE_CLASS(FSPrescribedFluidPressure, MODULE_BIPHASIC, FEBC_ID              , FE_PRESCRIBED_FLUID_PRESSURE, "Prescribed fluid pressure");
	REGISTER_FE_CLASS(FSFluidFlux              , MODULE_BIPHASIC, FELOAD_ID       , FE_FLUID_FLUX               , "Fluid flux");
	REGISTER_FE_CLASS(FSBPNormalTraction       , MODULE_BIPHASIC, FELOAD_ID       , FE_BP_NORMAL_TRACTION       , "Mixture normal traction");
	REGISTER_FE_CLASS(FSInitFluidPressure      , MODULE_BIPHASIC, FEIC_ID              , FE_INIT_FLUID_PRESSURE      , "Fluid pressure");
    REGISTER_FE_CLASS(FSInitShellFluidPressure , MODULE_BIPHASIC, FEIC_ID              , FE_INIT_SHELL_FLUID_PRESSURE, "Shell fluid pressure");
    REGISTER_FE_CLASS(FSPoroContact            , MODULE_BIPHASIC, FESURFACEINTERFACE_ID, FE_PORO_INTERFACE           , "Biphasic contact");
    REGISTER_FE_CLASS(FSTiedBiphasicInterface  , MODULE_BIPHASIC, FESURFACEINTERFACE_ID, FE_TIEDBIPHASIC_INTERFACE   , "Tied biphasic contact");

	// --- SOLUTES MODULE ---
	REGISTER_FE_CLASS(FSFixedConcentration      , MODULE_SOLUTES, FEBC_ID, FE_FIXED_CONCENTRATION      , "Zero concentration");
	REGISTER_FE_CLASS(FSPrescribedConcentration , MODULE_SOLUTES, FEBC_ID, FE_PRESCRIBED_CONCENTRATION , "Prescribed concentration");
	REGISTER_FE_CLASS(FSInitConcentration       , MODULE_SOLUTES, FEIC_ID, FE_INIT_CONCENTRATION       , "Concentration");
    REGISTER_FE_CLASS(FSInitShellConcentration  , MODULE_SOLUTES, FEIC_ID, FE_INIT_SHELL_CONCENTRATION , "Shell concentration");

	// --- MULTIPHASIC MODULE ---
	REGISTER_FE_CLASS(FSBiphasicSolutes           , MODULE_MULTIPHASIC, FEANALYSIS_ID        , FE_STEP_BIPHASIC_SOLUTE     , "Biphasic-solute");
	REGISTER_FE_CLASS(FSMultiphasicAnalysis       , MODULE_MULTIPHASIC, FEANALYSIS_ID        , FE_STEP_MULTIPHASIC         , "Multiphasic");
	REGISTER_FE_CLASS(FSPoroSoluteContact         , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_PORO_SOLUTE_INTERFACE    , "Biphasic-solute contact");
	REGISTER_FE_CLASS(FSMultiphasicContact        , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_MULTIPHASIC_INTERFACE    , "Multiphasic contact");
	REGISTER_FE_CLASS(FSTiedMultiphasicInterface  , MODULE_MULTIPHASIC, FESURFACEINTERFACE_ID, FE_TIEDMULTIPHASIC_INTERFACE, "Tied multiphasic contact");
	REGISTER_FE_CLASS(FSSoluteFlux                , MODULE_MULTIPHASIC, FELOAD_ID       , FE_SOLUTE_FLUX              , "Solute flux");
    REGISTER_FE_CLASS(FSMatchingOsmoticCoefficient, MODULE_MULTIPHASIC, FELOAD_ID       , FE_MATCHING_OSM_COEF        , "Matching osmotic coefficient");

#ifdef _DEBUG
	REGISTER_FE_CLASS(FSSBMPointSource, MODULE_MULTIPHASIC, FELOAD_ID, FE_SBM_POINT_SOURCE, "SBM point source");
#endif

	// --- FLUID MODULE ---
	REGISTER_FE_CLASS(FSFluidAnalysis               , MODULE_FLUID, FEANALYSIS_ID   , FE_STEP_FLUID                  , "Fluid Mechanics");
    REGISTER_FE_CLASS(FSFixedFluidVelocity          , MODULE_FLUID, FEBC_ID         , FE_FIXED_FLUID_VELOCITY        , "Zero fluid velocity");
    REGISTER_FE_CLASS(FSPrescribedFluidVelocity     , MODULE_FLUID, FEBC_ID         , FE_PRESCRIBED_FLUID_VELOCITY   , "Prescribed fluid velocity");
	REGISTER_FE_CLASS(FSFixedFluidDilatation        , MODULE_FLUID, FEBC_ID         , FE_FIXED_DILATATION            , "Zero fluid dilatation");
	REGISTER_FE_CLASS(FSPrescribedFluidDilatation   , MODULE_FLUID, FEBC_ID         , FE_PRESCRIBED_DILATATION       , "Prescribed fluid dilatation");
	REGISTER_FE_CLASS(FSFluidTraction               , MODULE_FLUID, FELOAD_ID  , FE_FLUID_TRACTION              , "Fluid viscous traction");
    REGISTER_FE_CLASS(FSFluidPressureLoad           , MODULE_FLUID, FELOAD_ID  , FE_FLUID_PRESSURE_LOAD         , "Fluid pressure");
    REGISTER_FE_CLASS(FSFluidVelocity               , MODULE_FLUID, FELOAD_ID  , FE_FLUID_VELOCITY              , "Fluid velocity");
    REGISTER_FE_CLASS(FSFluidNormalVelocity         , MODULE_FLUID, FELOAD_ID  , FE_FLUID_NORMAL_VELOCITY       , "Fluid normal velocity");
    REGISTER_FE_CLASS(FSFluidRotationalVelocity     , MODULE_FLUID, FELOAD_ID  , FE_FLUID_ROTATIONAL_VELOCITY   , "Fluid rotational velocity");
    REGISTER_FE_CLASS(FSFluidFlowResistance         , MODULE_FLUID, FELOAD_ID  , FE_FLUID_FLOW_RESISTANCE       , "Fluid flow resistance");
    REGISTER_FE_CLASS(FSFluidFlowRCR                , MODULE_FLUID, FELOAD_ID  , FE_FLUID_FLOW_RCR              , "Fluid RCR");
    REGISTER_FE_CLASS(FSFluidBackflowStabilization  , MODULE_FLUID, FELOAD_ID  , FE_FLUID_BACKFLOW_STABIL       , "Fluid back flow stabilization");
    REGISTER_FE_CLASS(FSFluidTangentialStabilization, MODULE_FLUID, FELOAD_ID  , FE_FLUID_TANGENTIAL_STABIL     , "Fluid tangential stabilization");
    REGISTER_FE_CLASS(FSInitFluidDilatation         , MODULE_FLUID, FEIC_ID         , FE_INIT_FLUID_DILATATION       , "Fluid dilatation");

	REGISTER_FE_CLASS(FSNormalFlowSurface       , MODULE_FLUID, FENLCONSTRAINT_ID, FE_NORMAL_FLUID_FLOW           , "Normal flow constraint");
    REGISTER_FE_CLASS(FSFrictionlessFluidWall   , MODULE_FLUID, FENLCONSTRAINT_ID, FE_FRICTIONLESS_FLUID_WALL     , "Frictionless fluid wall");

    // --- FLUID-FSI MODULE ---
    REGISTER_FE_CLASS(FSFluidFSIAnalysis        , MODULE_FLUID_FSI, FEANALYSIS_ID   , FE_STEP_FLUID_FSI   , "Fluid-FSI Mechanics");
    REGISTER_FE_CLASS(FSFSITraction             , MODULE_FLUID_FSI, FELOAD_ID  , FE_FSI_TRACTION     , "FSI Interface Traction");
    REGISTER_FE_CLASS(FSBFSITraction            , MODULE_FLUID_FSI, FELOAD_ID  , FE_BFSI_TRACTION     , "Biphasic-FSI Interface Traction");
    

	// --- REACTION-DIFFUSION MODULE ---
	REGISTER_FE_CLASS(FSReactionDiffusionAnalysis, MODULE_REACTION_DIFFUSION, FEANALYSIS_ID   , FE_STEP_REACTION_DIFFUSION, "Reaction-Diffusion");
	REGISTER_FE_CLASS(FSConcentrationFlux        , MODULE_REACTION_DIFFUSION, FELOAD_ID  , FE_CONCENTRATION_FLUX, "Concentration Flux");

	// FEBio interface classes
	REGISTER_FE_CLASS(FEBioBoundaryCondition, 0, FEBC_ID               , FE_FEBIO_BC                , "[febio]");
	REGISTER_FE_CLASS(FEBioNodalLoad        , 0, FELOAD_ID        , FE_FEBIO_NODAL_LOAD        , "[febio]");
	REGISTER_FE_CLASS(FEBioSurfaceLoad      , 0, FELOAD_ID        , FE_FEBIO_SURFACE_LOAD      , "[febio]");
	REGISTER_FE_CLASS(FEBioBodyLoad         , 0, FELOAD_ID        , FE_FEBIO_BODY_LOAD         , "[febio]");
	REGISTER_FE_CLASS(FEBioInterface        , 0, FESURFACEINTERFACE_ID , FE_FEBIO_INTERFACE         , "[febio]");
	REGISTER_FE_CLASS(FEBioInitialCondition , 0, FEIC_ID               , FE_FEBIO_INITIAL_CONDITION , "[febio]");
	REGISTER_FE_CLASS(FEBioNLConstraint     , 0, FENLCONSTRAINT_ID     , FE_FEBIO_NLCONSTRAINT      , "[febio]");
	REGISTER_FE_CLASS(FEBioSurfaceConstraint, 0, FENLCONSTRAINT_ID     , FE_FEBIO_SURFACECONSTRAINT , "[febio]");
	REGISTER_FE_CLASS(FEBioBodyConstraint   , 0, FENLCONSTRAINT_ID     , FE_FEBIO_BODYCONSTRAINT    , "[febio]");
	REGISTER_FE_CLASS(FEBioAnalysisStep     , 0, FEANALYSIS_ID         , FE_STEP_FEBIO_ANALYSIS     , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidConstraint  , 0, FERIGIDBC_ID          , FE_FEBIO_RIGID_CONSTRAINT  , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidConnector   , 0, FENLCONSTRAINT_ID     , FE_FEBIO_RIGID_CONNECTOR   , "[febio]");
	REGISTER_FE_CLASS(FEBioRigidLoad        , 0, FELOAD_ID             , FE_FEBIO_RIGID_LOAD        , "[febio]");
	REGISTER_FE_CLASS(FEBioMeshAdaptor      , 0, FEMESHADAPTOR_ID      , FE_FEBIO_MESH_ADAPTOR      , "[febio]");
	REGISTER_FE_CLASS(FEBioLoadController   , 0, FELOADCONTROLLER_ID   , FE_FEBIO_LOAD_CONTROLLER   , "[febio]");
	REGISTER_FE_CLASS(FEBioFunction1D       , 0, FEFUNCTION1D_ID       , FE_FEBIO_FUNCTION1D        , "[febio]");
	REGISTER_FE_CLASS(FEBioMeshDataGenerator, 0, FEMESHDATAGENERATOR_ID, FE_FEBIO_MESHDATA_GENERATOR, "[febio]");
	REGISTER_FE_CLASS(FEBioMaterialProperty , 0, FEMATERIALPROP_ID     , FE_FEBIO_MATERIAL_PROPERTY , "[febio]");
}

//-------------------------------------------------------------------------------------------------
void FSProject::SetDefaultPlotVariables()
{
	int moduleId = GetModule();
	const char* szmod = FEBio::GetModuleName(moduleId); assert(szmod);
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
