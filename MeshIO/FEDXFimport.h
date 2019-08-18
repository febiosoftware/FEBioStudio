// FEDXFimport.h: interface for the FEDXFimport class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEDXFIMPORT_H__332EACF7_70ED_40CA_96B0_029877E761FE__INCLUDED_)
#define AFX_FEDXFIMPORT_H__332EACF7_70ED_40CA_96B0_029877E761FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <list>
using namespace std;

class FEDXFimport : public FEFileImport
{
	enum {MAX_LINE=256};

protected:
	struct NODE
	{
		double x, y, z;
	};

	struct FACE
	{
		int		a, b, c, d;
		int		nodes;
	};

	struct POLYFACE
	{
		list<NODE>	m_Node;
		list<FACE>	m_Face;
	};

	struct FACE3D
	{
		vec3d	r[4];
		int		nn;
	};

public:
	FEDXFimport();
	virtual ~FEDXFimport();

	bool Load(FEProject& prj, const char* szfile);

protected:
	bool SearchFor(const char* sz);

	bool read_POLYLINE();
	bool read_3DFACE();

	void MakePolyFace(list<FACE3D>& Face);

protected:
	char	m_szline[256];

	list<POLYFACE*>	m_Obj;
	list<FACE3D>	m_Face;
	FEModel*		m_pfem;
};

#endif // !defined(AFX_FEDXFIMPORT_H__332EACF7_70ED_40CA_96B0_029877E761FE__INCLUDED_)
