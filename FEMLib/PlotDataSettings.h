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
#include <MeshLib/FEItemListBuilder.h>

class FSProject;

//-----------------------------------------------------------------------------
// Output variable for plot file
// TODO: inherit from IHasItemList?
class CPlotVariable
{
public:
	CPlotVariable(const std::string& name, bool bactive, bool bshow, DOMAIN_TYPE type);
	~CPlotVariable();
	CPlotVariable(const CPlotVariable& v);
	void operator = (const CPlotVariable& v);

	const std::string& name() const { return m_name; }
	void setName(const std::string& s) { m_name = s; }

	bool isActive() const { return m_bactive; }
	bool isShown() const { return m_bshow; }
	bool isCustom() const { return m_bcustom; }

	void setActive(bool b) { m_bactive = b; }
	void setShown(bool b) { m_bshow = b; }
	void setCustom(bool b) { m_bcustom = b; }

	unsigned int domainType() const { return m_domainType; }
	void setDomainType(DOMAIN_TYPE domainType) { m_domainType = domainType; }

	int Domains() const;
	FEItemListBuilder* GetDomain(int i);
	const FEItemListBuilder* GetDomain(int i) const;

	void addDomain(FEItemListBuilder* pi);
	void removeDomain(FEItemListBuilder* pi);
	void removeDomain(int n);

private:
	std::string		m_name;						// name of variable (as in FEBio file)
	bool			m_bactive;					// active flag
	bool			m_bshow;					// show flag
	bool			m_bcustom;					// user defined
	unsigned int	m_domainType;				// domain type for variable
	std::vector<FEItemListBuilder*>		m_domains;	// domains 
};

//-----------------------------------------------------------------------------
// Class that manages the plotfile settings
class CPlotDataSettings : public CSerializable
{
public:
	CPlotDataSettings(FSProject& prj);

	//! save to file
	void Save(OArchive& ar);

	//! load from file
	void Load(IArchive& ar);

	//! initialize all plot variables
	void Init();

	// Clear all plot variables
	void Clear();

public:
	int PlotVariables() { return (int)m_plot.size(); }
	CPlotVariable& PlotVariable(int i) { return m_plot[i]; }
	CPlotVariable* AddPlotVariable(const std::string& var, bool bactive = false, bool bshown = true, DOMAIN_TYPE type = DOMAIN_MESH);
	void AddPlotVariable(CPlotVariable& var);
	CPlotVariable* FindVariable(const std::string& var);

private:
	FSProject&	m_prj;
	std::vector<CPlotVariable>	m_plot;		// plot file variables
};
