#pragma once
#include "FEFileReader.h"

namespace Post {

class FEMeshBase;

class FENikeImport : public FEFileReader
{
public:
	FENikeImport();
	~FENikeImport();

	bool Load(FEModel& fem, const char* szfile);

protected:
	char* get_line(char* szline);

	bool ReadControlSection();
	bool ReadMaterialSection();
	bool ReadGeometrySection();

protected:
	int	m_nmat;	// nr of materials
	int	m_nn;	// nr of nodes
	int m_nhel;	// nr of solid elements
	int	m_nbel;	// nr of beam elements
	int	m_nsel;	// nr of shell elements

	FEModel*	m_pfem;

	FEMeshBase*	m_pm;
};
}
