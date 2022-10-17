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

#include "FEAxesCurvature.h"
#include "stdafx.h"
#include "FEDomain.h"
#include <MeshLib/MeshMetrics.h>
#include <map>
#include "PointCloud3d.h"
#include "BivariatePolynomialSpline.h"
#include "Quadric.h"
using namespace std;

//--------------------------------------------------------------------------------------
FEAxesCurvature::FEAxesCurvature() : FEModifier("Axes from Curvature")
{
    // to select the method
    AddChoiceParam(0, "generator")->SetEnumNames("spline\0quadric\0");
    
    //To see vectors on the screen
    AddBoolParam(true, "Activate axes");
    
    //To actually accept and apply
    AddBoolParam(false, "Whole part");
    
    fdata.clear();
    fel.clear();
    nface.clear();
    eln.clear();
    ncurve.clear();
    ctr.clear();
    fpart.clear();
    pel.clear();
    eigenvecFace.clear();
    eigenvalFace.clear();
}

//--------------------------------------------------------------------------------------
void FEAxesCurvature::clearData()
{
    //clear all data structures
    fdata.clear();
    fel.clear();
    nface.clear();
    eln.clear();
    ncurve.clear();
    ctr.clear();
    fpart.clear();
    pel.clear();
    eigenvecFace.clear();
    eigenvalFace.clear();
}

//--------------------------------------------------------------------------------------
FSMesh* FEAxesCurvature::Apply(FSMesh* pm)
{
    FSMesh* pnm = new FSMesh(*pm);
    
    bool apply = GetBoolValue(1);
    bool part = GetBoolValue(2);
    
    clearData();
    
    //Fit surface to spline or quadric
    Curvature(pnm);
    
    //Apply them as fiber axes for each element of face selected
    if(apply || part)
        ApplyCurvature(pnm);
    
    //Apply to all elements of part
    if (part)
        ApplyCurvaturePart(pnm);
    
    return pnm;
}

//--------------------------------------------------------------------------------------
void FEAxesCurvature::Curvature(FSMesh* pm)
{
    // option 0 = spline surface
    // option 1 = quadric surface
    int option = GetIntValue(0);

    // store all selected faces in fdata
    int numFaces = pm->Faces();
    
    for (int i = 0; i<numFaces; ++i)
    {
        FSFace& face = pm->Face(i);
        if (face.IsSelected())
            fdata.push_back(face);
    }
    int ne1 = (int)fdata.size();
    ctr.resize(ne1, vec3d(0,0,0));

    // store element attached to face; store neighboring faces; store face nodes
    for (int i = 0; i<ne1; ++i)
    {
        FSFace face = fdata[i];
        // get element to which this face belongs
        int iel = face.m_elem[0].eid;
        int faceEdges = face.Edges();
        int faceNodes = face.Nodes();

        // store value of element of face
        fel.push_back(iel);
        
        vector<int> temp;
        
        //store neighboring faces to a face
        for (int j = 0; j<faceEdges; ++j)
        {
			FSFace* pfj = pm->FacePtr(face.m_nbr[j]);
            if (pfj && pfj->IsSelected())
            {
                temp.push_back(face.m_nbr[j]);
            }
        }
        
        nface.push_back(temp);
        temp.clear();
        
        //store nodes to a face
        for (int k = 0; k<faceNodes; ++k)
        {
            temp.push_back(face.n[k]);
        }
        eln.push_back(temp);
        temp.clear();
        
        // store centroid of face
        for (int j=0; j<face.Nodes(); ++j)
            ctr[i] += pm->Node(face.n[j]).r;
        ctr[i] /= fdata[i].Nodes();
    }
    
    switch (option) {
        case 0:
        // store nodes of neighboring faces
        {
            for (int i = 0; i<ne1; ++i)
            {
                //For each neighbor face on selected surface, for each node add to vector
                //Unless already included (no duplicates)
                vector<int> temp;
                int numNodesFace = (int)eln[i].size();
                for (int j = 0; j<numNodesFace; ++j)
                {
                    temp.push_back(eln[i][j]);
                }
                int numNextFace = (int)nface[i].size();
                for (int k = 0; k<numNextFace; ++k)
                {
                    int numNodeNext = pm->Face(nface[i][k]).Nodes();
                    for(int l = 0; l<numNodeNext; ++l)
                    {
                        int sizeTemp = (int)temp.size();
                        bool isDuplicate = false;
                        for(int m = 0; m<sizeTemp; ++m)
                        {
                            if (temp[m] == pm->Face(nface[i][k]).n[l])
                            {
                                isDuplicate = true;
                            }
                        }
                        if (!isDuplicate)
                        {
                            temp.push_back(pm->Face(nface[i][k]).n[l]);
                        }
                    }
                }
                
                //If less than ten (can change) add more nodes from neighbors of neighbors
                //Keep adding faces to separate vector until target reached
                int numCurveNodes = (int)temp.size();
                int numNodesDesired = 10;
                vector<int> surroundingFaces = nface[i];
                while (numCurveNodes < numNodesDesired)
                {
                    int numSurrFace = (int)surroundingFaces.size();
                    for (int k = 0; k<numSurrFace; ++k)
                    {
                        int numFacesNextNext = pm->Face(surroundingFaces[k]).Edges();
                        for (int l = 0; l<numFacesNextNext; ++l)
                        {
                            bool isSel = pm->Face(pm->Face(surroundingFaces[k]).m_nbr[l]).IsSelected();
                            if (isSel)
                            {
                                surroundingFaces.push_back(pm->Face(surroundingFaces[k]).m_nbr[l]);
                                int numNodeNext = pm->Face(pm->Face(surroundingFaces[k]).m_nbr[l]).Nodes();
                                
                                //For each node in surrounding face check if duplicate. Includes just added nodes.
                                for(int n = 0; n<numNodeNext; ++n)
                                {
                                    bool isDuplicate = false;
                                    for(int m = 0; m<numCurveNodes; ++m)
                                    {
                                        if (temp[m] == pm->Face(pm->Face(surroundingFaces[k]).m_nbr[l]).n[n])
                                        {
                                            isDuplicate = true;
                                        }
                                    }
                                    if (!isDuplicate)
                                    {
                                        temp.push_back(pm->Face(pm->Face(surroundingFaces[k]).m_nbr[l]).n[n]);
                                        numCurveNodes = (int)temp.size();
                                        //we want it to be symmetric, no bias to certain neighboring nodes
                                        //if (numCurveNodes >= numNodesDesired) break;
                                    }
                                }
                            }
                        }
                    }
                }
                ncurve.push_back(temp);
            }
            break;
        }
            
        case 1:
        // store all nodes of selected faces
        {
            vector<int> temp;
            // mark all nodes on the selected faces
            for (int i = 0; i<pm->Nodes(); ++i) pm->Node(i).m_ntag = -1;
            for (int i = 0; i<ne1; ++i)
                for (int j = 0; j<fdata[i].Nodes(); ++j)
                    pm->Node(fdata[i].n[j]).m_ntag = 1;
            // extract marked nodes
            for (int i = 0; i<pm->Nodes(); ++i)
                if (pm->Node(i).m_ntag == 1) temp.push_back(i);
            ncurve.push_back(temp);
        }
            
        default:
            break;
    }
    
    //fit a quadric surface to the selected points
    
    //Get number of faces
    int numSelectedFaces = (int)fdata.size();
    
    switch (option) {
        case 0: // spline surface
        {
            for (int i = 0; i < numSelectedFaces; ++i)
            {
                int numNodesCurve = (int)ncurve[i].size();
                
                //extract coordinates of each node for surface fit
                PointCloud3d curvePoints;
                
                // store the nodal coordinates into the point cloud
                for (int j = 0; j < numNodesCurve; ++j)
                    curvePoints.AddPoint(pm->Node(ncurve[i][j]).r);
                
                // find parametric coordinates
                curvePoints.ParametricCoordinatesFromPlane();
                
                // performs surface fitting
                BivariatePolynomialSpline spline;
                spline.SetPointCloud3d(&curvePoints);
                spline.SetSplineDegree(2);
                if (spline.GetSplineCoeficients()) {
                    // get centroid of parametric coordinates
                    vec2d uv(0,0);
                    for (int i=0; i<curvePoints.Points(); ++i)
                        uv += curvePoints.m_u[i];
                    uv /= (double)curvePoints.Points();
                    // find curvature at this location
                    vec2d kappa;
                    vec3d theta[2];
                    vec3d faceNorm = to_vec3d(fdata[i].m_fn);
                    spline.SurfaceCurvature(uv.x(), uv.y(), faceNorm, kappa, theta);
                    vec3d xn = (theta[0] ^ theta[1]).Normalize();
                    mat3d X(theta[0].x, theta[1].x, xn.x,
                            theta[0].y, theta[1].y, xn.y,
                            theta[0].z, theta[1].z, xn.z);
                    eigenvecFace.push_back(X);
                }
                else {
                    mat3d X(1, 0, 0,
                            0, 1, 0,
                            0, 0, 1);
                    eigenvecFace.push_back(X);
                }
            }
            break;
        }
            
        case 1: // quadric surface
        {
            // there is only one point set in this case
            int numNodesCurve = (int)ncurve[0].size();
            
            //extract coordinates of each node for surface fit
            PointCloud3d curvePoints;
            
            // store the nodal coordinates into the point cloud
            for (int j = 0; j < numNodesCurve; ++j)
                curvePoints.AddPoint(pm->Node(ncurve[0][j]).r);
            
            // performs surface fitting
            Quadric quadric;
            quadric.SetPointCloud3d(&curvePoints);
            
            if (quadric.GetQuadricCoeficients()) {
                for (int i = 0; i < numSelectedFaces; ++i)
                {
                    // get centroid of face
                    vec3d c = ctr[i];
                    
                    vec3d faceNorm = to_vec3d(fdata[i].m_fn);
                    
                    // get closest point on quadric (using norm)
                    vec3d p = quadric.ClosestPoint(c,faceNorm);
                    
                    // find curvature at this location
                    vec2d kappa;
                    vec3d theta[2];
                    quadric.SurfaceCurvature(p, faceNorm, kappa, theta);
                    vec3d xn = (theta[0] ^ theta[1]).Normalize();
                    mat3d X(theta[0].x, theta[1].x, xn.x,
                            theta[0].y, theta[1].y, xn.y,
                            theta[0].z, theta[1].z, xn.z);
                    eigenvecFace.push_back(X);
                }
            }
            else {
                for (int i = 0; i < numSelectedFaces; ++i)
                {
                    mat3d X(1, 0, 0,
                            0, 1, 0,
                            0, 0, 1);
                    eigenvecFace.push_back(X);
                }
            }
            break;
        }
        default:
            break;
    }
}

//--------------------------------------------------------------------------------------
void FEAxesCurvature::ApplyCurvature(FSMesh* pm)
{
    int numElements = (int)fel.size();
    
    for(int i=0; i<numElements; ++i)
    {
        mat3d eigenVectors = eigenvecFace[i];
        
        vec3d a = vec3d(eigenVectors(0,0),eigenVectors(1,0), eigenVectors(2,0));
        vec3d b = vec3d(eigenVectors(0,1),eigenVectors(1,1), eigenVectors(2,1));
        vec3d c = vec3d(eigenVectors(0,2),eigenVectors(1,2), eigenVectors(2,2));
        
        FSElement& el = pm->Element(fel[i]);
        mat3d& m = el.m_Q;
        m.zero();
        m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
        m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
        m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;
    
        el.m_Qactive = true;
    }
}

//--------------------------------------------------------------------------------------
void FEAxesCurvature::ApplyCurvaturePart(FSMesh* pm)
{
    //Find the face with smallest distance between the face centroid and element centroid
    //of element not on surface in same part. Apply material axes of this closest face's element
    //to the element
    
    //Find all parts of selected faces
    int numFaceElm = (int)fel.size();
    
    for (int i = 0; i < numFaceElm; ++i)
    {
        int numParts = (int)fpart.size();
        bool isPartInc = false;
        for (int j = 0; j < numParts; ++j)
        {
            if (pm->Element(fel[i]).m_gid == fpart[j])
                isPartInc = true;
        }
        if (!isPartInc)
            fpart.push_back(pm->Element(fel[i]).m_gid);
    }
    
    //mark all elements already selected
    for (int i = 0; i<pm->Elements(); ++i) pm->Element(i).m_ntag = -1;
    for (int i = 0; i<numFaceElm; ++i)
        pm->Element(fel[i]).m_ntag = 1;
    
    //Then make for loop for each element of each part that is untagged push back to pel
    for (int i = 0; i<pm->Elements(); ++i)
    {
        bool canApply =false;
        for(int j =0; j < (int)fpart.size(); ++j)
        {
            if((pm->Element(i).m_ntag != 1) &&  (pm->Element(i).m_gid == fpart[j]))
                canApply = true;
        }
        if (canApply)
            pel.push_back(i);
    }
    
    //Find selected face of closest distance to each element of pel, then assign axes of that face
    for (int i = 0; i<(int)pel.size(); ++i)
    {
        //find centroid of element
        vec3d elCent(0,0,0);
        for (int j = 0; j < pm->Element(pel[i]).Nodes(); ++j)
        {
            elCent += pm->Node(pm->Element(pel[i]).m_node[j]).r;
        }
        elCent /= pm->Element(pel[i]).Nodes();
        
        //find closest face to that element
        int closestFace = 0;
        for (int j = 1; j < (int)ctr.size(); ++j)
        {
            double distClose = 0.0;
            double distNew = 0.0;
            distClose = sqrt(pow(elCent.x-ctr[closestFace].x,2)
                             +pow(elCent.y-ctr[closestFace].y,2)
                             +pow(elCent.z-ctr[closestFace].z,2));
            distNew = sqrt(pow(elCent.x-ctr[j].x,2)
                             +pow(elCent.y-ctr[j].y,2)
                             +pow(elCent.z-ctr[j].z,2));
            
            if (distClose > distNew)
                closestFace = j;
        }
        
        //assign same material axes
        pm->Element(pel[i]).m_Q = pm->Element(fel[closestFace]).m_Q;
        pm->Element(pel[i]).m_Qactive = true;
    }
    
}

