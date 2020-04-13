#include "stdafx.h"
#include "FEObjectProps.h"
#include <GeomLib/GObject.h>
#include <FEMLib/FEBoundaryCondition.h>
#include <FEMLib/FEAnalysisStep.h>
#include <MeshTools/FEModel.h>
#include <FEMLib/FERigidConstraint.h>
#include <MeshTools/GMaterial.h>
#include <MeshTools/FEProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEDataMap.h>

//=======================================================================================
FEObjectProps::FEObjectProps(FSObject* po, FEModel* fem) : CObjectProps(nullptr)
{
	m_fem = fem;
	if (po) BuildParamList(po);
}

QStringList FEObjectProps::GetEnumValues(const char* ch)
{
	QStringList ops;
	char sz[256] = { 0 };
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
CFixedDOFProps::CFixedDOFProps(FEFixedDOF* pbc) : m_bc(pbc)
{
	FEModel& fem = *pbc->GetFEModel();
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
CAnalysisTimeSettings::CAnalysisTimeSettings(FEAnalysisStep* step) : CObjectProps(0)
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
		int N = m_step->Parameters();
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
			case 5: return set.plot_stride; break;
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
		int N = m_step->Parameters();
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
			case 5: set.plot_stride = v.toInt(); break;
			}
		}
	}
	}
}

//=======================================================================================
CRigidInterfaceSettings::CRigidInterfaceSettings(FEModel& fem, FERigidInterface* pi) : m_ri(pi)
{
	QStringList mats;
	m_sel = -1;
	int n = 0;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
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
CRigidConstraintSettings::CRigidConstraintSettings(FEModel& fem, FERigidConstraint* pi) : CObjectProps(0), m_rc(pi)
{
	QStringList mats;
	m_sel = -1;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
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
CRigidConnectorSettings::CRigidConnectorSettings(FEModel& fem, FERigidConnector* pi) : CObjectProps(0), m_rc(pi)
{
	QStringList mats;
	m_rbA = -1, m_rbB = -1;
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
		{
			m_mat.push_back(pm);
			mats.push_back(QString::fromStdString(pm->GetName()));
			if (pm->GetID() == pi->m_rbA) m_rbA = (int)m_mat.size() - 1;
			if (pm->GetID() == pi->m_rbB) m_rbB = (int)m_mat.size() - 1;
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
			m_rc->m_rbA = m_mat[m_rbA]->GetID();
		}
	}
	else if (i == 1)
	{
		m_rbB = v.toInt();
		if (m_mat.empty() == false)
		{
			m_rc->m_rbB = m_mat[m_rbB]->GetID();
		}
	}
	else CObjectProps::SetPropertyValue(i - 2, v);
}

//=======================================================================================
CMaterialProps::CMaterialProps(FEModel& fem, FEMaterial* mat) : FEObjectProps(0, &fem), m_mat(mat)
{
	BuildPropertyList();
}

void CMaterialProps::BuildPropertyList()
{
	// clear all the properties
	Clear();
	m_params.clear();

	// get the material properties
	FEMaterial* pm = m_mat;

	// add the parameters
	if (pm) BuildParamList(pm, true);

	// add the fiber parameters
	FETransverselyIsotropic* ptiso = dynamic_cast<FETransverselyIsotropic*>(pm);
	if (ptiso)
	{
		FEOldFiberMaterial* fiber = ptiso->GetFiberMaterial();

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
		if (pm && pm->HasMaterialAxes())
		{
			// add the material axes selection option
			QStringList val;
			val << "(none)";
			val << "local node numbering";
			val << "vector";
			addProperty("Material axes", CProperty::Enum)->setEnumValues(val);

			switch (pm->m_naopt)
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
			}
		}
	}
}

QVariant CMaterialProps::GetPropertyValue(int i)
{
	if (i<m_params.size()) return CObjectProps::GetPropertyValue(i);
	i -= (int)m_params.size();

	FETransverselyIsotropic* ptiso = dynamic_cast<FETransverselyIsotropic*>(m_mat);
	if (ptiso)
	{
		FEOldFiberMaterial* fiber = ptiso->GetFiberMaterial();
		if (i == 0) return fiber->m_naopt;

		switch (fiber->m_naopt)
		{
		case FE_FIBER_LOCAL:
			if (i == 1) return fiber->m_n[0];
			if (i == 2) return fiber->m_n[1];
			if (i == 3) return fiber->m_n[2];
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
		if (i == 0) return m_mat->m_naopt + 1;

		switch (m_mat->m_naopt)
		{
		case FE_AXES_LOCAL:
			if (i == 1) return m_mat->m_n[0];
			if (i == 2) return m_mat->m_n[1];
			if (i == 3) return m_mat->m_n[2];
			break;
		case FE_AXES_VECTOR:
			if (i == 1) return Vec3dToString(m_mat->m_a);
			if (i == 2) return Vec3dToString(m_mat->m_d);
			break;
		}
	}

	return QVariant();
}

void CMaterialProps::SetPropertyValue(int i, const QVariant& v)
{
	if (i<m_params.size()) CObjectProps::SetPropertyValue(i, v);
	i -= (int)m_params.size();

	FETransverselyIsotropic* ptiso = dynamic_cast<FETransverselyIsotropic*>(m_mat);
	if (ptiso)
	{
		FEOldFiberMaterial* fiber = ptiso->GetFiberMaterial();
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
			if (i == 1) { fiber->m_n[0] = v.toInt(); return; }
			if (i == 2) { fiber->m_n[1] = v.toInt(); return; }
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
			if (naopt != m_mat->m_naopt)
			{
				m_mat->m_naopt = naopt;

				// rebuild the property list
				BuildPropertyList();

				// set the modified flag to that the viewer knows that the property list has changed
				SetModified(true);
			}
			return;
		}

		switch (m_mat->m_naopt)
		{
		case FE_AXES_LOCAL:
			if (i == 1) { m_mat->m_n[0] = v.toInt(); return; }
			if (i == 2) { m_mat->m_n[1] = v.toInt(); return; }
			break;
		case FE_AXES_VECTOR:
			if (i == 1) { m_mat->m_a = StringToVec3d(v.toString()); return; }
			if (i == 2) { m_mat->m_d = StringToVec3d(v.toString()); return; }
			break;
		}
	}
}

//=======================================================================================
CPlotfileProperties::CPlotfileProperties(FEProject& prj) : CObjectProps(0)
{
	CPlotDataSettings& plt = prj.GetPlotDataSettings();

	for (int i = 0; i<plt.PlotVariables(); ++i)
	{
		FEPlotVariable& var = plt.PlotVariable(i);
		if (var.isShown() && var.isActive())
		{
			addProperty(QString::fromStdString(var.displayName()), CProperty::Bool)->setFlags(CProperty::Visible);
		}
	}
}

QVariant CPlotfileProperties::GetPropertyValue(int i)
{
	return true;
}

void CPlotfileProperties::SetPropertyValue(int i, const QVariant& v)
{
}

//=======================================================================================
CLogfileProperties::CLogfileProperties(FEProject& prj) : CObjectProps(0)
{
	CLogDataSettings& log = prj.GetLogDataSettings();

	for (int i = 0; i<log.LogDataSize(); ++i)
	{
		FELogData& ld = log.LogData(i);
		addProperty(QString::fromStdString(ld.sdata), CProperty::Bool)->setFlags(CProperty::Visible);
	}
}

QVariant CLogfileProperties::GetPropertyValue(int i)
{
	return true;
}

void CLogfileProperties::SetPropertyValue(int i, const QVariant& v)
{
}

//=======================================================================================
CReactionReactantProperties::CReactionReactantProperties(FEReactionMaterial* mat, FEModel& fem) : CObjectProps(0), m_mat(mat)
{
	int NR = m_mat->Reactants();
	for (int i = 0; i<NR; ++i)
	{
		FEReactantMaterial* pr = m_mat->Reactant(i);
		int index = pr->GetIndex();

		if (pr->GetReactantType() == 1)
		{
			FESoluteData& sd = fem.GetSoluteData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			FESoluteData& sd = fem.GetSBMData(index);
			QString t = "vR (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionReactantProperties::GetPropertyValue(int i)
{
	FEReactantMaterial* pr = m_mat->Reactant(i);
	return pr->GetCoef();
}

void CReactionReactantProperties::SetPropertyValue(int i, const QVariant& v)
{
	FEReactantMaterial* pr = m_mat->Reactant(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
CReactionProductProperties::CReactionProductProperties(FEReactionMaterial* mat, FEModel& fem) : CObjectProps(0), m_mat(mat)
{
	int NP = m_mat->Products();
	for (int i = 0; i<NP; ++i)
	{
		FEProductMaterial* pr = m_mat->Product(i);
		int index = pr->GetIndex();

		if (pr->GetProductType() == 1)
		{
			FESoluteData& sd = fem.GetSoluteData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
		else
		{
			FESoluteData& sd = fem.GetSBMData(index);
			QString t = "vP (" + QString::fromStdString(sd.GetName()) + ")";
			addProperty(t, CProperty::Int);
		}
	}
}

QVariant CReactionProductProperties::GetPropertyValue(int i)
{
	FEProductMaterial* pr = m_mat->Product(i);
	return pr->GetCoef();
}

void CReactionProductProperties::SetPropertyValue(int i, const QVariant& v)
{
	FEProductMaterial* pr = m_mat->Product(i);
	pr->SetCoeff(v.toInt());
}

//=======================================================================================
CDataMapProps::CDataMapProps(FEDataMap* map) : CObjectProps(0), m_map(map)
{
	BuildPropertyList();
}

void CDataMapProps::BuildPropertyList()
{
	Clear();

	QStringList enums;
	enums << "const" << "math" << "mesh data";
	addProperty("generator", CProperty::Enum)->setEnumValues(enums);

	if (m_map->GetGenerator() == 0)
	{
		addProperty("value", CProperty::Float);
	}
	else if (m_map->GetGenerator() == 1)
	{
		addProperty("f(X,Y,Z) = ", CProperty::String);
	}
}

QVariant CDataMapProps::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_map->GetGenerator(); break;
	case 1:
		if (m_map->GetGenerator() == 0)
		{
			return m_map->GetConstValue();
		}
		else if (m_map->GetGenerator() == 1)
		{
			return QString::fromStdString(m_map->GetMathString());
		}
		break;
	}

	return QVariant();
}

void CDataMapProps::SetPropertyValue(int i, const QVariant& v)
{
	switch (i)
	{
	case 0:
	{
		int n = v.toInt();
		m_map->SetGenerator(n);
		BuildPropertyList();
		SetModified(true);
	}
	break;
	case 1:
		if (m_map->GetGenerator() == 0)
		{
			m_map->SetConstValue(v.toDouble());
		}
		else if (m_map->GetGenerator() == 1)
		{
			m_map->SetMathString(v.toString().toStdString());
		}
		break;
	}
}
