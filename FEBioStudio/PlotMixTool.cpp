#include "stdafx.h"
#include "PlotMixTool.h"
#include "MainWindow.h"
#include <QPushButton>
#include <QListWidget>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <PostLib/FEPlotMix.h>
#include "Document.h"
#include "CIntInput.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include "PostDoc.h"
#include "MainWindow.h"
using namespace Post;

class CPlotMixToolUI : public QWidget
{
public:
	QListWidget* list;
public:
	CPlotMixToolUI(QObject* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QHBoxLayout* ph = new QHBoxLayout;
		
		QPushButton* browse   = new QPushButton("Add file ...");
		QPushButton* remove   = new QPushButton("Remove");
		QPushButton* moveUp   = new QPushButton("Move Up");
		QPushButton* moveDown = new QPushButton("Move Down");
		ph->addWidget(browse);
		ph->addWidget(remove);
		ph->addWidget(moveUp);
		ph->addWidget(moveDown);
		pv->addLayout(ph);

		list = new QListWidget;
		pv->addWidget(list);

		QPushButton* apply = new QPushButton("Load");
		pv->addWidget(apply);
		pv->addStretch();

		setLayout(pv);

		QObject::connect(browse  , SIGNAL(clicked()), parent, SLOT(OnBrowse()));
		QObject::connect(remove  , SIGNAL(clicked()), parent, SLOT(OnRemove()));
		QObject::connect(moveUp  , SIGNAL(clicked()), parent, SLOT(OnMoveUp()));
		QObject::connect(moveDown, SIGNAL(clicked()), parent, SLOT(OnMoveDown()));
		QObject::connect(apply   , SIGNAL(clicked()), parent, SLOT(OnApply()));
	}
};

CPlotMixTool::CPlotMixTool(CMainWindow* wnd) : CAbstractTool("Plot Mix")
{
	ui = 0;
}

// get the property list
QWidget* CPlotMixTool::createUi()
{
	ui = new CPlotMixToolUI(this);
	return ui;
}

void CPlotMixTool::OnBrowse()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, "Open file", 0, "XPLT files(*.xplt)");
	if (filenames.isEmpty() == false)
	{
		for (int i=0; i<filenames.count(); ++i)
			ui->list->addItem(filenames[i]);
	}
}

void CPlotMixTool::OnRemove()
{
	qDeleteAll(ui->list->selectedItems());
}

void CPlotMixTool::OnMoveUp()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->insertItem(0, pi);
	}
}

void CPlotMixTool::OnMoveDown()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->addItem(pi);
	}
}

void CPlotMixTool::OnApply()
{
	FEPlotMix reader;

	int nitems = ui->list->count();
	vector<string> str(nitems);
	for (int i=0; i<nitems; ++i)
	{
		QListWidgetItem* pi = ui->list->item(i);
		QString s = pi->text();
		str[i] = s.toStdString();
	}

	vector<const char*> sz(nitems, 0);
	for (int i=0; i<nitems; ++i) sz[i] = str[i].c_str();

	// Create a new document
/*	CDocument* doc = m_wnd->NewDocument("plotmix");

	FEModel* pnew = reader.Load(&sz[0], nitems);
	if (pnew == 0) QMessageBox::critical(0, "Plot Mix Tool", "An error occured reading the plot files.");
	else
	{
		doc->SetFEModel(pnew);
	}
	ui->list->clear();
*/
	updateUi();
}

//=======================================================================================

class CKinematToolUI : public QWidget
{
public:
	QLineEdit* modelFile;
	QLineEdit* kineFile;
	CIntInput *start, *end, *stride;

public:
	CKinematToolUI(QObject* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QLabel* pl = new QLabel("Model file:");
		modelFile = new QLineEdit; pl->setBuddy(modelFile);
		QPushButton* browse1 = new QPushButton("..."); browse1->setFixedWidth(30);
		QHBoxLayout* ph = new QHBoxLayout;
		ph->addWidget(modelFile); 
		ph->addWidget(browse1);
		pv->addWidget(pl);
		pv->addLayout(ph);

		pl = new QLabel("Kine file:");
		kineFile = new QLineEdit; pl->setBuddy(kineFile);
		QPushButton* browse2 = new QPushButton("..."); browse2->setFixedWidth(30);
		ph = new QHBoxLayout;
		ph->addWidget(kineFile);
		ph->addWidget(browse2);
		pv->addWidget(pl);
		pv->addLayout(ph);

		ph = new QHBoxLayout;
		ph->addWidget(pl = new QLabel("From:"));
		ph->addWidget(start = new CIntInput); pl->setBuddy(start); start->setFixedWidth(70); start->setValue(1);
		ph->addWidget(pl = new QLabel("To:"));
		ph->addWidget(end = new CIntInput); pl->setBuddy(end); end->setFixedWidth(70); end->setValue(999);
		ph->addWidget(pl = new QLabel("Stride:"));
		ph->addWidget(stride = new CIntInput); pl->setBuddy(stride); stride->setFixedWidth(70); stride->setValue(1);
		pv->addLayout(ph);

		QPushButton* apply = new QPushButton("Apply");
		pv->addWidget(apply);

		pv->addStretch();
		setLayout(pv);

		QObject::connect(browse1, SIGNAL(clicked()), parent, SLOT(OnBrowse1()));
		QObject::connect(browse2, SIGNAL(clicked()), parent, SLOT(OnBrowse2()));
		QObject::connect(apply  , SIGNAL(clicked()), parent, SLOT(OnApply()));
	}
};

CKinematTool::CKinematTool() : CAbstractTool("Kinemat")
{
	ui = 0;
}

// get the property list
QWidget* CKinematTool::createUi()
{
	ui = new CKinematToolUI(this);
	return ui;
}

void CKinematTool::OnBrowse1()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "LSDYNA Keyword (*.k)");
	if (filename.isEmpty() == false)
	{
		ui->modelFile->setText(filename);
	}
}

void CKinematTool::OnBrowse2()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->kineFile->setText(filename);
	}
}

void CKinematTool::OnApply()
{
	int n0 = ui->start->value();
	int n1 = ui->end->value();
	int ni = ui->stride->value();

	// create a new job
	CMainWindow* wnd = GetMainWindow();
	CDocument* doc = wnd->GetDocument();
	CFEBioJob* job = new CFEBioJob(wnd->GetDocument());
	job->SetName("Kinemat");
	doc->AddFEbioJob(job);

	FEKinemat kine;
	kine.SetRange(n0, n1, ni);

	string modelFile = ui->modelFile->text().toStdString();
	string kineFile = ui->kineFile->text().toStdString();

	// load the file
	FELSDYNAimport* preader = new FELSDYNAimport;
	preader->read_displacements(true);
	if (job->LoadFEModel(preader, modelFile.c_str()) == false)
	{
		QMessageBox::critical(wnd, "PostView2", "Failed to load model file");
		return;
	}

	CPostDoc* postDoc = job->GetPostDoc();
	if (kine.Apply(postDoc->GetGLModel(), kineFile.c_str()) == false)
	{
		QMessageBox::critical(0, "Kinemat", "Failed applying Kinemat tool");
	}

	wnd->UpdateModel();
	wnd->Update();
	wnd->SetActivePostDoc(postDoc);
}
