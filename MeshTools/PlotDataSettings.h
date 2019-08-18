#pragma once
#include "Serializable.h"
#include "FEItemListBuilder.h"

class FEProject;

//-----------------------------------------------------------------------------
// Output variable for plot file
class FEPlotVariable
{
public:
	FEPlotVariable(int module, const string& name, const string& displayName, bool bactive, bool bshow, DOMAIN_TYPE type);
	FEPlotVariable(const FEPlotVariable& v);
	void operator = (const FEPlotVariable& v);

	const string& name() const { return m_name; }
	const string& displayName() const { return m_displayName; }

	bool isActive() const { return m_bactive; }
	bool isShown() const { return m_bshow; }

	void setActive(bool b) { m_bactive = b; }
	void setShown(bool b) { m_bshow = b; }

	unsigned int domainType() const { return m_domainType; }

	int Domains() const;
	FEItemListBuilder* GetDomain(int i);
	const FEItemListBuilder* GetDomain(int i) const;

	void addDomain(FEItemListBuilder* pi);
	void removeDomain(FEItemListBuilder* pi);
	void removeDomain(int n);

	int GetModule() const { return m_module; }

private:
	int				m_module;					// name of module this variable belongs to
	string			m_name;						// name of variable (as in FEBio file)
	string			m_displayName;				// display name
	bool			m_bactive;					// active flag
	bool			m_bshow;					// show flag
	unsigned int	m_domainType;				// domain type for variable
	vector<FEItemListBuilder*>		m_domains;	// domains 
};

//-----------------------------------------------------------------------------
// Class that manages the plotfile settings
class CPlotDataSettings : public CSerializable
{
public:
	CPlotDataSettings(FEProject& prj);

	//! save to file
	void Save(OArchive& ar);

	//! load from file
	void Load(IArchive& ar);

	//! initialize all plot variables
	void Init();

public:
	int PlotVariables() { return (int)m_plot.size(); }
	FEPlotVariable& PlotVariable(int i) { return m_plot[i]; }
	FEPlotVariable* AddPlotVariable(int module, const std::string& var, bool bactive = false, bool bshown = true, DOMAIN_TYPE type = DOMAIN_MESH);
	FEPlotVariable* FindVariable(const std::string& var, int module = -1);

	void SetAllVariables(bool b);
	void SetAllModuleVariables(int module, bool b);

private:
	FEProject&	m_prj;
	vector<FEPlotVariable>	m_plot;		// plot file variables
};
