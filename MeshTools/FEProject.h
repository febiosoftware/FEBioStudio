#pragma once
#include <FSCore/Serializable.h>
#include <stdlib.h>
#include "FEModel.h"
#include "PlotDataSettings.h"

//-----------------------------------------------------------------------------
#define FE_STATIC	0
#define FE_DYNAMIC	1

//-----------------------------------------------------------------------------
#define FE_PLOT_DEFAULT		0
#define FE_PLOT_NEVER		1
#define FE_PLOT_MAJOR_ITRS	2
#define FE_PLOT_MINOR_ITRS	3
#define FE_PLOT_MUST_POINTS	4

//-----------------------------------------------------------------------------
// FE Super classes
enum FESuperClass
{
	FE_ANALYSIS,
	FE_ESSENTIAL_BC,
	FE_SURFACE_LOAD,
	FE_BODY_LOAD,
	FE_INITIAL_CONDITION,
	FE_INTERFACE,
	FE_RIGID_CONSTRAINT,
	FE_RIGID_CONNECTOR,
	FE_CONSTRAINT
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
		matID = d.matID;
		groupID = d.groupID;
        rcID = d.rcID;
	}

public:
	int			type;			// type of data (node, element, rigid)
	string		sdata;			// data string
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
	FEModel& GetFEModel() { return m_fem; }

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

	//! activate plot variables for a new step
	void ActivatePlotVariables(FEAnalysisStep* step);

	unsigned int GetModule() const;
	void SetModule(unsigned int mod);

	static void InitModules();

private:
	unsigned int		m_module;	// active module
	string				m_title;	// Project Title
	FEModel				m_fem;		// FE model data
	CPlotDataSettings	m_plt;		// plot file settings
	CLogDataSettings	m_log;		// log file settings
};
