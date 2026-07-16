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

#include "stdafx.h"
#include "PlotDataSettings.h"
#include "FSProject.h"
#include <GeomLib/GModel.h>
using namespace std;

CPlotVariable::CPlotVariable(const string& name, bool bactive, bool bshow, DOMAIN_TYPE type)
{
	m_name = name;
	m_bactive = bactive;
	m_bshow = bshow;
	m_bcustom = false;
	m_domainType = type;
}

CPlotVariable::~CPlotVariable()
{
	for (auto it : m_domains) it->DecRef();
	m_domains.clear();
}

CPlotVariable::CPlotVariable(const CPlotVariable& v)
{
	m_name = v.m_name;
	m_bactive = v.m_bactive;
	m_bshow = v.m_bshow;
	m_bcustom = v.m_bcustom;
	m_domainType = v.m_domainType;
	m_domains = v.m_domains;
	for (auto it : m_domains) it->IncRef();
}

void CPlotVariable::operator = (const CPlotVariable& v)
{
	m_name = v.m_name;
	m_bactive = v.m_bactive;
	m_bshow = v.m_bshow;
	m_bcustom = v.m_bcustom;
	m_domainType = v.m_domainType;
	m_domains = v.m_domains;
	for (auto it : m_domains) it->IncRef();
}

int CPlotVariable::Domains() const
{
	return (int)m_domains.size();
}

FSItemListBuilder* CPlotVariable::GetDomain(int i)
{
	return m_domains[i];
}

const FSItemListBuilder* CPlotVariable::GetDomain(int i) const
{
	return m_domains[i];
}

void CPlotVariable::addDomain(FSItemListBuilder* pi)
{
	// make sure the domain is not already added
	for (size_t i=0; i<m_domains.size(); ++i)
	{
		if (m_domains[i] == pi) return;
	}

	// okay, let's add it
	m_domains.push_back(pi);
	pi->IncRef();
}

void CPlotVariable::removeDomain(FSItemListBuilder* pi)
{
	for (size_t i = 0; i<m_domains.size(); ++i)
	{
		if (m_domains[i] == pi)
		{
			pi->DecRef();
			m_domains.erase(m_domains.begin() + i);
		}
	}

	// hmmm, this shouldn't have happened
	assert(false);
}

void CPlotVariable::removeDomain(int n)
{
	if ((n >= 0) && (n < m_domains.size()))
	{
		m_domains[n]->DecRef();
		m_domains.erase(m_domains.begin() + n);
	}
	else
	{
		// hmmm, this shouldn't have happened
		assert(false);
	}
}

void CPlotVariable::removeAllDomains()
{
	m_domains.clear();
}

//=================================================================================================
CPlotDataSettings::CPlotDataSettings(FSModel& mdl) : m_fsm(mdl)
{
	Init();
}

void CPlotDataSettings::Init()
{
	m_plot.clear();
}

void CPlotDataSettings::Clear()
{
	m_plot.clear();
}

CPlotVariable* CPlotDataSettings::AddPlotVariable(const std::string& var, bool b, bool s, DOMAIN_TYPE type)
{
	CPlotVariable* pv = FindVariable(var);
	if (pv)
	{
		pv->setActive(b);
		pv->setShown(s);
		return pv;
	}

	CPlotVariable v(var, b, s, type);
	m_plot.push_back(v);
	return &m_plot[ m_plot.size() - 1];
}

void CPlotDataSettings::AddPlotVariable(CPlotVariable& var)
{
	CPlotVariable* pv = FindVariable(var.name());
	if (pv)
	{
		pv->setActive(var.isActive());
	}
	else m_plot.push_back(var);
}

// Find a plot file variable
CPlotVariable* CPlotDataSettings::FindVariable(const std::string& var)
{
	int N = (int)m_plot.size();
	for (int i = 0; i<N; ++i) 
	{
		CPlotVariable& pv = m_plot[i];
		if (var == m_plot[i].name()) return &m_plot[i];
	}
	return 0;
}

void CPlotDataSettings::Save(OArchive& ar)
{
	const int N = (int) m_plot.size();
	for (int i = 0; i<N; ++i)
	{
		ar.BeginChunk(CID_PRJ_OUTPUT_VAR);
		{
			CPlotVariable& v = m_plot[i];
			ar.WriteChunk(CID_PRJ_OUTPUT_VAR_NAME, v.name());
			ar.WriteChunk(CID_PRJ_OUTPUT_VAR_DOMAINTYPE, v.domainType());
			int n = (v.isActive()? 1 : 0); ar.WriteChunk(CID_PRJ_OUTPUT_VAR_ACTIVE , n);
			int m = (v.isShown() ? 1 : 0); ar.WriteChunk(CID_PRJ_OUTPUT_VAR_VISIBLE, m);
			int c = (v.isCustom()? 1 : 0); ar.WriteChunk(CID_PRJ_OUTPUT_VAR_CUSTOM , c);
			for (int j=0; j<v.Domains(); ++j)
			{
				FSItemListBuilder* pl = v.GetDomain(j);
				if (pl) ar.WriteChunk(CID_PRJ_OUTPUT_VAR_DOMAINID, pl->GetID());
			}
		}
		ar.EndChunk();
	}
}

void CPlotDataSettings::Load(IArchive& ar)
{
	GModel& mdl = m_fsm.GetModel();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		if (ar.GetChunkID() == CID_PRJ_OUTPUT_VAR)
		{
			string tmp;
			int n = 0, m = 0, c = 0, domainType = 0, id, module = -1;
			std::vector<int> dom;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PRJ_OUTPUT_VAR_NAME   : ar.read(tmp); break;
				case CID_PRJ_OUTPUT_VAR_DOMAINTYPE: ar.read(domainType); break;
				case CID_PRJ_OUTPUT_VAR_ACTIVE : ar.read(n); break;
				case CID_PRJ_OUTPUT_VAR_VISIBLE: ar.read(m); break;
				case CID_PRJ_OUTPUT_VAR_CUSTOM : ar.read(c); break;
				case CID_PRJ_OUTPUT_VAR_DOMAINID: 
					{
						ar.read(id);
						dom.push_back(id);
					}
					break;
				}
				ar.CloseChunk();
			}

			CPlotVariable* pv = FindVariable(tmp);
			if (pv == 0) pv = AddPlotVariable(tmp);

			pv->setActive(n != 0);
			pv->setShown(m != 0);
			pv->setCustom(c != 0);
			pv->setDomainType((DOMAIN_TYPE)domainType);

			for (int i=0; i<dom.size(); ++i)
			{
				FSItemListBuilder* pl = mdl.FindNamedSelection(dom[i]);
				if (pl)
				{
					pv->addDomain(pl);

					// The domain type was not store, so we'll have to use
					// some heuristics to determine it. 
					if (dynamic_cast<FSSurface*>(pl)) pv->setDomainType(DOMAIN_SURFACE);
				}
			}
		}
		ar.CloseChunk();
	}
}

void CPlotDataSettings::InitDefaultPlotVariables(const std::string& moduleName)
{
	if (moduleName == "solid")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("stress", true);
		AddPlotVariable("relative volume", true);
	}
	else if (moduleName == "biphasic")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("stress", true);
		AddPlotVariable("relative volume", true);
		AddPlotVariable("solid stress", true);
		AddPlotVariable("effective fluid pressure", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("fluid flux", true);
	}
	else if (moduleName == "heat")
	{
		AddPlotVariable("temperature", true);
		AddPlotVariable("heat flux", true);
	}
	else if ((moduleName == "multiphasic") || (moduleName == "solute"))
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("stress", true);
		AddPlotVariable("relative volume", true);
		AddPlotVariable("solid stress", true);
		AddPlotVariable("fluid flux", true);
		AddPlotVariable("effective fluid pressure", true);
		AddPlotVariable("effective solute concentration", true);
		AddPlotVariable("solute concentration", true);
		AddPlotVariable("solute flux", true);
	}
	else if (moduleName == "fluid")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("nodal fluid velocity", true);
		AddPlotVariable("fluid stress", true);
		AddPlotVariable("fluid velocity", true);
		AddPlotVariable("fluid acceleration", true);
		AddPlotVariable("fluid vorticity", true);
		AddPlotVariable("fluid rate of deformation", true);
		AddPlotVariable("fluid dilatation", true);
		AddPlotVariable("fluid volume ratio", true);
	}
	else if (moduleName == "fluid-FSI")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("velocity", true);
		AddPlotVariable("acceleration", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("fluid stress", true);
		AddPlotVariable("fluid velocity", true);
		AddPlotVariable("fluid acceleration", true);
		AddPlotVariable("fluid vorticity", true);
		AddPlotVariable("fluid rate of deformation", true);
		AddPlotVariable("fluid dilatation", true);
		AddPlotVariable("fluid volume ratio", true);
		AddPlotVariable("nodal fluid flux", true);
	}
	else if (moduleName == "fluid-solutes")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("effective fluid pressure", true);
		AddPlotVariable("effective solute concentration", true);
		AddPlotVariable("fluid acceleration", true);
		AddPlotVariable("fluid dilatation", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("fluid rate of deformation", true);
		AddPlotVariable("fluid stress", true);
		AddPlotVariable("fluid velocity", true);
		AddPlotVariable("fluid volume ratio", true);
		AddPlotVariable("fluid vorticity", true);
		AddPlotVariable("nodal fluid velocity", true);
		AddPlotVariable("solute concentration", true);
		AddPlotVariable("solute flux", true);
	}
	else if (moduleName == "thermo-fluid")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("effective fluid pressure", true);
		AddPlotVariable("fluid acceleration", true);
		AddPlotVariable("fluid dilatation", true);
		AddPlotVariable("fluid heat flux", true);
		AddPlotVariable("fluid isobaric specific heat capacity", true);
		AddPlotVariable("fluid isochoric specific heat capacity", true);
		AddPlotVariable("nodal fluid temperature", true);
		AddPlotVariable("nodal fluid velocity", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("fluid rate of deformation", true);
		AddPlotVariable("fluid specific free energy", true);
		AddPlotVariable("fluid specific entropy", true);
		AddPlotVariable("fluid specific internal energy", true);
		AddPlotVariable("fluid specific gauge enthalpy", true);
		AddPlotVariable("fluid specific free enthalpy", true);
		AddPlotVariable("fluid specific strain energy", true);
		AddPlotVariable("fluid stress", true);
		AddPlotVariable("fluid temperature", true);
		AddPlotVariable("fluid thermal conductivity", true);
		AddPlotVariable("fluid velocity", true);
		AddPlotVariable("fluid volume ratio", true);
		AddPlotVariable("fluid vorticity", true);
	}
	else if (moduleName == "polar fluid")
	{
		AddPlotVariable("displacement", true);
		AddPlotVariable("fluid pressure", true);
		AddPlotVariable("nodal fluid velocity", true);
		AddPlotVariable("fluid stress", true);
		AddPlotVariable("fluid velocity", true);
		AddPlotVariable("fluid acceleration", true);
		AddPlotVariable("fluid vorticity", true);
		AddPlotVariable("fluid rate of deformation", true);
		AddPlotVariable("fluid dilatation", true);
		AddPlotVariable("fluid volume ratio", true);
		AddPlotVariable("nodal polar fluid angular velocity", true);
		AddPlotVariable("polar fluid stress", true);
		AddPlotVariable("polar fluid couple stress", true);
		AddPlotVariable("polar fluid angular velocity", true);
		AddPlotVariable("polar fluid relative angular velocity", true);
		AddPlotVariable("polar fluid regional angular velocity", true);
	}
}
