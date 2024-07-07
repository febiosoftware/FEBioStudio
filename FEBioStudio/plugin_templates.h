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
const char* szcmake = \
"cmake_minimum_required(VERSION 3.5.0)\n\n" \
"set(CMAKE_CXX_STANDARD 17)\n" \
"set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n" \
"project($(PLUGIN_NAME))\n\n"\
"add_definitions(-DWIN32 -DFECORE_DLL /wd4251 /wd4275)\n\n"\
"include_directories(\"$(PLUGIN_SDK_INCLUDE)\")\n\n"\
"link_directories(\"$(PLUGIN_SDK_LIBS)\")\n\n"\
"add_library($(PLUGIN_NAME) SHARED $(CLASS_NAME).h $(CLASS_NAME).cpp main.cpp)\n\n"\
"target_link_libraries($(PLUGIN_NAME) fecore.lib febiomech.lib)\n\n"\
"set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT $(PLUGIN_NAME))\n\n"\
"";

// This defines the main.cpp file, which registers feature classes.
const char* szmain = \
"#include <FECore\\FECoreKernel.h>\n" \
"#include \"$(CLASS_NAME).h\"\n\n"\
"FECORE_EXPORT unsigned int GetSDKVersion()\n" \
"{\n" \
"	return FE_SDK_VERSION;\n" \
"}\n\n"\
"FECORE_EXPORT void PluginInitialize(FECoreKernel& febio)\n"\
"{\n"\
"	FECoreKernel::SetInstance(&febio);\n\n"\
"	febio.SetActiveModule(\"$(PLUGIN_MODULE)\");\n\n"
"	REGISTER_FECORE_CLASS($(CLASS_NAME), \"$(CLASS_TYPESTRING)\");\n"\
"}\n";

// ============================================================================
// elastic materials
// ============================================================================
const char* szhdr_mat = \
"#include <FEBioMech\\$(ARG1).h>\n\n" \
"class $(CLASS_NAME) : public $(ARG1)\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(CLASS_NAME)(FEModel* fem);\n\n"
"	// evaluate Cauchy stress\n"
"	mat3ds $(ARG2)(FEMaterialPoint& mp) override;\n\n" \
"	// evaluate spatial elasticity tangent\n"
"	tens4ds $(ARG3)(FEMaterialPoint& mp) override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

// This defines the source file for material plugins
const char* szsrc_mat = \
"#include \"$(CLASS_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(CLASS_NAME), $(ARG1))\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : $(ARG1)(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"mat3ds $(CLASS_NAME)::$(ARG2)(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement stress\n" \
"	mat3ds s;\n" \
"	return s;\n" \
"}\n\n" \
"tens4ds $(CLASS_NAME)::$(ARG3)(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement tangent\n" \
"	tens4ds c;\n" \
"	return c;\n" \
"}\n";

// ============================================================================
// meshdata generator
// ============================================================================
const char* szhdr_mdg = \
"#include <FECore\\FEDataGenerator.h>\n\n" \
"class $(CLASS_NAME) : public FEElemDataGenerator\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(CLASS_NAME)(FEModel* fem);\n\n"
"	bool Init() override;\n\n" \
"	FEDomainMap* Generate() override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

// This defines the source file for mesh data generator plugins
const char* szsrc_mdg = \
"#include \"$(CLASS_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(CLASS_NAME), FEElemDataGenerator)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FEElemDataGenerator(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"bool $(CLASS_NAME)::Init()\n" \
"{\n" \
"	return true;\n" \
"}\n\n" \
"FEDomainMap* $(CLASS_NAME)::Generate()\n" \
"{\n" \
"	return nullptr;\n" \
"}\n";

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
const char* szhdr_pd = \
"#include <FECore/FEPlotData.h>\n\n" \
"class $(CLASS_NAME) : public $(ARG1)\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(CLASS_NAME)(FEModel* fem) : $(ARG1)(fem, $(ARG3), $(ARG4)){}\n"
"	bool Save($(ARG2), FEDataStream& a);\n"
"};\n";

// This defines the source file for plot data plugins
const char* szsrc_pd = \
"#include \"$(CLASS_NAME).h\"\n" \
"#include <FECore/FEMesh.h>\n"\
"$(ARG7)\n"
"bool $(CLASS_NAME)::Save($(ARG2), FEDataStream& a)\n" \
"{\n" \
"$(ARG5)"
"	return true;\n" \
"}\n";

// code snippet for node 
const char* szpd_node = 
"	int N = mesh.Nodes();\n"\
"	for (int i = 0; i < N; ++i)\n"\
"	{\n"\
"		const FENode& node = mesh.Node(i);\n"\
"		$(ARG6) v;\n"\
"		// TODO: calculate something for v\n"\
"		a << v;\n"\
"	}\n";

// snippet for elem/FMT_ITEM
const char* szpd_elem_item = \
"	int N = dom.Elements();\n"\
"	for (int i = 0; i < N; ++i)\n"\
"	{\n"\
"		const FEElement& el = dom.ElementRef(i);\n"\
"		$(ARG6) v;\n"\
"		// TODO: calculate something for v\n"\
"		a << v;\n"\
"	}\n";

// snippet for elem/FMT_REGION
const char* szpd_elem_region = \
"	// TODO: calculate a single value for this domain.\n"\
"	$(ARG6) v;\n"\
"	a << v;\n";

// snippet for ARG2=FMT_ITEM
const char* szpd_surface_item = \
"	int N = surf.Elements();\n"\
"	for (int i = 0; i < N; ++i)\n"\
"	{\n"\
"		const FESurfaceElement& el = surf.Element(i);\n"\
"		$(ARG6) v;\n"\
"		// TODO: calculate something for v\n"\
"		a << v;\n"\
"	}\n";

// ============================================================================
// surface load
// ============================================================================
const char* szhdr_sl = \
"#include <FECore\\FESurfaceLoad.h>\n\n" \
"class $(CLASS_NAME) : public FESurfaceLoad\n" \
"{\n" \
"public:\n" \
"	$(CLASS_NAME)(FEModel* fem);\n\n"\
"	// initialization\n"\
"	bool Init() override;\n\n"\
"	// serialize data\n"\
"	void Serialize(DumpStream& ar) override;\n\n"\
"	// calculate residual\n"\
"	void LoadVector(FEGlobalVector& R) override;\n\n"\
"	// calculate pressure stiffness\n"\
"	void StiffnessMatrix(FELinearSystem& LS) override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

const char* szsrc_sl = \
"#include \"$(CLASS_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(CLASS_NAME), FESurfaceLoad)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FESurfaceLoad(fem)\n"\
"{\n"\
"	// TODO: Initialize all class members.\n"\
"}\n\n"\
"bool $(CLASS_NAME)::Init()\n"\
"{\n"\
"	// TODO: Do any additional initialization\n"\
"	return FESurfaceLoad::Init();\n"
"}\n\n"\
"void $(CLASS_NAME)::Serialize(DumpStream& ar)\n"\
"{\n"\
"	FESurfaceLoad::Serialize(ar);\n"
"	// TODO: Do any additional serialization\n"\
"}\n\n"\
"void $(CLASS_NAME)::LoadVector(FEGlobalVector& R)\n"\
"{\n"\
"}\n\n"\
"void $(CLASS_NAME)::StiffnessMatrix(FELinearSystem& LS)\n"\
"{\n"\
"}\n";

// ============================================================================
// log data
// ============================================================================
const char* szhdr_ld = \
"#include <FECore\\$(ARG1)>\n\n" \
"class $(CLASS_NAME) : public $(ARG2)\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(CLASS_NAME)(FEModel* fem) : $(ARG2)(fem){}\n"
"	double value($(ARG3)) override;\n"
"};\n";

const char* szsrc_ld = \
"#include \"$(CLASS_NAME).h\"\n\n" \
"double $(CLASS_NAME)::value($(ARG3))\n" \
"{\n" \
"	// TODO: calculate something for val\n"\
"	double val = 0.0;\n"\
"	return val;\n" \
"}\n";

// ============================================================================
// Callbacks
// ============================================================================
const char* szhdr_cb = \
"#include <FECore\\FECallBack.h>\n\n" \
"class $(CLASS_NAME) : public FECallBack\n" \
"{\n" \
"public:\n" \
"	$(CLASS_NAME)(FEModel* fem);\n\n"\
"	bool Init() override;\n\n"\
"	bool Execute(FEModel& fem, int nwhen) override;\n\n"\
"};\n";

const char* szsrc_cb = \
"#include <FECore/Callback.h>\n"\
"#include \"$(CLASS_NAME).h\"\n\n"\
"$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FECallBack(fem, CB_ALWAYS)\n"\
"{\n"\
"}\n\n"\
"bool $(CLASS_NAME)::Init()\n"\
"{\n"\
"	// TODO: Add additional initialization\n"\
"	return FECallBack::Init();\n"\
"}\n\n"\
"bool $(CLASS_NAME)::Execute(FEModel& fem, int nwhen)\n"\
"{\n"\
"	return true;\n"\
"}\n";

// ============================================================================
// Tasks
// ============================================================================
const char* szhdr_task = \
"#include <FECore\\FECoreTask.h>\n\n" \
"class $(CLASS_NAME) : public FECoreTask\n" \
"{\n" \
"public:\n" \
"	$(CLASS_NAME)(FEModel* fem);\n\n"\
"	bool Init(const char* szfile) override;\n\n"\
"	bool Run() override;\n\n"\
"};\n";

const char* szsrc_task = \
"#include <FECore/FEModel.h>\n"\
"#include \"$(CLASS_NAME).h\"\n\n"\
"$(CLASS_NAME)::$(CLASS_NAME)(FEModel* fem) : FECoreTask(fem)\n"\
"{\n"\
"}\n\n"\
"bool $(CLASS_NAME)::Init(const char* szfile)\n"\
"{\n"\
"	// get the FE model\n"\
"	FEModel* fem = GetFEModel();\n\n"\
"	// TODO: make any necessary changes to the model\n\n"\
"	// Then, initialize the model\n"\
"	return fem->Init();\n"\
"}\n\n"\
"bool $(CLASS_NAME)::Run()\n"\
"{\n"\
"	FEModel* fem = GetFEModel();\n\n"\
"	// TODO: This function is called by FEBio and should execute the task's logic.\n"\
"	//       To run the forward model, call FEModel::Solve()\n"\
"	//       To reset the model, call FEModel::Reset()\n"\
"	return fem->Solve();\n"\
"}\n";
