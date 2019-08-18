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
