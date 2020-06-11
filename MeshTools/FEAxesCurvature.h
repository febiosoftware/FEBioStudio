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

//
//  FEAxesCurvature.hpp
//  MeshTools
//
//  Created by Jay Shim on 4/19/18.
//  Copyright © 2018 febio.org. All rights reserved.
//

#pragma once
#include "FEModifier.h"
#include "stdafx.h"
#include <map>

class FEAxesCurvature : public FEModifier
{
public:
    FEAxesCurvature();
    
    FEMesh* Apply(FEMesh* pm);
    
protected:
    //Option to add the fiber axes on surface elements
    void ApplyCurvature(FEMesh* pm);
    
    //applies fiber axes to every element of part
    void ApplyCurvaturePart(FEMesh* pm);
    
    //method for calculating fiber axes
    void Curvature(FEMesh* pm);
    
    //Clears all data structures
    void clearData();
    
protected:
    
    // store list of selected faces in fdata
    vector<FEFace> fdata;
    
    // element corresponding to face
    vector<int> fel;
    
    // element corresponding to part (not including those of face
    vector<int> pel;
    
    // map neighboring faces to face
    vector<vector<int>> nface;
    
    // map nodes to face
    vector<vector<int>> eln;
    
    // map nodes of surrounding faces to each face. At least 10
    vector<vector<int>> ncurve;
    
    // centroid of faces
    vector<vec3d> ctr;
    
    //All parts of selected face
    vector<int> fpart;
    
    //For now used only for applying axes
    vector<mat3d> eigenvecFace;
    vector<vec3d> eigenvalFace;
};
