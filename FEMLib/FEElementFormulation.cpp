#include "FEElementFormulation.h"

FEElementFormulation::FEElementFormulation(FSModel* fem) : FSModelComponent(fem) {}
FESolidFormulation::FESolidFormulation(FSModel* fem) : FEElementFormulation(fem) {}
FEShellFormulation::FEShellFormulation(FSModel* fem) : FEElementFormulation(fem) {}
