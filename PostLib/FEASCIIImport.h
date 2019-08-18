#pragma once
#include "FEFileReader.h"
#include <list>
#include <vector>
using namespace std;

namespace Post {
//-----------------------------------------------------------------------------
// Imports a Tecplot data text file
class FEASCIIImport : public FEFileReader
{
	enum { MAX_VARS = 16 };
	enum { ZONE_TRIANGLE, ZONE_QUAD, ZONE_TET, ZONE_BRICK };

private:
	struct VARIABLE
	{
		char	szname[128];
	};

	struct NODE
	{
		float	v[MAX_VARS];
	};

	struct ELEM
	{
		int	node[8];
	};

	class ZONE
	{
	public:
		char	szname[128];	// zone name
		int		m_npack;		// packing mode
		int		m_nn;			// number of nodes
		int		m_ne;			// number of elements
		int		m_ntype;		// element type
		double	m_ftime;		// solution time

		vector<NODE>	m_Node;
		vector<ELEM>	m_Elem;

	public:
		ZONE();
		ZONE(const ZONE& z);
		void operator = (const ZONE& z);
	};

public:
	FEASCIIImport(void);
	~FEASCIIImport(void);

	bool Load(FEModel& fem, const char* szfile);

protected:
	bool ReadTitle    ();
	bool ReadVariables();
	bool ReadZone     ();
	bool ReadZoneInfo (ZONE& zone);
	bool ReadNodes   (ZONE& zone);
	bool ReadElements(ZONE& zone);

	bool BuildMesh(FEModel& fem);

	bool getline(char* szline, int nmax);

protected:
	char				m_sztitle[256];
	list<VARIABLE>		m_Var;
	vector<ZONE>		m_Zone;

private:
	char	m_szline[256];
};
}
