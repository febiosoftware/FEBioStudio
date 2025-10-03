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
#include "TextDocument.h"
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
#include <MeshIO/GMshExport.h>
#include <Abaqus/AbaqusImport.h>
#include <Ansys/AnsysImport.h>
#include <MeshIO/BYUimport.h>
#include <MeshIO/ObjImport.h>
#include <Comsol/COMSOLImport.h>
#include <MeshIO/DXFimport.h>
#include <MeshIO/GMshImport.h>
#include <MeshIO/HMASCIIimport.h>
#include <MeshIO/HyperSurfaceImport.h>
#include <MeshIO/IDEASimport.h>
#include <LSDyna/LSDYNAimport.h>
#include <MeshIO/MeshImport.h>
#include <MeshIO/NASTRANimport.h>
#include <MeshIO/PLYImport.h>
#include <MeshIO/RAWToMeshImport.h>
#include <MeshIO/STLimport.h>
#include <MeshIO/TetGenImport.h>
#include <MeshIO/ArtisynthImport.h>
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
#include "DlgExportLSDYNA.h"
#include "DlgBatchConvert.h"
#include "version.h"
#include "PostDocument.h"
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include <ImageLib/ImageModel.h>
#include <ImageLib/ImageSource.h>
#include <ImageLib/SITKImageSource.h>
#include <PostLib/FELSDYNAExport.h>
#include <PostLib/PLYExport.h>
#include <PostLib/AbaqusExport.h>
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
#include <ImageLib/TiffReader.h>
#include <FEBioMonitor/FEBioReportDoc.h>
#include <sstream>
#include <PostGL/PostObject.h>
#include "DlgScreenCapture.h"
#include "DlgStartThread.h"
#include "units.h"
#include <FSCore/ClassDescriptor.h>
#include <FEBioLink/FEBioModule.h>
#include "BatchConverter.h"
#include <GLLib/GLScene.h>
#include <RTLib/RayTracer.h>
#include "FEBioBatchDoc.h"
#include "DlgBatchRun.h"

// register file reader classes
REGISTER_CLASS4(PRVObjectImport    , CLASS_FILE_READER, "pvo"    , FSProject);
REGISTER_CLASS4(PLYImport          , CLASS_FILE_READER, "ply"    , FSProject);
REGISTER_CLASS4(BREPImport         , CLASS_FILE_READER, "brep"   , FSProject);
REGISTER_CLASS4(BRPImport          , CLASS_FILE_READER, "brp"    , FSProject);
REGISTER_CLASS4(STEPImport         , CLASS_FILE_READER, "step"   , FSProject);
REGISTER_CLASS4(STPImport          , CLASS_FILE_READER, "stp"    , FSProject);
REGISTER_CLASS4(IGESImport         , CLASS_FILE_READER, "iges"   , FSProject);
REGISTER_CLASS4(IGSImport          , CLASS_FILE_READER, "igs"    , FSProject);
REGISTER_CLASS4(AnsysImport        , CLASS_FILE_READER, "cdb"    , FSProject);
REGISTER_CLASS4(LSDYNAimport       , CLASS_FILE_READER, "k"      , FSProject);
REGISTER_CLASS4(LSDYNAimport_dyn   , CLASS_FILE_READER, "dyn"    , FSProject);
REGISTER_CLASS4(IDEASimport        , CLASS_FILE_READER, "unv"    , FSProject);
REGISTER_CLASS4(NASTRANimport      , CLASS_FILE_READER, "nas"    , FSProject);
REGISTER_CLASS4(DXFimport          , CLASS_FILE_READER, "dxf"    , FSProject);
REGISTER_CLASS4(STLimport          , CLASS_FILE_READER, "stl"    , FSProject);
REGISTER_CLASS4(HMASCIIimport      , CLASS_FILE_READER, "hmascii", FSProject);
REGISTER_CLASS4(HyperSurfaceImport , CLASS_FILE_READER, "surf"   , FSProject);
REGISTER_CLASS4(GMshImport         , CLASS_FILE_READER, "msh"    , FSProject);
REGISTER_CLASS4(BYUimport          , CLASS_FILE_READER, "byu"    , FSProject);
REGISTER_CLASS4(ObjImport          , CLASS_FILE_READER, "obj"    , FSProject);
REGISTER_CLASS4(MeshImport         , CLASS_FILE_READER, "mesh"   , FSProject);
REGISTER_CLASS4(TetGenImport       , CLASS_FILE_READER, "ele"    , FSProject);
REGISTER_CLASS4(ArtiSynthImport    , CLASS_FILE_READER, "elem"   , FSProject);
REGISTER_CLASS4(VTKimport          , CLASS_FILE_READER, "vtk"    , FSProject);
REGISTER_CLASS4(VTUimport          , CLASS_FILE_READER, "vtu"    , FSProject);
REGISTER_CLASS4(VTPimport          , CLASS_FILE_READER, "vtp"    , FSProject);
REGISTER_CLASS4(FEBioGeometryImport, CLASS_FILE_READER, "feb"    , FSProject);
REGISTER_CLASS4(AbaqusImport       , CLASS_FILE_READER, "inp"    , FSProject);
REGISTER_CLASS4(RAWToMeshImport    , CLASS_FILE_READER, "raw"    , FSProject);
REGISTER_CLASS4(COMSOLimport       , CLASS_FILE_READER, "mphtxt" , FSProject);

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
	CDocManager* dm = m_DocManager;
	string docName = dm->GenerateNewDocName();

	// show the dialog box
	CDlgNew dlg(this);
	dlg.SetModelName(QString::fromStdString(docName));
	if (dlg.exec())
	{
		int units = dlg.GetUnitSystem();
		docName = dlg.GetModelName().toStdString();
		CModelDocument* doc = nullptr;
		if (dlg.CreateMode() == CDlgNew::CREATE_NEW_MODEL)
		{
			doc = CreateNewModelDocument(this, dlg.GetSelection(), docName, units);
		}
		else if (dlg.CreateMode() == CDlgNew::CREATE_FROM_TEMPLATE)
		{
			doc = CreateDocumentFromTemplate(this, dlg.GetSelection(), docName, units);
			if (doc == nullptr)
			{
				QMessageBox::critical(this, "New", "Failed to initialize template.");
				return;
			}
		}
		assert(doc);
		if (doc)
		{
			int units = doc->GetUnitSystem();
			Units::SetUnitSystem(units);
			AddDocument(doc);
		}
	}
}

void CMainWindow::on_actionNewProject_triggered()
{
	CDlgNewProject dlg(this);
	dlg.SetProjectFolder(ui->m_settings.m_defaultProjectParent);
	if (dlg.exec())
	{
		ui->projectViewer->Update();
		ui->m_settings.m_defaultProjectParent = dlg.GetProjectFolder();
	}
}

QString CMainWindow::GetOpenModelFilename()
{
	QStringList filters;
	filters << "All supported files (*.fsm *.fs2 *.feb *.xplt *.n *.inp *.fsprj *.prv *.vtk *.vtu *.vtp *.vtm *.fsps *.k *.dyn *.stl)";
	filters << "FEBioStudio Model (*.fsm *.fs2 *.fsprj)";
	filters << "FEBio input files (*.feb)";
	filters << "FEBio plot files (*.xplt)";
	filters << "FEBioStudio Post Session (*.fsps)";
	filters << "PreView files (*.prv)";
	filters << "Abaus files (*.inp)";
	filters << "Nike3D files (*.n)";
	filters << "VTK files (*.vtk *.vtp *.vtu *.vtm)";
	filters << "LSDYNA keyword (*.k *.dyn)";
	filters << "STL file (*.stl)";
	filters << "LSDYNA database (*)";

	QString fileName;

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
		fileName = QDir::toNativeSeparators(files.first());
	}

	return fileName;
}

void CMainWindow::on_actionOpen_triggered()
{
	QString fileName = GetOpenModelFilename();
	if (!fileName.isEmpty())
	{
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
            // if the extension is fs2 or feb, we are going to change it to fsm
            size_t n = fileName.rfind('.');
            if (n != string::npos)
            {
                string ext = fileName.substr(n);
                if (ext == ".fs2" || ext == ".feb")
                {
                    on_actionSaveAs_triggered();
                    return;
                }
            }
        }
		
		SaveDocument(doc, QString::fromStdString(fileName));
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

bool CMainWindow::SaveDocument(CDocument* doc, const QString& fileName)
{
	if (doc == nullptr) return false;

	if (fileName.isEmpty()) return false;

	// start log message
	ui->logPanel->AddText(QString("Saving file: %1 ...").arg(fileName));

	// save the document
	bool success = doc->SaveDocument(fileName.toStdString());

	// clear the command stack
	if (ui->m_settings.clearUndoOnSave)
	{
		CUndoDocument* undoDoc = dynamic_cast<CUndoDocument*>(doc);
		if (undoDoc) undoDoc->ClearCommandStack();
	}

	ui->logPanel->AddText(success ? "SUCCESS\n" : "FAILED\n");

	if (success)
	{
		UpdateTab(doc);

		QString ext;
		int n = fileName.lastIndexOf('.');
		if (n >= 0) ext = fileName.right(fileName.length() - n - 1);
		if (ext == "fsm")
		{
			ui->addToRecentFiles(fileName);
			ui->m_project.AddFile(QDir::toNativeSeparators(fileName));
		}
		ui->projectViewer->Update();
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

	// get the file extension
	QString ext = QFileInfo(fileName).suffix().toLower();
	std::string szext = ext.toStdString();

	// this is needed as the constructor argument for file reader classes. 
	FSProject& prj = doc->GetProject();

	FSFileImport* fileReader = FSCore::CreateClass<FSFileImport, FSProject>(CLASS_FILE_READER, szext.c_str(), &prj);
	if (fileReader && fileReader->Parameters())
	{
		CDlgEditObject dlg(fileReader, "Import " + ext, this);
		if (dlg.exec() == QDialog::Rejected)
		{
			delete fileReader;
			fileReader = nullptr;
		}
	}
	return fileReader;
}

//-----------------------------------------------------------------------------
void CMainWindow::OpenFEModel(const QString& fileName)
{
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
	if      (ext.compare("feb", Qt::CaseInsensitive) == 0) reader = new FEBioFileImport(prj);
	else if (ext.compare("n"  , Qt::CaseInsensitive) == 0) reader = new NIKE3DImport(prj);
	else if (ext.compare("dyn", Qt::CaseInsensitive) == 0) reader = new LSDYNAimport(prj);
	else if (ext.compare("key", Qt::CaseInsensitive) == 0) reader = new LSDYNAimport(prj);
	else if (ext.compare("inp", Qt::CaseInsensitive) == 0)
	{
		AbaqusImport* abaqusReader = new AbaqusImport(prj);

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
	CXMLDocument* xml = new CXMLDocument(this);
	if (xml->ReadFromFile(fileName) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to open file:\n" + fileName);
		return;
	}

	xml->SetDocFilePath(fileName.toStdString());

	AddDocument(xml);
}

void CMainWindow::OpenTextFile(const QString& fileName)
{
	CTextDocument* txt = new CTextDocument(this);
	if (txt->ReadFromFile(fileName) == false)
	{
		QMessageBox::critical(this, "FEBio Studio", "Failed to open file:\n" + fileName);
		return;
	}
	txt->SetDocFilePath(fileName.toStdString());
	AddDocument(txt);
}

bool CMainWindow::OpenFEBioLogFile(const QString& fileName)
{
	CFEBioReportDoc* doc = new CFEBioReportDoc(this);
	if (doc->LoadFromLogFile(fileName) == false)
	{
		delete doc;
		return false;
	}
	AddDocument(doc);
	return true;
}

QString CMainWindow::GetExportGeometryFilename(QString& formatOption)
{
	QStringList filters;
	filters 
		<< "FEBio input files (*.feb)"
		<< "LSDYNA Keyword (*.k)"
		<< "STL file (*.stl)"
		<< "PLY file (*.ply)";

	QFileDialog dlg(this, "Save");
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setDirectory(CurrentWorkingDirectory());
	dlg.setNameFilters(filters);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec() == 0) return QString();

	QStringList files = dlg.selectedFiles();
	QString fileName = files.first();
	if (fileName.isEmpty()) return QString();

	QString filter = dlg.selectedNameFilter();
	int nfilter = filters.indexOf(filter);
	switch (nfilter)
	{
	case 0: formatOption = "feb"; break;
	case 1: formatOption = "k"; break;
	case 2: formatOption = "stl"; break;
	case 3: formatOption = "ply"; break;
	default:
		assert(false);
		formatOption.clear();
	}
	return fileName;
}

void CMainWindow::ExportPostGeometry()
{
	CPostDocument* doc = GetPostDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	QStringList filters;
	filters << "FEBio files (*.feb)"
			<< "LSDYNA Keyword (*.k)"
			<< "STL file (*.stl)"
			<< "PLY file (*.ply)";
	//		<< "ASCII files (*.*)"
	//		<< "VRML files (*.wrl)"
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
		Post::FEFEBioExport4 fr;
		bret = fr.Save(fem, szfilename);
	}
	break;
	case 1:
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
		ply.SetColorMap(GetGLView()->GetColorMap());
		bret = ply.Save(fem, szfilename);
	}
	break;
/*	case 4:
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
	case 5:
	{
	Post::VRMLExporter exporter;
	bret = exporter.Save(&fem, szfilename);
	}
	break;
	*/	
	/*	case 6:
	{
	bret = doc->ExportBYU(szfilename);
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
	*/	
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
	filters << "GLMesh files (*.msh)";
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
				{
					QString errMessage = QString::fromStdString(writer.GetErrorMessage());
					QString msg = QString("Couldn't save model to LSDYNA keyword file:\n%1").arg(errMessage);
					QMessageBox::critical(this, "FEBio Studio", msg);
				}
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
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to GLMesh file."));
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

void CMainWindow::on_recentImages_triggered(QAction* action)
{
	QString fileName = action->text();
	if (!ImportImage(fileName))
	{
		QMessageBox::critical(this, "Import image", QString("Failed to import image:\n%1").arg(fileName));
	}
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
				ui->projectViewer->Update();
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
		case 3:
		{
			Post::VRMLExporter exporter;
			bret = exporter.Save(&fem, szfilename);
		}
		break;
		case 4:
		{
			Post::FELSDYNAExport w;
			CDlgEditObject dlg(&w, "Export LSDyna", this);
			if (dlg.exec())
			{
				bret = w.Save(fem, doc->GetActiveState(), szfilename);
			}
		}
		break;
		case 5:
		{
			Post::BYUExport exporter;
			bret = exporter.Save(fem, szfilename);
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
		case 9:
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
	QString path = ui->m_settings.m_currentPath;
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

QString CMainWindow::GetSaveModelFilename(QString currentPath, QString fileName)
{
	QFileDialog dlg;
	dlg.setDirectory(currentPath);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setNameFilter("FEBio Studio Model (*.fsm)");
	dlg.setDefaultSuffix("fsm");
	dlg.selectFile(fileName);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	fileName.clear();
	if (dlg.exec())
	{
		QStringList fileNames = dlg.selectedFiles();
		if (fileNames.empty() == false)
		{
			fileName = QDir::toNativeSeparators(fileNames[0]);
		}
	}
	return fileName;
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
		else
		{
			CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(GetDocument());
			if (xmlDoc)
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
					QString fileName = QDir::toNativeSeparators(fileNames[0]);
					if (!fileName.isEmpty())
						SaveDocument(xmlDoc, fileName);
				}
			}

			FEBioBatchDoc* batchDoc = dynamic_cast<FEBioBatchDoc*>(GetDocument());
			if (batchDoc)
			{
				QFileDialog dlg;
				dlg.setDirectory(CurrentWorkingDirectory());
				dlg.setFileMode(QFileDialog::AnyFile);
				dlg.setNameFilter("FBS Batch files (*.fsbatch)");
				dlg.setDefaultSuffix("fsbatch");
				dlg.selectFile(QString::fromStdString(batchDoc->GetDocTitle()));
				dlg.setAcceptMode(QFileDialog::AcceptSave);
				if (dlg.exec())
				{
					QStringList fileNames = dlg.selectedFiles();
					QString fileName = QDir::toNativeSeparators(fileNames[0]);
					if (!fileName.isEmpty())
						SaveDocument(batchDoc, fileName);
				}
			}
			return;
		}
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
				if ((ext == ".fs2") || (ext == ".feb"))
				{
					fileName.replace(n, 4, ".fsm");
				}
			}
		}
	}

	QString filename = GetSaveModelFilename(currentPath, QString::fromStdString(fileName));
	if (!filename.isEmpty())
	{
		// clear the jobs
		doc->DeleteAllJobs();
		doc->SetFileWriter(new CModelFileWriter(doc));
		SaveDocument(doc, filename);
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
	ShowImageViewer(img);
}

class CRayTracerThread : public CustomThread
{
public:
	CRayTracerThread(GLScene* scene, GLContext rc, QImage* img, RayTracer* rayTracer) : m_img(img), m_scene(scene), m_rc(rc) 
	{
		m_rayTracer = rayTracer;
	}

	void run() Q_DECL_OVERRIDE
	{
	
		vec3f lp = m_rc.m_settings.m_light; lp.Normalize();

		m_rayTracer->start();
		m_rayTracer->setLightPosition(0, lp);
		m_scene->Render(*m_rayTracer, m_rc);
		m_rayTracer->finish();

		RayTraceSurface& trg = m_rayTracer->surface();
		int W = m_img->width();
		int H = m_img->height();

		for (size_t j=0; j<H; ++j)
			for (size_t i = 0; i < W; ++i)
			{
				GLColor c = trg.colorValue(i, j);
				QRgb rgb = qRgba(c.r, c.g, c.b, c.a);
				m_img->setPixel((int)i, (int)j, rgb);
			}
		emit resultReady(true);
	}

public:
	bool hasProgress() override { return (m_rayTracer ? m_rayTracer->hasProgress() : 0.); }

	double progress() override { 
		return (m_rayTracer ? m_rayTracer->progress() : 0.);
	}

	const char* currentTask() override 
	{ 
		if (hasProgress()) return "Rendering ...";
		else return "Processing scene ..."; 
	}

	void stop() override { if (m_rayTracer) m_rayTracer->cancel(); }

private:
	QImage* m_img = nullptr;
	GLScene* m_scene = nullptr;
	GLContext m_rc;
	RayTracer* m_rayTracer = nullptr;
};

void CMainWindow::on_actionRayTrace_triggered()
{
	CGLDocument* doc = GetGLDocument();
	if (doc == nullptr) return;

	GLScene* scene = doc->GetScene();
	if (scene == nullptr) return;

	int W = GetGLView()->width();
	int H = GetGLView()->height();

	RayTracer* rayTracer = new RayTracer;
	rayTracer->setWidth(W);
	rayTracer->setHeight(H);

	CDlgEditObject dlg(rayTracer, "RayTracer Settings", this);
	if (dlg.exec())
	{
		W = rayTracer->width();
		H = rayTracer->height();

		GLContext rc;
		rc.m_x = 0;
		rc.m_y = 0;
		rc.m_w = W;
		rc.m_h = H;
		rc.m_settings = GetGLView()->GetViewSettings();
		rc.m_cam = &scene->GetCamera();
		QImage img(W, H, QImage::Format_ARGB32);

		CGView& view = scene->GetView();
		rayTracer->setupProjection(view.m_fov, view.m_fnear);
		rayTracer->setBackgroundColor(rc.m_settings.m_col1);

		CRayTracerThread* render_thread = new CRayTracerThread(scene, rc, &img, rayTracer);
		CDlgStartThread dlg2(this, render_thread);
		dlg2.setTask("Rendering scene ...");
		if (dlg2.exec())
		{
			ShowImageViewer(img);
		}
		delete rayTracer;
	}
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
		ui->projectViewer->Update();
		UpdateTitle();
	}
}

QString CMainWindow::GetExportFEModelFilename(QString& formatOption)
{
	// supported file formats
	QStringList filters;
	filters << "FEBio files (*.feb)";
	filters << "NIKE3D files (*.n)";

	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		QStringList files = dlg.selectedFiles();
		QString fileName = QDir::toNativeSeparators(files.at(0));

		// get the filter
		QString flt = dlg.selectedNameFilter();
		int nflt = filters.indexOf(flt);
		switch (nflt)
		{
		case 0: formatOption = "feb"; break;
		case 1: formatOption = "n"; break;
		default:
			assert(false);
			formatOption.clear();
			break;
		}

		return fileName;
	}
	return QString();
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
		fileName = fi.completeBaseName() + QString(".feb");
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
						writer.SetMixedMeshFlag(dlg.m_allowHybrids);
						writer.SetSectionFlags(dlg.m_nsection);
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
						writer.SetSectionFlags(dlg.m_nsection);
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
						writer.SetSectionFlags(dlg.m_nsection);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0200)
					{
						// Write version 2.0
						FEBioExport2 writer(fem);
						writer.SetPlotfileCompressionFlag(dlg.m_compress);
						writer.SetSectionFlags(dlg.m_nsection);
						bsuccess = writer.Write(szfile);
						if (bsuccess == false) errMsg = QString::fromStdString(writer.GetErrorMessage());
					}
					else if (nformat == 0x0102)
					{
						// Write version 1.x
						FEBioExport12 writer(fem);
						writer.SetSectionFlags(dlg.m_nsection);
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
		filters << "VTU (*.vtu)";
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
				Post::VTKImport vtk(dummyFem);
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

bool CMainWindow::ImportImage(const QString& fileName)
{
	QFileInfo fi(fileName);
	QString ext = fi.suffix();
	if ((ext.compare("tiff", Qt::CaseInsensitive)==0) || (ext.compare("tif", Qt::CaseInsensitive)==0))
	{
		CImageModel* imageModel = new CImageModel(nullptr);
		imageModel->SetImageSource(new CTiffImageSource(imageModel, fileName.toStdString()));
		if (!ImportImage(imageModel))
		{
			delete imageModel;
			return false;
		}
		else ui->addToRecentImageFiles(fileName);
		return true;
	}
	else if (ext.compare("raw", Qt::CaseInsensitive)==0)
	{
		CDlgRAWImport dlg(this);
		if (dlg.exec())
		{
			BOX box(dlg.m_x0, dlg.m_y0, dlg.m_z0, dlg.m_x0 + dlg.m_w, dlg.m_y0 + dlg.m_h, dlg.m_z0 + dlg.m_d);

			CImageModel* imageModel = new CImageModel(nullptr);
			imageModel->SetImageSource(new CRawImageSource(imageModel, fileName.toStdString(), dlg.m_type, dlg.m_nx, dlg.m_ny, dlg.m_nz, box, dlg.m_swapEndianness));

			if (!ImportImage(imageModel))
			{
				delete imageModel;
			}
			else ui->addToRecentImageFiles(fileName);
		}
	}
	else if ((ext.compare("nrrd", Qt::CaseInsensitive) == 0) || (ext.compare("nhdr", Qt::CaseInsensitive) == 0))
	{
		return ProcessITKImage(fileName, CITKImageSource::NRRD);
	}
	else if ((ext.compare("dcm", Qt::CaseInsensitive) == 0) || (ext.compare("dicom", Qt::CaseInsensitive) == 0))
	{
		return ProcessITKImage(fileName, CITKImageSource::DICOM);
	}
	return false;
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
	filedlg.setFileMode(QFileDialog::ExistingFiles);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "RAW Files (*.raw)" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
        for(auto filename : filedlg.selectedFiles())
        {
            CDlgRAWImport dlg(this);
            if (dlg.exec())
            {
                BOX box(dlg.m_x0, dlg.m_y0, dlg.m_z0, dlg.m_x0 + dlg.m_w, dlg.m_y0 + dlg.m_h, dlg.m_z0 + dlg.m_d);

                CImageModel* imageModel = new CImageModel(nullptr);
                imageModel->SetImageSource(new CRawImageSource(imageModel, filename.toStdString(), dlg.m_type, dlg.m_nx, dlg.m_ny, dlg.m_nz, box, dlg.m_swapEndianness));

                if(!ImportImage(imageModel))
                {
                    delete imageModel;
                }
				else ui->addToRecentImageFiles(filename);
            }
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
		ProcessITKImage(filedlg.selectedFiles()[0], CITKImageSource::DICOM);
	}
}

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
	filedlg.setFileMode(QFileDialog::ExistingFiles);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "Tiff Files (*.tif *.tiff)" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
        for(auto filename : filedlg.selectedFiles())
        {
            CImageModel* imageModel = new CImageModel(nullptr);
            imageModel->SetImageSource(new CTiffImageSource(imageModel, filename.toStdString()));
            if (!ImportImage(imageModel))
            {
                delete imageModel;
            }
			else ui->addToRecentImageFiles(filename);

        }
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
	filedlg.setFileMode(QFileDialog::ExistingFiles);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	QStringList filters;
	filters << "NRRD Files (*.nrrd *.nhdr )" << "All Files (*)";
	filedlg.setNameFilters(filters);

	if (filedlg.exec())
	{
        for(auto filename : filedlg.selectedFiles())
        {
            ProcessITKImage(filename, CITKImageSource::NRRD);
        }
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
	filedlg.setFileMode(QFileDialog::ExistingFiles);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);

	if (filedlg.exec())
	{
        for(auto filename : filedlg.selectedFiles())
        {
		    ProcessITKImage(filename, CITKImageSource::OTHER);
        }
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
		ShowLogPanel();
		CConvertFEBtoFEB batch(this);
		batch.SetOutputFormat(dlg.getOutputFormat());
		batch.SetFiles(dlg.getFileNames());
		batch.SetOutputFolder(dlg.getOutPath());
		batch.SetSectionFlags(dlg.m_nsection);
		batch.Start();
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
			ShowLogPanel();
			CConvertFEBtoFSM f2f(this);
			f2f.SetFiles(fileNames);
			f2f.SetOutputFolder(dir);
			f2f.Start();
		}
	}
}

void CMainWindow::on_actionConvertFsm2Feb_triggered()
{
	CDlgConvertFEBio dlg(this);
	dlg.SetFileFilter(CDlgConvertFEBio::FSM_FILES);
	if (dlg.exec())
	{
		ShowLogPanel();
		CConvertFSMtoFEB f2f(this);
		f2f.SetFiles(dlg.getFileNames());
		f2f.SetOutputFolder(dlg.getOutPath());
		f2f.SetOutputFormat(dlg.getOutputFormat());
		f2f.SetSectionFlags(dlg.m_nsection);
		f2f.Start();
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
				CConvertGeoFiles conv(this);
				conv.SetFiles(fileNames);
				conv.SetOutputFolder(dir);
				conv.SetOutputFormat(dlg.GetFileFormat());
				conv.Start();
			}
		}
	}
}

void CMainWindow::on_actionExit_triggered()
{
	QApplication::closeAllWindows();
}

void CMainWindow::on_actionNewBatch_triggered()
{
	CDlgBatchRun dlg(this);
	if (dlg.exec())
	{
		FEBioBatchDoc* doc = new FEBioBatchDoc(this);
		doc->SetFileList(dlg.GetFileList());
		AddDocument(doc);
	}
}

void CMainWindow::on_actionOpenBatch_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Open FEBio Batch File", CurrentWorkingDirectory(), "FEBio Studio Batch files (*.fsbatch)");
	if (!fileName.isEmpty())
	{
		FEBioBatchDoc* doc = new FEBioBatchDoc(this);
		if (doc->LoadDocument(fileName) == false)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to open FEBio batch file.");
			delete doc;
		}
		else
		{
			AddDocument(doc);
		}
	}
}
