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

#include "FluentExport.h"
#include <FEMLib/FSProject.h>
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>

FluentExport::FluentExport(FSProject& prj) : FSFileExport(prj)
{
}

FluentExport::~FluentExport(void)
{
}

bool FluentExport::Write(const char* szfile)
{
    FILE* fp = fopen(szfile, "wt");
    if (fp == 0) return false;
    
    FSModel* ps = &m_prj.GetFSModel();
    GModel& model = ps->GetModel();
    
    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh* pm = model.Object(i)->GetFEMesh();
        if (pm == 0) return false;
        FSMesh& m = *pm;
        for (int j=0; j<m.Nodes(); ++j) m.Node(j).m_ntag = 0;
        for (int j=0; j<m.Elements(); ++j)
        {
            FSElement& e = m.Element(j);
            e.m_ntag = 1;
            int n = e.Nodes();
            for (int k=0; k<n; ++k) m.Node(e.m_node[k]).m_ntag = 1;
        }
    }
    
    // count nr of nodes
    int nodes = 0;
    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh& m = *model.Object(i)->GetFEMesh();
        for (int j=0; j<m.Nodes(); ++j) if (m.Node(j).m_ntag == 1) m.Node(j).m_ntag = ++nodes;
    }
    // count nr of elements
    int elems = 0;
    int faces = 0;
    std::vector<int> bfaces;    // boundary faces
    std::vector<int> ifaces;    // interior faces
    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh& m = *model.Object(i)->GetFEMesh();
        for (int j=0; j<m.Elements(); ++j) {
            FSElement& e = m.Element(j);
            e.m_ntag = ++elems;
            for (int k=0; k<e.Faces(); ++k) {
                FSFace& f = m.Face(e.m_face[k]);
                f.m_ntag = ++faces;
                if (f.m_elem[1].eid == -1) bfaces.push_back(f.m_ntag);
                else ifaces.push_back(f.m_ntag);
            }
        }
    }

    // --- H E A D E R ---
    // start with comment
    fprintf(fp, "(0 \"This file was generated with FEBioStudio\")\n");
    // provide official header of Fluent mesh file
    fprintf(fp, "(1 \"FEBioStudio 2.4\")\n");
    // specify dimensions (always 3D in FEBioStudio)
    fprintf(fp, "(2 3)\n");
    
    // --- P A R T ---
    fprintf(fp, "(0 \"Mesh info\")\n");
    fprintf(fp, "(12 (0 %X %X 0))\n\n",1, elems);
    fprintf(fp, "(13 (0 %X %X 0))\n\n",1, faces);
    fprintf(fp, "(10 (0 %X %X 0 3))\n\n",1, nodes);

    int zone = 0;

    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh* pm = model.Object(i)->GetFEMesh();
        if (pm == 0) return false;
        FSMesh& m = *pm;
        // save the nodal coordinates
        fprintf(fp, "(0 \"Nodes\")\n");
        fprintf(fp, "(10 (%d %X %X 1 3)(\n",++zone,m.Node(0).m_ntag,m.Node(m.Nodes()-1).m_ntag);
        for (int j=0; j<m.Nodes(); ++j) {
            FSNode& n = m.Node(j);
            fprintf(fp, "%g %g %g\n", n.r.x, n.r.y, n.r.z);
        }
        fprintf(fp, "))\n\n");
        
        std::vector<int> etype(m.Elements(),0);
        std::vector<std::vector<int>> adjf;
        bool mixed = false;
        for (int j=0; j<m.Elements(); ++j)
        {
            FSElement& f = m.Element(j);
            switch (f.Type()) {
                case FE_TRI3:
                    etype[j] = 1;
                    break;
                case FE_TET4:
                    etype[j] = 2;
                    break;
                case FE_QUAD4:
                    etype[j] = 3;
                    break;
                case FE_HEX8:
                {
                    etype[j] = 4;
                    adjf.assign(6,std::vector<int>(4));
                    adjf[0][0] = 4; adjf[0][1] = 1; adjf[0][2] = 5; adjf[0][3] = 3;
                    adjf[1][0] = 4; adjf[1][1] = 2; adjf[1][2] = 5; adjf[1][3] = 0;
                    adjf[2][0] = 4; adjf[2][1] = 3; adjf[2][2] = 5; adjf[2][3] = 1;
                    adjf[3][0] = 4; adjf[3][1] = 0; adjf[3][2] = 5; adjf[3][3] = 2;
                    adjf[4][0] = 3; adjf[4][1] = 2; adjf[4][2] = 1; adjf[4][3] = 0;
                    adjf[5][0] = 0; adjf[5][1] = 1; adjf[5][2] = 2; adjf[5][3] = 3;
                }
                    break;
                case FE_PYRA5:
                    etype[j] = 5;
                    break;
                case FE_PENTA6:
                    etype[j] = 6;
                    break;
                default:
                    return false;
                    break;
            }
            if ((j > 0) && (etype[j] != etype[j-1])) mixed = true;
        }
        // save the element nodes
        fprintf(fp, "(0 \"Elements\")\n\n");
        if (mixed) {
            fprintf(fp, "(0 \"Mixed mesh\")\n");
            fprintf(fp, "(12 (%d %X %X 1 0) (\n",++zone,m.Element(0).m_ntag, m.Element(m.Elements()-1).m_ntag);
            for (int j=0; j<m.Elements(); j +=9)
                fprintf(fp, "%d %d %d %d %d %d %d %d %d\n", etype[j], etype[j+1], etype[j+2], etype[j+3],
                        etype[j+4], etype[j+5], etype[j+6], etype[j+7], etype[j+8]);
            fprintf(fp, "))\n\n");
        }
        else {
            fprintf(fp, "(12 (%d %X %X 1 %d))\n\n",++zone,m.Element(0).m_ntag, m.Element(m.Elements()-1).m_ntag,etype[0]);
        }
        int et = mixed ? 0 : etype[0];

        if (bfaces.size() > 0) {
            ++zone;
            // save boundary faces
            fprintf(fp, "(13 (%d %X %X 3 0) (\n",zone,1, bfaces.size());
            for (int j=0; j<m.Elements(); ++j) {
                FSElement& e = m.Element(j);
                for (int k=0; k<e.Faces(); ++k) {
                    FSFace& f = m.Face(e.m_face[k]);
                    if (f.m_elem[1].eid == -1) {
                        fprintf(fp, "%d ",f.Nodes());
                        for (int l=0; l<f.Nodes(); ++l) fprintf(fp, "%d ",m.Node(f.n[l]).m_ntag);
                        FSElement& e = m.Element(f.m_elem[0].eid);
                        fprintf(fp,"0 %d",e.m_ntag);
                        fprintf(fp,"\n");
                    }
                }
            }
            fprintf(fp, "))\n\n");
        }

        if (ifaces.size() > 0) {
            ++zone;
            // save interior faces
            fprintf(fp, "(13 (%d %X %X 2 0) (\n",zone,bfaces.size()+1, m.Faces());
            for (int j=0; j<m.Elements(); ++j) {
                FSElement& e = m.Element(j);
                for (int k=0; k<e.Faces(); ++k) {
                    if (e.m_face[k] > -1) {
                        FSFace& f = m.Face(e.m_face[k]);
                        if (f.m_elem[1].eid != -1) {
                            fprintf(fp, "%d ",f.Nodes());
                            for (int l=0; l<f.Nodes(); ++l) fprintf(fp, "%d ",m.Node(f.n[l]).m_ntag);
                            FSElement& er = m.Element(f.m_elem[1].eid);
                            FSElement& el = m.Element(f.m_elem[0].eid);
                            fprintf(fp,"%d %d",er.m_ntag, el.m_ntag);
                            fprintf(fp,"\n");
                        }
                    }
                }
            }
            fprintf(fp, "))\n\n");
        }
    }
    fclose(fp);
    
    return true;
}
