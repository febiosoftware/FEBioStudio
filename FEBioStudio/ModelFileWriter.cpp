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
//		m_doc->SetDocFilePath(szfile);
	}
	catch (...)
	{
		return false;
	}

	// TODO: moved this to the save functions in CDocument so that it does not clear the modified
	// flag when the document is autosaved. Does this interfere with CMainWindow::on_actionConvertFeb_triggered
	// or CMainWindow::on_actionConvertGeo_triggered since this is called there.

	// clear the modified flag
//	m_doc->SetModifiedFlag(false);

	return true;
}
