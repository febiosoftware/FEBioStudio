#include "stdafx.h"
#include "FileViewer.h"
#include "MainWindow.h"
#include "DocManager.h"
#include "Document.h"
#include "ModelDocument.h"
#include "PostDocument.h"
#include "FEBioStudioProject.h"
#include <QTreeWidget>
#include <QFileSystemModel>
#include <QBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QSignalMapper>

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
	int ntype = item->data(0, Qt::UserRole).toInt();

	switch (ntype)
	{
	case 1:
	{
		int nview = item->data(0, Qt::UserRole + 1).toInt();
		ui->m_wnd->SetActiveView(nview);
	}
	break;
	case 3:
	case 4:
	{
		QString filePath = item->data(0, Qt::UserRole + 1).toString();
		CDocument* doc = ui->m_wnd->FindDocument(filePath.toStdString());
		if (doc)
			ui->m_wnd->SetActiveDocument(doc);
		else
			ui->m_wnd->OpenFile(filePath, false);
	}
	break;
	}
}

void CFileViewer::contextMenuEvent(QContextMenuEvent* ev)
{
	QList<QTreeWidgetItem*> sel = ui->m_tree->selectedItems();
	if (sel.size() != 1) return;

	int ntype = sel[0]->data(0, Qt::UserRole).toInt();
	if (ntype == 1)
	{
		const FEBioStudioProject* prj = ui->m_wnd->GetProject();
		CDocManager* dm = ui->m_wnd->GetDocManager();
		int n = sel[0]->data(0, Qt::UserRole + 1).toInt();
		QString file = QString::fromStdString(dm->GetDocument(n)->GetDocFilePath());

		QMenu menu(this);

		QSignalMapper* closeMap = new QSignalMapper(&menu);
		QAction* ac = menu.addAction("Close", closeMap, SLOT(map())); closeMap->setMapping(ac, file);
		connect(closeMap, SIGNAL(mapped(QString)), ui->m_wnd, SLOT(on_closeFile(const QString&)));

		if (prj->Contains(file) == false)
		{
			QSignalMapper* addMap = new QSignalMapper(&menu);
			QAction* ac = menu.addAction("Add to project", addMap, SLOT(map())); addMap->setMapping(ac, file);
			connect(addMap, SIGNAL(mapped(QString)), ui->m_wnd, SLOT(on_addToProject(const QString&)));
		}

		menu.exec(ev->globalPos());
	}
	if (ntype == 2)
	{
		QMenu menu(this);
		menu.addAction("Save Project As ...", ui->m_wnd, SLOT(on_actionSaveProject_triggered()));
		menu.addAction("Close project", ui->m_wnd, SLOT(on_closeProject()));
		menu.addAction("Clear project", ui->m_wnd, SLOT(on_clearProject()));

	}
	else if (ntype == 3)
	{
		QMenu menu(this);
		QString file = sel[0]->data(0, Qt::UserRole + 1).toString();

		QSignalMapper* map = new QSignalMapper;
		QAction* ac = menu.addAction("Remove from project", map, SLOT(map())); map->setMapping(ac, file);

		connect(map, SIGNAL(mapped(QString)), ui->m_wnd, SLOT(on_removeFromProject(const QString&)));

		menu.exec(ev->globalPos());
	}
}

void CFileViewer::Update()
{
	ui->m_tree->clear();

	CDocManager* dm = ui->m_wnd->GetDocManager();

	// Open files list
	QTreeWidgetItem* it = new QTreeWidgetItem(QStringList("OPEN FILES"));
	it->setData(0, Qt::UserRole, 0);
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
		t2->setData(0, Qt::UserRole  , 1);
		t2->setData(0, Qt::UserRole+1, i);
		if (docPath.isEmpty() == false)
		{
			t2->setToolTip(0, docPath);
		}
		t2->setSizeHint(0, QSize(100, px));
	}

	// Project list
	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	QString prjFile = prj->GetProjectFileName();

	QString prjName = "unsaved";
	if (prjFile.isEmpty() == false)
	{
		QFileInfo fi(prjFile);
		prjName = fi.fileName();
	}

	it = new QTreeWidgetItem(ui->m_tree);
	it->setText(0, QString("PROJECT (%1)").arg(prjName));
	it->setFont(0, f);
	ui->m_tree->addTopLevelItem(it);
	it->setExpanded(true);
	it->setSizeHint(0, QSize(0, px));
	it->setData(0, Qt::UserRole, 2);
	if (prjFile.isEmpty() == false) it->setToolTip(0, prjFile);

	for (int i = 0; i < prj->Files(); ++i)
	{
		QString file_i = prj->GetFileName(i);

		CModelDocument* doc = dynamic_cast<CModelDocument*>(ui->m_wnd->FindDocument(file_i.toStdString()));
		if (doc && (doc->GetDocFilePath().empty() == false))
		{
			QString docFile = QString::fromStdString(doc->GetDocFileName());
			QString docPath = QString::fromStdString(doc->GetDocFilePath());

			QTreeWidgetItem* t2 = new QTreeWidgetItem(QStringList(docFile));
			t2->setSizeHint(0, QSize(100, 50));
			t2->setToolTip(0, docPath);
			t2->setData(0, Qt::UserRole, 3);
			t2->setData(0, Qt::UserRole+1, docPath);
			t2->setSizeHint(0, QSize(100, px));

			if (doc->FEBioJobs()) t2->setExpanded(true);

			for (int n = 0; n < doc->FEBioJobs(); ++n)
			{
				CFEBioJob* job = doc->GetFEBioJob(n);

				std::string plotFile = job->GetPlotFileName();
				QString xpltPath(doc->ToAbsolutePath(plotFile));

				QFileInfo xpltFile(xpltPath);

				CPostDocument* postDoc = dynamic_cast<CPostDocument*>(ui->m_wnd->FindDocument(xpltPath.toStdString()));

				QTreeWidgetItem* t3 = new QTreeWidgetItem(t2);
				t3->setText(0, xpltFile.fileName());
				t3->setToolTip(0, xpltPath);
				t3->setData(0, Qt::UserRole, 4);
				t3->setData(0, Qt::UserRole+1, xpltPath);
				t3->setSizeHint(0, QSize(100, px));

				if (postDoc == nullptr)
				{
					QFont f = t3->font(0);
					f.setItalic(true);
					t3->setFont(0, f);
					t3->setForeground(0, Qt::gray);
				}
			}
			it->addChild(t2);
		}
		else
		{
			QFileInfo fi(file_i);
			QString fileName = fi.fileName();

			QTreeWidgetItem* t2 = new QTreeWidgetItem(it);
			t2->setText(0, fileName);

			QFont f = t2->font(0);
			f.setItalic(true);
			t2->setFont(0, f);
			t2->setForeground(0, Qt::gray);

			t2->setSizeHint(0, QSize(100, 50));
			t2->setToolTip(0, file_i);
			t2->setData(0, Qt::UserRole, 3);
			t2->setData(0, Qt::UserRole + 1, file_i);
			t2->setSizeHint(0, QSize(100, px));
		}
	}
}

