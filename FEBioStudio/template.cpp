#include "stdafx.h"
#include "Document.h"
#include <GeomLib/GPrimitive.h>
#include "PreViewLib/FEBox.h"
#include "DocTemplate.h"
#include <PreViewLib/PRVArchive.h>

//-----------------------------------------------------------------------------
bool CDocument::LoadTemplate(int n)
{
	int N = TemplateManager::Templates();
	if ((n<0) || (n>=N)) return false;

	NewDocument();

	const DocTemplate& doc = TemplateManager::GetTemplate(n);

/*	string fileName = TemplateManager::TemplatePath() + doc.fileName;
	const char* szfile = fileName.c_str();

	PRVArchive ar;
	if (ar.Load(szfile) == false) return false;
*/
	m_Project.SetModule(doc.module);

	return true;
}
