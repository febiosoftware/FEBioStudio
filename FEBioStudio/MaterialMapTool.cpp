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
#include "MaterialMapTool.h"
#include <LSDyna/LSDYNAimport.h>
#include "GLDocument.h"
#include <PostLib/ColorMap.h>
#include <FEMLib/FEMultiMaterial.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>

CMaterialMapTool::CMaterialMapTool(CMainWindow* wnd) : CBasicTool(wnd, "Material Map", HAS_APPLY_BUTTON)
{
	m_nmat = 5;

	QStringList flt;
	flt << "LSDYNA keyword files (*.k)";

	addResourceProperty(&m_file, "E")->setEnumValues(flt);
	addIntProperty(&m_nmat, "Materials");
}

bool CMaterialMapTool::OnApply()
{
	if ((m_nmat < 1) || (m_nmat > 100))
	{
		SetErrorString("Materials must be a number between 1 and 100.");
		return false;
	}

	CDocument* pdoc = GetDocument();

	std::string sfile = m_file.toStdString();
/*
	LSDYNAimport reader;
	if (!pdoc->ImportGeometry(&reader, sfile.c_str()))
	{
		SetErrorString(QString::fromStdString(reader.GetErrorMessage()));
		return false;
	}
	else
	{
		FSModel& fem = *pdoc->GetFSModel();
		GObject* po = fem.GetModel().Object(0);
		FSMesh& mesh = *po->GetFEMesh();
		int N = mesh.Nodes();
		int NE = mesh.Elements();

		// find the E-range
		double Emin, Emax;
		Emin = Emax = reader.NodeData(0, 0);
		for (int i = 1; i<N; ++i)
		{
			double E = reader.NodeData(i, 0);
			if (E < Emin) Emin = E;
			if (E > Emax) Emax = E;
		}
		if (Emin == Emax) Emax += 1.0;

		// find the v-range
		double vmin, vmax;
		vmin = vmax = reader.NodeData(0, 1);
		for (int i = 1; i<N; ++i)
		{
			double v = reader.NodeData(i, 1);
			if (v < vmin) vmin = v;
			if (v > vmax) vmax = v;
		}
		if (vmin == vmax) vmax += 1.0;

		if (m_nmat > 1)
		{
			// set the element material ID's
			for (int i = 0; i<NE; ++i)
			{
				FSElement& e = mesh.Element(i);
				int ne = e.Nodes();
				double E = 0;
				for (int j = 0; j<ne; ++j) E += reader.NodeData(e.m_node[j], 0);
				E /= (double)ne;

				double v = 0;
				for (int j = 0; j<ne; ++j) v += reader.NodeData(e.m_node[j], 1);
				v /= (double)ne;

				int n0 = (int)(m_nmat*(E - Emin) / (Emax - Emin));
				int n1 = (int)(m_nmat*(v - vmin) / (vmax - vmin));

				if (n0 >= m_nmat) n0 = m_nmat - 1;
				if (n0 < 0) n0 = 0;

				if (n1 >= m_nmat) n1 = m_nmat - 1;
				if (n1 < 0) n1 = 0;

				e.m_ntag = n0*m_nmat + n1;
			}

			// rebuild the object
			po->Update();

			// create the materials
			Post::CColorMap col;
			col.jet();
			for (int i = 0; i<m_nmat; ++i)
			{
				double f = (double)i / (m_nmat - 1.f);
				for (int j = 0; j<m_nmat; ++j)
				{
					double g = (double)j / (m_nmat - 1.f);

					FENeoHookean* pm = new FENeoHookean;
					pm->SetFloatValue(FENeoHookean::MP_E, Emin + f*(Emax - Emin));
					pm->SetFloatValue(FENeoHookean::MP_v, vmin + g*(vmax - vmin));

					double h = (double)(i*m_nmat + j) / (m_nmat*m_nmat - 1.f);
					GMaterial* pgm = new GMaterial(pm);
					pgm->Diffuse(col.map(h*255.f));
					pgm->Ambient(col.map(h*255.f));
					fem.AddMaterial(pgm);
				}
			}

			int NP = po->Parts();
			vector<int> MP; MP.assign(NP, 0);
			for (int i = 0; i<NE; ++i)
			{
				FSElement& e = mesh.Element(i);
				MP[e.m_gid] = e.m_ntag;
			}

			for (int i = 0; i<po->Parts(); ++i)
			{
				po->AssignMaterial(po->Part(i)->GetID(), fem.GetMaterial(MP[i])->GetID());
			}
		}
	}
*/
	return true;
}
