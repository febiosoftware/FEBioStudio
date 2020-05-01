#pragma once
#include "FEFileReader.h"
#include <MathLib/math3d.h>
#include <vector>
#include <list>

namespace Post {

class FESTLimport : public FEFileReader
{
	struct FACET
	{
		vec3f	r[3];
		int		n[3];
	};

public:
	FESTLimport(FEPostModel* fem);
	virtual ~FESTLimport(void);

	bool Load(const char* szfile) override;

protected:
	bool read_line(char* szline, const char* sz);

	void build_mesh();
	int find_node(vec3f& r, const double eps = 1e-12);

protected:
	std::list<FACET>	m_Face;
	std::vector<vec3f>	m_Node;
	int					m_nline;	// line counter
};

}
