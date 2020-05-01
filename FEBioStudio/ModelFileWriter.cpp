#include "stdafx.h"
#include "ModelFileWriter.h"
#include "ModelDocument.h"
#include <FSCore/Archive.h>

CModelFileWriter::CModelFileWriter(CModelDocument* doc) : m_doc(doc)
{

}

bool CModelFileWriter::Write(const char* szfile)
{
	OArchive ar;
	if (!ar.Create(szfile, 0x00505256))
	{
		return false;
	}

	try
	{
		m_doc->Save(ar);
		m_doc->SetDocFilePath(szfile);
	}
	catch (...)
	{
		return false;
	}

	// clear the modified flag
	m_doc->SetModifiedFlag(false);

	return true;
}
