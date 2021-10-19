#pragma once
#include <FEMLib/FEMaterial.h>
#include <FSCore/Archive.h>

class FEMaterialLibrary  
{
	enum { ML_VERSION = 2 };

public:
	struct MATENTRY
	{
		FEMaterial*	m_pmat;
		char		m_szname[256];
	};

public:
	FEMaterialLibrary();
	virtual ~FEMaterialLibrary();

	bool Load(IArchive& ar);
	bool Save(OArchive& ar);

	int Materials() { return (int)m_mat.size(); }

	void Add(const char* sz, FEMaterial* pmat);

	FEMaterial* Material(int i) { return m_mat[i].m_pmat; }
	const char* MaterialName(int i) { return m_mat[i].m_szname; }

protected:
	void Clear();

protected:
	std::vector<MATENTRY>	m_mat;
};
