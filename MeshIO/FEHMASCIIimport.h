#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
// Reads in an Amira Hypermesh

class FEHMASCIIimport : public FEFileImport
{
protected:
	struct NODE
	{
		vec3d	r;		// nodal position
		int		nid;	// node ID
	};

	struct ELEM
	{
		int		ntype;		// type is the number of nodes
		int		nid;		// element ID
		int		npid;
		int		node[8];
	};

	struct COMPONENT
	{
		char	szname[64];
	};

public:
	FEHMASCIIimport(FEProject& prj);
	~FEHMASCIIimport(void);

	bool Load(const char* szfile);

	void Clear();

	bool BuildMesh(FEModel& fem);

protected:
	list<NODE>			m_Node;
	list<ELEM>			m_Elem;
	list<COMPONENT>		m_Part;
};
