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

// FEBioExport2.cpp: implementation of the FEBioExport2 class.
//
//////////////////////////////////////////////////////////////////////

#include "FEBioExport2.h"
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GGroup.h>
#include <MeshTools/FEElementData.h>
#include <MeshTools/FEProject.h>
#include <MeshTools/GModel.h>
#include <GeomLib/GObject.h>
#include <memory>
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEBioExport2::FEBioExport2(FEProject& prj) : FEBioExport(prj)
{
}

FEBioExport2::~FEBioExport2()
{
	Clear();
}

void FEBioExport2::Clear()
{
	FEBioExport::Clear();
	m_pSurf.clear();
	m_pNSet.clear();
}

//----------------------------------------------------------------------------
bool FEBioExport2::HasSurface(FEItemListBuilder* pl)
{
	int N = (int) m_pSurf.size();
	for (int i=0; i<N; ++i)
		if (m_pSurf[i] == pl) return true;
	return false;
}

//----------------------------------------------------------------------------
bool FEBioExport2::HasNodeSet(FEItemListBuilder* pl)
{
	int N = (int) m_pNSet.size();
	for (int i=0; i<N; ++i)
		if (m_pNSet[i] == pl) return true;
	return false;
}

//-----------------------------------------------------------------------------

bool FEBioExport2::PrepareExport(FEProject& prj)
{
	if (FEBioExport::PrepareExport(prj) == false) return false;

	FEModel& fem = prj.GetFEModel();
	GModel& model = fem.GetModel();

	m_nodes  = model.FENodes();
	m_nsteps = fem.Steps();

  
	// see if there are any rigid body constraints
	m_nrc = 0;
	for (int i=0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		m_nrc += ps->RigidConstraints();

		for (int j=0; j<ps->Interfaces(); ++j)
		{
			FEInterface* pi = ps->Interface(j);
			if (pi->IsActive())
			{
				if (pi->Type() == FE_VOLUME_CONSTRAINT) m_nrc++;
				if (pi->Type() == FE_SYMMETRY_PLANE   ) m_nrc++;
				if (pi->Type() == FE_NORMAL_FLUID_FLOW) m_nrc++;
			}
		}
	}

	// get the named nodesets (bc's)
	for (int i=0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->BCs(); ++j)
		{
			FEBoundaryCondition* pl = pstep->BC(j);
			if (pl->IsActive())
			{
				FEItemListBuilder* ps = pl->GetItemList();
				if (ps)
				{
					const string& name = ps->GetName();
					if (name.empty() == false) m_pNSet.push_back(ps);
				}
			}
		}
	}

	// get the named surfaces (loads)
	for (int i=0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->Loads(); ++j)
		{
			FELoad* pl = pstep->Load(j);
			if (pl->IsActive())
			{
				// we need to exclude nodal loads
				if (dynamic_cast<FENodalLoad*>(pl)) pl = 0;
				if (pl)
				{
					FEItemListBuilder* ps = pl->GetItemList();
					if (ps)
					{
						const string& name = ps->GetName();
						if (name.empty() == false) m_pSurf.push_back(ps);
					}
				}
			}
		}
	}

	// get the named surfaces (paired interfaces)
	for (int i=0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j=0; j<pstep->Interfaces(); ++j)
		{
			FEInterface* pj = pstep->Interface(j);
			if (pj->IsActive())
			{
				FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(pj);
				if (pi)
				{
					FEItemListBuilder* pms = pi->GetSecondarySurface();
					if (pms)
					{
						const string& name = pms->GetName();
						if (name.empty() == false) m_pSurf.push_back(pms);
					}

					FEItemListBuilder* pss = pi->GetPrimarySurface();
					if (pss)
					{
						const string& name = pss->GetName();
						if (name.empty() == false) m_pSurf.push_back(pss);
					}
				}
			}
		}

		// check the constraints
		for (int j = 0; j < pstep->Constraints(); ++j)
		{
			FEModelConstraint* pj = pstep->Constraint(j);
			if (pj->IsActive())
			{
				FEVolumeConstraint* pvc = dynamic_cast<FEVolumeConstraint*>(pj);
				if (pvc)
				{
					FEItemListBuilder* pi = pvc->GetItemList();
					if (pi)
					{
						const string& name = pi->GetName();
						if (name.empty()) m_pSurf.push_back(pi);
					}
				}

				FENormalFlowSurface* pcs = dynamic_cast<FENormalFlowSurface*>(pj);
				if (pcs)
				{
					FEItemListBuilder* pi = pcs->GetItemList();
					if (pi)
					{
						const string& name = pi->GetName();
						if (name.empty()) m_pSurf.push_back(pi);
					}
				}

				FESymmetryPlane* psp = dynamic_cast<FESymmetryPlane*>(pj);
				if (psp)
				{
					FEItemListBuilder* pi = psp->GetItemList();
					if (pi)
					{
						const string& name = pi->GetName();
						if (name.empty()) m_pSurf.push_back(pi);
					}
				}
                
                FEFrictionlessFluidWall* pfw = dynamic_cast<FEFrictionlessFluidWall*>(pj);
                if (pfw)
                {
                    FEItemListBuilder* pi = pfw->GetItemList();
                    if (pi)
                    {
                        const string& name = pi->GetName();
                        if (name.empty()) m_pSurf.push_back(pi);
                    }
                }
			}
		}
	}

	// check all the (surface) plot variables
	CPlotDataSettings& plt = prj.GetPlotDataSettings();
	for (int i = 0; i<plt.PlotVariables(); ++i)
	{
		FEPlotVariable& var = plt.PlotVariable(i);
		if (var.domainType() == DOMAIN_SURFACE)
		{
			int ND = var.Domains();
			for (int j = 0; j<ND; ++j)
			{
				FEItemListBuilder* pl = var.GetDomain(j);
				m_pSurf.push_back(pl);
			}
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool FEBioExport2::Write(const char* szfile)
{
	// get the project and model
	FEModel& fem = m_prj.GetFEModel();
	m_pfem = &fem;

	// prepare for export
	if (PrepareExport(m_prj) == false) return errf("Not all objects are meshed.");

	// get the initial step
	FEStep* pstep = fem.GetStep(0);

	// the format for single step versus multi-step
	// is slightly different, so we need to see if the 
	// model is single step or not.
	// The model is single step if it has only one 
	// analysis-step and if that step does not define
	// any BCs, Loads, interfaces or RCs.
	int ntype = -1;
	bool bsingle_step = (m_nsteps <= 1);
	if (m_nsteps == 2)
	{
		FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(fem.GetStep(1));
		ntype = pstep->GetType();
		if (pstep == 0) return errf("Step 1 is not an analysis step.");
		if (pstep->BCs() + pstep->Loads() + pstep->Interfaces() + pstep->RigidConstraints() == 0) bsingle_step = true;
	}

	// see if any of the steps are poro
	bool bporo = false;
	for (int i=0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		int ntype = pstep->GetType();
		bporo |= (ntype == FE_STEP_BIPHASIC) || (ntype == FE_STEP_BIPHASIC_SOLUTE) || (ntype == FE_STEP_MULTIPHASIC);
	}

	// open the file
	if (!m_xml.open(szfile)) return errf("Failed opening file %s", szfile);

	XMLElement el;

	try
	{
		// output root element
		el.name("febio_spec");
		el.add_attribute("version", "2.0");

		m_xml.add_branch(el);
		{
			// output control section
			// this section is only written for single-step analysis
			// for multi-step analysis, the control section is 
			// written separately for each step
			if (bsingle_step && (m_nsteps == 2))
			{
				FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(fem.GetStep(1));
				if (pstep == 0) return errf("Step 1 is not an analysis step.");

				// write the module section
				if (m_section[FEBIO_MODULE]) WriteModuleSection(pstep);

				// write the control section
				if (m_section[FEBIO_CONTROL])
				{
					m_xml.add_branch("Control");
					{
						WriteControlSection(pstep);
					}
					m_xml.close_branch(); // Control
				}
			}

			// global variables
			int nvar = fem.Parameters();
			if ((nvar > 0) && m_section[FEBIO_GLOBAL])
			{
				m_xml.add_branch("Globals");
				{
					WriteGlobalsSection();
				}
				m_xml.close_branch();
			}

			// output material section
			if ((fem.Materials() > 0) && (m_section[FEBIO_MATERIAL]))
			{
				m_xml.add_branch("Material");
				{
					WriteMaterialSection();
				}
				m_xml.close_branch(); // Material
			}

			// output geometry section
			if ((fem.GetModel().Objects() > 0) && (m_section[FEBIO_GEOMETRY]))
			{
				m_xml.add_branch("Geometry");
				{
					WriteGeometrySection();
				}
				m_xml.close_branch(); // Geometry
			}

			// output boundary section
			int nbc = pstep->BCs() + pstep->Interfaces() + fem.GetModel().DiscreteObjects();
			if ((nbc > 0) && (m_section[FEBIO_BOUNDARY]))
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			// output loads section
			int nlc = pstep->Loads();
			if ((nlc > 0) && (m_section[FEBIO_LOADS]))
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(*pstep);
				}
				m_xml.close_branch(); // Boundary
			}

			// output contact
			int nci = pstep->Interfaces();
			int nLC = pstep->LinearConstraints();
			if (((nci > 0)||(nLC > 0)) && (m_section[FEBIO_CONTACT]))
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(*pstep);
				}
				m_xml.close_branch(); // Contact
			}

			// output constraints section
            int ncn = pstep->RigidConnectors();
			if (((m_nrc > 0) || (ncn > 0)) && (m_section[FEBIO_CONSTRAINTS]))
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(*pstep);
				}
				m_xml.close_branch();
			}

			// output initial section
			if ((pstep->ICs() > 0) && (m_section[FEBIO_INITIAL]))
			{
				m_xml.add_branch("Initial");
				{
					WriteInitialSection();
				}
				m_xml.close_branch(); // Initial
			}

			// output discrete elements
			int nrb = fem.GetModel().DiscreteObjects();
			if ((nrb > 0) && (m_section[FEBIO_DISCRETE]))
			{
				m_xml.add_branch("Discrete");
				{
					WriteDiscreteSection(*pstep);
				}
				m_xml.close_branch(); // Discrete
			}

			// loadcurve data
			if ((m_pLC.size() > 0) && (m_section[FEBIO_LOADDATA]))
			{
				m_xml.add_branch("LoadData");
				{
					WriteLoadDataSection();
				}
				m_xml.close_branch(); // LoadData
			}

			// Output data
			if (m_section[FEBIO_OUTPUT])
			{
				m_xml.add_branch("Output");
				{
					WriteOutputSection();
				}
				m_xml.close_branch(); // Output
			}

			// step data
			// this is only written if there is more than one step
			// that defines BCs, loads or interfaces
			if (bsingle_step == false) WriteStepSection();
		}
		m_xml.close_branch(); // febio_spec
	}
	catch (InvalidMaterialReference)
	{
		return errf("Invalid material reference.");
	}
	catch (InvalidItemListBuilder e)
	{
		return errf("Invalid reference to mesh item list when exporting:\n%s", (e.m_po ? e.m_po->GetName().c_str(): "(unknown"));
	}
	catch (MissingRigidBody e)
	{
		return errf("No rigid body defined for rigid constraint %s", (e.m_rbName.empty() ? "(unknown)" : e.m_rbName.c_str()));
	}
	catch (RigidContactException)
	{
		return errf("Missing rigid body in rigid contact definition.");
	}
	catch (...)
	{
		return errf("An unknown exception has occured.");
	}

	// close the file
	m_xml.close();

	return true;
}

//-----------------------------------------------------------------------------
// Write the MODULE section
void FEBioExport2::WriteModuleSection(FEAnalysisStep* pstep)
{
	XMLElement t;
	t.name("Module");
	switch (pstep->GetType())
	{
	case FE_STEP_MECHANICS          : t.add_attribute("type", "solid"      ); break;
	case FE_STEP_HEAT_TRANSFER      : t.add_attribute("type", "heat"       ); break;
	case FE_STEP_BIPHASIC           : t.add_attribute("type", "biphasic"   ); break;
	case FE_STEP_BIPHASIC_SOLUTE    : t.add_attribute("type", "solute"     ); break;
	case FE_STEP_MULTIPHASIC        : t.add_attribute("type", "multiphasic"); break;
    case FE_STEP_FLUID              : t.add_attribute("type", "fluid"      ); break;
    case FE_STEP_FLUID_FSI          : t.add_attribute("type", "fluid-FSI"  ); break;
	case FE_STEP_REACTION_DIFFUSION : t.add_attribute("type", "reaction-diffusion"); break;
	};

	m_xml.add_empty(t);
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteControlSection(FEAnalysisStep* pstep)
{
	STEP_SETTINGS& ops = pstep->GetSettings();
	int ntype = pstep->GetType();
	switch (ntype)
	{
	case FE_STEP_MECHANICS          : WriteSolidControlParams         (pstep); break;
	case FE_STEP_HEAT_TRANSFER      : WriteHeatTransferControlParams  (pstep); break;
	case FE_STEP_BIPHASIC           : WriteBiphasicControlParams      (pstep); break;
	case FE_STEP_BIPHASIC_SOLUTE    : WriteBiphasicSoluteControlParams(pstep); break;
	case FE_STEP_MULTIPHASIC        : WriteBiphasicSoluteControlParams(pstep); break;
    case FE_STEP_FLUID              : WriteFluidControlParams         (pstep); break;
    case FE_STEP_FLUID_FSI          : WriteFluidFSIControlParams      (pstep); break;
	case FE_STEP_REACTION_DIFFUSION : WriteReactionDiffusionControlParams(pstep); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteSolidControlParams(FEAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

	if (ops.bauto)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust) 
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	el.name("analysis");
	el.add_attribute("type", (ops.nanalysis==0?"static":"dynamic"));
	m_xml.add_empty(el);

    if (ops.nanalysis != 0) {
        el.name("alpha");
        el.value(ops.alpha);
        m_xml.add_leaf(el);
        
        el.name("beta");
        el.value(ops.beta);
        m_xml.add_leaf(el);
        
        el.name("gamma");
        el.value(ops.gamma);
        m_xml.add_leaf(el);
    }
    
	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteHeatTransferControlParams(FEAnalysisStep* pstep)
{
	XMLElement el;

	STEP_SETTINGS& ops = pstep->GetSettings();

	if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);

	if (ops.bauto)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust) 
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	el.name("analysis");
	el.add_attribute("type", (ops.nanalysis==0?"static":"dynamic"));
	m_xml.add_empty(el);
}


//-----------------------------------------------------------------------------
void FEBioExport2::WriteBiphasicControlParams(FEAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

	if (ops.bauto)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust) 
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	if (ops.nanalysis == 0) 
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_biphasic", (ops.nmatfmt == 1? 1: 0));
	}
}


//-----------------------------------------------------------------------------
void FEBioExport2::WriteBiphasicSoluteControlParams(FEAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

	if (ops.bauto)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust) 
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	if (ops.nanalysis == 0) 
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_biphasic", (ops.nmatfmt == 1? 1: 0));
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteFluidControlParams(FEAnalysisStep* pstep)
{
    XMLElement el;
    STEP_SETTINGS& ops = pstep->GetSettings();
    
    if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
    m_xml.add_leaf("time_steps", ops.ntime);
    m_xml.add_leaf("step_size", ops.dt);
    m_xml.add_leaf("max_refs", ops.maxref);
    m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
	m_xml.add_leaf("diverge_reform", ops.bdivref);
	m_xml.add_leaf("reform_each_time_step", ops.brefstep);

	// write the parameters
	WriteParamList(*pstep);

    if (ops.bauto)
    {
        FELoadCurve* plc = pstep->GetMustPointLoadCurve();
        el.name("time_stepper");
        m_xml.add_branch(el);
        {
            m_xml.add_leaf("dtmin", ops.dtmin);
            
            el.name("dtmax");
            if (ops.bmust)
                el.add_attribute("lc", plc->GetID());
            else el.value(ops.dtmax);
            m_xml.add_leaf(el);
            
            m_xml.add_leaf("max_retries", ops.mxback);
            m_xml.add_leaf("opt_iter", ops.iteopt);
            
            if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
        }
        m_xml.close_branch();
    }
    
    if (ops.nanalysis == 0)
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "steady-state");
        m_xml.add_empty(el);
    }
    else
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "dynamic");
        m_xml.add_empty(el);
    }
    
    if (ops.bminbw)
    {
        m_xml.add_leaf("optimize_bw", 1);
    }
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteFluidFSIControlParams(FEAnalysisStep* pstep)
{
    XMLElement el;
    STEP_SETTINGS& ops = pstep->GetSettings();
    
    if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
    m_xml.add_leaf("time_steps", ops.ntime);
    m_xml.add_leaf("step_size", ops.dt);
    m_xml.add_leaf("max_refs", ops.maxref);
    m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));
    m_xml.add_leaf("diverge_reform", ops.bdivref);
    m_xml.add_leaf("reform_each_time_step", ops.brefstep);
    
    // write the parameters
    WriteParamList(*pstep);
    
    if (ops.bauto)
    {
        FELoadCurve* plc = pstep->GetMustPointLoadCurve();
        el.name("time_stepper");
        m_xml.add_branch(el);
        {
            m_xml.add_leaf("dtmin", ops.dtmin);
            
            el.name("dtmax");
            if (ops.bmust)
                el.add_attribute("lc", plc->GetID());
            else el.value(ops.dtmax);
            m_xml.add_leaf(el);
            
            m_xml.add_leaf("max_retries", ops.mxback);
            m_xml.add_leaf("opt_iter", ops.iteopt);
            
            if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
        }
        m_xml.close_branch();
    }
    
    if (ops.nanalysis == 0)
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "steady-state");
        m_xml.add_empty(el);
    }
    else
    {
        XMLElement el;
        el.name("analysis");
        el.add_attribute("type", "dynamic");
        m_xml.add_empty(el);
    }
    
    if (ops.bminbw)
    {
        m_xml.add_leaf("optimize_bw", 1);
    }
    
    if (ops.nmatfmt != 0)
    {
        m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1? 1: 0));
    }
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteReactionDiffusionControlParams(FEAnalysisStep* pstep)
{
	XMLElement el;
	STEP_SETTINGS& ops = pstep->GetSettings();

	if (ops.sztitle[0]) m_xml.add_leaf("title", ops.sztitle);
	m_xml.add_leaf("time_steps", ops.ntime);
	m_xml.add_leaf("step_size", ops.dt);
	m_xml.add_leaf("max_refs", ops.maxref);
	m_xml.add_leaf("max_ups", (ops.mthsol == 0 ? ops.ilimit : 0));

	// write the parameters
	WriteParamList(*pstep);

	if (ops.bauto)
	{
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		el.name("time_stepper");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("dtmin", ops.dtmin);

			el.name("dtmax");
			if (ops.bmust && plc)
				el.add_attribute("lc", plc->GetID());
			else el.value(ops.dtmax);
			m_xml.add_leaf(el);

			m_xml.add_leaf("max_retries", ops.mxback);
			m_xml.add_leaf("opt_iter", ops.iteopt);

			if (ops.ncut > 0) m_xml.add_leaf("aggressiveness", ops.ncut);
		}
		m_xml.close_branch();
	}

	if (ops.nanalysis == 0)
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "steady-state");
		m_xml.add_empty(el);
	}
	else
	{
		XMLElement el;
		el.name("analysis");
		el.add_attribute("type", "transient");
		m_xml.add_empty(el);
	}

	if (ops.nmatfmt != 0)
	{
		m_xml.add_leaf("symmetric_stiffness", (ops.nmatfmt == 1 ? 1 : 0));
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteMaterialSection()
{
	XMLElement el;

	FEModel& s = *m_pfem;


	for (int i=0; i<s.Materials(); ++i)
	{
		GMaterial* pgm = s.GetMaterial(i);

		el.name("material");
		el.add_attribute("id", pgm->m_ntag);
		el.add_attribute("name", pgm->GetName().c_str());

		FEMaterial* pmat = pgm->GetMaterialProperties();
		if (pmat)
		{
			switch (pmat->Type())
			{
			case FE_VISCO_ELASTIC              : WriteMultiMaterial(pmat, el); break;
			case FE_UNCOUPLED_VISCO_ELASTIC    : WriteMultiMaterial(pmat, el); break;
			case FE_RV_MATERIAL                : WriteMultiMaterial(pmat, el); break;
			case FE_RV_MATERIAL_UC             : WriteMultiMaterial(pmat, el); break;
			case FE_BIPHASIC_MATERIAL          : WriteMultiMaterial(pmat, el); break;
			case FE_BIPHASIC_SOLUTE            : WriteMultiMaterial(pmat, el); break;
			case FE_TRIPHASIC_MATERIAL         : WriteMultiMaterial(pmat, el); break;
			case FE_MULTIPHASIC_MATERIAL       : WriteMultiMaterial(pmat, el); break;
			case FE_SOLID_MIXTURE              : WriteMultiMaterial(pmat, el); break;
			case FE_UNCOUPLED_SOLID_MIXTURE    : WriteMultiMaterial(pmat, el); break;
			case FE_CFD_MATERIAL               : WriteMultiMaterial(pmat, el); break;
			case FE_CFD_MATERIAL_UC            : WriteMultiMaterial(pmat, el); break;
			case FE_DMG_MATERIAL               : WriteMultiMaterial(pmat, el); break;
			case FE_DMG_MATERIAL_UC            : WriteMultiMaterial(pmat, el); break;
			case FE_FLUID_MATERIAL             : WriteMultiMaterial(pmat, el); break;
			case FE_FLUID_FSI_MATERIAL         : WriteMultiMaterial(pmat, el); break;
			case FE_REACTION_DIFFUSION_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_MUSCLE_MATERIAL            : WriteMultiMaterial(pmat, el); break;
			case FE_TENDON_MATERIAL            : WriteMultiMaterial(pmat, el); break;
			case FE_RIGID_MATERIAL             : WriteRigidMaterial(pmat, el); break;
			case FE_TCNL_ORTHO                 : WriteTCNLOrthoMaterial(pmat, el); break;
	//		case FE_PORO_ELASTIC       : WriteNestedMaterial(dynamic_cast<FENestedMaterial*>(pmat), el); break;
	//		case FE_PORO_HOLMES_MOW    : WriteNestedMaterial(dynamic_cast<FENestedMaterial*>(pmat), el); break;
			default:
				WriteMaterial(pmat, el);
			}
		}
		else
		{
			m_xml.add_empty(el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteRigidMaterial(FEMaterial* pmat, XMLElement& el)
{
	FEModel& s = *m_pfem;

	FERigidMaterial* pm = dynamic_cast<FERigidMaterial*> (pmat);
	el.add_attribute("type", "rigid body");
	m_xml.add_branch(el);
	{
		m_xml.add_leaf("density", pm->GetFloatValue(FERigidMaterial::MP_DENSITY));

		if (pm->GetBoolValue(FERigidMaterial::MP_COM) == false)
		{
			vec3d v = pm->GetParam(FERigidMaterial::MP_RC).GetVec3dValue();
			m_xml.add_leaf("center_of_mass", v);
		}

		if (pm->m_pid != -1)
		{
			GMaterial* ppm = s.GetMaterialFromID(pm->m_pid);
			assert(ppm);
			m_xml.add_leaf("parent_id", ppm->m_ntag);
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteTCNLOrthoMaterial(FEMaterial* pmat, XMLElement& el)
{
	FETCNonlinearOrthotropic* pm = dynamic_cast<FETCNonlinearOrthotropic*>(pmat);
	el.add_attribute("type", "TC nonlinear orthotropic");
	double C1, C2, K, beta[3], ksi[3], a[3], d[3];
	vec3d v;
	C1 = pm->GetParam(FETCNonlinearOrthotropic::MP_C1).GetFloatValue();
	C2 = pm->GetParam(FETCNonlinearOrthotropic::MP_C2).GetFloatValue();
	K = pm->GetParam(FETCNonlinearOrthotropic::MP_K).GetFloatValue();

	v = pm->GetParam(FETCNonlinearOrthotropic::MP_BETA).GetVec3dValue();
	beta[0] = v.x;
	beta[1] = v.y;
	beta[2] = v.z;

	v = pm->GetParam(FETCNonlinearOrthotropic::MP_KSI).GetVec3dValue();
	ksi[0] = v.x;
	ksi[1] = v.y;
	ksi[2] = v.z;

	v = pm->GetParam(FETCNonlinearOrthotropic::MP_A).GetVec3dValue();
	a[0] = v.x;
	a[1] = v.y;
	a[2] = v.z;

	v = pm->GetParam(FETCNonlinearOrthotropic::MP_D).GetVec3dValue();
	d[0] = v.x;
	d[1] = v.y;
	d[2] = v.z;

	m_xml.add_branch(el);
	{
		m_xml.add_leaf("c1", C1);
		m_xml.add_leaf("c2", C2);
		m_xml.add_leaf("k", K);
		m_xml.add_leaf("beta", beta, 3);
		m_xml.add_leaf("ksi", ksi, 3);

		el.name("mat_axis");
		el.add_attribute("type", "vector");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("a", a, 3);
			m_xml.add_leaf("d", d, 3);
		}
		m_xml.close_branch();

		//					el.name("fiber_b");
		//					el.add_attribute("type", "vector");
		//					el.value(d);
		//					m_xml.add_leaf(el);
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteMaterial(FEMaterial *pm, XMLElement& el)
{
	const char* sztype = FEMaterialFactory::TypeStr(pm);
	el.add_attribute("type", sztype);
	m_xml.add_branch(el);
	{
		if (pm->m_axes && (pm->m_axes->m_naopt > -1)) {
			el.name("mat_axis");
			if (pm->m_axes->m_naopt == FE_AXES_LOCAL)
			{
				el.add_attribute("type", "local");
				el.value(pm->m_axes->m_n,3);
				m_xml.add_leaf(el);
			}
			else if (pm->m_axes->m_naopt == FE_AXES_VECTOR)
			{
				el.add_attribute("type", "vector");
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("a", pm->m_axes->m_a);
					m_xml.add_leaf("d", pm->m_axes->m_d);
				}
				m_xml.close_branch();
			}
		}
		WriteMaterialParams(pm);
	}
	m_xml.close_branch();
}

void FEBioExport2::WriteFiberMaterial(FEOldFiberMaterial& fiber)
{
	FEOldFiberMaterial& f = fiber;
	XMLElement el;
	el.name("fiber");
	if (f.m_naopt == FE_FIBER_LOCAL) 
	{
		el.add_attribute("type", "local");
		el.value(f.m_n,2);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
	{
		el.add_attribute("type", "cylindrical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis"  , f.m_a);
			m_xml.add_leaf("vector", f.m_d);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_POLAR)
	{
		el.add_attribute("type", "polar");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis"  , f.m_a);
			m_xml.add_leaf("radius1", f.m_R0);
			m_xml.add_leaf("vector1", f.m_d0);
			m_xml.add_leaf("radius2", f.m_R1);
			m_xml.add_leaf("vector2", f.m_d1);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_SPHERICAL)
	{
		el.add_attribute("type", "spherical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("vector", f.m_d);
		}
		m_xml.close_branch();
	}
	else if (f.m_naopt == FE_FIBER_VECTOR)
	{
		el.add_attribute("type", "vector");
		el.value(f.m_a);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_USER)
	{
		el.add_attribute("type", "user");
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_ANGLES)
	{
		el.add_attribute("type", "angles");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("theta", f.m_theta);
			m_xml.add_leaf("phi"  , f.m_phi  );
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteMaterialParams(FEMaterial* pm)
{
	// Write the material parameters first
	WriteParamList(*pm);

	// if the material is transversely-isotropic, we need to write the fiber data as well
	FETransMooneyRivlin* ptmr = dynamic_cast<FETransMooneyRivlin*>(pm);
	if (ptmr)
	{
		FEOldFiberMaterial& f = *(ptmr->GetFiberMaterial());
		WriteFiberMaterial(f);
	}

	FETransVerondaWestmann* ptvw = dynamic_cast<FETransVerondaWestmann*>(pm);
	if (ptvw)
	{
		FEOldFiberMaterial& f = *(ptvw->GetFiberMaterial());
		WriteFiberMaterial(f);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteMultiMaterial(FEMaterial* pm, XMLElement& el)
{
	const char* sztype = 0;
        
	switch (pm->Type())
	{
        case FE_VISCO_ELASTIC          : sztype = "viscoelastic"           ; break;
        case FE_UNCOUPLED_VISCO_ELASTIC: sztype = "uncoupled viscoelastic" ; break;
        case FE_RV_MATERIAL            : sztype = "reactive viscoelastic"  ; break;
        case FE_RV_MATERIAL_UC         : sztype = "uncoupled reactive viscoelastic"; break;
        case FE_BIPHASIC_MATERIAL      : sztype = "biphasic"               ; break;
        case FE_BIPHASIC_SOLUTE        : sztype = "biphasic-solute"        ; break;
        case FE_TRIPHASIC_MATERIAL     : sztype = "triphasic"              ; break;
        case FE_MULTIPHASIC_MATERIAL   : sztype = "multiphasic"            ; break;
        case FE_SOLID_MIXTURE          : sztype = "solid mixture"          ; break;
        case FE_UNCOUPLED_SOLID_MIXTURE: sztype = "uncoupled solid mixture"; break;
        case FE_CFD_MATERIAL           : sztype = "continuous fiber distribution"; break;
        case FE_CFD_MATERIAL_UC        : sztype = "continuous fiber distribution uncoupled"; break;
        case FE_DMG_MATERIAL           : sztype = "elastic damage"; break;
        case FE_DMG_MATERIAL_UC        : sztype = "uncoupled elastic damage"; break;
        case FE_SOLUTE_MATERIAL        : sztype = "solute"                 ; break;
        case FE_SBM_MATERIAL           : sztype = "solid_bound"            ; break;
        case FE_REACTANT_MATERIAL      : sztype = "vR"                     ; break;
        case FE_PRODUCT_MATERIAL       : sztype = "vP"                     ; break;
        case FE_MASS_ACTION_FORWARD    : sztype = "mass-action-forward"    ; break;
        case FE_MASS_ACTION_REVERSIBLE : sztype = "mass-action-reversible" ; break;
        case FE_MICHAELIS_MENTEN       : sztype = "Michaelis-Menten"       ; break;
        case FE_FLUID_MATERIAL         : sztype = "fluid"                  ; break;
        case FE_FLUID_FSI_MATERIAL     : sztype = "fluid-FSI"              ; break;
		case FE_REACTION_DIFFUSION_MATERIAL : sztype = "reaction-diffusion"; break;
        default:
            assert(false);
	}

	// set the type attribute
    if (pm->Type() == FE_SOLUTE_MATERIAL)
    {
        FESoluteMaterial* psm = dynamic_cast<FESoluteMaterial*>(pm); assert(psm);
        el.add_attribute("sol", psm->GetSoluteIndex()+1);
    }
    else if (pm->Type() == FE_SBM_MATERIAL)
    {
        FESBMMaterial* psb = dynamic_cast<FESBMMaterial*>(pm); assert(psb);
        el.add_attribute("sbm", psb->GetSBMIndex()+1);
    }
    else if (pm->Type() == FE_REACTANT_MATERIAL)
    {
        FEReactantMaterial* psb = dynamic_cast<FEReactantMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetReactantType();
        el.value(psb->GetCoef());
        switch (type)
        {
		case FEReactionMaterial::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
		case FEReactionMaterial::SBM_SPECIES   : el.add_attribute("sbm", idx + 1); break;
		default:
			assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else if (pm->Type() == FE_PRODUCT_MATERIAL)
    {
        FEProductMaterial* psb = dynamic_cast<FEProductMaterial*>(pm); assert(psb);
        int idx = psb->GetIndex();
        int type = psb->GetProductType();
        el.value(psb->GetCoef());
        switch (type)
        {
		case FEReactionMaterial::SOLUTE_SPECIES: el.add_attribute("sol", idx + 1); break;
		case FEReactionMaterial::SBM_SPECIES   : el.add_attribute("sbm", idx + 1); break;
		default:
			assert(false);
        }
        m_xml.add_leaf(el);
        return;
    }
    else
        el.add_attribute("type", sztype);
    
	m_xml.add_branch(el);
	{
		// write the material axes (if any)
		if (pm->m_axes && (pm->m_axes->m_naopt > -1)) {
			el.name("mat_axis");
			if (pm->m_axes->m_naopt == FE_AXES_LOCAL)
			{
				el.add_attribute("type", "local");
				el.value(pm->m_axes->m_n,3);
				m_xml.add_leaf(el);
			}
			else if (pm->m_axes->m_naopt == FE_AXES_VECTOR)
			{
				el.add_attribute("type", "vector");
				m_xml.add_branch(el);
				{
					m_xml.add_leaf("a", pm->m_axes->m_a);
					m_xml.add_leaf("d", pm->m_axes->m_d);
				}
				m_xml.close_branch();
			}
		}
		
		// write the material parameters (if any)
		if (pm->Parameters()) WriteMaterialParams(pm);

		// write the components
		int NC = pm->Properties();
		for (int i=0; i<NC; ++i)
		{
			FEMaterialProperty& mc = pm->GetProperty(i);
			for (int j=0; j<mc.Size(); ++j)
			{
				FEMaterial* pc = mc.GetMaterial(j);
				if (pc)
				{
					el.name(mc.GetName().c_str());
					const string& name = pc->GetName();
					if (name.empty()) el.add_attribute("name", name.c_str());

					// TODO: some materials need to be treated as multi-materials
					//       although they technically aren't. I need to simplify this.
					bool is_multi = false;
					switch (pc->Type())
					{
					case FE_SBM_MATERIAL     : is_multi = true; break;
					case FE_REACTANT_MATERIAL: is_multi = true; break;
					case FE_PRODUCT_MATERIAL : is_multi = true; break;
					}

					if ((pc->Properties() > 0)||is_multi) WriteMultiMaterial(pc, el);
					else
					{
						el.add_attribute("type", FEMaterialFactory::TypeStr(pc));
						m_xml.add_branch(el);
						{
							WriteMaterialParams(pc);
						}
						m_xml.close_branch();
					}
				}
			}
		}
	}
	m_xml.close_branch();
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteGeometrySection()
{
	XMLElement el;

	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	vec3d r;

	// export the nodes
	WriteGeometryNodes();

	// Write the elements
	WriteGeometryElements();

	// see if we need to add an element data section
	bool bdata = false;
	if (model.ShellElements() > 0) bdata = true;
	for (int i=0; i<s.Materials(); ++i)
	{
		FETransverselyIsotropic* pmat = dynamic_cast<FETransverselyIsotropic*>(s.GetMaterial(i)->GetMaterialProperties());
		if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER)) bdata = true;
	}
	for (int i=0; i<model.Objects(); ++i)
	{
		FEMesh* pm = model.Object(i)->GetFEMesh();
		
		for (int j=0; j<pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			if (e.m_Qactive) {
				bdata = true;
				break;
			}
		}

		if (pm->MeshDataFields() > 0) bdata = true;
	}

	// write element data section if necessary
	if (bdata) WriteGeometryElementData();

	// write the node sets
	WriteGeometryNodeSets();

	// write named surfaces
	WriteGeometrySurfaces();
}

//-----------------------------------------------------------------------------
bool FEBioExport2::WriteNodeSet(const string& name, FENodeList* pl)
{
	int nn = pl->Size();
	FENodeList::Iterator pn = pl->First();
	vector<int> m(nn);
	for (int n=0; n<nn; ++n, pn++)
	{
		FENode* pnode = pn->m_pi;
		if (pnode == 0) return false;
		m[n] = pnode->m_nid;
	}

	XMLElement el("NodeSet");
	el.add_attribute("name", name.c_str());
	m_xml.add_branch(el);
	{
		XMLElement nd("node");
		nd.add_attribute("id",0);
		for (int n=0; n<nn; ++n)
		{
			nd.set_attribute(0, m[n]);
			m_xml.add_empty(nd, false);
		}
	}
	m_xml.close_branch();
	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometryNodeSets()
{
	// Write the user-defined node sets
	FEModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	int nobj = model.Objects();
	for (int i=0; i<nobj; ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nset = po->FENodeSets();
			for (int j=0; j<nset; ++j)
			{
				FENodeSet* pns = po->GetFENodeSet(j);
				auto_ptr<FENodeList> pl(pns->BuildNodeList());
				if (WriteNodeSet(pns->GetName(), pl.get()) == false)
				{
					throw InvalidItemListBuilder(po);
				}
			}
		}
	}

	// Write the BC node sets
	int NS = (int) m_pNSet.size();
	for (int i=0; i<NS; ++i)
	{
		FEItemListBuilder* pil = m_pNSet[i];
		auto_ptr<FENodeList> pl(pil->BuildNodeList());
		if (WriteNodeSet(pil->GetName(), pl.get()) == false)
		{
			throw InvalidItemListBuilder(pil);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometrySurfaces()
{
	int NS = (int)m_pSurf.size();
	for (int i=0; i<NS; ++i)
	{
		FEItemListBuilder* pl = m_pSurf[i];
		auto_ptr<FEFaceList> ps(pl->BuildFaceList());
		XMLElement el("Surface");
		el.add_attribute("name", pl->GetName().c_str());
		m_xml.add_branch(el);
		{
			WriteSurfaceSection(*ps);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometryNodes()
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	XMLWriter::SetFloatFormat(XMLWriter::ScientificFormat);

	// nodes
	m_xml.add_branch("Nodes");
	{
		XMLElement el("node");
		int nid = el.add_attribute("id", 0);

		int n = 1;
		for (int i=0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			FECoreMesh* pm = po->GetFEMesh();

			for (int j=0; j<pm->Nodes(); ++j, ++n)
			{
				FENode& node = pm->Node(j);
				node.m_nid = n;
				el.set_attribute(nid, n);
				vec3d r = po->GetTransform().LocalToGlobal(node.r);
				el.value(r);
				m_xml.add_leaf(el, false);
			}
		}
	}
	m_xml.close_branch(); // Nodes

	XMLWriter::SetFloatFormat(XMLWriter::FixedFormat);
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometryElements()
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// reset element counter
	m_ntotelem = 0;

	// loop over all objects
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		// loop over all parts
		int NP = po->Parts();
		for (int p=0; p<NP; ++p)
		{
			// get the part
			GPart* pg = po->Part(p);

			// write this part
			WriteGeometryPart(pg);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometryPart(GPart* pg)
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();
	GObject* po = dynamic_cast<GObject*>(pg->Object());
	FECoreMesh* pm = po->GetFEMesh();
	int pid = pg->GetLocalID();

	// Parts must be split up by element type
	int NE = pm->Elements();
	int NEP = 0; // number of elements in part
	for (int i=0; i<NE; ++i) 
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.m_gid == pid) { el.m_ntag = 1; NEP++; } else el.m_ntag = -1; 
	}

	// make sure this part has elements
	if (NEP == 0) return;

	// get the material
	int nmat = 0;
	GMaterial* pmat = s.GetMaterialFromID(pg->GetMaterialID());
	if (pmat) nmat = pmat->m_ntag;

	// loop over unprocessed elements
	int ncount = 0;
	int nn[FEElement::MAX_NODES];
	for (int i=0;ncount<NEP;++i)
	{
		FEElement_& el = pm->ElementRef(i);
		if (el.m_ntag == 1)
		{
			assert(el.m_gid == pid);
			int ntype = el.Type();
			const char* sztype = 0;
			switch (ntype)
			{
			case FE_TET4  :	sztype = "tet4"  ; break;
			case FE_TET10 :	sztype = "tet10" ; break;
			case FE_TET15 :	sztype = "tet15" ; break;
			case FE_TET20 : sztype = "tet20" ; break;
			case FE_PENTA6:	sztype = "penta6"; break;
			case FE_HEX8  :	sztype = "hex8"  ; break;
			case FE_HEX20 :	sztype = "hex20" ; break;
			case FE_HEX27 : sztype = "hex27" ; break;
			case FE_QUAD4 :	sztype = "quad4" ; break;
			case FE_TRI3  :	sztype = "tri3"  ; break;
            case FE_TRI6  :	sztype = "tri6"  ; break;
            case FE_QUAD8 :	sztype = "quad8" ; break;
            case FE_QUAD9 :	sztype = "quad9" ; break;
			case FE_PYRA5 : sztype = "pyra5" ; break;
			case FE_PENTA15: sztype = "penta15"; break;
            case FE_PYRA13: sztype = "pyra13"; break;
			default:
				assert(false);
			}
			XMLElement xe;
			xe.name("Elements");
			if (sztype) xe.add_attribute("type", sztype);
			if (nmat > 0) xe.add_attribute("mat", nmat);
			xe.add_attribute("elset", pg->GetName().c_str());

			m_xml.add_branch(xe);
			{
				XMLElement xej;
				xej.name("elem");
				int n1 = xej.add_attribute("id",(int)0);

				for (int j=i; j<NE; ++j)
				{
					FEElement_& ej = pm->ElementRef(j);
					if ((ej.m_ntag == 1) && (ej.Type() == ntype))
					{
						int eid = m_ntotelem + ncount + 1;
						xej.set_attribute(n1, eid);
						int ne = ej.Nodes();
						assert(ne == el.Nodes());
						for (int k=0; k<ne; ++k) nn[k] = pm->Node(ej.m_node[k]).m_nid;
						xej.value(nn, ne);
						m_xml.add_leaf(xej, false);
						ej.m_ntag = -1;	// mark as processed
						ej.m_nid = eid;
						ncount++;
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// update total element counter
	m_ntotelem += ncount;
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteGeometryElementData()
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	m_xml.add_branch("ElementData");

	int nid;
	XMLElement elem;
	elem.name("element");
	nid = elem.add_attribute("id", 0);
	for (int i=0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		int ND = pm->MeshDataFields();
		const Transform& T = po->GetTransform();

		for (int j=0; j<pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			GMaterial* pmat = s.GetMaterialFromID(po->Part(e.m_gid)->GetMaterialID());
			FETransverselyIsotropic* ptiso = 0;
			if (pmat) ptiso = dynamic_cast<FETransverselyIsotropic*>(pmat->GetMaterialProperties());

			elem.set_attribute(nid, e.m_nid);
			if (e.IsShell() || e.m_Qactive || (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER)) || (ND > 0))
			{
				m_xml.add_branch(elem, false);
				if (e.IsShell()) m_xml.add_leaf("thickness", e.m_h, e.Nodes());
				// if material is transversely isotropic with user-defined fibers,
				// export fiber direction, otherwise export local material orientation
				if (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER))
				{
					vec3d a = T.LocalToGlobalNormal(e.m_fiber);
					m_xml.add_leaf("fiber", a);
				}
				else if (e.m_Qactive) 
				{
					// e.m_Q is in local coordinates, so transform it to global coordinates
					mat3d& Q = e.m_Q;
					vec3d a(Q[0][0], Q[1][0], Q[2][0]);
					vec3d d(Q[0][1], Q[1][1], Q[2][1]);
					a = T.LocalToGlobalNormal(a);
					d = T.LocalToGlobalNormal(d);
					m_xml.add_branch("mat_axis");
					{
						m_xml.add_leaf("a", a);
						m_xml.add_leaf("d", d);
					}
					m_xml.close_branch();	// mat_axis
				}
				for (int k=0; k<ND; ++k)
				{
					FEElementData* data = dynamic_cast<FEElementData*>(pm->GetMeshDataField(k));
/*					if (data.GetTag(j) > 0)
					{
						double val = data[j];
						m_xml.add_leaf(data.GetName().c_str(), val);
					}
*/				}
				m_xml.close_branch(); // element
			}
		}
	}

	m_xml.close_branch(); // ElementData
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteBoundarySection(FEStep& s)
{
	XMLElement el;

	// --- B O U N D A R Y   C O N D I T I O N S ---
	// fixed constraints
	WriteBCFixed(s);

	// prescribed displacements
	WriteBCPrescribed(s);
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteContactSection(FEStep& s)
{
	// --- C O N T A C T ---
	// rigid interfaces
	WriteContactRigid(s);

	// sliding interfaces
	WriteContactSliding(s);

	// poro interfaces
	WriteContactPoro(s);

	// poro-solute interfaces
	WriteContactPoroSolute(s);

	// multiphasic interfaces
	WriteContactMultiphasic(s);
    
	// tied interfaces
	WriteContactTied(s);

	// sticky interfaces
	WriteContactSticky(s);

	// periodic boundaries
	WriteContactPeriodic(s);

	// rigid walls
	WriteContactWall(s);

	// rigid joints
	WriteContactJoint(s);

	// tension-compression
	WriteContactTC(s);

	// tied-biphasic
	WriteContactTiedPoro(s);

	// spring-tied interface
	WriteSpringTied(s);

	// linear constraints
	WriteLinearConstraints(s);
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteLoadsSection(FEStep& s)
{
	XMLElement el;

	// nodal forces
	WriteLoadNodal(s);

	// pressure forces
	WriteLoadPressure(s);

	// surface tractions
	WriteLoadTraction(s);

	// fluid flux
	WriteFluidFlux(s);

	// mixture normal traction
	WriteBPNormalTraction(s);

	// solute flux
	WriteSoluteFlux(s);

	// concentration flux
	WriteConcentrationFlux(s);

	// heat flux
	WriteHeatFlux(s);

	// convective heat flux
	WriteConvectiveHeatFlux(s);

	// body forces
	WriteBodyForces(s);

	// write heat sources
	WriteHeatSources(s);

    // fluid tractions
    WriteFluidTraction(s);
    
    // fluid velocities
    WriteFluidVelocity(s);
    
    // fluid normal velocities
    WriteFluidNormalVelocity(s);
    
    // fluid rotational velocities
    WriteFluidRotationalVelocity(s);
    
    // fluid flow resistance
    WriteFluidFlowResistance(s);
    
    // fluid backflow stabilization
    WriteFluidBackflowStabilization(s);
    
    // fluid tangential stabilization
    WriteFluidTangentialStabilization(s);
    
    // fluid-FSI interface traction
    WriteFSITraction(s);
    
}

//-----------------------------------------------------------------------------
// write discrete elements
//
void FEBioExport2::WriteDiscreteSection(FEStep& s)
{
	FEModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i=0; i<model.DiscreteObjects(); ++i)
	{
		GLinearSpring* ps = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
		if (ps)
		{
			GNode* pn0 = model.FindNode(ps->m_node[0]);
			GNode* pn1 = model.FindNode(ps->m_node[1]);

			if (pn0 && pn1)
			{
				m_xml.add_branch("spring");
				{
					int n[2];
					GObject* po; FENode* pn;

					po = dynamic_cast<GObject*>(pn0->Object()); assert(po);
					pn = po->GetFENode(pn0->GetLocalID()); assert(pn);
					n[0] = pn->m_nid;

					po = dynamic_cast<GObject*>(pn1->Object()); assert(po);
					pn = po->GetFENode(pn1->GetLocalID()); assert(pn);
					n[1] = pn->m_nid;

					double E = ps->GetFloatValue(GLinearSpring::MP_E);

					m_xml.add_leaf("node", n, 2);
					m_xml.add_leaf("E", E);
				}
				m_xml.close_branch(); // spring
			}
		}
		GGeneralSpring* pg = dynamic_cast<GGeneralSpring*>(model.DiscreteObject(i));
		if (pg)
		{
			GNode* pn0 = model.FindNode(pg->m_node[0]);
			GNode* pn1 = model.FindNode(pg->m_node[1]);

			if (pn0 && pn1)
			{
				XMLElement e;
				e.name("spring");
				e.add_attribute("type", "nonlinear");
				m_xml.add_branch(e);
				{
					GObject* po0 = dynamic_cast<GObject*>(pn0->Object());
					GObject* po1 = dynamic_cast<GObject*>(pn1->Object());

					int n[2];
					n[0] = po0->GetFENode(pn0->GetLocalID())->m_nid;
					n[1] = po1->GetFENode(pn1->GetLocalID())->m_nid;

					Param& p = pg->GetParam(GGeneralSpring::MP_F);

					double F = p.GetFloatValue();
					int lc = p.GetLoadCurve()->GetID();

					m_xml.add_leaf("node", n, 2);

					XMLElement f;
					f.name("force");
					f.value(F);
					f.add_attribute("lc", lc);
					m_xml.add_leaf(f);
				}
				m_xml.close_branch(); // spring
			}
		}
		GDiscreteSpringSet* pl = dynamic_cast<GDiscreteSpringSet*>(model.DiscreteObject(i));
		if (pl)
		{
			FEDiscreteMaterial* m = pl->GetMaterial();
			if (dynamic_cast<FELinearSpringMaterial*>(m))
			{
				double E = m->GetFloatValue(0);
				int N = pl->size();
				for (int n=0; n<N; ++n)
				{
					GDiscreteElement& el = pl->element(n);
					GNode* pn0 = model.FindNode(el.Node(0));
					GNode* pn1 = model.FindNode(el.Node(1));
					if (pn0 && pn1)
					{
						int n[2];
						GObject* po; FENode* pn;

						po = dynamic_cast<GObject*>(pn0->Object()); assert(po);
						pn = po->GetFENode(pn0->GetLocalID()); assert(pn);
						n[0] = pn->m_nid;

						po = dynamic_cast<GObject*>(pn1->Object()); assert(po);
						pn = po->GetFENode(pn1->GetLocalID()); assert(pn);
						n[1] = pn->m_nid;

						m_xml.add_branch("spring");
						{
							m_xml.add_leaf("node", n, 2);
							m_xml.add_leaf("E", E);
						}
						m_xml.close_branch();
					}
				}
			}
			else if (dynamic_cast<FENonLinearSpringMaterial*>(m))
			{
				double F = 0; // TODO: Get the F value
				int N = pl->size();
				for (int n = 0; n < N; ++n)
				{
					GDiscreteElement& el = pl->element(n);
					GNode* pn0 = model.FindNode(el.Node(0));
					GNode* pn1 = model.FindNode(el.Node(1));
					if (pn0 && pn1)
					{
						int n[2];
						GObject* po; FENode* pn;

						po = dynamic_cast<GObject*>(pn0->Object()); assert(po);
						pn = po->GetFENode(pn0->GetLocalID()); assert(pn);
						n[0] = pn->m_nid;

						po = dynamic_cast<GObject*>(pn1->Object()); assert(po);
						pn = po->GetFENode(pn1->GetLocalID()); assert(pn);
						n[1] = pn->m_nid;

						XMLElement el("spring");
						el.add_attribute("type", "nonlinear");
						m_xml.add_branch(el);
						{
							m_xml.add_leaf("node", n, 2);

							XMLElement f("force");
//							f.add_attribute("lc", lc);
							f.value(F);
							m_xml.add_leaf(f);
						}
						m_xml.close_branch();
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid joints
//
void FEBioExport2::WriteContactJoint(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		// rigid joints
		FERigidJoint* pj = dynamic_cast<FERigidJoint*> (s.Interface(i));
		if (pj && pj->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type","rigid joint");
            const char* sz = pj->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				int na = (pj->m_pbodyA? pj->m_pbodyA->m_ntag:0);
				int nb = (pj->m_pbodyB? pj->m_pbodyB->m_ntag:0);

				m_xml.add_leaf("tolerance", pj->GetFloatValue(FERigidJoint::TOL));
				m_xml.add_leaf("penalty"  , pj->GetFloatValue(FERigidJoint::PENALTY));
				m_xml.add_leaf("body_a"   , na);
				m_xml.add_leaf("body_b"   , nb);

				vec3d v = pj->GetVecValue(FERigidJoint::RJ);
				m_xml.add_leaf("joint"    , v);
			}
			m_xml.close_branch(); // contact - rigid joint
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid walls
//
void FEBioExport2::WriteContactWall(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FERigidWallInterface* pw = dynamic_cast<FERigidWallInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "rigid_wall");
            const char* sz = pw->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				m_xml.add_leaf("laugon", (pw->GetBoolValue(FERigidWallInterface::LAUGON)? 1 : 0));
				m_xml.add_leaf("tolerance", pw->GetFloatValue(FERigidWallInterface::ALTOL));
				m_xml.add_leaf("penalty", pw->GetFloatValue(FERigidWallInterface::PENALTY));

				FELoadCurve* plc = pw->GetParamLC(FERigidWallInterface::OFFSET);

				XMLElement plane;
				if (plc) plane.add_attribute("lc", plc->GetID());
				plane.name("plane");
				double a[4];
				a[0] = pw->GetFloatValue(FERigidWallInterface::PA);
				a[1] = pw->GetFloatValue(FERigidWallInterface::PB);
				a[2] = pw->GetFloatValue(FERigidWallInterface::PC);
				a[3] = pw->GetFloatValue(FERigidWallInterface::PD);
				plane.value(a, 4);
				m_xml.add_leaf(plane);

				int j, k;
				int nn[4], n=1;

				// slave surface
				XMLElement el("surface");
				m_xml.add_branch(el, false);
				{
					XMLElement ef;
					FEItemListBuilder* pitem = pw->GetItemList();
					if (pitem == 0) throw InvalidItemListBuilder(pw);
					auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
					FEFaceList::Iterator pf = pg->First();
					for (j=0; j<pg->Size(); ++j, ++pf)
					{
						FEFace& face = *(pf->m_pi);
						FECoreMesh* pm = pf->m_pm;
						for (k=0; k<face.Nodes(); ++k) nn[k] = pm->Node(face.n[k]).m_nid;
						switch(face.Nodes())
						{
						case 3:
							{
								ef.name("tri3");
								ef.add_attribute("id", n);
								ef.value(nn, 3);
							}
							break;
						case 4:
							{
                                ef.name("quad4");
                                ef.add_attribute("id", n);
                                ef.value(nn, 4);
							}
							break;
                        case 6:
                            {
                                ef.name("tri6");
                                ef.add_attribute("id", n);
                                ef.value(nn, 6);
                            }
                            break;
                        case 8:
                            {
                                ef.name("quad8");
                                ef.add_attribute("id", n);
                                ef.value(nn, 8);
                            }
                            break;
                        case 9:
                            {
                                ef.name("quad9");
                                ef.add_attribute("id", n);
                                ef.value(nn, 9);
                            }
                            break;
						}
						m_xml.add_leaf(ef);
						++n;
					}
				}
				m_xml.close_branch();
			}
			m_xml.close_branch(); // contact - sliding
		}
	}
}

//-----------------------------------------------------------------------------
// write poro-contact
//
void FEBioExport2::WriteContactPoro(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FEPoroContact* pp = dynamic_cast<FEPoroContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-biphasic");
			const char* sz = pp->GetName().c_str();
            ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pp->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pp->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pp->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding2
		}
	}
}

//-----------------------------------------------------------------------------
// write poro-solute contact
//
void FEBioExport2::WriteContactPoroSolute(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FEPoroSoluteContact* pp = dynamic_cast<FEPoroSoluteContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding3");
			const char* sz = pp->GetName().c_str();
            ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pp->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pp->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pp->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding3
		}
	}
}

//-----------------------------------------------------------------------------
// write multiphasic contact
//
void FEBioExport2::WriteContactMultiphasic(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FEMultiphasicContact* pp = dynamic_cast<FEMultiphasicContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-multiphasic");
			const char* sz = pp->GetName().c_str();
            ec.add_attribute("name", sz);
            
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pp->GetParam(n));
                
				// master surface
				FEItemListBuilder* pms = pp->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}
                
				// slave surface
				FEItemListBuilder* pss = pp->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding-multiphasic
		}
	}
}

//-----------------------------------------------------------------------------
// write Tension-Compression contact
//
void FEBioExport2::WriteContactTC(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FETensionCompressionInterface* pp = dynamic_cast<FETensionCompressionInterface*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-tension-compression");
			const char* sz = pp->GetName().c_str();
            ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pp->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pp->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pp->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact
		}
	}
}

//-----------------------------------------------------------------------------
// write Tied-Biphasic interface
//
void FEBioExport2::WriteContactTiedPoro(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FETiedBiphasicInterface* pp = dynamic_cast<FETiedBiphasicInterface*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "tied-biphasic");
			const char* sz = pp->GetName().c_str();
            ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pp->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pp->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pp->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid interfaces
//
void FEBioExport2::WriteContactRigid(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		// rigid interfaces
		FERigidInterface* pr = dynamic_cast<FERigidInterface*> (s.Interface(i));
		if (pr && pr->IsActive())
		{
			GMaterial* pm = pr->GetRigidBody();
			if (pm==0) throw RigidContactException();
			int rb = pm->m_ntag;

			int i, j;
			vector<int> RC; RC.resize(m_nodes);
			for (i=0; i<m_nodes; ++i) RC[i] = 0;

			FEItemListBuilder* pitem = pr->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pr);
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
			for (j=0; j<pg->Size(); ++j, ++pn) RC[ (pn->m_pi)->m_nid-1 ] = 1;

			XMLElement ec("contact");
			ec.add_attribute("type", "rigid");
			const char* sz = pr->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				XMLElement el("node");
				int n1 = el.add_attribute("id", 0);
				int n2 = el.add_attribute("rb", 0);
	
				for (j=0; j<m_nodes; ++j)
				{
					if (RC[j])
					{
						el.set_attribute(n1, j+1);
						el.set_attribute(n2, rb);
						m_xml.add_leaf(el, false);
					}
				}
			}
			m_xml.close_branch(); // contact - rigid
		}
	}
}

//-----------------------------------------------------------------------------
// write tied interfaces
//
void FEBioExport2::WriteContactTied(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FETiedInterface* pt = dynamic_cast<FETiedInterface*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "tied");
			const char* sz = pt->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pt->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pt->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pt->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - tied
		}	
	}
}

//-----------------------------------------------------------------------------
// write tied interfaces
//
void FEBioExport2::WriteContactSticky(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FEStickyInterface* pt = dynamic_cast<FEStickyInterface*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sticky");
			const char* sz = pt->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pt->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pt->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pt->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sticky
		}	
	}
}

//-----------------------------------------------------------------------------
// write periodic boundary constraints
//
void FEBioExport2::WriteContactPeriodic(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FEPeriodicBoundary* pt = dynamic_cast<FEPeriodicBoundary*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "periodic boundary");
			const char* sz = pt->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pt->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pt->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pt->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - periodic
		}	
	}
}

//-----------------------------------------------------------------------------
// write sliding interfaces
//
void FEBioExport2::WriteContactSliding(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FESlidingInterface* ps = dynamic_cast<FESlidingInterface*> (s.Interface(i));
		if (ps && ps->IsActive())
		{
			XMLElement ec("contact");
			int ntype = ps->GetIntValue(FESlidingInterface::NTYPE);
			if      (ntype == 0) ec.add_attribute("type", "sliding_with_gaps");
			else if (ntype == 1) ec.add_attribute("type", "facet-to-facet sliding"   );
			const char* sz = ps->GetName().c_str();
            ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters (except type)
				int NP = ps->Parameters();
				for (int n=0; n<NP; ++n)
				{
					if (n != FESlidingInterface::NTYPE)
					{
						Param& p = ps->GetParam(n);
						WriteParam(p);
					}
				}

				// master surface
				FEItemListBuilder* pms = ps->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = ps->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding
		}

		FESlidingWithGapsInterface* pswg = dynamic_cast<FESlidingWithGapsInterface*> (s.Interface(i));
		if (pswg && pswg->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding_with_gaps");
			const char* sz = pswg->GetName().c_str();
			ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pswg->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pswg->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pswg->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pswg->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding
		}

		FEFacetOnFacetInterface* pf2f = dynamic_cast<FEFacetOnFacetInterface*> (s.Interface(i));
		if (pf2f && pf2f->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "facet-to-facet sliding");
			const char* sz = pf2f->GetName().c_str();
			ec.add_attribute("name", sz);

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pf2f->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pf2f->GetParam(n));

				// master surface
				FEItemListBuilder* pms = pf2f->GetSecondarySurface();
				if (pms)
				{
					XMLElement el("surface");
					el.add_attribute("type", "master");
					WriteSurface(el, pms);
				}

				// slave surface
				FEItemListBuilder* pss = pf2f->GetPrimarySurface();
				if (pss)
				{
					XMLElement el("surface");
					el.add_attribute("type", "slave");
					WriteSurface(el, pss);
				}
			}
			m_xml.close_branch(); // contact - sliding
		}
	}
}

//-----------------------------------------------------------------------------
// write Tied-Biphasic interface
//
void FEBioExport2::WriteSpringTied(FEStep& s)
{
	for (int i=0; i<s.Interfaces(); ++i)
	{
		FESpringTiedInterface* ps = dynamic_cast<FESpringTiedInterface*> (s.Interface(i));
		if (ps && ps->IsActive())
		{
			double E = ps->GetFloatValue(FESpringTiedInterface::ECONST);
			vector<pair<int, int> > L;
			ps->BuildSpringList(L);
			if (L.empty() == false)
			{
				for (int j=0; j<(int)L.size(); ++j)
				{
					int n[2] = {L[j].first, L[j].second};
					m_xml.add_branch("spring");
					{
						m_xml.add_leaf("node", n, 2);
						m_xml.add_leaf("E", E);
					}
					m_xml.close_branch();
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// write linear constraints
void FEBioExport2::WriteLinearConstraints(FEStep& s)
{
	const char* szbc[]={"x","y","z"};

	for (int i=0; i<s.LinearConstraints(); ++i)
	{
		FELinearConstraintSet* pset = s.LinearConstraint(i);
		XMLElement ec("contact");
		ec.add_attribute("type", "linear constraint");
		m_xml.add_branch(ec);
		{
			m_xml.add_leaf("tol"    , pset->m_atol   );
			m_xml.add_leaf("penalty", pset->m_penalty);
			m_xml.add_leaf("maxaug" , pset->m_nmaxaug);

			int NC = (int) pset->m_set.size();
			for (int j=0; j<NC; ++j)
			{
				FELinearConstraintSet::LinearConstraint& LC = pset->m_set[j];
				m_xml.add_branch("linear_constraint");
				{
					int ND = (int) LC.m_dof.size();
					XMLElement ed("node");
					int n1 = ed.add_attribute("id", 0);
					int n2 = ed.add_attribute("bc", 0);
					for (int n=0; n<ND; ++n)
					{
						FELinearConstraintSet::LinearConstraint::DOF& dof = LC.m_dof[n];
						ed.set_attribute(n1, dof.node);
						ed.set_attribute(n2, szbc[dof.bc] );
						ed.value(dof.s);
						m_xml.add_leaf(ed, false);
					}
				}
				m_xml.close_branch();
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteVolumeConstraint(FEStep& s)
{
	for (int i=0; i<s.Constraints(); ++i)
	{
		FEVolumeConstraint* pw = dynamic_cast<FEVolumeConstraint*> (s.Constraint(i));
		if (pw && pw->IsActive())
		{
			XMLElement ec("constraint");
			ec.add_attribute("type", "volume");
			const char* sz = pw->GetName().c_str();
            ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pw->Parameters();
				for (int n=0; n<NP; ++n) WriteParam(pw->GetParam(n));

				FEItemListBuilder* pitem = pw->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pw);

				// slave surface
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // constraint
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteSymmetryPlane(FEStep& s)
{
	for (int i = 0; i<s.Constraints(); ++i)
	{
		FESymmetryPlane* pw = dynamic_cast<FESymmetryPlane*> (s.Constraint(i));
		if (pw && pw->IsActive())
		{
			XMLElement ec("constraint");
			ec.add_attribute("type", "symmetry plane");
			const char* sz = pw->GetName().c_str();
			ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pw->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pw->GetParam(n));

				FEItemListBuilder* pitem = pw->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pw);

				// slave surface
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // constraint
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteNormalFlow(FEStep& s)
{
    for (int i = 0; i<s.Constraints(); ++i)
    {
        FENormalFlowSurface* pw = dynamic_cast<FENormalFlowSurface*> (s.Constraint(i));
        if (pw && pw->IsActive())
        {
            XMLElement ec("constraint");
            ec.add_attribute("type", "normal fluid flow");
            const char* sz = pw->GetName().c_str();
            ec.add_attribute("name", sz);
            m_xml.add_branch(ec);
            {
                // write all parameters
                int NP = pw->Parameters();
                for (int n = 0; n<NP; ++n) WriteParam(pw->GetParam(n));
                
                FEItemListBuilder* pitem = pw->GetItemList();
                if (pitem == 0) throw InvalidItemListBuilder(pw);
                
                // slave surface
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // constraint
        }
    }
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteFrictionlessFluidWall(FEStep& s)
{
    for (int i = 0; i<s.Constraints(); ++i)
    {
        FEFrictionlessFluidWall* pw = dynamic_cast<FEFrictionlessFluidWall*> (s.Constraint(i));
        if (pw && pw->IsActive())
        {
            XMLElement ec("constraint");
            ec.add_attribute("type", "frictionless fluid wall");
            const char* sz = pw->GetName().c_str();
            ec.add_attribute("name", sz);
            m_xml.add_branch(ec);
            {
                // write all parameters
                int NP = pw->Parameters();
                for (int n = 0; n<NP; ++n) WriteParam(pw->GetParam(n));

                FEItemListBuilder* pitem = pw->GetItemList();
                if (pitem == 0) throw InvalidItemListBuilder(pw);

                // slave surface
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // constraint
        }
    }
}

//-----------------------------------------------------------------------------
// write rigid connectors
//
void FEBioExport2::WriteConnectors(FEStep& s)
{
    for (int i=0; i<s.RigidConnectors(); ++i)
    {
        // rigid connectors
		FERigidConnector* pj = s.RigidConnector(i);
        if (pj && pj->IsActive())
        {
            XMLElement ec("constraint");
            if (dynamic_cast<FERigidSphericalJoint*       >(pj)) ec.add_attribute("type","rigid spherical joint");
            else if (dynamic_cast<FERigidRevoluteJoint*   >(pj)) ec.add_attribute("type","rigid revolute joint");
            else if (dynamic_cast<FERigidPrismaticJoint*  >(pj)) ec.add_attribute("type","rigid prismatic joint");
            else if (dynamic_cast<FERigidCylindricalJoint*>(pj)) ec.add_attribute("type","rigid cylindrical joint");
            else if (dynamic_cast<FERigidPlanarJoint*     >(pj)) ec.add_attribute("type","rigid planar joint");
            else if (dynamic_cast<FERigidLock*            >(pj)) ec.add_attribute("type","rigid lock");
            else if (dynamic_cast<FERigidSpring*          >(pj)) ec.add_attribute("type","rigid spring");
            else if (dynamic_cast<FERigidDamper*          >(pj)) ec.add_attribute("type","rigid damper");
            else if (dynamic_cast<FERigidAngularDamper*   >(pj)) ec.add_attribute("type","rigid angular damper");
            else if (dynamic_cast<FERigidContractileForce*>(pj)) ec.add_attribute("type","rigid contractile force");
            else
                assert(false);

			GMaterial* pgA = m_pfem->GetMaterialFromID(pj->m_rbA);
			if (pgA == 0) throw MissingRigidBody(pj->GetName().c_str());
			FERigidMaterial* pA = dynamic_cast<FERigidMaterial*>(pgA->GetMaterialProperties());
			if (pA == 0) throw InvalidMaterialReference();

			GMaterial* pgB = m_pfem->GetMaterialFromID(pj->m_rbB);
			if (pgB == 0) throw MissingRigidBody(pj->GetName().c_str());
			FERigidMaterial* pB = dynamic_cast<FERigidMaterial*>(pgB->GetMaterialProperties());
			if (pB == 0) throw InvalidMaterialReference();

			const char* sz = pj->GetName().c_str();
			ec.add_attribute("name", sz);
			m_xml.add_branch(ec);
			{
				int na = pgA->m_ntag;
				int nb = pgB->m_ntag;
				m_xml.add_leaf("body_a", na);
				m_xml.add_leaf("body_b", nb);

				WriteParamList(*pj);
			}
			m_xml.close_branch();
		}
    }
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteBCFixed(FEStep &s)
{
	for (int i=0; i<s.BCs(); ++i)
	{
		FEBoundaryCondition* pbc = s.BC(i);
		if (pbc->IsActive())
		{
			switch(pbc->Type())
			{
			case FE_FIXED_DISPLACEMENT      : WriteBCFixedDisplacement     (dynamic_cast<FEFixedDisplacement& >(*pbc), s); break;
			case FE_FIXED_SHELL_DISPLACEMENT: WriteBCFixedShellDisplacement(dynamic_cast<FEFixedShellDisplacement& >(*pbc), s); break;
			case FE_FIXED_ROTATION          : WriteBCFixedRotation         (dynamic_cast<FEFixedRotation&     >(*pbc), s); break;
			case FE_FIXED_FLUID_PRESSURE    : WriteBCFixedFluidPressure    (dynamic_cast<FEFixedFluidPressure&>(*pbc), s); break;
			case FE_FIXED_TEMPERATURE       : WriteBCFixedTemperature      (dynamic_cast<FEFixedTemperature&  >(*pbc), s); break;
			case FE_FIXED_CONCENTRATION     : WriteBCFixedConcentration    (dynamic_cast<FEFixedConcentration&>(*pbc), s); break;
            case FE_FIXED_FLUID_VELOCITY    : WriteBCFixedFluidVelocity    (dynamic_cast<FEFixedFluidVelocity&>(*pbc), s); break;
            case FE_FIXED_DILATATION        : WriteBCFixedFluidDilatation  (dynamic_cast<FEFixedFluidDilatation&   >(*pbc), s); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteBCFixedDisplacement(FEFixedDisplacement& rbc, FEStep& s)
{
	const char* xyz[] = {"x", "y", "xy", "z", "xz", "yz", "xyz"};

	// get the item list
	FEItemListBuilder* pItem = rbc.GetItemList();

	// see if this is defined in the geometry section or not
	const char* szset = 0;
	FENodeList* pns = 0;
	if (HasNodeSet(pItem))
	{
		szset = pItem->GetName().c_str();
	}
	else
	{
		// build the node list
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);
		pns = pItem->BuildNodeList();
		if (pns == 0) throw InvalidItemListBuilder(&rbc);
	}

	auto_ptr<FENodeList> pg(pns);

	// get the BC for this constraint
	int bc = rbc.GetBC();

	// figure out the BC string
	const char* sbc = 0;
	int nbc = 0;
	nbc = bc&7;
	assert(nbc < 8);

	if (nbc != 0)
	{
		sbc = xyz[nbc-1];

		XMLElement el;
		el.name("fix");
		el.add_attribute("bc", sbc);

		if (szset)
		{
			el.add_attribute("set", szset);
			m_xml.add_empty(el);
		}
		else
		{
			assert(pns);
			// write fix DOFS
			m_xml.add_branch(el);
			{
				XMLElement el;
				el.name("node");
				int n1 = el.add_attribute("id", 0);

				// write the BC's
				int N = pg->Size();
				FENodeList::Iterator pi = pg->First();
				for (int k=0; k<N; ++k, ++pi)
				{
					FENode* pn = pi->m_pi;
					int nid = pn->m_nid;

					el.set_attribute(n1, nid);
					m_xml.add_empty(el, false);
				}
			}
			m_xml.close_branch(); // fix
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteBCFixedShellDisplacement(FEFixedShellDisplacement& rbc, FEStep& s)
{
	const char* xyz[] = { "sx", "sy", "sxy", "sz", "sxz", "syz", "sxyz" };

	// get the item list
	FEItemListBuilder* pItem = rbc.GetItemList();

	// see if this is defined in the geometry section or not
	const char* szset = 0;
	FENodeList* pns = 0;
	if (HasNodeSet(pItem))
	{
		szset = pItem->GetName().c_str();
	}
	else
	{
		// build the node list
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);
		pns = pItem->BuildNodeList();
		if (pns == 0) throw InvalidItemListBuilder(&rbc);
	}

	auto_ptr<FENodeList> pg(pns);

	// get the BC for this constraint
	int bc = rbc.GetBC();

	// figure out the BC string
	const char* sbc = 0;
	int nbc = 0;
	nbc = bc & 7;
	assert(nbc < 8);

	if (nbc != 0)
	{
		sbc = xyz[nbc - 1];

		XMLElement el;
		el.name("fix");
		el.add_attribute("bc", sbc);

		if (szset)
		{
			el.add_attribute("set", szset);
			m_xml.add_empty(el);
		}
		else
		{
			assert(pns);
			// write fix DOFS
			m_xml.add_branch(el);
			{
				XMLElement el;
				el.name("node");
				int n1 = el.add_attribute("id", 0);

				// write the BC's
				int N = pg->Size();
				FENodeList::Iterator pi = pg->First();
				for (int k = 0; k<N; ++k, ++pi)
				{
					FENode* pn = pi->m_pi;
					int nid = pn->m_nid;

					el.set_attribute(n1, nid);
					m_xml.add_empty(el, false);
				}
			}
			m_xml.close_branch(); // fix
		}
	}
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport2::WriteBCFixedRotation(FEFixedRotation& rbc, FEStep& s)
{
	const char* uvw[] = {"u", "v", "uv", "w", "uw", "vw", "uvw"};

	// get the item list
	FEItemListBuilder* pItem = rbc.GetItemList();

	// see if this is defined in the geometry section or not
	const char* szset = 0;
	FENodeList* pns = 0;
	if (HasNodeSet(pItem))
	{
		szset = pItem->GetName().c_str();
	}
	else
	{
		// build the node list
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);
		pns = pItem->BuildNodeList();
		if (pns == 0) throw InvalidItemListBuilder(&rbc);
	}

	auto_ptr<FENodeList> pg(pns);

	// get the BC for this constraint
	int bc = rbc.GetBC();

	// figure out the BC string
	const char* sbc = 0;
	int nbc = 0;
	nbc = bc&7;
	assert(nbc < 8);

	if (nbc != 0)
	{
		sbc = uvw[nbc-1];

		XMLElement el;
		el.name("fix");
		el.add_attribute("bc", sbc);

		if (szset)
		{
			el.add_attribute("set", szset);
			m_xml.add_empty(el);
		}
		else
		{
			assert(pns);
			// write fix DOFS
			m_xml.add_branch(el);
			{
				XMLElement el;
				el.name("node");
				int n1 = el.add_attribute("id", 0);

				// write the BC's
				int N = pg->Size();
				FENodeList::Iterator pi = pg->First();
				for (int k=0; k<N; ++k, ++pi)
				{
					FENode* pn = pi->m_pi;
					int nid = pn->m_nid;

					el.set_attribute(n1, nid);
					m_xml.add_empty(el, false);
				}
			}
			m_xml.close_branch(); // fix
		}
	}
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport2::WriteBCFixedFluidPressure(FEFixedFluidPressure& rbc, FEStep& s)
{
	// build the node list
	FEItemListBuilder* pItem = rbc.GetItemList();

	// see if this is defined in the geometry section or not
	const char* szset = 0;
	FENodeList* pns = 0;
	if (HasNodeSet(pItem))
	{
		szset = pItem->GetName().c_str();
	}
	else
	{
		// build the node list
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);
		pns = pItem->BuildNodeList();
		if (pns == 0) throw InvalidItemListBuilder(&rbc);
	}

	auto_ptr<FENodeList> pg(pns);

	// get the BC for this constraint
	int bc = rbc.GetBC();

	XMLElement tag("fix");
	tag.add_attribute("bc", "p");

	if (szset)
	{
		tag.add_attribute("set", szset);
		m_xml.add_empty(tag);
	}
	else
	{
		m_xml.add_branch(tag);
		{
			if (pItem == 0) throw InvalidItemListBuilder(&rbc);

			FENodeList::Iterator it = pg->First();
			for (int k=0; k<pg->Size(); ++k, ++it) 
			{
				FENode& node = *(it->m_pi);

				XMLElement el("node");
				el.add_attribute("id",node.m_nid);
				m_xml.add_empty(el);
			}
		}
		m_xml.close_branch(); // fix
	}
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport2::WriteBCFixedTemperature(FEFixedTemperature& rbc, FEStep& s)
{
	XMLElement fix("fix");
	fix.add_attribute("bc", "t");
	m_xml.add_branch(fix);
	{
		XMLElement el;
		el.name("node");
		int n1 = el.add_attribute("id", 0);

		// build the node list
		FEItemListBuilder* pItem = rbc.GetItemList();
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);

		auto_ptr<FENodeList> pg(pItem->BuildNodeList());

		FENodeList::Iterator it = pg->First();
		for (int k=0; k<pg->Size(); ++k, ++it) 
		{
			FENode& node = *(it->m_pi);
			el.set_attribute(n1, node.m_nid);
			m_xml.add_empty(el, false);
		}
	}
	m_xml.close_branch(); // fix
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport2::WriteBCFixedConcentration(FEFixedConcentration& rbc, FEStep& s)
{
	vector<int> BC; BC.resize(m_nodes);

	// TODO: This won't work when multiple constraints are selected
	// get the BC for this constraint
	int bc = rbc.GetBC();

	// build the constraint string
	const char* szbc="";
	if (bc &  1) szbc = "c1";
	if (bc &  2) szbc = "c2";
	if (bc &  4) szbc = "c3";
	if (bc &  8) szbc = "c4";
	if (bc & 16) szbc = "c5";
	if (bc & 32) szbc = "c6";

	XMLElement fix("fix");
	fix.add_attribute("bc", szbc);

	m_xml.add_branch(fix);
	{
		XMLElement el;
		el.name("node");
		int n1 = el.add_attribute("id", 0);

		// build the node list
		FEItemListBuilder* pItem = rbc.GetItemList();
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);

		auto_ptr<FENodeList> pg(pItem->BuildNodeList());

		FENodeList::Iterator it = pg->First();
		for (int k=0; k<pg->Size(); ++k, ++it) 
		{
			FENode& node = *(it->m_pi);
			el.set_attribute(n1, node.m_nid);
			m_xml.add_empty(el, false); 
		}
	}
	m_xml.close_branch(); // fix
}

//-----------------------------------------------------------------------------
// Export the fixed velocity degrees of freedom
//
void FEBioExport2::WriteBCFixedFluidVelocity(FEFixedFluidVelocity& rbc, FEStep& s)
{
    const char* xyz[] = {"wx", "wy", "wxy", "wz", "wxz", "wyz", "wxyz"};
    
    // get the item list
    FEItemListBuilder* pItem = rbc.GetItemList();
    
    // see if this is defined in the geometry section or not
    const char* szset = 0;
    FENodeList* pns = 0;
    if (HasNodeSet(pItem))
    {
		szset = pItem->GetName().c_str();
    }
    else
    {
        // build the node list
        if (pItem == 0) throw InvalidItemListBuilder(&rbc);
        pns = pItem->BuildNodeList();
        if (pns == 0) throw InvalidItemListBuilder(&rbc);
    }
    
    auto_ptr<FENodeList> pg(pns);
    
    // get the BC for this constraint
    int bc = rbc.GetBC();
    
    // figure out the BC string
    const char* sbc = 0;
    int nbc = 0;
    nbc = bc&7;
    assert(nbc < 8);
    
    if (nbc != 0)
    {
        sbc = xyz[nbc-1];
        
        XMLElement el;
        el.name("fix");
        el.add_attribute("bc", sbc);
        
        if (szset)
        {
            el.add_attribute("set", szset);
            m_xml.add_empty(el);
        }
        else
        {
            assert(pns);
            // write fix DOFS
            m_xml.add_branch(el);
            {
                XMLElement el;
                el.name("node");
                int n1 = el.add_attribute("id", 0);
                
                // write the BC's
                int N = pg->Size();
                FENodeList::Iterator pi = pg->First();
                for (int k=0; k<N; ++k, ++pi)
                {
                    FENode* pn = pi->m_pi;
                    int nid = pn->m_nid;
                    
                    el.set_attribute(n1, nid);
                    m_xml.add_empty(el, false);
                }
            }
            m_xml.close_branch(); // fix
        }
    }
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport2::WriteBCFixedFluidDilatation(FEFixedFluidDilatation& rbc, FEStep& s)
{
    vector<int> BC; BC.resize(m_nodes);
    
    // get the BC for this constraint
    int bc = rbc.GetBC();
    
    XMLElement tag("fix");
    tag.add_attribute("bc", "ef");
    
    m_xml.add_branch(tag);
    {
        // build the node list
        FEItemListBuilder* pItem = rbc.GetItemList();
        if (pItem == 0) throw InvalidItemListBuilder(&rbc);
        
        auto_ptr<FENodeList> pg(pItem->BuildNodeList());
        
        FENodeList::Iterator it = pg->First();
        for (int k=0; k<pg->Size(); ++k, ++it)
        {
            FENode& node = *(it->m_pi);
            
            XMLElement el("node");
            el.add_attribute("id",node.m_nid);
            m_xml.add_empty(el);
        }
    }
    m_xml.close_branch(); // fix
}

//-----------------------------------------------------------------------------
// Export prescribed boundary conditions
void FEBioExport2::WriteBCPrescribed(FEStep &s)
{
	for (int i=0; i<s.BCs(); ++i)
	{
		FEBoundaryCondition* pbc = s.BC(i);
		if (pbc->IsActive())
		{
			switch (pbc->Type())
			{
			case FE_PRESCRIBED_DISPLACEMENT  : WriteBCPrescribedDisplacement (dynamic_cast<FEPrescribedDisplacement &>(*pbc), s); break;
			case FE_PRESCRIBED_ROTATION      : WriteBCPrescribedRotation     (dynamic_cast<FEPrescribedRotation     &>(*pbc), s); break;
			case FE_PRESCRIBED_FLUID_PRESSURE: WriteBCPrescribedFluidPressure(dynamic_cast<FEPrescribedFluidPressure&>(*pbc), s); break;
			case FE_PRESCRIBED_TEMPERATURE   : WriteBCPrescribedTemperature  (dynamic_cast<FEPrescribedTemperature  &>(*pbc), s); break;
			case FE_PRESCRIBED_CONCENTRATION : WriteBCPrescribedConcentration(dynamic_cast<FEPrescribedConcentration&>(*pbc), s); break;
            case FE_PRESCRIBED_FLUID_VELOCITY: WriteBCPrescribedFluidVelocity(dynamic_cast<FEPrescribedFluidVelocity&>(*pbc), s); break;
            case FE_PRESCRIBED_DILATATION    : WriteBCPrescribedFluidDilatation (dynamic_cast<FEPrescribedFluidDilatation&>(*pbc), s); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Export prescribed displacements
//
void FEBioExport2::WriteBCPrescribedDisplacement(FEPrescribedDisplacement& rbc, FEStep& s)
{
	int l;
	int lc;
	char bc[3][2] = {"x", "y", "z"};
	double val;

	FELoadCurve* plc = rbc.GetLoadCurve();
	l = rbc.GetDOF();
	lc = plc->GetID();
	val = rbc.GetScaleFactor();

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");
	e.add_attribute("bc", bc[l]);
	e.add_attribute("lc", lc);

	FEItemListBuilder* pitem = rbc.GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(&rbc);

	if (HasNodeSet(pitem))
	{
		e.add_attribute("set", pitem->GetName().c_str());
		e.value(val);
		m_xml.add_leaf(e);
	}
	else
	{
		m_xml.add_branch(e);
		{
			XMLElement el;
			el.name("node");
			el.value(1.0);
			int n1 = el.add_attribute("id", 0);
			
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			int N = pg->Size();
			for (int k=0; k<N; ++k, ++it) 
			{
				FENode* pn = it->m_pi;
				el.value(val);
				el.set_attribute(n1, pn->m_nid);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch(); // prescribe
	}
}


//-----------------------------------------------------------------------------
// Export prescribed rotations
//
void FEBioExport2::WriteBCPrescribedRotation(FEPrescribedRotation& rbc, FEStep& s)
{
	int l;
	int lc;
	char bc[3][2] = {"u", "v", "w"};
	double val;

	FELoadCurve* plc = rbc.GetLoadCurve();
	l = rbc.GetDOF();
	lc = plc->GetID();
	val = rbc.GetScaleFactor();

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");
	e.add_attribute("bc", bc[l]);
	e.add_attribute("lc", lc);

	FEItemListBuilder* pitem = rbc.GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(&rbc);

	if (HasNodeSet(pitem))
	{
		e.add_attribute("set", pitem->GetName().c_str());
		e.value(val);
		m_xml.add_leaf(e);
	}
	else
	{
		m_xml.add_branch(e);
		{
			XMLElement el;
			el.name("node");
			el.value(1.0);
			int n1 = el.add_attribute("id", 0);
			
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			int N = pg->Size();
			for (int k=0; k<N; ++k, ++it) 
			{
				FENode* pn = it->m_pi;
				el.value(val);
				el.set_attribute(n1, pn->m_nid);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch(); // prescribe
	}
}

//-----------------------------------------------------------------------------
// Export prescribed fluid pressures
//
void FEBioExport2::WriteBCPrescribedFluidPressure(FEPrescribedFluidPressure& rbc, FEStep& s)
{
	int k;
	int lc;
	bool bn;
	double val;

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	FELoadCurve* plc = rbc.GetLoadCurve();
	lc = plc->GetID();
	bn = true; // plc->IsActive();
	val = rbc.GetScaleFactor();

	e.add_attribute("bc", "p");
	e.add_attribute("lc", lc);

	FEItemListBuilder* pitem = rbc.GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(&rbc);

	if (HasNodeSet(pitem))
	{
		e.add_attribute("set", pitem->GetName().c_str());
		e.value(val);
		m_xml.add_leaf(e);
	}
	else
	{
		m_xml.add_branch(e);
		{
			XMLElement el;
			el.name("node");
			el.value(1.0);
			int n1 = el.add_attribute("id", 0);
			
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k=0; k<N; ++k, ++it) 
			{
				pn = it->m_pi;
				el.value(val);
				el.set_attribute(n1, pn->m_nid);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch(); // prescribe
	}
}

//-----------------------------------------------------------------------------
// Export prescribed temperatures
//
void FEBioExport2::WriteBCPrescribedTemperature(FEPrescribedTemperature& rbc, FEStep& s)
{
	int k;
	int lc;
	double val;

	FELoadCurve* plc = rbc.GetLoadCurve();
	lc = plc->GetID();
	val = rbc.GetScaleFactor();

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");
	e.add_attribute("bc", "T");
	e.add_attribute("lc", lc);

	FEItemListBuilder* pitem = rbc.GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(&rbc);

	if (HasNodeSet(pitem))
	{
		e.add_attribute("set", pitem->GetName().c_str());
		e.value(val);
		m_xml.add_leaf(e);
	}
	else
	{
		m_xml.add_branch(e);
		{
			XMLElement el;
			el.name("node");
			el.value(1.0);
			int n1 = el.add_attribute("id", 0);


			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k=0; k<N; ++k, ++it) 
			{
				pn = it->m_pi;
				el.value(val);
				el.set_attribute(n1, pn->m_nid);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch(); // prescribe
	}
}

//-----------------------------------------------------------------------------
// Export prescribed concentration
//
void FEBioExport2::WriteBCPrescribedConcentration(FEPrescribedConcentration& rbc, FEStep& s)
{
	int k;
	int lc;
	bool bn;
	double val;
	char szbc[6][3] = {"c1", "c2", "c3", "c4", "c5", "c6"};

	int l = rbc.GetDOF();
	FELoadCurve* plc = rbc.GetLoadCurve();
	lc = plc->GetID();
	bn = true; // plc->IsActive();
	val = rbc.GetScaleFactor();

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	e.add_attribute("bc", szbc[l]);
	e.add_attribute("lc", lc);

	FEItemListBuilder* pitem = rbc.GetItemList();
	if (pitem == 0) throw InvalidItemListBuilder(&rbc);

	if (HasNodeSet(pitem))
	{
		e.add_attribute("set", pitem->GetName().c_str());
		e.value(val);
		m_xml.add_leaf(e);
	}
	else
	{
		m_xml.add_branch(e);
		{
			XMLElement el;
			el.name("node");
			el.value(1.0);
			int n1 = el.add_attribute("id", 0);
			
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k=0; k<N; ++k, ++it) 
			{
				pn = it->m_pi;
				el.value(val);
				el.set_attribute(n1, pn->m_nid);
				m_xml.add_leaf(el, false);
			}
		}
		m_xml.close_branch(); // prescribe
	}
}

//-----------------------------------------------------------------------------
// Export prescribed velocity
//
void FEBioExport2::WriteBCPrescribedFluidVelocity(FEPrescribedFluidVelocity& rbc, FEStep& s)
{
    int l;
    int lc;
    char bc[3][3] = {"wx", "wy", "wz"};
    double val;
    
    FELoadCurve* plc = rbc.GetLoadCurve();
	l = rbc.GetDOF();
    lc = plc->GetID();
    val = rbc.GetScaleFactor();
    
    XMLElement e;
    e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");
    e.add_attribute("bc", bc[l]);
    e.add_attribute("lc", lc);
    
    FEItemListBuilder* pitem = rbc.GetItemList();
    if (pitem == 0) throw InvalidItemListBuilder(&rbc);
    
    if (HasNodeSet(pitem))
    {
		e.add_attribute("set", pitem->GetName().c_str());
        e.value(val);
        m_xml.add_leaf(e);
    }
    else
    {
        m_xml.add_branch(e);
        {
            XMLElement el;
            el.name("node");
            el.value(1.0);
            int n1 = el.add_attribute("id", 0);
            
            auto_ptr<FENodeList> pg(pitem->BuildNodeList());
            FENodeList::Iterator it = pg->First();
            int N = pg->Size();
            for (int k=0; k<N; ++k, ++it)
            {
                FENode* pn = it->m_pi;
                el.value(val);
                el.set_attribute(n1, pn->m_nid);
                m_xml.add_leaf(el, false);
            }
        }
        m_xml.close_branch(); // prescribe
    }
}

//-----------------------------------------------------------------------------
// Export prescribed fluid dilatations
//
void FEBioExport2::WriteBCPrescribedFluidDilatation(FEPrescribedFluidDilatation& rbc, FEStep& s)
{
    int k;
    int lc;
    bool bn;
    double val;
    
    XMLElement e;
    e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");
    
    FELoadCurve* plc = rbc.GetLoadCurve();
    lc = plc->GetID();
    bn = true; // plc->IsActive();
    val = rbc.GetScaleFactor();
    
    e.add_attribute("bc", "ef");
    e.add_attribute("lc", lc);
    
    FEItemListBuilder* pitem = rbc.GetItemList();
    if (pitem == 0) throw InvalidItemListBuilder(&rbc);
    
    if (HasNodeSet(pitem))
    {
		e.add_attribute("set", pitem->GetName().c_str());
        e.value(val);
        m_xml.add_leaf(e);
    }
    else
    {
        m_xml.add_branch(e);
        {
            XMLElement el;
            el.name("node");
            el.value(1.0);
            int n1 = el.add_attribute("id", 0);
            
            auto_ptr<FENodeList> pg(pitem->BuildNodeList());
            FENodeList::Iterator it = pg->First();
            FENode* pn;
            int N = pg->Size();
            for (k=0; k<N; ++k, ++it)
            {
                pn = it->m_pi;
                el.value(val);
                el.set_attribute(n1, pn->m_nid);
                m_xml.add_leaf(el, false);
            }
        }
        m_xml.close_branch(); // prescribe
    }
}

//-----------------------------------------------------------------------------
// export nodal loads
//
void FEBioExport2::WriteLoadNodal(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FENodalLoad* pbc = dynamic_cast<FENodalLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{

			char bc[][3] = {"x", "y", "z", "p", "c1", "c2", "c3", "c4", "c5", "c6"};

			int l = pbc->GetDOF();
			FELoadCurve* plc = pbc->GetLoadCurve();
			int lc = (plc ? plc->GetID() : 0);

			XMLElement load("nodal_load");
			load.add_attribute("bc", bc[l]);
			load.add_attribute("lc", lc);

			m_xml.add_branch(load);
			{
				XMLElement el;
				el.name("node");
				el.value(1.0);
				int n1 = el.add_attribute("id", 0);
				el.value(pbc->GetLoad());

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					el.set_attribute(n1, pn->m_nid);
					m_xml.add_leaf(el, false);
				}
			}
			m_xml.close_branch(); // nodal_load
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure loads
//
void FEBioExport2::WriteLoadPressure(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FEPressureLoad* pbc = dynamic_cast<FEPressureLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement load;
			load.name("surface_load");
			load.add_attribute("type", "pressure");
			m_xml.add_branch(load);
			{
				// get the load curve ID
				int lc = pbc->GetLoadCurve()->GetID();

				// write the pressure parameter
				XMLElement press("pressure");
				press.add_attribute("lc", lc);
				press.value(pbc->GetLoad());
				m_xml.add_leaf(press);

				// write the linear parameter
				m_xml.add_leaf("linear", pbc->GetLinearFlag());

				// create the surface list
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export fluid flux
//
void FEBioExport2::WriteFluidFlux(FEStep& s)
{

	for (int j=0; j<s.Loads(); ++j)
	{
		FEFluidFlux* pbc = dynamic_cast<FEFluidFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "fluidflux");
			m_xml.add_branch(flux);
			{
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				// get the loadcurve ID
				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();

				XMLElement load("flux");
				load.add_attribute("lc", lc);
				load.value(pbc->GetLoad());
				m_xml.add_leaf(load);

				m_xml.add_leaf("linear", pbc->GetLinearFlag());
				m_xml.add_leaf("mixture", pbc->GetMixtureFlag());

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}


//----------------------------------------------------------------------------
// Export mixture normal traction
//
void FEBioExport2::WriteBPNormalTraction(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FEBPNormalTraction* pbc = dynamic_cast<FEBPNormalTraction*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			FEFaceList* pfl = pitem->BuildFaceList();
			if (pfl == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "normal_traction");
			m_xml.add_branch(flux);
			{
				// get load curve ID
				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();

				XMLElement load("traction");
				load.add_attribute("lc", lc);
				load.value(pbc->GetLoad());
				m_xml.add_leaf(load);

				m_xml.add_leaf("linear", pbc->GetLinearFlag());
				m_xml.add_leaf("effective", pbc->GetMixtureFlag());

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // normal_traction
		}
	}
}

//----------------------------------------------------------------------------
// Export heat flux
//
void FEBioExport2::WriteHeatFlux(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FEHeatFlux* pbc = dynamic_cast<FEHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "heatflux");
			m_xml.add_branch(flux);
			{
				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();

				XMLElement load("flux");
				load.add_attribute("lc", lc);
				load.value(pbc->GetLoad());
				m_xml.add_leaf(load);

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export convective heat flux
//
void FEBioExport2::WriteConvectiveHeatFlux(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FEConvectiveHeatFlux* pbc = dynamic_cast<FEConvectiveHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "convective_heatflux");
			m_xml.add_branch(flux);
			{
				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();

				XMLElement temp("Ta");
				temp.add_attribute("lc", lc);
				temp.value(pbc->GetTemperature());
				m_xml.add_leaf(temp);

				m_xml.add_leaf("hc", pbc->GetCoefficient());

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export solute flux
//
void FEBioExport2::WriteSoluteFlux(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FESoluteFlux* pbc = dynamic_cast<FESoluteFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			// get the load curve id
			FELoadCurve* plc = pbc->GetLoadCurve();
			int lc = plc->GetID();

			// get the item list builder
			FEItemListBuilder* pitem = pbc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pbc);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "soluteflux");
			m_xml.add_branch(flux);
			{
				XMLElement load("flux");
				load.add_attribute("lc", lc);
				load.value(pbc->GetLoad());
				m_xml.add_leaf(load);

				m_xml.add_leaf("linear", pbc->GetLinearFlag());
				m_xml.add_leaf("solute_id", pbc->GetBC() + 1);

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // soluteflux
		}
	}
}

//----------------------------------------------------------------------------
void FEBioExport2::WriteConcentrationFlux(FEStep& s)
{
	for (int j = 0; j<s.Loads(); ++j)
	{
		FEConcentrationFlux* pcf = dynamic_cast<FEConcentrationFlux*>(s.Load(j));
		if (pcf && pcf->IsActive())
		{
			// get the load curve id
			FELoadCurve* plc = pcf->GetLoadCurve();
			int lc = plc->GetID();

			// get the item list builder
			FEItemListBuilder* pitem = pcf->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pcf);

			XMLElement flux;
			flux.name("surface_load");
			flux.add_attribute("type", "concentration flux");
			m_xml.add_branch(flux);
			{
				m_xml.add_leaf("solute_id", pcf->GetSoluteID() + 1);

				XMLElement load("flux");
				load.add_attribute("lc", lc);
				load.value(pcf->GetFlux());
				m_xml.add_leaf(load);

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // soluteflux
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure tractions
//
void FEBioExport2::WriteLoadTraction(FEStep& s)
{
	for (int j=0; j<s.Loads(); ++j)
	{
		FESurfaceTraction* ptc = dynamic_cast<FESurfaceTraction*>(s.Load(j));
		if (ptc && ptc->IsActive())
		{
			FEItemListBuilder* pitem = ptc->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(ptc);

			XMLElement flux("surface_load");
			flux.add_attribute("type", "traction");
			m_xml.add_branch(flux);
			{
				FELoadCurve* plc = ptc->GetLoadCurve();
				int lc = plc->GetID();

				XMLElement scl("scale");
				scl.add_attribute("lc", lc);
				scl.value(1.0);
				m_xml.add_leaf(scl);

				m_xml.add_leaf("traction", ptc->GetTraction());

				// Write surface element
				XMLElement el("surface");
				WriteSurface(el, pitem);
			}
			m_xml.close_branch(); // surface_load
		}
	}
}

//----------------------------------------------------------------------------
// Export fluid tractions
//
void FEBioExport2::WriteFluidTraction(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidTraction* ptc = dynamic_cast<FEFluidTraction*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid viscous traction");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement scl("scale");
                scl.add_attribute("lc", lc);
                scl.value(ptc->GetScale());
                m_xml.add_leaf(scl);
                
                m_xml.add_leaf("traction", ptc->GetTraction());
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid velocities
//
void FEBioExport2::WriteFluidVelocity(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidVelocity* ptc = dynamic_cast<FEFluidVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid velocity");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement scl("scale");
                scl.add_attribute("lc", lc);
                scl.value(1.0);
                m_xml.add_leaf(scl);
                
                m_xml.add_leaf("velocity", ptc->GetLoad());
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid normal velocities
//
void FEBioExport2::WriteFluidNormalVelocity(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidNormalVelocity* ptc = dynamic_cast<FEFluidNormalVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid normal velocity");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement load("velocity");
                load.add_attribute("lc", lc);
                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement bp("prescribe_nodal_velocities");
                bp.value(ptc->GetBP());
                m_xml.add_leaf(bp);
                
                XMLElement bparab("parabolic");
                bparab.value(ptc->GetBParab());
                m_xml.add_leaf(bparab);
                
                XMLElement brimp("prescribe_rim_pressure");
                brimp.value(ptc->GetBRimP());
                m_xml.add_leaf(brimp);

                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid rotational velocities
//
void FEBioExport2::WriteFluidRotationalVelocity(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidRotationalVelocity* ptc = dynamic_cast<FEFluidRotationalVelocity*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid rotational velocity");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement load("angular_speed");
                load.add_attribute("lc", lc);
                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                XMLElement axis("axis");
                axis.value(ptc->GetAxis());
                m_xml.add_leaf(axis);
                
                XMLElement origin("origin");
                origin.value(ptc->GetOrigin());
                m_xml.add_leaf(origin);
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid flow resistance
//
void FEBioExport2::WriteFluidFlowResistance(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidFlowResistance* ptc = dynamic_cast<FEFluidFlowResistance*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid resistance");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement load("R");
                load.add_attribute("lc", lc);
                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                // Write pressure offset element
                FELoadCurve* polc = ptc->GetPOLoadCurve();
                int lcpo = polc->GetID();
                
                XMLElement po("pressure_offset");
                po.add_attribute("lc", lcpo);
                po.value(ptc->GetPO());
                m_xml.add_leaf(po);
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid backflow stabilization
//
void FEBioExport2::WriteFluidBackflowStabilization(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidBackflowStabilization* ptc = dynamic_cast<FEFluidBackflowStabilization*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid backflow stabilization");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement load("beta");
                load.add_attribute("lc", lc);
                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid tangential stabilization
//
void FEBioExport2::WriteFluidTangentialStabilization(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFluidTangentialStabilization* ptc = dynamic_cast<FEFluidTangentialStabilization*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid tangential stabilization");
            m_xml.add_branch(flux);
            {
                FELoadCurve* plc = ptc->GetLoadCurve();
                int lc = plc->GetID();
                
                XMLElement load("beta");
                load.add_attribute("lc", lc);
                load.value(ptc->GetLoad());
                m_xml.add_leaf(load);
                
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//----------------------------------------------------------------------------
// Export fluid normal velocities
//
void FEBioExport2::WriteFSITraction(FEStep& s)
{
    for (int j=0; j<s.Loads(); ++j)
    {
        FEFSITraction* ptc = dynamic_cast<FEFSITraction*>(s.Load(j));
        if (ptc && ptc->IsActive())
        {
            FEItemListBuilder* pitem = ptc->GetItemList();
            if (pitem == 0) throw InvalidItemListBuilder(ptc);
            
            XMLElement flux("surface_load");
            flux.add_attribute("type", "fluid-FSI traction");
            m_xml.add_branch(flux);
            {
                // Write surface element
                XMLElement el("surface");
                WriteSurface(el, pitem);
            }
            m_xml.close_branch(); // surface_load
        }
    }
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport2::WriteInitialSection()
{
	FEModel& fem = m_prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	vector<int> VC; VC.resize(m_nodes);

	// initial velocities
	for (int j=0; j<s.ICs(); ++j)
	{
		FENodalVelocities* pbc = dynamic_cast<FENodalVelocities*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			vec3d v = pbc->GetVelocity();
			m_xml.add_branch("velocity");
			{
				XMLElement el;

				for (int k=0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid-1] = 1;
				}

				for (int k=0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k+1);
						el.value(v);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial concentration
	for (int j=0; j<s.BCs(); ++j)
	{
		FEInitConcentration* pbc = dynamic_cast<FEInitConcentration*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double c = pbc->GetValue();
			int bc = pbc->GetBC();

			XMLElement ec;
			ec.name("concentration");
			ec.add_attribute("sol", bc+1);
			m_xml.add_branch(ec);
			{
				XMLElement el;

				for (int k=0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid-1] = 1;
				}

				for (int k=0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k+1);
						el.value(c);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial fluid pressure
	for (int j=0; j<s.BCs(); ++j)
	{
		FEInitFluidPressure* pbc = dynamic_cast<FEInitFluidPressure*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double p = pbc->GetValue();
			m_xml.add_branch("fluid_pressure");
			{
				XMLElement el;

				for (int k=0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid-1] = 1;
				}

				for (int k=0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k+1);
						el.value(p);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial temperature
	for (int j=0; j<s.BCs(); ++j)
	{
		FEInitTemperature* pbc = dynamic_cast<FEInitTemperature*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double T = pbc->GetValue();
			m_xml.add_branch("temperature");
			{
				XMLElement el;

				for (int k=0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k=0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid-1] = 1;
				}

				for (int k=0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k+1);
						el.value(T);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteBodyForces(FEStep &s)
{
	for (int i=0; i<s.Loads(); ++i)
	{
		FEConstBodyForce* pbl = dynamic_cast<FEConstBodyForce*>(s.Load(i));
		if (pbl && pbl->IsActive())
		{
			XMLElement el("body_load");
			el.add_attribute("type", "const");
			m_xml.add_branch(el);
			{
				char sz[3][2] = {"x", "y", "z"};
				XMLElement el;
				for (int i=0; i<3; ++i) 
				{
					el.name(sz[i]);
					if (pbl->GetLoadCurve(i))el.add_attribute("lc", pbl->GetLoadCurve(i)->GetID());
					el.value(pbl->GetLoad(i));
					m_xml.add_leaf(el);
				}
			}
			m_xml.close_branch();
		}
	}

}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteHeatSources(FEStep& s)
{
	for (int i=0; i<s.Loads(); ++i)
	{
		FEHeatSource* phs = dynamic_cast<FEHeatSource*>(s.Load(i));
		if (phs && phs->IsActive())
		{
			XMLElement el("body_load");
			el.add_attribute("type", "heat_source");
			m_xml.add_branch(el);
			{
				XMLElement el;
				el.name("Q");
				el.add_attribute("lc", phs->GetLoadCurve()->GetID());
				el.value(phs->GetLoad());
				m_xml.add_leaf(el);
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteGlobalsSection()
{
	XMLElement el;
	FEModel& fem = *m_pfem;

	if (fem.Parameters())
	{
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i=0; i<N; ++i)
			{
				Param& p = fem.GetParam(i);
				m_xml.add_leaf(p.GetShortName(), p.GetFloatValue());
			}
		}
		m_xml.close_branch();

		if (fem.Solutes()>0)
		{
			m_xml.add_branch("Solutes");
			{
				int NS = fem.Solutes();
				for (int i=0; i<NS; ++i)
				{
					FESoluteData& s = fem.GetSoluteData(i);
					XMLElement el;
					el.name("solute");
					el.add_attribute("id", i+1);
					el.add_attribute("name", s.GetName().c_str());
					m_xml.add_branch(el);
					{
						m_xml.add_leaf("charge_number", s.GetChargeNumber());
						m_xml.add_leaf("molar_mass", s.GetMolarMass());
						m_xml.add_leaf("density", s.GetDensity());
					}
					m_xml.close_branch();
				}
			}
			m_xml.close_branch();
		}
        
		if (fem.SBMs()>0)
		{
			m_xml.add_branch("SolidBoundMolecules");
			{
				int NS = fem.SBMs();
				for (int i=0; i<NS; ++i)
				{
					FESoluteData& s = fem.GetSBMData(i);
					XMLElement el;
					el.name("solid_bound");
					el.add_attribute("id", i+1);
					el.add_attribute("name", s.GetName().c_str());
					m_xml.add_branch(el);
					{
						m_xml.add_leaf("charge_number", s.GetChargeNumber());
						m_xml.add_leaf("molar_mass", s.GetMolarMass());
						m_xml.add_leaf("density", s.GetDensity());
					}
					m_xml.close_branch();
				}
			}
			m_xml.close_branch();
		}
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteLoadDataSection()
{
	for (int i=0; i<(int) m_pLC.size(); ++i)
	{
		FELoadCurve* plc = m_pLC[i];

		XMLElement el;
		el.name("loadcurve");
		el.add_attribute("id", i+1);

		switch (plc->GetType())
		{
		case FELoadCurve::LC_STEP  : el.add_attribute("type", "step"  ); break;
		case FELoadCurve::LC_LINEAR: el.add_attribute("type", "linear"); break;
		case FELoadCurve::LC_SMOOTH: el.add_attribute("type", "smooth"); break;
		}

		switch (plc->GetExtend())
		{
//		case FELoadCurve::EXT_CONSTANT     : el.add_attribute("extend", "constant"     ); break;
		case FELoadCurve::EXT_EXTRAPOLATE  : el.add_attribute("extend", "extrapolate"  ); break;
		case FELoadCurve::EXT_REPEAT       : el.add_attribute("extend", "repeat"       ); break;
		case FELoadCurve::EXT_REPEAT_OFFSET: el.add_attribute("extend", "repeat offset"); break;
		}

		double d[2];
		m_xml.add_branch(el);
		{
			for (int j=0; j<plc->Size(); ++j)
			{
				LOADPOINT& pt = plc->Item(j);
				d[0] = pt.time;
				d[1] = pt.load;
				m_xml.add_leaf("point", d, 2);
			}
		}
		m_xml.close_branch(); // loadcurve
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteSurfaceSection(FEFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[9];

	int NF = s.Size();
	FEFaceList::Iterator pf = s.First();

/*	
	FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(m_pfem->GetStep(1));
	assert(pstep);
	STEP_SETTINGS& ops = pstep->GetSettings();
	if (ops.beface)
	{
		for (int i=0; i<NF; ++i, ++n, ++pf)
		{
			FEFace& face = *(pf->m_pi);
			FECoreMesh* pm = pf->m_pm;
			FEElement& el = pm->Element(face.m_elem[0]);
			nn[0] = el.m_ntag;
			nn[1] = face.m_face+1;
	
			ef.name((face.m_nodes==3?"tri3":"quad4"));
			ef.add_attribute("id", n);
			ef.value(nn, 2);

			m_xml.add_leaf(ef);
		}
	}
	else
*/	{
		int nfn;
		for (int j=0; j<NF; ++j, ++n, ++pf)
		{
			FEFace& face = *(pf->m_pi);
			FECoreMesh* pm = pf->m_pm;
			nfn = face.Nodes();
			for (int k=0; k<nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
			switch(nfn)
			{
			case 3: ef.name("tri3" ); break;
			case 4: ef.name("quad4"); break;
			case 6: ef.name("tri6" ); break;
			case 7: ef.name("tri7" ); break;
			case 8: ef.name("quad8"); break;
			case 9: ef.name("quad9"); break;
			default:
				assert(false);
			}
			ef.add_attribute("id", n);
			ef.value(nn, nfn);
			m_xml.add_leaf(ef);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport2::WriteSurface(XMLElement& el, FEItemListBuilder* pl)
{
	if (HasSurface(pl))
	{
		el.add_attribute("set", pl->GetName().c_str());
		m_xml.add_empty(el);
	}
	else
	{
		const string& name = pl->GetName();
		if (name.empty() == false) el.add_attribute("name", name.c_str());
		auto_ptr<FEFaceList> ps(pl->BuildFaceList());
		if (ps.get() == 0) throw InvalidItemListBuilder(pl);
		m_xml.add_branch(el);
		{
			WriteSurfaceSection(*ps);
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteOutputSection()
{
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	int N = plt.PlotVariables();
	if (N > 0)
	{
		XMLElement p;
		p.name("plotfile");
		p.add_attribute("type", "febio");

		// count the nr of active plot variables
		int na = 0;
		for (int i=0; i<N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i=0; i<N; ++i) 
				{
					FEPlotVariable& v = plt.PlotVariable(i);
					if (v.isShown() && v.isActive())
					{
						if (v.Domains() == 0)
						{
							XMLElement e;
							e.name("var");
							e.add_attribute("type", v.name());
							m_xml.add_empty(e);
						}
						else
						{
							if (v.domainType() == DOMAIN_SURFACE)
							{
								for (int n = 0; n<v.Domains(); ++n)
								{
									FEItemListBuilder* pl = v.GetDomain(n);
									if (pl)
									{
										XMLElement e;
										e.name("var");
										e.add_attribute("type", v.name());
										e.add_attribute("surface", pl->GetName());
										m_xml.add_empty(e);
									}
								}
							}
							else
							{
								assert(false);
							}
						}
					}
				}

				if (m_compress) m_xml.add_leaf("compression", 1);
			}
			m_xml.close_branch();	
		}
		else m_xml.add_empty(p);
	}

	FEModel& fem = m_prj.GetFEModel();
	GModel& mdl = fem.GetModel();
	CLogDataSettings& log = m_prj.GetLogDataSettings();
	N = log.LogDataSize();
	if (N > 0)
	{
		m_xml.add_branch("logfile");
		{
			for (int i=0; i<N; ++i)
			{
				FELogData& d = log.LogData(i);
				switch (d.type)
				{
				case FELogData::LD_NODE:
					{
						XMLElement e;
						e.name("node_data");
						e.add_attribute("data", d.sdata);

						GGroup* pg = dynamic_cast<GGroup*>(mdl.FindNamedSelection(d.groupID));
						if (pg)
						{
							vector<int> L;
							FENodeList* pl = pg->BuildNodeList();
							FENodeList::Iterator pi = pl->First();
							int M = pl->Size();
							for (int i=0; i<M; ++i, ++pi) L.push_back(pi->m_pi->m_nid);
							m_xml.add_leaf(e, L);
						}
						else m_xml.add_empty(e);
					}
					break;
				case FELogData::LD_ELEM:
					{
						XMLElement e;
						e.name("element_data");
						e.add_attribute("data", d.sdata);

						GGroup* pg = dynamic_cast<GGroup*>(mdl.FindNamedSelection(d.groupID));
						if (pg)
						{
							vector<int> L;
							FEElemList* pl = pg->BuildElemList();
							FEElemList::Iterator pi = pl->First();
							int M = pl->Size();
							for (int i=0; i<M; ++i, ++pi) L.push_back(pi->m_pi->m_ntag);
							m_xml.add_leaf(e, L);
						}
						else m_xml.add_empty(e);
					}
					break;
				case FELogData::LD_RIGID:
					{
						XMLElement e;
						e.name("rigid_body_data");
						e.add_attribute("data", d.sdata);

						GMaterial* pm = fem.GetMaterialFromID(d.matID);
						if (pm)
						{
							e.value(pm->m_ntag);
							m_xml.add_leaf(e);
						}
						else m_xml.add_empty(e);
					}
					break;
				}
			}
		}
		m_xml.close_branch();
	}
}

//-----------------------------------------------------------------------------

void FEBioExport2::WriteStepSection()
{
	// we've already written the initial step
	// so now we simply output all the analysis steps
	for (int i=1; i<m_pfem->Steps(); ++i)
	{
		FEAnalysisStep& s = dynamic_cast<FEAnalysisStep&>(*m_pfem->GetStep(i));

		XMLElement e;
		e.name("Step");

		const string& name = s.GetName();
		if (name.empty() == false) e.add_attribute("name", name.c_str());

		m_xml.add_branch(e);
		{
			// module
			WriteModuleSection(&s);

			// output control section
			m_xml.add_branch("Control");
			{
				WriteControlSection(&s);
			}
			m_xml.close_branch(); // Control

			// output boundary section
			int nbc = s.BCs() + s.Interfaces();
			if (nbc>0)
			{
				m_xml.add_branch("Boundary");
				{
					WriteBoundarySection(s);
				}
				m_xml.close_branch(); // Boundary
			}

			// output loads section
			int nlc = s.Loads();
			if (nlc>0)
			{
				m_xml.add_branch("Loads");
				{
					WriteLoadsSection(s);
				}
				m_xml.close_branch(); // Loads
			}

			// output contact section
			int nci = s.Interfaces();
			if (nci)
			{
				m_xml.add_branch("Contact");
				{
					WriteContactSection(s);
				}
				m_xml.close_branch();
			}

			// output constraint section
			if (s.RigidConstraints())
			{
				m_xml.add_branch("Constraints");
				{
					WriteConstraintSection(s);
				}
				m_xml.close_branch(); // Constraints
			}
		}
		m_xml.close_branch(); // Step
	}
}

void FEBioExport2::WriteConstraintSection(FEStep &s)
{
	const char* szbc[6] = { "x", "y", "z", "Rx", "Ry", "Rz" };

	for (int i = 0; i<s.RigidConstraints(); ++i)
	{
		FERigidConstraint* ps = s.RigidConstraint(i);

		GMaterial* pgm = m_pfem->GetMaterialFromID(ps->GetMaterialID());
		if (pgm == 0) throw MissingRigidBody(ps->GetName());
		FERigidMaterial* pm = dynamic_cast<FERigidMaterial*>(pgm->GetMaterialProperties());
		if (pm == 0) throw InvalidMaterialReference();

		if (ps->Type() == FE_RIGID_FIXED)
		{
			FERigidFixed* rc = dynamic_cast<FERigidFixed*>(ps);

			XMLElement el;
			el.name("rigid_body");
			el.add_attribute("mat", pgm->m_ntag);
			m_xml.add_branch(el);
			{
				for (int j = 0; j<6; ++j)
				if (rc->GetDOF(j))
				{
					XMLElement el("fixed");
					el.add_attribute("bc", szbc[j]);
					m_xml.add_empty(el);
				}
			}
			m_xml.close_branch();
		}
		else if (ps->Type() == FE_RIGID_DISPLACEMENT)
		{
			FERigidPrescribed* rc = dynamic_cast<FERigidPrescribed*>(ps);
			XMLElement el;
			el.name("rigid_body");
			el.add_attribute("mat", pgm->m_ntag);
			m_xml.add_branch(el);
			{
				XMLElement el("prescribed");
				el.add_attribute("bc", szbc[rc->GetDOF()]);
				el.add_attribute("lc", rc->GetLoadCurve()->GetID());
				el.value(rc->GetValue());
				m_xml.add_leaf(el);
			}
			m_xml.close_branch();
		}
		else if (ps->Type() == FE_RIGID_FORCE)
		{
			FERigidPrescribed* rc = dynamic_cast<FERigidPrescribed*>(ps);
			XMLElement el;
			el.name("rigid_body");
			el.add_attribute("mat", pgm->m_ntag);
			m_xml.add_branch(el);
			{
				XMLElement el("force");
				el.add_attribute("bc", szbc[rc->GetDOF()]);
				el.add_attribute("lc", rc->GetLoadCurve()->GetID());
				el.value(rc->GetValue());
				m_xml.add_leaf(el);
			}
			m_xml.close_branch();
		}
	}

	// some contact definitions are actually stored in the constraint section
	WriteVolumeConstraint(s);
	WriteSymmetryPlane(s);
    WriteNormalFlow(s);
    WriteConnectors(s);
}
