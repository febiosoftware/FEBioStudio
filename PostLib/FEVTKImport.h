#pragma once
#include "FEFileReader.h"
#include <vector>

namespace Post {

class FEState;

class FEVTKimport :	public FEFileReader
{

public:
	FEVTKimport(FEPostModel* fem);
	~FEVTKimport(void);

	bool Load(const char* szfile) override;

protected:
	bool readPointData(char* ch);
	
protected:
	bool BuildMesh();
	FEState*		m_ps;
};
}
