#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <list>
using namespace std;

class FEIDEASimport : public FEFileImport
{
protected:
	struct NODE
	{
		int		id;
		double	x, y, z;
	};

	struct ELEMENT
	{
		int id;		// element ID
		int	ntype;	// element type
		int	n[27];	// node labels
	};

public:
	FEIDEASimport(FEProject& prj);
	virtual ~FEIDEASimport();

	bool Load(const char* szfile);

protected:
	bool BuildMesh(FEModel& fem);

	bool ReadHeader(bool& bend);
	bool ReadNodes(bool& bend);
	bool ReadElements(bool& bend);

protected:
	list<NODE>		m_Node;
	list<ELEMENT>	m_Elem;
	FEModel*	m_pfem;
};
