/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "LSDYNAimport.h"
#include "LSDynaFile.h"
#include "LSDynaFileParser.h"
#include "GeomLib/GMeshObject.h"
#include <GeomLib/GModel.h>
#include <FEBioLink/FEBioModule.h>
#include <vector>
#include <sstream>

LSDYNAimport::LSDYNAimport(FSProject& prj) : FSFileImport(prj) { m_pprj = nullptr; }

LSDYNAimport::~LSDYNAimport() {}

bool LSDYNAimport::Load(const char* szfile)
{
	// clear all data
	m_dyna.clear();

	// try to open the file
	if (Open(szfile, "rt") == false) return errf("Failed to open file or file is not valid .k file.");

	LSDynaFile lsfile;
	if (lsfile.Open(FilePtr()) == false) return errf("Failed to open file or file is not valid .k file.");
	lsfile.SetFileName(szfile);

	// read the file
	LSDynaFileParser lsparser(lsfile, m_dyna);
	if (lsparser.ParseFile() == false) {
		return errf(lsparser.GetErrorString());
	}
	else if (lsparser.GetErrorString()) errf(lsparser.GetErrorString());

	// Make sure the solid module is set
	int moduleId = FEBio::GetModuleId("solid"); assert(moduleId >= 0);
	if (m_prj.GetModule() != moduleId)
		m_prj.SetModule(moduleId, false);

	// build the model
	FSModel& fem = m_prj.GetFSModel();
	bool b = m_dyna.BuildModel(fem);
	if (b)
	{
		// if we get here we are good to go!
		GMeshObject* po = m_dyna.TakeObject();
		char szname[256];
		FileTitle(szname);
		po->SetName(szname);
		fem.GetModel().AddObject(po);

		// we need to convert to the new format
		std::ostringstream log;
		m_prj.ConvertToNewFormat(log);
		std::string s = log.str();
		if (s.empty() == false) errf(s.c_str());
	}

	// clean up
	m_dyna.clear();

	return (b ? true : errf("Failed building model"));
}
