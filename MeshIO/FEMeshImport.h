#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEMeshImport : public FEFileImport
{
protected:
	struct NODE
	{
		float x, y, z;
	};

	struct ELEM
	{
		int		ntype;	// 0 = hex, 1 = tet, 2 = tri
		int		n[8];
		int		npid;
	};

public:
	FEMeshImport(FEProject& prj);
	~FEMeshImport();

	bool Load(const char* szfile);

	//! Set the read surface flag
	void ReadSurface(bool b);

protected:
	void ReadNodes(FILE* fp);
	void ReadHex  (FILE* fp);
	void ReadTet  (FILE* fp);
	void ReadTri  (FILE* fp);

	void BuildMesh(FEProject& prj);

protected:
	vector<NODE>	m_Node;
	vector<ELEM>	m_Elem;

private:
	bool	m_bread_surface;
};
