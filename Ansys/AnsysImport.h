#pragma once

#include <MeshIO/FileReader.h>
#include <MeshTools/FEProject.h>
#include <list>
using namespace std;

class AnsysImport : public FEFileImport
{
	struct NODE
	{
		int		nid;
		int		n;
		vec3d	r;
	};

	struct ELEM
	{
		int		eid;
		int		nn;
		int		n[20];
		int		tag;
	};

public:
	AnsysImport(FEProject& prj);
	~AnsysImport(void);

	bool read_NBLOCK();
	bool read_EBLOCK();

	bool Load(const char* szfile);

protected:
	bool BuildMesh(FEModel& fem);

protected:
	list<NODE>	m_Node;
	list<ELEM>	m_Elem;

private:
	char	m_szline[256];
};
