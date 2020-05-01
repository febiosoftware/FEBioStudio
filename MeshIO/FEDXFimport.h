#pragma once
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
	FEDXFimport(FEProject& prj);
	virtual ~FEDXFimport();

	bool Load(const char* szfile) override;

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
