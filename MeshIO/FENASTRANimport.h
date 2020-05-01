#pragma once

#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <list>
using namespace std;

class FENASTRANimport : public FEFileImport
{
protected:
	enum {MAX_ITEMS = 32};
	struct CARD
	{
		char	szitem[MAX_ITEMS][17];
		int		nitems;
	};

	struct GRID
	{
		double	x, y, z;
	};

	struct ELEM
	{
		int	pid;
		int	nn;
		int	n[20];
	};

	struct PSOLID
	{
		int	pid;
		int	mid;
	};

	struct MAT1
	{
		int	mid;
		double	E, v, d;
	};

public:
	FENASTRANimport(FEProject& prj) : FEFileImport(prj) {}
	bool Load(const char* szfile);

protected:
	bool ParseFile();

	char* get_line(char* szline);
	bool read_card(CARD& c, char* szline);

	bool read_GRID  (CARD& c);
	bool read_CTETRA(CARD& c);
	bool read_PSOLID(CARD& c);
	bool read_MAT1  (CARD& c);
	bool read_CHEXA (CARD& c);

	bool BuildMesh(FEModel& fem);

protected:
	list<GRID>		m_Node;
	list<ELEM>		m_Elem;
	list<PSOLID>	m_Part;
	list<MAT1>		m_Mat;
};
