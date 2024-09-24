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
#include "BatchConverter.h"
#include "MainWindow.h"
#include <FEMLib/FSProject.h>
#include <FEBio/FEBioImport.h>
#include <FEBio/FEBioExport12.h>
#include <FEBio/FEBioExport2.h>
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include <FEBio/FEBioExport4.h>
#include <MeshIO/VTKExport.h>
#include <MeshIO/PLYExport.h>
#include <QMessageBox>
#include <QCoreApplication>
#include "ModelDocument.h"
#include "ModelFileReader.h"
#include <LSDyna/LSDYNAexport.h>
#include <MeshIO/HypersurfaceExport.h>
#include <MeshIO/BYUExport.h>
#include <MeshIO/STLExport.h>
#include <MeshIO/ViewpointExport.h>
#include <MeshIO/MeshExport.h>
#include <MeshIO/TetGenExport.h>


CConvertFEBtoFEB::CConvertFEBtoFEB(CMainWindow* wnd) : m_wnd(wnd)
{
	m_nformat = 0;
	m_section = 0xFFFF;
}

void CConvertFEBtoFEB::SetOutputFormat(int format)
{
	m_nformat = format;
}

void CConvertFEBtoFEB::SetFiles(const QStringList& fileNames)
{
	m_files = fileNames;
}

void CConvertFEBtoFEB::SetSectionFlags(unsigned int flags)
{
	m_section = flags;
}

void CConvertFEBtoFEB::SetOutputFolder(const QString& dir)
{
	m_dir = dir;
}

void CConvertFEBtoFEB::Start()
{
	m_wnd->AddLogEntry("Starting batch conversion ...\n");
	int nsuccess = 0, nfails = 0, nwarnings = 0;
	QStringList::iterator it;
	for (it = m_files.begin(); it != m_files.end(); ++it)
	{
		FSProject prj;
		FEBioFileImport reader(prj);

		FSFileExport* exporter = 0;
		if (m_nformat == 0x0400)
		{
			// write version 4
			FEBioExport4* writer = new FEBioExport4(prj); exporter = writer;
			writer->SetSectionFlags(m_section);
		}
		else if (m_nformat == 0x0300)
		{
			// write version 3
			FEBioExport3* writer = new FEBioExport3(prj); exporter = writer;
			writer->SetSectionFlags(m_section);
		}
		else if (m_nformat == 0x0205)
		{
			// write version 2.5
			FEBioExport25* writer = new FEBioExport25(prj); exporter = writer;
			writer->SetSectionFlags(m_section);
		}
		else if (m_nformat == 0x0200)
		{
			// Write version 2.0
			FEBioExport2* writer = new FEBioExport2(prj); exporter = writer;
			writer->SetSectionFlags(m_section);
		}
		else if (m_nformat == 0x0102)
		{
			// Write version 1.x
			FEBioExport12* writer = new FEBioExport12(prj); exporter = writer;
			writer->SetSectionFlags(m_section);
		}
		else
		{
			QMessageBox::critical(m_wnd, "Convert", "Cannot convert to this file format.");
			return;
		}

		std::string inFile = it->toStdString();
		m_wnd->AddLogEntry(QString("Converting %1 ... ").arg(*it));
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

		// create an output file name
		QString outName = createFileName(*it, m_dir, "feb");
		string outFile = outName.toStdString();

		// try to read the project
		bool bret = reader.Load(inFile.c_str());

		// try to save the project
		exporter->ClearLog();
		bret = (bret ? exporter->Write(outFile.c_str()) : false);

		m_wnd->AddLogEntry(bret ? "success\n" : "FAILED\n");
		string err = reader.GetErrorString();
		if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
		err = exporter->GetErrorMessage();
		if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

		if (bret) nsuccess++; else nfails++;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

		delete exporter;
	}

	m_wnd->AddLogEntry("Batch conversion completed:\n");
	if (nwarnings == 0)
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
	else
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
}


CConvertFEBtoFSM::CConvertFEBtoFSM(CMainWindow* wnd) : m_wnd(wnd)
{

}

void CConvertFEBtoFSM::SetFiles(const QStringList& fileNames)
{
	m_files = fileNames;
}

void CConvertFEBtoFSM::SetOutputFolder(const QString& dir)
{
	m_dir = dir;
}

void CConvertFEBtoFSM::Start()
{
	m_wnd->AddLogEntry("Starting batch conversion ...\n");
	int nsuccess = 0, nfails = 0, nwarnings = 0;
	QStringList::iterator it;
	for (it = m_files.begin(); it != m_files.end(); ++it)
	{
		CModelDocument doc(m_wnd);

		// we need to set this document as the active document
		// NOTE: This might cause problems if the user modifies the currently open document
		//       while the file is reading. 
		// CDocument::SetActiveDocument(doc);

		FSProject& prj = doc.GetProject();
		FEBioFileImport reader(prj);

		std::string inFile = it->toStdString();
		m_wnd->AddLogEntry(QString("Converting %1 ... ").arg(*it));
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

		// create an output file name
		QString outName = createFileName(*it, m_dir, "fsm");
		string outFile = outName.toStdString();
		doc.SetDocFilePath(outFile);

		// try to read the project
		bool bret = reader.Load(inFile.c_str());

		// try to save the project
		bret = (bret ? doc.SaveDocument() : false);

		m_wnd->AddLogEntry(bret ? "success\n" : "FAILED\n");
		string err = reader.GetErrorString();
		if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
		if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

		if (bret) nsuccess++; else nfails++;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
	}

	m_wnd->AddLogEntry("Batch conversion completed:\n");
	if (nwarnings == 0)
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
	else
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
}

CConvertFSMtoFEB::CConvertFSMtoFEB(CMainWindow* wnd) : m_wnd(wnd)
{
	m_format = 0;
	m_section = 0xFFFF;
}

void CConvertFSMtoFEB::SetFiles(const QStringList& fileNames)
{
	m_files = fileNames;
}

void CConvertFSMtoFEB::SetOutputFolder(const QString& dir)
{
	m_dir = dir;
}

void CConvertFSMtoFEB::SetOutputFormat(int format)
{
	m_format = format;
}

void CConvertFSMtoFEB::SetSectionFlags(unsigned int flags)
{
	m_section = flags;
}

void CConvertFSMtoFEB::Start()
{
	m_wnd->AddLogEntry("Starting batch conversion ...\n");

	int nsuccess = 0, nfails = 0, nwarnings = 0;
	QStringList::iterator it;
	for (it = m_files.begin(); it != m_files.end(); ++it)
	{
		CModelDocument doc(m_wnd);

		std::string inFile = it->toStdString();
		m_wnd->AddLogEntry(QString("Converting %1 ... ").arg(*it));
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

		// read the model
		ModelFileReader reader(&doc);
		bool bret = reader.Load(inFile.c_str());

		if (bret)
		{
			FSProject& prj = doc.GetProject();

			FSFileExport* exporter = 0;
			if (m_format == 0x0400)
			{
				// write version 4
				FEBioExport4* writer = new FEBioExport4(prj); exporter = writer;
				writer->SetSectionFlags(m_section);
			}
			else if (m_format == 0x0300)
			{
				// write version 3
				FEBioExport3* writer = new FEBioExport3(prj); exporter = writer;
				writer->SetSectionFlags(m_section);
			}
			else if (m_format == 0x0205)
			{
				// write version 2.5
				FEBioExport25* writer = new FEBioExport25(prj); exporter = writer;
				writer->SetSectionFlags(m_section);
			}
			else if (m_format == 0x0200)
			{
				// Write version 2.0
				FEBioExport2* writer = new FEBioExport2(prj); exporter = writer;
				writer->SetSectionFlags(m_section);
			}
			else if (m_format == 0x0102)
			{
				// Write version 1.x
				FEBioExport12* writer = new FEBioExport12(prj); exporter = writer;
				writer->SetSectionFlags(m_section);
			}
			else
			{
				QMessageBox::critical(m_wnd, "Convert", "Cannot convert to this file format.");
				return;
			}

			// create an output file name
			QString outName = createFileName(*it, m_dir, "feb");
			string outFile = outName.toStdString();

			// try to save the project
			exporter->ClearLog();
			bret = (bret ? exporter->Write(outFile.c_str()) : false);

			m_wnd->AddLogEntry(bret ? "success\n" : "FAILED\n");
			string err = reader.GetErrorString();
			if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
			err = exporter->GetErrorMessage();
			if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

			if (bret) nsuccess++; else nfails++;

			delete exporter;
		}
		else
		{
			m_wnd->AddLogEntry("FAILED\n");
			string err = reader.GetErrorString();
			if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
			nfails++;
		}
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
	}

	m_wnd->AddLogEntry("Batch conversion completed:\n");
	if (nwarnings == 0)
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
	else
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));

}

CConvertGeoFiles::CConvertGeoFiles(CMainWindow* wnd) : m_wnd(wnd)
{
	m_format = -1;
}

void CConvertGeoFiles::SetFiles(const QStringList& fileNames)
{
	m_files = fileNames;
}

void CConvertGeoFiles::SetOutputFormat(int format)
{
	m_format = format;
}

void CConvertGeoFiles::SetOutputFolder(const QString& dir)
{
	m_dir = dir;
}

void CConvertGeoFiles::Start()
{
	// create a file writer
	QString ext;

	// start convert process
	m_wnd->AddLogEntry("Starting batch conversion ...\n");
	int nsuccess = 0, nfails = 0, nwarnings = 0;
	QStringList::iterator it;
	for (it = m_files.begin(); it != m_files.end(); ++it)
	{
		std::string inFile = it->toStdString();
		m_wnd->AddLogEntry(QString("Converting %1 ... ").arg(*it));
		QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

		// create a file reader based on the file extension
		FileReader* reader = m_wnd->CreateFileReader(*it);
		FSFileImport* importer = dynamic_cast<FSFileImport*>(reader);
		if (importer)
		{
			FSProject& prj = importer->GetProject();

			FSFileExport* exporter = nullptr;
			switch (m_format)
			{
			case 0: exporter = new VTKExport(prj); ext = "vtk"; break;
			case 1: exporter = new PLYExport(prj); ext = "ply"; break;
			case 2: exporter = new LSDYNAexport(prj); ext = "k"; break;
			case 3: exporter = new HypersurfaceExport(prj); ext = "surf"; break;
			case 4: exporter = new BYUExport(prj); ext = "byu"; break;
			case 5: exporter = new STLExport(prj); ext = "stl"; break;
			case 6: exporter = new ViewpointExport(prj); ext = "vp"; break;
			case 7: exporter = new MeshExport(prj); ext = "mesh"; break;
			case 8: exporter = new TetGenExport(prj); ext = "ele"; break;
			}

			if (exporter == nullptr)
			{
				QMessageBox::critical(m_wnd, "FEBio Studio", "Cannot create file exporter.");
				return;
			}

			// create an output file name
			QString outName = createFileName(*it, m_dir, ext);
			string outFile = outName.toStdString();

			// try to read the project
			bool bret = importer->Load(inFile.c_str());

			// try to save the project
			exporter->ClearLog();
			bret = (bret ? exporter->Write(outFile.c_str()) : false);

			m_wnd->AddLogEntry(bret ? "success\n" : "FAILED\n");
			string err = reader->GetErrorString();
			if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
			err = exporter->GetErrorMessage();
			if (err.empty() == false) { m_wnd->AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

			if (bret) nsuccess++; else nfails++;
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

			delete reader;
			delete exporter;
		}
		else
		{
			m_wnd->AddLogEntry("FAILED (can't create file reader!)\n");
		}
	}

	m_wnd->AddLogEntry("Batch conversion completed:\n");
	if (nwarnings == 0)
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
	else
		m_wnd->AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
}
