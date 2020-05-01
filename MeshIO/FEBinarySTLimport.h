#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

#include <vector>
#include <list>
using namespace std;

class FEBinarySTLimport : public FEFileImport
{
	struct FACET
	{
		float	norm[3];
		float	v1[3];
		float	v2[3];
		float	v3[3];

		int		n[3];
		int		nid;
	};

	struct NODE
	{
		vec3d	r;
		int		n;
	};

	class OBOX
	{
	public:
		vec3d		r0, r1;		// points defining box
		vector<int>	NL;			// list of nodes inside this box

	public:
		OBOX(){}
		OBOX(const OBOX& b) 
		{
			r0 = b.r0; r1 = b.r1;
			NL = b.NL; 
		}
	};

public:
	FEBinarySTLimport(FEProject& prj);
	virtual ~FEBinarySTLimport(void);

	bool Load(const char* szfile);

protected:
	bool read_facet(FACET& f);

	void build_mesh();
	int find_node(vec3d& r, const double eps = 1e-14);
	int FindBox(vec3d& r);

	::BOX BoundingBox();

protected:
	FEModel*		m_pfem;
	list<FACET>		m_Face;
	vector<NODE>	m_Node;

	int				m_NB;
	vector<OBOX>	m_BL;		// box lists

	BOX				m_box;		// bounding box
};
