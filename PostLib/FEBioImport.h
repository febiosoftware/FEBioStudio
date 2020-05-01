#pragma once
#include "FEFileReader.h"
#include <XML/XMLReader.h>

namespace Post {

class FEPostModel;
class FEPostMesh;

class FEBioImport :	public FEFileReader
{
public:
	FEBioImport(FEPostModel* fem);
	~FEBioImport();

	bool Load(const char* szfile) override;

protected:
	void ParseMaterialSection(FEPostModel& fem, XMLTag& tag);
	void ParseGeometrySection(FEPostModel& fem, XMLTag& tag);
	void ParseGeometrySection2(FEPostModel& fem, XMLTag& tag);
	bool ParseVersion(XMLTag& tag);

protected:
	FEPostMesh*		m_pm;

	int		m_nmat;
	int		m_nversion;

	XMLReader	m_xml;
};
}
