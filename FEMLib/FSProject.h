/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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
#include <FSCore/Serializable.h>
#include <FECore/fecore_enum.h>
#include <stdlib.h>
#include "FSModel.h"
#include "PlotDataSettings.h"
#include "LogDataSettings.h"
#include <ostream>

//-----------------------------------------------------------------------------
#define FE_STATIC	0
#define FE_DYNAMIC	1

//-----------------------------------------------------------------------------
// Module flags
// Note that these are single bit flags! 
// A particular project can use several of these flags to activate functionality
enum MODULE_FLAG
{
	MODULE_MECH					= 0x0001,
	MODULE_HEAT					= 0x0002,
	MODULE_BIPHASIC				= 0x0004,	
	MODULE_SOLUTES				= 0x0008,
	MODULE_MULTIPHASIC			= 0x0010,
	MODULE_FLUID				= 0x0020,
	MODULE_REACTIONS			= 0x0040,
    MODULE_FLUID_FSI			= 0x0080,
	MODULE_REACTION_DIFFUSION	= 0x0100,
    MODULE_POLAR_FLUID          = 0x0120,
    MODULE_FLUID_SOLUTES        = 0x0140,
    MODULE_THERMO_FLUID         = 0x0160,

	MODULE_ALL = 0xFFFF
};

//! The FSProject class stores all the FE data.
class FSProject : public CSerializable
{
public:
	//! constructor 
	FSProject(void);

	//! class destructor
	virtual ~FSProject(void);

	// get the FE data
	FSModel& GetFSModel() { return m_fem; }

	//! save project to file
	void Save(OArchive& ar);

	//! load project from file
	void Load(IArchive& ar);

	//! Get the project title
	const std::string& GetTitle() { return m_title; }

	//! set the project title
	void SetTitle(const std::string& title);

	//! reset project data
	void Reset();

	//! validate the project
	int Validate(std::string& szerr);

	//! Get the plot file settings
	CPlotDataSettings& GetPlotDataSettings() { return m_fem.GetPlotDataSettings(); }

	//! Get the log file settings
	CLogDataSettings& GetLogDataSettings() { return m_fem.GetLogDataSettings(); }

	//! set default plot variables
	void SetDefaultPlotVariables();

	int GetModule() const;
	void SetModule(int mod, bool setDefaultPlotVariables = true);

	std::string GetModuleName() const;

	static void InitModules();

	void SetUnits(int units);
	int GetUnits() const;

    void GetActivePluginIDs(std::unordered_set<int>& allocatorIDs);

public:
	// convert the old format to the new
	void ConvertToNewFormat(std::ostream& log);

	void ConvertMaterial(GMaterial* pm, std::ostream& log);

protected:
	void ConvertMaterials(std::ostream& log);
	bool ConvertSteps(std::ostream& log);
	bool ConvertDiscrete(std::ostream& log);
	void ConvertStepSettings(std::ostream& log, FEBioAnalysisStep& febStep, FSAnalysisStep& oldStep);
	void ConvertStep(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepBCs(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepICs(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepLoads(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepContact(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepRigidConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertStepRigidConnectors(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertLinearConstraints(std::ostream& log, FSStep& newStep, FSStep& oldStep);
	void ConvertMeshAdaptors(std::ostream& log, FSStep& newStep, FSStep& oldStep);

private:
	std::string			m_title;	// Project Title
	FSModel				m_fem;		// FE model data
	int					m_module;	// active module
	int					m_units;	// unit system (read from feb file)
};
