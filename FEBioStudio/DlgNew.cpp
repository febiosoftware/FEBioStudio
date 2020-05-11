#include "stdafx.h"
#include "DlgNew.h"
#include <QListWidget>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QFormLayout>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QGuiApplication>
#include <QPushButton>
#include <QScreen>
#include <QDesktopWidget>
#include "DocTemplate.h"
#include "MainWindow.h"
#include "ModelDocument.h"

class Ui::CDlgNew
{
public:
	::CMainWindow*	m_wnd;

	QListWidget*	m_list;
	QLineEdit*		m_modelName;
	QLineEdit*		m_modelFolder;
	QCheckBox*		m_createSubFolder;
	QCheckBox*		m_showDialog;

public:
	void setup(::CMainWindow* wnd, QDialog* dlg)
	{
		m_wnd = wnd;

		m_list = new QListWidget;
		QStackedWidget* s = new QStackedWidget;

		int ntemp = TemplateManager::Templates();
		for (int i = 0; i<ntemp; ++i)
		{
			const DocTemplate& doc = TemplateManager::GetTemplate(i);
			QLabel* label = new QLabel;
			label->setWordWrap(true);
			label->setText(QString("<h3>%1</h3><p>%2</p>").arg(doc.title.c_str()).arg(doc.description.c_str()));
			label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			m_list->addItem(doc.title.c_str());
			s->addWidget(label);
		}

		m_list->setCurrentRow(0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(m_list);
		h->addWidget(s);

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);

		QFormLayout* f = new QFormLayout;

		QToolButton* tb = new QToolButton;
		tb->setObjectName("folder");
		tb->setIcon(QIcon(":/icons/folder.png"));

		QHBoxLayout* fh = new QHBoxLayout;
		fh->setMargin(0);
		fh->addWidget(m_modelFolder = new QLineEdit);
		fh->addWidget(tb);

		f->addRow("Model name:", m_modelName = new QLineEdit);
		m_modelName->setText("MyModel");
		f->addRow("Model folder:", fh);
		
		v->addLayout(f);

		m_createSubFolder = new QCheckBox("create model subfolder");
		m_createSubFolder->setChecked(true);
		v->addWidget(m_createSubFolder);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QHBoxLayout* hb = new  QHBoxLayout;
		hb->setMargin(0);
		m_showDialog = new QCheckBox("Don't show this dialog box again");
		hb->addWidget(m_showDialog);
		hb->addWidget(bb);
		v->addLayout(hb);

		dlg->setLayout(v);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(m_list, SIGNAL(currentRowChanged(int)), s, SLOT(setCurrentIndex(int)));
		QObject::connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(tb, SIGNAL(clicked(bool)), dlg, SLOT(OnFolderName()));

		m_list->setWhatsThis("Select the model template. This will adjust the UI to show only relevant features.");
		m_modelName->setWhatsThis("This is the model's name and the base of the model's filename.");
		m_modelFolder->setWhatsThis("The model will be saved in this location");
		m_createSubFolder->setWhatsThis("Check this box to create a subfolder in the model folder where the model will be saved.");
		tb->setWhatsThis("Change the model folder.");
	}
};

CDlgNew::CDlgNew(CMainWindow* parent ) : QDialog(parent), ui(new Ui::CDlgNew)
{
	setWindowTitle("New Model");
	ui->setup(parent, this);
}

void CDlgNew::setShowDialogOption(bool b)
{
	ui->m_showDialog->setChecked(b);
}

bool CDlgNew::showDialogOption()
{
	return ui->m_showDialog->isChecked();
}

void CDlgNew::SetModelFolder(const QString& modelPath)
{
	ui->m_modelFolder->setText(modelPath);
}

QString CDlgNew::GetModelFolder()
{
	return QDir::toNativeSeparators(ui->m_modelFolder->text());
}

QString CDlgNew::GetModelName()
{
	return ui->m_modelName->text();
}

void CDlgNew::showEvent(QShowEvent* ev)
{
	QList<QScreen*> screenList = QGuiApplication::screens();
	QRect screenGeometry = screenList.at(0)->geometry();
	int x = (screenGeometry.width() - width()) / 2;
	int y = (screenGeometry.height() - height()) / 2;
	move(x, y);
}

void CDlgNew::OnFolderName()
{
	QString folderName = QFileDialog::getExistingDirectory(this, "Select Folder");
	if (folderName.isEmpty() == false)
	{
		ui->m_modelFolder->setText(QDir::toNativeSeparators(folderName));
	}
}

void CDlgNew::accept()
{
	int ntemplate = ui->m_list->currentRow();
	QString modelName   = ui->m_modelName->text();
	QString modelFolder = ui->m_modelFolder->text();
	bool createSubFolder = ui->m_createSubFolder->isChecked();

	if (ntemplate < 0)
	{
		QMessageBox::critical(this, "New Model", "Please choose a model template.");
		return;
	}

	// check the model's name
	if (modelName.isEmpty())
	{
		QMessageBox::critical(this, "New Model", "Please enter new name for the model.");
		return;
	}

	// check the folder
	if (modelFolder.isEmpty())
	{
		QMessageBox::critical(this, "New Model", "Please enter a valid folder name.");
		return;
	}

	// see if the folder exists
	QDir dir(modelFolder);
	if (dir.exists() == false)
	{
		QMessageBox::critical(this, "New Model", "The specified folder name does not exist.\nPlease choose a different folder name.");
		return;
	}

	// create the subfolder
	dir.mkdir(modelName);

	// cd into this folder
	if (dir.cd(modelName) == false)
	{
		QMessageBox::critical(this, "New Model", QString("Something went wrong saving the model file. Please select a different model name or model folder."));
		return;
	}

	// compose the model filename
	QString fileName = QDir::toNativeSeparators(dir.absoluteFilePath(modelName + ".fsm"));

	// see if this file already exists
	QFile file(fileName);
	if (file.exists())
	{
		if (QMessageBox::question(this, "New Model", QString("This file already exists:\n%1\nDo you want to overwrite it?").arg(fileName)) != QMessageBox::Yes)
		{ 
			return;
		}
	}

	// create a new model
	CModelDocument * doc = new CModelDocument(ui->m_wnd);
	doc->SetDocFilePath(fileName.toStdString());
	doc->LoadTemplate(ntemplate);
	if (doc->SaveDocument() == false)
	{
		QMessageBox::critical(this, "New Model", QString("Something went wrong saving the model file. Please select a different model name or model folder."));
		delete doc;
		return;
	}

	// all is good, so we are ready to return
	ui->m_wnd->AddDocument(doc);

	QDialog::accept();
}

int CDlgNew::getTemplate()
{
	return ui->m_list->currentRow();
}
