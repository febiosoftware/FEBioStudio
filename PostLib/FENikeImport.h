#pragma once
#include "FEFileReader.h"

namespace Post {

class FEPostMesh;

class FENikeImport : public FEFileReader
{
public:
	FENikeImport(FEPostModel* fem);
	~FENikeImport();

	bool Load(const char* szfile) override;

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

	FEPostMesh*	m_pm;
};
}
