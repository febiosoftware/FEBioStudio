#pragma once
#include "FileReader.h"
#include <FSCore/Archive.h>

//-------------------------------------------------------------------
class GObject;
class GDiscreteObject;

//-------------------------------------------------------------------
// Class for reading the Preview Object File format.
class PRVObjectImport : public FEFileImport
{
public:
	PRVObjectImport();

	// read the file
	bool Load(FEProject& prj, const char* szfile);

	// close the file
	void Close();

protected:
	bool LoadObjects(IArchive& ar, FEProject& prj);
	GObject* LoadObject(IArchive& ar, FEProject& prj);
	GDiscreteObject* LoadDiscreteObject(IArchive& ar, FEProject& prj);
	void ReindexObject(GObject* po);
	void ReindexDiscreteObject(GDiscreteObject* po);

private:
	IArchive			m_ar;
	vector<GObject*>	m_objList;
};
