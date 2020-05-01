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

        FEMesh& mesh = *po->GetFEMesh();

        // tag faces and their nodes
        int N = mesh.Nodes();
        for (int i=0; i<N; ++i) mesh.Node(i).m_ntag = 0;
        int F = mesh.Faces();
        for (int i=0; i<F; ++i) mesh.Face(i).m_ntag = 0;
        for (int i=0; i<F; ++i)
        {
            FEFace& f = mesh.Face(i);
            if (f.IsSelected())
                mesh.Face(i).m_ntag = 1;
        }

        // transform nodal coordinates
        vector<vec3d> y(N);
        for (int i=0; i<N; ++i)
            y[i] = po->GetTransform().LocalToGlobal(mesh.Node(i).r);

        // evaluate and sum up face areas
        vec3d A(0,0,0);
        double Amag = 0;
        vec3d x[FEFace::MAX_NODES];
        double gr[FEFace::MAX_NODES], gs[FEFace::MAX_NODES], gw[FEFace::MAX_NODES];
        for (int i=0; i<F; ++i) {
            FEFace& f = mesh.Face(i);
            if (f.IsSelected()) {
                // extract nodal coordinates
                for (int j=0; j<f.Nodes(); ++j)
                    x[j] = y[f.n[j]];
                int nint = f.gauss(gr, gs, gw);
                for (int k=0; k<nint; ++k) {
                    vec3d g1 = f.eval_deriv1(x, gr[k], gs[k]);
                    vec3d g2 = f.eval_deriv2(x, gr[k], gs[k]);
                    vec3d g1xg2 = g1 ^ g2;
                    A += g1xg2*gw[k];
                    Amag += g1xg2.Length();
                }
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
