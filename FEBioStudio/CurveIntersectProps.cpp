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
#include "CurveIntersectProps.h"

CCurveIntersectProps::CCurveIntersectProps(GModel* geo, FECurveIntersect* pmod) : m_mod(pmod), m_geo(geo)
{
	m_edge = m_mod->InsertEdges();
	m_tol = m_mod->Tolerance();

	addCurveListProperty(&m_curves, "Curves");
	addBoolProperty(&m_edge, "Insert Edges");
	addDoubleProperty(&m_tol, "Tolerance");
#ifndef NDEBUG
	addEnumProperty(&m_method, "Method")->setEnumValues(QStringList() << "method1" << "method2");
#endif
}

void CCurveIntersectProps::SetPropertyValue(int i, const QVariant& v)
{
	CDataPropertyList::SetPropertyValue(i, v);
	if (i == 0)
	{
		m_mod->ClearCurveList();
		int N = m_geo->Edges();
		QStringList curves = v.toStringList();
		for (int i = 0; i < curves.size(); ++i)
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
