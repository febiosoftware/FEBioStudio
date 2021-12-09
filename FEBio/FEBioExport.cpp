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
#include "FEBioExport.h"
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FELoad.h>
#include <FEMLib/FEModelConstraint.h>
#include <FEMLib/FERigidLoad.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>
#include <MeshTools/FEProject.h>
#include <sstream>

//=============================================================================

template <> std::string type_to_string<vec2i>(const vec2i& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y;
	return ss.str();
}

template <> std::string type_to_string<vec2f>(const vec2f& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y;
	return ss.str();
}

template <> std::string type_to_string<vec2d>(const vec2d& v)
{
	std::stringstream ss;
	ss << v.x() << "," << v.y();
	return ss.str();
}

template <> std::string type_to_string<vec3f>(const vec3f& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y << "," << v.z;
	return ss.str();
}

template <> std::string type_to_string<vec3d>(const vec3d& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y << "," << v.z;
	return ss.str();
}

template <> std::string type_to_string<quatd>(const quatd& v)
{
	std::stringstream ss;
	ss << v.x << "," << v.y << "," << v.z << "," << v.w;
	return ss.str();
}

template <> std::string type_to_string<mat3ds>(const mat3ds& v)
{
	std::stringstream ss;
	ss << v.xx() << "," << v.yy() << "," << v.zz() << "," << v.xy() << "," << v.yz() << "," << v.xz();
	return ss.str();
}

template <> std::string type_to_string<mat3d>(const mat3d& v)
{
	std::stringstream ss;
	ss << v[0][0] << "," << v[0][1] << "," << v[0][2] << ",";
	ss << v[1][0] << "," << v[1][1] << "," << v[1][2] << ",";
	ss << v[2][0] << "," << v[2][1] << "," << v[2][2];
	return ss.str();
}

//=============================================================================
FEBioExport::FEBioExport(FEProject& prj) : FEFileExport(prj)
{
	m_compress = false;
	m_exportSelections = false;
	m_exportEnumStrings = false;
	m_exportNonPersistentParams = true;

	// initialize section flags
	for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) m_section[i] = true;
}

void FEBioExport::SetPlotfileCompressionFlag(bool b)
{
	m_compress = b;
}

void FEBioExport::SetExportSelectionsFlag(bool b)
{
	m_exportSelections = b;
}

//-----------------------------------------------------------------------------
XMLWriter& FEBioExport::GetXMLWriter()
{
	return m_xml;
}

//-----------------------------------------------------------------------------
const char* FEBioExport::GetEnumValue(Param& p)
{
	assert(p.GetParamType() == Param_CHOICE);
	FSModel& fem = m_prj.GetFSModel();
	return fem.GetEnumValue(p.GetEnumNames(), p.GetIntValue());
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteParam(Param &p)
{
	// don't export hidden parameters
	if (p.IsReadWrite() == false) return;

	// see if we are writing non-persistent parameters
	if ((m_exportNonPersistentParams==false) && (p.IsPersistent() == false)) return;

	// if parameter is checkable, we only write it when it's checked
	if (p.IsCheckable() && !p.IsChecked()) return;

	// get the (short) name
	const char* szname = p.GetShortName();

	// get the index data (if any)
	const char* szindex = p.GetIndexName();
	int nindex = p.GetIndexValue();

	FSModel& fem = m_prj.GetFSModel();

	// setup the xml-element
	XMLElement e;
	e.name(szname);
	if (szindex) e.add_attribute(szindex, nindex);
	FELoadCurve* plc = p.GetLoadCurve();
	if (plc && plc->Size())
	{
		assert(plc->GetID() > 0);
		e.add_attribute("lc", plc->GetID());
	}
	switch (p.GetParamType())
	{
	case Param_CHOICE: 
		{
			if (m_exportEnumStrings && (p.GetEnumNames()) && (p.GetEnumNames()[0] != '$'))
			{
				const char* sz = GetEnumValue(p);
				e.value(sz);
			}
			else if (m_exportEnumStrings && p.GetEnumNames() && (p.GetEnumNames()[0] == '$'))
			{
				const char* sz = GetEnumValue(p);
				e.value(sz);
			}
			else if (p.GetEnumNames() && (p.GetEnumNames()[0] == '$'))
			{
				int n = fem.GetEnumIntValue(p);
				e.value(n);
			}
			else
			{
				int n = p.GetIntValue() + p.GetOffset();
				e.value(n); 
			}
		}
		break;
	case Param_INT   : e.value(p.GetIntValue  ()); break;
	case Param_FLOAT : e.value(p.GetFloatValue()); break;
	case Param_BOOL  : e.value(p.GetBoolValue ()); break;
	case Param_VEC3D : e.value(p.GetVec3dValue()); break;
	case Param_VEC2I : e.value(p.GetVec2iValue()); break;
	case Param_MAT3D : e.value(p.GetMat3dValue()); break;
	case Param_MAT3DS : e.value(p.GetMat3dsValue()); break;
	case Param_STD_VECTOR_INT:
	{
		if (p.GetEnumNames())
		{
			std::vector<int> l = p.GetVectorIntValue();
			char buf[256] = { 0 };
			if (fem.GetEnumValues(buf, l, p.GetEnumNames()))
			{
				e.value(buf);
			}
			else
			{
				std::stringstream ss;
				for (int i = 0; i < l.size(); ++i)
				{
					if (i != 0) ss << ",";
					const char* sz = p.GetEnumName(l[i]); assert(sz);
					if (sz) ss << sz;
				}
				string s = ss.str();
				e.value(s.c_str());
			}
		}
		else e.value(p.GetVectorIntValue());
	}
	break;
	case Param_STD_VECTOR_DOUBLE:
	{
		e.value(p.GetVectorDoubleValue());
	}
	break;
	case Param_MATH  :
	{
		e.add_attribute("type", "math");
		std::string smath = p.GetMathString();
		e.value(smath.c_str());
	}
	break;
	case Param_STRING:
	{
		e.add_attribute("type", "map");
		std::string s = p.GetStringValue();
		e.value(s.c_str());
	}
	break;
    default:
		assert(false);
	}

	// write the xml-element
	m_xml.add_leaf(e);
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteParamList(ParamContainer& c)
{
	int NP = c.Parameters();
	for (int i = 0; i < NP; ++i)
	{
		Param& p = c.GetParam(i);
		// don't write attribute params
		if ((p.GetFlags() & 0x01) == 0) WriteParam(p);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport::AddLoadCurve(FELoadCurve* plc)
{
	assert(plc);
	m_pLC.push_back(plc);
	plc->SetID((int)m_pLC.size());
}

//-----------------------------------------------------------------------------
void FEBioExport::AddLoadCurves(ParamContainer& PC)
{
	int N = PC.Parameters();
	for (int i = 0; i<N; ++i)
	{
		Param& p = PC.GetParam(i);
		FELoadCurve* plc = p.GetLoadCurve();
		if (plc) AddLoadCurve(plc);
	}
}

//-----------------------------------------------------------------------------
void FEBioExport::Clear()
{
	m_pLC.clear();
}

//-----------------------------------------------------------------------------
void FEBioExport::WriteNote(FSObject* po)
{
	if (po)
	{
		m_xml.add_comment(po->GetInfo());
	}
}

//-----------------------------------------------------------------------------
bool FEBioExport::PrepareExport(FEProject& prj)
{
	Clear();

	FSModel& fem = prj.GetFSModel();

	// set nodal ID's
	GModel& model = fem.GetModel();
	int noff = 1;
	for (int i = 0; i<model.Objects(); ++i)
	{
		FECoreMesh* pm = model.Object(i)->GetFEMesh();
		if (pm == 0) return errf("Not all objects are meshed.");
		for (int j = 0; j<pm->Nodes(); ++j) pm->Node(j).m_nid = noff++;
	}

	// set material tags
	for (int i = 0; i<fem.Materials(); ++i) fem.GetMaterial(i)->m_ntag = i + 1;

	// build the load curve list
	BuildLoadCurveList(fem);

	return true;
}

//-----------------------------------------------------------------------------
void FEBioExport::BuildLoadCurveList(FSModel& fem)
{
	// must point load curves
	for (int j = 0; j<fem.Steps(); ++j)
	{
		FEStep* step = fem.GetStep(j);
		FEAnalysisStep* pstep = dynamic_cast<FEAnalysisStep*>(step);
		if (pstep)
		{
			if (pstep->GetSettings().bmust) AddLoadCurve(pstep->GetMustPointLoadCurve());
		}
		else
		{
			AddLoadCurves(*step);
			for (int i = 0; i < step->ControlProperties(); ++i)
			{
				FEStepControlProperty& prop = step->GetControlProperty(i);
				if (prop.m_prop) AddLoadCurves(*prop.m_prop);
			}
		}
	}

	// material curves
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pgm = fem.GetMaterial(i);
		FEMaterial* pm = pgm->GetMaterialProperties();
		if (pm)
		{
			AddLoadCurves(*pm);
			MultiMaterialCurves(pm);
		}
	}

	// see if any of the body force load curves are active
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->BCs(); ++j)
		{
			FEBoundaryCondition* pbc = pstep->BC(j);
			if (pbc && pbc->IsActive()) AddLoadCurves(*pbc);
		}
	}

	// see if any of the body force load curves are active
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Loads(); ++j)
		{
			FELoad* pl = pstep->Load(j);
			if (pl && pl->IsActive()) AddLoadCurves(*pl);
		}
	}

	// add interface loadcurves
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->Interfaces(); ++j)
		{
			FEInterface* pci = pstep->Interface(j);
			if (pci && pci->IsActive()) AddLoadCurves(*pci);
		}
	}

	// add rigid constraints loadcurves
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->RigidConstraints(); ++j)
		{
			FERigidConstraint* prc = pstep->RigidConstraint(j);
			if (prc->IsActive()) AddLoadCurves(*prc);
		}
	}

	// add NL constraints
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Constraints(); ++j)
		{
			FEModelConstraint* pw = step->Constraint(j);
			if (pw && pw->IsActive()) AddLoadCurves(*pw);
		}
	}

	// add rigid load loadcurves
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j < pstep->RigidLoads(); ++j)
		{
			FERigidLoad* prl = pstep->RigidLoad(j);
			if (prl->IsActive()) AddLoadCurves(*prl);
		}
	}

	// add connector loadcurves
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);
		for (int j = 0; j<pstep->RigidConnectors(); ++j)
		{
			FERigidConnector* pci = pstep->RigidConnector(j);
			if (pci->IsActive()) AddLoadCurves(*pci);
		}
	}

	// see if there are any rigid body constraints
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		for (int j = 0; j<ps->RigidConstraints(); ++j)
		{
			FERigidPrescribed* prc = dynamic_cast<FERigidPrescribed*>(ps->RigidConstraint(j));
			if (prc && prc->IsActive() && (prc->GetDOF() >= 0) && (prc->GetLoadCurve()))
			{
				AddLoadCurve(prc->GetLoadCurve());
			}
		}
	}

	// spring loadcurves
	GModel& mdl = fem.GetModel();
	for (int i = 0; i<(int)mdl.DiscreteObjects(); ++i)
	{
		GDiscreteObject* pg = mdl.DiscreteObject(i);
		AddLoadCurves(*pg);

		GDiscreteSpringSet* ps = dynamic_cast<GDiscreteSpringSet*>(pg);
		if (ps)
		{
			FEDiscreteMaterial* pm = ps->GetMaterial();
			if (pm) AddLoadCurves(*pm);
		}
	}
}

//-----------------------------------------------------------------------------
void FEBioExport::MultiMaterialCurves(FEMaterial* pm)
{
	if (pm->Properties() > 0) {
		for (int k = 0; k<pm->Properties(); ++k)
		{
			FEMaterialProperty& mc = pm->GetProperty(k);
			for (int l = 0; l<mc.Size(); ++l)
			{
				FEMaterial* pmat = mc.GetMaterial(l);
				if (pmat)
				{
					AddLoadCurves(*pmat);
					MultiMaterialCurves(pmat);
				}
			}
		}
	}
	return;
}
