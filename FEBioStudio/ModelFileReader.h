#pragma once
#include <MeshTools/PRVArchive.h>

class CModelDocument;

class ModelFileReader : public PRVArchive
{
public:
	ModelFileReader(CModelDocument* doc);

	bool Load(const char* szfile) override;

private:
	CModelDocument*	m_doc;
};
