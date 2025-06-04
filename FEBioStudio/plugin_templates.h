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

// NOTE: This file should only be included in ui_createplugin.h!

// Instructions:
// -------------
// This file collects all the template code that is used for generating plugins. 
// The code snippets contain special fields, starting with a dollar sign $, that will be set by the user. 
// The main plugin fields define data that should be defined for all plugins:
//
// $(CLASS_NAME)       : the name of the plugin. Also used as the class name. 
// $(PLUGIN_MODULE)    : the FEBio module to which the class will be added. 
// $(CLASS_TYPESTRING) : the type string for the plugin class. (I.e. how the feature is referenced in the input file)
//
// In addition, the code snippets use additional special fields, denoted $(ARG1), $(ARG2), ...
// These special fields are defined by the plugin generator and may also depend on user input.
// These fields will be substituted by strings that are provided by the plugin generator. 
// Note the these fields are substituted in sequential order. That is, first $(ARG1) will be replaced, then $(ARG2), etc. 


// This generates the makefile (CMakeFile.txt) needed to build the plugin
const char* szcmake =
R"delim(
# CMake Housekeeping
cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_FILES_DIRECTORY ${CMAKE_BINARY_DIR}/CMakeFiles)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Name of your plugin
project($(PLUGIN_NAME))

# Name of the FEBio libraries you need to link to
set(FEBIO_LIB_NAMES $(FEBIO_LIB_NAMES))

# Set a default build type if none was specified
set(default_build_type "Release")
 
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Find FEBio Libs
set(FEBIO_LIBS "")
set(FEBIO_DEBUG_LIBS "")

foreach(name IN LISTS FEBIO_LIB_NAMES)
    find_library(TEMP NAMES ${name} PATHS "$(PLUGIN_SDK_LIBS)Release")
    list(APPEND FEBIO_LIBS ${TEMP})
    unset(TEMP CACHE)

    find_library(TEMP NAMES ${name} PATHS "$(PLUGIN_SDK_LIBS)Debug")
    list(APPEND FEBIO_DEBUG_LIBS ${TEMP})
    unset(TEMP CACHE)
endforeach(name)


##### Set definitions and compile options #####
if(WIN32)
    add_definitions(-DWIN32  -DFECORE_DLL /MP /openmp /wd4251 /wd4275)
elseif(APPLE)
    add_definitions(-D__APPLE__)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
else()
	add_definitions(-DLINUX)
    add_compile_options(-static-libstdc++ -static-libgcc -w -Wall)
    
    set(CMAKE_BUILD_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_BUILD_RPATH $ORIGIN/../lib/)
endif()

# Set include directory
include_directories($(PLUGIN_SDK_INCLUDE))

# Add library
add_library($(PLUGIN_NAME) SHARED $(CLASS_NAME).h $(CLASS_NAME).cpp main.cpp)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT $(PLUGIN_NAME))

# Link FEBio Libraries
foreach(name IN LISTS FEBIO_LIBS)
    target_link_libraries($(PLUGIN_NAME) optimized ${name})
endforeach()

foreach(name IN LISTS FEBIO_DEBUG_LIBS)
    target_link_libraries($(PLUGIN_NAME) debug ${name})
endforeach()

)delim";

// This defines the main.cpp file, which registers feature classes.
const char* szmain =
R"delim(
#include <FECore/FECoreKernel.h>
#include "$(CLASS_NAME).h"

FECORE_EXPORT unsigned int GetSDKVersion()
{
	return FE_SDK_VERSION;
}

FECORE_EXPORT void PluginInitialize(FECoreKernel& febio)
{
	FECoreKernel::SetInstance(&febio);
	febio.SetActiveModule("$(PLUGIN_MODULE)");
	REGISTER_FECORE_CLASS($(CLASS_NAME), "$(CLASS_TYPESTRING)");
}
)delim";

const char* szmain_no_module =
R"delim(
#include <FECore/FECoreKernel.h>
#include "$(CLASS_NAME).h"

FECORE_EXPORT unsigned int GetSDKVersion()
{
	return FE_SDK_VERSION;
}

FECORE_EXPORT void PluginInitialize(FECoreKernel& febio)
{
	FECoreKernel::SetInstance(&febio);
	REGISTER_FECORE_CLASS($(CLASS_NAME), "$(CLASS_TYPESTRING)");
}
)delim";

// ============================================================================
// elastic materials
// ============================================================================
const char* szhdr_mat =
R"delim(
#include <FEBioMech/$(ARG1).h>

class $(CLASS_NAME) : public $(ARG1)
{
public:
	// class constructor
	$(CLASS_NAME)(FEModel* fem);
	
    // evaluate Cauchy stress
	mat3ds $(ARG2)(FEMaterialPoint& mp) override;

	// evaluate spatial elasticity tangent
	tens4ds $(ARG3)(FEMaterialPoint& mp) override;

private:
	// TODO: Add member variables here

	DECLARE_FECORE_CLASS();
};
)delim";

// This defines the source file for material plugins
const char* szsrc_mat =
R"delim(
#include "$(CLASS_NAME).h"

BEGIN_FECORE_CLASS($(CLASS_NAME), $(ARG1))
	// TODO: Add parameters
END_FECORE_CLASS();

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : $(ARG1)(fem)
{
	// TODO: initialize all class member variables
}

mat3ds $(CLASS_NAME)::$(ARG2)(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// TODO: implement stress
	mat3ds s;
	return s;
}

tens4ds $(CLASS_NAME)::$(ARG3)(FEMaterialPoint& mp)
{
	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();

	// TODO: implement tangent
	tens4ds c;
	return c;
}
)delim";

// ============================================================================
// meshdata generator
// ============================================================================
const char* szhdr_mdg =
R"delim(
#include <FECore/FEDataGenerator.h>
#include <FECore/FEDomainMap.h>

class $(CLASS_NAME) : public FEElemDataGenerator
{
public:
	// class constructor
	$(CLASS_NAME)(FEModel* fem);

	bool Init() override;

	FEDomainMap* Generate() override;
private:
	// TODO: Add member variables here

	DECLARE_FECORE_CLASS();
};
)delim";

// This defines the source file for mesh data generator plugins
const char* szsrc_mdg =
R"delim(
#include "$(CLASS_NAME).h"

BEGIN_FECORE_CLASS($(CLASS_NAME), FEElemDataGenerator)
	// TODO: Add parameters
END_FECORE_CLASS();

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FEElemDataGenerator(fem)
{
	// TODO: initialize all class member variables
}

bool $(CLASS_NAME)::Init()
{
	return true;
}

FEDomainMap* $(CLASS_NAME)::Generate()
{
	return nullptr;
}
)delim";

// ============================================================================
// plot data
// ============================================================================
// $(ARG1) : base class
// $(ARG2) : first argument to Save
// $(ARG3) : data type
// $(ARG4) : data format
// $(ARG5) : code snippet
// $(ARG6) : c++ data type
// $(ARG7) : additional include files for source
const char* szhdr_pd =
R"delim(
#include <FECore/FEPlotData.h>

class $(CLASS_NAME) : public $(ARG1)
{
public:
	// class constructor
	$(CLASS_NAME)(FEModel* fem) : $(ARG1)(fem, $(ARG3), $(ARG4)){}
	bool Save($(ARG2), FEDataStream& a);
};
)delim";

// This defines the source file for plot data plugins
const char* szsrc_pd =
R"delim(
#include "$(CLASS_NAME).h"
#include <FECore/FEMesh.h>
$(ARG7)

bool $(CLASS_NAME)::Save($(ARG2), FEDataStream& a)
{
$(ARG5)
	return true;
}
)delim";

// code snippet for node 
const char* szpd_node = 
R"delim(
	int N = mesh.Nodes();
	for (int i = 0; i < N; ++i)
	{
		const FENode& node = mesh.Node(i);
		$(ARG6) v;
		// TODO: calculate something for v
		a << v;
	}
)delim";

// snippet for elem/FMT_ITEM
const char* szpd_elem_item =
R"delim(
	int N = dom.Elements();
	for (int i = 0; i < N; ++i)
	{
		const FEElement& el = dom.ElementRef(i);
		$(ARG6) v;
		// TODO: calculate something for v
		a << v;
	}
)delim";

// snippet for elem/FMT_REGION
const char* szpd_elem_region =
R"delim(
	// TODO: calculate a single value for this domain.
	$(ARG6) v;
	a << v;
)delim";

// snippet for ARG2=FMT_ITEM
const char* szpd_surface_item =
R"delim(
	int N = surf.Elements();
	for (int i = 0; i < N; ++i)
	{
		const FESurfaceElement& el = surf.Element(i);
		$(ARG6) v;
		// TODO: calculate something for v
		a << v;
	}
)delim";

// ============================================================================
// surface load
// ============================================================================
const char* szhdr_sl =
R"delim(
#include <FECore/FESurfaceLoad.h>

class $(CLASS_NAME) : public FESurfaceLoad
{
public:
	$(CLASS_NAME)(FEModel* fem);

	// initialization
	bool Init() override;

	// serialize data
	void Serialize(DumpStream& ar) override;

	// calculate residual
	void LoadVector(FEGlobalVector& R) override;

	// calculate pressure stiffness
	void StiffnessMatrix(FELinearSystem& LS) override;

private:
	// TODO: Add member variables here

	DECLARE_FECORE_CLASS();
};
)delim";

const char* szsrc_sl =
R"delim(
#include "$(CLASS_NAME).h"

BEGIN_FECORE_CLASS($(CLASS_NAME), FESurfaceLoad)
	// TODO: Add parameters
END_FECORE_CLASS();

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FESurfaceLoad(fem)
{
	// TODO: Initialize all class members.
}

bool $(CLASS_NAME)::Init()
{
	// TODO: Do any additional initialization
	return FESurfaceLoad::Init();
}

void $(CLASS_NAME)::Serialize(DumpStream& ar)
{
	FESurfaceLoad::Serialize(ar);
	// TODO: Do any additional serialization
}

void $(CLASS_NAME)::LoadVector(FEGlobalVector& R)
{
}

void $(CLASS_NAME)::StiffnessMatrix(FELinearSystem& LS)
{
}
)delim";

// ============================================================================
// log data
// ============================================================================
const char* szhdr_ld =
R"delim(
#include <FECore/$(ARG1)>

class $(CLASS_NAME) : public $(ARG2)
{
public:
	// class constructor
	$(CLASS_NAME)(FEModel* fem) : $(ARG2)(fem){}
	double value($(ARG3)) override;
};
)delim";

const char* szsrc_ld =
R"delim(
#include "$(CLASS_NAME).h"

double $(CLASS_NAME)::value($(ARG3))
{
	// TODO: calculate something for val
	double val = 0.0;
	return val;
}
)delim";

// ============================================================================
// Callbacks
// ============================================================================
const char* szhdr_cb =
R"delim(
#include <FECore/FECallBack.h>

class $(CLASS_NAME) : public FECallBack
{
public:
	$(CLASS_NAME)(FEModel* fem);

	bool Init() override;

	bool Execute(FEModel& fem, int nwhen) override;

};
)delim";

const char* szsrc_cb =
R"delim(
#include <FECore/Callback.h>
#include "$(CLASS_NAME).h"

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FECallBack(fem, CB_ALWAYS)
{
}

bool $(CLASS_NAME)::Init()
{
	// TODO: Add additional initialization
	return FECallBack::Init();
}

bool $(CLASS_NAME)::Execute(FEModel& fem, int nwhen)
{
	return true;
}
)delim";

// ============================================================================
// Tasks
// ============================================================================
const char* szhdr_task =
R"delim(
#include <FECore/FECoreTask.h>

class $(CLASS_NAME) : public FECoreTask
{
public:
	$(CLASS_NAME)(FEModel* fem);

	bool Init(const char* szfile) override;

	bool Run() override;

};
)delim";

const char* szsrc_task =
R"delim(
#include <FECore/FEModel.h>
#include "$(CLASS_NAME).h"

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FECoreTask(fem)
{
}

bool $(CLASS_NAME)::Init(const char* szfile)
{
	// get the FE model
	FEModel* fem = GetFEModel();

	// TODO: make any necessary changes to the model

	// Then, initialize the model
	return fem->Init();
}

bool $(CLASS_NAME)::Run()
{
	FEModel* fem = GetFEModel();

	// TODO: This function is called by FEBio and should execute the task's logic.
	//       To run the forward model, call FEModel::Solve()
	//       To reset the model, call FEModel::Reset()
	return fem->Solve();
}
)delim";

// ============================================================================
// Linear Solver
// ============================================================================
const char* szhdr_ls =
R"delim(
#include <FECore/LinearSolver.h>

class $(CLASS_NAME) : public LinearSolver
{
public:
	$(CLASS_NAME)(FEModel* fem);

	~$(CLASS_NAME)();

	bool PreProcess() override;

	bool Factor() override;

	bool BackSolve(double* x, double* y) override;

	void Destroy() override;

	SparseMatrix* CreateSparseMatrix(Matrix_Type ntype) override;

	bool SetSparseMatrix(SparseMatrix* pA) override;

};
)delim";

const char* szsrc_ls =
R"delim(
#include "$(CLASS_NAME).h"

$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : LinearSolver(fem)
{
}

$(CLASS_NAME)::~$(CLASS_NAME)()
{
}

bool $(CLASS_NAME)::PreProcess()
{
	return true;
}

bool $(CLASS_NAME)::Factor()
{
	return true;
}

bool $(CLASS_NAME)::BackSolve(double* x, double* y)
{
	return true;
}

void $(CLASS_NAME)::Destroy()
{
}

SparseMatrix* $(CLASS_NAME)::CreateSparseMatrix(Matrix_Type ntype)
{
	return nullptr;
}

bool $(CLASS_NAME)::SetSparseMatrix(SparseMatrix* pA)
{
	return false;
}
)delim";
