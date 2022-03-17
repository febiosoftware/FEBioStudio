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
#include <map>
#include <FECore/vec3d.h>
#include <FECore/mat3d.h>

// Forward declaration of FEBio Studio classes
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
class FESolidFormulation;
class FEShellFormulation;

namespace FEBio {

	// NOTE: This is an exact copy of the FEParamType enum in FEBio (defined in FEParam.h)!
	//       Make sure that this remains so! 
	enum FEBIO_PARAM_TYPE {
		FEBIO_PARAM_INVALID,
		FEBIO_PARAM_INT,
		FEBIO_PARAM_BOOL,
		FEBIO_PARAM_DOUBLE,
		FEBIO_PARAM_VEC2D,
		FEBIO_PARAM_VEC3D,
		FEBIO_PARAM_MAT3D,
		FEBIO_PARAM_MAT3DS,
		FEBIO_PARAM_STRING,
		FEBIO_PARAM_DATA_ARRAY,
		FEBIO_PARAM_TENS3DRS,
		FEBIO_PARAM_STD_STRING,
		FEBIO_PARAM_STD_VECTOR_INT,
		FEBIO_PARAM_STD_VECTOR_DOUBLE,
		FEBIO_PARAM_STD_VECTOR_VEC2D,
		FEBIO_PARAM_STD_VECTOR_STRING,
		FEBIO_PARAM_DOUBLE_MAPPED,
		FEBIO_PARAM_VEC3D_MAPPED,
		FEBIO_PARAM_MAT3D_MAPPED,
		FEBIO_PARAM_MAT3DS_MAPPED,
		FEBIO_PARAM_MATERIALPOINT
	};

	// helper structure for retrieving febio class info 
	struct FEBioClassInfo
	{
		unsigned int	classId;		// the class ID
		int				baseClassId;	// base class index
		const char*		sztype;			// the type string
		const char*		szmod;			// the module name
	};

	enum ClassSearchFlags {
		IncludeModuleDependencies = 0x01,
		IncludeFECoreClasses = 0x02,
		AllFlags = 0xFF
	};

	std::vector<FEBioClassInfo> FindAllClasses(int mod, int superId, int baseClassId = -1, unsigned int flags = ClassSearchFlags::AllFlags);
	std::vector<FEBioClassInfo> FindAllActiveClasses(int superId, int baseClassId = -1, unsigned int flags = ClassSearchFlags::AllFlags);
	int GetClassId(int superClassId, const std::string& typeStr);

	FEBioClassInfo GetClassInfo(int classId);

	// get the base class Index from the base class name
	int GetBaseClassIndex(const std::string& baseClassName);

	// get the base class name from its index (returns nullptr if not found)
	std::string GetBaseClassName(int baseClassIndex);

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
	FEShellFormulation*  CreateShellFormulation (const std::string& typeStr, FSModel* fem);
	FESolidFormulation*  CreateSolidFormulation (const std::string& typeStr, FSModel* fem);

	FSModelComponent* CreateClass(int superClassID, const std::string& typeStr, FSModel* fem);
	FSModelComponent* CreateClass(int classId, FSModel* fem);
	FSModelComponent* CreateFSClass(int superClassID, int baseClassId, FSModel* fem);

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

	bool BuildModelComponent(FSModelComponent* po);

	class FEBioOutputHandler
	{
	public:
		FEBioOutputHandler() {}
		virtual ~FEBioOutputHandler() {}
		virtual void write(const char* sztxt) = 0;
	};

	bool runModel(const std::string& fileName, FEBioOutputHandler* outputHandler = nullptr);
	void TerminateRun();

	const char* GetSuperClassString(int superClassID);

	std::map<int, const char*> GetSuperClassMap();

	vec3d GetMaterialFiber(void* vec3dvaluator, const vec3d& p);

	void DeleteClass(void* p);
}
