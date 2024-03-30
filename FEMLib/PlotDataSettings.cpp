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

FEItemListBuilder* CPlotVariable::GetDomain(int i)
{
	return m_domains[i];
}

const FEItemListBuilder* CPlotVariable::GetDomain(int i) const
{
	return m_domains[i];
}

void CPlotVariable::addDomain(FEItemListBuilder* pi)
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

void CPlotVariable::removeDomain(FEItemListBuilder* pi)
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

//=================================================================================================
CPlotDataSettings::CPlotDataSettings(FSProject& prj) : m_prj(prj)
{
	Init();
}

void CPlotDataSettings::Init()
{
	m_plot.clear();
/*
	// add default plot file variables
	AddPlotVariable(MODULE_MECH, "acceleration"                      );
	AddPlotVariable(MODULE_MECH, "contact area"                      , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "contact force"                     , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "contact gap"                       , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "contact penalty"                   , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "contact pressure"                  , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "contact stick"                     , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "contact traction"                  , false, true, DOMAIN_SURFACE);
	AddPlotVariable(MODULE_MECH, "current density"                   );
    AddPlotVariable(MODULE_MECH, "damage"                            );
    AddPlotVariable(MODULE_MECH, "density"                           );
    AddPlotVariable(MODULE_MECH, "deviatoric fiber stretch"          );
	AddPlotVariable(MODULE_MECH, "deviatoric strain energy density"  );
    AddPlotVariable(MODULE_MECH, "deviatoric strong bond SED"        );
    AddPlotVariable(MODULE_MECH, "deviatoric weak bond SED"          );
    AddPlotVariable(MODULE_MECH, "weak bond SED"                     );
	AddPlotVariable(MODULE_MECH, "discrete element stretch"          );
	AddPlotVariable(MODULE_MECH, "discrete element force"            );
	AddPlotVariable(MODULE_MECH, "displacement"                      );
	AddPlotVariable(MODULE_MECH, "effective elasticity"              );
    AddPlotVariable(MODULE_MECH, "elasticity"                        );
    AddPlotVariable(MODULE_MECH, "element angular momentum"          );
    AddPlotVariable(MODULE_MECH, "element center of mass"            );
    AddPlotVariable(MODULE_MECH, "element kinetic energy"            );
    AddPlotVariable(MODULE_MECH, "element linear momentum"           );
    AddPlotVariable(MODULE_MECH, "element strain energy"             );
    AddPlotVariable(MODULE_MECH, "element stress power"              );
    AddPlotVariable(MODULE_MECH, "enclosed volume"                   , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "fatigue bond fraction"             );
    AddPlotVariable(MODULE_MECH, "fiber stretch"                     );
    AddPlotVariable(MODULE_MECH, "fiber vector"                      );
    AddPlotVariable(MODULE_MECH, "intact bond fraction"              );
    AddPlotVariable(MODULE_MECH, "kinetic energy density"            );
    AddPlotVariable(MODULE_MECH, "Lagrange strain"                   );
    AddPlotVariable(MODULE_MECH, "infinitesimal strain"              );
    AddPlotVariable(MODULE_MECH, "left Hencky"                       );
    AddPlotVariable(MODULE_MECH, "left stretch"                      );
    AddPlotVariable(MODULE_MECH, "material axes"                     );
    AddPlotVariable(MODULE_MECH, "nested damage"                     );
    AddPlotVariable(MODULE_MECH, "nodal contact gap"                 , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "nodal contact pressure"            , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "nodal contact traction"            , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "nodal stress"                      );
    AddPlotVariable(MODULE_MECH, "nodal surface traction"            , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "nodal vector gap"                  );
    AddPlotVariable(MODULE_MECH, "octahedral plastic strain"         );
    AddPlotVariable(MODULE_MECH, "rate of deformation"               );
    AddPlotVariable(MODULE_MECH, "reaction forces"                   );
    AddPlotVariable(MODULE_MECH, "relative volume");
    AddPlotVariable(MODULE_MECH, "right Hencky"                      );
    AddPlotVariable(MODULE_MECH, "right stretch"                     );
    AddPlotVariable(MODULE_MECH, "rigid acceleration"                );
    AddPlotVariable(MODULE_MECH, "rigid angular acceleration"        );
    AddPlotVariable(MODULE_MECH, "rigid angular momentum"            );
    AddPlotVariable(MODULE_MECH, "rigid angular position"            );
    AddPlotVariable(MODULE_MECH, "rigid angular velocity"            );
    AddPlotVariable(MODULE_MECH, "rigid Euler"                       );
    AddPlotVariable(MODULE_MECH, "rigid force"                       );
    AddPlotVariable(MODULE_MECH, "rigid kinetic energy"              );
    AddPlotVariable(MODULE_MECH, "rigid linear momentum"             );
    AddPlotVariable(MODULE_MECH, "rigid position"                    );
    AddPlotVariable(MODULE_MECH, "rigid rotation vector"             );
    AddPlotVariable(MODULE_MECH, "rigid torque"                      );
    AddPlotVariable(MODULE_MECH, "rigid velocity"                    );
    AddPlotVariable(MODULE_MECH, "RVE generations"                   );
    AddPlotVariable(MODULE_MECH, "RVE reforming bonds"               );
    AddPlotVariable(MODULE_MECH, "RVE strain"                        );
	AddPlotVariable(MODULE_MECH, "Euler angle"                       );
    AddPlotVariable(MODULE_MECH, "shell director"                    );
    AddPlotVariable(MODULE_MECH, "shell relative volume"             );
    AddPlotVariable(MODULE_MECH, "shell strain"                      );
    AddPlotVariable(MODULE_MECH, "shell thickness"                   );
    AddPlotVariable(MODULE_MECH, "SPR stress"                        );
    AddPlotVariable(MODULE_MECH, "specific strain energy"            );
    AddPlotVariable(MODULE_MECH, "strain energy density"             );
    AddPlotVariable(MODULE_MECH, "strong bond SED"                   );
    AddPlotVariable(MODULE_MECH, "weak bond SED"                     );
    AddPlotVariable(MODULE_MECH, "stress"                            );
    AddPlotVariable(MODULE_MECH, "PK1 stress"                        );
    AddPlotVariable(MODULE_MECH, "PK2 stress"                        );
    AddPlotVariable(MODULE_MECH, "surface area"                      , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "facet area"                        , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "surface traction"                  , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "uncoupled pressure"                );
    AddPlotVariable(MODULE_MECH, "vector gap"                        );
    AddPlotVariable(MODULE_MECH, "velocity"                          );
    AddPlotVariable(MODULE_MECH, "yielded bond fraction"             );

    AddPlotVariable(MODULE_BIPHASIC, "effective fluid pressure"          );
    AddPlotVariable(MODULE_BIPHASIC, "effective friction coefficient"    );
    AddPlotVariable(MODULE_BIPHASIC, "fluid pressure"                    );
    AddPlotVariable(MODULE_BIPHASIC, "fluid flux");
    AddPlotVariable(MODULE_BIPHASIC, "fluid flow rate", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_BIPHASIC, "fluid force"                       );
    AddPlotVariable(MODULE_BIPHASIC, "fluid load support"                );
    AddPlotVariable(MODULE_BIPHASIC, "local fluid load support"          );
    AddPlotVariable(MODULE_BIPHASIC, "permeability"                      );
    AddPlotVariable(MODULE_BIPHASIC, "porosity"                          );
    AddPlotVariable(MODULE_BIPHASIC, "pressure gap"                      );
    AddPlotVariable(MODULE_BIPHASIC, "referential solid volume fraction" );
    AddPlotVariable(MODULE_BIPHASIC, "solid stress"                      );

	AddPlotVariable(MODULE_MULTIPHASIC, "effective solute concentration"  );
    AddPlotVariable(MODULE_MULTIPHASIC, "electric potential"              );
    AddPlotVariable(MODULE_MULTIPHASIC, "fixed charge density"            );
    AddPlotVariable(MODULE_MULTIPHASIC, "osmolarity"                      );
    AddPlotVariable(MODULE_MULTIPHASIC, "osmotic coefficient"             );
    AddPlotVariable(MODULE_MULTIPHASIC, "referential fixed charge density");
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm referential apparent density");
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration");
    AddPlotVariable(MODULE_MULTIPHASIC, "solid stress"                    );
	AddPlotVariable(MODULE_MULTIPHASIC, "solute concentration");
	AddPlotVariable(MODULE_MULTIPHASIC, "solute flux");

    AddPlotVariable(MODULE_HEAT, "temperature");
	AddPlotVariable(MODULE_HEAT, "heat flux"  );

	AddPlotVariable(MODULE_REACTION_DIFFUSION, "concentration");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "effective concentration");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "actual concentration");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "concentration flux");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "sbs concentration");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "sbs apparent density");
	AddPlotVariable(MODULE_REACTION_DIFFUSION, "solid volume fraction");

    AddPlotVariable(MODULE_FLUID, "fluid acceleration"                );
    AddPlotVariable(MODULE_FLUID, "fluid density"                     );
    AddPlotVariable(MODULE_FLUID, "fluid dilatation"                  );
    AddPlotVariable(MODULE_FLUID, "fluid element angular momentum"    );
    AddPlotVariable(MODULE_FLUID, "fluid element center of mass"      );
    AddPlotVariable(MODULE_FLUID, "fluid element kinetic energy"      );
    AddPlotVariable(MODULE_FLUID, "fluid element linear momentum"     );
    AddPlotVariable(MODULE_FLUID, "fluid element strain energy"       );
    AddPlotVariable(MODULE_FLUID, "fluid energy density"              );
    AddPlotVariable(MODULE_FLUID, "fluid heat supply density"         );
    AddPlotVariable(MODULE_FLUID, "fluid kinetic energy density"      );
    AddPlotVariable(MODULE_FLUID, "fluid mass flow rate", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_FLUID, "fluid pressure"                    );
    AddPlotVariable(MODULE_FLUID, "fluid rate of deformation"         );
    AddPlotVariable(MODULE_FLUID, "fluid shear viscosity"             );
    AddPlotVariable(MODULE_FLUID, "fluid strain energy density"       );
    AddPlotVariable(MODULE_FLUID, "fluid stress"                      );
    AddPlotVariable(MODULE_FLUID, "fluid stress power density"        );
    AddPlotVariable(MODULE_FLUID, "fluid surface force", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_FLUID, "fluid surface energy flux", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_FLUID, "fluid surface traction power", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_FLUID, "fluid velocity"                    );
    AddPlotVariable(MODULE_FLUID, "fluid volume ratio"                );
    AddPlotVariable(MODULE_FLUID, "fluid vorticity"                   );
    AddPlotVariable(MODULE_FLUID, "nodal fluid velocity"              );

    AddPlotVariable(MODULE_FLUID_FSI, "nodal fluid flux"              );
    AddPlotVariable(MODULE_FLUID_FSI, "fluid flux"                    );
    AddPlotVariable(MODULE_FLUID_FSI, "permeability"                  );
    AddPlotVariable(MODULE_FLUID_FSI, "porosity"                      );
    AddPlotVariable(MODULE_FLUID_FSI, "relative fluid velocity"       );
    AddPlotVariable(MODULE_FLUID_FSI, "solid stress"                  );
*/
}

//-----------------------------------------------------------------------------
void CPlotDataSettings::Clear()
{
	m_plot.clear();
}

//-----------------------------------------------------------------------------
CPlotVariable* CPlotDataSettings::AddPlotVariable(const std::string& var, bool b, bool s, DOMAIN_TYPE type)
{
	CPlotVariable* pv = FindVariable(var);
	if (pv) return pv;

	CPlotVariable v(var, b, s, type);
	m_plot.push_back(v);
	return &m_plot[ m_plot.size() - 1];
}

//-----------------------------------------------------------------------------
void CPlotDataSettings::AddPlotVariable(CPlotVariable& var)
{
	CPlotVariable* pv = FindVariable(var.name());
	if (pv)
	{
		pv->setActive(var.isActive());
	}
	else m_plot.push_back(var);
}

//-----------------------------------------------------------------------------
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
				FEItemListBuilder* pl = v.GetDomain(j);
				if (pl) ar.WriteChunk(CID_PRJ_OUTPUT_VAR_DOMAINID, pl->GetID());
			}
		}
		ar.EndChunk();
	}
}

void CPlotDataSettings::Load(IArchive& ar)
{
	FSModel& fem = m_prj.GetFSModel();
	GModel& mdl = fem.GetModel();
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
				FEItemListBuilder* pl = mdl.FindNamedSelection(dom[i]);
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
