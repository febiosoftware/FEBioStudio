//
//  FESelectElementsFromFaces.hpp
//  MeshTools
//
//  Created by Gerard Ateshian on 6/29/18.
//  Copyright © 2018 febio.org. All rights reserved.
//

#ifndef FESelectElementsFromFaces_hpp
#define FESelectElementsFromFaces_hpp

#include "FEModifier.h"

//-----------------------------------------------------------------------------
// This is a routine for selecting elements lying under selected faces
class FESelectElementsFromFaces : public FEModifier
{
public:
    FESelectElementsFromFaces();
    
    FEMesh* Apply(FEMesh* pm);
    
protected:
    void SelectElementsFromFaces(FEMesh* pm);
};

#endif /* FESelectElementsFromFaces_hpp */
