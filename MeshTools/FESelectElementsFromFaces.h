#pragma once
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
