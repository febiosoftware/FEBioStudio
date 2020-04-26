#include "stdafx.h"
#include "DlgPlotMix.h"
#include "MainWindow.h"
#include <QPushButton>
#include <QListWidget>
#include <QBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <PostLib/FEPlotMix.h>
#include "Document.h"
#include "CIntInput.h"
#include <PostLib/FEKinemat.h>
#include <PostLib/FELSDYNAimport.h>
#include "PostDoc.h"
#include "MainWindow.h"

class CDlgPlotMixUI
{
public:
	CMainWindow*	m_wnd;
	QListWidget* list;

public:
	void setup(QDialog* parent)
	{
		QVBoxLayout* pv = new QVBoxLayout;
		QHBoxLayout* ph = new QHBoxLayout;

		QPushButton* browse = new QPushButton("Add file ...");
		QPushButton* remove = new QPushButton("Remove");
		QPushButton* moveUp = new QPushButton("Move Up");
		QPushButton* moveDown = new QPushButton("Move Down");
		ph->addWidget(browse);
		ph->addWidget(remove);
		ph->addWidget(moveUp);
		ph->addWidget(moveDown);
		pv->addLayout(ph);

		list = new QListWidget;
		pv->addWidget(list);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		pv->addWidget(bb);

		parent->setLayout(pv);

		QObject::connect(browse, SIGNAL(clicked()), parent, SLOT(OnBrowse()));
		QObject::connect(remove, SIGNAL(clicked()), parent, SLOT(OnRemove()));
		QObject::connect(moveUp, SIGNAL(clicked()), parent, SLOT(OnMoveUp()));
		QObject::connect(moveDown, SIGNAL(clicked()), parent, SLOT(OnMoveDown()));
		QObject::connect(bb, SIGNAL(accepted()), parent, SLOT(OnApply()));
		QObject::connect(bb, SIGNAL(rejected()), parent, SLOT(reject()));
	}
};

CDlgPlotMix::CDlgPlotMix(CMainWindow* wnd) : QDialog(wnd), ui(new CDlgPlotMixUI)
{
	setWindowTitle("Plot Mix");
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CDlgPlotMix::OnBrowse()
{
	QStringList filenames = QFileDialog::getOpenFileNames(0, "Open file", 0, "XPLT files(*.xplt)");
	if (filenames.isEmpty() == false)
	{
		for (int i = 0; i<filenames.count(); ++i)
			ui->list->addItem(filenames[i]);
	}
}

void CDlgPlotMix::OnRemove()
{
	qDeleteAll(ui->list->selectedItems());
}

void CDlgPlotMix::OnMoveUp()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->insertItem(0, pi);
	}
}

void CDlgPlotMix::OnMoveDown()
{
	QList<QListWidgetItem*> items = ui->list->selectedItems();
	QList<QListWidgetItem*>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		QListWidgetItem* pi = ui->list->takeItem(ui->list->row(*it));
		ui->list->addItem(pi);
	}
}

void CDlgPlotMix::OnApply()
{
	Post::FEPlotMix reader;

	int nitems = ui->list->count();
	vector<string> str(nitems);
	for (int i = 0; i<nitems; ++i)
	{
		QListWidgetItem* pi = ui->list->item(i);
		QString s = pi->text();
		str[i] = s.toStdString();
	}

	vector<const char*> sz(nitems, 0);
	for (int i = 0; i<nitems; ++i) sz[i] = str[i].c_str();


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

	accept();
}
