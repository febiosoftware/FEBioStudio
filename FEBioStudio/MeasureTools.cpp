/*This file is part of the FEBio Studio source code and is licensed under the MIT license
 listed below.
 
 See Copyright-FEBio-Studio.txt for details.
 
 Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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

// This calculates the moment of inertia, but only approximately!
// Requires a fine mesh for accurate results.

#include "stdafx.h"
#include <MeshLib/FSMeshBuilder.h>
#include <MeshLib/MeshTools.h>
#include <FECore/mat3d.h>
#include <FECore/vec3d.h>
#include "MeasureTools.h"
#include <GeomLib/GObject.h>

// This algorithm calculates the center of mass approximately,
// by weighing the element centers by the element volumes.
// TODO: I should look into bringing the numerical integration tools
//       from FEBio to FBS so this can be calculated accurately.
vec3d CalculateCOM(FSMesh& mesh)
{
    vec3d rc(0, 0, 0);
    double Vtotal = 0.0;
    
    int selectedElements = mesh.CountSelectedElements();
    
    // loop over all elements
    int NE = mesh.Elements();
    for (int iel = 0; iel < NE; ++iel)
    {
        FSElement& el = mesh.Element(iel);
        if ((selectedElements == 0) || el.IsSelected())
        {
            // Volume is calculated in global frame
            double V0 = mesh.ElementVolume(el);
            
            // center of mass is calculated in local frame
            vec3d c_local = mesh.ElementCenter(el);
            vec3d c_global = mesh.LocalToGlobal(c_local);
            
            rc += c_global * V0;
            Vtotal += V0;
        }
    }
    if (Vtotal != 0.0)
        rc /= Vtotal;
    
    const double eps = 1e-12;
    double L = rc.Length();
    if (L > eps)
    {
        if (fabs(rc.x) < eps) rc.x = 0;
        if (fabs(rc.y) < eps) rc.y = 0;
        if (fabs(rc.z) < eps) rc.z = 0;
    }
    
    return rc;
}

vec3d CalculateAreaCOM(FSMesh& mesh)
{
    int selectedFaces = mesh.CountSelectedFaces();
    vec3d c = vec3d(0, 0, 0);
    double Atotal = 0.0;
    
    // loop over all elements
    for (int i = 0; i < mesh.Faces(); ++i)
    {
        FSFace& face = mesh.Face(i);
        if ((selectedFaces == 0) || face.IsSelected())
        {
            double Ai = mesh.FaceArea(face);
            vec3d ci = mesh.FaceCenter(face);
            
            Atotal += Ai;
            c += ci * Ai;
        }
    }
    if (Atotal != 0.0)
        c /= Atotal;

    return mesh.LocalToGlobal(c);
}

mat3d CalculateMOI(FSMesh& mesh)
{
    int selectedElements = mesh.CountSelectedElements();
    mat3dd I(1);        // identity tensor
    
    mat3d moi; moi.zero();
    
    // calculate the COM
    vec3d com = CalculateCOM(mesh);
    
    // loop over all elements
    int NE = mesh.Elements();
    for (int iel = 0; iel < NE; ++iel)
    {
        FSElement& el = mesh.Element(iel);
        
        if ((selectedElements == 0) || el.IsSelected()) {
            // ElementCenter is calculated in local frame
            vec3d c_local = mesh.ElementCenter(el);
            vec3d c_global = mesh.LocalToGlobal(c_local);
            
            // Element volume is calculated in global frame
            double V0 = mesh.ElementVolume(el);
            
            vec3d r = c_global - com;
            mat3d Iij = (r * r) * I - (r & r);
            moi += Iij * V0;
        }
    }
    
    const double eps = 1e-12;
    return CleanUp(moi, eps);
}

mat3d CalculateAreaMOI(FSMesh& mesh)
{
    int selectedFaces = mesh.CountSelectedFaces();
    mat3dd I(1);        // identity tensor
    
    mat3d moi; moi.zero();
    
    // calculate the COM
    vec3d com = CalculateAreaCOM(mesh);
    
    // loop over all elements
    for (int i = 0; i < mesh.Faces(); ++i)
    {
        FSFace& face = mesh.Face(i);
        if ((selectedFaces == 0) || face.IsSelected()) {
            double Ai = mesh.FaceArea(face);
            // face center is calculated in local frame
            vec3d ci = mesh.FaceCenter(face);
            
            vec3d cg = mesh.LocalToGlobal(ci);
            
            vec3d r = cg - com;
            mat3d Iij = (r * r) * I - (r & r);
            moi += Iij * Ai;
        }
    }
    
    const double eps = 1e-12;
    return CleanUp(moi, eps);
}

mat3d CleanUp(mat3d& m, const double eps)
{
    double L = m.norm();
    if (L > eps)
    {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                if (fabs(m[i][j]) < eps) m[i][j] = 0.0;
    }
    return m;

}
