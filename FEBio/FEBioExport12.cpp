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

// FEBioExport12.cpp: implementation of the FEBioExport12 class.
//
//////////////////////////////////////////////////////////////////////

#include "FEBioExport12.h"
#include <FEMLib/FERigidConstraint.h>
#include <MeshTools/GModel.h>
#include <MeshTools/GDiscreteObject.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEInitialCondition.h>
#include <FEMLib/FESurfaceLoad.h>
#include <FEMLib/FEBodyLoad.h>
#include <MeshLib/FEMesh.h>
#include <MeshTools/GGroup.h>
#include <memory>
using namespace std;

//-----------------------------------------------------------------------------
FEBioExport12::FEBioExport12(FEProject& prj) : FEBioExport(prj)
{
	// initialize section flags
	for (int i = 0; i<MAX_SECTIONS; ++i) m_section[i] = true;
}

//-----------------------------------------------------------------------------
FEBioExport12::~FEBioExport12()
{
	Clear();
}

//-----------------------------------------------------------------------------
void FEBioExport12::Clear()
{
	FEBioExport::Clear();
	m_pSurf.clear(); // TODO: Do I need to delete items?
}

//----------------------------------------------------------------------------
//! See if the pl has been added to the named surface list
bool FEBioExport12::HasSurface(FEItemListBuilder* pl)
{
	int N = (int)m_pSurf.size();
	for (int i = 0; i<N; ++i)
	if (m_pSurf[i] == pl) return true;
	return false;
}

//-----------------------------------------------------------------------------
//! Prepare for export. Collect all loadcurves and named surfaces.
bool FEBioExport12::PrepareExport(FEProject& prj)
{
	if (FEBioExport::PrepareExport(prj) == false) return false;

	FEModel& fem = prj.GetFEModel();
	GModel& model = fem.GetModel();

	m_nodes = model.FENodes();
	m_nsteps = fem.Steps();

	// get the named surfaces (loads)
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Loads(); ++j)
		{
			FELoad* pl = pstep->Load(j);
			if (pl->IsActive())
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

	// get the named surfaces (paired interfaces)
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
		{
			FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(pstep->Interface(j));
			if (pi && pi->IsActive())
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

	return true;
}

//-----------------------------------------------------------------------------
//! Export the project to the FEBio 1.x format.
bool FEBioExport12::Write(const char* szfile)
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
	for (int i = 0; i<fem.Steps(); ++i)
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
		// Note: This exporter writes format 1.2 of the FEBio format specification
		el.name("febio_spec");
		el.add_attribute("version", "1.2");

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

			// output constraints section
			if ((m_nrc > 0) && (m_section[FEBIO_CONSTRAINTS]))
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
		const char* sz = "(unknown)";
		if (e.m_name.empty() == false) sz = e.m_name.c_str();
		return errf("Invalid reference to mesh item list when exporting:\n%s", sz);
	}
	catch (MissingRigidBody e)
	{
		return errf("No rigid body defined for rigid constraint %s", e.m_rbName.c_str());
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
void FEBioExport12::WriteModuleSection(FEAnalysisStep* pstep)
{
	XMLElement t;
	t.name("Module");
	switch (pstep->GetType())
	{
	case FE_STEP_MECHANICS: t.add_attribute("type", "solid"); break;
	case FE_STEP_HEAT_TRANSFER: t.add_attribute("type", "heat"); break;
	case FE_STEP_BIPHASIC: t.add_attribute("type", "biphasic"); break;
	case FE_STEP_BIPHASIC_SOLUTE: t.add_attribute("type", "solute"); break;
	case FE_STEP_MULTIPHASIC: t.add_attribute("type", "multiphasic"); break;
	};

	m_xml.add_empty(t);
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteControlSection(FEAnalysisStep* pstep)
{
	STEP_SETTINGS& ops = pstep->GetSettings();
	int ntype = pstep->GetType();
	switch (ntype)
	{
	case FE_STEP_MECHANICS: WriteSolidControlParams(pstep); break;
	case FE_STEP_HEAT_TRANSFER: WriteHeatTransferControlParams(pstep); break;
	case FE_STEP_BIPHASIC: WriteBiphasicControlParams(pstep); break;
	case FE_STEP_BIPHASIC_SOLUTE: WriteBiphasicSoluteControlParams(pstep); break;
	case FE_STEP_MULTIPHASIC: WriteBiphasicSoluteControlParams(pstep); break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteSolidControlParams(FEAnalysisStep* pstep)
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
	el.add_attribute("type", (ops.nanalysis == 0 ? "static" : "dynamic"));
	m_xml.add_empty(el);

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteHeatTransferControlParams(FEAnalysisStep* pstep)
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
	el.add_attribute("type", (ops.nanalysis == 0 ? "static" : "dynamic"));
	m_xml.add_empty(el);

	if (ops.bminbw)
	{
		m_xml.add_leaf("optimize_bw", 1);
	}
}


//-----------------------------------------------------------------------------
void FEBioExport12::WriteBiphasicControlParams(FEAnalysisStep* pstep)
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
		m_xml.add_leaf("symmetric_biphasic", (ops.nmatfmt == 1 ? 1 : 0));
	}
}


//-----------------------------------------------------------------------------
void FEBioExport12::WriteBiphasicSoluteControlParams(FEAnalysisStep* pstep)
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
		m_xml.add_leaf("symmetric_biphasic", (ops.nmatfmt == 1 ? 1 : 0));
	}
}

//-----------------------------------------------------------------------------

void FEBioExport12::WriteMaterialSection()
{
	XMLElement el;

	FEModel& s = *m_pfem;


	for (int i = 0; i<s.Materials(); ++i)
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
			case FE_NEO_HOOKEAN: WriteMaterial(pmat, el); break;
			case FE_ISOTROPIC_ELASTIC: WriteMaterial(pmat, el); break;
			case FE_ORTHO_ELASTIC: WriteMaterial(pmat, el); break;
			case FE_MOONEY_RIVLIN: WriteMaterial(pmat, el); break;
			case FE_INCOMP_NEO_HOOKEAN: WriteMaterial(pmat, el); break;
			case FE_VERONDA_WESTMANN: WriteMaterial(pmat, el); break;
			case FE_HOLMES_MOW: WriteMaterial(pmat, el); break;
			case FE_ARRUDA_BOYCE: WriteMaterial(pmat, el); break;
			case FE_CELL_GROWTH: WriteMaterial(pmat, el); break;
			case FE_CLE_CUBIC: WriteMaterial(pmat, el); break;
			case FE_CLE_ORTHOTROPIC: WriteMaterial(pmat, el); break;
			case FE_EFD_MOONEY_RIVLIN: WriteMaterial(pmat, el); break;
			case FE_EFD_DONNAN: WriteMaterial(pmat, el); break;
			case FE_EFD_NEO_HOOKEAN: WriteMaterial(pmat, el); break;
			case FE_OGDEN_MATERIAL: WriteMaterial(pmat, el); break;
			case FE_OGDEN_UNCONSTRAINED: WriteMaterial(pmat, el); break;
			case FE_PRLIG: WriteMaterial(pmat, el); break;
			case FE_FUNG_ORTHO: WriteMaterial(pmat, el); break;
			case FE_FUNG_ORTHO_COUPLED: WriteMaterial(pmat, el); break;
			case FE_LINEAR_ORTHO: WriteMaterial(pmat, el); break;
			case FE_ISOTROPIC_FOURIER: WriteMaterial(pmat, el); break;
			case FE_TRANS_MOONEY_RIVLIN_OLD: WriteMaterial(pmat, el); break;
			case FE_TRANS_ISO_MOONEY_RIVLIN: WriteMaterial(pmat, el); break;
			case FE_TRANS_VERONDA_WESTMANN_OLD: WriteMaterial(pmat, el); break;
			case FE_TRANS_ISO_VERONDA_WESTMANN: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_UNI: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_TISO: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_ISO: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_UNI_UC: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_TISO_UC: WriteMaterial(pmat, el); break;
			case FE_ACTIVE_CONTRACT_ISO_UC: WriteMaterial(pmat, el); break;
			case FE_USER_MATERIAL: WriteMaterial(pmat, el); break;
				//		case FE_PORO_ELASTIC       : WriteNestedMaterial(dynamic_cast<FENestedMaterial*>(pmat), el); break;
				//		case FE_PORO_HOLMES_MOW    : WriteNestedMaterial(dynamic_cast<FENestedMaterial*>(pmat), el); break;
			case FE_VISCO_ELASTIC: WriteMultiMaterial(pmat, el); break;
			case FE_UNCOUPLED_VISCO_ELASTIC: WriteMultiMaterial(pmat, el); break;
			case FE_RV_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_RV_MATERIAL_UC: WriteMultiMaterial(pmat, el); break;
			case FE_CFD_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_CFD_MATERIAL_UC: WriteMultiMaterial(pmat, el); break;
			case FE_DMG_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_DMG_MATERIAL_UC: WriteMultiMaterial(pmat, el); break;
			case FE_BIPHASIC_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_BIPHASIC_SOLUTE: WriteMultiMaterial(pmat, el); break;
			case FE_TRIPHASIC_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_MULTIPHASIC_MATERIAL: WriteMultiMaterial(pmat, el); break;
			case FE_SOLID_MIXTURE: WriteMultiMaterial(pmat, el); break;
			case FE_UNCOUPLED_SOLID_MIXTURE: WriteMultiMaterial(pmat, el); break;
			case FE_MUSCLE_MATERIAL:
			{
									   FEMuscleMaterial* pm = dynamic_cast<FEMuscleMaterial*> (pmat);
									   FEOldFiberMaterial& f = *pm->GetFiberMaterial();
									   el.add_attribute("type", "muscle material");
									   m_xml.add_branch(el);
									   {
										   m_xml.add_leaf("density", pm->GetFloatValue(FEMuscleMaterial::MP_DENSITY));
										   m_xml.add_leaf("g1", pm->GetFloatValue(FEMuscleMaterial::MP_G1));
										   m_xml.add_leaf("g2", pm->GetFloatValue(FEMuscleMaterial::MP_G2));
										   m_xml.add_leaf("k", pm->GetFloatValue(FEMuscleMaterial::MP_K));
										   m_xml.add_leaf("p1", f.GetFloatValue(FEMuscleMaterial::MP_P1));
										   m_xml.add_leaf("p2", f.GetFloatValue(FEMuscleMaterial::MP_P2));
										   m_xml.add_leaf("Lofl", f.GetFloatValue(FEMuscleMaterial::MP_LOFL));
										   m_xml.add_leaf("lam_max", f.GetFloatValue(FEMuscleMaterial::MP_LAM));
										   m_xml.add_leaf("smax", f.GetFloatValue(FEMuscleMaterial::MP_SMAX));

										   el.name("fiber");
										   if (f.m_naopt == FE_FIBER_LOCAL)
										   {
											   el.add_attribute("type", "local");
											   el.value(f.m_n, 2);
											   m_xml.add_leaf(el);
										   }
										   else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
										   {
											   el.add_attribute("type", "cylindrical");
											   m_xml.add_branch(el);
											   {
												   m_xml.add_leaf("center", f.m_r);
												   m_xml.add_leaf("axis", f.m_a);
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
												   m_xml.add_leaf("axis", f.m_a);
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

										   //					FELoadCurve& ac = f.GetParam(FEMuscleMaterial::Fiber::MP_AC).GetLoadCurve();
										   //					el.name("active_contraction");
										   //					el.add_attribute("lc", ac.m_nID);
										   //					el.value(1.0);
										   //					m_xml.add_leaf(el);
									   }
									   m_xml.close_branch();
			}
				break;
			case FE_TENDON_MATERIAL:
			{
									   FETendonMaterial* pm = dynamic_cast<FETendonMaterial*> (pmat);
									   FEOldFiberMaterial& f = *pm->GetFiberMaterial();
									   el.add_attribute("type", "tendon material");
									   m_xml.add_branch(el);
									   {
										   m_xml.add_leaf("density", pm->GetFloatValue(FETendonMaterial::MP_DENSITY));
										   m_xml.add_leaf("g1", pm->GetFloatValue(FETendonMaterial::MP_G1));
										   m_xml.add_leaf("g2", pm->GetFloatValue(FETendonMaterial::MP_G2));
										   m_xml.add_leaf("k", pm->GetFloatValue(FETendonMaterial::MP_K));
										   m_xml.add_leaf("l1", f.GetFloatValue(FETendonMaterial::MP_L1));
										   m_xml.add_leaf("l2", f.GetFloatValue(FETendonMaterial::MP_L2));
										   m_xml.add_leaf("lam_max", f.GetFloatValue(FETendonMaterial::MP_LAM));

										   el.name("fiber");
										   if (f.m_naopt == FE_FIBER_LOCAL)
										   {
											   el.add_attribute("type", "local");
											   el.value(f.m_n, 2);
											   m_xml.add_leaf(el);
										   }
										   else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
										   {
											   el.add_attribute("type", "cylindrical");
											   m_xml.add_branch(el);
											   {
												   m_xml.add_leaf("center", f.m_r);
												   m_xml.add_leaf("axis", f.m_a);
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
												   m_xml.add_leaf("axis", f.m_a);
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
									   }
									   m_xml.close_branch();
			}
				break;
			case FE_RIGID_MATERIAL:
			{
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
			};
				break;
			case FE_TCNL_ORTHO:
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
				break;
			default: assert(false);
			}
		}
		else
		{
			m_xml.add_empty(el);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteMaterial(FEMaterial *pm, XMLElement& el)
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
				el.value(pm->m_axes->m_n, 3);
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

//-----------------------------------------------------------------------------
//! Write a fiber material to file.
//! TODO: Can I use the fiber material's parameters?
void FEBioExport12::WriteFiberMaterial(FEOldFiberMaterial& f)
{
	m_xml.add_leaf("c3", f.GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C3));
	m_xml.add_leaf("c4", f.GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C4));
	m_xml.add_leaf("c5", f.GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_C5));
	m_xml.add_leaf("lam_max", f.GetFloatValue(FETransMooneyRivlinOld::Fiber::MP_LAM));

	XMLElement el;
	el.name("fiber");
	if (f.m_naopt == FE_FIBER_LOCAL)
	{
		el.add_attribute("type", "local");
		el.value(f.m_n, 2);
		m_xml.add_leaf(el);
	}
	else if (f.m_naopt == FE_FIBER_CYLINDRICAL)
	{
		el.add_attribute("type", "cylindrical");
		m_xml.add_branch(el);
		{
			m_xml.add_leaf("center", f.m_r);
			m_xml.add_leaf("axis", f.m_a);
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
			m_xml.add_leaf("axis", f.m_a);
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
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteMaterialParams(FEMaterial* pm)
{
	// Write the parameters first
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
void FEBioExport12::WriteMultiMaterial(FEMaterial* pm, XMLElement& el)
{
	const char* sztype = 0;

	switch (pm->Type())
	{
	case FE_VISCO_ELASTIC: sztype = "viscoelastic"; break;
	case FE_UNCOUPLED_VISCO_ELASTIC: sztype = "uncoupled viscoelastic"; break;
	case FE_RV_MATERIAL: sztype = "reactive viscoelastic"; break;
	case FE_RV_MATERIAL_UC: sztype = "uncoupled reactive viscoelastic"; break;
	case FE_BIPHASIC_MATERIAL: sztype = "biphasic"; break;
	case FE_BIPHASIC_SOLUTE: sztype = "biphasic-solute"; break;
	case FE_TRIPHASIC_MATERIAL: sztype = "triphasic"; break;
	case FE_MULTIPHASIC_MATERIAL: sztype = "multiphasic"; break;
	case FE_SOLID_MIXTURE: sztype = "solid mixture"; break;
	case FE_UNCOUPLED_SOLID_MIXTURE: sztype = "uncoupled solid mixture"; break;
	case FE_CFD_MATERIAL: sztype = "continuous fiber distribution"; break;
	case FE_CFD_MATERIAL_UC: sztype = "continuous fiber distribution uncoupled"; break;
	case FE_DMG_MATERIAL: sztype = "elastic damage"; break;
	case FE_DMG_MATERIAL_UC: sztype = "uncoupled elastic damage"; break;
	case FE_SOLUTE_MATERIAL: sztype = "solute"; break;
	case FE_SBM_MATERIAL: sztype = "solid_bound"; break;
	case FE_REACTANT_MATERIAL: sztype = "vR"; break;
	case FE_PRODUCT_MATERIAL: sztype = "vP"; break;
	case FE_MASS_ACTION_FORWARD: sztype = "mass-action-forward"; break;
	case FE_MASS_ACTION_REVERSIBLE: sztype = "mass-action-reversible"; break;
	case FE_MICHAELIS_MENTEN: sztype = "Michaelis-Menten"; break;
	default:
		assert(false);
	}

	// set the type attribute
	if (pm->Type() == FE_SOLUTE_MATERIAL)
	{
		FESoluteMaterial* psm = dynamic_cast<FESoluteMaterial*>(pm); assert(psm);
		el.add_attribute("sol", psm->GetSoluteIndex() + 1);
	}
	else if (pm->Type() == FE_SBM_MATERIAL)
	{
		FESBMMaterial* psb = dynamic_cast<FESBMMaterial*>(pm); assert(psb);
		el.add_attribute("sbm", psb->GetSBMIndex() + 1);
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
				el.value(pm->m_axes->m_n, 3);
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
		for (int i = 0; i<NC; ++i)
		{
			FEMaterialProperty& mc = pm->GetProperty(i);
			for (int j = 0; j<mc.Size(); ++j)
			{
				FEMaterial* pc = mc.GetMaterial(j);
				if (pc)
				{
					el.name(mc.GetName().c_str());
					const string& name = pc->GetName();
					if (name.empty() == false) el.add_attribute("name", name.c_str());

					// TODO: some materials need to be treated as multi-materials
					//       although they technically aren't. I need to simplify this.
					bool is_multi = false;
					switch (pc->Type())
					{
					case FE_SBM_MATERIAL: is_multi = true; break;
					case FE_REACTANT_MATERIAL: is_multi = true; break;
					case FE_PRODUCT_MATERIAL: is_multi = true; break;
					}

					if ((pc->Properties() > 0) || is_multi) WriteMultiMaterial(pc, el);
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
void FEBioExport12::WriteGeometrySection()
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
	for (int i = 0; i<s.Materials(); ++i)
	{
		FETransverselyIsotropic* pmat = dynamic_cast<FETransverselyIsotropic*>(s.GetMaterial(i)->GetMaterialProperties());
		if (pmat && (pmat->GetFiberMaterial()->m_naopt == FE_FIBER_USER)) bdata = true;
	}
	for (int i = 0; i<model.Objects(); ++i)
	{
		FECoreMesh* pm = model.Object(i)->GetFEMesh();

		for (int j = 0; j<pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			if (e.m_Qactive) {
				bdata = true;
				break;
			}
		}
	}

	// write element data section if necessary
	if (bdata) WriteGeometryElementData();
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteGeometryNodes()
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
		for (int i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			FECoreMesh* pm = po->GetFEMesh();

			for (int j = 0; j<pm->Nodes(); ++j, ++n)
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
void FEBioExport12::WriteGeometryElements()
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	// elements
	m_xml.add_branch("Elements");
	{
		XMLElement hex8;
		XMLElement hex20;
		XMLElement hex27;
		XMLElement tet4;
		XMLElement pen6;
		XMLElement quad4;
		XMLElement tri3;
		XMLElement tet10;
		XMLElement tet15;
		XMLElement pyra5;
        XMLElement pyra13;

		hex8.name("hex8");
		int n1 = hex8.add_attribute("id", 0);
		int n2 = hex8.add_attribute("mat", 0);

		pyra5.name("pyra5");
		pyra5.add_attribute("id", 0);
		pyra5.add_attribute("mat", 0);

        pyra13.name("pyra13");
        pyra13.add_attribute("id", 0);
        pyra13.add_attribute("mat", 0);
        
		hex20.name("hex20");
		hex20.add_attribute("id", 0);
		hex20.add_attribute("mat", 0);

		hex27.name("hex27");
		hex27.add_attribute("id", 0);
		hex27.add_attribute("mat", 0);

		tet4.name("tet4");
		tet4.add_attribute("id", 0);
		tet4.add_attribute("mat", 0);

		tet10.name("tet10");
		tet10.add_attribute("id", 0);
		tet10.add_attribute("mat", 0);

		tet15.name("tet15");
		tet15.add_attribute("id", 0);
		tet15.add_attribute("mat", 0);

		pen6.name("penta6");
		pen6.add_attribute("id", 0);
		pen6.add_attribute("mat", 0);

		quad4.name("quad4");
		quad4.add_attribute("id", 0);
		quad4.add_attribute("mat", 0);

		tri3.name("tri3");
		tri3.add_attribute("id", 0);
		tri3.add_attribute("mat", 0);

		int n = 1, i, j;
		int nmat;
		int nn[20], N;

		// first we write all solid elements
		for (i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			FECoreMesh* pm = po->GetFEMesh();

			for (j = 0; j<pm->Elements(); ++j)
			{
				FEElement_& e = pm->ElementRef(j);
				nmat = 0;
				assert(e.m_gid >= 0);
				GMaterial* pmat = s.GetMaterialFromID(po->Part(e.m_gid)->GetMaterialID());
				if (pmat) nmat = pmat->m_ntag;
				N = e.Nodes();
				for (int k = 0; k<N; ++k) nn[k] = pm->Node(e.m_node[k]).m_nid;

				switch (e.Type())
				{
				case FE_HEX8:
				{
								e.m_ntag = n;
								hex8.set_attribute(n1, n++);
								hex8.set_attribute(n2, nmat);
								hex8.value(nn, 8);
								m_xml.add_leaf(hex8, false);
				}
					break;
				case FE_PYRA5:
				{
					e.m_ntag = n;
					pyra5.set_attribute(n1, n++);
					pyra5.set_attribute(n2, nmat);
					pyra5.value(nn, 8);
					m_xml.add_leaf(pyra5, false);
				}
				break;
                case FE_PYRA13:
                {
                    e.m_ntag = n;
                    pyra13.set_attribute(n1, n++);
                    pyra13.set_attribute(n2, nmat);
                    pyra13.value(nn, 13);
                    m_xml.add_leaf(pyra13, false);
                }
                break;
				case FE_HEX20:
				{
								 e.m_ntag = n;
								 hex20.set_attribute(n1, n++);
								 hex20.set_attribute(n2, nmat);
								 hex20.value(nn, 20);
								 m_xml.add_leaf(hex20, false);
				}
					break;
				case FE_HEX27:
				{
								 e.m_ntag = n;
								 hex27.set_attribute(n1, n++);
								 hex27.set_attribute(n2, nmat);
								 hex27.value(nn, 27);
								 m_xml.add_leaf(hex27, false);
				}
					break;
				case FE_PENTA6:
				{
								  e.m_ntag = n;
								  pen6.set_attribute(n1, n++);
								  pen6.set_attribute(n2, nmat);
								  pen6.value(nn, 6);
								  m_xml.add_leaf(pen6, false);
				}
					break;
				case FE_TET4:
				{
								e.m_ntag = n;
								tet4.set_attribute(n1, n++);
								tet4.set_attribute(n2, nmat);
								tet4.value(nn, 4);
								m_xml.add_leaf(tet4, false);
				}
					break;
				case FE_TET10:
				{
								 e.m_ntag = n;
								 tet10.set_attribute(n1, n++);
								 tet10.set_attribute(n2, nmat);
								 tet10.value(nn, 10);
								 m_xml.add_leaf(tet10, false);
				}
					break;
				case FE_TET15:
				{
								 e.m_ntag = n;
								 tet15.set_attribute(n1, n++);
								 tet15.set_attribute(n2, nmat);
								 tet15.value(nn, 15);
								 m_xml.add_leaf(tet15, false);
				}
					break;
				}
			}
		}

		// next we write all shell elements
		for (i = 0; i<model.Objects(); ++i)
		{
			GObject* po = model.Object(i);
			FECoreMesh* pm = po->GetFEMesh();

			// next we write all shell elements
			for (j = 0; j<pm->Elements(); ++j)
			{
				FEElement_& e = pm->ElementRef(j);
				nmat = 0;
				assert(e.m_gid >= 0);
				GMaterial* pmat = s.GetMaterialFromID(po->Part(e.m_gid)->GetMaterialID());
				if (pmat) nmat = pmat->m_ntag;
				N = e.Nodes();
				for (int k = 0; k<N; ++k) nn[k] = pm->Node(e.m_node[k]).m_nid;

				switch (e.Type())
				{
				case FE_QUAD4:
				{
								 e.m_nid = n;
								 quad4.set_attribute(n1, n++);
								 quad4.set_attribute(n2, nmat);
								 quad4.value(nn, 4);
								 m_xml.add_leaf(quad4, false);
				}
					break;
				case FE_TRI3:
				{
								e.m_nid = n;
								tri3.set_attribute(n1, n++);
								tri3.set_attribute(n2, nmat);
								tri3.value(nn, 3);
								m_xml.add_leaf(tri3, false);
				}
					break;
				}
			}
		}
	}
	m_xml.close_branch(); // Elements
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteGeometryElementData()
{
	FEModel& s = *m_pfem;
	GModel& model = s.GetModel();

	m_xml.add_branch("ElementData");

	int nid;
	XMLElement elem;
	elem.name("element");
	nid = elem.add_attribute("id", 0);
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FECoreMesh* pm = po->GetFEMesh();
		const Transform& T = po->GetTransform();

		for (int j = 0; j<pm->Elements(); ++j)
		{
			FEElement_& e = pm->ElementRef(j);
			GMaterial* pmat = s.GetMaterialFromID(po->Part(e.m_gid)->GetMaterialID());
			FETransverselyIsotropic* ptiso = 0;
			if (pmat) ptiso = dynamic_cast<FETransverselyIsotropic*>(pmat->GetMaterialProperties());

			elem.set_attribute(nid, e.m_nid);
			if (e.IsShell() || e.m_Qactive || (ptiso && (ptiso->GetFiberMaterial()->m_naopt == FE_FIBER_USER)))
			{
				m_xml.add_branch(elem, false);
				if (e.IsShell()) m_xml.add_leaf("thickness", e.m_h, e.Nodes());
				// if material is transversely isotropic with user-defined fibers,
				// export fiber direction, otherwise export local material orientation
				if (ptiso) 
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
				m_xml.close_branch();
			}
		}
	}

	m_xml.close_branch(); // ElementData
}

//-----------------------------------------------------------------------------

void FEBioExport12::WriteBoundarySection(FEStep& s)
{
	XMLElement el;

	// --- B O U N D A R Y   C O N D I T I O N S ---
	// fixed constraints
	WriteBCFixed(s);

	// prescribed displacements
	WriteBCPrescribed(s);

	// --- C O N T A C T ---
	WriteContactSection(s);

	// discrete elements
	// TODO: these are only exported for the initial step
	WriteDiscrete();
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteContactSection(FEStep& s)
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
}

//-----------------------------------------------------------------------------

void FEBioExport12::WriteLoadsSection(FEStep& s)
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

	// heat flux
	WriteHeatFlux(s);

	// convective heat flux
	WriteConvectiveHeatFlux(s);

	// body forces
	WriteBodyForces(s);

	// write heat sources
	WriteHeatSources(s);
}

//-----------------------------------------------------------------------------
// write discrete elements
//
void FEBioExport12::WriteDiscrete()
{
	FEModel& fem = *m_pfem;
	GModel& model = fem.GetModel();
	for (int i = 0; i<model.DiscreteObjects(); ++i)
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
	}
}

//-----------------------------------------------------------------------------
// write rigid joints
//
void FEBioExport12::WriteContactJoint(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		// rigid joints
		FERigidJoint* pj = dynamic_cast<FERigidJoint*> (s.Interface(i));
		if (pj && pj->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "rigid joint");
			m_xml.add_branch(ec);
			{
				int na = (pj->m_pbodyA ? pj->m_pbodyA->m_ntag : 0);
				int nb = (pj->m_pbodyB ? pj->m_pbodyB->m_ntag : 0);

				m_xml.add_leaf("tolerance", pj->GetFloatValue(FERigidJoint::TOL));
				m_xml.add_leaf("penalty", pj->GetFloatValue(FERigidJoint::PENALTY));
				m_xml.add_leaf("body_a", na);
				m_xml.add_leaf("body_b", nb);

				vec3d v = pj->GetVecValue(FERigidJoint::RJ);
				m_xml.add_leaf("joint", v);
			}
			m_xml.close_branch(); // contact - rigid joint
		}
	}
}

//-----------------------------------------------------------------------------
// write rigid walls
//
void FEBioExport12::WriteContactWall(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FERigidWallInterface* pw = dynamic_cast<FERigidWallInterface*> (s.Interface(i));
		if (pw && pw->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "rigid_wall");
			m_xml.add_branch(ec);
			{
				m_xml.add_leaf("laugon", (pw->GetBoolValue(FERigidWallInterface::LAUGON) ? 1 : 0));
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
				int nn[4], n = 1;

				// slave surface
				XMLElement el("surface");
				m_xml.add_branch(el, false);
				{
					XMLElement ef;
					FEItemListBuilder* pitem = pw->GetItemList();
					if (pitem == 0) throw InvalidItemListBuilder(pw);
					auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
					FEFaceList::Iterator pf = pg->First();
					for (j = 0; j<pg->Size(); ++j, ++pf)
					{
						FEFace& face = *(pf->m_pi);
						FECoreMesh* pm = pf->m_pm;
						for (k = 0; k<face.Nodes(); ++k) nn[k] = pm->Node(face.n[k]).m_nid;
						switch (face.Nodes())
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
void FEBioExport12::WriteContactPoro(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FEPoroContact* pp = dynamic_cast<FEPoroContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-biphasic");

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pp->GetParam(n));

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
void FEBioExport12::WriteContactPoroSolute(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FEPoroSoluteContact* pp = dynamic_cast<FEPoroSoluteContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding3");

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pp->GetParam(n));

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
void FEBioExport12::WriteContactMultiphasic(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FEMultiphasicContact* pp = dynamic_cast<FEMultiphasicContact*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-multiphasic");

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pp->GetParam(n));

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
void FEBioExport12::WriteContactTC(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FETensionCompressionInterface* pp = dynamic_cast<FETensionCompressionInterface*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sliding-tension-compression");

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pp->GetParam(n));

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
void FEBioExport12::WriteContactTiedPoro(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FETiedBiphasicInterface* pp = dynamic_cast<FETiedBiphasicInterface*> (s.Interface(i));
		if (pp && pp->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "tied-biphasic");

			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pp->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pp->GetParam(n));

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
void FEBioExport12::WriteContactRigid(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		// rigid interfaces
		FERigidInterface* pr = dynamic_cast<FERigidInterface*> (s.Interface(i));
		if (pr && pr->IsActive())
		{
			GMaterial* pm = pr->GetRigidBody();
			if (pm == 0) throw RigidContactException();
			int rb = pm->m_ntag;

			int i, j;
			vector<int> RC; RC.resize(m_nodes);
			for (i = 0; i<m_nodes; ++i) RC[i] = 0;

			FEItemListBuilder* pitem = pr->GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(pr);
			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator pn = pg->First();
			for (j = 0; j<pg->Size(); ++j, ++pn) RC[(pn->m_pi)->m_nid - 1] = 1;

			XMLElement ec("contact");
			ec.add_attribute("type", "rigid");
			m_xml.add_branch(ec);
			{
				XMLElement el("node");
				int n1 = el.add_attribute("id", 0);
				int n2 = el.add_attribute("rb", 0);

				for (j = 0; j<m_nodes; ++j)
				{
					if (RC[j])
					{
						el.set_attribute(n1, j + 1);
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
void FEBioExport12::WriteContactTied(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FETiedInterface* pt = dynamic_cast<FETiedInterface*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "tied");
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pt->GetParam(n));

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
// write sticky interfaces
//
void FEBioExport12::WriteContactSticky(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FEStickyInterface* pt = dynamic_cast<FEStickyInterface*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "sticky");
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pt->GetParam(n));

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
void FEBioExport12::WriteContactPeriodic(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FEPeriodicBoundary* pt = dynamic_cast<FEPeriodicBoundary*> (s.Interface(i));
		if (pt && pt->IsActive())
		{
			XMLElement ec("contact");
			ec.add_attribute("type", "periodic boundary");
			m_xml.add_branch(ec);
			{
				// write all parameters
				int NP = pt->Parameters();
				for (int n = 0; n<NP; ++n) WriteParam(pt->GetParam(n));

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
void FEBioExport12::WriteContactSliding(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FESlidingInterface* ps = dynamic_cast<FESlidingInterface*> (s.Interface(i));
		if (ps && ps->IsActive())
		{
			XMLElement ec("contact");
			int ntype = ps->GetIntValue(FESlidingInterface::NTYPE);
			if (ntype == 0) ec.add_attribute("type", "sliding_with_gaps");
			else if (ntype == 1) ec.add_attribute("type", "facet-to-facet sliding");

			m_xml.add_branch(ec);
			{
				// write all parameters (except type)
				int NP = ps->Parameters();
				for (int n = 0; n<NP; ++n)
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
void FEBioExport12::WriteSpringTied(FEStep& s)
{
	for (int i = 0; i<s.Interfaces(); ++i)
	{
		FESpringTiedInterface* ps = dynamic_cast<FESpringTiedInterface*> (s.Interface(i));
		if (ps && ps->IsActive())
		{
			double E = ps->GetFloatValue(FESpringTiedInterface::ECONST);
			vector<pair<int, int> > L;
			ps->BuildSpringList(L);
			if (L.empty() == false)
			{
				for (int j = 0; j<(int)L.size(); ++j)
				{
					int n[2] = { L[j].first, L[j].second };
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
void FEBioExport12::WriteBCFixed(FEStep &s)
{
	for (int i = 0; i<s.BCs(); ++i)
	{
		FEBoundaryCondition* pbc = s.BC(i);
		if (pbc->IsActive())
		{
			switch (pbc->Type())
			{
			case FE_FIXED_DISPLACEMENT: WriteBCFixedDisplacement(dynamic_cast<FEFixedDisplacement& >(*pbc), s); break;
			case FE_FIXED_ROTATION: WriteBCFixedRotation(dynamic_cast<FEFixedRotation&     >(*pbc), s); break;
			case FE_FIXED_FLUID_PRESSURE: WriteBCFixedFluidPressure(dynamic_cast<FEFixedFluidPressure&>(*pbc), s); break;
			case FE_FIXED_TEMPERATURE: WriteBCFixedTemperature(dynamic_cast<FEFixedTemperature&  >(*pbc), s); break;
			case FE_FIXED_CONCENTRATION: WriteBCFixedConcentration(dynamic_cast<FEFixedConcentration&>(*pbc), s); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteBCFixedDisplacement(FEFixedDisplacement& rbc, FEStep& s)
{
	const char* xyz[] = { "x", "y", "xy", "z", "xz", "yz", "xyz" };

	// build the node list
	FEItemListBuilder* pItem = rbc.GetItemList();
	if (pItem == 0) throw InvalidItemListBuilder(&rbc);
	FENodeList* pns = pItem->BuildNodeList();
	if (pns == 0) throw InvalidItemListBuilder(&rbc);
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

		// write fix DOFS
		m_xml.add_branch("fix");
		{
			XMLElement el;
			el.name("node");
			int n1 = el.add_attribute("id", 0);
			int n2 = el.add_attribute("bc", 0);

			// write the BC's
			int N = pg->Size();
			FENodeList::Iterator pi = pg->First();
			for (int k = 0; k<N; ++k, ++pi)
			{
				FENode* pn = pi->m_pi;
				int nid = pn->m_nid;

				el.set_attribute(n1, nid);
				el.set_attribute(n2, sbc);
				m_xml.add_empty(el, false);
			}
		}
		m_xml.close_branch(); // fix
	}
}

//-----------------------------------------------------------------------------
void FEBioExport12::WriteBCFixedRotation(FEFixedRotation& rbc, FEStep& s)
{
	const char* uvw[] = { "u", "v", "uv", "w", "uw", "vw", "uvw" };

	// build the node list
	FEItemListBuilder* pItem = rbc.GetItemList();
	if (pItem == 0) throw InvalidItemListBuilder(&rbc);
	FENodeList* pns = pItem->BuildNodeList();
	if (pns == 0) throw InvalidItemListBuilder(&rbc);
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
		sbc = uvw[nbc - 1];

		// write fix DOFS
		m_xml.add_branch("fix");
		{
			XMLElement el;
			el.name("node");
			int n1 = el.add_attribute("id", 0);
			int n2 = el.add_attribute("bc", 0);

			// write the BC's
			int N = pg->Size();
			FENodeList::Iterator pi = pg->First();
			for (int k = 0; k<N; ++k, ++pi)
			{
				FENode* pn = pi->m_pi;
				int nid = pn->m_nid;

				el.set_attribute(n1, nid);
				el.set_attribute(n2, sbc);
				m_xml.add_empty(el, false);
			}
		}
		m_xml.close_branch(); // fix
	}
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport12::WriteBCFixedFluidPressure(FEFixedFluidPressure& rbc, FEStep& s)
{
	vector<int> BC; BC.resize(m_nodes);

	m_xml.add_branch("fix");
	{
		XMLElement el;
		el.name("node");
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);

		// get the BC for this constraint
		int bc = rbc.GetBC();

		// reset nodal BC's
		for (int k = 0; k<m_nodes; ++k) BC[k] = 0;

		// build the node list
		FEItemListBuilder* pItem = rbc.GetItemList();
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);

		auto_ptr<FENodeList> pg(pItem->BuildNodeList());

		FENodeList::Iterator it = pg->First();
		for (int k = 0; k<pg->Size(); ++k, ++it)
		{
			FENode& node = *(it->m_pi);
			BC[node.m_nid - 1] |= bc;
		}

		// write the BC's
		for (int k = 0; k<m_nodes; ++k)
		{
			if (BC[k] > 0)
			{
				el.set_attribute(n1, k + 1);
				bc = BC[k];

				// fluid pressure dofs
				el.set_attribute(n2, "p");
				m_xml.add_empty(el, false);
			}
		}
	}
	m_xml.close_branch(); // fix
}


//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport12::WriteBCFixedTemperature(FEFixedTemperature& rbc, FEStep& s)
{
	vector<int> BC; BC.resize(m_nodes);
	m_xml.add_branch("fix");
	{
		XMLElement el;
		el.name("node");
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);

		// get the BC for this constraint
		int bc = rbc.GetBC();

		// reset nodal BC's
		for (int k = 0; k<m_nodes; ++k) BC[k] = 0;

		// build the node list
		FEItemListBuilder* pItem = rbc.GetItemList();
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);

		auto_ptr<FENodeList> pg(pItem->BuildNodeList());

		FENodeList::Iterator it = pg->First();
		for (int k = 0; k<pg->Size(); ++k, ++it)
		{
			FENode& node = *(it->m_pi);
			BC[node.m_nid - 1] |= bc;
		}

		// write the BC's
		for (int k = 0; k<m_nodes; ++k)
		{
			if (BC[k] > 0)
			{
				el.set_attribute(n1, k + 1);
				bc = BC[k];

				// fluid pressure dofs
				el.set_attribute(n2, "t");
				m_xml.add_empty(el, false);
			}
		}
	}
	m_xml.close_branch(); // fix
}

//-----------------------------------------------------------------------------
// Export the fixed degrees of freedom
//
void FEBioExport12::WriteBCFixedConcentration(FEFixedConcentration& rbc, FEStep& s)
{
	vector<int> BC; BC.resize(m_nodes);

	m_xml.add_branch("fix");
	{
		XMLElement el;
		el.name("node");
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);

		// get the BC for this constraint
		int bc = rbc.GetBC();

		// reset nodal BC's
		for (int k = 0; k<m_nodes; ++k) BC[k] = 0;

		// build the node list
		FEItemListBuilder* pItem = rbc.GetItemList();
		if (pItem == 0) throw InvalidItemListBuilder(&rbc);

		auto_ptr<FENodeList> pg(pItem->BuildNodeList());

		FENodeList::Iterator it = pg->First();
		for (int k = 0; k<pg->Size(); ++k, ++it)
		{
			FENode& node = *(it->m_pi);
			BC[node.m_nid - 1] |= bc;
		}

		// write the BC's
		for (int k = 0; k<m_nodes; ++k)
		{
			if (BC[k] > 0)
			{
				el.set_attribute(n1, k + 1);
				bc = BC[k];

				if (bc & 1) { el.set_attribute(n2, "c1"); m_xml.add_empty(el, false); }
				if (bc & 2) { el.set_attribute(n2, "c2"); m_xml.add_empty(el, false); }
				if (bc & 4) { el.set_attribute(n2, "c3"); m_xml.add_empty(el, false); }
				if (bc & 8) { el.set_attribute(n2, "c4"); m_xml.add_empty(el, false); }
				if (bc & 16) { el.set_attribute(n2, "c5"); m_xml.add_empty(el, false); }
				if (bc & 32) { el.set_attribute(n2, "c6"); m_xml.add_empty(el, false); }
			}
		}
	}
	m_xml.close_branch(); // fix
}

//-----------------------------------------------------------------------------
// Export prescribed boundary conditions
void FEBioExport12::WriteBCPrescribed(FEStep &s)
{
	for (int i = 0; i<s.BCs(); ++i)
	{
		FEBoundaryCondition* pbc = s.BC(i);
		if (pbc->IsActive())
		{
			switch (pbc->Type())
			{
			case FE_PRESCRIBED_DISPLACEMENT: WriteBCPrescribedDisplacement(dynamic_cast<FEPrescribedDisplacement &>(*pbc), s); break;
			case FE_PRESCRIBED_ROTATION: WriteBCPrescribedRotation(dynamic_cast<FEPrescribedRotation     &>(*pbc), s); break;
			case FE_PRESCRIBED_FLUID_PRESSURE: WriteBCPrescribedFluidPressure(dynamic_cast<FEPrescribedFluidPressure&>(*pbc), s); break;
			case FE_PRESCRIBED_TEMPERATURE: WriteBCPrescribedTemperature(dynamic_cast<FEPrescribedTemperature  &>(*pbc), s); break;
			case FE_PRESCRIBED_CONCENTRATION: WriteBCPrescribedConcentration(dynamic_cast<FEPrescribedConcentration&>(*pbc), s); break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Export prescribed displacements
//
void FEBioExport12::WriteBCPrescribedDisplacement(FEPrescribedDisplacement& rbc, FEStep& s)
{
	int k, l;
	int lc;
	bool bn;
	char bc[3][2] = { "x", "y", "z" };
	double val;

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	m_xml.add_branch(e);
	{
		XMLElement el;
		el.name("node");
		el.value(1.0);
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);
		int n3 = el.add_attribute("lc", 0);

		vector<int> DC; DC.resize(m_nodes);

		FELoadCurve* plc = rbc.GetLoadCurve();
		l = rbc.GetDOF();
		lc = plc->GetID();
		bn = true; // plc->IsActive();
		val = rbc.GetScaleFactor();

		if (bn)
		{
			for (k = 0; k<m_nodes; ++k) DC[k] = 0;

			FEItemListBuilder* pitem = rbc.GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(&rbc);

			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k = 0; k<N; ++k, ++it)
			{
				pn = it->m_pi;
				DC[pn->m_nid - 1] = 1;
			}

			for (k = 0; k<m_nodes; ++k)
			{
				if (DC[k])
				{
					el.value(val);
					el.set_attribute(n1, k + 1);
					el.set_attribute(n2, bc[l]);
					el.set_attribute(n3, lc);
					m_xml.add_leaf(el, false);
				}
			}
		}
	}
	m_xml.close_branch(); // prescribe
}

//-----------------------------------------------------------------------------
// Export prescribed rotations
//
void FEBioExport12::WriteBCPrescribedRotation(FEPrescribedRotation& rbc, FEStep& s)
{
	int k, l;
	int lc;
	bool bn;
	char bc[3][2] = { "u", "v", "w" };
	double val;

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	m_xml.add_branch(e);
	{
		XMLElement el;
		el.name("node");
		el.value(1.0);
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);
		int n3 = el.add_attribute("lc", 0);

		vector<int> DC; DC.resize(m_nodes);

		FELoadCurve* plc = rbc.GetLoadCurve();
		l = rbc.GetDOF();
		lc = plc->GetID();
		bn = true; // plc->IsActive();
		val = rbc.GetScaleFactor();

		if (bn)
		{
			for (k = 0; k<m_nodes; ++k) DC[k] = 0;

			FEItemListBuilder* pitem = rbc.GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(&rbc);

			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k = 0; k<N; ++k, ++it)
			{
				pn = it->m_pi;
				DC[pn->m_nid - 1] = 1;
			}

			for (k = 0; k<m_nodes; ++k)
			{
				if (DC[k])
				{
					el.value(val);
					el.set_attribute(n1, k + 1);
					el.set_attribute(n2, bc[l]);
					el.set_attribute(n3, lc);
					m_xml.add_leaf(el, false);
				}
			}
		}
	}
	m_xml.close_branch(); // prescribe
}

//-----------------------------------------------------------------------------
// Export prescribed fluid pressures
//
void FEBioExport12::WriteBCPrescribedFluidPressure(FEPrescribedFluidPressure& rbc, FEStep& s)
{
	int k;
	int lc;
	bool bn;
	double val;

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	m_xml.add_branch(e);
	{
		XMLElement el;
		el.name("node");
		el.value(1.0);
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);
		int n3 = el.add_attribute("lc", 0);

		vector<int> DC; DC.resize(m_nodes);

		FELoadCurve* plc = rbc.GetLoadCurve();
		lc = plc->GetID();
		bn = true; // plc->IsActive();
		val = rbc.GetScaleFactor();

		if (bn)
		{
			for (k = 0; k<m_nodes; ++k) DC[k] = 0;

			FEItemListBuilder* pitem = rbc.GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(&rbc);

			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k = 0; k<N; ++k, ++it)
			{
				pn = it->m_pi;
				DC[pn->m_nid - 1] = 1;
			}

			for (k = 0; k<m_nodes; ++k)
			{
				if (DC[k])
				{
					el.value(val);
					el.set_attribute(n1, k + 1);
					el.set_attribute(n2, "p");
					el.set_attribute(n3, lc);
					m_xml.add_leaf(el, false);
				}
			}
		}
	}
	m_xml.close_branch(); // prescribe
}

//-----------------------------------------------------------------------------
// Export prescribed temperatures
//
void FEBioExport12::WriteBCPrescribedTemperature(FEPrescribedTemperature& rbc, FEStep& s)
{
	int k;
	int lc;
	bool bn;
	double val;

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	m_xml.add_branch(e);
	{
		XMLElement el;
		el.name("node");
		el.value(1.0);
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);
		int n3 = el.add_attribute("lc", 0);

		vector<int> DC; DC.resize(m_nodes);

		FELoadCurve* plc = rbc.GetLoadCurve();
		lc = plc->GetID();
		bn = true; // plc->IsActive();
		val = rbc.GetScaleFactor();

		if (bn)
		{
			for (k = 0; k<m_nodes; ++k) DC[k] = 0;

			FEItemListBuilder* pitem = rbc.GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(&rbc);

			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k = 0; k<N; ++k, ++it)
			{
				pn = it->m_pi;
				DC[pn->m_nid - 1] = 1;
			}

			for (k = 0; k<m_nodes; ++k)
			{
				if (DC[k])
				{
					el.value(val);
					el.set_attribute(n1, k + 1);
					el.set_attribute(n2, "t");
					el.set_attribute(n3, lc);
					m_xml.add_leaf(el, false);
				}
			}
		}
	}
	m_xml.close_branch(); // prescribe
}

//-----------------------------------------------------------------------------
// Export prescribed concentration
//
void FEBioExport12::WriteBCPrescribedConcentration(FEPrescribedConcentration& rbc, FEStep& s)
{
	int k;
	int lc;
	bool bn;
	double val;
	char szbc[6][3] = { "c1", "c2", "c3", "c4", "c5", "c6" };

	XMLElement e;
	e.name("prescribe");
	if (rbc.GetRelativeFlag()) e.add_attribute("type", "relative");

	m_xml.add_branch(e);
	{
		XMLElement el;
		el.name("node");
		el.value(1.0);
		int n1 = el.add_attribute("id", 0);
		int n2 = el.add_attribute("bc", 0);
		int n3 = el.add_attribute("lc", 0);

		vector<int> DC; DC.resize(m_nodes);

		int l = rbc.GetDOF();
		FELoadCurve* plc = rbc.GetLoadCurve();
		lc = plc->GetID();
		bn = true; // plc->IsActive();
		val = rbc.GetScaleFactor();

		if (bn)
		{
			for (k = 0; k<m_nodes; ++k) DC[k] = 0;

			FEItemListBuilder* pitem = rbc.GetItemList();
			if (pitem == 0) throw InvalidItemListBuilder(&rbc);

			auto_ptr<FENodeList> pg(pitem->BuildNodeList());
			FENodeList::Iterator it = pg->First();
			FENode* pn;
			int N = pg->Size();
			for (k = 0; k<N; ++k, ++it)
			{
				pn = it->m_pi;
				DC[pn->m_nid - 1] = 1;
			}

			for (k = 0; k<m_nodes; ++k)
			{
				if (DC[k])
				{
					el.value(val);
					el.set_attribute(n1, k + 1);
					el.set_attribute(n2, szbc[l]);
					el.set_attribute(n3, lc);
					m_xml.add_leaf(el, false);
				}
			}
		}
	}
	m_xml.close_branch(); // prescribe
}

//-----------------------------------------------------------------------------
// export nodal loads
//
void FEBioExport12::WriteLoadNodal(FEStep& s)
{
	for (int j = 0; j<s.Loads(); ++j)
	{
		FENodalLoad* pbc = dynamic_cast<FENodalLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			m_xml.add_branch("force");
			{
				XMLElement el;
				el.name("node");
				el.value(1.0);
				int n1 = el.add_attribute("id", 0);
				int n2 = el.add_attribute("bc", 0);
				int n3 = el.add_attribute("lc", 0);

				vector<int> FC; FC.resize(m_nodes);

				FELoadCurve* plc;
				FENode* pn;
				int lc;
				bool bn;
				char bc[][3] = { "x", "y", "z", "p", "c1", "c2", "c3", "c4", "c5", "c6" };

				int l = pbc->GetDOF();
				plc = pbc->GetLoadCurve();
				lc = plc->GetID();
				bn = true; // plc->IsActive();
				el.value(pbc->GetLoad());

				if (bn)
				{
					for (int k = 0; k<m_nodes; ++k) FC[k] = 0;

					FEItemListBuilder* pitem = pbc->GetItemList();
					if (pitem == 0) throw InvalidItemListBuilder(pbc);

					auto_ptr<FENodeList> pg(pitem->BuildNodeList());
					FENodeList::Iterator it = pg->First();
					for (int k = 0; k<pg->Size(); ++k, ++it)
					{
						pn = it->m_pi;
						FC[pn->m_nid - 1] = 1;
					}

					for (int k = 0; k<m_nodes; ++k)
					{
						if (FC[k])
						{
							el.set_attribute(n1, k + 1);
							el.set_attribute(n2, bc[l]);
							el.set_attribute(n3, lc);
							m_xml.add_leaf(el, false);
						}
					}
				}
			}
			m_xml.close_branch(); // force
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure loads
//
void FEBioExport12::WriteLoadPressure(FEStep& s)
{
	int j, k, l, n, nn[8];

	for (j = 0; j<s.Loads(); ++j)
	{
		FEPressureLoad* pbc = dynamic_cast<FEPressureLoad*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement load;
			load.name("pressure");
			if (pbc->GetLinearFlag()) load.add_attribute("type", "linear");
			m_xml.add_branch(load);
			{
				XMLElement quad, tri3, tri6, tri7, quad8, quad9;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);

				tri3.name("tri3");
				tri3.add_attribute("id", 0);
				tri3.add_attribute("lc", 0);
				tri3.add_attribute("scale", 1.0);

				tri6.name("tri6");
				tri6.add_attribute("id", 0);
				tri6.add_attribute("lc", 0);
				tri6.add_attribute("scale", 1.0);

				tri7.name("tri7");
				tri7.add_attribute("id", 0);
				tri7.add_attribute("lc", 0);
				tri7.add_attribute("scale", 1.0);

				quad8.name("quad8");
				quad8.add_attribute("id", 0);
				quad8.add_attribute("lc", 0);
				quad8.add_attribute("scale", 1.0);

				quad9.name("quad9");
				quad9.add_attribute("id", 0);
				quad9.add_attribute("lc", 0);
				quad9.add_attribute("scale", 1.0);

				n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				FEFaceList* pfl = pitem->BuildFaceList();
				if (pfl == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pfl);

				FEFaceList::Iterator pf = pg->First();

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;
					switch (face.Nodes())
					{
					case 3:
					{
							  for (l = 0; l<3; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  tri3.set_attribute(n1, n);
							  tri3.set_attribute(n2, lc);
							  tri3.set_attribute(n3, pbc->GetLoad());
							  tri3.value(nn, 3);
							  m_xml.add_leaf(tri3, false);
					}
						break;
					case 4:
					{
							  for (l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetLoad());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					case 6:
					{
							  for (l = 0; l<6; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  tri6.set_attribute(n1, n);
							  tri6.set_attribute(n2, lc);
							  tri6.set_attribute(n3, pbc->GetLoad());
							  tri6.value(nn, 6);
							  m_xml.add_leaf(tri6, false);
					}
						break;
					case 7:
					{
							  for (l = 0; l<7; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  tri7.set_attribute(n1, n);
							  tri7.set_attribute(n2, lc);
							  tri7.set_attribute(n3, pbc->GetLoad());
							  tri7.value(nn, 7);
							  m_xml.add_leaf(tri7, false);
					}
						break;
					case 8:
					{
							  for (l = 0; l<8; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  quad8.set_attribute(n1, n);
							  quad8.set_attribute(n2, lc);
							  quad8.set_attribute(n3, pbc->GetLoad());
							  quad8.value(nn, 8);
							  m_xml.add_leaf(quad8, false);
					}
						break;
					case 9:
					{
							  for (l = 0; l<9; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
							  quad9.set_attribute(n1, n);
							  quad9.set_attribute(n2, lc);
							  quad9.set_attribute(n3, pbc->GetLoad());
							  quad9.value(nn, 9);
							  m_xml.add_leaf(quad9, false);
					}
						break;
					default:
						assert(false);
					}
					++n;
				}
			}
			m_xml.close_branch(); // pressure
		}
	}
}

//----------------------------------------------------------------------------
// Export fluid flux
//
void FEBioExport12::WriteFluidFlux(FEStep& s)
{
	for (int j = 0; j<s.Loads(); ++j)
	{
		FEFluidFlux* pbc = dynamic_cast<FEFluidFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement flux;
			flux.name("fluidflux");
			flux.add_attribute("type", (pbc->GetLinearFlag() ? "linear" : "nonlinear"));
			flux.add_attribute("flux", (pbc->GetMixtureFlag() ? "mixture" : "fluid"));
			m_xml.add_branch(flux);
			{
				XMLElement quad, tri;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);

				tri.name("tri3");
				tri.add_attribute("id", 0);
				tri.add_attribute("lc", 0);
				tri.add_attribute("scale", 1.0);

				int n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();

				for (int k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					int nn[4];
					for (int l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (face.Nodes())
					{
					case 3:
					{
							  tri.set_attribute(n1, n);
							  tri.set_attribute(n2, lc);
							  tri.set_attribute(n3, pbc->GetLoad());
							  tri.value(nn, 3);
							  m_xml.add_leaf(tri, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetLoad());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // fluidflux
		}
	}
}


//----------------------------------------------------------------------------
// Export mixture normal traction
//
void FEBioExport12::WriteBPNormalTraction(FEStep& s)
{
	int j, k, l, n, nn[4];

	for (j = 0; j<s.Loads(); ++j)
	{
		FEBPNormalTraction* pbc = dynamic_cast<FEBPNormalTraction*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement flux;
			flux.name("normal_traction");
			flux.add_attribute("type", (pbc->GetLinearFlag() ? "linear" : "nonlinear"));
			flux.add_attribute("traction", (pbc->GetMixtureFlag() ? "mixture" : "effective"));
			m_xml.add_branch(flux);
			{
				XMLElement quad, tri;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);

				tri.name("tri3");
				tri.add_attribute("id", 0);
				tri.add_attribute("lc", 0);
				tri.add_attribute("scale", 1.0);

				n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					for (l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (face.Nodes())
					{
					case 3:
					{
							  tri.set_attribute(n1, n);
							  tri.set_attribute(n2, lc);
							  tri.set_attribute(n3, pbc->GetLoad());
							  tri.value(nn, 3);
							  m_xml.add_leaf(tri, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetLoad());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // normal_traction
		}
	}
}

//----------------------------------------------------------------------------
// Export heat flux
//
void FEBioExport12::WriteHeatFlux(FEStep& s)
{
	int j, k, l, n, nn[4];

	for (j = 0; j<s.Loads(); ++j)
	{
		FEHeatFlux* pbc = dynamic_cast<FEHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			m_xml.add_branch("heatflux");
			{
				XMLElement quad, tri;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);

				tri.name("tri3");
				tri.add_attribute("id", 0);
				tri.add_attribute("lc", 0);
				tri.add_attribute("scale", 1.0);

				n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					for (l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (face.Nodes())
					{
					case 3:
					{
							  tri.set_attribute(n1, n);
							  tri.set_attribute(n2, lc);
							  tri.set_attribute(n3, pbc->GetLoad());
							  tri.value(nn, 3);
							  m_xml.add_leaf(tri, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetLoad());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // heatflux
		}
	}
}

//----------------------------------------------------------------------------
// Export convective heat flux
//
void FEBioExport12::WriteConvectiveHeatFlux(FEStep& s)
{
	int j, k, l, n, nn[4];

	for (j = 0; j<s.Loads(); ++j)
	{
		FEConvectiveHeatFlux* pbc = dynamic_cast<FEConvectiveHeatFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			m_xml.add_branch("convective_heatflux");
			{
				XMLElement quad, tri;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);
				int n4 = quad.add_attribute("hc", 0.0);

				tri.name("tri3");
				tri.add_attribute("id", 0);
				tri.add_attribute("lc", 0);
				tri.add_attribute("scale", 1.0);
				tri.add_attribute("hc", 0.0);

				n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					for (l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (face.Nodes())
					{
					case 3:
					{
							  tri.set_attribute(n1, n);
							  tri.set_attribute(n2, lc);
							  tri.set_attribute(n3, pbc->GetTemperature());
							  tri.set_attribute(n4, pbc->GetCoefficient());
							  tri.value(nn, 3);
							  m_xml.add_leaf(tri, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetTemperature());
							  quad.set_attribute(n4, pbc->GetCoefficient());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // convective_heatflux
		}
	}
}

//----------------------------------------------------------------------------
// Export solute flux
//
void FEBioExport12::WriteSoluteFlux(FEStep& s)
{
	int j, k, l, n, nn[4];

	for (j = 0; j<s.Loads(); ++j)
	{
		FESoluteFlux* pbc = dynamic_cast<FESoluteFlux*>(s.Load(j));
		if (pbc && pbc->IsActive())
		{
			XMLElement flux;
			flux.name("soluteflux");
			flux.add_attribute("type", (pbc->GetLinearFlag() ? "linear" : "nonlinear"));
			flux.add_attribute("sol", pbc->GetBC() + 1);
			m_xml.add_branch(flux);
			{
				XMLElement quad, tri;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("scale", 1.0);

				tri.name("tri3");
				tri.add_attribute("id", 0);
				tri.add_attribute("lc", 0);
				tri.add_attribute("scale", 1.0);

				n = 1;

				FELoadCurve* plc = pbc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					for (l = 0; l<4; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (face.Nodes())
					{
					case 3:
					{
							  tri.set_attribute(n1, n);
							  tri.set_attribute(n2, lc);
							  tri.set_attribute(n3, pbc->GetLoad());
							  tri.value(nn, 3);
							  m_xml.add_leaf(tri, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, pbc->GetLoad());
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // soluteflux
		}
	}
}

//----------------------------------------------------------------------------
// Export pressure tractions
//
void FEBioExport12::WriteLoadTraction(FEStep& s)
{
	int j, k, l, n, nn[10];
	for (j = 0; j<s.Loads(); ++j)
	{
		FESurfaceTraction* ptc = dynamic_cast<FESurfaceTraction*>(s.Load(j));
		if (ptc && ptc->IsActive())
		{
			m_xml.add_branch("traction");
			{
				XMLElement quad, tri3, tri6, tri7, quad8, quad9;

				quad.name("quad4");
				int n1 = quad.add_attribute("id", 0);
				int n2 = quad.add_attribute("lc", 0);
				int n3 = quad.add_attribute("tx", 0.0);
				int n4 = quad.add_attribute("ty", 0.0);
				int n5 = quad.add_attribute("tz", 0.0);

				tri3.name("tri3");
				tri3.add_attribute("id", 0);
				tri3.add_attribute("lc", 0);
				tri3.add_attribute("tx", 0.0);
				tri3.add_attribute("ty", 0.0);
				tri3.add_attribute("tz", 0.0);

				tri6.name("tri6");
				tri6.add_attribute("id", 0);
				tri6.add_attribute("lc", 0);
				tri6.add_attribute("tx", 0.0);
				tri6.add_attribute("ty", 0.0);
				tri6.add_attribute("tz", 0.0);

				tri7.name("tri7");
				tri7.add_attribute("id", 0);
				tri7.add_attribute("lc", 0);
				tri7.add_attribute("tx", 0.0);
				tri7.add_attribute("ty", 0.0);
				tri7.add_attribute("tz", 0.0);

				quad8.name("quad8");
				quad8.add_attribute("id", 0);
				quad8.add_attribute("lc", 0);
				quad8.add_attribute("tx", 0.0);
				quad8.add_attribute("ty", 0.0);
				quad8.add_attribute("tz", 0.0);

				quad9.name("quad9");
				quad9.add_attribute("id", 0);
				quad9.add_attribute("lc", 0);
				quad9.add_attribute("tx", 0.0);
				quad9.add_attribute("ty", 0.0);
				quad9.add_attribute("tz", 0.0);

				n = 1;

				FELoadCurve* plc = ptc->GetLoadCurve();
				int lc = plc->GetID();
				FEItemListBuilder* pitem = ptc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(ptc);

				auto_ptr<FEFaceList> pg(pitem->BuildFaceList());
				FEFaceList::Iterator pf = pg->First();
				vec3d t = ptc->GetTraction();

				if (pg->Size() == 0)
				{
					//					flx_error("Empty facet list when exporting load %s", ptc->GetName());
					return;
				}

				for (k = 0; k<pg->Size(); ++k, ++pf)
				{
					FEFace& face = *(pf->m_pi);
					FECoreMesh* pm = pf->m_pm;

					int nf = face.Nodes();
					for (l = 0; l<nf; ++l) nn[l] = pm->Node(face.n[l]).m_nid;
					switch (nf)
					{
					case 3:
					{
							  tri3.set_attribute(n1, n);
							  tri3.set_attribute(n2, lc);
							  tri3.set_attribute(n3, t.x);
							  tri3.set_attribute(n4, t.y);
							  tri3.set_attribute(n5, t.z);
							  tri3.value(nn, 3);
							  m_xml.add_leaf(tri3, false);
					}
						break;
					case 4:
					{
							  quad.set_attribute(n1, n);
							  quad.set_attribute(n2, lc);
							  quad.set_attribute(n3, t.x);
							  quad.set_attribute(n4, t.y);
							  quad.set_attribute(n5, t.z);
							  quad.value(nn, 4);
							  m_xml.add_leaf(quad, false);
					}
						break;
					case 6:
					{
							  tri6.set_attribute(n1, n);
							  tri6.set_attribute(n2, lc);
							  tri6.set_attribute(n3, t.x);
							  tri6.set_attribute(n4, t.y);
							  tri6.set_attribute(n5, t.z);
							  tri6.value(nn, 6);
							  m_xml.add_leaf(tri6, false);
					}
						break;
					case 7:
					{
							  tri7.set_attribute(n1, n);
							  tri7.set_attribute(n2, lc);
							  tri7.set_attribute(n3, t.x);
							  tri7.set_attribute(n4, t.y);
							  tri7.set_attribute(n5, t.z);
							  tri7.value(nn, 7);
							  m_xml.add_leaf(tri7, false);
					}
						break;
					case 8:
					{
							  quad8.set_attribute(n1, n);
							  quad8.set_attribute(n2, lc);
							  quad8.set_attribute(n3, t.x);
							  quad8.set_attribute(n4, t.y);
							  quad8.set_attribute(n5, t.z);
							  quad8.value(nn, 8);
							  m_xml.add_leaf(quad8, false);
					}
						break;
					case 9:
					{
							  quad9.set_attribute(n1, n);
							  quad9.set_attribute(n2, lc);
							  quad9.set_attribute(n3, t.x);
							  quad9.set_attribute(n4, t.y);
							  quad9.set_attribute(n5, t.z);
							  quad9.value(nn, 9);
							  m_xml.add_leaf(quad9, false);
					}
						break;
					}
					++n;
				}
			}
			m_xml.close_branch(); // pressure
		}
	}
}

//-----------------------------------------------------------------------------
// Export initial conditions
//
void FEBioExport12::WriteInitialSection()
{
	FEModel& fem = m_prj.GetFEModel();
	FEStep& s = *fem.GetStep(0);

	vector<int> VC; VC.resize(m_nodes);

	// initial velocities
	for (int j = 0; j<s.ICs(); ++j)
	{
		FENodalVelocities* pbc = dynamic_cast<FENodalVelocities*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			vec3d v = pbc->GetVelocity();
			m_xml.add_branch("velocity");
			{
				XMLElement el;

				for (int k = 0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k = 0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid - 1] = 1;
				}

				for (int k = 0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k + 1);
						el.value(v);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial concentration
	for (int j = 0; j<s.BCs(); ++j)
	{
		FEInitConcentration* pbc = dynamic_cast<FEInitConcentration*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double c = pbc->GetValue();
			int bc = pbc->GetBC();

			XMLElement ec;
			ec.name("concentration");
			ec.add_attribute("sol", bc + 1);
			m_xml.add_branch(ec);
			{
				XMLElement el;

				for (int k = 0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k = 0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid - 1] = 1;
				}

				for (int k = 0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k + 1);
						el.value(c);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial fluid pressure
	for (int j = 0; j<s.BCs(); ++j)
	{
		FEInitFluidPressure* pbc = dynamic_cast<FEInitFluidPressure*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double p = pbc->GetValue();
			m_xml.add_branch("fluid_pressure");
			{
				XMLElement el;

				for (int k = 0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k = 0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid - 1] = 1;
				}

				for (int k = 0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k + 1);
						el.value(p);
						m_xml.add_leaf(el);
					}
				}
			}
			m_xml.close_branch();
		}
	}

	// initial temperature
	for (int j = 0; j<s.BCs(); ++j)
	{
		FEInitTemperature* pbc = dynamic_cast<FEInitTemperature*>(s.IC(j));
		if (pbc && pbc->IsActive())
		{
			double T = pbc->GetValue();
			m_xml.add_branch("temperature");
			{
				XMLElement el;

				for (int k = 0; k<m_nodes; ++k) VC[k] = 0;

				FEItemListBuilder* pitem = pbc->GetItemList();
				if (pitem == 0) throw InvalidItemListBuilder(pbc);

				auto_ptr<FENodeList> pg(pitem->BuildNodeList());
				FENodeList::Iterator it = pg->First();
				for (int k = 0; k<pg->Size(); ++k, ++it)
				{
					FENode* pn = it->m_pi;
					VC[pn->m_nid - 1] = 1;
				}

				for (int k = 0; k<m_nodes; ++k)
				{
					if (VC[k])
					{
						el.name("node");
						el.add_attribute("id", k + 1);
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
void FEBioExport12::WriteBodyForces(FEStep &s)
{
	for (int i = 0; i<s.Loads(); ++i)
	{
		FEConstBodyForce* pbl = dynamic_cast<FEConstBodyForce*>(s.Load(i));
		if (pbl && pbl->IsActive())
		{
			m_xml.add_branch("body_force");
			{
				char sz[3][2] = { "x", "y", "z" };
				XMLElement el;
				for (int i = 0; i<3; ++i)
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
void FEBioExport12::WriteHeatSources(FEStep& s)
{
	for (int i = 0; i<s.Loads(); ++i)
	{
		FEHeatSource* phs = dynamic_cast<FEHeatSource*>(s.Load(i));
		if (phs && phs->IsActive())
		{
			m_xml.add_branch("heat_source");
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

void FEBioExport12::WriteGlobalsSection()
{
	XMLElement el;
	FEModel& fem = *m_pfem;

	if (fem.Parameters())
	{
		m_xml.add_branch("Constants");
		{
			int N = fem.Parameters();
			for (int i = 0; i<N; ++i)
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
				for (int i = 0; i<NS; ++i)
				{
					FESoluteData& s = fem.GetSoluteData(i);
					XMLElement el;
					el.name("solute");
					el.add_attribute("id", i + 1);
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
				for (int i = 0; i<NS; ++i)
				{
					FESoluteData& s = fem.GetSBMData(i);
					XMLElement el;
					el.name("solid_bound");
					el.add_attribute("id", i + 1);
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

void FEBioExport12::WriteLoadDataSection()
{
	for (int i = 0; i<(int)m_pLC.size(); ++i)
	{
		FELoadCurve* plc = m_pLC[i];

		XMLElement el;
		el.name("loadcurve");
		el.add_attribute("id", i + 1);

		switch (plc->GetType())
		{
		case FELoadCurve::LC_STEP: el.add_attribute("type", "step"); break;
		case FELoadCurve::LC_LINEAR: el.add_attribute("type", "linear"); break;
		case FELoadCurve::LC_SMOOTH: el.add_attribute("type", "smooth"); break;
		}

		switch (plc->GetExtend())
		{
			//		case FELoadCurve::EXT_CONSTANT     : el.add_attribute("extend", "constant"     ); break;
		case FELoadCurve::EXT_EXTRAPOLATE: el.add_attribute("extend", "extrapolate"); break;
		case FELoadCurve::EXT_REPEAT: el.add_attribute("extend", "repeat"); break;
		case FELoadCurve::EXT_REPEAT_OFFSET: el.add_attribute("extend", "repeat offset"); break;
		}

		double d[2];

		const char* szpt = "loadpoint";

		m_xml.add_branch(el);
		{
			for (int j = 0; j<plc->Size(); ++j)
			{
				LOADPOINT& pt = plc->Item(j);
				d[0] = pt.time;
				d[1] = pt.load;
				m_xml.add_leaf(szpt, d, 2);
			}
		}
		m_xml.close_branch(); // loadcurve
	}
}

//-----------------------------------------------------------------------------

void FEBioExport12::WriteSurfaceSection(FEFaceList& s)
{
	XMLElement ef;
	int n = 1, nn[8];

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
		for (int j = 0; j<NF; ++j, ++n, ++pf)
		{
			FEFace& face = *(pf->m_pi);
			FECoreMesh* pm = pf->m_pm;
			nfn = face.Nodes();
			for (int k = 0; k<nfn; ++k) nn[k] = pm->Node(face.n[k]).m_nid;
			switch (nfn)
			{
			case 3: ef.name("tri3"); break;
			case 4: ef.name("quad4"); break;
			case 6: ef.name("tri6"); break;
			case 7: ef.name("tri7"); break;
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
void FEBioExport12::WriteSurface(XMLElement& el, FEItemListBuilder* pl)
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

//-----------------------------------------------------------------------------

void FEBioExport12::WriteOutputSection()
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
		for (int i = 0; i<N; ++i) if (plt.PlotVariable(i).isActive()) na++;

		if (na > 0)
		{
			m_xml.add_branch(p);
			{
				for (int i = 0; i<N; ++i)
				{
					FEPlotVariable& v = plt.PlotVariable(i);
					if (v.isShown() && v.isActive())
					{
						XMLElement e;
						e.name("var");
						e.add_attribute("type", v.name());
						m_xml.add_empty(e);
					}
				}
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

void FEBioExport12::WriteStepSection()
{
	// we've already written the initial step
	// so now we simply output all the analysis steps
	for (int i = 1; i<m_pfem->Steps(); ++i)
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

void FEBioExport12::WriteConstraintSection(FEStep &s)
{
	const char* szbc[6] = { "trans_x", "trans_y", "trans_z", "rot_x", "rot_y", "rot_z" };

	for (int i = 0; i<s.RigidConstraints(); ++i)
	{
		FERigidConstraint* ps = s.RigidConstraint(i);

		GMaterial* pgm = m_pfem->GetMaterialFromID(ps->GetMaterialID());
		if (pgm == 0) throw MissingRigidBody(ps->GetName());
		FERigidMaterial* pm = dynamic_cast<FERigidMaterial*>(pgm->GetMaterialProperties());
		if (pm == 0) throw InvalidMaterialReference();

		// see if any DOF's are active
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
						XMLElement el(szbc[j]);
						el.add_attribute("type", "fixed");
						m_xml.add_leaf(el);
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
				XMLElement el;
				el.name(szbc[rc->GetDOF()]);
				el.add_attribute("type", "prescribed");
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
				XMLElement el;
				el.name(szbc[rc->GetDOF()]);
				el.add_attribute("type", "force");
				el.add_attribute("lc", rc->GetLoadCurve()->GetID());
				el.value(rc->GetValue());
				m_xml.add_leaf(el);
			}
			m_xml.close_branch();
		}
	}
}
