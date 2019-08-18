//
//  FECreateShells.h
//  MeshTools
//
//  Created by Martin Xiberras on 5/31/18.


#pragma once
#include "FEModifier.h"
#include <GeomLib/GMeshObject.h>

class FECreateShells : public FEModifier
{
public:
    FECreateShells();

    FEMesh* Apply(FEMesh* pm);
    FEMesh* Apply(FEGroup* pg);
    
protected:
    void CreateShells(FEMesh* pm, vector<int>& faceList);
};

