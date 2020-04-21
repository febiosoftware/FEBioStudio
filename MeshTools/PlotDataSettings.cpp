#include "stdafx.h"
#include "PlotDataSettings.h"
#include "FEProject.h"
#include "GModel.h"

FEPlotVariable::FEPlotVariable(int module, const string& name, const string& displayName, bool bactive, bool bshow, DOMAIN_TYPE type)
{
	m_module = module;
	m_name = name;
	m_displayName = displayName;
	m_bactive = bactive;
	m_bshow = bshow;
	m_domainType = type;
}

FEPlotVariable::FEPlotVariable(const FEPlotVariable& v)
{
	m_module = v.m_module;
	m_name = v.m_name;
	m_displayName = v.m_displayName;
	m_bactive = v.m_bactive;
	m_bshow = v.m_bshow;
	m_domainType = v.m_domainType;
	m_domains = v.m_domains;
}

void FEPlotVariable::operator = (const FEPlotVariable& v)
{
	m_module = v.m_module;
	m_name = v.m_name;
	m_displayName = v.m_displayName;
	m_bactive = v.m_bactive;
	m_bshow = v.m_bshow;
	m_domainType = v.m_domainType;
	m_domains = v.m_domains;
}

int FEPlotVariable::Domains() const
{
	return (int)m_domains.size();
}

FEItemListBuilder* FEPlotVariable::GetDomain(int i)
{
	return m_domains[i];
}

const FEItemListBuilder* FEPlotVariable::GetDomain(int i) const
{
	return m_domains[i];
}

void FEPlotVariable::addDomain(FEItemListBuilder* pi)
{
	// make sure the domain is not already added
	for (size_t i=0; i<m_domains.size(); ++i)
	{
		if (m_domains[i] == pi) return;
	}

	// okay, let's add it
	m_domains.push_back(pi);
}

void FEPlotVariable::removeDomain(FEItemListBuilder* pi)
{
	for (size_t i = 0; i<m_domains.size(); ++i)
	{
		if (m_domains[i] == pi)
		{
			m_domains.erase(m_domains.begin() + i);
		}
	}

	// hmmm, this shouldn't have happened
	assert(false);
}

void FEPlotVariable::removeDomain(int n)
{
	if ((n >= 0) && (n < m_domains.size()))
	{
		m_domains.erase(m_domains.begin() + n);
	}
	else
	{
		// hmmm, this shouldn't have happened
		assert(false);
	}
}

//=================================================================================================
CPlotDataSettings::CPlotDataSettings(FEProject& prj) : m_prj(prj)
{
	Init();
}

void CPlotDataSettings::Init()
{
	m_plot.clear();

	// add default plot file variables
	AddPlotVariable(MODULE_MECH, "acceleration"                      );
	AddPlotVariable(MODULE_MECH, "contact area"                      );
	AddPlotVariable(MODULE_MECH, "contact force"                     );
	AddPlotVariable(MODULE_MECH, "contact gap"                       );
    AddPlotVariable(MODULE_MECH, "contact penalty"                   );
	AddPlotVariable(MODULE_MECH, "contact pressure"                  );
	AddPlotVariable(MODULE_MECH, "contact stick"                     );
	AddPlotVariable(MODULE_MECH, "contact traction"                  );
	AddPlotVariable(MODULE_MECH, "current density"                   );
    AddPlotVariable(MODULE_MECH, "damage"                            );
    AddPlotVariable(MODULE_MECH, "density"                           );
    AddPlotVariable(MODULE_MECH, "deviatoric fiber stretch"          );
	AddPlotVariable(MODULE_MECH, "deviatoric strain energy density"  );
	AddPlotVariable(MODULE_MECH, "displacement"                      );
	AddPlotVariable(MODULE_MECH, "effective elasticity"              );
    AddPlotVariable(MODULE_MECH, "elasticity"                        );
    AddPlotVariable(MODULE_MECH, "element angular momentum"           );
    AddPlotVariable(MODULE_MECH, "element center of mass"            );
    AddPlotVariable(MODULE_MECH, "element kinetic energy"            );
    AddPlotVariable(MODULE_MECH, "element linear momentum"           );
    AddPlotVariable(MODULE_MECH, "element strain energy"             );
    AddPlotVariable(MODULE_MECH, "element stress power"              );
    AddPlotVariable(MODULE_MECH, "enclosed volume"                   , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "fiber stretch"                     );
    AddPlotVariable(MODULE_MECH, "fiber vector"                      );
    AddPlotVariable(MODULE_MECH, "kinetic energy density"            );
    AddPlotVariable(MODULE_MECH, "Lagrange strain"                   );
    AddPlotVariable(MODULE_MECH, "nested damage"                     );
    AddPlotVariable(MODULE_MECH, "nodal contact gap"                 );
    AddPlotVariable(MODULE_MECH, "nodal contact pressure"            );
    AddPlotVariable(MODULE_MECH, "nodal contact traction"            );
    AddPlotVariable(MODULE_MECH, "nodal stress"                      );
    AddPlotVariable(MODULE_MECH, "nodal surface traction"            );
    AddPlotVariable(MODULE_MECH, "nodal vector gap"                  );
    AddPlotVariable(MODULE_MECH, "reaction forces"                   );
    AddPlotVariable(MODULE_MECH, "relative volume");
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
	AddPlotVariable(MODULE_MECH, "Euler angle"                       );
    AddPlotVariable(MODULE_MECH, "shell director"                    );
    AddPlotVariable(MODULE_MECH, "shell relative volume"             );
    AddPlotVariable(MODULE_MECH, "shell strain"                      );
    AddPlotVariable(MODULE_MECH, "shell thickness"                   );
    AddPlotVariable(MODULE_MECH, "SPR stress"                        );
    AddPlotVariable(MODULE_MECH, "specific strain energy"            );
    AddPlotVariable(MODULE_MECH, "strain energy density"             );
    AddPlotVariable(MODULE_MECH, "stress"                            );
    AddPlotVariable(MODULE_MECH, "surface area"                      , false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_MECH, "surface traction"                  );
    AddPlotVariable(MODULE_MECH, "uncoupled pressure"                );
    AddPlotVariable(MODULE_MECH, "vector gap"                        );
    AddPlotVariable(MODULE_MECH, "velocity");

    AddPlotVariable(MODULE_BIPHASIC, "effective fluid pressure"          );
    AddPlotVariable(MODULE_BIPHASIC, "fluid pressure"                    );
    AddPlotVariable(MODULE_BIPHASIC, "fluid flux");
    AddPlotVariable(MODULE_BIPHASIC, "fluid flow rate", false, true, DOMAIN_SURFACE);
    AddPlotVariable(MODULE_BIPHASIC, "fluid force"                       );
    AddPlotVariable(MODULE_BIPHASIC, "osmolarity"                        );
    AddPlotVariable(MODULE_BIPHASIC, "porosity"                          );
    AddPlotVariable(MODULE_BIPHASIC, "pressure gap"                      );
    AddPlotVariable(MODULE_BIPHASIC, "referential solid volume fraction" );

	AddPlotVariable(MODULE_MULTIPHASIC,"effective solute concentration");
/*    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 1 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 2 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 3 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 4 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 5 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 6 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 7 concentration"  , false, false);
    AddPlotVariable(MODULE_MULTIPHASIC,"effective solute 8 concentration"  , false, false);
*/    AddPlotVariable(MODULE_MULTIPHASIC,"electric potential");
    AddPlotVariable(MODULE_MULTIPHASIC,"fixed charge density");
    AddPlotVariable(MODULE_MULTIPHASIC,"referential fixed charge density");
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm referential apparent density");
/*
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm 1 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 2 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 3 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 4 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 5 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 6 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 7 referential apparent density", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 8 referential apparent density", false, false);
*/	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration");
/*	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 1 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 2 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 3 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 4 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 5 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 6 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 7 concentration"               , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"sbm 8 concentration"               , false, false);
*/	
	AddPlotVariable(MODULE_MULTIPHASIC, "solute concentration"); 
/*	AddPlotVariable(MODULE_MULTIPHASIC, "solute 1 concentration", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 2 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 3 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 4 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 5 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 6 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 7 concentration"            , false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 8 concentration"            , false, false);
*/	
	AddPlotVariable(MODULE_MULTIPHASIC, "solute flux"); 
/*	AddPlotVariable(MODULE_MULTIPHASIC,"solute 1 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 2 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 3 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 4 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 5 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 6 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 7 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC,"solute 8 flux", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c1']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c2']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c3']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c4']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c5']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c6']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c7']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "concentration['c8']", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[1]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[2]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[3]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[4]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[5]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[6]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[7]", false, false);
	AddPlotVariable(MODULE_MULTIPHASIC, "sbm concentration[8]", false, false);
*/
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

    AddPlotVariable(MODULE_FLUID_FSI, "nodal relative fluid velocity"     );
    AddPlotVariable(MODULE_FLUID_FSI, "relative fluid velocity"           );
}

//-----------------------------------------------------------------------------
FEPlotVariable* CPlotDataSettings::AddPlotVariable(int module, const std::string& var, bool b, bool s, DOMAIN_TYPE type)
{
	FEPlotVariable v(module, var, var, b, s, type);
	m_plot.push_back(v);
	return &m_plot[ m_plot.size() - 1];
}

//-----------------------------------------------------------------------------
// Find a plot file variable
FEPlotVariable* CPlotDataSettings::FindVariable(const std::string& var, int module)
{
	int N = (int)m_plot.size();
	for (int i = 0; i<N; ++i) 
	{
		FEPlotVariable& pv = m_plot[i];
		if ((module == -1) || (pv.GetModule() == module))
		{
			if (var == m_plot[i].name()) return &m_plot[i];
		}
	}
	return 0;
}

void CPlotDataSettings::SetAllVariables(bool b)
{
	int N = PlotVariables();
	for (int i=0; i<N; ++i)
	{
		FEPlotVariable& var = PlotVariable(i);
		var.setActive(b);
	}
}

void CPlotDataSettings::SetAllModuleVariables(int module, bool b)
{
	int N = PlotVariables();
	for (int i = 0; i<N; ++i)
	{
		FEPlotVariable& var = PlotVariable(i);
		if (var.GetModule() == module)
		{
			var.setActive(b);
		}
	}
}

void CPlotDataSettings::Save(OArchive& ar)
{
	const int N = (int) m_plot.size();
	for (int i = 0; i<N; ++i)
	{
		ar.BeginChunk(CID_PRJ_OUTPUT_VAR);
		{
			FEPlotVariable& v = m_plot[i];
			ar.WriteChunk(CID_PRJ_OUTPUT_VAR_NAME, v.name());
			ar.WriteChunk(CID_PRJ_OUTPUT_VAR_MODULE, v.GetModule());
			int n = (v.isActive()? 1 : 0); ar.WriteChunk(CID_PRJ_OUTPUT_VAR_ACTIVE, n);
			int m = (v.isShown() ? 1 : 0); ar.WriteChunk(CID_PRJ_OUTPUT_VAR_VISIBLE, m);
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
	FEModel& fem = m_prj.GetFEModel();
	GModel& mdl = fem.GetModel();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		if (ar.GetChunkID() == CID_PRJ_OUTPUT_VAR)
		{
			string tmp;
			int n = 0, m = 0, id, module = -1;
			vector<int> dom;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_PRJ_OUTPUT_VAR_NAME   : ar.read(tmp); break;
				case CID_PRJ_OUTPUT_VAR_MODULE : ar.read(module); break;
				case CID_PRJ_OUTPUT_VAR_ACTIVE : ar.read(n); break;
				case CID_PRJ_OUTPUT_VAR_VISIBLE: ar.read(m); break;
				case CID_PRJ_OUTPUT_VAR_DOMAINID: 
					{
						ar.read(id);
						dom.push_back(id);
					}
					break;
				}
				ar.CloseChunk();
			}

			FEPlotVariable* pv = FindVariable(tmp, module);
			if (pv == 0) pv = AddPlotVariable(MODULE_ALL, tmp);

			pv->setActive(n != 0);
			pv->setShown(m != 0);

			for (int i=0; i<dom.size(); ++i)
			{
				FEItemListBuilder* pl = mdl.FindNamedSelection(dom[i]);
				if (pl) pv->addDomain(pl);
			}
		}
		ar.CloseChunk();
	}
}
