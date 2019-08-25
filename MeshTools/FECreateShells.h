#pragma once
#include "FEModifier.h"

class FECreateShells : public FEModifier
{
public:
    FECreateShells();

    FEMesh* Apply(FEMesh* pm);
    FEMesh* Apply(FEGroup* pg);
    
protected:
    void CreateShells(FEMesh* pm, vector<int>& faceList);
};
