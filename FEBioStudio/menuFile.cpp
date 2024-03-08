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
#include "MainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include "ui_mainwindow.h"
#include "ModelDocument.h"
#include "ModelFileWriter.h"
#include "Commands.h"
#include "DocManager.h"
#include "DlgConvertFEBio.h"
#include <FEBio/FEBioImport.h>
#include <FEBio/FEBioExport12.h>
#include <FEBio/FEBioExport2.h>
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include <FEBio/FEBioExport4.h>
#include <Nike3D/NIKE3DExport.h>
#include <MeshIO/BYUExport.h>
#include <MeshIO/FluentExport.h>
#include <MeshIO/HypersurfaceExport.h>
#include <LSDyna/LSDYNAexport.h>
#include <MeshIO/MeshExport.h>
#include <MeshIO/STLExport.h>
#include <MeshIO/ViewpointExport.h>
#include <MeshIO/TetGenExport.h>
#include <MeshIO/VTKExport.h>
#include <MeshIO/VTUImport.h>
#include <MeshIO/PLYExport.h>
#include <MeshIO/GMshExport.h>
#include <GeomLib/GPrimitive.h>
#include <Abaqus/AbaqusImport.h>
#include <Ansys/AnsysImport.h>
#include <MeshIO/BYUimport.h>
#include <Comsol/COMSOLImport.h>
#include <MeshIO/DXFimport.h>
#include <MeshIO/GMshImport.h>
#include <MeshIO/HMASCIIimport.h>
#include <MeshIO/HyperSurfaceImport.h>
#include <MeshIO/IDEASimport.h>
#include <MeshIO/IGESFileImport.h>
#include <LSDyna/LSDYNAimport.h>
#include <MeshIO/MeshImport.h>
#include <MeshIO/NASTRANimport.h>
#include <MeshIO/PLYImport.h>
#include <MeshIO/RAWToMeshImport.h>
#include <MeshIO/STLimport.h>
#include <MeshIO/TetGenImport.h>
#include <MeshIO/VTKImport.h>
#include <Nike3D/NIKE3DImport.h>
#include <MeshIO/PRVObjectImport.h>
#include <MeshIO/PRVObjectExport.h>
#include <MeshIO/BREPImport.h>
#include <MeshIO/STEPImport.h>
#include <Abaqus/AbaqusExport.h>
#include <FSCore/FSDir.h>
#include "DlgEditObject.h"
#include "DlgRAWImport.h"
#include "DlgExportFEBio.h"
#include "DlgNew.h"
#include "DlgNewProject.h"
#include "DlgModelInfo.h"
#include "DlgExportLSDYNA.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include "DlgPurge.h"
#include "DlgBatchConvert.h"
#include "version.h"
#include "PostDocument.h"
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include <QtCore/QTextStream>
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include <ImageLib/SITKImageSource.h>
#include <ImageLib/DICOMImageSource.h>
#include <PostLib/FELSDYNAExport.h>
#include <PostLib/PLYExport.h>
#include <PostLib/AbaqusExport.h>
#include <GeomLib/GModel.h>
#include "DlgExportXPLT.h"
#include <XPLTLib/xpltFileExport.h>
#include <iostream>
#include "XMLDocument.h"
#include "FileThread.h"
#include <PostLib/FEFEBioExport.h>
#include <PostLib/FEAsciiExport.h>
#include <PostLib/VRMLExporter.h>
#include <PostLib/FENikeExport.h>
#include <PostLib/FEVTKExport.h>
#include <PostLib/FELSDYNAPlot.h>
#include <PostLib/BYUExport.h>
#include <PostLib/VTKImport.h>
#include <PostLib/VolumeRenderer.h>
#include <ImageLib/TiffReader.h>
#include <sstream>
#include "PostObject.h"
#include "DlgScreenCapture.h"
#include "ModelFileReader.h"
#include "units.h"

using std::stringstream;

#ifdef HAS_LIBZIP
#include "ZipFiles.h"
#endif

void CMainWindow::on_actionOpenProject_triggered()
{
	QString projectFile = QFileDialog::getOpenFileName(this, "Open Project", CurrentWorkingDirectory(), QString("FEBioStudio Projects (*.fsp)"));
	if (projectFile.isEmpty() == false)
	{
		OpenProject(projectFile);
	}
}

void CMainWindow::on_actionNewModel_triggered()
{
	// create a unique name for this model
	int n = 1;
	string docTitle;
	CDocManager* dm = m_DocManager;
	bool bok = true;
	do
	{
		stringstream ss;
		ss << "Model" << n++;
		docTitle = ss.str();
		bok = true;
		for (int i = 0; i < dm->Documents(); ++i)
		{
			CDocument* doci = dm->GetDocument(i);
			if ((doci->GetDocTitle() == docTitle) || (doci->GetDocFileBase() == docTitle))
			{
				bok = false;
				break;
			}
		}
	} while (bok == false);

	// show the dialog box
	CDlgNew dlg(this);
	dlg.SetModelName(QString::fromStdString(docTitle));
	if (dlg.exec())
	{
		CModelDocument* doc = CreateNewDocument();
		if (dlg.CreateMode() == CDlgNew::CREATE_NEW_MODEL)
		{
			int nmodule = dlg.GetSelection();
			doc->GetProject().SetModule(nmodule);
		}
		else if (dlg.CreateMode() == CDlgNew::CREATE_FROM_TEMPLATE)
		{
			if (doc->LoadTemplate(dlg.GetSelection()) == false)
			{
				QMessageBox::critical(this, "New", "Failed to initialize template.");
				delete doc;
				doc = nullptr;
			}
		}
		assert(doc);
		if (doc)
		{
			int units = dlg.GetUnitSystem();
			Units::SetUnitSystem(units);
			doc->SetUnitSystem(units);

			docTitle = dlg.GetModelName().toStdString();
			doc->SetDocTitle(docTitle);
			AddDocument(doc);
		}
	}
	else return;
}

void CMainWindow::on_actionNewProject_triggered()
{
	CDlgNewProject dlg(this);
	dlg.SetProjectFolder(ui->m_defaultProjectParent);
	if (dlg.exec())
	{
		ui->fileViewer->Update();
		ui->m_defaultProjectParent = dlg.GetProjectFolder();
	}
}

void CMainWindow::on_actionOpen_triggered()
{
	QStringList filters;
	filters << "All supported files (*.fs2 *.fsm *.feb *.xplt *.n *.inp *.fsprj *.prv *.vtk *.fsps *.k *.dyn *.stl)";
	filters << "FEBioStudio Model (*.fs2 *.fsm *.fsprj)";
	filters << "FEBio input files (*.feb)";
	filters << "FEBio plot files (*.xplt)";
	filters << "FEBioStudio Post Session (*.fsps)";
	filters << "PreView files (*.prv)";
	filters << "Abaus files (*.inp)";
	filters << "Nike3D files (*.n)";
	filters << "VTK files (*.vtk)";
	filters << "LSDYNA keyword (*.k *.dyn)";
	filters << "STL file (*.stl)";
	filters << "LSDYNA database (*)";

	QFileDialog dlg(this, "Open");
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// store the current path
		QDir dir = dlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		OpenFile(fileName);
	}
}

void CMainWindow::on_actionSave_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	std::string fileName = doc->GetDocFilePath();
	if (fileName.empty()) on_actionSaveAs_triggered();
	else
	{
        auto modelDoc = dynamic_cast<CModelDocument*>(doc);

        if(modelDoc)
        {
            // if the extension is fsm or feb, we are going to change it to fs2
            size_t n = fileName.rfind('.');
            if (n != string::npos)
            {
                string ext = fileName.substr(n);
                if (ext == ".fsm" || ext == ".feb")
                {
                    on_actionSaveAs_triggered();
                    return;
                }
            }
        }
		
		SaveDocument(QString::fromStdString(fileName));
	}
}


#ifdef HAS_LIBZIP
void CMainWindow::on_actionImportProject_triggered()
{
	QStringList filters;
	filters << "FBS Project Archives (*.prj)";

	QFileDialog dlg(this, "Open");
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// store the current path
		QDir dir = dlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		ImportProjectArchive(fileName);
	}
}

void CMainWindow::on_actionExportProject_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	QString fileName = QFileDialog::getSaveFileName(this, "Export", CurrentWorkingDirectory(), "FEBio Studio Project (*.prj)");
	if (fileName.isEmpty() == false)
	{
		// make sure the file has an extension
		std::string sfile = fileName.toStdString();
		std::size_t found = sfile.rfind(".");
		if (found == std::string::npos) sfile.append(".prj");

		std::string pathName = doc->GetDocFolder();

		archive(QString::fromStdString(sfile), QDir(QString::fromStdString(pathName)));
	}
}
#else
void CMainWindow::on_actionImportProject_triggered() {}
void CMainWindow::on_actionExportProject_triggered() {}
#endif

bool CMainWindow::SaveDocument(const QString& fileName)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return false;

	CGLDocument* glDoc = dynamic_cast<CGLDocument*>(doc);
	if (glDoc && (glDoc->GetFileWriter() == nullptr))
	{
		on_actionSaveAs_triggered();
		return true;
	}

	// start log message
	ui->logPanel->AddText(QString("Saving file: %1 ...").arg(fileName));

	// save the document
	bool success = doc->SaveDocument(fileName.toStdString());

	// clear the command stack
	if (ui->m_settings.clearUndoOnSave)
	{
		CGLDocument* gldoc = dynamic_cast<CGLDocument*>(doc);
		if (gldoc) gldoc->ClearCommandStack();
	}

	ui->logPanel->AddText(success ? "SUCCESS\n" : "FAILED\n");

	if (success)
	{
		UpdateTab(doc);
		ui->addToRecentFiles(fileName);
		ui->m_project.AddFile(QDir::toNativeSeparators(fileName));
		ui->fileViewer->Update();
	}
	else
	{
		QString errStr = QString("Failed storing file to:\n%1").arg(fileName);
		QMessageBox::critical(this, "FEBio Studio", errStr);
	}

	return success;
}

void CMainWindow::SaveImage(QImage& image)
{
	const uchar* bits = image.bits();

	QStringList filters;
	filters << "Bitmap files (*.bmp)"
		<< "PNG files (*.png)"
		<< "JPEG files (*.jpg)";

	QFileDialog dlg(this, "Save Image");
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec())
	{
		QString fileName = dlg.selectedFiles().first();
		int nfilter = filters.indexOf(dlg.selectedNameFilter());
		bool bret = false;
		switch (nfilter)
		{
		case 0: bret = image.save(fileName, "BMP"); break;
		case 1: bret = image.save(fileName, "PNG"); break;
		case 2: bret = image.save(fileName, "JPG"); break;
		}
		if (bret == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving image file.");
		}
	}
}

FileReader* CMainWindow::CreateFileReader(const QString& fileName)
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return nullptr;

	FSProject& prj = doc->GetProject();

	// get the file extension
	QString ext = QFileInfo(fileName).suffix();
	if (ext.compare("pvo", Qt::CaseInsensitive) == 0) return new PRVObjectImport(prj);
	if (ext.compare("inp", Qt::CaseInsensitive) == 0)
	{
		AbaqusImport* reader = new AbaqusImport(prj);
		CDlgEditObject dlg(reader, "Import Abaqus", this);
		if (dlg.exec())
		{
			return reader;
		}
		else return nullptr;
	}
	if (ext.compare("cdb", Qt::CaseInsensitive) == 0) return new AnsysImport(prj);
	if (ext.compare("k", Qt::CaseInsensitive) == 0) return new LSDYNAimport(prj);
	if (ext.compare("dyn", Qt::CaseInsensitive) == 0) return new LSDYNAimport(prj);
	if (ext.compare("unv", Qt::CaseInsensitive) == 0) return new IDEASimport(prj);
	if (ext.compare("nas", Qt::CaseInsensitive) == 0) return new NASTRANimport(prj);
	if (ext.compare("dxf", Qt::CaseInsensitive) == 0) return new DXFimport(prj);
	if (ext.compare("stl", Qt::CaseInsensitive) == 0) return new STLimport(prj);
	if (ext.compare("hmascii", Qt::CaseInsensitive) == 0) return new HMASCIIimport(prj);
	if (ext.compare("surf", Qt::CaseInsensitive) == 0) return new HyperSurfaceImport(prj);
	if (ext.compare("msh", Qt::CaseInsensitive) == 0) return new GMshImport(prj);
	if (ext.compare("byu", Qt::CaseInsensitive) == 0) return new BYUimport(prj);
	if (ext.compare("mesh", Qt::CaseInsensitive) == 0) return new MeshImport(prj);
	if (ext.compare("ele", Qt::CaseInsensitive) == 0) return new TetGenImport(prj);
	//	if (ext.compare("iges"   , Qt::CaseInsensitive) == 0) return new IGESFileImport(prj);
	if (ext.compare("vtk", Qt::CaseInsensitive) == 0) return new VTKimport(prj);
	if (ext.compare("vtu", Qt::CaseInsensitive) == 0) return new VTUimport(prj);
	if (ext.compare("raw", Qt::CaseInsensitive) == 0)
	{
		RAWToMeshImport* reader = new RAWToMeshImport(prj);
		CDlgEditObject dlg(reader, "Import RAW", this);
		if (dlg.exec()) {
			return reader;
		}
		else {
			delete reader;
			return nullptr;
		}
	}
	if (ext.compare("mphtxt", Qt::CaseInsensitive) == 0)
	{
		COMSOLimport* reader = new COMSOLimport(prj);
		CDlgEditObject dlg(reader, "Import COMSOL", this);
		if (dlg.exec())
		{
			return reader;
		}
		else return 0;
	}
	if (ext.compare("ply", Qt::CaseInsensitive) == 0) return new PLYImport(prj);
	if ((ext.compare("brep", Qt::CaseInsensitive) == 0) ||
		(ext.compare("brp", Qt::CaseInsensitive) == 0)) return new BREPImport(prj);
	if ((ext.compare("step", Qt::CaseInsensitive) == 0) ||
		(ext.compare("stp", Qt::CaseInsensitive) == 0)) return new STEPImport(prj);
	if ((ext.compare("iges", Qt::CaseInsensitive) == 0) ||
		(ext.compare("igs", Qt::CaseInsensitive) == 0)) return new IGESImport(prj);
	if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
		FEBioFileImport* febio = new FEBioFileImport(prj);
		febio->SetGeometryOnlyFlag(true);
		return febio;
	}

	return 0;
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenFEModel(const QString& fileName)
{
	m_fileQueue.clear();

	// create a new document
	CModelDocument* doc = new CModelDocument(this);
	doc->SetFileWriter(nullptr);

	// we need to set this document as the active document
	// NOTE: This might cause problems if the user modifies the currently open document
	//       while the file is reading. 
	CDocument::SetActiveDocument(doc);

	FSProject& prj = doc->GetProject();

	// create a file reader
	FileReader* reader = 0;
	QString ext = QFileInfo(fileName).suffix();
	if (ext.compare("feb", Qt::CaseInsensitive) == 0) reader = new FEBioFileImport(prj);
	else if (ext.compare("n", Qt::CaseInsensitive) == 0) reader = new NIKE3DImport(prj);
	else if (ext.compare("dyn", Qt::CaseInsensitive) == 0) reader = new LSDYNAimport(prj);
	else if (ext.compare("inp", Qt::CaseInsensitive) == 0)
	{
		AbaqusImport* abaqusReader = new AbaqusImport(prj);

		CDlgEditObject dlg(abaqusReader, "Import Abaqus", this);
		if (dlg.exec() == 0)
		{
			return;
		}

		// when reading an Abaqus model, we process the physics
		abaqusReader->m_breadPhysics = true;
		reader = abaqusReader;
	}
	else
	{
		QMessageBox::critical(this, "Read File", QString("Cannot reade file\n%0").arg(fileName));
		return;
	}

	/*	// remove quotes from the filename
	char* ch = strchr((char*)szfile, '"');
	if (ch) szfile++;
	ch = strrchr((char*)szfile, '"');
	if (ch) *ch = 0;
	*/

	// start reading the file
	ReadFile(doc, fileName, reader, QueuedFile::NEW_DOCUMENT);
}

void CMainWindow::OpenFEBioFile(const QString& fileName)
{
	// CTextDocument* txt = new CTextDocument(this);
	// if (txt->ReadFromFile(fileName) == false)
	// {
	// 	QMessageBox::critical(this, "FEBio Studio", "Failed to open file:\n" + fileName);
	// 	return;
	// }

	// txt->SetDocFilePath(fileName.toStdString());

	// AddDocument(txt);

    CXMLDocument* xml = new CXMLDocument(this);
	if (xml->ReadFromFile(fileName) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to open file:\n" + fileName);
		return;
	}

	xml->SetDocFilePath(fileName.toStdString());

	AddDocument(xml);
}

void CMainWindow::ExportPostGeometry()
{
	CPostDocument* doc = GetPostDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	QStringList filters;
	filters << "FEBio xplt files (*.xplt)"
		//		<< "FEBio files (*.feb)"
		//		<< "ASCII files (*.*)"
		//		<< "VRML files (*.wrl)"
		<< "LSDYNA Keyword (*.k)"
		<< "STL file (*.stl)"
		<< "PLY file (*.ply)";
	//		<< "BYU files(*.byu)"
	//		<< "NIKE3D files (*.n)"
	//		<< "VTK files (*.vtk)"
	//		<< "LSDYNA database (*.d3plot)";

	QFileDialog dlg(this, "Save");
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec() == 0) return;

	QStringList files = dlg.selectedFiles();
	QString filter = dlg.selectedNameFilter();

	int nfilter = filters.indexOf(filter);

	QString fileName = files.first();

	if (fileName.isEmpty()) return;
	string sfilename = fileName.toStdString();
	const char* szfilename = sfilename.c_str();

	Post::FEPostModel& fem = *doc->GetFSModel();

	bool bret = false;
	QString error("(unknown)");
	switch (nfilter)
	{
	case 0:
	{
		CDlgExportXPLT dlg(this);
		if (dlg.exec() == QDialog::Accepted)
		{
			Post::xpltFileExport ex;
			ex.SetCompression(dlg.m_bcompress);
			bret = ex.Save(fem, szfilename);
			error = ex.GetErrorMessage();
		}
	}
	break;
	/*	case 1:
	{
	Post::FEFEBioExport fr;
	bret = fr.Save(fem, szfilename);
	}
	break;
	case 2:
	{
	CDlgExportAscii dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
	// decide which time steps to export
	int n0, n1;
	if (dlg.m_nstep == 0) n0 = n1 = doc->currentTime();
	else
	{
	n0 = 0;
	n1 = fem.GetStates() - 1;
	}

	// export the data
	Post::FEASCIIExport out;
	out.m_bcoords = dlg.m_bcoords;
	out.m_bedata = dlg.m_bedata;
	out.m_belem = dlg.m_belem;
	out.m_bface = dlg.m_bface;
	out.m_bfnormals = dlg.m_bfnormals;
	out.m_bndata = dlg.m_bndata;
	out.m_bselonly = dlg.m_bsel;

	bret = out.Save(&fem, n0, n1, szfilename);
	}
	}
	break;
	case 3:
	{
	Post::VRMLExporter exporter;
	bret = exporter.Save(&fem, szfilename);
	}
	break;
	*/	case 1:
	{
		Post::FELSDYNAExport w;
		CDlgEditObject dlg(&w, "Export LSDyna", this);
		if (dlg.exec())
		{
			bret = w.Save(fem, doc->GetActiveState(), szfilename);
		}
	}
	break;
	case 2:
	{
		// We need a dummy project
		FSProject prj;
		STLExport stl(prj);
		bret = stl.Write(szfilename, doc->GetPostObject());
	}
	break;
	case 3:
	{
		Post::PLYExport ply;
		// we need to get the current colormap
		Post::CGLModel* gm = doc->GetGLModel();
		Post::CGLColorMap* cmap = (gm ? gm->GetColorMap() : nullptr);
		if (cmap && cmap->IsActive()) ply.SetColorMap(cmap->GetColorMap()->ColorMap());
		bret = ply.Save(fem, szfilename);
	}
	break;
	/*	case 5:
	{
	bret = doc->ExportBYU(szfilename);
	}
	break;
	case 6:
	{
	Post::FENikeExport fr;
	bret = fr.Save(fem, szfilename);
	}
	break;
	case 7:
	{
	Post::FEVTKExport w;
	CDlgEditObject dlg(&w, "Export VTK", this);
	if (dlg.exec())
	{
	bret = w.Save(fem, szfilename);
	error = "Failed writing VTK file";
	}
	}
	break;
	case 8:
	{
	CDlgExportLSDYNAPlot dlg(&fem, this);
	if (dlg.exec())
	{
	Post::FELSDYNAPlotExport ls;
	bret = ls.Save(fem, szfilename, dlg.m_flag, dlg.m_code);
	error = "Failed writing LSDYNA database file";
	}
	}
	break;
	*/	default:
		assert(false);
		error = "Unknown file type";
		break;
	}

	if (bret == false)
	{
		QMessageBox b;
		b.setText(QString("Failed saving file.\nReason:%1").arg(error));
		b.setIcon(QMessageBox::Critical);
		b.exec();
	}
	else
	{
		QMessageBox::information(this, "FEBio Studio", "Success saving file!");
	}

	if (bret == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed exporting geometry.");
	}
}

void CMainWindow::ExportGeometry()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// supported file formats
	QStringList filters;
	filters << "PreView Object File (*.pvo)";
	filters << "LSDYNA keyword (*.k)";
	filters << "Abaqus files (*.inp)";
	filters << "HyperSurface files (*.surf)";
	filters << "BYU files (*.byu)";
	filters << "STL-ASCII files (*.stl)";
	filters << "ViewPoint files (*.*)";
	filters << "Mesh files (*.mesh)";
	filters << "TetGen files (*.ele)";
	filters << "VTK files (*.vtk)";
	filters << "GMesh files (*.msh)";
    filters << "Fluent files (*.msh)";

	// default extensions
	const char* szext[] = {
		".pvo",
		".k",
		".inp",
		".surf",
		".byu",
		".stl",
		".txt",
		".mesh",
		".ele",
		".vtk",
		".msh"
	};

	// file name
	std::string sfile = GetDocument()->GetDocFilePath();

	// get rid of the extension
	size_t ext = sfile.rfind(".");
	if (ext != std::string::npos) sfile.erase(ext);

	// present file save dialog
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	dlg.selectFile(QString(sfile.c_str()));
	if (dlg.exec())
	{
		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.at(0);
		sfile = fileName.toStdString();

		// get the filter
		QString flt = dlg.selectedNameFilter();
		int nflt = filters.indexOf(flt);

		// make sure the file has an extension
		size_t epos = sfile.rfind(".");
		if (epos == std::string::npos) sfile.append(szext[nflt]);

		// get the zero-terminated string
		const char* szfile = sfile.c_str();

		// get the project
		FSProject& fem = doc->GetProject();

		AddLogEntry(QString("Writing file %1 ... ").arg(fileName));

		// export file based on selected filter
		switch (nflt)
		{
		case 0: // PreView Object files
		{
			PRVObjectExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to PreView Object file."));
		}
		break;
		case 1: // LSDYNA keyword
		{
			LSDYNAexport writer(fem);
			CDlgEditObject dlg(&writer, "Export LSDYNA", this);
			if (dlg.exec())
			{
				if (!writer.Write(szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to LSDYNA keyword file."));
			}
		}
		break;
		case 2:
		{
			AbaqusExport writer(fem);
			stringstream ss;
			ss << "Written by FEBio Studio " << FBS_VERSION << "." << FBS_SUBVERSION << "." << FBS_SUBSUBVERSION;
			writer.SetHeading(ss.str());
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to Abaqus file."));
		}
		break;
		case 3:
		{
			HypersurfaceExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to surf file."));
		}
		break;
		case 4: // BYU files
		{
			BYUExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to byu file."));
		}
		break;
		case 5: // STL files
		{
			STLExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to STL file:\n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
		}
		break;
		case 6: // ViewPoint files
		{
			ViewpointExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to viewpoint file."));
		}
		break;
		case 7:
		{
			MeshExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to Mesh file."));
		}
		break;
		case 8:
		{
			TetGenExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to TetGen file."));
		}
		break;
		case 9: // VTK files
		{
			VTKExport writer(fem);
			CDlgEditObject dlg(&writer, "Export VTK", this);
			if (dlg.exec())
			{
				if (!writer.Write(szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to vtk file."));
			}
		}
		break;
		case 10:
		{
			GMeshExport writer(fem);
			if (!writer.Write(szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to GMesh file."));
		}
		break;
        case 11:
        {
            FluentExport writer(fem);
            if (!writer.Write(szfile))
                QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to Fluent file."));
            }
        break;
		default:
			QMessageBox::critical(this, "FEBio Studio", "Don't know how to save this file.");
		}
		AddLogEntry(QString("success!\n"));
	}
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	OpenFile(fileName);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentProjects_triggered(QAction* action)
{
	QString fileName = action->text();
	OpenFile(fileName);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentFEFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	OpenFEModel(fileName);
}

//-----------------------------------------------------------------------------
void CMainWindow::on_recentGeomFiles_triggered(QAction* action)
{
	QString fileName = action->text();
	ImportFiles(QStringList(fileName));
}

//-----------------------------------------------------------------------------
void CMainWindow::SavePostDoc()
{
	CPostDocument* doc = GetPostDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	string fileName = doc->GetDocTitle();
	size_t n = fileName.rfind('.');
	if (n != string::npos) fileName.erase(n);

	QStringList filters;
	filters << "FEBio Studio Post Session (*.fsps)"
		<< "FEBio xplt files (*.xplt)"
		<< "FEBio files (*.feb)"
		<< "ASCII files (*.*)"
		<< "VRML files (*.wrl)"
		<< "LSDYNA Keyword (*.k)"
		<< "BYU files(*.byu)"
		<< "NIKE3D files (*.n)"
		<< "VTK files (*.vtk)"
		<< "LSDYNA database (*.d3plot)"
		<< "Abaqus files (*.inp)";

	QFileDialog dlg(this, "Save");
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setNameFilters(filters);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.selectFile(QString::fromStdString(fileName));
	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString filter = dlg.selectedNameFilter();

		int nfilter = filters.indexOf(filter);

		QString fileName = files.first();
		if (fileName.isEmpty()) return;
		string sfilename = fileName.toStdString();
		const char* szfilename = sfilename.c_str();

		Post::FEPostModel& fem = *doc->GetFSModel();

		bool bret = false;
		QString error("(unknown)");
		switch (nfilter)
		{
		case 0:
		{
			bret = doc->SavePostSession(fileName.toStdString());

			if (bret)
			{
				ui->addToRecentFiles(fileName);
				ui->m_project.AddFile(QDir::toNativeSeparators(fileName));
				ui->fileViewer->Update();
			}
		}
		break;
		case 1:
		{
			CDlgExportXPLT dlg(this);
			if (dlg.exec() == QDialog::Accepted)
			{
				Post::xpltFileExport ex;
				ex.SetCompression(dlg.m_bcompress);
				bret = ex.Save(fem, szfilename);
				error = ex.GetErrorMessage();
			}
		}
		break;
		case 2:
		{
			Post::FEFEBioExport4 fr;
			bret = fr.Save(fem, szfilename);
		}
		break;
		case 3:
		{
			Post::FEASCIIExport out;
			CDlgEditObject dlg(&out, "Export ASCII", this);
			if (dlg.exec() == QDialog::Accepted)
			{
				// decide which time steps to export
				int n0, n1;
				if (out.m_alltimes == 0) n0 = n1 = doc->GetActiveState();
				else
				{
					n0 = 0;
					n1 = fem.GetStates() - 1;
				}

				bret = out.Save(&fem, n0, n1, szfilename);
			}
		}
		break;
		case 4:
		{
			Post::VRMLExporter exporter;
			bret = exporter.Save(&fem, szfilename);
		}
		break;
		case 5:
		{
			Post::FELSDYNAExport w;
			CDlgEditObject dlg(&w, "Export LSDyna", this);
			if (dlg.exec())
			{
				bret = w.Save(fem, doc->GetActiveState(), szfilename);
			}
		}
		break;
		case 6:
		{
			Post::BYUExport exporter;
			bret = exporter.Save(fem, szfilename);
		}
		break;
		case 7:
		{
			Post::FENikeExport fr;
			bret = fr.Save(fem, szfilename);
		}
		break;
		case 8:
		{
			Post::FEVTKExport w;
			CDlgEditObject dlg(&w, "Export VTK", this);
			if (dlg.exec())
			{
				bret = w.Save(fem, szfilename);
				error = "Failed writing VTK file";
			}
		}
		break;
		case 9:
		{
			CDlgExportLSDYNAPlot dlg(&fem, this);
			if (dlg.exec())
			{
				Post::FELSDYNAPlotExport ls;
				bret = ls.Save(fem, szfilename, dlg.m_flag, dlg.m_code);
				error = "Failed writing LSDYNA database file";
			}
		}
		break;
		case 10:
		{
			Post::AbaqusExport w;
			stringstream ss;
			ss << "Written by FEBio Studio " << FBS_VERSION << "." << FBS_SUBVERSION << "." << FBS_SUBSUBVERSION;
			w.SetHeading(ss.str());
			bret = w.Save(fem, doc->GetActiveState(), szfilename);
			error = "Failed writing Abaqus file.";
		}
		break;
		default:
			assert(false);
			error = "Unknown file type";
			break;
		}

		if (bret == false)
		{
			QMessageBox b;
			b.setText(QString("Failed saving file.\nReason:%1").arg(error));
			b.setIcon(QMessageBox::Critical);
			b.exec();
		}
		else
		{
			QMessageBox::information(this, "Save", "Success saving file!");
		}
	}
}

//-----------------------------------------------------------------------------
QString CMainWindow::CurrentWorkingDirectory()
{
	QString path = ui->m_currentPath;
	if (ui->m_project.GetProjectFileName().isEmpty() == false)
	{
		QFileInfo fi(ui->m_project.GetProjectFileName());
		path = fi.absolutePath();
	}
	else
	{
		CDocument* doc = GetDocument();
		if (doc)
		{
			string docfile = doc->GetDocFilePath();
			if (docfile.empty() == false)
			{
				QFileInfo fi(QString::fromStdString(docfile));
				path = fi.absolutePath();
			}
		}
	}

	return path;
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionSaveAs_triggered()
{
	CModelDocument* doc = GetModelDocument();
	if (doc == nullptr)
	{
		CPostDocument* postDoc = GetPostDocument();
		if (postDoc)
		{
			SavePostDoc();
		}

        CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(GetDocument());
        if(xmlDoc)
        {
            QFileDialog dlg;
            dlg.setDirectory(CurrentWorkingDirectory());
            dlg.setFileMode(QFileDialog::AnyFile);
            dlg.setNameFilter("FEBio Input files (*.feb)");
            dlg.setDefaultSuffix("feb");
            dlg.selectFile(QString::fromStdString(xmlDoc->GetDocTitle()));
            dlg.setAcceptMode(QFileDialog::AcceptSave);
            if (dlg.exec())
            {
                QStringList fileNames = dlg.selectedFiles();
                SaveDocument(QDir::toNativeSeparators(fileNames[0]));
            }
        }
		return;
	}

	string fileName = doc->GetDocTitle();
	QString currentPath = CurrentWorkingDirectory();
	if (ui->m_project.GetProjectFileName().isEmpty() == false)
	{
		QFileInfo fi(ui->m_project.GetProjectFileName());
		currentPath = fi.absolutePath();
	}
	else
	{
		string docfile = doc->GetDocFilePath();
		if (docfile.empty() == false)
		{
			QFileInfo fi(QString::fromStdString(docfile));
			currentPath = fi.absolutePath();

			size_t n = fileName.rfind('.');
			if (n != string::npos)
			{
				string ext = fileName.substr(n);
				if ((ext == ".fsm") || (ext == ".feb"))
				{
					fileName.replace(n, 4, ".fs2");
				}
			}
		}
	}

	QFileDialog dlg;
	dlg.setDirectory(currentPath);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setNameFilter("FEBio Studio Model (*.fs2)");
    dlg.setDefaultSuffix("fs2");
	dlg.selectFile(QString::fromStdString(fileName));
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec())
	{
		// clear the jobs
		doc->DeleteAllJobs();

		QStringList fileNames = dlg.selectedFiles();
		doc->SetFileWriter(new CModelFileWriter(doc));
		SaveDocument(QDir::toNativeSeparators(fileNames[0]));

		UpdateModel();
	}
}

void CMainWindow::on_actionSaveAll_triggered()
{
	int docs = m_DocManager->Documents();
	int fails = 0;
	for (int i = 0; i < docs; ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		if (doc->IsModified())
		{
			if (doc->SaveDocument() == false) fails++;
			else UpdateTab(doc);
		}
	}

	if (fails > 0)
	{
		QMessageBox::critical(this, "ERROR", "Not all files could be saved.");
	}
}

void CMainWindow::on_actionCloseAll_triggered()
{
	// see if any files are not save
	int unsaved = 0;
	int docs = m_DocManager->Documents();
	for (int i = 0; i < docs; ++i)
	{
		CDocument* doc = m_DocManager->GetDocument(i);
		if (doc->IsModified())
		{
			unsaved++;
		}
	}

	if (unsaved > 0)
	{
		if (QMessageBox::question(this, "Close All", "There are modified files that have not been saved.\nAre you sure you want to close all files?") == QMessageBox::No)
		{
			return;
		}
	}

	while (m_DocManager->Documents()) CloseView(0, true);
}

void CMainWindow::on_actionSnapShot_triggered()
{
	QImage img = GetGLView()->CaptureScreen();

    CDlgScreenCapture dlg(&img, this);

    dlg.exec();
}

void CMainWindow::on_actionSaveProject_triggered()
{
	QString prjPath = ui->m_project.GetProjectPath();
	QString fileName = QFileDialog::getSaveFileName(this, "Save Project As", prjPath, QString("FEBioStudio Projects (*.fsp)"));
	if (fileName.isEmpty() == false)
	{
		fileName = QDir::toNativeSeparators(fileName);
		bool b = ui->m_project.Save(fileName);
		if (b == false)
		{
			QMessageBox::critical(this, "ERROR", "Failed saving the project file.");
		}
		else ui->addToRecentProjects(fileName);
		ui->fileViewer->Update();
		UpdateTitle();
	}
}

void CMainWindow::on_actionExportFEModel_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// supported file formats
	QStringList filters;
	filters << "FEBio files (*.feb)";
	filters << "NIKE3D files (*.n)";

	// default extensions
	const char* szext[] = {
		".feb",
		".n",
	};

	QString path = CurrentWorkingDirectory();
	QString fileName = QString::fromStdString(GetDocument()->GetDocFilePath());

	if (fileName.isEmpty() == false)
	{
		QFileInfo fi(fileName);
		path = fi.absolutePath();
		fileName = fi.baseName() + QString(".feb");
	}

	// present file save dialog
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setDirectory(path);
	dlg.setNameFilters(filters);
	dlg.selectFile(fileName);
	if (dlg.exec())
	{
		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.at(0);
		std::string sfile = QDir::toNativeSeparators(fileName).toStdString();

		// get the filter
		QString flt = dlg.selectedNameFilter();
		int nflt = filters.indexOf(flt);

		// make sure the file has an extension
		size_t epos = sfile.rfind(".");
		if (epos == std::string::npos) sfile.append(szext[nflt]);

		// get the zero-terminated string
		const char* szfile = sfile.c_str();

		// get the project
		FSProject& fem = doc->GetProject();

		// pass the units to the model project
		fem.SetUnits(doc->GetUnitSystem());

		AddLogEntry(QString("Writing file %1 ... ").arg(fileName));

		// export file based on selected filter
		bool bsuccess = true;
		QString errMsg = "(unknown)";
		switch (nflt)
		{
		case 0: // FEBio files
		{
			CDlgExportFEBio dlg(this);
			if (dlg.exec())
			{
				// Do a model check
				if (DoModelCheck(doc) == false)
				{
					AddLogEntry(QString("cancelled\n"));
					return;
				}

				int nformat = dlg.FEBioFormat();
				try {
					if (nformat == 0x0400)
					{
						// write version 4.0
						FEBioExport4 writer(fem);
						writer.SetPlotfileCompressionFlag(dlg.m_compress);
						writer.SetExportSelectionsFlag(dlg.m_bexportSelections);
						writer.SetWriteNotesFlag(dlg.m_writeNotes);
						for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0300)
					{
						// write version 3.0
						FEBioExport3 writer(fem);
						writer.SetPlotfileCompressionFlag(dlg.m_compress);
						writer.SetExportSelectionsFlag(dlg.m_bexportSelections);
						writer.SetWriteNotesFlag(dlg.m_writeNotes);
						for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0205)
					{
						// write version 2.5
						FEBioExport25 writer(fem);
						writer.SetPlotfileCompressionFlag(dlg.m_compress);
						writer.SetExportSelectionsFlag(dlg.m_bexportSelections);
						writer.SetWriteNotesFlag(dlg.m_writeNotes);
						for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0200)
					{
						// Write version 2.0
						FEBioExport2 writer(fem);
						writer.SetPlotfileCompressionFlag(dlg.m_compress);
						for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0102)
					{
						// Write version 1.x
						FEBioExport12 writer(fem);
						for (int i = 0; i < FEBioExport12::MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else
					{
						assert(false);
					}
				}
				catch (...)
				{
					bsuccess = false;
					errMsg = QString("An exception was deteced.\nFailed writing FEBio file.");
				}
			}
		}
		break;
		case 1: // NIKE3D files
		{
			NIKE3DExport writer(fem);
			bsuccess = writer.Write(szfile);
			if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
		}
		break;
		default:
			QMessageBox::critical(this, "FEBio Studio", "Don't know how to save this file.");
			AddLogEntry(QString("failed!\n"));
			return;
		}
		if (bsuccess)
			AddLogEntry(QString("success!\n"));
		else
		{
			AddLogEntry(QString("failed!\n"));
			QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to FEBio file: \n%1").arg(errMsg));
		}
	}
}

void CMainWindow::on_actionImportGeometry_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	CPostDocument* postDoc = GetPostDocument();
	if (doc)
	{
		// file filters
		QStringList filters;
		filters << "All files (*)";
		filters << "PreView Object File (*.pvo)";
		filters << "FEBio (*.feb)";
		filters << "ABAQUS (*.inp)";
		filters << "ANSYS (*.cdb)";
		filters << "LSDYNA Keyword (*.k *.dyn)";
		filters << "IDEAS Universal (*.unv)";
		filters << "NASTRAN (*.nas)";
		filters << "DXF (*.dxf)";
		filters << "STL (*.stl)";
		filters << "HyperMesh ASCII (*.hmascii)";
		filters << "HyperSurface ASCII (*.surf)";
		filters << "GMsh (*.msh)";
		filters << "BYU (*.byu)";
		filters << "Mesh (*.mesh)";
		filters << "TetGen (*.ele)";
		filters << "IGES (*.iges, *.igs)";
		filters << "VTK (*.vtk)";
		filters << "RAW Image (*.raw)";
		filters << "COMSOL Mesh (*.mphtxt)";
		filters << "PLY (*.ply)";
		filters << "BREP files (*.brep *.brp)";
		filters << "STEP files (*.step *.stp)";

		// present the file selection dialog box
		QFileDialog dlg(this);
		dlg.setDirectory(CurrentWorkingDirectory());
		dlg.setFileMode(QFileDialog::ExistingFiles);
		dlg.setAcceptMode(QFileDialog::AcceptOpen);
		dlg.setNameFilters(filters);
		if (dlg.exec())
		{
			// store the current path
			QDir dir = dlg.directory();
//			SetCurrentFolder(dir.absolutePath());

			// get the file names
			QStringList files = dlg.selectedFiles();

			// import the file
			ImportFiles(files);
		}
	}
	else if (postDoc)
	{
		// file filters
		QStringList filters;
		filters << "VTK (*.vtk)";

		QFileDialog dlg(this);
		dlg.setDirectory(CurrentWorkingDirectory());
		dlg.setFileMode(QFileDialog::ExistingFile);
		dlg.setAcceptMode(QFileDialog::AcceptOpen);
		dlg.setNameFilters(filters);
		if (dlg.exec())
		{
			// store the current path
			QDir dir = dlg.directory();
//			SetCurrentFolder(dir.absolutePath());

			// get the file names
			QStringList files = dlg.selectedFiles();
			if (files.size() > 0)
			{
				QString fileName = files.at(0);

				std::string sfile = fileName.toStdString();

				// create a dummy post model
				Post::FEPostModel* dummyFem = new Post::FEPostModel;
				Post::VTKimport vtk(dummyFem);
				if (vtk.Load(sfile.c_str()))
				{
					if (postDoc->MergeFEModel(dummyFem) == false)
					{
						QMessageBox::critical(this, "FEBio Studio", QString("Failed merging model in file:\n%1").arg(fileName));
					}
				}
				else
				{
					QMessageBox::critical(this, "FEBio Studio", QString("Failed importing file:\n%1").arg(fileName));
				}
				delete dummyFem;
				Post::FEPostModel::SetInstance(postDoc->GetFSModel());
			}

			ui->postPanel->Update(true);
			RedrawGL();
		}
	}
	else
	{
		QMessageBox::critical(this, "Import Geometry", "Importing geometry requires an active model.\nPlease create a new model or open an existing model first.");
	}
}

void CMainWindow::on_actionImportRawImage_triggered()
{
	CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	// present the file selection dialog box
	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "RAW Files (*.raw)" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
		CImageModel* imageModel = nullptr;
		
		CDlgRAWImport dlg(this);
		if (dlg.exec())
		{
			BOX box(dlg.m_x0, dlg.m_y0, dlg.m_z0, dlg.m_x0 + dlg.m_w, dlg.m_y0 + dlg.m_h, dlg.m_z0 + dlg.m_d);

            // we pass the relative path to the image model
	        string relFile = FSDir::makeRelative(filedlg.selectedFiles()[0].toStdString(), "$(ProjectDir)");

            imageModel = new CImageModel(nullptr);
            imageModel->SetImageSource(new CRawImageSource(imageModel, relFile, dlg.m_type, dlg.m_nx, dlg.m_ny, dlg.m_nz, box, dlg.m_swapEndianness));

            if(!ImportImage(imageModel))
            {
                delete imageModel;
                imageModel = nullptr;
            }

		}

		if(imageModel)
		{
			Update(0, true);
			ZoomTo(imageModel->GetBoundingBox());

			// only for model docs
			if (dynamic_cast<CModelDocument*>(doc))
			{
				Post::CVolumeRenderer* vr = new Post::CVolumeRenderer(imageModel);
				vr->Create();
				imageModel->AddImageRenderer(vr);

				Update(0, true);
				ShowInModelViewer(imageModel);
			}
			else
			{
				Update(0, true);
			}
			ZoomTo(imageModel->GetBoundingBox());
		}
	}
}

void CMainWindow::on_actionImportDICOMImage_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "DICOM Files (*.dcm *.dicom)" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
		ProcessITKImage(filedlg.selectedFiles()[0], ImageFileType::DICOM);
	}
}

// void CMainWindow::on_actionImportTiffImage_triggered()
// {
//     CGLDocument* doc = GetGLDocument();
//     if(!doc)
//     {
//         QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
//         return;
//     }

// 	QFileDialog filedlg(this);
// 	filedlg.setFileMode(QFileDialog::ExistingFile);
// 	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

// 	QStringList filters;
// 	filters << "Tiff Files (*.tif *.tiff)" << "All Files (*)";
// 	filedlg.setNameFilters(filters);

// 	if (filedlg.exec())
// 	{
// 		ProcessITKImage(filedlg.selectedFiles()[0], ImageFileType::TIFF);
// 	}
// }

void CMainWindow::on_actionImportTiffImage_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "Tiff Files (*.tif *.tiff)" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
		std::string fileName = filedlg.selectedFiles()[0].toStdString();

		// we pass the relative path to the image model
		string relFile = FSDir::makeRelative(fileName, "$(ProjectDir)");

		CImageModel* imageModel = new CImageModel(nullptr);
		imageModel->SetImageSource(new CTiffImageSource(imageModel, relFile));

		if (!ImportImage(imageModel))
		{
			delete imageModel;
			imageModel = nullptr;
			return;
		}

		// take the name from the source
		imageModel->SetName(FSDir::fileName(fileName));

		Update(0, true);
		ZoomTo(imageModel->GetBoundingBox());

		// only for model docs
		if (dynamic_cast<CModelDocument*>(doc))
		{
			Post::CVolumeRenderer* vr = new Post::CVolumeRenderer(imageModel);
			vr->Create();
			imageModel->AddImageRenderer(vr);

			Update(0, true);
			ShowInModelViewer(imageModel);
		}
		else
		{
			Update(0, true);
		}
		ZoomTo(imageModel->GetBoundingBox());
	}
}

void CMainWindow::on_actionImportOMETiffImage_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "OME Tiff XML Files (*.xml)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
		ProcessITKImage(filedlg.selectedFiles()[0], ImageFileType::OMETIFF);
	}
}

void CMainWindow::on_actionImportNrrdImage_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "NRRD Files (*.nrrd *.nhdr )" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
		ProcessITKImage(filedlg.selectedFiles()[0], ImageFileType::OTHER);
	}
}

void CMainWindow::on_actionImportImageOther_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	if (filedlg.exec())
	{
		ProcessITKImage(filedlg.selectedFiles()[0], ImageFileType::OTHER);
	}
}

void CMainWindow::on_actionImportImageSequence_triggered()
{
    CGLDocument* doc = GetGLDocument();
    if(!doc)
    {
        QMessageBox::critical(this, "FEBio Studio", "You must have a model open in order to import an image.");
        return;
    }

	QFileDialog filedlg(this);
	filedlg.setDirectory(CurrentWorkingDirectory());
	filedlg.setFileMode(QFileDialog::ExistingFiles);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	if (filedlg.exec())
	{
        QStringList files = filedlg.selectedFiles();

        if(files.length() == 0) return;

        std::vector<std::string> stdFiles;
        for(auto filename : files)
        {
            // we pass the relative path to the image model
            stdFiles.push_back(FSDir::makeRelative(filename.toStdString(), "$(ProjectDir)"));
        }

        CGLDocument* doc = GetGLDocument();

        CImageModel* imageModel = new CImageModel(nullptr);
        imageModel->SetImageSource(new CITKSeriesImageSource(imageModel, stdFiles));

        if(!ImportImage(imageModel))
        {
            delete imageModel;
            imageModel = nullptr;
        }

        if(imageModel)
        {
            Update(0, true);
            ZoomTo(imageModel->GetBoundingBox());

            // only for model docs
            if (dynamic_cast<CModelDocument*>(doc))
            {
                Post::CVolumeRenderer* vr = new Post::CVolumeRenderer(imageModel);
                vr->Create();
                imageModel->AddImageRenderer(vr);

                Update(0, true);
                ShowInModelViewer(imageModel);
            }
            else
            {
                Update(0, true);
            }
            ZoomTo(imageModel->GetBoundingBox());
        }
    }

}

void CMainWindow::on_actionExportGeometry_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc) ExportPostGeometry();
	else ExportGeometry();
}

QString createFileName(const QString& fileName, const QString& dirName, const QString& ext)
{
	// extract the file title
	QString baseName = QFileInfo(fileName).baseName();

	// create the new name
	QString outName = dirName + "/" + baseName + "." + ext;

	return outName;
}

void CMainWindow::on_actionConvertFeb_triggered()
{
	CDlgConvertFEBio dlg(this);
	if (dlg.exec())
	{
		QStringList fileNames = dlg.getFileNames();
		QString dir = dlg.getOutPath();

		int nformat = dlg.getOutputFormat();

		QStringList::iterator it;

		ShowLogPanel();

		AddLogEntry("Starting batch conversion ...\n");
		int nsuccess = 0, nfails = 0, nwarnings = 0;
		for (it = fileNames.begin(); it != fileNames.end(); ++it)
		{
			FSProject prj;
			FEBioFileImport reader(prj);

			FSFileExport* exporter = 0;
			if (nformat == 0x0400)
			{
				// write version 4
				FEBioExport4* writer = new FEBioExport4(prj); exporter = writer;
				for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
			}
			else if (nformat == 0x0300)
			{
				// write version 3
				FEBioExport3* writer = new FEBioExport3(prj); exporter = writer;
				for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
			}
			else if (nformat == 0x0205)
			{
				// write version 2.5
				FEBioExport25* writer = new FEBioExport25(prj); exporter = writer;
				for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
			}
			else if (nformat == 0x0200)
			{
				// Write version 2.0
				FEBioExport2* writer = new FEBioExport2(prj); exporter = writer;
				for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
			}
			else if (nformat == 0x0102)
			{
				// Write version 1.x
				FEBioExport12* writer = new FEBioExport12(prj); exporter = writer;
				for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
			}
			else
			{
				QMessageBox::critical(this, "Convert", "Cannot convert to this file format.");
				return;
			}

			std::string inFile = it->toStdString();
			AddLogEntry(QString("Converting %1 ... ").arg(*it));
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

			// create an output file name
			QString outName = createFileName(*it, dir, "feb");
			string outFile = outName.toStdString();

			// try to read the project
			bool bret = reader.Load(inFile.c_str());

			// try to save the project
			exporter->ClearLog();
			bret = (bret ? exporter->Write(outFile.c_str()) : false);

			AddLogEntry(bret ? "success\n" : "FAILED\n");
			string err = reader.GetErrorString();
			if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
			err = exporter->GetErrorMessage();
			if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

			if (bret) nsuccess++; else nfails++;
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

			delete exporter;
		}

		AddLogEntry("Batch conversion completed:\n");
		if (nwarnings == 0)
			AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
		else
			AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
	}
}

void CMainWindow::on_actionConvertFeb2Fsm_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Files", CurrentWorkingDirectory(), "FEBio files (*.feb)");
	if (fileNames.isEmpty() == false)
	{
		QString dir = QFileDialog::getExistingDirectory();
		if (dir.isEmpty() == false)
		{
            QStringList::iterator it;

            ShowLogPanel();

            AddLogEntry("Starting batch conversion ...\n");
            int nsuccess = 0, nfails = 0, nwarnings = 0;
            for (it = fileNames.begin(); it != fileNames.end(); ++it)
            {
                CModelDocument doc(this);

                // we need to set this document as the active document
                // NOTE: This might cause problems if the user modifies the currently open document
                //       while the file is reading. 
                // CDocument::SetActiveDocument(doc);

                FSProject& prj = doc.GetProject();
				FEBioFileImport reader(prj);

                std::string inFile = it->toStdString();
                AddLogEntry(QString("Converting %1 ... ").arg(*it));
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

                // create an output file name
                QString outName = createFileName(*it, dir, "fsm");
                string outFile = outName.toStdString();
                doc.SetDocFilePath(outFile);

                // try to read the project
                bool bret = reader.Load(inFile.c_str());

                // try to save the project
                bret = (bret ? doc.SaveDocument() : false);

                AddLogEntry(bret ? "success\n" : "FAILED\n");
                string err = reader.GetErrorString();
                if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
                if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

                if (bret) nsuccess++; else nfails++;
                QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

            }

            AddLogEntry("Batch conversion completed:\n");
            if (nwarnings == 0)
                AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
            else
                AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
        }
	}
}

void CMainWindow::on_actionConvertFsm2Feb_triggered()
{
	CDlgConvertFEBio dlg(this);
	dlg.SetFileFilter(CDlgConvertFEBio::FSM_FILES);
	if (dlg.exec())
	{
		QStringList fileNames = dlg.getFileNames();
		QString dir = dlg.getOutPath();

		int nformat = dlg.getOutputFormat();

		QStringList::iterator it;

		ShowLogPanel();

		AddLogEntry("Starting batch conversion ...\n");
		int nsuccess = 0, nfails = 0, nwarnings = 0;
		for (it = fileNames.begin(); it != fileNames.end(); ++it)
		{
			CModelDocument doc(this);

			std::string inFile = it->toStdString();
			AddLogEntry(QString("Converting %1 ... ").arg(*it));
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

			// read the model
			ModelFileReader reader(&doc);
			bool bret = reader.Load(inFile.c_str());

			if (bret)
			{
				FSProject& prj = doc.GetProject();

				FSFileExport* exporter = 0;
				if (nformat == 0x0400)
				{
					// write version 4
					FEBioExport4* writer = new FEBioExport4(prj); exporter = writer;
					for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (nformat == 0x0300)
				{
					// write version 3
					FEBioExport3* writer = new FEBioExport3(prj); exporter = writer;
					for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (nformat == 0x0205)
				{
					// write version 2.5
					FEBioExport25* writer = new FEBioExport25(prj); exporter = writer;
					for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (nformat == 0x0200)
				{
					// Write version 2.0
					FEBioExport2* writer = new FEBioExport2(prj); exporter = writer;
					for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (nformat == 0x0102)
				{
					// Write version 1.x
					FEBioExport12* writer = new FEBioExport12(prj); exporter = writer;
					for (int i = 0; i < FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else
				{
					QMessageBox::critical(this, "Convert", "Cannot convert to this file format.");
					return;
				}

				// create an output file name
				QString outName = createFileName(*it, dir, "feb");
				string outFile = outName.toStdString();

				// try to save the project
				exporter->ClearLog();
				bret = (bret ? exporter->Write(outFile.c_str()) : false);

				AddLogEntry(bret ? "success\n" : "FAILED\n");
				string err = reader.GetErrorString();
				if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
				err = exporter->GetErrorMessage();
				if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

				if (bret) nsuccess++; else nfails++;

				delete exporter;
			}
			else
			{
				AddLogEntry("FAILED\n");
				string err = reader.GetErrorString();
				if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
				nfails++;
			}
			QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
		}

		AddLogEntry("Batch conversion completed:\n");
		if (nwarnings == 0)
			AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
		else
			AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
	}
}

void CMainWindow::on_actionConvertGeo_triggered()
{
	// file filters
	QStringList filters;
	filters << "All files (*)";
	filters << "PreView Object File (*.pvo)";
	filters << "ABAQUS (*.inp)";
	filters << "ANSYS (*.cdb)";
	filters << "LSDYNA Keyword (*.k)";
	filters << "IDEAS Universal (*.unv)";
	filters << "NASTRAN (*.nas)";
	filters << "DXF (*.dxf)";
	filters << "STL (*.stl)";
	filters << "HyperMesh ASCII (*.hmascii)";
	filters << "HyperSurface ASCII (*.surf)";
	filters << "GMsh (*.msh)";
	filters << "BYU (*.byu)";
	filters << "Mesh (*.mesh)";
	filters << "TetGen (*.ele)";
	filters << "IGES (*.iges)";
	filters << "VTK (*.vtk)";
	filters << "RAW Image (*.raw)";
	filters << "COMSOL Mesh (*.mphtxt)";
	filters << "PLY (*.ply)";

	// present the file selection dialog box
	QFileDialog dlg(this);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setFileMode(QFileDialog::ExistingFiles);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// get the file names
		QStringList fileNames = dlg.selectedFiles();

		// ask for an directory to output the converted files
		QString dir = QFileDialog::getExistingDirectory();
		if (dir.isEmpty() == false)
		{
			// select the output format
			CDlgBatchConvert dlg(this);
			if (dlg.exec())
			{
				// note that the format is an index into the options presented in
				// the dialog box.
				int format = dlg.GetFileFormat();

				// create a file writer
				QString ext;

				// start convert process
				AddLogEntry("Starting batch conversion ...\n");
				int nsuccess = 0, nfails = 0, nwarnings = 0;
				QStringList::iterator it;
				for (it = fileNames.begin(); it != fileNames.end(); ++it)
				{
					std::string inFile = it->toStdString();
					AddLogEntry(QString("Converting %1 ... ").arg(*it));
					QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

					// create a file reader based on the file extension
					FileReader* reader = CreateFileReader(*it);
					FSFileImport* importer = dynamic_cast<FSFileImport*>(reader);
					if (importer)
					{
						FSProject& prj = importer->GetProject();

						FSFileExport* exporter = nullptr;
						switch (format)
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
							QMessageBox::critical(this, "FEBio Studio", "Cannot create file exporter.");
							return;
						}

						// create an output file name
						QString outName = createFileName(*it, dir, ext);
						string outFile = outName.toStdString();

						// try to read the project
						bool bret = importer->Load(inFile.c_str());

						// try to save the project
						exporter->ClearLog();
						bret = (bret ? exporter->Write(outFile.c_str()) : false);

						AddLogEntry(bret ? "success\n" : "FAILED\n");
						string err = reader->GetErrorString();
						if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
						err = exporter->GetErrorMessage();
						if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

						if (bret) nsuccess++; else nfails++;
						QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

						delete reader;
						delete exporter;
					}
					else
					{
						AddLogEntry("FAILED (can't create file reader!)\n");
					}
				}

				AddLogEntry("Batch conversion completed:\n");
				if (nwarnings == 0)
					AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
				else
					AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));
			}
		}
	}
}

void CMainWindow::on_actionExit_triggered()
{
	QApplication::closeAllWindows();
}
