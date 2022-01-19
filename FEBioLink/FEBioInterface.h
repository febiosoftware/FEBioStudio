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
#pragma once
#include <vector>
#include <string>

class FSModelComponent;
class FSMaterial;
class FEBioMaterial;
class FSMaterialProperty;
class FSDiscreteMaterial;
class FEBioDiscreteMaterial;
class FSStep;
class FSBoundaryCondition;
class FSNodalLoad;
class FSSurfaceLoad;
class FSBodyLoad;
class FSInitialCondition;
class FSPairedInterface;
class FSModelConstraint;
class FSSurfaceConstraint;
class FSRigidConstraint;
class FSRigidConnector;
class FSRigidLoad;
class FSModel;
class FSCoreBase;
class FSLoadController;
class FSFunction1D;
class FSGenericClass;

namespace FEBio {

	// helper functions for creating FEBio classes.
	FSStep*              CreateStep             (const std::string& typeStr, FSModel* fem);
	FSMaterial*          CreateMaterial         (const std::string& typeStr, FSModel* fem);
	FSMaterialProperty*  CreateMaterialProperty (const std::string& typeStr, FSModel* fem);
	FSDiscreteMaterial*  CreateDiscreteMaterial (const std::string& typeStr, FSModel* fem);
	FSBoundaryCondition* CreateBoundaryCondition(const std::string& typeStr, FSModel* fem);
	FSNodalLoad*         CreateNodalLoad        (const std::string& typeStr, FSModel* fem);
	FSSurfaceLoad*       CreateSurfaceLoad      (const std::string& typeStr, FSModel* fem);
	FSBodyLoad*          CreateBodyLoad         (const std::string& typeStr, FSModel* fem);
	FSPairedInterface*   CreatePairedInterface  (const std::string& typeStr, FSModel* fem);
	FSModelConstraint*	 CreateNLConstraint     (const std::string& typeStr, FSModel* fem);
	FSSurfaceConstraint* CreateSurfaceConstraint(const std::string& typeStr, FSModel* fem);
	FSRigidConstraint*	 CreateRigidConstraint  (const std::string& typeStr, FSModel* fem);
	FSRigidConnector*	 CreateRigidConnector   (const std::string& typeStr, FSModel* fem);
	FSRigidLoad*	     CreateRigidLoad        (const std::string& typeStr, FSModel* fem);
	FSInitialCondition*  CreateInitialCondition (const std::string& typeStr, FSModel* fem);
	FSLoadController*    CreateLoadController   (const std::string& typeStr, FSModel* fem);
	FSFunction1D*        CreateFunction1D       (const std::string& typeStr, FSModel* fem);
	FSGenericClass*		 CreateGenericClass     (const std::string& typeStr, FSModel* fem);

	FSModelComponent* CreateClass(int superClassID, const std::string& typeStr, FSModel* fem);
	FSModelComponent* CreateClass(int classId, FSModel* fem);

	template<class T> T* CreateFEBioClass(int classId, FSModel* fem)
	{
		FSModelComponent* pc = CreateClass(classId, fem);
		T* pt = dynamic_cast<T*>(pc);
		if (pt == nullptr) { delete pc; return nullptr; }
		return pt;
	}

	// this is only used by LoadClassMetaData
	bool BuildModelComponent(FSModelComponent* pc, const std::string& typeStr);

	// Call this to initialize default properties
	bool InitDefaultProps(FSModelComponent* pc);

	void UpdateFEBioMaterial(FEBioMaterial* pm);
	void UpdateFEBioDiscreteMaterial(FEBioDiscreteMaterial* pm);
}
