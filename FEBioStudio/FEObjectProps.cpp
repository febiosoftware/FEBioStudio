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

//=======================================================================================
FEObjectProps::FEObjectProps(FSObject* po, FSModel* fem) : CObjectProps(nullptr)
{
	m_fem = fem;
	if (po) BuildParamList(po);
}

QStringList FEObjectProps::GetEnumValues(const char* ch)
{
	QStringList ops;
	char sz[1024] = { 0 };
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

//=======================================================================================
CFixedDOFProps::CFixedDOFProps(FSFixedDOF* pbc) : m_bc(pbc)
{
	FSModel& fem = *pbc->GetFSModel();
	FEDOFVariable& var = fem.Variable(pbc->GetVarID());
	for (int i = 0; i<var.DOFs(); ++i)
	{
		FEDOF& dof = var.GetDOF(i);
		addProperty(dof.name(), CProperty::Bool);
	}
}

QVariant CFixedDOFProps::GetPropertyValue(int i)
{
	return m_bc->GetBC(i);
}

void CFixedDOFProps::SetPropertyValue(int i, const QVariant& v)
{
	m_bc->SetBC(i, v.toBool());
}

//=======================================================================================
CAnalysisTimeSettings::CAnalysisTimeSettings(FSAnalysisStep* step) : CObjectProps(0)
{
	m_step = step;
	addProperty("Analysis", CProperty::Group);

	vector<string> s = step->GetAnalysisStrings();
	QStringList sl;
	for (int i = 0; i<(int)s.size(); ++i) sl << QString::fromStdString(s[i]);
	addProperty("Type", CProperty::Enum)->setEnumValues(sl);

	addProperty("Time settings", CProperty::Group);
	addProperty("Time steps", CProperty::Int);
	addProperty("Step size", CProperty::Float);
	addProperty("Max step size", CProperty::Float);
	addProperty("Min step size", CProperty::Float);
	addProperty("Auto time stepper", CProperty::Bool);
	addProperty("Use must points", CProperty::Bool);
	addProperty("Max retries", CProperty::Int);
	addProperty("Optimal iterations", CProperty::Int);
	addProperty("Cutback", CProperty::Enum)->setEnumValues(QStringList() << "default" << "aggressive");

	addProperty("Linear solver", CProperty::Group);
	addProperty("Matrix Symmetry", CProperty::Enum)->setEnumValues(QStringList() << "default" << "symmetric" << "non-symmetric");
	addProperty("Equation Scheme", CProperty::Enum)->setEnumValues(QStringList() << "default" << "block");

	addProperty("Nonlinear solver", CProperty::Group);
	BuildParamList(step);
	addProperty("Max reformations", CProperty::Int);
	addProperty("Max udpates", CProperty::Int);
	addProperty("Reform on diverge", CProperty::Bool);
	addProperty("Reform each timestep", CProperty::Bool);

	addProperty("Output options", CProperty::Group);
	addProperty("plot level", CProperty::Enum)->setEnumValues(QStringList() << "Never" << "Major iterations" << "Minor iterations" << "Must points" << "Final" << "Augmentations" << "Step final");
	addProperty("plot stride", CProperty::Int);
}

QVariant CAnalysisTimeSettings::GetPropertyValue(int i)
{
	STEP_SETTINGS& set = m_step->GetSettings();

	switch (i)
	{
	case 0: return 0;
	case 1: return set.nanalysis;
	case 2: return 0;
	case 3: return set.ntime;
	case 4: return set.dt;
	case 5: return set.dtmax;
	case 6: return set.dtmin;
	case 7: return set.bauto;
	case 8: return set.bmust;
	case 9: return set.mxback;
	case 10: return set.iteopt;
	case 11: return set.ncut;
	case 12: return 0;
	case 13: return set.nmatfmt;
	case 14: return set.neqscheme;
	case 15: return 0;
	default:
	{
		i -= 16;
		int N = m_params.size();
		if (i < N)
			return CObjectProps::GetPropertyValue(i);
		else
		{
			i -= N;
			switch (i)
			{
			case 0: return set.maxref; break;
			case 1: return set.ilimit; break;
			case 2: return set.bdivref; break;
			case 3: return set.brefstep; break;
			case 4: return 0; break;
			case 5: return set.plot_level; break;
			case 6: return set.plot_stride; break;
			}
		}
	}
	}

	assert(false);
	return QVariant();
}

void CAnalysisTimeSettings::SetPropertyValue(int i, const QVariant& v)
{
	STEP_SETTINGS& set = m_step->GetSettings();

	switch (i)
	{
	case 0: break;
	case 1: set.nanalysis = v.toInt(); break;
	case 2: break;
	case 3: set.ntime = v.toInt(); break;
	case 4: set.dt = v.toDouble(); break;
	case 5: set.dtmax = v.toDouble(); break;
	case 6: set.dtmin = v.toDouble(); break;
	case 7: set.bauto = v.toBool(); break;
	case 8: set.bmust = v.toBool(); break;
	case 9: set.mxback = v.toInt(); break;
	case 10: set.iteopt = v.toInt(); break;
	case 11: set.ncut = v.toInt(); break;
	case 12: break;
	case 13: set.nmatfmt = v.toInt(); break;
	case 14: set.neqscheme = v.toInt(); break;
	case 15: break;
	default:
	{
		i -= 16;
		int N = m_params.size();
		if (i < N)
			CObjectProps::SetPropertyValue(i, v);
		else
		{
			i -= N;
			switch (i)
			{
			case 0: set.maxref = v.toInt(); break;
			case 1: set.ilimit = v.toInt(); break;
			case 2: set.bdivref = v.toBool(); break;
			case 3: set.brefstep = v.toBool(); break;
			case 4: break;
			case 5: set.plot_level = v.toInt(); break;
			case 6: set.plot_stride = v.toInt(); break;
			}
		}
	}
	}
}

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

CStepSettings::CStepSettings(FSProject& prj, FSStep* step) : CObjectProps(0)
{
	m_step = step;
	m_moduleId = prj.GetModule();
	BuildStepProperties();
}

void CStepSettings::BuildStepProperties()
{
	Clear();
	BuildParamList(m_step);
	for (int i = 0; i < m_step->Properties(); ++i)
	{
		FSProperty& prop = m_step->GetProperty(i);
		QStringList ops = GetFEBioChoices(m_moduleId, prop.GetSuperClassID());
		if (prop.IsRequired() == false) ops << "(none)";
		addProperty(QString::fromStdString(prop.GetName()), CProperty::Group)->setEnumValues(ops);
		FSStepComponent* pc = dynamic_cast<FSStepComponent*>(prop.GetComponent());
		if (pc) BuildParamList(pc);
	}
}

QVariant CStepSettings::GetPropertyValue(int n)
{
	int params = m_step->Parameters();
	if (n < params)
	{
		return CObjectProps::GetPropertyValue(m_step->GetParam(n));
	}
	n -= params;
	for (int i = 0; i < m_step->Properties(); ++i)
	{
		FSProperty& prop = m_step->GetProperty(i);
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
	int params = m_step->Parameters();
	if (n < params)
	{
		CObjectProps::SetPropertyValue(m_step->GetParam(n), v);
		return;
	}
	n -= params;
	for (int i = 0; i < m_step->Properties(); ++i)
	{
		FSProperty& prop = m_step->GetProperty(i);
		params = (prop.GetComponent() ? prop.GetComponent()->Parameters() : 0);
		if (n == 0)
		{
			vector<FEBio::FEBioClassInfo> fci = FEBio::FindAllClasses(m_moduleId, prop.GetSuperClassID(), -1);
			int m = v.toInt();
			if ((m >= 0) && (m < fci.size()))
			{
				FSModelComponent* pc = FEBio::CreateClass(fci[m].classId, m_step->GetFSModel()); assert(pc);
				prop.SetComponent(pc);
			}
			BuildStepProperties();
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
CRigidInterfaceSettings::CRigidInterfaceSettings(FSModel& fem, FSRigidInterface* pi) : m_ri(pi)
{
	QStringList mats;
	m_sel = -1;
	int n = 0;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			m_mat.push_back(pm);
			mats.push_back(QString::fromStdString(pm->GetName()));
			if (pi->GetRigidBody() == pm) m_sel = n;
			n++;
		}
	}
	addProperty("Rigid Material", CProperty::Enum)->setEnumValues(mats);
}

QVariant CRigidInterfaceSettings::GetPropertyValue(int i)
{
	return m_sel;
}

void CRigidInterfaceSettings::SetPropertyValue(int i, const QVariant& v)
{
	m_sel = v.toInt();
	if ((m_mat.empty() == false) && (m_sel != -1))
	{
		m_ri->SetRigidBody(m_mat[m_sel]);
	}
	else m_ri->SetRigidBody(0);
}

//=======================================================================================
CRigidConstraintSettings::CRigidConstraintSettings(FSModel& fem, FSRigidConstraint* pi) : CObjectProps(0), m_rc(pi)
{
	QStringList mats;
	m_sel = -1;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (pm->GetMaterialProperties()->IsRigid())
		{
			m_mat.push_back(pm);
			mats.push_back(QString::fromStdString(pm->GetName()));
			if (pm->GetID() == pi->GetMaterialID()) m_sel = (int)m_mat.size() - 1;
		}
	}
	addProperty("Rigid Material", CProperty::Enum)->setEnumValues(mats);

	// add the parameters
	BuildParamList(m_rc);
}

QVariant CRigidConstraintSettings::GetPropertyValue(int i)
{
	if (i == 0) return m_sel;
	else return CObjectProps::GetPropertyValue(i - 1);
}

void CRigidConstraintSettings::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 0)
	{
		m_sel = v.toInt();
		if (m_mat.empty() == false)
		{
			m_rc->SetMaterialID(m_mat[m_sel]->GetID());
		}
	}
	else CObjectProps::SetPropertyValue(i - 1, v);
}

//=======================================================================================
CRigidConnectorSettings::CRigidConnectorSettings(FSModel& fem, FSRigidConnector* pi) : CObjectProps(0), m_rc(pi)
{
	QStringList mats;
	m_rbA = -1, m_rbB = -1;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
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
	BuildParamList(m_rc);
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
	if (i == 0)
	{
		m_rbA = v.toInt();
		if (m_mat.empty() == false)
		{
			m_rc->SetRigidBody1(m_mat[m_rbA]->GetID());
		}
	}
	else if (i == 1)
	{
		m_rbB = v.toInt();
		if (m_mat.empty() == false)
		{
			m_rc->SetRigidBody2(m_mat[m_rbB]->GetID());
		}
	}
	else CObjectProps::SetPropertyValue(i - 2, v);
}

//=======================================================================================
CMaterialProps::CMaterialProps(FSModel& fem, FSMaterial* mat) : FEObjectProps(0, &fem), m_mat(mat)
{
	BuildPropertyList();
}

void CMaterialProps::BuildPropertyList()
{
	// clear all the properties
	Clear();
	m_params.clear();

	// get the material properties
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
CLogfileProperties::CLogfileProperties(CModelViewer* wnd, FSProject& prj) : CObjectProps(0)
{
	m_prj = &prj;
	m_wnd = wnd;
	Update();
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
CReactionReactantProperties::CReactionReactantProperties(FSReactionMaterial* mat, FSModel& fem) : CObjectProps(0), m_mat(mat)
{
	int NR = m_mat->Reactants();
	for (int i = 0; i<NR; ++i)
	{
		FSReactantMaterial* pr = m_mat->Reactant(i);
		int index = pr->GetIndex();

		if (pr->GetReactantType() == 1)
		{
			SoluteData& sd = fem.GetSoluteData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			SoluteData& sd = fem.GetSBMData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionReactantProperties::GetPropertyValue(int i)
{
	FSReactantMaterial* pr = m_mat->Reactant(i);
	return pr->GetCoef();
}

void CReactionReactantProperties::SetPropertyValue(int i, const QVariant& v)
{
	FSReactantMaterial* pr = m_mat->Reactant(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
CReactionProductProperties::CReactionProductProperties(FSReactionMaterial* mat, FSModel& fem) : CObjectProps(0), m_mat(mat)
{
	int NP = m_mat->Products();
	for (int i = 0; i<NP; ++i)
	{
		FSProductMaterial* pr = m_mat->Product(i);
		int index = pr->GetIndex();

		if (pr->GetProductType() == 1)
		{
			SoluteData& sd = fem.GetSoluteData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			SoluteData& sd = fem.GetSBMData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionProductProperties::GetPropertyValue(int i)
{
	FSProductMaterial* pr = m_mat->Product(i);
	return pr->GetCoef();
}

void CReactionProductProperties::SetPropertyValue(int i, const QVariant& v)
{
	FSProductMaterial* pr = m_mat->Product(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
CPartProperties::CPartProperties(GPart* pg, FSModel& fem) : FEObjectProps(0)
{
	m_fem = &fem;
	m_pg = pg;

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

	QStringList mats;
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* m = fem.GetMaterial(i);
		mats.push_back(QString::fromStdString(m->GetName()));
	}
	addProperty("material", CProperty::Enum)->setEnumValues(mats);
}

QStringList CPartProperties::GetEnumValues(const char* ch)
{
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
		assert(m_pg->IsSolid());
		QStringList sl;
		sl << "default";

		GObject* po = dynamic_cast<GObject*>(m_pg->Object());
		if (po && po->GetFEMesh())
		{
			FSMesh* pm = po->GetFEMesh();
			int lid = m_pg->GetLocalID();
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

	int lid = -1;
	int mid = m_pg->GetMaterialID();
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
	GPartSection* section = m_pg->GetSection();
	if (i < Properties() - 1) return CObjectProps::SetPropertyValue(i, v);
	else
	{
		int lid = v.toInt();
		if (lid < 0) m_pg->SetMaterialID(-1);
		else
		{
			GMaterial* m = m_fem->GetMaterial(lid);
			m_pg->SetMaterialID(m->GetID());
		}
	}
}

//=======================================================================================
CImageModelProperties::CImageModelProperties(CImageModel* model)
    : m_model(model), CObjectProps(nullptr)
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
    BOX box = m_model->GetBoundingBox();

    switch (i)
    {
    case PIXELTYPE:
    {
        if(!m_model->Get3DImage())
        {
            return "";
        }
        else
        {
            return m_model->Get3DImage()->PixelTypeString().c_str();
        }
        
    }
    case PXLDIM:
    {
        C3DImage* img = m_model->Get3DImage();
        if(!img)
        {
            return "";
        }
        else
        {
            return QString("%1, %2, %3").arg(img->Width()).arg(img->Height()).arg(img->Depth());
        }
    }
    case SHOWBOX:
        return m_model->ShowBox();
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
    BOX box = m_model->GetBoundingBox();

    switch (i)
    {
    case SHOWBOX:
        m_model->ShowBox(v.toBool());
        break;
    case X0:
        box.x0 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    case Y0:
        box.y0 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    case Z0:
        box.z0 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    case X1:
        box.x1 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    case Y1:
        box.y1 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    case Z1:
        box.z1 = v.toDouble();
        m_model->SetBoundingBox(box);
        break;
    default:
        break;
    }
}