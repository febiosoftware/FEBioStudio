#pragma once
#include <MeshIO/FileWriter.h>

class CModelDocument;

class CModelFileWriter : public FileWriter
{
public:
	CModelFileWriter(CModelDocument* doc);

	bool Write(const char* szfile) override;

private:
	CModelDocument*	m_doc;
};
