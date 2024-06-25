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

// NOTE: This file should only be included in DlgCreatePlugin!

// This generates the make file needed to build the plugin
const char* szcmake = \
"cmake_minimum_required(VERSION 3.5.0)\n\n" \
"set(CMAKE_CXX_STANDARD 17)\n" \
"set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n" \
"project($(PLUGIN_NAME))\n\n"\
"add_definitions(-DWIN32 -DFECORE_DLL /wd4251)\n\n"\
"include_directories(\"$(PLUGIN_SDK_INCLUDE)\")\n\n"\
"link_directories(\"$(PLUGIN_SDK_LIBS)\")\n\n"\
"add_library($(PLUGIN_NAME) SHARED $(PLUGIN_NAME).h $(PLUGIN_NAME).cpp main.cpp)\n\n"\
"target_link_libraries($(PLUGIN_NAME) fecore.lib febiomech.lib)\n\n"\
"set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT $(PLUGIN_NAME))\n\n"\
"";

// This defines the main.cpp file, which registers feature classes.
const char* szmain = \
"#include <FECore\\FECoreKernel.h>\n" \
"#include \"$(PLUGIN_NAME).h\"\n\n"\
"FECORE_EXPORT unsigned int GetSDKVersion()\n" \
"{\n" \
"	return FE_SDK_VERSION;\n" \
"}\n\n"\
"FECORE_EXPORT void PluginInitialize(FECoreKernel& febio)\n"\
"{\n"\
"	FECoreKernel::SetInstance(&febio);\n\n"\
"	febio.SetActiveModule(\"$(PLUGIN_MODULE)\");\n\n"
"	REGISTER_FECORE_CLASS($(PLUGIN_NAME), \"$(PLUGIN_TYPESTRING)\");\n"\
"}\n";

// This defines the header file for material plugins
const char* szhdr_mat = \
"#include <FEBioMech\\FEElasticMaterial.h>\n\n" \
"class $(PLUGIN_NAME) : public FEElasticMaterial\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(PLUGIN_NAME)(FEModel* fem);\n\n"
"	// evaluate Cauchy stress\n"
"	mat3ds Stress(FEMaterialPoint& mp) override;\n\n" \
"	// evaluate spatial elasticity tangent\n"
"	tens4ds Tangent(FEMaterialPoint& mp) override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

// This defines the source file for material plugins
const char* szsrc_mat = \
"#include \"$(PLUGIN_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(PLUGIN_NAME), FEElasticMaterial)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(PLUGIN_NAME)::$(PLUGIN_NAME)(FEModel* fem) : FEElasticMaterial(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"mat3ds $(PLUGIN_NAME)::Stress(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement stress\n" \
"	mat3ds s;\n" \
"	return s;\n" \
"}\n\n" \
"tens4ds $(PLUGIN_NAME)::Tangent(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement tangent\n" \
"	tens4ds c;\n" \
"	return c;\n" \
"}\n";

// This defines the header file for uncoupled material plugins
const char* szhdr_ucm = \
"#include <FEBioMech\\FEUncoupledMaterial.h>\n\n" \
"class $(PLUGIN_NAME) : public FEUncoupledMaterial\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(PLUGIN_NAME)(FEModel* fem);\n\n"
"	// evaluate Cauchy stress\n"
"	mat3ds DevStress(FEMaterialPoint& mp) override;\n\n" \
"	// evaluate spatial elasticity tangent\n"
"	tens4ds DevTangent(FEMaterialPoint& mp) override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

// This defines the source file for uncoupled material plugins
const char* szsrc_ucm = \
"#include \"$(PLUGIN_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(PLUGIN_NAME), FEUncoupledMaterial)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(PLUGIN_NAME)::$(PLUGIN_NAME)(FEModel* fem) : FEUncoupledMaterial(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"mat3ds $(PLUGIN_NAME)::DevStress(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement stress\n" \
"	mat3ds s;\n" \
"	return s;\n" \
"}\n\n" \
"tens4ds $(PLUGIN_NAME)::DevTangent(FEMaterialPoint& mp)\n" \
"{\n" \
"	FEElasticMaterialPoint& pt = *mp.ExtractData<FEElasticMaterialPoint>();\n\n"\
"	// TODO: implement tangent\n" \
"	tens4ds c;\n" \
"	return c;\n" \
"}\n";

// This defines the header file for meshdata generator plugins
const char* szhdr_mdg = \
"#include <FECore\\FEDataGenerator.h>\n\n" \
"class $(PLUGIN_NAME) : public FEElemDataGenerator\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(PLUGIN_NAME)(FEModel* fem);\n\n"
"	bool Init() override;\n\n" \
"	FEDomainMap* Generate() override;\n\n"
"private:\n"
"	// TODO: Add member variables here\n\n"\
"	DECLARE_FECORE_CLASS();\n"
"};\n";

// This defines the source file for mesh data generator plugins
const char* szsrc_mdg = \
"#include \"$(PLUGIN_NAME).h\"\n\n" \
"BEGIN_FECORE_CLASS($(PLUGIN_NAME), FEElemDataGenerator)\n"\
"	// TODO: Add parameters\n"\
"END_FECORE_CLASS();\n\n"\
"$(PLUGIN_NAME)::$(PLUGIN_NAME)(FEModel* fem) : FEElemDataGenerator(fem)\n"\
"{\n" \
"	// TODO: initialize all class member variables\n" \
"}\n\n" \
"bool $(PLUGIN_NAME)::Init()\n" \
"{\n" \
"	return true;\n" \
"}\n\n" \
"FEDomainMap* $(PLUGIN_NAME)::Generate()\n" \
"{\n" \
"	return nullptr;\n" \
"}\n";

// This defines the header file for node plot data plugins
const char* szhdr_npd = \
"#include <FECore\\FEPlotData.h>\n\n" \
"class $(PLUGIN_NAME) : public FEPlotNodeData\n" \
"{\n" \
"public:\n" \
"	// class constructor\n"
"	$(PLUGIN_NAME)(FEModel* fem) : FEPlotNodeData(fem, PLT_FLOAT, FMT_NODE){}\n"
"	bool Save(FEMesh& m, FEDataStream& a);\n"
"};\n";

// This defines the source file for node plot data plugins
const char* szsrc_npd = \
"#include \"$(PLUGIN_NAME).h\"\n" \
"#include <FECore\\FEMesh.h>\n\n"\
"bool $(PLUGIN_NAME)::Save(FEMesh& m, FEDataStream& a)\n" \
"{\n" \
"	int N = m.Nodes();\n"\
"	for (int i = 0; i < N; ++i)\n"\
"	{\n"\
"		double f;\n"\
"		// TODO: calculate something for f\n"\
"		a << f;\n"\
"	}\n"\
"	return true;\n" \
"}\n";
