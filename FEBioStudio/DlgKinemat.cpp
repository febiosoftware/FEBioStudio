#include "stdafx.h"
#include "DlgKinemat.h"
#include "CIntInput.h"
#include <QListWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "MainWindow.h"
#include "Document.h"
#include "PostDocument.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include "PostDocument.h"

class CDlgKinematUI
{
public:
	CMainWindow*	m_wnd;

	QLineEdit* modelFile;
	QLineEdit* kineFile;
	CIntInput *start, *end, *stride;

public:
	void setup(QDialog* dlg)
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

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);
		dlg->setLayout(pv);

		QObject::connect(browse1, SIGNAL(clicked()), dlg, SLOT(OnBrowse1()));
		QObject::connect(browse2, SIGNAL(clicked()), dlg, SLOT(OnBrowse2()));
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgKinemat::CDlgKinemat(CMainWindow* parent) : QDialog(parent), ui(new CDlgKinematUI)
{
	setWindowTitle("Kinemat");
	ui->m_wnd = parent;
	ui->setup(this);
}

void CDlgKinemat::OnBrowse1()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "LSDYNA Keyword (*.k)");
	if (filename.isEmpty() == false)
	{
		ui->modelFile->setText(filename);
	}
}

void CDlgKinemat::OnBrowse2()
{
	QString filename = QFileDialog::getOpenFileName(0, "Open file", 0, "All files(*)");
	if (filename.isEmpty() == false)
	{
		ui->kineFile->setText(filename);
	}
}

void CDlgKinemat::OnApply()
{
	int n0 = ui->start->value();
	int n1 = ui->end->value();
	int ni = ui->stride->value();

	// create a new document
	CMainWindow* wnd = ui->m_wnd;
	CPostDocument* doc = new CPostDocument(wnd);

	FEKinemat kine;
	kine.SetRange(n0, n1, ni);

	string modelFile = ui->modelFile->text().toStdString();
	string kineFile = ui->kineFile->text().toStdString();

	// load the file
	Post::FELSDYNAimport* preader = new Post::FELSDYNAimport(nullptr);
	preader->read_displacements(true);
//	if (wnd->LoadFEModel(preader, modelFile.c_str()) == false)
//	{
		//QMessageBox::critical(wnd, "PostView2", "Failed to load model file");
		//return;
	//}

	if (kine.Apply(doc->GetGLModel(), kineFile.c_str()) == false)
	{
		QMessageBox::critical(0, "Kinemat", "Failed applying Kinemat tool");
	}

	// create new post document
	CPostDocument* postDocument = new CPostDocument(wnd);

	wnd->UpdateModel();
	wnd->Update();
	wnd->SetActiveDocument(postDocument);

	accept();
}
