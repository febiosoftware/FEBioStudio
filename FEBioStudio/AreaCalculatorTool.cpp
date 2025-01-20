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
#include "AreaCalculatorTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "ModelDocument.h"
#include <GeomLib/GObject.h>

// constructor
CAreaCalculatorTool::CAreaCalculatorTool(CMainWindow* wnd) : CBasicTool(wnd, "Area Calculator", HAS_APPLY_BUTTON)
{
    m_Ax = m_Ay = m_Az = 0.0;
    m_A = 0.0;
    addDoubleProperty(&m_A, "Area A")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_Ax, "Projected Ax")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_Ay, "Projected Ay")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_Az, "Projected Az")->setFlags(CProperty::Visible);

    SetApplyButtonText("Calculate");
}

bool CAreaCalculatorTool::OnApply()
{
    // get the nodal coordinates (surface only)
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc && doc->IsValid())
    {
        GObject* po = doc->GetActiveObject();
        if ((po == 0) || (po->GetFEMesh() == 0))
        {
            SetErrorString("You must select an object that has a mesh.");
            return false;
        }

        FSMesh& mesh = *po->GetFEMesh();

        // tag faces and their nodes
        int N = mesh.Nodes();
        for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = 0;
        int F = mesh.Faces();
        for (int i=0; i<F; ++i) mesh.Face(i).m_ntag = 0;
        for (int i=0; i<F; ++i)
        {
            FSFace& f = mesh.Face(i);
            if (f.IsSelected())
                mesh.Face(i).m_ntag = 1;
        }

        // transform nodal coordinates
        std::vector<vec3d> y(N);
        for (int i=0; i<N; ++i)
            y[i] = po->GetTransform().LocalToGlobal(mesh.Node(i).r);

        // evaluate and sum up face areas
        vec3d A(0,0,0);
        double Amag = 0;
        vec3d x[FSFace::MAX_NODES];
        double gr[FSFace::MAX_NODES], gs[FSFace::MAX_NODES], gw[FSFace::MAX_NODES];
        for (int i=0; i<F; ++i) {
            FSFace& f = mesh.Face(i);
            if (f.IsSelected()) {
                // extract nodal coordinates
                for (int j=0; j<f.Nodes(); ++j)
                    x[j] = y[f.n[j]];
                int nint = f.gauss(gr, gs, gw);
				vec3d Ai(0, 0, 0);
                for (int k=0; k<nint; ++k) {
                    vec3d g1 = f.eval_deriv1(x, gr[k], gs[k]);
                    vec3d g2 = f.eval_deriv2(x, gr[k], gs[k]);
                    vec3d g1xg2 = g1 ^ g2;
                    Ai += g1xg2*gw[k];
                }
				A += Ai;
				Amag += Ai.Length();
			}
        }

        // update GUI
        m_Ax = A.x;
        m_Ay = A.y;
        m_Az = A.z;
        m_A = Amag;
    }

    return true;
}
