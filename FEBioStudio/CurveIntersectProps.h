#pragma once
#include "PropertyList.h"
#include <PreViewLib/GModel.h>
#include <PreViewLib/FECurveIntersect.h>

class CCurveIntersectProps : public CDataPropertyList
{
public:
	CCurveIntersectProps(GModel* geo, FECurveIntersect* pmod) : m_mod(pmod), m_geo(geo)
	{
		m_edge = m_mod->InsertEdges();
		m_tol = m_mod->Tolerance();

		addCurveListProperty(&m_curves, "Curves");
		addBoolProperty(&m_edge, "Insert Edges");
		addDoubleProperty(&m_tol, "Tolerance");
#ifdef _DEBUG
		QStringList methods;
		methods << "method1";
		methods << "method2";
		addEnumProperty(&m_method, "Method")->setEnumValues(methods);
#endif
	}

	void SetPropertyValue(int i, const QVariant& v)
	{
		CDataPropertyList::SetPropertyValue(i, v);
		if (i == 0)
		{
			m_mod->ClearCurveList();
			int N = m_geo->Edges();
			QStringList curves = v.toStringList();
			for (int i = 0; i<curves.size(); ++i)
			{
				QString curvei = curves.at(i);
				string name = curvei.toStdString();
				GEdge* edge = m_geo->FindEdgeFromName(name.c_str());
				m_mod->AddCurve(edge);
			}
		}
		if (i == 1) m_mod->SetInsertEdges(m_edge);
		if (i == 2) m_mod->SetTolerance(m_tol);
		if (i == 3) m_mod->SetMethod(m_method);
	}

private:
	GModel*				m_geo;
	FECurveIntersect*	m_mod;
	QStringList			m_curves;
	int					m_method;
	bool				m_edge;
	double				m_tol;
};
