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

#include <cstring>
#include "stdafx.h"
#include "FEBioFormat.h"
#include "FEBioImport.h"
#include <GeomLib/GMeshObject.h>
#include <FEMLib/FEDiscreteMaterial.h>
#include <MeshTools/FEProject.h>

#ifndef WIN32
#define stricmp strcmp
#endif

//-----------------------------------------------------------------------------
int GetDOFCode(const char* sz)
{
	int dof = 0;
	const char* ch = sz;
	char szdof[8] = { 0 }, *c = szdof;
	do
	{
		if ((*ch == ',') || (*ch == 0))
		{
			*c = 0;
			if      (strcmp(szdof, "x" ) == 0) dof |= 1;			// = 1
			else if (strcmp(szdof, "y" ) == 0) dof |= (1 << 1);		// = 2
			else if (strcmp(szdof, "z" ) == 0) dof |= (1 << 2);		// = 4
			else if (strcmp(szdof, "u" ) == 0) dof |= (1 << 3);		// = 8
			else if (strcmp(szdof, "v" ) == 0) dof |= (1 << 4);		// = 16
			else if (strcmp(szdof, "w" ) == 0) dof |= (1 << 5);		// = 32
			else if (strcmp(szdof, "T" ) == 0) dof |= (1 << 6);		// = 64
			else if (strcmp(szdof, "p" ) == 0) dof |= (1 << 7);		// = 128
			else if (strcmp(szdof, "wx") == 0) dof |= (1 << 8);		// = 256
			else if (strcmp(szdof, "wy") == 0) dof |= (1 << 9);		// = 512
			else if (strcmp(szdof, "wz") == 0) dof |= (1 << 10);	// = 1024
			else if (strcmp(szdof, "ef") == 0) dof |= (1 << 11);	// = 2048
			else if (strcmp(szdof, "sx") == 0) dof |= (1 << 12);	// = 4096
			else if (strcmp(szdof, "sy") == 0) dof |= (1 << 13);	// = 8192
			else if (strcmp(szdof, "sz") == 0) dof |= (1 << 14);	// = 16384
			else if (strcmp(szdof, "c" ) == 0) dof |= (1 << 15);
			else if (strcmp(szdof, "c1") == 0) dof |= (1 << 15);
			else if (strcmp(szdof, "c2") == 0) dof |= (1 << 16);
			else if (strcmp(szdof, "c3") == 0) dof |= (1 << 17);
			else if (strcmp(szdof, "c4") == 0) dof |= (1 << 18);
			else if (strcmp(szdof, "c5") == 0) dof |= (1 << 29);
			else if (strcmp(szdof, "c6") == 0) dof |= (1 << 20);
			else
			{
				assert(false);
			}

			c = szdof;
			if (*ch != 0) ch++; else ch = 0;
		}
		else *c++ = *ch++;
	} while (ch);

	return dof;
}

//-----------------------------------------------------------------------------
FEBioFormat::FEBioFormat(FEBioImport* fileReader, FEBioModel& febio) : m_febio(febio)
{
	m_fileReader = fileReader;

	m_pstep = 0;
	m_pBCStep = febio.GetFEModel().GetStep(0);

	m_geomOnly = false;

	m_nAnalysis = -1;
}

FEBioFormat::~FEBioFormat()
{
}

void FEBioFormat::SetGeometryOnlyFlag(bool b)
{
	m_geomOnly = b;
}

void FEBioFormat::ParseUnknownTag(XMLTag& tag)
{
	m_fileReader->ParseUnknownTag(tag);
}

void FEBioFormat::ParseUnknownAttribute(XMLTag& tag, const char* szatt)
{
	m_fileReader->ParseUnknownAttribute(tag, szatt);
}

//-----------------------------------------------------------------------------
//! Create a new step
FEAnalysisStep* FEBioFormat::NewStep(FEModel& fem, int nanalysis, const char* szname)
{
	FEAnalysisStep* pstep = 0;
	switch (nanalysis)
	{
	case FE_STEP_MECHANICS        : pstep = new FENonLinearMechanics (&fem); break;
	case FE_STEP_HEAT_TRANSFER    : pstep = new FEHeatTransfer       (&fem); break;
	case FE_STEP_BIPHASIC         : pstep = new FENonLinearBiphasic  (&fem); break;
	case FE_STEP_BIPHASIC_SOLUTE : pstep = new FEBiphasicSolutes    (&fem); break;
	case FE_STEP_MULTIPHASIC      : pstep = new FEMultiphasicAnalysis(&fem); break;
	case FE_STEP_FLUID            : pstep = new FEFluidAnalysis      (&fem); break;
    case FE_STEP_FLUID_FSI        : pstep = new FEFluidFSIAnalysis   (&fem); break;
	case FE_STEP_REACTION_DIFFUSION : pstep = new FEReactionDiffusionAnalysis(&fem); break;
	default:
		pstep = new FENonLinearMechanics(&fem);
		FileReader()->AddLogEntry("Unknown step type. Creating Structural Mechanics step");
	}
	assert(pstep);
	if (pstep)
	{
		if ((szname == 0) || (strlen(szname) == 0))
		{
			char sz[256] = { 0 };
			sprintf(sz, "Step%02d", fem.Steps() + 1);
			pstep->SetName(sz);
		}
		else pstep->SetName(szname);
		fem.AddStep(pstep);
	}
	return pstep;
}

//-----------------------------------------------------------------------------
bool FEBioFormat::ReadChoiceParam(Param& p, XMLTag& tag)
{
	const char* szval = tag.szvalue();

	// see if the value string matches an enum string
	int n = 0;
	const char* sz = nullptr;
	while (sz = p.GetEnumName(n))
	{
		if (strcmp(szval, sz) == 0)
		{
			p.SetIntValue(n - p.GetOffset());
			return true;
		}
		n++;
	}

	// it wasn't a string. Let's assume it was a number
	tag.value(n); p.SetIntValue(n - p.GetOffset());

	return true;
}

//-----------------------------------------------------------------------------
// read a parameter from file
bool FEBioFormat::ReadParam(ParamContainer& PC, XMLTag& tag)
{
    // check if parameter is indexed by looking for tag attributes other than "lc"
    const char* szi = 0;
    int idx = 0;
    for (int i=0; i<tag.m_natt; ++i) {
        if (strcmp(tag.m_att[i].m_sztag, "lc") != 0) {
            szi = tag.m_att[i].m_sztag;
            idx = atoi(tag.m_att[i].m_szval);
            break;
        }
    }

	// try to find the parameter
    Param* pp = 0;
    if (szi) pp = PC.GetParam(tag.Name(), szi, idx);
    if (pp == 0) pp = PC.GetParam(tag.Name());

	if (pp == 0) return false;

	assert(pp->IsReadWrite());

	FEBioModel& febio = GetFEBioModel();

	// read (optional) load curve
	XMLAtt* pa = tag.AttributePtr("lc");
	if (pa)
	{
		int lc = pa->value<int>();
		febio.AddParamCurve(pp, lc - 1);
	}

	// check for type attribute
	XMLAtt* atype = tag.AttributePtr("type");
	if (atype == nullptr)
	{
		// read parameter value
		switch (pp->GetParamType())
		{
		case Param_INT: { int n; tag.value(n); pp->SetIntValue(n); } break;
		case Param_CHOICE: ReadChoiceParam(*pp, tag); break;
		case Param_BOOL: { int n; tag.value(n); pp->SetBoolValue(n == 1); } break;
		case Param_VEC3D: { vec3d v; tag.value(v); pp->SetVec3dValue(v); } break;
		case Param_VEC2I: { vec2i v; tag.value(v); pp->SetVec2iValue(v); } break;
		case Param_MAT3D: { mat3d v; tag.value(v); pp->SetMat3dValue(v); } break;
		case Param_FLOAT: { double d; tag.value(d); pp->SetFloatValue(d); } break;
		case Param_MATH: { string s; tag.value(s); pp->SetMathString(s); } break;
		case Param_STRING: { string s; tag.value(s); pp->SetStringValue(s); } break;
		default:
			assert(false);
			return false;
		}
	}
	else if (*atype == "math")
	{
		if (pp->IsVariable())
		{
			pp->SetParamType(Param_MATH);
			pp->SetMathString(tag.szvalue());
		}
	}
	else if (*atype == "map")
	{
		if (pp->IsVariable())
		{
			pp->SetParamType(Param_STRING);
			pp->SetStringValue(tag.szvalue());
		}
	}

	// if parameter is checkable, mark it as checked
	if (pp->IsCheckable()) pp->SetChecked(true);

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat::ReadParameters(ParamContainer& PC, XMLTag& tag)
{
	if (tag.isleaf()) return;

	// read parameters
	++tag;
	do
	{
		// try to read the parameters
		if (ReadParam(PC, tag) == false) ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());
}

//=============================================================================
//
//                                C O N T R O L
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function parses the control section from the xml file
//
bool FEBioFormat::ParseControlSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	// initialize default settings
	STEP_SETTINGS ops; ops.Defaults();
	ops.bauto = false;
	int nmplc = -1;
	ops.nanalysis = -1;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	// create a new analysis step from these control settings
	if (m_pstep == 0) m_pstep = NewStep(fem, m_nAnalysis);
	FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(m_pstep);
	assert(pstep);

	// The default in FEBio3 for rhoi is -2, for solid mechanics models
	if (pstep->GetType() == FE_STEP_MECHANICS)
	{
		Param* p = pstep->GetParam("rhoi"); assert(p);
		if (p) p->SetFloatValue(-2.0);
	}

	// parse the settings
	++tag;
	do
	{
		// This flag needs to be read into the FEBioModel.
		// The problem is that this flag is defined in the multi-phasic analysis step
		// so we need to read this flag before the usual parameter processing
		if (tag == "shell_normal_nodal")
		{
			tag.value(febio.m_shellNodalNormals);
		}

		if (ReadParam(*pstep, tag) == false)
		{
			if (tag == "title") tag.value(ops.sztitle);
			else if (tag == "time_steps") tag.value(ops.ntime);
			else if (tag == "final_time") tag.value(ops.tfinal);
			else if (tag == "step_size") tag.value(ops.dt);
			else if (tag == "max_refs") tag.value(ops.maxref);
			else if (tag == "max_ups")
			{
				tag.value(ops.ilimit);
				if (ops.ilimit == 0)
				{
					ops.mthsol = 1;
					ops.ilimit = 10;
				}
			}
			else if (tag == "time_stepper")
			{
				ops.bauto = true;
				++tag;
				do
				{
					if (tag == "dtmin") tag.value(ops.dtmin);
					else if (tag == "dtmax")
					{
						tag.value(ops.dtmax);
						XMLAtt* pa = tag.AttributePtr("lc");
						if (pa)
						{
							pa->value(nmplc);
						}
					}
					else if (tag == "max_retries") tag.value(ops.mxback);
					else if (tag == "opt_iter") tag.value(ops.iteopt);
					else if (tag == "aggressiveness") tag.value(ops.ncut);
					else ParseUnknownTag(tag);

					++tag;
				} while (!tag.isend());
			}
			else if (tag == "plot_level")
			{
				char sz[256]; tag.value(sz);
				ops.plot_level = FE_PLOT_MAJOR_ITRS;
				if      (strcmp(sz, "PLOT_NEVER"        ) == 0) ops.plot_level = FE_PLOT_NEVER;
				else if (strcmp(sz, "PLOT_MAJOR_ITRS"   ) == 0) ops.plot_level = FE_PLOT_MAJOR_ITRS;
				else if (strcmp(sz, "PLOT_MINOR_ITRS"   ) == 0) ops.plot_level = FE_PLOT_MINOR_ITRS;
				else if (strcmp(sz, "PLOT_MUST_POINTS"  ) == 0) ops.plot_level = FE_PLOT_MUST_POINTS;
				else if (strcmp(sz, "PLOT_FINAL"        ) == 0) ops.plot_level = FE_PLOT_FINAL;
				else if (strcmp(sz, "PLOT_AUGMENTATIONS") == 0) ops.plot_level = FE_PLOT_AUGMENTS;
				else if (strcmp(sz, "PLOT_STEP_FINAL"   ) == 0) ops.plot_level = FE_PLOT_STEP_FINAL;
				else
				{
					FileReader()->AddLogEntry("unknown plot_level (line %d)", tag.currentLine());
				}
			}
			else if (tag == "analysis")
			{
				XMLAtt& att = tag.Attribute("type");
				if (att == "static") ops.nanalysis = FE_STATIC;
				else if (att == "steady-state") ops.nanalysis = FE_STATIC;
				else if (att == "dynamic") ops.nanalysis = FE_DYNAMIC;
				else if (att == "transient") ops.nanalysis = FE_DYNAMIC;
				else FileReader()->AddLogEntry("unknown type in analysis. Assuming static analysis (line %d)", tag.currentLine());
			}
			else if (tag == "alpha") {
				tag.value(ops.alpha); ops.override_rhoi = true;
			}
			else if (tag == "beta") tag.value(ops.beta);
			else if (tag == "gamma") tag.value(ops.gamma);
			else if (tag == "optimize_bw") tag.value(ops.bminbw);
			else if (tag == "symmetric_biphasic")
			{
				int nval; tag.value(nval);
				if (nval == 1) ops.nmatfmt = 1; else ops.nmatfmt = 2;
			}
			else if (tag == "symmetric_stiffness")
			{
				int nval; tag.value(nval);
				if (nval == 1) ops.nmatfmt = 1; else ops.nmatfmt = 2;
			}
			else if (tag == "diverge_reform") tag.value(ops.bdivref);
			else if (tag == "reform_each_time_step") tag.value(ops.brefstep);
			else if (tag == "plot_stride") tag.value(ops.plot_stride);
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());

	// check the analysis flag
	if (ops.nanalysis == -1)
	{
		// default analysis depends on step type
		int ntype = m_pstep->GetType();
		if ((ntype == FE_STEP_BIPHASIC) || (ntype == FE_STEP_BIPHASIC_SOLUTE) || (ntype == FE_STEP_MULTIPHASIC) || (ntype == FE_STEP_FLUID) || (ntype == FE_STEP_FLUID_FSI)) ops.nanalysis = FE_DYNAMIC;
		else ops.nanalysis = FE_STATIC;
	}

	// check if final_time was set on import
	if ((ops.tfinal > 0) && (ops.dt > 0)) ops.ntime = (int)(ops.tfinal / ops.dt);

	// copy settings
	pstep->GetSettings() = ops;
	if (nmplc >= 0)
	{
		STEP_SETTINGS& ops = pstep->GetSettings();
		ops.bmust = true;
		FELoadCurve* plc = pstep->GetMustPointLoadCurve();
		febio.AddParamCurve(plc, nmplc - 1);
	}
	else ops.bmust = false;

	return true;
}

//=============================================================================
//
//                                G L O B A L
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function reads the global variables from the xml file
//
bool FEBioFormat::ParseGlobalsSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEModel& fem = GetFEModel();

	++tag;
	do
	{
		if (tag == "Constants")
		{
			double v;
			++tag;
			do
			{
				tag.value(v);
				const char* sz = tag.Name();
				Param* p = fem.GetParam(sz);
				if (p) p->SetFloatValue(v);
				++tag;
			} while (!tag.isend());
		}
		else if (tag == "Solutes")
		{
			// clear solutes (TODO: I don't think this is necessary)
			fem.ClearSolutes();

			++tag;
			do
			{
				if (tag == "solute")
				{
					int id = tag.AttributeValue<int>("id", 0) - 1;
					const char* sz = tag.Attribute("name").m_szval;
					int z = 0;
					double M = 1;
					double d = 1;
					if (tag.isempty() == false)
					{
						++tag;
						do
						{
							if (tag == "charge_number") tag.value(z);
							else if (tag == "molar_mass") tag.value(M);
							else if (tag == "density") tag.value(d);
							else ParseUnknownTag(tag);
							++tag;
						} while (!tag.isend());
					}
					fem.AddSolute(sz, z, M, d);
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());
		}
		else if (tag == "SolidBoundMolecules")
		{
			// clear solid-bound molecules (TODO: I don't think this is necessary)
			fem.ClearSBMs();

			++tag;
			do
			{
				if (tag == "solid_bound")
				{
					int id = tag.AttributeValue<int>("id", 0) - 1;
					const char* sz = tag.Attribute("name").m_szval;
					int z = 0;
					double M = 1;
					double d = 1;
					if (tag.isempty() == false)
					{
						++tag;
						do
						{
							if (tag == "charge_number") tag.value(z);
							else if (tag == "molar_mass") tag.value(M);
							else if (tag == "density") tag.value(d);
							else ParseUnknownTag(tag);
							++tag;
						} while (!tag.isend());
						fem.AddSBM(sz, z, M, d);
					}
				}
				else ParseUnknownTag(tag);
				++tag;
			} while (!tag.isend());
		}
		else ParseUnknownTag(tag);
		++tag;
	} while (!tag.isend());

	return true;
}

//=============================================================================
//
//                                M A T E R I A L
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function parses the material section from the xml file
//
bool FEBioFormat::ParseMaterialSection(XMLTag& tag)
{
	char szname[256] = { 0 };

	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel& febio = GetFEBioModel();
	FEModel& fem = GetFEModel();

	++tag;
	do
	{
		// get the material type
		XMLAtt& mtype = tag.Attribute("type");
		const char* sztype = mtype.cvalue();

		// get the material name
		XMLAtt* pan = tag.AttributePtr("name");
		if (pan) strcpy(szname, pan->cvalue());
		else sprintf(szname, "Material%02d", febio.Materials() + 1);

		// get the comment
		std::string comment = tag.comment();

		// allocate a new material
		FEMaterial* pmat = 0;

		// see if a material already exists with this name
		GMaterial* gmat = fem.FindMaterial(szname);
		if (gmat)
		{
			FileReader()->AddLogEntry("Material with name \"%s\" already exists.", szname);

			string oldName = szname;
			int n = 2;
			while (gmat)
			{
				sprintf(szname, "%s(%d)", oldName.c_str(), n++);
				gmat = fem.FindMaterial(szname);
			}
		}

		// first check special cases
		if (mtype == "rigid body") pmat = ParseRigidBody(tag);
		else
		{
			// deal with the general case
			pmat = ParseMaterial(tag, sztype);
		}

		// if pmat is set we need to add the material to the list
		gmat = new GMaterial(pmat);
		gmat->SetName(szname);
		gmat->SetInfo(comment);
		febio.AddMaterial(gmat);
		fem.AddMaterial(gmat);

		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat::ParseMatAxis(XMLTag& tag, FEMaterial* pm)
{
	FEAxisMaterial* axes = new FEAxisMaterial;

	// allow all materials to define mat_axis, even if not required for that material
	XMLAtt& atype = tag.Attribute("type");
	if (atype == "local")
	{
		axes->m_naopt = FE_AXES_LOCAL;
		tag.value(axes->m_n, 3);
	}
	else if (atype == "vector")
	{
		axes->m_naopt = FE_AXES_VECTOR;
		vec3d a(1, 0, 0), d(0, 1, 0);
		++tag;
		do
		{
			if      (tag == "a") tag.value(axes->m_a);
			else if (tag == "d") tag.value(axes->m_d);
			else ParseUnknownTag(tag);

			++tag;
		}
		while (!tag.isend());
	}
    else if (atype == "angles")
    {
        axes->m_naopt = FE_AXES_ANGLES;
        ++tag;
        do
        {
            if      (tag == "theta") tag.value(axes->m_theta);
            else if (tag == "phi") tag.value(axes->m_phi);
            else ParseUnknownTag(tag);

            ++tag;
        }
        while (!tag.isend());
    }
	else if (atype == "cylindrical")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_CYLINDRICAL;
		++tag;
		do {
			if      (tag == "center") tag.value(axes->m_center);
			else if (tag == "axis") tag.value(axes->m_axis);
			else if (tag == "vector") tag.value(axes->m_vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetAxisMaterial(axes);
	}
	else if (atype == "spherical")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_SPHERICAL;
		++tag;
		do {
			if      (tag == "center") tag.value(axes->m_center);
			else if (tag == "vector") tag.value(axes->m_vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetAxisMaterial(axes);
	}
	else ParseUnknownAttribute(tag, "type");
	++tag;

	pm->SetAxisMaterial(axes);
}

//-----------------------------------------------------------------------------
void FEBioFormat::ParseFiber(XMLTag& tag, FEMaterial* pm)
{
	// allow all materials to define mat_axis, even if not required for that material
	XMLAtt& atype = tag.Attribute("type");
	if (atype == "local")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_LOCAL;
		tag.value(axes->m_n, 3);
		pm->SetAxisMaterial(axes);
	}
	else if (atype == "vector")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_VECTOR;
		tag.value(axes->m_a);
		pm->SetAxisMaterial(axes);
	}
	else if (atype == "angles")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_ANGLES;

		++tag;
		do
		{
			if      (tag == "theta") tag.value(axes->m_theta);
			else if (tag == "phi"  ) tag.value(axes->m_phi);
			else ParseUnknownAttribute(tag, "type");
			++tag;
		} 
		while (!tag.isend());
		
		pm->SetAxisMaterial(axes);
	}
	else if (atype == "cylindrical")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_CYLINDRICAL;
		++tag;
		do {
			if (tag == "center") tag.value(axes->m_center);
			else if (tag == "axis") tag.value(axes->m_axis);
			else if (tag == "vector") tag.value(axes->m_vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetAxisMaterial(axes);
	}
	else if (atype == "spherical")
	{
		FEAxisMaterial* axes = new FEAxisMaterial;
		axes->m_naopt = FE_AXES_SPHERICAL;
		++tag;
		do {
			if      (tag == "center") tag.value(axes->m_center);
			else if (tag == "vector") tag.value(axes->m_vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetAxisMaterial(axes);
	}
	else ParseUnknownAttribute(tag, "type");
	++tag;
}

//-----------------------------------------------------------------------------
void FEBioFormat::ParseFiberProperty(XMLTag& tag, FEFiberMaterial* pm)
{
	// allow all materials to define mat_axis, even if not required for that material
	XMLAtt& atype = tag.Attribute("type");
	if (atype == "local")
	{
		int n[2] = { 0, 0 };
		tag.value(n, 2);
		pm->SetFiberGenerator(new FEFiberGeneratorLocal(n[0], n[1]));
	}
	else if (atype == "vector")
	{
		vec3d a;
		tag.value(a);
		pm->SetFiberGenerator(new FEFiberGeneratorVector(a));
	}
	else if (atype == "angles")
	{
		double theta = 0, phi = 90;
		++tag;
		do
		{
			if (tag == "theta") tag.value(theta);
			else if (tag == "phi") tag.value(phi);
			else ParseUnknownAttribute(tag, "type");
			++tag;
		} while (!tag.isend());

		pm->SetFiberGenerator(new FEAnglesVectorGenerator(theta, phi));
	}
	else if (atype == "cylindrical")
	{
		vec3d center, axis, vec;
		++tag;
		do {
			if (tag == "center") tag.value(center);
			else if (tag == "axis") tag.value(axis);
			else if (tag == "vector") tag.value(vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetFiberGenerator(new FECylindricalVectorGenerator(center, axis, vec));
	}
	else if (atype == "spherical")
	{
		vec3d center, vec;
		++tag;
		do {
			if (tag == "center") tag.value(center);
			else if (tag == "vector") tag.value(vec);
			else ParseUnknownTag(tag);
			++tag;
		} while (!tag.isend());
		pm->SetFiberGenerator(new FESphericalVectorGenerator(center, vec));
	}
	else ParseUnknownAttribute(tag, "type");
	++tag;
}

//-----------------------------------------------------------------------------
// helper function for updating uncoupled materials to ensure that the bulk modulus
// is only defined for the top-level uncoupled material
void FixUncoupledMaterial(FEMaterial* mat)
{
	if (mat->ClassID() != FE_MAT_ELASTIC_UNCOUPLED) return;

	// find the bulk-modulus parameter
	Param* pk = mat->GetParam("k"); assert(pk);
	if (pk == nullptr) return;

	// loop over all child materials
	double k = pk->GetFloatValue();
	for (int i = 0; i < mat->Properties(); ++i)
	{
		FEMaterialProperty& prop = mat->GetProperty(i);
		int n = prop.Size();
		for (int j = 0; j < n; ++j)
		{
			FEMaterial* mat_j =  prop.GetMaterial(j);
			if (mat_j && (mat_j->ClassID() == FE_MAT_ELASTIC_UNCOUPLED))
			{
				Param* pk_j = mat_j->GetParam("k"); assert(pk_j);
				if (pk_j)
				{
					FixUncoupledMaterial(mat_j);
					k += pk_j->GetFloatValue();
					pk_j->SetFloatValue(0.0);
				}
			}
		}
	}

	// assign the sum to the top-level material
	pk->SetFloatValue(k);
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseMaterial(XMLTag& tag, const char* szmat, int classId)
{
	// create a material
	FEMaterial* pm = FEMaterialFactory::Create(szmat, classId);
	if (pm == 0) 
	{
		// HACK: a little hack to read in the "EFD neo-Hookean2" materials of the old datamap plugin. 
		if (strcmp(szmat, "EFD neo-Hookean2") == 0) pm = FEMaterialFactory::Create("EFD neo-Hookean");

		if (pm == 0)
		{
			ParseUnknownAttribute(tag, "type");
			return 0;
		}
	}

	// some materials still require special handling
	int ntype = pm->Type();
	switch (ntype)
	{
	case FE_TRANS_MOONEY_RIVLIN_OLD    : return ParseTransIsoMR       (pm, tag); break;
	case FE_TRANS_VERONDA_WESTMANN_OLD : return ParseTransIsoVW       (pm, tag); break;
	case FE_BIPHASIC_SOLUTE            : return ParseBiphasicSolute   (pm, tag); break;
	case FE_TRIPHASIC_MATERIAL         : return ParseTriphasic        (pm, tag); break;
	case FE_MULTIPHASIC_MATERIAL       : return ParseMultiphasic      (pm, tag); break;
	case FE_REACTION_DIFFUSION_MATERIAL: return ParseReactionDiffusion(pm, tag); break;
	case FE_FNC1D_POINT                : return Parse1DFunction       (pm, tag); break;
    case FE_OSMO_WM                    : return ParseOsmoManning      (pm, tag); break;
	}

	// parse the material parameters
	if (pm->m_axes) pm->m_axes->m_naopt = -1;
	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (ReadParam(*pm, tag) == false)
			{
				if (tag == "mat_axis")
				{
					ParseMatAxis(tag, pm);
				}
				else if (tag == "fiber")
				{
					FETransverselyIsotropic* ptiso = dynamic_cast<FETransverselyIsotropic*>(pm);
					if (ptiso)
					{
						ParseFiberMaterial(*ptiso->GetFiberMaterial(), tag);
						++tag;
					}
					else if (dynamic_cast<FEFiberMaterial*>(pm))
					{
						FEFiberMaterial* fiberMat = dynamic_cast<FEFiberMaterial*>(pm);
						ParseFiberProperty(tag, fiberMat);
					}
					else
					{
						// treat it as mat_axis for now
						ParseFiber(tag, pm);
					}
				}
				else if(dynamic_cast<FEFiberMaterial*>(pm))
				{
					// Some fiber materials used to define the theta and phi 
					// parameters, but these are now defined via the "fiber" property. 
					// This "hack" converts from the old format to the new one. 

					FEFiberMaterial* fiberMat = dynamic_cast<FEFiberMaterial*>(pm);
					FEAnglesVectorGenerator* fiber = dynamic_cast<FEAnglesVectorGenerator*>(fiberMat->GetFiberGenerator());
					if (fiber == nullptr)
					{
						fiberMat->SetFiberGenerator(fiber = new FEAnglesVectorGenerator(0.0, 90.0));
					}

					double theta, phi;
					fiber->GetAngles(theta, phi);
					if (tag == "theta")
					{
						tag.value(theta);
					}
					else if (tag == "phi")
					{ 
						tag.value(phi);
					}
					else ParseUnknownTag(tag);
					++tag;

					fiber->SetAngles(theta, phi);
				}
				else 
				{
					if (pm->Properties() > 0)
					{
						// if this is a multi-material, this may be a component
						const char* szname = tag.AttributeValue("name", true);
						char szbuf[256] = { 0 };
						if (szname) strcpy(szbuf, szname);

						const char* sztype = tag.AttributeValue("type", true);
						if (sztype == 0) sztype = tag.Name();

						const char* sztag = tag.Name();
						FEMaterialProperty* pmc = pm->FindProperty(sztag);

						int classId = -1;
						if (pmc) classId = pmc->GetClassID();

						FEMaterial* pms = ParseMaterial(tag, sztype, classId);
						if (pms)
						{
							if (szname) pms->SetName(szbuf);

							szname = tag.Name();
							if (pmc) pmc->AddMaterial(pms);
						}
					}
					else ParseUnknownTag(tag);

					++tag;
				}
			}
			else ++tag;
		}
		while (!tag.isend());
	}

	// NOTE: As of FEBio3, the bulk-modulus of uncoupled materials must be defined at the top-level
	//       uncoupled material. However, to preserve backward compatibility, we add this little hack
	//       that sums up all the k values of the child uncoupled materials, and assigns it to the 
	//       top-level material.
	if (pm->ClassID() == FE_MAT_ELASTIC_UNCOUPLED) FixUncoupledMaterial(pm);

	return pm;
}

//-----------------------------------------------------------------------------
// This function reads the rigid body material
//
FEMaterial* FEBioFormat::ParseRigidBody(XMLTag &tag)
{
	FERigidMaterial* pm = new FERigidMaterial;
	if (tag.isleaf()) return pm;

	pm->GetParam(FERigidMaterial::MP_COM).SetBoolValue(true);

	++tag;
	do
	{
		if (tag == "center_of_mass")
		{
			vec3d r;
			tag.value(r);
			pm->SetAutoCOM(false);
			pm->SetCenterOfMass(r);
		}
		else if (ReadParam(*pm, tag) == false)
		{
			if (tag == "parent_id") tag.value(pm->m_pid);
			else ParseUnknownTag(tag);
		}
		++tag;
	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
void FEBioFormat::ParseFiberMaterial(FEOldFiberMaterial& fibermat, XMLTag& tag)
{
	FEOldFiberMaterial& fiber = fibermat;
	XMLAtt& atype = tag.Attribute("type");
	if (atype == "local")
	{
		fiber.m_naopt = FE_FIBER_LOCAL;
		tag.value(fiber.m_n, 2);
	}
	else if (atype == "cylindrical")
	{
		fiber.m_naopt = FE_FIBER_CYLINDRICAL;
		++tag;
		do
		{
			if (tag == "center") tag.value(fiber.m_r);
			if (tag == "axis"  ) tag.value(fiber.m_a);
			if (tag == "vector") tag.value(fiber.m_d);
			++tag;
		} 
		while (!tag.isend());
	}
	else if (atype == "spherical")
	{
		fiber.m_naopt = FE_FIBER_SPHERICAL;
		if (tag.isleaf())
		{
			tag.value(fiber.m_r);
			fiber.m_d = vec3d(1, 0, 0);
		}
		else
		{
			++tag;
			do
			{
				if (tag == "center") tag.value(fiber.m_r);
				if (tag == "vector") tag.value(fiber.m_d);
				++tag;
			} 
			while (!tag.isend());
		}
	}
	else if (atype == "vector")
	{
		fiber.m_naopt = FE_FIBER_VECTOR;
		tag.value(fiber.m_a);
	}
	else if (atype == "user")
	{
		fiber.m_naopt = FE_FIBER_USER;
	}
	else if (atype == "angles")
	{
		fiber.m_naopt = FE_FIBER_ANGLES;
		++tag;
		do
		{
			if      (tag == "theta") tag.value(fiber.m_theta);
			else if (tag == "phi"  ) tag.value(fiber.m_phi);
			else ParseUnknownTag(tag);
			++tag;
		} 
		while (!tag.isend());
	}
	else if (atype == "polar")
	{
		fiber.m_naopt = FE_FIBER_POLAR;
		++tag;
		do
		{
			if (tag == "center" ) tag.value(fiber.m_r);
			if (tag == "axis"   ) tag.value(fiber.m_a);
			if (tag == "vector1") tag.value(fiber.m_d0);
			if (tag == "vector2") tag.value(fiber.m_d1);
			if (tag == "radius1") tag.value(fiber.m_R0);
			if (tag == "radius2") tag.value(fiber.m_R1);
			++tag;
		} while (!tag.isend());
	}
	else
	{
		ParseUnknownAttribute(tag, "type");
	}

	// NOTE: we need to do this so the correct parameters are made visible
	fiber.UpdateData(false);
	fiber.UpdateData(true);
}


//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseTransIsoMR(FEMaterial* pmat, XMLTag& tag)
{
	FETransMooneyRivlinOld* pm = dynamic_cast<FETransMooneyRivlinOld*>(pmat);
	if (pm == 0) return 0;

	FETransMooneyRivlinOld::Fiber& f = dynamic_cast<FETransMooneyRivlinOld::Fiber&>(*pm->GetFiberMaterial());

	f.m_naopt = -1;

	FELoadCurve& ac = *f.GetParam(FETransMooneyRivlinOld::Fiber::MP_AC).GetLoadCurve();
	ac.SetID(-1);

	if (!tag.isleaf())
	{
		++tag;
		do
		{
			// read the (parent) material's parameters
			if (ReadParam(*pm, tag) == false)
			{
				// read the fiber material's parameters
				if (ReadParam(f, tag) == false)
				{
					if (tag == "fiber")
					{
						ParseFiberMaterial(f, tag);
					}
					else if (tag == "active_contraction")
					{
						ac.SetID(tag.AttributeValue<int>("lc", 0) - 1);

						double ca0 = 0, beta = 0, l0 = 0, refl = 0, ascl = 0;
						++tag;
						do
						{
							if (tag == "ca0") tag.value(ca0);
							else if (tag == "beta") tag.value(beta);
							else if (tag == "l0") tag.value(l0);
							else if (tag == "refl") tag.value(refl);
							else ParseUnknownTag(tag);
							++tag;
						} while (!tag.isend());
						f.GetParam(FETransMooneyRivlinOld::Fiber::MP_CA0).SetFloatValue(ca0);
						f.GetParam(FETransMooneyRivlinOld::Fiber::MP_BETA).SetFloatValue(beta);
						f.GetParam(FETransMooneyRivlinOld::Fiber::MP_L0).SetFloatValue(l0);
						f.GetParam(FETransMooneyRivlinOld::Fiber::MP_LREF).SetFloatValue(refl);
					}
					else ParseUnknownTag(tag);
				}
			}
			++tag;
		} 
		while (!tag.isend());
	}

	return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseTransIsoVW(FEMaterial* pmat, XMLTag& tag)
{
	FETransVerondaWestmannOld* pm = dynamic_cast<FETransVerondaWestmannOld*>(pmat);
	if (pm == 0) return 0;

	FETransVerondaWestmannOld::Fiber& f = dynamic_cast<FETransVerondaWestmannOld::Fiber&>(*pm->GetFiberMaterial());

	f.m_naopt = -1;

	FELoadCurve& ac = *f.GetParam(FETransVerondaWestmannOld::Fiber::MP_AC).GetLoadCurve();

	ac.SetID(-1);

	if (!tag.isleaf())
	{
		++tag;
		do
		{
			if (ReadParam(*pm, tag) == false)
			{
				if (ReadParam(f, tag) == false)
				{
					if (tag == "fiber") ParseFiberMaterial(f, tag);
					else if (tag == "active_contraction")
					{
						ac.SetID(tag.AttributeValue<int>("lc", 0) - 1);

						double ca0 = 0, beta = 0, l0 = 0, refl = 0;
						++tag;
						do
						{
							if (tag == "ca0") tag.value(ca0);
							else if (tag == "beta") tag.value(beta);
							else if (tag == "l0") tag.value(l0);
							else if (tag == "refl") tag.value(refl);
							++tag;
						} while (!tag.isend());
						f.GetParam(FETransVerondaWestmannOld::Fiber::MP_CA0).SetFloatValue(ca0);
						f.GetParam(FETransVerondaWestmannOld::Fiber::MP_BETA).SetFloatValue(beta);
						f.GetParam(FETransVerondaWestmannOld::Fiber::MP_L0).SetFloatValue(l0);
						f.GetParam(FETransVerondaWestmannOld::Fiber::MP_LREF).SetFloatValue(refl);
					}
					else ParseUnknownTag(tag);
				}
			}
			++tag;
		} 
		while (!tag.isend());
	}

	return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseBiphasicSolute(FEMaterial* pmat, XMLTag &tag)
{
	FEBiphasicSolute* pm = dynamic_cast<FEBiphasicSolute*>(pmat);
	if (pm == 0) return 0;

	++tag;
	do
	{
		if (ReadParam(*pm, tag) == false)
		{
			if (tag == "solid")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pme = ParseMaterial(tag, atype.cvalue());
				assert(pme);
				pm->SetSolidMaterial(pme);
				++tag;
			}
			else if (tag == "permeability")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmp = ParseMaterial(tag, atype.cvalue());
				assert(pmp);
				pm->SetPermeability(pmp);
				++tag;
			}
			else if (tag == "osmotic_coefficient")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmc = ParseMaterial(tag, atype.cvalue());
				assert(pmc);
				pm->SetOsmoticCoefficient(pmc);
				++tag;
			}
			else if (tag == "solute")
			{
				XMLAtt& asol = tag.Attribute("sol");
				int nsol; asol.value(nsol); nsol -= 1;
				FESoluteMaterial* psm = dynamic_cast<FESoluteMaterial*>(ParseMaterial(tag, "solute"));
				psm->SetSoluteIndex(nsol);
				pm->SetSoluteMaterial(psm);
				++tag;
			}
			else ParseUnknownTag(tag);
		}
		else ++tag;
	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseTriphasic(FEMaterial* pmat, XMLTag &tag)
{
	FETriphasicMaterial* pm = dynamic_cast<FETriphasicMaterial*>(pmat);
	if (pm == 0) return 0;

	// counter for number of solutes read
	int nsol = 0;

	++tag;
	do
	{
		if (ReadParam(*pm, tag) == false)
		{
			if (tag == "solid")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pme = ParseMaterial(tag, atype.cvalue());
				assert(pme);
				pm->SetSolidMaterial(pme);
				++tag;
			}
			else if (tag == "permeability")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmp = ParseMaterial(tag, atype.cvalue());
				assert(pmp);
				pm->SetPermeability(pmp);
				++tag;
			}
			else if (tag == "osmotic_coefficient")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmc = ParseMaterial(tag, atype.cvalue());
				assert(pmc);
				pm->SetOsmoticCoefficient(pmc);
				++tag;
			}
			else if (tag == "solute")
			{
				if (nsol >= 2) throw XMLReader::InvalidTag(tag);

				XMLAtt& asol = tag.Attribute("sol");
				int sid; asol.value(sid); sid -= 1;
				FESoluteMaterial* psm = dynamic_cast<FESoluteMaterial*>(ParseMaterial(tag, "solute"));
				assert(psm);
				psm->SetSoluteIndex(sid);
				pm->SetSoluteMaterial(psm, nsol);
				nsol++;
				++tag;
			}
			else ParseUnknownTag(tag);
		}
		else ++tag;
	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseMultiphasic(FEMaterial* pmat, XMLTag &tag)
{
	FEMultiphasicMaterial* pm = dynamic_cast<FEMultiphasicMaterial*>(pmat);
	if (pm == 0) return 0;

	++tag;
	do
	{
		if (ReadParam(*pm, tag) == false)
		{
			if (tag == "solid")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pme = ParseMaterial(tag, atype.cvalue());
				if (pme) pm->SetSolidMaterial(pme);
				++tag;
			}
			else if (tag == "permeability")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmp = ParseMaterial(tag, atype.cvalue());
				if (pmp) pm->SetPermeability(pmp);
				++tag;
			}
			else if (tag == "osmotic_coefficient")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEMaterial* pmc = ParseMaterial(tag, atype.cvalue());
				if (pmc) pm->SetOsmoticCoefficient(pmc);
				++tag;
			}
			else if (tag == "solute")
			{
				XMLAtt& asol = tag.Attribute("sol");
				int sid; asol.value(sid); sid -= 1;
				FESoluteMaterial* psm = dynamic_cast<FESoluteMaterial*>(ParseMaterial(tag, "solute"));
				if (psm)
				{
					psm->SetSoluteIndex(sid);
					pm->AddSoluteMaterial(psm);
				}
				++tag;
			}
			else if (tag == "solid_bound")
			{
				XMLAtt& asbm = tag.Attribute("sbm");
				int sid; asbm.value(sid); sid -= 1;
				FESBMMaterial* psb = dynamic_cast<FESBMMaterial*>(ParseMaterial(tag, "solid_bound"));
				if (psb)
				{
					psb->SetSBMIndex(sid);
					pm->AddSBMMaterial(psb);
				}
				++tag;
			}
			else if (tag == "reaction")
			{
				XMLAtt& atype = tag.Attribute("type");
				FEReactionMaterial* psr = ParseReaction(tag);
				if (psr) pm->AddReactionMaterial(psr);
				++tag;
			}
            else if (tag == "membrane_reaction")
            {
                XMLAtt& atype = tag.Attribute("type");
                FEMembraneReactionMaterial* psr = ParseMembraneReaction(tag);
                if (psr) pm->AddMembraneReactionMaterial(psr);
                ++tag;
            }
			else
			{
				ParseUnknownTag(tag);
				++tag;
			}
		}
		else ++tag;
	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseReactionDiffusion(FEMaterial* mat, XMLTag& tag)
{
	FEReactionDiffusionMaterial* pm = dynamic_cast<FEReactionDiffusionMaterial*>(mat);
	if (pm == 0) return 0;

	FEModel& fem = GetFEModel();

	++tag;
	do
	{
		if (ReadParam(*pm, tag) == false)
		{
			if (tag == "species")
			{
				XMLAtt& att = tag.Attribute("name");
				int nsol = fem.FindSolute(att.cvalue());
				FESpeciesMaterial* spec = dynamic_cast<FESpeciesMaterial*>(ParseMaterial(tag, "species"));
				assert(spec);
				spec->SetSpeciesIndex(nsol);
				pm->AddSpeciesMaterial(spec);
				++tag;
			}
			else if (tag == "solid_bound_species")
			{
				XMLAtt& att = tag.Attribute("name");
				int nsbm = fem.FindSBM(att.cvalue());
				FESolidSpeciesMaterial* spec = dynamic_cast<FESolidSpeciesMaterial*>(ParseMaterial(tag, "solid_bound_species"));
				assert(spec);
				spec->SetSBMIndex(nsbm);
				pm->AddSolidSpeciesMaterial(spec);
				++tag;
			}
			else if (tag == "reaction")
			{
				FEReactionMaterial* rm = ParseReaction2(tag);
				pm->AddReactionMaterial(rm);
				++tag;
			}
			else ParseUnknownTag(tag);
		}
		else ++tag;

	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
bool ProcessReactionEquation(FEModel& fem, FEReactionMaterial* pm, const char* szeq)
{
	if (szeq == 0) return true;

	// copy the equation, removing all white space
	char szbuf[512] = {0};
	const char* cs = szeq;
	char* cd = szbuf;
	while (*cs)
	{
		if (isspace(*cs)==0) *cd++ = *cs++;
		else cs++;
	}
	*cd = 0;

	int m = 0; // m = 0, reactants; m = 1, products

	int nu = 1;

	char* ch = szbuf;
	char* l = ch;
	while (*l)
	{
		if ((*ch=='+')||(*ch==0)||(*ch=='-'))
		{
			if (*ch!=0) *ch++ = 0;

			int nsol = fem.FindSolute(l), nsbm = -1;
			if (nsol == -1) { nsbm = fem.FindSBM(l); if (nsbm == -1) return false; }

			if (m == 0)
			{
				FEReactantMaterial* vR = new FEReactantMaterial;
				if (nsol != -1) { vR->SetReactantType(FEReactionMaterial::SOLUTE_SPECIES); vR->SetIndex(nsol); }
				else { vR->SetReactantType(FEReactionMaterial::SBM_SPECIES); vR->SetIndex(nsbm); }
				vR->SetCoeff(nu);
				pm->AddReactantMaterial(vR);
			}
			else
			{
				FEProductMaterial* vP = new FEProductMaterial;
				if (nsol != -1) { vP->SetProductType(FEReactionMaterial::SOLUTE_SPECIES); vP->SetIndex(nsol); }
				else { vP->SetProductType(FEReactionMaterial::SBM_SPECIES); vP->SetIndex(nsbm); }
				vP->SetCoeff(nu);
				pm->AddProductMaterial(vP);
			}

			if (*ch=='-') ch++;
			if (*ch=='>') { ch++; m = 1; }

			l = ch;
			nu = 1;
		}
		else if (*ch=='*')
		{
			*ch++ = 0;
			nu = atoi(l);
			l = ch;
		}
		else ++ch;
	}

	return true;
}

//-----------------------------------------------------------------------------
FEReactionMaterial* FEBioFormat::ParseReaction2(XMLTag &tag)
{
	XMLAtt* att = tag.AttributePtr("name");

	// get the material type
	XMLAtt& mtype = tag.Attribute("type");
	const char* sztype = mtype.cvalue();

	FEReactionMaterial* pm = 0;
	if (strcmp(sztype, "mass action") == 0) 
	{
		pm = new FEMassActionForward;

		++tag;
		do
		{
			if (tag == "equation")
			{
				ProcessReactionEquation(GetFEModel(), pm, tag.m_szval);
			}
			else if (tag == "rate_constant")
			{
				double k;
				tag.value(k);

				FEReactionRateConst* rc = new FEReactionRateConst;
				rc->SetRateConstant(k);
				pm->SetForwardRate(rc);
			}
			++tag;
		}
		while (!tag.isend());
	}
	else throw XMLReader::InvalidAttributeValue(tag, "type", sztype);
	
	return pm;
}

//-----------------------------------------------------------------------------
FEReactionMaterial* FEBioFormat::ParseReaction(XMLTag &tag)
{
	char szname[256] = { 0 };

	// get the material type
	XMLAtt& mtype = tag.Attribute("type");

	// get the material name
	XMLAtt* pan = tag.AttributePtr("name");
	if (pan) strcpy(szname, pan->cvalue());

	FEReactionMaterial* pm = nullptr;
	const char* sztype = mtype.cvalue();
	if (strcmp(sztype, "mass-action-forward") == 0)
		pm = new FEMassActionForward;
	else if (strcmp(sztype, "mass-action-reversible") == 0)
		pm = new FEMassActionReversible;
	else if (strcmp(sztype, "Michaelis-Menten") == 0)
		pm = new FEMichaelisMenten;
	else
	{
		assert(false);
		return nullptr;
	}

	FEReactantMaterial* psr = 0;
	FEProductMaterial* psp = 0;
	FEMaterial* pfr = 0;
	FEMaterial* prr = 0;

	pm->SetName(szname);

	++tag;
	do
	{
		if (tag == "Vbar") pm->SetOvrd(true);    // this parameter is optional

		if (ReadParam(*pm, tag) == false)
		{
			if (tag == "vR")
			{
				XMLAtt* asol = tag.AttributePtr("sol");
				XMLAtt* asbm = tag.AttributePtr("sbm");
				if (asol) {
					int sid; asol->value(sid); sid -= 1;
					psr = dynamic_cast<FEReactantMaterial*>(ParseMaterial(tag, "Reactant"));
					assert(psr);
					psr->SetIndex(sid);
					psr->SetReactantType(FEReactionMaterial::SOLUTE_SPECIES);
					ReadParam(*psr, tag);
					pm->AddReactantMaterial(psr);
				}
				else if (asbm) {
					int sid; asbm->value(sid); sid -= 1;
					psr = dynamic_cast<FEReactantMaterial*>(ParseMaterial(tag, "Reactant"));
					assert(psr);
					psr->SetIndex(sid);
					psr->SetReactantType(FEReactionMaterial::SBM_SPECIES);
					ReadParam(*psr, tag);
					pm->AddReactantMaterial(psr);
				}
				else
					assert(false);
				++tag;
			}
			else if (tag == "vP")
			{
				XMLAtt* asol = tag.AttributePtr("sol");
				XMLAtt* asbm = tag.AttributePtr("sbm");
				if (asol) {
					int sid; asol->value(sid); sid -= 1;
					psp = dynamic_cast<FEProductMaterial*>(ParseMaterial(tag, "Product"));
					assert(psp);
					psp->SetIndex(sid);
					psp->SetProductType(FEReactionMaterial::SOLUTE_SPECIES);
					ReadParam(*psp, tag);
					pm->AddProductMaterial(psp);
				}
				else if (asbm) {
					int sid; asbm->value(sid); sid -= 1;
					psp = dynamic_cast<FEProductMaterial*>(ParseMaterial(tag, "Product"));
					assert(psp);
					psp->SetIndex(sid);
					psp->SetProductType(FEReactionMaterial::SBM_SPECIES);
					ReadParam(*psp, tag);
					pm->AddProductMaterial(psp);
				}
				else
					assert(false);
				++tag;
			}
			else if (tag == "forward_rate")
			{
				XMLAtt& atype = tag.Attribute("type");
				pfr = ParseMaterial(tag, atype.cvalue());
				assert(pfr);
				pm->SetForwardRate(pfr);
				++tag;
			}
			else if (tag == "reverse_rate")
			{
				XMLAtt& atype = tag.Attribute("type");
				prr = ParseMaterial(tag, atype.cvalue());
				assert(prr);
				pm->SetReverseRate(prr);
				++tag;
			}
			else ParseUnknownTag(tag);
		}
		else ++tag;
	}
	while (!tag.isend());

	return pm;
}

//-----------------------------------------------------------------------------
FEMembraneReactionMaterial* FEBioFormat::ParseMembraneReaction(XMLTag &tag)
{
    char szname[256] = { 0 };
    
    // get the material type
    XMLAtt& mtype = tag.Attribute("type");
    
    // get the material name
    XMLAtt* pan = tag.AttributePtr("name");
    if (pan) strcpy(szname, pan->cvalue());
    
    FEMembraneReactionMaterial* pm = nullptr;
    const char* sztype = mtype.cvalue();
    if (strcmp(sztype, "membrane-mass-action-forward") == 0)
        pm = new FEMembraneMassActionForward;
    else if (strcmp(sztype, "membrane-mass-action-reversible") == 0)
        pm = new FEMembraneMassActionReversible;
    
    FEReactantMaterial* psr = 0;
    FEProductMaterial* psp = 0;
    FEInternalReactantMaterial* psri = 0;
    FEInternalProductMaterial* pspi = 0;
    FEExternalReactantMaterial* psre = 0;
    FEExternalProductMaterial* pspe = 0;
    FEMaterial* pfr = 0;
    FEMaterial* prr = 0;
    
    pm->SetName(szname);
    
    ++tag;
    do
    {
        if (tag == "Vbar") pm->SetOvrd(true);    // this parameter is optional
        
        if (ReadParam(*pm, tag) == false)
        {
            if (tag == "vR")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                XMLAtt* asbm = tag.AttributePtr("sbm");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    psr = dynamic_cast<FEReactantMaterial*>(ParseMaterial(tag, "vR"));
                    assert(psr);
                    psr->SetIndex(sid);
                    psr->SetReactantType(FEReactionMaterial::SOLUTE_SPECIES);
                    ReadParam(*psr, tag);
                    pm->AddReactantMaterial(psr);
                }
                else if (asbm) {
                    int sid; asbm->value(sid); sid -= 1;
                    psr = dynamic_cast<FEReactantMaterial*>(ParseMaterial(tag, "vR"));
                    assert(psr);
                    psr->SetIndex(sid);
                    psr->SetReactantType(FEReactionMaterial::SBM_SPECIES);
                    ReadParam(*psr, tag);
                    pm->AddReactantMaterial(psr);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "vRi")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    psri = dynamic_cast<FEInternalReactantMaterial*>(ParseMaterial(tag, "vRi"));
                    assert(psri);
                    psri->SetIndex(sid);
                    psri->SetReactantType(FEMembraneReactionMaterial::INT_SPECIES);
                    ReadParam(*psri, tag);
                    pm->AddInternalReactantMaterial(psri);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "vRe")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    psre = dynamic_cast<FEExternalReactantMaterial*>(ParseMaterial(tag, "vRe"));
                    assert(psre);
                    psre->SetIndex(sid);
                    psre->SetReactantType(FEMembraneReactionMaterial::EXT_SPECIES);
                    ReadParam(*psre, tag);
                    pm->AddExternalReactantMaterial(psre);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "vP")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                XMLAtt* asbm = tag.AttributePtr("sbm");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    psp = dynamic_cast<FEProductMaterial*>(ParseMaterial(tag, "vP"));
                    assert(psp);
                    psp->SetIndex(sid);
                    psp->SetProductType(FEReactionMaterial::SOLUTE_SPECIES);
                    ReadParam(*psp, tag);
                    pm->AddProductMaterial(psp);
                }
                else if (asbm) {
                    int sid; asbm->value(sid); sid -= 1;
                    psp = dynamic_cast<FEProductMaterial*>(ParseMaterial(tag, "vP"));
                    assert(psp);
                    psp->SetIndex(sid);
                    psp->SetProductType(FEReactionMaterial::SBM_SPECIES);
                    ReadParam(*psp, tag);
                    pm->AddProductMaterial(psp);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "vPi")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    pspi = dynamic_cast<FEInternalProductMaterial*>(ParseMaterial(tag, "vPi"));
                    assert(pspi);
                    pspi->SetIndex(sid);
                    pspi->SetProductType(FEMembraneReactionMaterial::INT_SPECIES);
                    ReadParam(*pspi, tag);
                    pm->AddInternalProductMaterial(pspi);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "vPe")
            {
                XMLAtt* asol = tag.AttributePtr("sol");
                if (asol) {
                    int sid; asol->value(sid); sid -= 1;
                    pspe = dynamic_cast<FEExternalProductMaterial*>(ParseMaterial(tag, "vPe"));
                    assert(pspe);
                    pspe->SetIndex(sid);
                    pspe->SetProductType(FEMembraneReactionMaterial::EXT_SPECIES);
                    ReadParam(*pspe, tag);
                    pm->AddExternalProductMaterial(pspe);
                }
                else
                    assert(false);
                ++tag;
            }
            else if (tag == "forward_rate")
            {
                XMLAtt& atype = tag.Attribute("type");
                pfr = ParseMaterial(tag, atype.cvalue());
                assert(pfr);
                pm->SetForwardRate(pfr);
                ++tag;
            }
            else if (tag == "reverse_rate")
            {
                XMLAtt& atype = tag.Attribute("type");
                prr = ParseMaterial(tag, atype.cvalue());
                assert(prr);
                pm->SetReverseRate(prr);
                ++tag;
            }
            else ParseUnknownTag(tag);
        }
        else ++tag;
    }
    while (!tag.isend());
    
    return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::ParseOsmoManning(FEMaterial* pmat, XMLTag& tag)
{
    FEOsmoWellsManning* pm = dynamic_cast<FEOsmoWellsManning*>(pmat);
    if (pm == 0) return 0;
    
    double ksi = 0;
    int coion = -1;
    
    ++tag;
    do
    {
        if (tag == "ksi") tag.value(ksi);
        else if (tag == "co_ion") tag.value(coion);
        else ParseUnknownTag(tag);
        ++tag;
    } while (!tag.isend());
    
    pm->GetParam(FEOsmoWellsManning::MP_KSI).SetFloatValue(ksi);
    pm->SetCoIonIndex(coion-1);

    return pm;
}

//-----------------------------------------------------------------------------
FEMaterial* FEBioFormat::Parse1DFunction(FEMaterial* pm, XMLTag& tag)
{
	FE1DPointFunction* fnc = dynamic_cast<FE1DPointFunction*>(pm);
	if (fnc == nullptr) return 0;

	FELoadCurve* plc = fnc->GetPointCurve();
	plc->Clear();

	++tag;
	do
	{
		if (tag == "interpolate")
		{
			const char* szval = tag.szvalue();
			if (stricmp(szval, "smooth") == 0) plc->SetType(FELoadCurve::LC_SMOOTH);
			if (stricmp(szval, "linear") == 0) plc->SetType(FELoadCurve::LC_LINEAR);
			if (stricmp(szval, "step"  ) == 0) plc->SetType(FELoadCurve::LC_STEP);
            if (stricmp(szval, "cubic spline" ) == 0) plc->SetType(FELoadCurve::LC_CSPLINE);
            if (stricmp(szval, "control point") == 0) plc->SetType(FELoadCurve::LC_CPOINTS);
            if (stricmp(szval, "approximation") == 0) plc->SetType(FELoadCurve::LC_APPROX);
		}
		else if (tag == "extend")
		{
			const char* szval = tag.szvalue();
			if (stricmp(szval, "constant"     ) == 0) plc->SetExtend(FELoadCurve::EXT_CONSTANT);
			if (stricmp(szval, "extrapolate"  ) == 0) plc->SetExtend(FELoadCurve::EXT_EXTRAPOLATE);
			if (stricmp(szval, "repeat"       ) == 0) plc->SetExtend(FELoadCurve::EXT_REPEAT);
			if (stricmp(szval, "repeat offset") == 0) plc->SetExtend(FELoadCurve::EXT_REPEAT_OFFSET);
		}
		else if (tag == "points")
		{
			++tag;
			do
			{
				double d[2] = { 0.0, 0.0 };
				tag.value(d, 2);
				plc->Add(d[0], d[1]);
				++tag;
			}
			while (!tag.isend());
		}
		++tag;
	}
	while (!tag.isend());
    plc->Update();
	return pm;
}

//=============================================================================
//
//                                L O A D D A T A
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function reads the load data section from the xml file
//
bool FEBioFormat::ParseLoadDataSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	FEBioModel &febio = GetFEBioModel();

	// read all loadcurves
	++tag;
	do
	{
		if (tag == "loadcurve")
		{
			// create the loadcurve
			FELoadCurve lc;

			// remove default points
			lc.Clear();

			// get the load curve ID
			int nid = tag.Attribute("id").value<int>();

			// set the interpolation type
			XMLAtt* pat = tag.AttributePtr("type");
			if (pat)
			{
				if (*pat == "step") lc.SetType(FELoadCurve::LC_STEP);
				else if (*pat == "linear") lc.SetType(FELoadCurve::LC_LINEAR);
				else if (*pat == "smooth") lc.SetType(FELoadCurve::LC_SMOOTH);
                else if (*pat == "cubic spline") lc.SetType(FELoadCurve::LC_CSPLINE);
                else if (*pat == "control points") lc.SetType(FELoadCurve::LC_CPOINTS);
                else if (*pat == "approximation") lc.SetType(FELoadCurve::LC_APPROX);
				else FileReader()->AddLogEntry("unknown type for loadcurve %d (line %d)", nid, tag.m_nstart_line);
			}
			else lc.SetType(FELoadCurve::LC_LINEAR);

			// set the extend mode
			XMLAtt* pae = tag.AttributePtr("extend");
			if (pae)
			{
				if (*pae == "constant") lc.SetExtend(FELoadCurve::EXT_CONSTANT);
				else if (*pae == "extrapolate") lc.SetExtend(FELoadCurve::EXT_EXTRAPOLATE);
				else if (*pae == "repeat") lc.SetExtend(FELoadCurve::EXT_REPEAT);
				else if (*pae == "repeat offset") lc.SetExtend(FELoadCurve::EXT_REPEAT_OFFSET);
				else FileReader()->AddLogEntry("unknown extend mode for loadcurve %d (line %d)", nid, tag.m_nstart_line);
			}

			// read the points
			double d[2];
			++tag;
			do
			{
				tag.value(d, 2);

				LOADPOINT pt;
				pt.time = d[0];
				pt.load = d[1];
				lc.Add(pt);

				++tag;
			} while (!tag.isend());

			febio.AddLoadCurve(lc);
		}
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	return true;
}

//=============================================================================
//
//                                O U T P U T
//
//=============================================================================

//-----------------------------------------------------------------------------
//  This function parses the Output section
bool FEBioFormat::ParseOutputSection(XMLTag& tag)
{
	// make sure the section is not empty
	if (tag.isleaf()) return true;

	++tag;
	do
	{
		if (tag == "logfile") ParseLogfileSection(tag);
		else if (tag == "plotfile") ParsePlotfileSection(tag);
		else ParseUnknownTag(tag);

		++tag;
	} while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFormat::ParsePlotfileSection(XMLTag &tag)
{
	FEBioModel &fem = GetFEBioModel();

	XMLAtt* pat = tag.AttributePtr("type");
	if ((pat == 0) || ((*pat != "febio") && (*pat != "febio2"))) { ParseUnknownAttribute(tag, "type"); return true; }
	if (tag.isleaf())
	{
		// add the default ones
		fem.AddPlotVariable(FEBioModel::PlotVariable("displacement"));
		fem.AddPlotVariable(FEBioModel::PlotVariable("stress"));
		return true;
	}

	++tag;
	do
	{
		if (tag == "var")
		{
			XMLAtt& avar = tag.Attribute("type");
			const char* szsurf = tag.AttributeValue("surface", true);
			if (szsurf) 
			{
				fem.AddPlotVariable(FEBioModel::PlotVariable(avar.cvalue(), szsurf, DOMAIN_SURFACE));
			}
			else
			{
				fem.AddPlotVariable(FEBioModel::PlotVariable(avar.cvalue()));
			}
		}
		else if (tag == "compression")
		{
			// ignore this tag
		}
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());

	return true;
}

//-----------------------------------------------------------------------------
bool FEBioFormat::ParseLogfileSection(XMLTag &tag)
{
	static int n = 1;

	FEBioModel &fem = GetFEBioModel();
	
	++tag;
	do
	{
		if (tag == "node_data")
		{
			const char* szdata = tag.AttributeValue("data", true);
			if (szdata == 0) szdata = "";

			FEBioModel::LogVariable logVar = FEBioModel::LogVariable(FELogData::LD_NODE, szdata);

			const char* szfile = tag.AttributeValue("file", true);
			if (szfile) logVar.setFile(szfile);

			const char* szset = tag.AttributeValue("node_set", true);
			if (szset)
			{
				FENodeSet* nset = fem.BuildFENodeSet(szset);
				if (nset)
				{
					GObject* po = nset->GetGObject();
					po->AddFENodeSet(nset);
					logVar.SetGroupID(nset->GetID());
				}
			}
			else
			{
				// read the node list
				vector<int> l;
				tag.value(l);

				if (l.empty() == false)
				{
					for (int i = 0; i < l.size(); ++i) l[i] -= 1;

					// create a new node set for this
					FEBioModel::PartInstance* inst = fem.GetInstance(0);
					GMeshObject* po = inst->GetGObject();
					FEMesh* pm = po->GetFEMesh();

					char sz[32] = { 0 };
					sprintf(sz, "nodeset%02d", po->FENodeSets() + 1);
					FENodeSet* ps = new FENodeSet(po, l);
					ps->SetName(sz);
					po->AddFENodeSet(ps);

					logVar.SetGroupID(ps->GetID());
				}
			}
			fem.AddLogVariable(logVar);
		}
		else if (tag == "element_data")
		{
			const char* szdata = tag.AttributeValue("data", true);
			if (szdata == 0) szdata = "";

			FEBioModel::LogVariable logVar = FEBioModel::LogVariable(FELogData::LD_ELEM, szdata);

			const char* szfile = tag.AttributeValue("file", true);
			if (szfile) logVar.setFile(szfile);

			const char* szset = tag.AttributeValue("elem_set", true);
			if (szset)
			{
				FEPart* pg = fem.BuildFEPart(szset);
				if (pg)
				{
					GObject* po = pg->GetGObject();
					po->AddFEPart(pg);
					logVar.SetGroupID(pg->GetID());
				}
			}
			else if (tag.isempty() == false)
			{
				// read the element list
				vector<int> l;
				tag.value(l);

				if (l.empty() == false)
				{
					for (int i = 0; i < l.size(); ++i) l[i] -= 1;

					// create a new element set for this
					FEBioModel::PartInstance* inst = fem.GetInstance(0);
					GMeshObject* po = inst->GetGObject();

					char sz[32] = { 0 };
					sprintf(sz, "elementset%02d", po->FEParts() + 1);
					FEPart* ps = new FEPart(po, l);
					ps->SetName(sz);
					po->AddFEPart(ps);

					logVar.SetGroupID(ps->GetID());
				}
			}

			fem.AddLogVariable(logVar);
		}
		else if (tag == "rigid_body_data")
		{
			const char* szdata = tag.AttributeValue("data", true);
			if (szdata == 0) szdata = "";
			fem.AddLogVariable(FEBioModel::LogVariable(FELogData::LD_RIGID, szdata));
		}
        else if (tag == "rigid_connector_data")
        {
            const char* szdata = tag.AttributeValue("data", true);
            if (szdata == 0) szdata = "";
            fem.AddLogVariable(FEBioModel::LogVariable(FELogData::LD_CNCTR, szdata));
        }
		else ParseUnknownTag(tag);
		++tag;
	}
	while (!tag.isend());


	return true;
}

//-----------------------------------------------------------------------------
void FEBioFormat::ParseMappedParameter(XMLTag& tag, Param* param)
{
	const char* sztype = tag.AttributeValue("type", true);
	if (sztype && (strcmp(sztype, "map") == 0))
	{
		param->SetParamType(Param_STRING);
		param->SetStringValue(tag.szvalue());
	}
	else if (sztype && (strcmp(sztype, "math") == 0))
	{
		param->SetParamType(Param_MATH);
		string smath;
		if (tag.isleaf()) smath = tag.szvalue();
		else
		{
			++tag;
			do
			{
				if (tag == "math") smath = tag.szvalue();
				++tag;
			} while (!tag.isend());
		}
		param->SetMathString(smath);
	}
	else
	{
		double scale;
		tag.value(scale);
		param->SetParamType(Param_FLOAT);
		param->SetFloatValue(scale);
	}
}
