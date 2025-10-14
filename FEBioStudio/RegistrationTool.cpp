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

#include "RegistrationTool.h"

vector<vec3d> extractSurfaceNodes(GObject* po, vector<GPart*> partList)
{
    vector<vec3d> points;
    FSMesh* pm = po->GetFEMesh();
    if (pm == nullptr) return points;
    
    pm->TagAllNodes(0);
    
    for (GPart* pg : partList)
    {
        int pid = pg->GetLocalID();
        for (int i = 0; i < pm->Elements(); ++i)
        {
            FSElement& el = pm->Element(i);
            if (el.m_gid == pid)
            {
                int nn = el.Nodes();
                for (int j = 0; j < nn; ++j) pm->Node(el.m_node[j]).m_ntag = 1;
            }
        }
    }
    
    for (int i = 0; i < pm->Faces(); ++i)
    {
        FSFace& face = pm->Face(i);
        if (face.IsExternal())
        {
            int nn = face.Nodes();
            for (int j = 0; j < nn; ++j)
            {
                FSNode& nj = pm->Node(face.n[j]);
                if (nj.m_ntag == 1) nj.m_ntag = 2;
            }
        }
    }
    
    const Transform& Q = po->GetTransform();
    points.reserve(pm->Nodes());
    for (int i = 0; i < pm->Nodes(); ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 2)
        {
            vec3d r = ni.pos();
            vec3d p = Q.LocalToGlobal(r);
            points.push_back(p);
        }
    }
    
    return points;
}

vector<vec3d> extractSurfaceNodes(GObject* po, vector<GFace*> faceList)
{
    vector<vec3d> points;
    FSMesh* pm = po->GetFEMesh();
    if (pm == nullptr) return points;
    
    pm->TagAllNodes(0);
    
    for (GFace* pg : faceList)
    {
        int pid = pg->GetLocalID();
        for (int i = 0; i < pm->Faces(); ++i)
        {
            FSFace& face = pm->Face(i);
            if (face.m_gid == pid)
            {
                int nn = face.Nodes();
                for (int j = 0; j < nn; ++j) pm->Node(face.n[j]).m_ntag = 1;
            }
        }
    }
    
    for (int i = 0; i < pm->Faces(); ++i)
    {
        FSFace& face = pm->Face(i);
        if (face.IsExternal())
        {
            int nn = face.Nodes();
            for (int j = 0; j < nn; ++j)
            {
                FSNode& nj = pm->Node(face.n[j]);
                if (nj.m_ntag == 1) nj.m_ntag = 2;
            }
        }
    }
    
    const Transform& Q = po->GetTransform();
    points.reserve(pm->Nodes());
    for (int i = 0; i < pm->Nodes(); ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 2)
        {
            vec3d r = ni.pos();
            vec3d p = Q.LocalToGlobal(r);
            points.push_back(p);
        }
    }
    
    return points;
}

vector<vec3d> extractSurfaceNodes(GObject* po, vector<GEdge*> edgeList)
{
    vector<vec3d> points;
    FSMesh* pm = po->GetFEMesh();
    if (pm == nullptr) return points;
    
    pm->TagAllNodes(0);
    
    for (GEdge* pg : edgeList)
    {
        int pid = pg->GetLocalID();
        for (int i = 0; i < pm->Edges(); ++i)
        {
            FSEdge& edge = pm->Edge(i);
            if (edge.m_gid == pid)
            {
                int nn = edge.Nodes();
                for (int j = 0; j < nn; ++j) pm->Node(edge.n[j]).m_ntag = 2;
            }
        }
    }
    
    const Transform& Q = po->GetTransform();
    points.reserve(pm->Nodes());
    for (int i = 0; i < pm->Nodes(); ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 2)
        {
            vec3d r = ni.pos();
            vec3d p = Q.LocalToGlobal(r);
            points.push_back(p);
        }
    }
    
    return points;
}

vector<vec3d> extractSurfaceNodes(GObject* po, FSSurface* surface)
{
    vector<vec3d> points;
    FSMesh* pm = po->GetFEMesh();
    if (pm == nullptr) return points;
    
    pm->TagAllNodes(0);
    
    for (int i = 0; i < surface->size(); ++i)
    {
        FSFace& face = *surface->GetFace(i);
        
        int nn = face.Nodes();
        for (int j = 0; j < nn; ++j)
        {
            FSNode& nj = pm->Node(face.n[j]);
            nj.m_ntag = 2;
        }
    }
    
    const Transform& Q = po->GetTransform();
    points.reserve(pm->Nodes());
    for (int i = 0; i < pm->Nodes(); ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 2)
        {
            vec3d r = ni.pos();
            vec3d p = Q.LocalToGlobal(r);
            points.push_back(p);
        }
    }
    
    return points;
}

vector<vec3d> extractSurfaceNodes(GObject* po, FSNodeSet* nodeSet)
{
    vector<vec3d> points;
    FSMesh* pm = po->GetFEMesh();
    if (pm == nullptr) return points;
    
    pm->TagAllNodes(0);
    
    for (int i = 0; i < nodeSet->size(); ++i)
    {
        FSNode& node = *nodeSet->GetNode(i);
        node.m_ntag = 2;
    }
    
    const Transform& Q = po->GetTransform();
    points.reserve(pm->Nodes());
    for (int i = 0; i < pm->Nodes(); ++i)
    {
        FSNode& ni = pm->Node(i);
        if (ni.m_ntag == 2)
        {
            vec3d r = ni.pos();
            vec3d p = Q.LocalToGlobal(r);
            points.push_back(p);
        }
    }
    
    return points;
}

GObject* GetSelectionNodes(FSItemListBuilder* sel, std::vector<vec3d>& nodes)
{
    GObject* po = nullptr;
    if (dynamic_cast<GPartList*>(sel))
    {
        GPartList* partList = dynamic_cast<GPartList*>(sel);
        vector<GPart*> parts = partList->GetPartList();
        if (parts.empty()) return nullptr;
        
        // make sure all parts belong to the same object
        po = dynamic_cast<GObject*>(parts[0]->Object());
        if (po == nullptr) return nullptr;
        for (int i = 1; i < parts.size(); ++i)
        {
            GObject* poi = dynamic_cast<GObject*>(parts[i]->Object());
            if (poi != po) return nullptr;
        }
        nodes = extractSurfaceNodes(po, parts);
    }
    else if (dynamic_cast<GFaceList*>(sel))
    {
        GFaceList* faceList = dynamic_cast<GFaceList*>(sel);
        vector<GFace*> surfs = faceList->GetFaceList();
        if (surfs.empty()) return nullptr;
        
        // make sure all parts belong to the same object
        po = dynamic_cast<GObject*>(surfs[0]->Object());
        if (po == nullptr) return nullptr;
        for (int i = 1; i < surfs.size(); ++i)
        {
            GObject* poi = dynamic_cast<GObject*>(surfs[i]->Object());
            if (poi != po) return nullptr;
        }
        nodes = extractSurfaceNodes(po, surfs);
    }
    else if (dynamic_cast<GEdgeList*>(sel))
    {
        GEdgeList* edgeList = dynamic_cast<GEdgeList*>(sel);
        vector<GEdge*> edges = edgeList->GetEdgeList();
        if (edges.empty()) return nullptr;
        
        // make sure all parts belong to the same object
        po = dynamic_cast<GObject*>(edges[0]->Object());
        if (po == nullptr) return nullptr;
        for (int i = 1; i < edges.size(); ++i)
        {
            GObject* poi = dynamic_cast<GObject*>(edges[i]->Object());
            if (poi != po) return nullptr;
        }
        nodes = extractSurfaceNodes(po, edges);
    }
    else if (dynamic_cast<FSSurface*>(sel))
    {
        FSSurface* surf = dynamic_cast<FSSurface*>(sel);
        po = surf->GetGObject();
        nodes = extractSurfaceNodes(po, surf);
    }
    else if (dynamic_cast<FSNodeSet*>(sel))
    {
        FSNodeSet* nodeSet = dynamic_cast<FSNodeSet*>(sel);
        po = nodeSet->GetGObject();
        nodes = extractSurfaceNodes(po, nodeSet);
    }
    
    return po;
}

