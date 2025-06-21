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
#include "FEObjectProps.h"
#include <GeomLib/GObject.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FEAnalysisStep.h>
#include <FEMLib/FSModel.h>
#include <FEMLib/FERigidConstraint.h>
#include <FEMLib/GMaterial.h>
#include <FEMLib/FSProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEElementFormulation.h>
#include <FEBioLink/FEBioInterface.h>
#include <FEBioLink/FEBioClass.h>
#include "ModelViewer.h"
#include <ImageLib/ImageModel.h>
#include <ImageLib/3DImage.h>
#include "SSHThread.h"
#include "SSHHandler.h"
#include "FEBioJob.h"
#include <PostGL/GLModel.h>
#include <FEMLib/PlotDataSettings.h>

//=======================================================================================
FEObjectProps::FEObjectProps(FSObject* po, FSModel* fem) : CObjectProps(nullptr)
{
	m_fem = fem;
	if (po) BuildParamList(po);
}

QStringList FEObjectProps::GetEnumValues(const char* ch)
{
	QStringList ops;
	char sz[2048] = { 0 };
	if (ch[0] == '$')
	{
		if (m_fem)
		{
			m_fem->GetVariableNames(ch, sz);
			ch = sz;
		}
		else ch = 0;
	}

	while (ch && (*ch))
	{
		ops << QString(ch);
		ch = ch + strlen(ch) + 1;
	}

	return ops;
}

void CFSObjectProps::SetFSObject(FSObject* po)
{
	Clear();
	BuildParamList(po);
}

//=======================================================================================
QStringList GetFEBioChoices(int moduleId, int superClassId)
{
	vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllClasses(moduleId, superClassId, -1, FEBio::ClassSearchFlags::IncludeFECoreClasses);

	QStringList ops;
	for (int i = 0; i < fci.size(); ++i)
	{
		ops << QString(fci[i].sztype);
	}
	return ops;
}

CStepSettings::CStepSettings(FSProject& prj)
{
	m_moduleId = prj.GetModule();
}

void CStepSettings::BuildProperties()
{
	Clear();
	FSStep* step = m_pobj;
	if (step)
	{
		BuildParamList(step);
		for (int i = 0; i < step->Properties(); ++i)
		{
			FSProperty& prop = step->GetProperty(i);
			QStringList ops = GetFEBioChoices(m_moduleId, prop.GetSuperClassID());
			if (prop.IsRequired() == false) ops << "(none)";
			addProperty(QString::fromStdString(prop.GetName()), CProperty::Group)->setEnumValues(ops);
			FSStepComponent* pc = dynamic_cast<FSStepComponent*>(prop.GetComponent());
			if (pc) BuildParamList(pc);
		}
	}
}

QVariant CStepSettings::GetPropertyValue(int n)
{
	FSStep* step = m_pobj;

	int params = step->Parameters();
	if (n < params)
	{
		return CObjectProps::GetPropertyValue(step->GetParam(n));
	}
	n -= params;
	for (int i = 0; i < step->Properties(); ++i)
	{
		FSProperty& prop = step->GetProperty(i);
		params = (prop.GetComponent() ? prop.GetComponent()->Parameters() : 0);
		if (n == 0)
		{
			QStringList ops = GetFEBioChoices(m_moduleId, prop.GetSuperClassID());

			// this is the control property selection.
			if (prop.GetComponent() == nullptr)
			{
				if (prop.IsRequired() == false) return ops.size();
				else return -1;
			}
			QString typeStr(prop.GetComponent()->GetTypeString());
			int n = ops.indexOf(typeStr);
			return n;
		}
		else if (n <= params)
		{
			return CObjectProps::GetPropertyValue(prop.GetComponent()->GetParam(n - 1));
		}
		n -= params + 1;
	}

	return 0;
}

void CStepSettings::SetPropertyValue(int n, const QVariant& v)
{
	FSStep* step = m_pobj;
	int params = step->Parameters();
	if (n < params)
	{
		CObjectProps::SetPropertyValue(step->GetParam(n), v);
		return;
	}
	n -= params;
	for (int i = 0; i < step->Properties(); ++i)
	{
		FSProperty& prop = step->GetProperty(i);
		params = (prop.GetComponent() ? prop.GetComponent()->Parameters() : 0);
		if (n == 0)
		{
			vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllClasses(m_moduleId, prop.GetSuperClassID(), -1);
			int m = v.toInt();
			if ((m >= 0) && (m < fci.size()))
			{
				FSModelComponent* pc = FEBio::CreateClass(fci[m].classId, step->GetFSModel()); assert(pc);
				prop.SetComponent(pc);
			}
			BuildProperties();
			SetModified(true);
			return;
		}
		else if (n <= params)
		{
			CObjectProps::SetPropertyValue(prop.GetComponent()->GetParam(n - 1), v);
			return;
		}
		n -= params + 1;
	}
}

//=======================================================================================
CRigidConnectorSettings::CRigidConnectorSettings(FSModel* fem) : CFSObjectProps_T<FSRigidConnector>(fem)
{
}

void CRigidConnectorSettings::BuildProperties()
{
	FSRigidConnector* pi = m_pobj;
	if (pi == nullptr) return;

	QStringList mats;
	m_rbA = -1, m_rbB = -1;
	for (int i = 0; i<m_fem->Materials(); ++i)
	{
		GMaterial* pm = m_fem->GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			m_mat.push_back(pm);
			mats.push_back(QString::fromStdString(pm->GetName()));
			if (pm->GetID() == pi->GetRigidBody1()) m_rbA = (int)m_mat.size() - 1;
			if (pm->GetID() == pi->GetRigidBody2()) m_rbB = (int)m_mat.size() - 1;
		}
	}
	addProperty("Rigid Material A", CProperty::Enum)->setEnumValues(mats);
	addProperty("Rigid Material B", CProperty::Enum)->setEnumValues(mats);

	// add the parameters
	BuildParamList(pi);
}

QVariant CRigidConnectorSettings::GetPropertyValue(int i)
{
	if (i == 0)
	{
		return m_rbA;
	}
	else if (i == 1)
	{
		return m_rbB;
	}
	else return CObjectProps::GetPropertyValue(i - 2);
}

void CRigidConnectorSettings::SetPropertyValue(int i, const QVariant& v)
{
	FSRigidConnector* pi = m_pobj;
	if (pi == nullptr) return;

	if (i == 0)
	{
		m_rbA = v.toInt();
		if (m_mat.empty() == false)
		{
			pi->SetRigidBody1(m_mat[m_rbA]->GetID());
		}
	}
	else if (i == 1)
	{
		m_rbB = v.toInt();
		if (m_mat.empty() == false)
		{
			pi->SetRigidBody2(m_mat[m_rbB]->GetID());
		}
	}
	else CObjectProps::SetPropertyValue(i - 2, v);
}

//=======================================================================================
CMaterialProps::CMaterialProps(FSModel* fem) : CFSObjectProps_T<GMaterial>(fem)
{
}

void CMaterialProps::BuildProperties()
{
	m_mat = nullptr;
	GMaterial* gmat = m_pobj;
	if (gmat) { m_mat = gmat->GetMaterialProperties(); }

	if (m_mat) BuildPropertyList();
}

void CMaterialProps::BuildPropertyList()
{
	// clear all the properties
	Clear();
	m_params.clear();

	FSMaterial* pm = m_mat;

	// add the parameters
	if (pm) BuildParamList(pm, true);

	// add the fiber parameters
	FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(pm);
	if (ptiso)
	{
		FSOldFiberMaterial* fiber = ptiso->GetFiberMaterial();

		int NP = fiber->Parameters();
		for (int i = 0; i<NP; ++i)
		{
			Param& p = fiber->GetParam(i);
			if (p.IsEditable()) AddParameter(p);
		}

		// add the fiber selection option
		QStringList val;
		val << "local node numbering";
		val << "cylindrical";
		val << "spherical";
		val << "vector";
		val << "user";
		val << "angles";
		val << "polar";
		addProperty("Fiber generator", CProperty::Enum)->setEnumValues(val);

		switch (fiber->m_naopt)
		{
		case FE_FIBER_LOCAL:
			addProperty("n0", CProperty::Int);
			addProperty("n1", CProperty::Int);
			break;
		case FE_FIBER_CYLINDRICAL:
			addProperty("center", CProperty::String);
			addProperty("axis", CProperty::String);
			addProperty("vector", CProperty::String);
			break;
		case FE_FIBER_SPHERICAL:
			addProperty("center", CProperty::String);
			addProperty("vector", CProperty::String);
			break;
		case FE_FIBER_VECTOR:
			addProperty("vector", CProperty::String);
			break;
		case FE_FIBER_USER:
			break;
		case FE_FIBER_ANGLES:
			addProperty("theta", CProperty::Float);
			addProperty("phi", CProperty::Float);
			break;
		case FE_FIBER_POLAR:
			addProperty("center", CProperty::String);
			addProperty("axis", CProperty::String);
			addProperty("R1", CProperty::Float);
			addProperty("vector1", CProperty::String);
			addProperty("R2", CProperty::Float);
			addProperty("vector2", CProperty::String);
			break;
		}
	}
	else
	{
		if (pm && pm->m_axes)
		{
			// add the material axes selection option
			QStringList val;
			val << "(none)";
			val << "local node numbering";
			val << "vector";
            val << "angles";
			addProperty("Material axes", CProperty::Enum)->setEnumValues(val);

			switch (pm->m_axes->m_naopt)
			{
			case FE_AXES_LOCAL:
				addProperty("n0", CProperty::Int);
				addProperty("n1", CProperty::Int);
				addProperty("n2", CProperty::Int);
				break;
			case FE_AXES_VECTOR:
				addProperty("a", CProperty::String);
				addProperty("d", CProperty::String);
				break;
            case FE_AXES_ANGLES:
                addProperty("theta", CProperty::String);
                addProperty("phi", CProperty::String);
                break;
			}
		}
	}
}

QVariant CMaterialProps::GetPropertyValue(int i)
{
	if (i<m_params.size()) return CObjectProps::GetPropertyValue(i);
	i -= (int)m_params.size();

	FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(m_mat);
	if (ptiso)
	{
		FSOldFiberMaterial* fiber = ptiso->GetFiberMaterial();
		if (i == 0) return fiber->m_naopt;

		switch (fiber->m_naopt)
		{
		case FE_FIBER_LOCAL:
			if (i == 1) return fiber->m_n[0] + 1;
			if (i == 2) return fiber->m_n[1] + 1;
			if (i == 3) return fiber->m_n[2] + 1;
			break;
		case FE_FIBER_CYLINDRICAL:
			if (i == 1) return Vec3dToString(fiber->m_r);
			if (i == 2) return Vec3dToString(fiber->m_a);
			if (i == 3) return Vec3dToString(fiber->m_d);
			break;
		case FE_FIBER_SPHERICAL:
			if (i == 1) return Vec3dToString(fiber->m_r);
			if (i == 2) return Vec3dToString(fiber->m_d);
			break;
		case FE_FIBER_VECTOR:
			if (i == 1) return Vec3dToString(fiber->m_a);
			break;
		case FE_FIBER_USER:
			break;
		case FE_FIBER_ANGLES:
			if (i == 1) return fiber->m_theta;
			if (i == 2) return fiber->m_phi;
			break;
		case FE_FIBER_POLAR:
			if (i == 1) return Vec3dToString(fiber->m_r);
			if (i == 2) return Vec3dToString(fiber->m_a);
			if (i == 3) return fiber->m_R0;
			if (i == 4) return Vec3dToString(fiber->m_d0);
			if (i == 5) return fiber->m_R1;
			if (i == 6) return Vec3dToString(fiber->m_d1);
			break;
		}
	}
	else
	{
		if (i == 0) return m_mat->m_axes->m_naopt + 1;

		switch (m_mat->m_axes->m_naopt)
		{
		case FE_AXES_LOCAL:
			if (i == 1) return m_mat->m_axes->m_n[0] + 1;
			if (i == 2) return m_mat->m_axes->m_n[1] + 1;
			if (i == 3) return m_mat->m_axes->m_n[2] + 1;
			break;
		case FE_AXES_VECTOR:
			if (i == 1) return Vec3dToString(m_mat->m_axes->m_a);
			if (i == 2) return Vec3dToString(m_mat->m_axes->m_d);
			break;
        case FE_AXES_ANGLES:
            if (i == 1) return m_mat->m_axes->m_theta;
            if (i == 2) return m_mat->m_axes->m_phi;
            break;
		}
	}

	return QVariant();
}

void CMaterialProps::SetPropertyValue(int i, const QVariant& v)
{
	if (i<m_params.size()) CObjectProps::SetPropertyValue(i, v);
	i -= (int)m_params.size();

	FSTransverselyIsotropic* ptiso = dynamic_cast<FSTransverselyIsotropic*>(m_mat);
	if (ptiso)
	{
		FSOldFiberMaterial* fiber = ptiso->GetFiberMaterial();
		if (i == 0)
		{
			int naopt = v.toInt();
			if (naopt != fiber->m_naopt)
			{
				fiber->m_naopt = naopt;

				// rebuild the property list
				BuildPropertyList();

				// set the modified flag to that the viewer knows that the property list has changed
				SetModified(true);
			}
			return;
		}

		switch (fiber->m_naopt)
		{
		case FE_FIBER_LOCAL:
			if (i == 1) { fiber->m_n[0] = v.toInt() - 1; return; }
			if (i == 2) { fiber->m_n[1] = v.toInt() - 1; return; }
			break;
		case FE_FIBER_CYLINDRICAL:
			if (i == 1) { fiber->m_r = StringToVec3d(v.toString()); return; }
			if (i == 2) { fiber->m_a = StringToVec3d(v.toString()); return; }
			if (i == 3) { fiber->m_d = StringToVec3d(v.toString()); return; }
			break;
		case FE_FIBER_SPHERICAL:
			if (i == 1) { fiber->m_r = StringToVec3d(v.toString()); return; }
			if (i == 2) { fiber->m_d = StringToVec3d(v.toString()); return; }
			break;
		case FE_FIBER_VECTOR:
			if (i == 1) { fiber->m_a = StringToVec3d(v.toString()); return; }
			break;
		case FE_FIBER_USER:
			break;
		case FE_FIBER_ANGLES:
			if (i == 1) { fiber->m_theta = v.toFloat(); return; }
			if (i == 2) { fiber->m_phi = v.toFloat(); return; }
			break;
		case FE_FIBER_POLAR:
			if (i == 1) { fiber->m_r = StringToVec3d(v.toString()); return; }
			if (i == 2) { fiber->m_a = StringToVec3d(v.toString()); return; }
			if (i == 3) { fiber->m_R0 = v.toFloat(); return; }
			if (i == 4) { fiber->m_d0 = StringToVec3d(v.toString()); return; }
			if (i == 5) { fiber->m_R1 = v.toFloat(); return; }
			if (i == 6) { fiber->m_d1 = StringToVec3d(v.toString()); return; }
			break;
		}
	}
	else
	{
		if (i == 0)
		{
			int naopt = v.toInt() - 1;
			if (naopt != m_mat->m_axes->m_naopt)
			{
				m_mat->m_axes->m_naopt = naopt;

				// rebuild the property list
				BuildPropertyList();

				// set the modified flag to that the viewer knows that the property list has changed
				SetModified(true);
			}
			return;
		}

		switch (m_mat->m_axes->m_naopt)
		{
		case FE_AXES_LOCAL:
			if (i == 1) { m_mat->m_axes->m_n[0] = v.toInt() - 1; return; }
			if (i == 2) { m_mat->m_axes->m_n[1] = v.toInt() - 1; return; }
			if (i == 3) { m_mat->m_axes->m_n[2] = v.toInt() - 1; return; }
			break;
		case FE_AXES_VECTOR:
			if (i == 1) { m_mat->m_axes->m_a = StringToVec3d(v.toString()); return; }
			if (i == 2) { m_mat->m_axes->m_d = StringToVec3d(v.toString()); return; }
			break;
        case FE_AXES_ANGLES:
            if (i == 1) { m_mat->m_axes->m_theta = v.toFloat(); return; }
            if (i == 2) { m_mat->m_axes->m_phi   = v.toFloat(); return; }
            break;
		}
	}
}

//=======================================================================================
CLogfileProperties::CLogfileProperties(CModelViewer* wnd, FSProject& prj) : CFSObjectProps(0)
{
	m_prj = &prj;
	m_wnd = wnd;
}

void CLogfileProperties::Update()
{
	Clear();
	if (m_prj == nullptr) return;

	CLogDataSettings& log = m_prj->GetLogDataSettings();

	for (int i = 0; i<log.LogDataSize(); ++i)
	{
		FSLogData& ld = log.LogData(i);
		addProperty(QString::fromStdString(ld.GetDataString()), CProperty::Bool)->setFlags(CProperty::Visible);
	}

	addProperty("", CProperty::Action, "Edit log variables ...");
	m_actionIndex = log.LogDataSize();
}

QVariant CLogfileProperties::GetPropertyValue(int i)
{
	return (i != m_actionIndex ? true : QVariant());
}

void CLogfileProperties::SetPropertyValue(int i, const QVariant& v)
{
	if (i == m_actionIndex)
	{
		m_wnd->blockUpdate(true);
		m_wnd->OnEditOutputLog();
		m_wnd->blockUpdate(false);
	}
}

//=======================================================================================
CReactionReactantProperties::CReactionReactantProperties(FSModel* fem) : CFSObjectProps_T<FSReactionMaterial>(fem)
{

}

void CReactionReactantProperties::BuildProperties()
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return;

	int NR = mat->Reactants();
	for (int i = 0; i<NR; ++i)
	{
		FSReactantMaterial* pr = mat->Reactant(i);
		int index = pr->GetIndex();

		if (pr->GetReactantType() == 1)
		{
			SoluteData& sd = m_fem->GetSoluteData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			SoluteData& sd = m_fem->GetSBMData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionReactantProperties::GetPropertyValue(int i)
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return QVariant();

	FSReactantMaterial* pr = mat->Reactant(i);
	return pr->GetCoef();
}

void CReactionReactantProperties::SetPropertyValue(int i, const QVariant& v)
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return;

	FSReactantMaterial* pr = mat->Reactant(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
CReactionProductProperties::CReactionProductProperties(FSModel* fem) : CFSObjectProps_T<FSReactionMaterial>(fem)
{
}

void CReactionProductProperties::BuildProperties()
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return;

	int NP = mat->Products();
	for (int i = 0; i<NP; ++i)
	{
		FSProductMaterial* pr = mat->Product(i);
		int index = pr->GetIndex();

		if (pr->GetProductType() == 1)
		{
			SoluteData& sd = m_fem->GetSoluteData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			SoluteData& sd = m_fem->GetSBMData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionProductProperties::GetPropertyValue(int i)
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return QVariant();

	FSProductMaterial* pr = mat->Product(i);
	return pr->GetCoef();
}

void CReactionProductProperties::SetPropertyValue(int i, const QVariant& v)
{
	FSReactionMaterial* mat = m_pobj;
	if (mat == nullptr) return;

	FSProductMaterial* pr = mat->Product(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
void CPartProperties::BuildProperties()
{
	GPart* pg = m_pobj;
	if (pg == nullptr) return;

	GPartSection* section = pg->GetSection();
	if (section)
	{
		BuildParamList(section);

		GSolidSection* solidSection = dynamic_cast<GSolidSection*>(section);
		if (solidSection && solidSection->GetElementFormulation())
		{
			FESolidFormulation* form = solidSection->GetElementFormulation();
			AddParameterList(form);
		}

		GShellSection* shellSection = dynamic_cast<GShellSection*>(section);
		if (shellSection && shellSection->GetElementFormulation())
		{
			FEShellFormulation* form = shellSection->GetElementFormulation();
			AddParameterList(form);
		}

		GBeamSection* beamSection = dynamic_cast<GBeamSection*>(section);
		if (beamSection && beamSection->GetElementFormulation())
		{
			FEBeamFormulation* form = beamSection->GetElementFormulation();
			AddParameterList(form);
		}
	}

	if (m_fem)
	{
		QStringList mats;
		for (int i = 0; i < m_fem->Materials(); ++i)
		{
			GMaterial* m = m_fem->GetMaterial(i);
			mats.push_back(QString::fromStdString(m->GetName()));
		}
		addProperty("material", CProperty::Enum)->setEnumValues(mats);
	}
}

QStringList CPartProperties::GetEnumValues(const char* ch)
{
	GPart* pg = m_pobj;
	if (pg == nullptr) return QStringList();

	if (strcmp(ch, "$(solid_domain)") == 0)
	{
		vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESOLIDDOMAIN_ID);
		QStringList sl;
		sl << "default";
		for (int i = 0; i < l.size(); ++i) sl << l[i].sztype;
		return sl;
	}

	if (strcmp(ch, "$(shell_domain)") == 0)
	{
		vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FESHELLDOMAIN_ID);
		QStringList sl;
		sl << "default";
		for (int i = 0; i < l.size(); ++i) sl << l[i].sztype;
		return sl;
	}

	if (strcmp(ch, "$(beam_domain)") == 0)
	{
		vector<FEBio::FEBioClassInfo> l = FEBio::FindAllActiveClasses(FEBEAMDOMAIN_ID);
		QStringList sl;
		sl << "default";
		for (int i = 0; i < l.size(); ++i) sl << l[i].sztype;
		return sl;
	}

	if (strcmp(ch, "$(solid_element)") == 0)
	{
		assert(pg->IsSolid());
		QStringList sl;
		sl << "default";

		GObject* po = dynamic_cast<GObject*>(pg->Object());
		if (po && po->GetFEMesh())
		{
			FSMesh* pm = po->GetFEMesh();
			int lid = pg->GetLocalID();
			for (int i = 0; i < pm->Elements(); ++i)
			{
				FSElement& el = pm->Element(i);
				if (el.m_gid == lid)
				{
					switch (el.Type())
					{
					case FE_TET4 : sl << "TET4G4"; break;
					case FE_TET10: sl << "TET10G4" << "TET10G8" << "TET10GL11"; break;
					case FE_TET15: sl << "TET15G8" << "TET15G11" << "TET15G15"; break;
					case FE_HEX8 : sl << "HEX8G8"; break;
					case FE_HEX20: sl << "HEX20G8"; break;
					}

					break;
				}
			}
		}

		return sl;
	}

	return FEObjectProps::GetEnumValues(ch);
}

QVariant CPartProperties::GetPropertyValue(int i)
{
	if (i < Properties() - 1) return CObjectProps::GetPropertyValue(i);

	GPart* pg = m_pobj;
	if (pg == nullptr) return QVariant();

	int lid = -1;
	int mid = pg->GetMaterialID();
	for (int i = 0; i < m_fem->Materials(); ++i)
	{
		GMaterial* m = m_fem->GetMaterial(i);
		if (m->GetID() == mid) {
			lid = i; break;
		}
	}
	return lid;
}

void CPartProperties::SetPropertyValue(int i, const QVariant& v)
{
	if (i < Properties() - 1) return CObjectProps::SetPropertyValue(i, v);
	else
	{
		GPart* pg = m_pobj;
		if (pg == nullptr) return;

		GObject* po = dynamic_cast<GObject*>(pg->Object());
		if (po)
		{
			int lid = v.toInt();
			if (lid < 0)
			{
				m_fem->AssignMaterial(pg, nullptr);
			}
			else
			{
				GMaterial* m = m_fem->GetMaterial(lid);
				m_fem->AssignMaterial(pg, m);
			}
		}
	}
}

CImageModelProperties::CImageModelProperties()
{
}

void CImageModelProperties::BuildProperties()
{
    addProperty("Pixel Type", CProperty::String)->setFlags(CProperty::Visible);
    addProperty("Dimensions (voxels)", CProperty::String)->setFlags(CProperty::Visible);
    addProperty("Show Box", CProperty::Bool);
    addProperty("x0", CProperty::Float);
    addProperty("y0", CProperty::Float);
    addProperty("z0", CProperty::Float);
    addProperty("x1", CProperty::Float);
    addProperty("y1", CProperty::Float);
    addProperty("z1", CProperty::Float);
}

QVariant CImageModelProperties::GetPropertyValue(int i)
{
	CImageModel* imgModel = m_pobj;
	if (imgModel == nullptr) return QVariant();

    BOX box = imgModel->GetBoundingBox();

    switch (i)
    {
    case PIXELTYPE:
    {
        if(!imgModel->Get3DImage())
        {
            return "";
        }
        else
        {
            return imgModel->Get3DImage()->PixelTypeString().c_str();
        }
        
    }
    case PXLDIM:
    {
        C3DImage* img3d = imgModel->Get3DImage();
        if(!img3d)
        {
            return "";
        }
        else
        {
            return QString("%1, %2, %3").arg(img3d->Width()).arg(img3d->Height()).arg(img3d->Depth());
        }
    }
    case SHOWBOX:
        return imgModel->ShowBox();
    case X0:
        return box.x0;
    case Y0:
        return box.y0;
    case Z0:
        return box.z0;
    case X1:
        return box.x1;
    case Y1:
        return box.y1;
    case Z1:
        return box.z1;
    default:
        return false;
    }
}

void CImageModelProperties::SetPropertyValue(int i, const QVariant& v)
{
	CImageModel* imgModel = m_pobj;
	if (imgModel == nullptr) return;

    BOX box = imgModel->GetBoundingBox();

    switch (i)
    {
    case SHOWBOX:
		imgModel->ShowBox(v.toBool());
        break;
    case X0:
        box.x0 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    case Y0:
        box.y0 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    case Z0:
        box.z0 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    case X1:
        box.x1 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    case Y1:
        box.y1 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    case Z1:
        box.z1 = v.toDouble();
		imgModel->SetBoundingBox(box);
        break;
    default:
        break;
    }
}

CFEBioJobProps::CFEBioJobProps(CMainWindow* wnd, CModelViewer* tree) : m_wnd(wnd), m_tree(tree)
{
}

void CFEBioJobProps::BuildProperties()
{
	CFEBioJob* job = m_pobj;
	if (job == nullptr) return;

	addProperty("Status", CProperty::Enum)->setEnumValues(QStringList() << "NONE" << "NORMAL TERMINATION" << "ERROR TERMINATION" << "CANCELLED" << "RUNNING").setFlags(CProperty::Visible);
	addProperty("FEBio File", CProperty::ExternalLink)->setFlags(CProperty::Editable | CProperty::Visible);
	addProperty("Plot File", CProperty::InternalLink)->setFlags(CProperty::Editable | CProperty::Visible);
	addProperty("Log File", CProperty::ExternalLink)->setFlags(CProperty::Editable | CProperty::Visible);

	int launchType = job->GetLaunchConfig()->type;
	if ((launchType != LOCAL) && (launchType != DEFAULT))
	{
		addProperty("", CProperty::Action)->info = QString("Get Remote Files");
//		addProperty("", CProperty::Action)->info = QString("Orphan Process");
	}

	if (launchType == PBS || launchType == SLURM)
	{
		addProperty("", CProperty::Action)->info = QString("Get Queue Status");
	}
}

QVariant CFEBioJobProps::GetPropertyValue(int i)
{
	CFEBioJob* job = m_pobj;
	if (job == nullptr) return QVariant();

	switch (i)
	{
	case 0: return job->GetStatus();
	case 1:
	{
		QStringList fileNames;
		fileNames.append(QString(job->GetFEBFileName().c_str()));
		fileNames.append(QString(job->GetFEBFileName(true).c_str()));
		return fileNames;
	}
	case 2:
	{
		QStringList fileNames;
		fileNames.append(QString(job->GetPlotFileName().c_str()));
		fileNames.append(QString(job->GetPlotFileName(true).c_str()));
		return fileNames;
	}
	case 3:
	{
		QStringList fileNames;
		fileNames.append(QString(job->GetLogFileName().c_str()));
		fileNames.append(QString(job->GetLogFileName(true).c_str()));
		return fileNames;
	}
	}

	return QVariant();
}

void CFEBioJobProps::SetPropertyValue(int i, const QVariant& v)
{
	CFEBioJob* job = m_pobj;
	if (job == nullptr) return;

#ifdef HAS_SSH
	if (i == 4)
	{
		if (!job->GetSSHHandler()->IsBusy())
		{
			// Copy remote files to local dir
			job->GetSSHHandler()->SetTargetFunction(GETJOBFILES);

			CSSHThread* sshThread = new CSSHThread(job->GetSSHHandler(), STARTSSHSESSION);
			QObject::connect(sshThread, &CSSHThread::FinishedPart, m_wnd, &CMainWindow::NextSSHFunction);
			sshThread->start();
		}
	}
//		else if (i == 6)
//		{
//			job->GetSSHHandler()->Orphan();
//		}
	else if (i == 5)
	{
		if (!job->GetSSHHandler()->IsBusy())
		{
			// Copy remote files to local dir
			m_wnd->ClearOutput();

			job->GetSSHHandler()->SetTargetFunction(GETQUEUESTATUS);
			CSSHThread* sshThread = new CSSHThread(job->GetSSHHandler(), STARTSSHSESSION);
			QObject::connect(sshThread, &CSSHThread::FinishedPart, m_wnd, &CMainWindow::NextSSHFunction);
			sshThread->start();
		}
	}
#endif // HAS_SSH
}

void CPostModelProps::BuildProperties()
{
	addProperty("Element subdivisions", CProperty::Int)->setIntRange(0, 10).setAutoValue(true);
	addProperty("Render mode", CProperty::Enum, "Render mode")->setEnumValues(QStringList() << "default" << "wireframe" << "solid");
	addProperty("Render undeformed outline", CProperty::Bool);
	addProperty("Outline color", CProperty::Color);
	addProperty("Node color", CProperty::Color);
	addProperty("Selection color", CProperty::Color);
	addProperty("Render shells as solid", CProperty::Bool);
	addProperty("Shell reference surface", CProperty::Enum, "set the shell reference surface")->setEnumValues(QStringList() << "Mid surface" << "bottom surface" << "top surface");
	addProperty("Render beams as solid", CProperty::Bool);
	addProperty("Smoothing angle", CProperty::Float);
	addProperty("Render internal surfaces", CProperty::Bool);
}

QVariant CPostModelProps::GetPropertyValue(int i)
{
	Post::CGLModel* glm = m_pobj;
	if (glm == nullptr) return QVariant();

	QVariant v;
	switch (i)
	{
	case 0: v = glm->m_nDivs; break;
	case 1: v = glm->m_nrender; break;
	case 2: v = glm->m_bghost; break;
	case 3: v = toQColor(glm->m_line_col); break;
	case 4: v = toQColor(glm->m_node_col); break;
	case 5: v = toQColor(glm->m_sel_col); break;
	case 6: v = glm->ShowShell2Solid(); break;
	case 7: v = glm->ShellReferenceSurface(); break;
	case 8: v = glm->ShowBeam2Solid(); break;
	case 9: v = glm->GetSmoothingAngle(); break;
	case 10: v = glm->RenderInnerSurfaces(); break;
	}
	return v;
}

void CPostModelProps::SetPropertyValue(int i, const QVariant& v)
{
	Post::CGLModel* glm = m_pobj;
	if (glm == nullptr) return;

	switch (i)
	{
	case 0: glm->m_nDivs = v.toInt(); break;
	case 1: glm->m_nrender = v.toInt(); break;
	case 2: glm->m_bghost = v.toBool(); break;
	case 3: glm->m_line_col = toGLColor(v.value<QColor>());
	case 4: glm->m_node_col = toGLColor(v.value<QColor>());
	case 5: glm->m_sel_col = toGLColor(v.value<QColor>());
	case 6: glm->ShowShell2Solid(v.toBool()); break;
	case 7: glm->ShellReferenceSurface(v.toInt()); break;
	case 8: glm->ShowBeam2Solid(v.toBool()); break;
	case 9: glm->SetSmoothingAngle(v.toDouble());  break;
	case 10: glm->RenderInnerSurfaces(v.toBool()); break;
	}
}

void CDiscreteObjectProps::BuildProperties()
{
	GDiscreteObject* po = m_pobj;

	if (dynamic_cast<GDiscreteSpringSet*>(po))
	{
		GDiscreteSpringSet* pg = dynamic_cast<GDiscreteSpringSet*>(po);
		FSDiscreteMaterial* dm = pg->GetMaterial();
		if (dm) BuildParamList(dm);
	}
	else if (dynamic_cast<GDeformableSpring*>(po))
	{
		BuildParamList(po);
	}
}

CPlotfileProperties::CPlotfileProperties(CModelViewer* wnd, FSProject& prj) : CFSObjectProps(0), m_wnd(wnd), m_prj(prj)
{
}

void CPlotfileProperties::SetFSObject(FSObject* po)
{
	Update();
}

void CPlotfileProperties::Update()
{
	Clear();

	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();

	addProperty("File Properties", CProperty::Group);

	addProperty("plotfile format", CProperty::Enum)->setEnumValues(QStringList() << "febio" << "vtk");

	addProperty("Plot Variables", CProperty::Group);

	int ncount = 3;
	for (int i = 0; i < plt.PlotVariables(); ++i)
	{
		CPlotVariable& var = plt.PlotVariable(i);
		if (var.isShown() && var.isActive())
		{
			addProperty(QString::fromStdString(var.name()), CProperty::Bool)->setFlags(CProperty::Visible);
			ncount++;
		}
	}

	addProperty("", CProperty::Action, "Edit plot variables ...");
	m_actionIndex = ncount;
}

QVariant CPlotfileProperties::GetPropertyValue(int i) 
{ 
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	if (i == 1)
	{
		return (int)plt.GetPlotFormat();
	}
	else return (i != m_actionIndex ? true : QVariant()); 
}

void CPlotfileProperties::SetPropertyValue(int i, const QVariant& v)
{
	CPlotDataSettings& plt = m_prj.GetPlotDataSettings();
	if (i == 1)
	{
		int fmt = v.toInt();
		plt.SetPlotFormat(static_cast<CPlotDataSettings::PlotFormat>(fmt));
	}
	else if (i == m_actionIndex)
	{
		m_wnd->blockUpdate(true);
		m_wnd->OnEditOutput();
		m_wnd->blockUpdate(false);
	}
}

void FSGlobalsProps::SetFSObject(FSObject* po)
{
	Clear();
	BuildParamList(m_fem);
}
