#include "stdafx.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include "ui_mainwindow.h"
#include "Document.h"
#include "Commands.h"
#include <FEBio/FEBioExport12.h>
#include <FEBio/FEBioExport2.h>
#include <FEBio/FEBioExport25.h>
#include <FEBio/FEBioExport3.h>
#include <Nike3D/FENIKEExport.h>
#include <MeshIO/FEBYUExport.h>
#include <MeshIO/FEHypersurfaceExport.h>
#include <LSDyna/FELSDYNAexport.h>
#include <MeshIO/FEMeshExport.h>
#include <MeshIO/FESTLExport.h>
#include <MeshIO/FEViewpointExport.h>
#include <MeshIO/FETetGenExport.h>
#include <MeshIO/FEVTKExport.h>
#include <MeshIO/FEPLYExport.h>
#include <GeomLib/GPrimitive.h>
#include <FEBio/FEBioImport.h>
#include <Abaqus/AbaqusImport.h>
#include <Ansys/AnsysImport.h>
#include <MeshIO/FEBinarySTLimport.h>
#include <MeshIO/FEBYUimport.h>
#include <Comsol/COMSOLImport.h>
#include <MeshIO/FEDXFimport.h>
#include <MeshIO/FEGMshImport.h>
#include <MeshIO/FEHMASCIIimport.h>
#include <MeshIO/FEHyperSurfImport.h>
#include <MeshIO/FEIDEASimport.h>
#include <MeshIO/FEIGESFileImport.h>
#include <LSDyna/FELSDYNAimport.h>
#include <MeshIO/FEMeshImport.h>
#include <MeshIO/FENASTRANimport.h>
#include <MeshIO/FEPLYImport.h>
#include <MeshIO/FERAWImport.h>
#include <MeshIO/FESTLimport.h>
#include <MeshIO/FETetGenImport.h>
#include <MeshIO/FEVTKImport.h>
#include <Nike3D/NikeImport.h>
#include <MeshIO/PRVObjectImport.h>
#include <MeshIO/PRVObjectExport.h>
#include <MeshIO/BREPImport.h>
#include <MeshIO/STEPImport.h>
#include "DlgImportAbaqus.h"
#include "DlgRAWImport.h"
#include "DlgImportCOMSOL.h"
#include "DlgLSDYNAExport.h"
#include "DlgVTKExport.h"
#include "DlgExportFEBio.h"
#include "DlgTransform.h"
#include "DlgMergeObjects.h"
#include "DlgNew.h"
#include "DlgCloneObject.h"
#include "DlgCloneGrid.h"
#include "DlgCloneRevolve.h"
#include "DlgImportSTL.h"
#include "DlgEditProject.h"
#include "DlgDetachSelection.h"
#include "DlgModelInfo.h"
#include "DlgExportLSDYNA.h"
#include <GeomLib/GSurfaceMeshObject.h>
#include <QDesktopServices>
#include "DlgPurge.h"
#include "DlgBatchConvert.h"
#include "version.h"
#include "PostDoc.h"
#include "DlgFind.h"
#include "DlgSelectRange.h"
#include <PostGL/GLColorMap.h>
#include <PostGL/GLModel.h>
#include <QtCore/QTextStream>
#include <PostLib/ImageModel.h>
#include <PostLib/GView.h>
#include <PostLib/FELSDYNAExport.h>
#include <MeshTools/GModel.h>
#include "DlgExportXPLT.h"
#include <XPLTLib/xpltFileExport.h>
#include <iostream>

#ifdef HAS_QUAZIP
#include "ZipFiles.h"
#endif

void CMainWindow::on_actionNew_triggered()
{
	// check to see if the document is modified or not
	if (m_doc->IsModified())
	{
		QString msg("The project was modified since the last save.\nDo you want to save the project ?");
		int n = QMessageBox::question(this, "FEBio Studio", msg, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
		if (n == QMessageBox::Yes)
		{
			on_actionSave_triggered();
		}
		else if (n == QMessageBox::Cancel) return;
	}

	// close all views first
	int n = ui->tab->count();
	for (int i = 1; i < n; ++i) ui->tab->closeView(1);

	// ask the user for a new project template
	bool btemplate = false;
	CDlgNew dlg(this);

	dlg.setShowOnStart(ui->m_showNewOnStartup);

	if (ui->m_defaultProjectFolder.isEmpty() == false)
	{
		dlg.setProjectFolder(ui->m_defaultProjectFolder);
	}

	bool projectAccepted = false;
	while (projectAccepted == false)
	{
		projectAccepted = true;
		if (dlg.exec())
		{
			if (dlg.createNew())
			{
				// get the project name and folder
				QString projectName = dlg.getProjectName();
				QString projectFolder = dlg.getProjectFolder();

				QDir dir(projectFolder);

				// see if we should create a project folder
				if (dlg.createProjectFolder())
				{
					// try to create a new folder
					if (dir.mkdir(projectName) == false)
					{
						QMessageBox::critical(this, "FEBio Studio", QString("The folder \"%1\" already exists in the selected project folder.\nPlease choose a different project name or folder.").arg(projectName));
						projectAccepted = false;
					}
					else
					{
						// cd into this directory
						dir.cd(projectName);
					}
				}

				if (projectAccepted)
				{
					ui->m_defaultProjectFolder = projectFolder;

					// setup model file name and path
					QString modelFileName = projectName + ".fsprj";
					QString modelFilePath = dir.absoluteFilePath(modelFileName);
//					QString modelJobsDir = modelFilePath +"/jobs";
//					QString modelResourcesDir = modelFilePath +"/resources";

					// see if this file already exists
					QFile file(modelFilePath);
					if (file.exists())
					{
						QMessageBox::critical(this, "FEBio Studio", QString("The file \"%1\" already exists in the selected project folder.\nPlease choose a different project name or folder").arg(modelFileName));
						projectAccepted = false;
					}
					else
					{
						// load the template
						int nchoice = dlg.getTemplate();
						btemplate = m_doc->LoadTemplate(nchoice);
						if (btemplate == false)
						{
							QMessageBox::critical(this, "Load Template", "Failed loading project template");
						}

						// reset all UI elements
						UpdatePhysicsUi();
						Reset();
						UpdateTitle();
						UpdateModel();
						Update();
						ClearLog();
						ClearOutput();

						// save empty model file
						SaveProject(modelFilePath);
					}
				}
			}
			else
			{
				if (dlg.openRecentFile())
				{
					QString fileName = dlg.getRecentFileName();

					// read the file
					std::string sfile = fileName.toStdString();
					OpenDocument(sfile.c_str());
				}
				else
				{
					// Open a file
					on_actionOpen_triggered();
					return;
				}
			}
		}
		else
		{
			m_doc->NewDocument();
			UpdatePhysicsUi();
			Reset();
			UpdateTitle();
			UpdateModel();
			Update();
			ClearLog();
			ClearOutput();
		}
	}

	ui->m_showNewOnStartup = dlg.showOnStart();

	ui->glview->Reset();
	if (ui->modelViewer && btemplate) ui->modelViewer->Show();
}

void CMainWindow::OpenFile(const QString& fileName, bool showLoadOptions)
{
	QString ext = QFileInfo(fileName).suffix();
	if (ext.compare("fsprj", Qt::CaseInsensitive) == 0)
	{
		// read the file
		OpenDocument(fileName);
	}
	else if (ext.compare("prv", Qt::CaseInsensitive) == 0)
	{
		// read the file
		OpenDocument(fileName);
	}
	else if (ext.compare("xplt", Qt::CaseInsensitive) == 0)
	{
		// load the plot file
		OpenPlotFile(fileName, showLoadOptions);
	}
	else if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
		// load the feb file
		OpenFEModel(fileName);
	}
	else if (ext.compare("prj", Qt::CaseInsensitive) == 0)
	{
		// Extract the project file and open it
		ImportProject(fileName);
	}
	else
	{
		// Open any other files (e.g. log files) with the system's associated program
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
//		assert(false);
//		QMessageBox::critical(this, "FEBio Studio", "Does not compute!");
	}
}

void CMainWindow::on_actionOpen_triggered()
{
	QStringList filters;
	filters << "FEBio Studio Projects (*.fsprj)";
	filters << "PreView files (*.prv)";
	filters << "FEBio plot files (*.xplt)";

	QFileDialog dlg(this, "Open");
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(ui->currentPath);
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
	std::string fileName = m_doc->GetDocFilePath();
	if (fileName.empty()) on_actionSaveAs_triggered();
	else
	{
		SaveProject(QString::fromStdString(fileName));
	}
}


#ifdef HAS_QUAZIP
void CMainWindow::on_actionImportProject_triggered()
{
	QStringList filters;
	filters << "FBS Project Archives (*.prj)";

	QFileDialog dlg(this, "Open");
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(ui->currentPath);
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// store the current path
		QDir dir = dlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		ImportProject(fileName);


//		QFileInfo fileInfo(fileName);
//
//		// get the parent directory's name
//		QString parentDirName = fileInfo.path();
//
//		// create folder in which to unzip
//		QDir parentDir(parentDirName);
//		parentDir.mkdir(fileInfo.completeBaseName());
//		QString destDir = parentDirName + "/" + fileInfo.completeBaseName();
//
//		// extract files
//		QStringList extractedFiles = JlCompress::extractFiles(fileName, JlCompress::getFileList(fileName), destDir);
//
//		// open first .fsprj file
//		for(QString str : extractedFiles)
//		{
//			if(QFileInfo(str).suffix().compare("fsprj", Qt::CaseInsensitive) == 0)
//			{
//				OpenDocument(str);
//				break;
//			}
//		}

	}
}

void CMainWindow::on_actionExportProject_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export", ui->currentPath, "FEBio Studio Project (*.prj)");
	if (fileName.isEmpty() == false)
	{
		// make sure the file has an extension
		std::string sfile = fileName.toStdString();
		std::size_t found = sfile.rfind(".");
		if (found == std::string::npos) sfile.append(".prj");

		std::string pathName = m_doc->GetDocFolder();

		archive(QString::fromStdString(sfile), QDir(QString::fromStdString(pathName)));

	}
}
#else
void CMainWindow::on_actionImportProject_triggered() {}
void CMainWindow::on_actionExportProject_triggered() {}
#endif

bool CMainWindow::SaveProject(const QString& fileName)
{
	// make sure the file has an extension
	std::string sfile = fileName.toStdString();
	std::size_t found = sfile.rfind(".");
	if (found == std::string::npos) sfile.append(".fsprj");

	// start log message
	ui->logPanel->AddText(QString("Saving project: %1 ...").arg(QString::fromStdString(sfile)));

	// save the document
	bool success = m_doc->SaveDocument(sfile.c_str());

	ui->logPanel->AddText(success ? "SUCCESS\n" : "FAILED\n");

	// save the document
	if (success)
	{
		UpdateTitle();
		ui->addToRecentFiles(fileName);
	}
	else
	{
		QString errStr = QString("Failed storing file to:\n%1").arg(QString::fromStdString(sfile));
		QMessageBox::critical(this, "FEBio Studio", errStr);
	}

	return success;
}

void CMainWindow::on_actionSaveAs_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this, "Export", ui->currentPath, "FEBio Studio Project (*.fsprj)");
	if (fileName.isEmpty() == false)
	{
		SaveProject(fileName);
	}
}

void CMainWindow::SaveImage(QImage& image)
{
	const uchar* bits = image.bits();

	QStringList filters;
	filters << "Bitmap files (*.bmp)"
		<< "PNG files (*.png)"
		<< "JPEG files (*.jpg)";

	QFileDialog dlg(this, "Save Image");
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

void CMainWindow::on_actionSnapShot_triggered()
{
	QImage img = ui->glview->CaptureScreen();
	SaveImage(img);
}

void CMainWindow::on_actionInfo_triggered()
{
	CDocument* doc = GetDocument();
	QString s = QString::fromStdString(doc->getModelInfo());
	CDlgModelInfo dlg(this);
	dlg.setText(s);
	dlg.exec();
	s = dlg.text();
	doc->setModelInfo(s.toStdString());
}

FileReader* CMainWindow::CreateFileReader(const QString& fileName)
{
	// get the file extension
	QString ext = QFileInfo(fileName).suffix();
	if (ext.compare("pvo" , Qt::CaseInsensitive) == 0) return new PRVObjectImport();
	if (ext.compare("inp"    , Qt::CaseInsensitive) == 0) 
	{
		AbaqusImport* reader = new AbaqusImport();
		CDlgImportAbaqus dlg(reader, this);
		if (dlg.exec())
		{
			return reader;
		}
		else return 0;
	}
	if (ext.compare("cdb"    , Qt::CaseInsensitive) == 0) return new AnsysImport();
	if (ext.compare("k"      , Qt::CaseInsensitive) == 0) return new FELSDYNAimport();
	if (ext.compare("unv"    , Qt::CaseInsensitive) == 0) return new FEIDEASimport();
	if (ext.compare("nas"    , Qt::CaseInsensitive) == 0) return new FENASTRANimport();
	if (ext.compare("dxf"    , Qt::CaseInsensitive) == 0) return new FEDXFimport();
	if (ext.compare("stl"    , Qt::CaseInsensitive) == 0)
	{
		CDlgImportSTL dlg(this);
		if (dlg.exec())
		{
			switch (dlg.m_nselect)
			{
			case 0: return new FESTLimport(); break;
			case 1: return new FEBinarySTLimport(); break;
			default:
				assert(false);
				return 0;
			}
		}
		else return 0;
	}
	if (ext.compare("hmascii", Qt::CaseInsensitive) == 0) return new FEHMASCIIimport();
	if (ext.compare("surf"   , Qt::CaseInsensitive) == 0) return new FEHyperSurfImport();
	if (ext.compare("msh"    , Qt::CaseInsensitive) == 0) return new FEGMshImport();
	if (ext.compare("byu"    , Qt::CaseInsensitive) == 0) return new FEBYUimport();
	if (ext.compare("mesh"   , Qt::CaseInsensitive) == 0) return new FEMeshImport();
	if (ext.compare("ele"    , Qt::CaseInsensitive) == 0) return new FETetGenImport();
//	if (ext.compare("iges"   , Qt::CaseInsensitive) == 0) return new FEIGESFileImport();
	if (ext.compare("vtk"    , Qt::CaseInsensitive) == 0) return new FEVTKimport();
	if (ext.compare("raw"    , Qt::CaseInsensitive) == 0)
	{
		CDlgRAWImport dlg(this);
		if (dlg.exec())
		{
			FERAWImport* reader = new FERAWImport();
			reader->SetImageDimensions(dlg.m_nx, dlg.m_ny, dlg.m_nz);
			reader->SetBoxSize(dlg.m_x0, dlg.m_y0, dlg.m_z0, dlg.m_w, dlg.m_h, dlg.m_d);
			return reader;
		}
		else return 0;
	}
	if (ext.compare("mphtxt", Qt::CaseInsensitive) == 0)
	{
		COMSOLimport* reader = new COMSOLimport();
		CDlgImportCOMSOL dlg(reader, this);
		if (dlg.exec())
		{
			return reader;
		}
		else return 0;
	}
	if (ext.compare("ply", Qt::CaseInsensitive) == 0) return new FEPLYImport();
	if ((ext.compare("brep", Qt::CaseInsensitive) == 0) ||
        (ext.compare("brp", Qt::CaseInsensitive) == 0)) return new BREPImport();
	if ((ext.compare("step", Qt::CaseInsensitive) == 0) ||
        (ext.compare("stp", Qt::CaseInsensitive) == 0)) return new STEPImport;
	if ((ext.compare("iges", Qt::CaseInsensitive) == 0) ||
        (ext.compare("igs", Qt::CaseInsensitive) == 0)) return new IGESImport;
	if (ext.compare("feb", Qt::CaseInsensitive) == 0)
	{
		FEBioImport* febio = new FEBioImport;
		febio->SetGeometryOnlyFlag(true);
		return febio;
	}

	return 0;
}

void CMainWindow::on_actionImportFEModel_triggered()
{
	QStringList filters;
	filters << "All files (*.feb *.n *.inp)";
	filters << "FEBio files (*.feb)";
	filters << "NIKE3D files (*.n)";
	filters << "Abaqus files (*.inp)";

	QFileDialog dlg(this, "Open");
	dlg.setFileMode(QFileDialog::ExistingFile);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setDirectory(ui->currentPath);
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// store the current path
		QDir dir = dlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file name
		QStringList files = dlg.selectedFiles();
		QString fileName = files.first();

		// read the file
		std::string sfile = fileName.toStdString();
		OpenFEModel(sfile.c_str());
	}
}

void CMainWindow::on_actionExportFEModel_triggered()
{
	// supported file formats
	QStringList filters;
	filters << "FEBio files (*.feb)";
	filters << "NIKE3D files (*.n)";

	// default extensions
	const char* szext[] = {
		".feb",
		".n",
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
	dlg.setDirectory(ui->currentPath);
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
		FEProject& fem = m_doc->GetProject();

		AddLogEntry(QString("Writing file %1 ... ").arg(fileName));

		// export file based on selected filter
		switch (nflt)
		{
		case 0: // FEBio files
		{
			CDlgExportFEBio dlg(this);
			if (dlg.exec())
			{
				// Do a model check
				if (DoModelCheck() == false)
				{
					AddLogEntry(QString("cancelled\n"));
					return;
				}

				if (dlg.m_nversion == 0)
				{
					// Write version 1.x
					FEBioExport12 writer;
					for (int i = 0; i<FEBioExport12::MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to FEBio file: \n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
				}
				else if (dlg.m_nversion == 1)
				{
					// Write version 2.0
					FEBioExport2 writer;
					writer.SetPlotfileCompressionFlag(dlg.m_compress);
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to FEBio file: \n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
				}
				else if (dlg.m_nversion == 2)
				{
					// write version 2.5
					FEBioExport25 writer;
					writer.SetPlotfileCompressionFlag(dlg.m_compress);
					writer.SetExportSelectionsFlag(dlg.m_bexportSelections);
					writer.SetWriteNotesFlag(dlg.m_writeNotes);
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to FEBio file: \n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
				}
				else if (dlg.m_nversion == 3)
				{
					// write version 3.0
					FEBioExport3 writer;
					writer.SetPlotfileCompressionFlag(dlg.m_compress);
					writer.SetExportSelectionsFlag(dlg.m_bexportSelections);
					writer.SetWriteNotesFlag(dlg.m_writeNotes);
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer.SetSectionFlag(i, dlg.m_nsection[i]);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to FEBio file: \n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
				}
			}
		}
		break;
		case 1: // NIKE3D files
		{
			FENIKEExport writer;
			if (!writer.Export(fem, szfile))
				QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to NIKE file."));
		}
		break;
		default:
			QMessageBox::critical(this, "FEBio Studio", "Don't know how to save this file.");
		}
		AddLogEntry(QString("success!\n"));
	}
}

void CMainWindow::on_actionImportGeometry_triggered()
{
	// file filters
	QStringList filters;
	filters << "All files (*)";
	filters << "PreView Object File (*.pvo)";
	filters << "FEBio (*.feb)";
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
	filters << "IGES (*.iges, *.igs)";
	filters << "VTK (*.vtk)";
	filters << "RAW Image (*.raw)";
	filters << "COMSOL Mesh (*.mphtxt)";
	filters << "PLY (*.ply)";
	filters << "BREP files (*.brep *.brp)";
	filters << "STEP files (*.step *.stp)";
	
	// get the project
	FEProject& prj = m_doc->GetProject();

	// present the file selection dialog box
	QFileDialog dlg(this);
	dlg.setFileMode(QFileDialog::ExistingFiles);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	dlg.setNameFilters(filters);
	if (dlg.exec())
	{
		// store the current path
		QDir dir = dlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file names
		QStringList files = dlg.selectedFiles();

		// import the file
		ImportFiles(files);
	}
}

void CMainWindow::on_actionImportImage_triggered()
{
	QStringList filters;
	filters << "RAW files (*.raw)";

	CDocument* doc = GetDocument();

	// present the file selection dialog box
	QFileDialog filedlg(this);
	filedlg.setFileMode(QFileDialog::ExistingFile);
	filedlg.setAcceptMode(QFileDialog::AcceptOpen);
	filedlg.setNameFilters(filters);
	if (filedlg.exec())
	{
		// store the current path
		QDir dir = filedlg.directory();
		SetCurrentFolder(dir.absolutePath());

		// get the file name
		QStringList files = filedlg.selectedFiles();
		QString fileName = files.at(0);
		std::string sfile = fileName.toStdString();

		// create 'resources' subdirectory
		std::string sPath = doc->GetDocFolder();

		if (!sPath.empty())
		{
			QString projectPath = QString::fromStdString(sPath);
			QDir projectDir(projectPath);
			projectDir.mkdir("resources");
			projectDir.cd("resources");
			QString resourceDir = projectDir.absolutePath();


			// store path for linked file
			QString linkName = projectDir.absoluteFilePath(QFileInfo(fileName).fileName());

			// add .lnk extension to link when on windows
#ifdef WIN32
			linkName += ".lnk";
#endif
			// create link in resources directory
			QFile originalFile(fileName);
			originalFile.link(linkName);

			// store path to newly created link
			sfile = linkName.toStdString();
		}

		CDlgRAWImport dlg(this);
		if (dlg.exec())
		{
			BOX box(dlg.m_x0, dlg.m_y0, dlg.m_z0, dlg.m_x0 + dlg.m_w, dlg.m_y0 + dlg.m_h, dlg.m_z0 + dlg.m_d);

			Post::CImageModel* po = doc->ImportImage(sfile, dlg.m_nx, dlg.m_ny, dlg.m_nz, box);
			if (po == nullptr)
			{
				QMessageBox::critical(this, "FEBio Studio", "Failed importing image data.");
			}
			else
			{
				Update(0, true);
				ZoomTo(po->GetBoundingBox());
				ShowInModelViewer(po);
			}
		}
		else return;
	}
}

void CMainWindow::on_actionExportGeometry_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc) ExportPostGeometry();
	else ExportGeometry();
}

void CMainWindow::ExportPostGeometry()
{
	CPostDoc* doc = GetActiveDocument();
	if ((doc == nullptr) || (doc->IsValid() == false)) return;

	QStringList filters;
	filters << "FEBio xplt files (*.xplt)"
		//		<< "FEBio files (*.feb)"
		//		<< "ASCII files (*.*)"
		//		<< "VRML files (*.wrl)"
		<< "LSDYNA Keyword (*.k)";
//		<< "BYU files(*.byu)"
//		<< "NIKE3D files (*.n)"
//		<< "VTK files (*.vtk)"
//		<< "LSDYNA database (*.d3plot)";

	QFileDialog dlg(this, "Save");
	dlg.setFileMode(QFileDialog::AnyFile);
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

	Post::FEPostModel& fem = *doc->GetFEModel();

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
		CDlgExportLSDYNA dlg(this);
		if (dlg.exec())
		{
			Post::FELSDYNAExport w;
			w.m_bsel = dlg.m_bsel;
			w.m_bsurf = dlg.m_bsurf;
			w.m_bnode = dlg.m_bnode;
			bret = w.Save(fem, doc->GetActiveState(), szfilename);
		}
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
		CDlgExportVTK dlg(this);
		if (dlg.exec())
		{
			Post::FEVTKExport w;
			w.ExportAllStates(dlg.m_ops[0]);
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
	// supported file formats
	QStringList filters;
	filters << "PreView Object File (*.pvo)";
	filters << "LSDYNA keyword (*.k)";
	filters << "HyperSurface files (*.surf)";
	filters << "BYU files (*.byu)";
	filters << "STL-ASCII files (*.stl)";
	filters << "ViewPoint files (*.*)";
	filters << "Mesh files (*.mesh)";
	filters << "TetGen files (*.ele)";
	filters << "VTK files (*.vtk)";

	// default extensions
	const char* szext[] = {
		".pvo",
		".k",
		".surf",
		".byu",
		".stl",
		".txt",
		".mesh",
		".ele",
		".vtk"
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
	dlg.setDirectory(ui->currentPath);
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
		FEProject& fem = m_doc->GetProject();

		AddLogEntry(QString("Writing file %1 ... ").arg(fileName));

		// export file based on selected filter
		switch (nflt)
		{
		case 0: // PreView Object files
			{
				PRVObjectExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to PreView Object file."));
			}
			break;
		case 1: // LSDYNA keyword
			{
				CDlgLSDYNAExport dlg(this);
				if (dlg.exec())
				{
					LSDYNAEXPORT ops;
					ops.bselonly = dlg.m_bselonly;
					ops.bshellthick = dlg.m_bshell_thick;

					FELSDYNAexport writer;
					writer.SetOptions(ops);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to LSDYNA keyword file."));
				}
			}
			break;
		case 2:
			{
			  FEHypersurfaceExport writer;
			  if (!writer.Export(fem, szfile))
				  QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to surf file."));
			}
			break;
		case 3: // BYU files
			{
				FEBYUExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to byu file."));
			}
			break;
		case 4: // STL files
			{
				FESTLExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save model to STL file:\n%1").arg(QString::fromStdString(writer.GetErrorMessage())));
			}
			break;
		case 5: // ViewPoint files
			{
				FEViewpointExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to viewpoint file."));
			}
			break;
		case 6:
			{
				FEMeshExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to Mesh file."));
			}
			break;
		case 7:
			{
				FETetGenExport writer;
				if (!writer.Export(fem, szfile))
					QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to TetGen file."));
			}
			break;
		case 8: // VTK files
			{
				CDlgVTKExport dlg(this);
				if (dlg.exec())
				{
					VTKEXPORT ops;
					ops.bshellthick = dlg.m_bshell_thick;
					ops.bscalar_data = dlg.m_bscalar_data;
					FEVTKExport writer;
					writer.SetOptions(ops);
					if (!writer.Export(fem, szfile))
						QMessageBox::critical(this, "FEBio Studio", QString("Couldn't save project to vtk file."));
				}
			}
			break;
		default:
			QMessageBox::critical(this, "FEBio Studio", "Don't know how to save this file.");
		}
		AddLogEntry(QString("success!\n"));
	}
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
	QStringList fileNames = QFileDialog::getOpenFileNames(this, "Select Files", "", "FEBio files (*.feb)");
	if (fileNames.isEmpty() == false)
	{
		QString dir = QFileDialog::getExistingDirectory();
		if (dir.isEmpty() == false)
		{
			CDlgExportFEBio dlg(this);
			if (dlg.exec())
			{
				CDocument& doc = *GetDocument();
				FEBioImport reader;

				FEFileExport* exporter = 0;			
				if (dlg.m_nversion == 0)
				{
					// Write version 1.x
					FEBioExport12* writer = new FEBioExport12; exporter = writer;
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (dlg.m_nversion == 1)
				{
					// Write version 2.0
					FEBioExport2* writer = new FEBioExport2; exporter = writer;
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}
				else if (dlg.m_nversion == 2)
				{
					// write version 2.5
					FEBioExport25* writer = new FEBioExport25; exporter = writer;
					for (int i = 0; i<FEBIO_MAX_SECTIONS; ++i) writer->SetSectionFlag(i, dlg.m_nsection[i]);
				}

				QStringList::iterator it;

				AddLogEntry("Starting batch conversion ...\n");
				int nsuccess = 0, nfails = 0, nwarnings = 0;
				for (it = fileNames.begin(); it != fileNames.end(); ++it)
				{
					std::string file = it->toStdString();
					AddLogEntry(QString("Converting %1 ... ").arg(*it));
					QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

					// create an output file name
					QString outName = createFileName(*it, dir, "feb");
					string sout = outName.toStdString();

					bool bret = doc.Convert(file, sout, &reader, exporter);
					AddLogEntry(bret ? "success\n" : "FAILED\n");
					string err = reader.GetErrorMessage();
					if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
					err = exporter->GetErrorMessage();
					if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
					
					if (bret) nsuccess++; else nfails++;
					QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
				}

				AddLogEntry("Batch conversion completed:\n");
				if (nwarnings == 0)
					AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
				else
					AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));

				delete exporter;
			}
		}
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
				FEFileExport* exporter = nullptr;
				switch (format)
				{
				case 0: exporter = new FEVTKExport; ext = "vtk"; break;
				case 1: exporter = new FEPLYExport; ext = "ply"; break;
				case 2: exporter = new FELSDYNAexport; ext = "k"; break;
				case 3: exporter = new FEHypersurfaceExport; ext = "surf"; break;
				case 4: exporter = new FEBYUExport; ext = "byu"; break;
				case 5: exporter = new FESTLExport; ext = "stl"; break;
				case 6: exporter = new FEViewpointExport; ext = "vp"; break;
				case 7: exporter = new FEMeshExport; ext = "mesh"; break;
				case 8: exporter = new FETetGenExport; ext = "ele"; break;
				}

				if (exporter == nullptr)
				{
					QMessageBox::critical(this, "FEBio Studio", "Cannot create file exporter.");
					return;
				}

				// start convert process
				CDocument* doc = GetDocument();
				AddLogEntry("Starting batch conversion ...\n");
				int nsuccess = 0, nfails = 0, nwarnings = 0;
				QStringList::iterator it;
				for (it = fileNames.begin(); it != fileNames.end(); ++it)
				{
					std::string file = it->toStdString();
					AddLogEntry(QString("Converting %1 ... ").arg(*it));
					QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);

					// create a file reader based on the file extension
					FileReader* reader = CreateFileReader(*it);

					FEFileImport* importer = dynamic_cast<FEFileImport*>(reader);
					if (importer)
					{
						// create an output file name
						QString outName = createFileName(*it, dir, ext);
						string sout = outName.toStdString();

						bool bret = doc->Convert(file, sout, importer, exporter);
						AddLogEntry(bret ? "success\n" : "FAILED\n");
						string err = reader->GetErrorMessage();
						if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }
						err = exporter->GetErrorMessage();
						if (err.empty() == false) { AddLogEntry(QString::fromStdString(err) + "\n"); nwarnings++; }

						if (bret) nsuccess++; else nfails++;
						QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
					}
					else
					{
						AddLogEntry("FAILED (can't create file reader!)\n");
					}
					delete reader;
				}

				AddLogEntry("Batch conversion completed:\n");
				if (nwarnings == 0)
					AddLogEntry(QString("%1 converted, %2 failed\n").arg(nsuccess).arg(nfails));
				else
					AddLogEntry(QString("%1 converted, %2 failed, warnings were generated\n").arg(nsuccess).arg(nfails));

				delete exporter;

			}
		}
	}
}

void CMainWindow::on_actionExit_triggered()
{
	QApplication::closeAllWindows();
}

void CMainWindow::on_actionUndo_triggered()
{
	if (m_doc->CanUndo())
	{
		m_doc->UndoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionRedo_triggered()
{
	if (m_doc->CanRedo())
	{
		m_doc->RedoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionInvertSelection_triggered()
{
	FESelection* ps = m_doc->GetCurrentSelection();
	if (ps)
	{
		m_doc->DoCommand(new CCmdInvertSelection());
		Update();
	}
}

void CMainWindow::on_actionClearSelection_triggered()
{
	FESelection* ps = m_doc->GetCurrentSelection();
	if (ps && ps->Size()) 
	{
		int item = m_doc->GetItemMode();
		int nsel = m_doc->GetSelectionMode();
		GObject* po = m_doc->GetActiveObject();
		FEMesh* pm = (po?po->GetFEMesh():0);
		FEMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
		FELineMesh* pml = (po ? po->GetEditableLineMesh() : 0);
		switch (item)
		{
		case ITEM_MESH: 
			{
				switch (nsel)
				{
				case SELECT_OBJECT  : m_doc->DoCommand(new CCmdSelectObject  (0, false)); break;
				case SELECT_PART    : m_doc->DoCommand(new CCmdSelectPart    (m_doc->GetFEModel(), 0, 0, false)); break;
				case SELECT_FACE    : m_doc->DoCommand(new CCmdSelectSurface (m_doc->GetFEModel(), 0, 0, false)); break;
				case SELECT_EDGE    : m_doc->DoCommand(new CCmdSelectEdge    (m_doc->GetFEModel(), 0, 0, false)); break;
				case SELECT_NODE    : m_doc->DoCommand(new CCmdSelectNode    (m_doc->GetFEModel(), 0, 0, false)); break;
				case SELECT_DISCRETE: m_doc->DoCommand(new CCmdSelectDiscrete(m_doc->GetGModel(), 0, 0, false)); break;
				}
			}
			break;
		case ITEM_ELEM: m_doc->DoCommand(new CCmdSelectElements(pm, 0, 0, false)); break;
		case ITEM_FACE: m_doc->DoCommand(new CCmdSelectFaces   (pmb, 0, 0, false)); break;
		case ITEM_EDGE: m_doc->DoCommand(new CCmdSelectFEEdges (pml, 0, 0, false)); break;
		case ITEM_NODE: m_doc->DoCommand(new CCmdSelectFENodes (pml, 0, 0, false)); break;
		}
		
		Update();
	}
}

void CMainWindow::on_actionDeleteSelection_triggered()
{
	// give the build panel a chance to process this event first
	if (ui->buildPanel->OnDeleteEvent())
	{
		return;
	}

	// see if the focus is on the model viewer
	if (ui->modelViewer->IsFocus())
	{
		if (ui->modelViewer->OnDeleteEvent())
			return;
	}

	FESelection* psel = m_doc->GetCurrentSelection();
	if (psel == 0) return;

	if (dynamic_cast<GDiscreteSelection*>(psel))
	{
		GDiscreteSelection* pds = dynamic_cast<GDiscreteSelection*>(psel);
		CCmdGroup* pcmd = new CCmdGroup("Delete Discrete");
		FEModel* ps = m_doc->GetFEModel();
		GModel& model = ps->GetModel();
		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteObject* po = model.DiscreteObject(i);
			if (po->IsSelected())
				pcmd->AddCommand(new CCmdDeleteDiscreteObject(po));
		}
		m_doc->DoCommand(pcmd);

		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(i));
			if (pds)
			{
				for (int i = 0; i<pds->size();)
				{
					if (pds->element(i).IsSelected()) pds->RemoveElement(i);
					else i++;
				}
			}
		}
	}
	else if (dynamic_cast<GPartSelection*>(psel))
	{
		GModel& m = *m_doc->GetGModel();
		GPartSelection* sel = dynamic_cast<GPartSelection*>(psel);
		int n = sel->Count();
		if (n == 0) return;
		GPartSelection::Iterator it(sel);
		vector<int> pid(n);
		for (int i=0; i<n; ++i, ++it)
		{
			pid[i] = it->GetID();
		}

		for (int i=0; i<n; ++i)
		{
			GPart* pg = m.FindPart(pid[i]); assert(pg);
			if (pg)
			{
				m.DeletePart(pg);
			}
		}
	}
	else
	{
		int item = m_doc->GetItemMode();
		if (item == ITEM_MESH)
		{
			int nsel = m_doc->GetSelectionMode();
			if (nsel == SELECT_OBJECT)
			{
				CCmdGroup* pcmd = new CCmdGroup("Delete");
				FEModel* ps = m_doc->GetFEModel();
				GModel& model = ps->GetModel();
				for (int i = 0; i<model.Objects(); ++i)
				{
					GObject* po = model.Object(i);
					if (po->IsSelected())
						pcmd->AddCommand(new CCmdDeleteGObject(&model, po));
				}
				m_doc->DoCommand(pcmd);
			}
			else
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete this selection.");
			}
		}
		else
		{
			GObject* po = m_doc->GetActiveObject();
			if (po == 0) return;

			GMeshObject* pgo = dynamic_cast<GMeshObject*>(po);
			if (pgo && pgo->GetFEMesh()) m_doc->DoCommand(new CCmdDeleteFESelection(pgo));

			GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
			if (pso && pso->GetSurfaceMesh()) m_doc->DoCommand(new CCmdDeleteFESurfaceSelection(pso));

			GPrimitive* pp = dynamic_cast<GPrimitive*>(po);
			if (pp)
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete mesh selections of a primitive object.");
			}
		}
	}
	Update(0, true);
	ClearStatusMessage();
}

void CMainWindow::on_actionHideSelection_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		switch (mdl.GetSelectionMode())
		{
		case Post::SELECT_NODES: mdl.HideSelectedNodes(); break;
		case Post::SELECT_EDGES: mdl.HideSelectedEdges(); break;
		case Post::SELECT_FACES: mdl.HideSelectedFaces(); break;
		case Post::SELECT_ELEMS: mdl.HideSelectedElements(); break;
		}
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CDocument* pdoc = GetDocument();

		pdoc->HideCurrentSelection();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionHideUnselected_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.HideUnselectedElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CDocument* pdoc = GetDocument();
		pdoc->HideUnselected();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionUnhideAll_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.UnhideAll();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CDocument* doc = GetDocument();
		doc->DoCommand(new CCmdUnhideAll());
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionFind_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	Post::CGLModel* model = doc->GetGLModel(); assert(model);
	if (model == 0) return;

	int nview = model->GetSelectionMode();
	int nsel = 0;
	if (nview == Post::SELECT_NODES) nsel = 0;
	if (nview == Post::SELECT_EDGES) nsel = 1;
	if (nview == Post::SELECT_FACES) nsel = 2;
	if (nview == Post::SELECT_ELEMS) nsel = 3;

	CDlgFind dlg(this, nsel);

	if (dlg.exec())
	{
		Post::CGLModel* pm = doc->GetGLModel();

		if (dlg.m_bsel[0]) nview = Post::SELECT_NODES;
		if (dlg.m_bsel[1]) nview = Post::SELECT_EDGES;
		if (dlg.m_bsel[2]) nview = Post::SELECT_FACES;
		if (dlg.m_bsel[3]) nview = Post::SELECT_ELEMS;

		CGLControlBar* pb = ui->glc;
		switch (nview)
		{
		case Post::SELECT_NODES: pb->SetMeshItem(ITEM_NODE); pm->SelectNodes(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_EDGES: pb->SetMeshItem(ITEM_EDGE); pm->SelectEdges(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_FACES: pb->SetMeshItem(ITEM_FACE); pm->SelectFaces(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_ELEMS: pb->SetMeshItem(ITEM_ELEM); pm->SelectElements(dlg.m_item, dlg.m_bclear); break;
		}

		doc->GetGLModel()->UpdateSelectionLists();
		ReportSelection();
		RedrawGL();
	}
}

void CMainWindow::on_actionSelectRange_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return;
	if (!postDoc->IsValid()) return;

	Post::CGLModel* model = postDoc->GetGLModel(); assert(model);
	if (model == 0) return;

	Post::CGLColorMap* pcol = postDoc->GetGLModel()->GetColorMap();
	if (pcol == 0) return;

	float d[2];
	pcol->GetRange(d);

	CDlgSelectRange dlg(this);
	dlg.m_min = d[0];
	dlg.m_max = d[1];

	if (dlg.exec())
	{
		CDocument* doc = GetDocument();
		switch (model->GetSelectionMode())
		{
		case Post::SELECT_NODES: doc->SetItemMode(ITEM_NODE); model->SelectNodesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_EDGES: doc->SetItemMode(ITEM_EDGE); model->SelectEdgesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_FACES: doc->SetItemMode(ITEM_FACE); model->SelectFacesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_ELEMS: doc->SetItemMode(ITEM_ELEM); model->SelectElemsInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		}

		model->UpdateSelectionLists();
		postDoc->UpdateFEModel();
		ReportSelection();
		UpdateGLControlBar();
		RedrawGL();
	}
}

void CMainWindow::on_actionToggleVisible_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.ToggleVisibleElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CDocument* doc = GetDocument();

		int nsel = doc->GetSelectionMode();
		int nitem = doc->GetItemMode();

		CCommand* cmd = 0;
		if (nitem == ITEM_MESH)
		{
			switch (nsel)
			{
			case SELECT_OBJECT: cmd = new CCmdToggleObjectVisibility; break;
			case SELECT_PART: cmd = new CCmdTogglePartVisibility; break;
			case SELECT_DISCRETE: cmd = new CCmdToggleDiscreteVisibility; break;
			}
		}
		else
		{
			GObject* po = doc->GetActiveObject();
			if (po == 0) return;

			FEMesh* pm = po->GetFEMesh();
			FEMeshBase* pmb = po->GetEditableMesh();

			switch (nitem)
			{
			case ITEM_ELEM: if (pm) cmd = new CCmdToggleElementVisibility(); break;
			case ITEM_FACE: if (pmb) cmd = new CCmdToggleFEFaceVisibility(); break;
			}
		}

		if (cmd)
		{
			doc->DoCommand(cmd);
			UpdateModel();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionNameSelection_triggered()
{
	static int nparts = 1;
	static int nsurfs = 1;
	static int nedges = 1;
	static int nnodes = 1;

	char szname[256] = {0};

	FEModel* pfem = m_doc->GetFEModel();

	// make sure there is a selection
	FESelection* psel = m_doc->GetCurrentSelection();
	if (psel->Size() == 0) return;

	// set the name
	int item = m_doc->GetItemMode();
	switch (item)
	{
	case ITEM_ELEM: sprintf(szname, "Part%02d"   , nparts); break;
	case ITEM_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
	case ITEM_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
	case ITEM_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
	case ITEM_MESH:
		{
			int nsel = m_doc->GetSelectionMode();
			switch (nsel)
			{
			case SELECT_PART: sprintf(szname, "Part%02d", nparts); break;
			case SELECT_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
			case SELECT_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
			case SELECT_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
			default:
				return;
			}
		}
		break;
	}

	bool ok;
	QString text = QInputDialog::getText(this, "Name Selection", "Name:", QLineEdit::Normal, szname, &ok);

	if (ok && !text.isEmpty())
	{
		string sname = text.toStdString();
		const char* szname = sname.c_str();

		GObject* po = m_doc->GetActiveObject();

		// create a new group
		switch(item)
		{
		case ITEM_ELEM: 
			{
				assert(po);
				FEElementSelection* pes = dynamic_cast<FEElementSelection*>(psel); assert(pes);
				FEPart* pg = dynamic_cast<FEPart*>(pes->CreateItemList()); 
				pg->SetName(szname); 
				m_doc->DoCommand(new CCmdAddPart(po, pg));
				++nparts;
				UpdateModel(pg);
			}
			break;
		case ITEM_FACE: 
			{
				assert(po);
				FEFaceSelection* pfs = dynamic_cast<FEFaceSelection*>(psel);
				FESurface* pg = dynamic_cast<FESurface*>(pfs->CreateItemList());
				pg->SetName(szname); 
				m_doc->DoCommand(new CCmdAddSurface(po, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			break;
		case ITEM_EDGE: 
			{
				assert(po);
				FEEdgeSelection* pes = dynamic_cast<FEEdgeSelection*>(psel);
				FEEdgeSet* pg = dynamic_cast<FEEdgeSet*>(pes->CreateItemList());
				pg->SetName(szname); 
				m_doc->DoCommand(new CCmdAddFEEdgeSet(po, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			break;
		case ITEM_NODE: 
			{
				assert(po);
				FENodeSelection* pns = dynamic_cast<FENodeSelection*>(psel);
				FENodeSet* pg = dynamic_cast<FENodeSet*>(pns->CreateItemList());
				pg->SetName(szname); 
				m_doc->DoCommand(new CCmdAddNodeSet(po, pg));
				++nnodes;
				UpdateModel(pg);
			}
			break;
		case ITEM_MESH:
			{
				int nsel = m_doc->GetSelectionMode();
				switch (nsel)
				{
				case SELECT_PART:
					{
						GPartList* pg = new GPartList(pfem, dynamic_cast<GPartSelection*>(psel));
						pg->SetName(szname);
						m_doc->DoCommand(new CCmdAddGPartGroup(pg));
						++nparts;
						UpdateModel(pg);
					}
					break;
				case SELECT_FACE:
					{
						GFaceList* pg = new GFaceList(pfem, dynamic_cast<GFaceSelection*>(psel));
						pg->SetName(szname);
						m_doc->DoCommand(new CCmdAddGFaceGroup(pg));
						++nsurfs;
						UpdateModel(pg);
					}
					break;
				case SELECT_EDGE:
					{
						GEdgeList* pg = new GEdgeList(pfem, dynamic_cast<GEdgeSelection*>(psel));
						pg->SetName(szname);
						m_doc->DoCommand(new CCmdAddGEdgeGroup(pg));
						++nedges;
						UpdateModel(pg);
					}
					break;
				case SELECT_NODE:
					{
						GNodeList* pg = new GNodeList(pfem, dynamic_cast<GNodeSelection*>(psel));
						pg->SetName(szname);
						m_doc->DoCommand(new CCmdAddGNodeGroup(pg));
						++nnodes;
						UpdateModel(pg);
					}
					break;
				}
			}
		}
	}
}

void CMainWindow::on_actionTransform_triggered()
{
	FESelection* ps = m_doc->GetCurrentSelection();

	if (ps && ps->Size())
	{
		vec3d pos = ps->GetPivot();
		quatd rot = ps->GetOrientation();
		vec3d r = rot.GetVector()*(180 * rot.GetAngle() / PI);
		vec3d scl = ps->GetScale();

		CDlgTransform dlg(this);
		dlg.m_pos = pos;
		dlg.m_relPos = vec3d(0,0,0);

		dlg.m_rot = r;
		dlg.m_relRot = vec3d(0,0,0);

		dlg.m_scl = scl;
		dlg.m_relScl = vec3d(1,1,1);

		dlg.Init();

		if (dlg.exec())
		{
			CCmdGroup* pcmd = new CCmdGroup("Transform");

			// translation
			vec3d dr = dlg.m_pos - pos + dlg.m_relPos;
			pcmd->AddCommand(new CCmdTranslateSelection(dr));

			// rotation
			vec3d r = dlg.m_rot;
			double w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot.Inverse();

			r = dlg.m_relRot;
			w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot;

			pcmd->AddCommand(new CCmdRotateSelection(rot, ui->glview->GetPivotPosition()));

			// scale
			vec3d s1(1, 0, 0);
			vec3d s2(0, 1, 0);
			vec3d s3(0, 0, 1);

			r = ui->glview->GetPivotPosition();

			pcmd->AddCommand(new CCmdScaleSelection(dlg.m_scl.x*dlg.m_relScl.x / scl.x, s1, r));
			pcmd->AddCommand(new CCmdScaleSelection(dlg.m_scl.y*dlg.m_relScl.y / scl.y, s2, r));
			pcmd->AddCommand(new CCmdScaleSelection(dlg.m_scl.z*dlg.m_relScl.z / scl.z, s3, r));

			m_doc->DoCommand(pcmd);
			UpdateGLControlBar();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionCollapseTransform_triggered()
{
	GObject* po = m_doc->GetActiveObject();
	if (po == 0) QMessageBox::critical(this, "FEBio Studio", "Please select an object");
	else
	{
		po->CollapseTransform();
		UpdateModel(po);
		RedrawGL();
	}
}

void CMainWindow::on_actionClone_triggered()
{
	// get the active object
	GObject* po = m_doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneObject dlg(this);
	if (dlg.exec())
	{
		// get the model
		GModel& m = *m_doc->GetGModel();

		// clone the object
		GObject* pco = m.CloneObject(po);
		if (pco == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Could not clone this object.");
			return;
		}

		// set the name
		QString name = dlg.GetNewObjectName();
		std::string sname = name.toStdString();
		pco->SetName(sname);

		// add and select the new object
		m_doc->DoCommand(new CCmdAddAndSelectObject(pco));

		// update windows
		Update(0, true);
	}
}

void CMainWindow::on_actionCloneGrid_triggered()
{
	// get the active object
	GObject* po = m_doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneGrid dlg(this);
	if (dlg.exec())
	{
		GModel& m = *m_doc->GetGModel();

		// clone the object
		vector<GObject*> newObjects = m.CloneGrid(po, dlg.m_rangeX[0], dlg.m_rangeX[1], dlg.m_rangeY[0], dlg.m_rangeY[1], dlg.m_rangeZ[0], dlg.m_rangeZ[1], dlg.m_inc[0], dlg.m_inc[1], dlg.m_inc[2]);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to grid clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone grid");
		for (int i=0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(newObjects[i]));
		}
		m_doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionCloneRevolve_triggered()
{
	// get the active object
	GObject* po = m_doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneRevolve dlg(this);
	if (dlg.exec())
	{
		GModel& m = *m_doc->GetGModel();

		vector<GObject*> newObjects = m.CloneRevolve(po, dlg.m_count, dlg.m_range, dlg.m_spiral, dlg.m_center, dlg.m_axis, dlg.m_rotateClones);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to revolve clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone revolve");
		for (int i = 0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(newObjects[i]));
		}
		m_doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionMerge_triggered()
{
	CDlgMergeObjects dlg(this);
	if (dlg.exec() == QDialog::Rejected) return;

	CDocument* pdoc = GetDocument();

	// make sure we have an object selection
	FESelection* currentSelection = pdoc->GetCurrentSelection();
	if (currentSelection->Type() != SELECT_OBJECTS)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}
	GObjectSelection* sel = dynamic_cast<GObjectSelection*>(currentSelection);

	// merge the objects
	GModel& m = *pdoc->GetGModel();
	GObject* newObject = m.MergeSelectedObjects(sel, dlg.m_name, dlg.m_weld, dlg.m_tol);
	if (newObject == 0)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}

	// we need to delete the selected objects and add the new object
	// create the command that will do the attaching
	CCmdGroup* pcmd = new CCmdGroup("Attach");
	for (int i = 0; i<sel->Count(); ++i)
	{
		// remove the old object
		GObject* po = sel->Object(i);
		pcmd->AddCommand(new CCmdDeleteGObject(&m, po));
	}
	// add the new object
	pcmd->AddCommand(new CCmdAddAndSelectObject(newObject));

	// perform the operation
	pdoc->DoCommand(pcmd);

	// update UI
	Update(0, true);
}

void CMainWindow::on_actionDetach_triggered()
{
	CDocument* doc = GetDocument();
	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_ELEMENTS))
	{ 
		QMessageBox::warning(this, "Detach Selection", "Cannot detach this selection");
		return;
	}

	CDlgDetachSelection dlg(this);
	if (dlg.exec())
	{
		GMeshObject* po = dynamic_cast<GMeshObject*>(doc->GetActiveObject()); assert(po);
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = po->DetachSelection();

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(newObject));

		UpdateModel(newObject, true);
	}
}


void CMainWindow::on_actionExtract_triggered()
{
	CDocument* doc = GetDocument();
	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_FACES))
	{
		QMessageBox::warning(this, "Extract Selection", "Cannot extract this selection");
		return;
	}

	CDlgExtractSelection dlg(this);
	if (dlg.exec())
	{
		GObject* po = doc->GetActiveObject();
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = ExtractSelection(po);

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(newObject));

		UpdateModel(newObject, true);
	}
}

void CMainWindow::on_actionPurge_triggered()
{
	CDlgPurge dlg(this);
	if (dlg.exec())
	{
		FEModel* ps = m_doc->GetFEModel();
		ps->Purge(dlg.getOption());
		m_doc->ClearCommandStack();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionEditProject_triggered()
{
	CDlgEditProject dlg(GetDocument()->GetProject(), this);
	dlg.exec();
	UpdatePhysicsUi();
}

void CMainWindow::on_actionFaceToElem_triggered()
{
	CDocument* doc = GetDocument();
	FESelection* sel = doc->GetCurrentSelection();
	if (sel->Type() == SELECT_FE_FACES)
	{
		FEFaceSelection* selectedFaces = dynamic_cast<FEFaceSelection*>(sel);
		FEMesh* mesh = dynamic_cast<FEMesh*>(selectedFaces->GetMesh());
		if (mesh)
		{
			vector<int> selectedElems = mesh->GetElementsFromSelectedFaces();
			if (selectedElems.empty() == false)
			{
				CCmdSelectElements* cmd = new CCmdSelectElements(mesh, selectedElems, false);
				doc->DoCommand(cmd);
			}
		}
	}
}

void CMainWindow::on_actionUndoViewChange_triggered()
{
	ui->glview->UndoViewChange();
}

void CMainWindow::on_actionRedoViewChange_triggered()
{
	ui->glview->RedoViewChange();
}

void CMainWindow::on_actionZoomSelect_triggered()
{
	ui->glview->ZoomSelection();
}

void CMainWindow::on_actionZoomExtents_triggered()
{
	ui->glview->ZoomExtents();
}

void CMainWindow::on_actionViewCapture_toggled(bool bchecked)
{
	ui->glview->showSafeFrame(bchecked);
	RedrawGL();
}

void CMainWindow::on_actionOrtho_toggled(bool b)
{
	ui->glview->TogglePerspective(b);
}

void CMainWindow::on_actionShowGrid_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bgrid = b;
	RedrawGL();
}

void CMainWindow::on_actionShowMeshLines_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bmesh = b;
	Update(this);
}

void CMainWindow::on_actionShowEdgeLines_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bfeat = b;
	Update(this);
}

void CMainWindow::on_actionBackfaceCulling_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bcull = b;
	Update(this);
}

void CMainWindow::on_actionViewSmooth_toggled(bool bchecked)
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	Post::CGLModel* po = doc->GetGLModel();
	if (po)
	{
		Post::CGLColorMap* pcm = po->GetColorMap();
		if (pcm)
		{
			pcm->SetColorSmooth(bchecked);
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionShowNormals_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bnorm = b;
	RedrawGL();
}

void CMainWindow::on_actionShowFibers_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bfiber = b;
	RedrawGL();
}

void CMainWindow::on_actionShowMatAxes_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_blma = b;
	RedrawGL();
}

void CMainWindow::on_actionShowDiscrete_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_showDiscrete = b;
	RedrawGL();
}

void CMainWindow::on_actionToggleLight_triggered()
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_bLighting = !view.m_bLighting;
	RedrawGL();
}

void CMainWindow::changeViewMode(View_Mode vm)
{
	ui->glview->SetViewMode(vm);

	// switch to ortho view if we're not in it
	bool bortho = ui->glview->OrhographicProjection();
	if (bortho == false)
	{
		ui->actionOrtho->trigger();
	}
}

void CMainWindow::on_actionFront_triggered()
{
	changeViewMode(VIEW_FRONT);
}

void CMainWindow::on_actionBack_triggered()
{
	changeViewMode(VIEW_BACK);
}

void CMainWindow::on_actionLeft_triggered()
{
	changeViewMode(VIEW_LEFT);
}

void CMainWindow::on_actionRight_triggered()
{
	changeViewMode(VIEW_RIGHT);
}

void CMainWindow::on_actionTop_triggered()
{
	changeViewMode(VIEW_TOP);
}

void CMainWindow::on_actionBottom_triggered()
{
	changeViewMode(VIEW_BOTTOM);
}

void CMainWindow::on_actionWireframe_toggled(bool b)
{
	VIEW_SETTINGS& view = m_doc->GetViewSettings();
	view.m_nrender = (b ? RENDER_WIREFRAME : RENDER_SOLID);
	RedrawGL();
}

void CMainWindow::on_actionSnap3D_triggered()
{
	vec3d r = GetGLView()->GetPivotPosition();
	GetDocument()->Set3DCursor(r);
	RedrawGL();
}

void CMainWindow::on_actionTrack_toggled(bool b)
{
	ui->glview->TrackSelection(b);
	RedrawGL();
}

void CMainWindow::on_actionViewVPSave_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	CGLCamera& cam = ui->glview->GetCamera();
	GLCameraTransform t;
	cam.GetTransform(t);

	Post::CGView& view = *doc->GetView();
	static int n = 0; n++;
	char szname[64] = { 0 };
	sprintf(szname, "key%02d", n);
	t.SetName(szname);
	view.AddCameraKey(t);
	ui->postPanel->Update();
}

void CMainWindow::on_actionViewVPPrev_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	Post::CGView& view = *doc->GetView();
	if (view.CameraKeys() > 0)
	{
		view.PrevKey();
		ui->glview->GetCamera().SetTransform(view.GetCurrentKey());
		RedrawGL();
	}
}

void CMainWindow::on_actionViewVPNext_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	Post::CGView& view = *doc->GetView();
	if (view.CameraKeys() > 0)
	{
		view.NextKey();
		ui->glview->GetCamera().SetTransform(view.GetCurrentKey());
		RedrawGL();
	}
}

// sync the views of all documents to the currently active one
void CMainWindow::on_actionSyncViews_triggered()
{
	CPostDoc* doc = GetActiveDocument();
	if (doc == nullptr) return;

	Post::CGView& view = *doc->GetView();
	CGLCamera& cam = view.GetCamera();
	GLCameraTransform transform;
	cam.GetTransform(transform);
	int views = ui->tab->views();
	for (int i = 1; i < views; ++i)
	{
		CPostDoc* doci = ui->tab->getPostDoc(i);
		if (doci != doc)
		{
			CGLCamera& cami = doci->GetView()->GetCamera();

			// copy the transforms
			cami.SetTransform(transform);
			cami.Update(true);
		}
	}
}

void CMainWindow::on_actionOnlineHelp_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/support/"));
}

void CMainWindow::on_actionFEBioURL_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/"));
}

void CMainWindow::on_actionFEBioForum_triggered()
{
	QDesktopServices::openUrl(QUrl("https://forums.febio.org/"));
}

void CMainWindow::on_actionFEBioResources_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/teaching-hub/"));
}

void CMainWindow::on_actionFEBioPubs_triggered()
{
	QDesktopServices::openUrl(QUrl("https://febio.org/publications/"));
}

void CMainWindow::on_actionAbout_triggered()
{
	QString txt = QString(\
		"<h1>FEBio Studio</h1>\
		<p><b>Version %1.%2.%3</b></p>\
		<p>Musculoskeletal Research Laboratories, University of Utah</p>\
		<p>Musculoskeletal Biomechanics Laboratory, Columbia University</p>\
		<p>Copyright (c) 2019 - 2020, All rights reserved</p>\
		<hr>\
		<p>When using FEBio or FEBioStudio in your publications, please cite:</p>\
		<p><b>Maas SA, Ellis BJ, Ateshian GA, Weiss JA: FEBio: Finite Elements for Biomechanics. Journal of Biomechanical Engineering, 134(1):011005, 2012</b></p>"\
		).arg(VERSION).arg(SUBVERSION).arg(SUBSUBVERSION);

	QMessageBox about(this);
	about.setWindowTitle("About FEBio Studio");
	about.setText(txt);
	about.setIconPixmap(QPixmap(":/icons/FEBioStudio_large.png"));
	about.exec();
}

void CMainWindow::on_actionSelect_toggled(bool b)
{
	m_doc->SetTransformMode(TRANSFORM_NONE); 
	RedrawGL();
}

void CMainWindow::on_actionTranslate_toggled(bool b)
{
	m_doc->SetTransformMode(TRANSFORM_MOVE);
	RedrawGL();
}

void CMainWindow::on_actionRotate_toggled(bool b)
{
	m_doc->SetTransformMode(TRANSFORM_ROTATE);
	RedrawGL();
}

void CMainWindow::on_actionScale_toggled(bool b)
{
	m_doc->SetTransformMode(TRANSFORM_SCALE);
	RedrawGL();
}

void CMainWindow::on_selectCoord_currentIndexChanged(int n)
{
	switch (n)
	{
	case 0: ui->glview->SetCoordinateSystem(COORD_GLOBAL); break;
	case 1: ui->glview->SetCoordinateSystem(COORD_LOCAL); break;
	case 2: ui->glview->SetCoordinateSystem(COORD_SCREEN); break;
	}
	Update();
}

QStringList CMainWindow::GetRecentFileList()
{
	return ui->m_recentFiles;
}
