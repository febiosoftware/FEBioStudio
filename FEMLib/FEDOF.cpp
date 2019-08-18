#include "FEDOF.h"

//-----------------------------------------------------------------------------
FEDOF::FEDOF(const std::string& name, const std::string& symbol)
{
	m_name = name;
	m_symbol = symbol;
}

FEDOF::FEDOF(const FEDOF& dof)
{
	m_name = dof.m_name;
	m_symbol = dof.m_symbol;
}

void FEDOF::operator=(const FEDOF& dof)
{
	m_name = dof.m_name;
	m_symbol = dof.m_symbol;
}

FEDOFVariable::FEDOFVariable(const char* szname)
{
	m_name = szname;
}

FEDOFVariable::FEDOFVariable(const FEDOFVariable& var)
{
	m_name = var.m_name;
	m_dofs = var.m_dofs;
}

void FEDOFVariable::operator = (const FEDOFVariable& var)
{
	m_name = var.m_name;
	m_dofs = var.m_dofs;
}

void FEDOFVariable::AddDOF(const std::string& name, const std::string& symbol)
{
	FEDOF dof(name, symbol);
	m_dofs.push_back(dof);
}
