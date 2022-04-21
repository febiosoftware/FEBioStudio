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

#include "stdafx.h"
#include "QuadricFitTool.h"
#include <QWidget>
#include <QBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QFormLayout>
#include <QLineEdit>
#include "ModelDocument.h"
#include <MeshTools/QuadricFit.h>
#include <MeshTools/PointCloud3d.h>
#include <GeomLib/GObject.h>

// constructor
CQuadricFitTool::CQuadricFitTool(CMainWindow* wnd) : CBasicTool(wnd, "Quadric Fit", HAS_APPLY_BUTTON)
{
    m_bsel = true;
    m_x = vec3d(0,0,0);
    m_A = m_B = m_C = 0.0;
    m_U = m_V = m_W = 0.0;
    m_c = 0.0;
    m_qtype = QuadricFit::Q_UNKNOWN;
    m_quadric = QString("Unknown");
    
    addBoolProperty(&m_bsel, "selection only");
    addStringProperty(&m_quadric, "Quadric Type");
    addVec3Property(&m_x, "Center")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_A, "A")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_B, "B")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_C, "C")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_U, "U")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_V, "V")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_W, "W")->setFlags(CProperty::Visible);
    addDoubleProperty(&m_c, "c")->setFlags(CProperty::Visible);
    addVec3Property(&m_ax[0], "Axis 1")->setFlags(CProperty::Visible);
    addVec3Property(&m_ax[1], "Axis 2")->setFlags(CProperty::Visible);
    addVec3Property(&m_ax[2], "Axis 3")->setFlags(CProperty::Visible);
    
    SetApplyButtonText("Fit");
}

bool CQuadricFitTool::OnApply()
{
    // get the nodal coordinates (surface only)
    CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
    if (doc && doc->IsValid())
    {
        GObject* po = doc->GetActiveObject();
        if ((po == 0) || (po->GetFEMesh() == 0))
        {
            SetErrorString("You must select an object with a mesh.");
            return false;
        }
        
        FSMesh& mesh = *po->GetFEMesh();
        
        int N = mesh.Nodes();
        int F = mesh.Faces();
        for (int i = 0; i<N; ++i) mesh.Node(i).m_ntag = 0;
        for (int i = 0; i<F; ++i)
        {
            FSFace& f = mesh.Face(i);
            if ((m_bsel == false) || (f.IsSelected()))
            {
                int nf = f.Nodes();
                for (int j = 0; j<nf; ++j) mesh.Node(f.n[j]).m_ntag = 1;
            }
        }
        
        PointCloud3d* pc = new PointCloud3d();
        for (int i = 0; i<N; ++i)
        {
            if (mesh.Node(i).m_ntag == 1) pc->AddPoint(mesh.Node(i).r);
        }
        
        // find the best fit sphere
        QuadricFit* fit = new QuadricFit();
        fit->Fit(pc);
        
        // update GUI
        m_x = fit->m_rc;
        m_A = fit->m_c2.x;
        m_B = fit->m_c2.y;
        m_C = fit->m_c2.z;
        m_U = fit->m_v.x;
        m_V = fit->m_v.y;
        m_W = fit->m_v.z;
        m_ax[0] = fit->m_ax[0];
        m_ax[1] = fit->m_ax[1];
        m_ax[2] = fit->m_ax[2];
        m_c = fit->m_c;

        m_qtype = fit->GetType();
        std::string quadric = fit->GetStringType(m_qtype);
        m_quadric = QString(quadric.c_str());
    }
    
    return true;
}
