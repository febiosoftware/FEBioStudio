#pragma once
#include "FEFileReader.h"
#include <XML/XMLReader.h>

namespace Post {

class FEPostModel;
class FEPostMesh;

class FEBioImport :	public FEFileReader
{
public:
	FEBioImport();
	~FEBioImport();

	bool Load(FEPostModel& fem, const char* szfile);

protected:
	void ParseMaterialSection(FEPostModel& fem, XMLTag& tag);
	void ParseGeometrySection(FEPostModel& fem, XMLTag& tag);
	void ParseGeometrySection2(FEPostModel& fem, XMLTag& tag);
	bool ParseVersion(XMLTag& tag);

protected:
	FEPostModel*	m_pfem;
	FEPostMesh*		m_pm;

	int		m_nmat;
	int		m_nversion;

	XMLReader	m_xml;
};
}
