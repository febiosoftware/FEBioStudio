#pragma once
#include <FEMLib/FECoreMaterial.h>

class FEUserMaterial : public FEMaterial
{
	enum { SZTYPE, PARAMDATA, PARAMNAME, PARAMVALUE, PARAMTYPE };

public:
	FEUserMaterial(int ntype) : FEMaterial(ntype) {}
	~FEUserMaterial();

	void SetTypeString(const char* sz) override;
	const char* TypeStr() override { return m_sztype; }

	void AddParameter(const char* szname, double v);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void copy(FEMaterial* pm);

protected:
	char			m_sztype[256];	// type name
	vector<char*>	m_pname;		// list of parameter names
};
