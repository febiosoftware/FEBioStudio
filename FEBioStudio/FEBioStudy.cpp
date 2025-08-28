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
#include "stdafx.h"
#include "FEBioStudy.h"
#include "ModelDocument.h"
#include <QFileInfo>
#include <QDir>
#include <FEBioLink/FEBioClass.h>
#include <FEBio/FEBioExport4.h>

CFEBioStudy::CFEBioStudy(CModelDocument* doc) : m_doc(doc)
{

}

COptimizationStudy::COptimizationStudy(CModelDocument* doc)
	: CFEBioStudy(doc)
{
	SetTypeString("Parameter optimization");
}

bool COptimizationStudy::Run()
{
	CModelDocument* doc = GetDocument();
	std::string filepath = doc->GetDocFilePath();
	QFileInfo fi(QString::fromStdString(filepath));
	QString dir = fi.absolutePath();

	// set the working directory to this folder
	QDir::setCurrent(dir);

	QString name = QString::fromStdString(GetName());
	QString studyPath = name + ".opt";
	QString febPath = name + ".feb";
	string febfile = febPath.toStdString();
	string optfile = studyPath.toStdString();

	m_logFileName = dir + "/" + name + ".log";

	try {
		setCurrentTask("Saving FEBio input file ...");
		FEBioExport4* febExport = new FEBioExport4(doc->GetProject());
		febExport->SetMixedMeshFlag(false);
//		febExport->SetProgressTracker(prg);
		bool ret = febExport->Write(febfile.c_str());
		if (ret == false)
		{
			setErrorString(febExport->GetErrorMessage());
			return false;
		}

		setCurrentTask("Saving optimization input file ...");
		if (GenerateFEBioOptimizationFile(optfile, Options()) == false)
		{
			return errf("Failed creating the optimization input file.");
		}

		setCurrentTask("Running optimization ...");
		string cmd = "-i " + febfile + " -s " + optfile;
		int returnCode = FEBio::runModel(cmd, nullptr, nullptr, nullptr);
		if (returnCode != 0) return errf("Study failed! FEBio error terminated.");
	}
	catch (...)
	{
		return errf("Something went terribly wrong!");
	}
	return true;
}
