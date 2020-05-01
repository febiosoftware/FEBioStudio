#include "stdafx.h"
#include "FileViewer.h"
#include "MainWindow.h"
#include "DocManager.h"
#include "Document.h"
#include "ModelDocument.h"
#include <QTreeWidget>
#include <QFileSystemModel>
#include <QBoxLayout>
#include <QHeaderView>

class Ui::CFileViewer
{
public:
	::CMainWindow*	m_wnd;
	QTreeWidget*	m_tree;

public:
	void setupUi(QWidget* parent)
	{
		QVBoxLayout* l = new QVBoxLayout(parent);
		l->setMargin(0);

		m_tree = new QTreeWidget;
		m_tree->setColumnCount(1);
		m_tree->header()->hide();
		m_tree->setObjectName("fileList");
		
		l->addWidget(m_tree);

		parent->setLayout(l);
	}
};

CFileViewer::CFileViewer(CMainWindow* pwnd, QWidget* parent) : QWidget(parent), ui(new Ui::CFileViewer)
{
	ui->m_wnd = pwnd;
	// build Ui
	ui->setupUi(this);

	QMetaObject::connectSlotsByName(this);
}

void CFileViewer::on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column)
{
	QVariant v = item->data(0, Qt::UserRole);
	if (v.type() == QVariant::Int)
	{
		ui->m_wnd->SetActiveView(v.toInt());
	}
	else if (v.type() == QVariant::String)
	{
		QString filePath = v.toString();
		CDocument* doc = ui->m_wnd->FindDocument(filePath.toStdString());
		if (doc)
			ui->m_wnd->SetActiveDocument(doc);
		else
			ui->m_wnd->OpenFile(filePath, true);
	}
}

void CFileViewer::Update()
{
	ui->m_tree->clear();

	CDocManager* dm = ui->m_wnd->GetDocManager();

	// Open files list
	QTreeWidgetItem* it = new QTreeWidgetItem(QStringList("OPEN FILES"));
	QFont f = it->font(0);
	f.setBold(true);
	it->setFont(0, f);
	ui->m_tree->addTopLevelItem(it);
	it->setExpanded(true);

	QFontInfo fi(f);

	int px = fi.pixelSize();
	px = 5 * px / 3;
	it->setSizeHint(0, QSize(100, px));
	
	for (int i = 0; i < dm->Documents(); ++i)
	{
		CDocument* doc = dm->GetDocument(i);
		QString docPath = QString::fromStdString(doc->GetDocFilePath());

		QTreeWidgetItem* t2 = new QTreeWidgetItem(it);
		t2->setText(0, QString::fromStdString(doc->GetDocTitle()));
		t2->setData(0, Qt::UserRole, i);
		if (docPath.isEmpty() == false)
		{
			t2->setToolTip(0, docPath);
		}
		t2->setSizeHint(0, QSize(100, px));
	}

	// Project list
	it = new QTreeWidgetItem(QStringList("PROJECT (unsaved)"));
	it->setFont(0, f);
	ui->m_tree->addTopLevelItem(it);
	it->setExpanded(true);
	it->setSizeHint(0, QSize(0, px));

	for (int i = 0; i < dm->Documents(); ++i)
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(dm->GetDocument(i));
		if (doc && (doc->GetDocFilePath().empty() == false))
		{
			QString docFile = QString::fromStdString(doc->GetDocFileName());
			QString docPath = QString::fromStdString(doc->GetDocFilePath());

			QTreeWidgetItem* t2 = new QTreeWidgetItem(QStringList(docFile));
			t2->setSizeHint(0, QSize(100, 50));
			t2->setToolTip(0, docPath);
			t2->setData(0, Qt::UserRole, docPath);
			t2->setSizeHint(0, QSize(100, px));

			for (int n = 0; n < doc->FEBioJobs(); ++n)
			{
				CFEBioJob* job = doc->GetFEBioJob(n);

				std::string plotFile = job->GetPlotFileName();
				QString xpltPath(doc->ToAbsolutePath(plotFile));

				QFileInfo xpltFile(xpltPath);

				QTreeWidgetItem* t3 = new QTreeWidgetItem(t2);
				t3->setText(0, xpltFile.fileName());
				t3->setToolTip(0, xpltPath);
				t3->setData(0, Qt::UserRole, xpltPath);
				t3->setSizeHint(0, QSize(100, px));
			}
			it->addChild(t2);
		}
	}
}

