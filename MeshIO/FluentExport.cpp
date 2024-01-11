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

// This exporter uses information about Fluent mesh file format given at
// https://romeo.univ-reims.fr/documents/fluent/tgrid/ug/appb.pdf

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
    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh& m = *model.Object(i)->GetFEMesh();
        for (int j=0; j<m.Elements(); ++j) {
            FSElement& e = m.Element(j);
            e.m_ntag = ++elems;
            for (int k=0; k<e.Faces(); ++k) {
                FSFace nf;
                switch (e.Type()) {
                    case FE_TET4:
                        nf.m_type = FE_FACE_TRI;
                        break;
                    case FE_HEX8:
                        nf.m_type = FE_FACE_QUAD4;
                        break;
                    case FE_PYRA5:
                        if (k < 4) nf.m_type = FE_FACE_TRI;
                        else nf.m_type = FE_FACE_QUAD4;
                        break;
                    case FE_PENTA6:
                        if (k > 2) nf.m_type = FE_FACE_TRI;
                        else nf.m_type = FE_FACE_QUAD4;
                        break;
                    default:
                        return false;
                        break;
                }
                e.GetFace(k, nf);
                int ief = -1;
                // check if this face exists already
                if (FaceExists(nf,ief)) {
                    FSFace& ef = nface[ief];
                    ef.m_elem[0].eid = e.m_ntag;
                    ef.m_elem[0].lid = k;
                }
                else {
                    // if not, add it to the database
                    nf.m_elem[1].eid = e.m_ntag;
                    nf.m_elem[1].lid = k;
                    nf.m_ntag = ++faces;
                    nface.push_back(nf);
                }
            }
        }
    }
    
    // purge nface of boundary faces
    std::vector<FSFace> iface;
    iface.reserve(nface.size());
    for (int i=0; i<nface.size(); ++i) {
        FSFace f = nface[i];
        if (f.m_elem[0].eid >0) iface.push_back(f);
    }
    nface = iface;

    // --- H E A D E R ---
    // start with comment
    fprintf(fp, "(0 \"This file was generated with FEBioStudio\")\n\n");
    // provide official header of Fluent mesh file
    fprintf(fp, "(1 \"FEBioStudio 2.4\")\n\n");
    // specify dimensions (always 3D in FEBioStudio)
    fprintf(fp, "(0 \"Dimensions\")\n");
    fprintf(fp, "(2 3)\n\n");
    
    // --- P A R T ---
    fprintf(fp, "(0 \"Mesh info\")\n");
    fprintf(fp, "(10 (0 %X %X 1 3))\n",1, nodes);
    fprintf(fp, "(12 (0 %X %X 0))\n",1, elems);
    fprintf(fp, "(13 (0 %X %X 0))\n\n",1, faces);

    int zone = 0;

    for (int i=0; i<model.Objects(); ++i)
    {
        FSMesh* pm = model.Object(i)->GetFEMesh();
        if (pm == 0) return false;
        FSMesh& m = *pm;
        // save the nodal coordinates
        fprintf(fp, "(0 \"Nodes\")\n");
        fprintf(fp, "(10 (%X %X %X 1 3)(\n",++zone,m.Node(0).m_ntag,m.Node(m.Nodes()-1).m_ntag);
        for (int j=0; j<m.Nodes(); ++j) {
            FSNode& n = m.Node(j);
            fprintf(fp, "%g %g %g\n", n.r.x, n.r.y, n.r.z);
        }
        fprintf(fp, "))\n\n");
        
        std::vector<int> etype(m.Elements(),0);
        bool mixed = false;
        for (int j=0; j<m.Elements(); ++j)
        {
            FSElement& e = m.Element(j);
            switch (e.Type()) {
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
                    etype[j] = 4;
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
            fprintf(fp, "(12 (%X %X %X 1 0) (\n",++zone,m.Element(0).m_ntag, m.Element(m.Elements()-1).m_ntag);
            for (int j=0; j<m.Elements(); j +=9)
                fprintf(fp, "%X %X %X %X %X %X %X %X %X\n", etype[j], etype[j+1], etype[j+2], etype[j+3],
                        etype[j+4], etype[j+5], etype[j+6], etype[j+7], etype[j+8]);
            fprintf(fp, "))\n\n");
        }
        else {
            fprintf(fp, "(12 (%X %X %X 1 %X))\n\n",++zone,m.Element(0).m_ntag, m.Element(m.Elements()-1).m_ntag,etype[0]);
        }
        int et = mixed ? 0 : etype[0];

        if (m.Faces() > 0) {
            fprintf(fp, "(0 \"Boundary Faces\")\n\n");
            ++zone;
            // save boundary faces
            fprintf(fp, "(13 (%X %X %X 3 0) (\n",zone,1, m.Faces());
            for (int k=0; k<m.Faces(); ++k) {
                FSFace& f = m.Face(k);
                fprintf(fp, "%X ",f.Nodes());
                for (int l=f.Nodes()-1; l>-1; --l) fprintf(fp, "%X ",m.Node(f.n[l]).m_ntag);
                FSElement& e = m.Element(f.m_elem[0].eid);
                fprintf(fp,"%X 0",e.m_ntag);
                fprintf(fp,"\n");
            }
            fprintf(fp, "))\n\n");
        }

        if (nface.size() > 0) {
            fprintf(fp, "(0 \"Internal Faces\")\n\n");
            ++zone;
            // save interior faces
            fprintf(fp, "(13 (%X %X %X 2 0) (\n",zone,m.Faces()+1, m.Faces()+nface.size());
            for (int j=0; j<nface.size(); ++j) {
                FSFace& f = nface[j];
                fprintf(fp, "%X ",f.Nodes());
                for (int l=f.Nodes()-1; l>-1; --l) fprintf(fp, "%X ",m.Node(f.n[l]).m_ntag);
                fprintf(fp,"%X %X",f.m_elem[1].eid, f.m_elem[0].eid);
                fprintf(fp,"\n");
            }
            fprintf(fp, "))\n\n");
        }
    }
    fclose(fp);
    
    return true;
}

bool FluentExport::FaceExists(FSFace nf, int& ief)
{
    ief = -1;
    for (int i=0; i<nface.size(); ++i) {
        FSFace ef = nface[i];
        if (nf.Nodes() == ef.Nodes()) {
            int n = nf.Nodes();
            // save some time with the search process
            // by using a quick method of checking
            int nn = 0;
            for (int j=0; j<n; ++j) {
                nn += ef.n[j];
                nn -= nf.n[j];
            }
            if (nn == 0) {
                // now do the more thorough check
                std::vector<bool> matched(n,false);
                for (int j=0; j<n; ++j) {
                    for (int k=0; k<n; ++k) {
                        if (ef.n[k] == nf.n[j]) {
                            matched[j] = true;
                            break;
                        }
                    }
                }
                bool found = true;
                for (int k=0; k<n; ++k) found = found && matched[k];
                if (found) {
                    ief = i;
                    return true;
                }
            }
        }
    }
    return false;
}
