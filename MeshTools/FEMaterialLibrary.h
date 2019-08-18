// FEMaterialLibrary.h: interface for the FEMaterialLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEMATERIALLIBRARY_H__2C8D0AEF_B83D_47E5_9871_9D682879EF59__INCLUDED_)
#define AFX_FEMATERIALLIBRARY_H__2C8D0AEF_B83D_47E5_9871_9D682879EF59__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FEMaterial.h"
#include "Archive.h"

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

#endif // !defined(AFX_FEMATERIALLIBRARY_H__2C8D0AEF_B83D_47E5_9871_9D682879EF59__INCLUDED_)
