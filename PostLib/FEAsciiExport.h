#pragma once
#include "FEModel.h"

namespace Post {

class FEASCIIExport
{
public:
	FEASCIIExport();
	bool Save(FEModel* pfem, int n0, int n1, const char* szfile);

public:
	bool	m_bselonly;	// export selection only
	bool	m_bcoords;	// export nodal coords
	bool	m_belem;	// element connectivity
	bool	m_bface;	// facet connectivity
	bool	m_bndata;	// export nodal values
	bool	m_bedata;	// export element data
	bool	m_bfnormals;	// export facet normals
	char	m_szfmt[256];	// format string
};
}
