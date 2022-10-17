#pragma once
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
// class representing a degree of freedom
// A degree of freedom has a name (which is used by PreView)
// and a symbol, which is used for import/export from FEBio.
class FEDOF
{
public:
	FEDOF(const std::string& name, const std::string& symbol);
	FEDOF(const FEDOF& dof);
	void operator = (const FEDOF& dof);

	const char* name() const { return m_name.c_str(); }
	const char* symbol() const { return m_symbol.c_str(); }

	void SetSymbol(const char* sz) { m_symbol = sz; }

	void SetName(const std::string& name) { m_name = name; }

private:
	std::string m_name;
	std::string m_symbol;
};

//-----------------------------------------------------------------------------
class FEDOFVariable
{
public:
	FEDOFVariable(const std::string& name);
	FEDOFVariable(const FEDOFVariable& var);
	void operator = (const FEDOFVariable& var);

	void AddDOF(const std::string& name, const std::string& symbol);

	void RemoveDOF(int n) { m_dofs.erase(m_dofs.begin() + n); }

	const char* name() const { return m_name.c_str(); }

	int DOFs() const { return (int)m_dofs.size(); }

	FEDOF& GetDOF(int i) { return m_dofs[i]; }
	const FEDOF& GetDOF(int i) const { return m_dofs[i]; }

	void Clear() { m_dofs.clear(); }

private:
	std::string				m_name;
	std::vector<FEDOF>		m_dofs;
};
