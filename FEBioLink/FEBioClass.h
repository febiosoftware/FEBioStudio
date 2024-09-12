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
#include <FEMLib/FEBase.h>

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
class FSBodyConstraint;
class FSRigidBC;
class FSRigidIC;
class FSRigidConnector;
class FSRigidLoad;
class FSModel;
class FSCoreBase;
class FSLoadController;
class FSMeshDataGenerator;
class FSFunction1D;
class FSGenericClass;
class FESolidFormulation;
class FEShellFormulation;
class FEBeamFormulation;
class FSMeshAdaptor;
class FSVec3dValuator;
class FSMat3dValuator;
class FSProject;
class CFEBioJob;

// forward declarations of FECore classes
class FEModel;
class FECoreBase;


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
		unsigned int	classId;		// the class ID (i.e. index into kernel's factory class list)
		int				superClassId;	// the super class
		int				baseClassId;	// base class index
		const char*		sztype;			// the type string
		const char*		szclass;		// the (C++) class name
		const char*		szmod;			// the module name
		int				spec;			// spec ID (i.e. FEBio file version)
	};

	enum ClassSearchFlags {
		IncludeModuleDependencies  = 0x01,
		IncludeFECoreClasses       = 0x02,
		AllFlags = 0xFF
	};

	std::vector<FEBioClassInfo> FindAllClasses(int mod, int superId, int baseClassId = -1, unsigned int flags = ClassSearchFlags::AllFlags);
	std::vector<FEBioClassInfo> FindAllPluginClasses(int allocId);
	std::vector<FEBioClassInfo> FindAllActiveClasses(int superId, int baseClassId = -1, unsigned int flags = ClassSearchFlags::AllFlags);
	int GetClassId(int superClassId, const std::string& typeStr);
	std::vector<std::string> GetModuleDependencies(int mod);

	FEBioClassInfo GetClassInfo(int classId);

	// get the base class Index from the base class name
	int GetBaseClassIndex(const std::string& baseClassName);

	// get the base class index from super class ID and type string
	int GetBaseClassIndex(int superId, const std::string& typeStr);

	// get the base class name from its index (returns nullptr if not found)
	std::string GetBaseClassName(int baseClassIndex);

	// see if pm is derived from a base class
	bool HasBaseClass(FSModelComponent* pm, const char* szbase);

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
	FSModelConstraint*	 CreateModelConstraint  (const std::string& typeStr, FSModel* fem);
	FSModelConstraint*	 CreateNLConstraint     (const std::string& typeStr, FSModel* fem);
	FSSurfaceConstraint* CreateSurfaceConstraint(const std::string& typeStr, FSModel* fem);
	FSBodyConstraint*    CreateBodyConstraint   (const std::string& typeStr, FSModel* fem);
	FSRigidBC*	         CreateRigidBC          (const std::string& typeStr, FSModel* fem);
	FSRigidIC*	         CreateRigidIC          (const std::string& typeStr, FSModel* fem);
	FSRigidConnector*	 CreateRigidConnector   (const std::string& typeStr, FSModel* fem);
	FSRigidLoad*	     CreateRigidLoad        (const std::string& typeStr, FSModel* fem);
	FSInitialCondition*  CreateInitialCondition (const std::string& typeStr, FSModel* fem);
	FSLoadController*    CreateLoadController   (const std::string& typeStr, FSModel* fem);
	FSMeshDataGenerator* CreateNodeDataGenerator(const std::string& typeStr, FSModel* fem);
	FSMeshDataGenerator* CreateEdgeDataGenerator(const std::string& typeStr, FSModel* fem);
	FSMeshDataGenerator* CreateFaceDataGenerator(const std::string& typeStr, FSModel* fem);
	FSMeshDataGenerator* CreateElemDataGenerator(const std::string& typeStr, FSModel* fem);
	FSMeshAdaptor*       CreateMeshAdaptor      (const std::string& typeStr, FSModel* fem);
	FSFunction1D*        CreateFunction1D       (const std::string& typeStr, FSModel* fem);
	FSGenericClass*		 CreateGenericClass     (const std::string& typeStr, FSModel* fem);
	FEShellFormulation*  CreateShellFormulation (const std::string& typeStr, FSModel* fem);
	FESolidFormulation*  CreateSolidFormulation (const std::string& typeStr, FSModel* fem);
	FEBeamFormulation*   CreateBeamFormulation  (const std::string& typeStr, FSModel* fem);
	FSVec3dValuator*     CreateVec3dValuator    (const std::string& typeStr, FSModel* fem);
	FSMat3dValuator*     CreateMat3dValuator    (const std::string& typeStr, FSModel* fem);
	FSGenericClass*      CreateLinearSolver     (const std::string& typeStr, FSModel* fem);

	FSModelComponent* CreateClass(int superClassID, const std::string& typeStr, FSModel* fem, unsigned int flags = FSProperty::TOPLEVEL);
	FSModelComponent* CreateClass(int classId, FSModel* fem, unsigned int flags = 0);
	FSModelComponent* CreateFSClass(int superClassID, int baseClassId, FSModel* fem);

	template<class T> T* CreateFEBioClass(int classId, FSModel* fem, unsigned int flags = FSProperty::TOPLEVEL)
	{
		FSModelComponent* pc = CreateClass(classId, fem, flags);
		T* pt = dynamic_cast<T*>(pc);
		if (pt == nullptr) { delete pc; return nullptr; }
		return pt;
	}

	// this is only used by LoadClassMetaData
	bool BuildModelComponent(FSModelComponent* pc, const std::string& typeStr, unsigned int flags);

	// Call this to initialize default properties
	bool InitDefaultProps(FSModelComponent* pc);

	bool UpdateFEBioClass(FSModelComponent* pc);
	void UpdateFEBioMaterial(FEBioMaterial* pm);
	void UpdateFEBioDiscreteMaterial(FEBioDiscreteMaterial* pm);

	bool BuildModelComponent(FSModelComponent* po, unsigned int flags);

	class FEBioOutputHandler
	{
	public:
		FEBioOutputHandler() {}
		virtual ~FEBioOutputHandler() {}
		virtual void write(const char* sztxt) = 0;
	};

	class FEBioProgressTracker
	{
	public:
		FEBioProgressTracker() {}
		virtual ~FEBioProgressTracker() {};
		virtual void SetProgress(double pct) = 0;
	};

	int runModel(const std::string& fileName, 
		FEBioOutputHandler* outputHandler,
		FEBioProgressTracker* progressTracker,
		CFEBioJob* job);

	void TerminateRun();

	const char* GetSuperClassString(int superClassID);

	std::map<unsigned int, const char*> GetSuperClassMap();

	vec3d GetMaterialFiber(void* vec3dvaluator, const vec3d& p);
	mat3d GetMaterialAxis (void* mat3dvaluator, const vec3d& p);

	void DeleteClass(void* p);

	FECoreBase* CreateFECoreClassFromModelComponent(FSModelComponent* pmc, FEModel* fem);

	FSModelComponent* CloneModelComponent(FSModelComponent* pmc, FSModel* fem);

	void SetActiveProject(FSProject* prj);
}
