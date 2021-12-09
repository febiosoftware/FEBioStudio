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
#include <FEMLib/enums.h>
#include <stdlib.h>
#include "FEModel.h"
#include "PlotDataSettings.h"

//-----------------------------------------------------------------------------
#define FE_STATIC	0
#define FE_DYNAMIC	1

//-----------------------------------------------------------------------------
enum FEPlotLevel {
	FE_PLOT_NEVER,
	FE_PLOT_MAJOR_ITRS,
	FE_PLOT_MINOR_ITRS,
	FE_PLOT_MUST_POINTS,
	FE_PLOT_FINAL,
	FE_PLOT_AUGMENTS,
	FE_PLOT_STEP_FINAL
};

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

	MODULE_ALL = 0xFFFF
};

//-----------------------------------------------------------------------------
// Output for log file
class FELogData
{
public:
	enum { LD_NODE, LD_ELEM, LD_RIGID, LD_CNCTR };

public:
    FELogData(){ matID = -1; groupID = -1; rcID = -1; }
	FELogData(const FELogData& d)
	{
		type = d.type;
		sdata = d.sdata;
		fileName = d.fileName;
		matID = d.matID;
		groupID = d.groupID;
        rcID = d.rcID;
	}
	void operator = (const FELogData& d)
	{
		type = d.type;
		sdata = d.sdata;
		fileName = d.fileName;
		matID = d.matID;
		groupID = d.groupID;
		rcID = d.rcID;
	}

public:
	int			type;			// type of data (node, element, rigid)
	string		sdata;			// data string
	string		fileName;		// file name (optional)
	int			matID;			// for LD_RIGID
	int			groupID;		// for LD_NODE, LD_ELEM
    int         rcID;           // for LD_CNCTR
};

//-----------------------------------------------------------------------------
// class that manages the log file settings
class CLogDataSettings
{
public:
	CLogDataSettings();

	//! save to file
	void Save(OArchive& ar);

	//! load from file
	void Load(IArchive& ar);

public:
	int LogDataSize() { return (int)m_log.size(); }
	FELogData& LogData(int i) { return m_log[i]; }
	void AddLogData(FELogData& d) { m_log.push_back(d); }
	void ClearLogData() { m_log.clear(); }
	void RemoveLogData(int item);

private:
	vector<FELogData>		m_log;		// log data
};

//-----------------------------------------------------------------------------
//! The FEProject class stores all the FE data.
class FEProject : public CSerializable
{
public:
	//! constructor 
	FEProject(void);

	//! class destructor
	virtual ~FEProject(void);

	// get the FE data
	FSModel& GetFSModel() { return m_fem; }

	//! save project to file
	void Save(OArchive& ar);

	//! load project from file
	void Load(IArchive& ar);

	//! Get the project title
	const string& GetTitle() { return m_title; }

	//! set the project title
	void SetTitle(const std::string& title);

	//! reset project data
	void Reset();

	//! validate the project
	int Validate(string& szerr);

	//! Get the plot file settings
	CPlotDataSettings& GetPlotDataSettings() { return m_plt; }

	//! Get the log file settings
	CLogDataSettings& GetLogDataSettings() { return m_log; }

	//! set default plot variables
	void SetDefaultPlotVariables();

	unsigned int GetModule() const;
	void SetModule(unsigned int mod);

	static void InitModules();

private:
	unsigned int		m_module;	// active module
	string				m_title;	// Project Title
	FSModel				m_fem;		// FE model data
	CPlotDataSettings	m_plt;		// plot file settings
	CLogDataSettings	m_log;		// log file settings
};
