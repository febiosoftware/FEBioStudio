#pragma once
#include "FEFileReader.h"
#include <vector>

namespace Post {

class FEState;

class FEVTKimport :	public FEFileReader
{

public:
	FEVTKimport();
	~FEVTKimport(void);

	bool Load(FEPostModel& fem, const char* szfile);

protected:
	bool readPointData(char* ch);
	
protected:
	bool BuildMesh();
	FEPostModel*	m_pfem;
	FEState*		m_ps;
};
}
