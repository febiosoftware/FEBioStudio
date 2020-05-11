#include "stdafx.h"
#include "DlgNewProject.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QCheckBox>
#include <QMessageBox>
#include "MainWindow.h"

class CDlgNewProjectUI
{
public:
	CMainWindow*	m_wnd;
	QLineEdit*		name;
	QLineEdit*		folder;
	QToolButton*	selectFolder;
	QCheckBox*		createSubFolder;

public:
	void setup(QDialog* dlg)
	{
		QFormLayout* f = new QFormLayout;
		f->addRow("Project name:", name = new QLineEdit);
		name->setText("MyProject");

		QHBoxLayout* h = new QHBoxLayout;
		h->setMargin(0);
		h->addWidget(folder = new QLineEdit);
		h->addWidget(selectFolder = new QToolButton);
		f->addRow("Project folder:", h);
		f->addRow("", createSubFolder = new QCheckBox("Create project subfolder"));
		createSubFolder->setChecked(true);

		selectFolder->setIcon(QIcon(":/icons/folder.png"));
		selectFolder->setToolTip("Select the folder where the project will be created.");

		createSubFolder->setToolTip("Check this to create a subfolder in the project folder.\nThe subfolder will have the project's name.");

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(f);
		l->addStretch();
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(selectFolder, SIGNAL(clicked(bool)), dlg, SLOT(OnSelectFolder()));
	}
};

CDlgNewProject::CDlgNewProject(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgNewProjectUI)
{
	setWindowTitle("New Project");
	ui->m_wnd = wnd;
	ui->setup(this);
	setMinimumSize(QSize(600, 300));
}

void CDlgNewProject::OnSelectFolder()
{
	QString folderName = QFileDialog::getExistingDirectory(this, "Select Folder");
	ui->folder->setText(QDir::toNativeSeparators(folderName));
}

void CDlgNewProject::SetProjectFolder(const QString& folder)
{
	ui->folder->setText(QDir::toNativeSeparators(folder));
}

QString CDlgNewProject::GetProjectFolder()
{
	return QDir::toNativeSeparators(ui->folder->text());
}

void CDlgNewProject::accept()
{
	QString projectName = ui->name->text();
	QString projectFolder = ui->folder->text();
	bool createSubFolder = ui->createSubFolder->isChecked();

	// check the project name
	if (projectName.isEmpty())
	{
		QMessageBox::critical(this, "New Project", "You must provide a project name.");
		return;
	}

	// check the project folder
	if (projectFolder.isEmpty())
	{
		QMessageBox::critical(this, "New Project", "You must provide a project folder.");
		return;
	}

	// see if the project folder exists
	QDir dir(projectFolder);
	if (dir.exists() == false)
	{
		QMessageBox::critical(this, "New Project", "The project folder location does not appear to exsit.\nPlease select a different location.");
		return;
	}

	// create the project sub-folder, if needed
	if (createSubFolder)
	{
		if (dir.mkdir(projectName) == false)
		{
			QMessageBox::critical(this, "New Project", "The project subfolder could not be created. Possibly, because it already exists.\nPlease select a different project name or folder.");
			return;
		}
		else
		{
			dir.cd(projectName);
		}
	}

	// create project file name
	QString fileBase = projectName + ".fsp";
	QString fileName = QDir::toNativeSeparators(dir.absoluteFilePath(fileBase));

	// see if this file already exists
	QFile file(fileName);
	if (file.exists())
	{
		if (QMessageBox::question(this, "New Project", QString("The project file already exists:%1\nAre you sure you want to overwrite this file?").arg(fileName)) != QMessageBox::Yes)
		{
			return;
		}
	}

	// create the project
	if (ui->m_wnd->CreateNewProject(fileName) == false)
	{
		QMessageBox::critical(this, "New Project", "Could not create the project file.\nTry a different project name or folder.");
		return;
	}

	// all done
	QDialog::accept();
}
