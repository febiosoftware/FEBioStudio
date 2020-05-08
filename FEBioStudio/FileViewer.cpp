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
#include <QInputDialog>
#include <QMessageBox>

enum FileItemType {
	OPEN_FILES,
	OPEN_FILE,
	PROJECT,
	PROJECT_GROUP,
	PROJECT_FILE,
	EXTERNAL_FILE
};

class Ui::CFileViewer
{
public:
	::CMainWindow*	m_wnd;
	QTreeWidget*	m_tree;

	QString	m_activeFile;

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
	case FileItemType::OPEN_FILE:
	{
		int nview = item->data(0, Qt::UserRole + 1).toInt();
		ui->m_wnd->SetActiveView(nview);
	}
	break;
	case FileItemType::PROJECT_FILE:
	case FileItemType::EXTERNAL_FILE:
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
	ui->m_activeFile.clear();

	QList<QTreeWidgetItem*> sel = ui->m_tree->selectedItems();
	if (sel.size() != 1) return;

	const FEBioStudioProject* prj = ui->m_wnd->GetProject();

	int ntype = sel[0]->data(0, Qt::UserRole).toInt();
	if (ntype == FileItemType::OPEN_FILE)
	{
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
	if (ntype == FileItemType::PROJECT)
	{
		QMenu menu(this);
		menu.addAction("Save Project As ...", ui->m_wnd, SLOT(on_actionSaveProject_triggered()));
		menu.addAction("Create Group ...", this, SLOT(onCreateGroup()));
		menu.addAction("Close project", ui->m_wnd, SLOT(on_closeProject()));
		menu.addAction("Clear project", ui->m_wnd, SLOT(on_clearProject()));
		menu.exec(ev->globalPos());
	}
	else if (ntype == FileItemType::PROJECT_GROUP)
	{
		ui->m_activeFile = sel[0]->text(0);
		QMenu menu(this);
		menu.addAction("Remove Group", this, SLOT(onRemoveGroup()));
		menu.addAction("Rename Group ...", this, SLOT(onRenameGroup()));
		menu.exec(ev->globalPos());
	}
	else if (ntype == FileItemType::PROJECT_FILE)
	{
		QMenu menu(this);
		QString file = sel[0]->data(0, Qt::UserRole + 1).toString();

		QSignalMapper* map = new QSignalMapper;
		QAction* ac = menu.addAction("Remove from project", map, SLOT(map())); map->setMapping(ac, file);

		if (prj->Groups())
		{
			ui->m_activeFile = file;
			QSignalMapper* groupmap = new QSignalMapper;
			QMenu* groupMenu = new QMenu("Move to group");
			for (int i = 0; i < prj->Groups(); ++i)
			{
				QAction* aci = groupMenu->addAction(prj->GetGroupName(i), groupmap, SLOT(map())); groupmap->setMapping(aci, i);
			}
			QAction* aci = groupMenu->addAction("(none)", groupmap, SLOT(map())); groupmap->setMapping(aci, -1);
			menu.addAction(groupMenu->menuAction());
			connect(groupmap, SIGNAL(mapped(int)), this, SLOT(onMoveToGroup(int)));
		}

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
	it->setData(0, Qt::UserRole, FileItemType::OPEN_FILES);
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
		t2->setData(0, Qt::UserRole  , FileItemType::OPEN_FILE);
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
	it->setData(0, Qt::UserRole, FileItemType::PROJECT);
	if (prjFile.isEmpty() == false) it->setToolTip(0, prjFile);

	for (int n = 0; n <= prj->Groups(); ++n)
	{
		QTreeWidgetItem* parent = it;
		int group = (n < prj->Groups() ? n : -1);
		if (group >= 0)
		{
			QTreeWidgetItem* t2 = new QTreeWidgetItem(it);
			t2->setText(0, prj->GetGroupName(group));
			t2->setData(0, Qt::UserRole, FileItemType::PROJECT_GROUP);
			QFont f = t2->font(0);
			f.setBold(true);
			t2->setFont(0, f);
			parent = t2;
		}

		for (int i = 0; i < prj->Files(); ++i)
		{
			FEBioStudioProject::File file_i = prj->GetFile(i);
			if (file_i.m_group == group)
			{
				QString filename_i = file_i.m_fileName;

				CDocument* doc = ui->m_wnd->FindDocument(filename_i.toStdString());

				if (doc && (doc->GetDocFilePath().empty() == false))
				{
					QString docFile = QString::fromStdString(doc->GetDocFileName());
					QString docPath = QString::fromStdString(doc->GetDocFilePath());

					QTreeWidgetItem* t2 = new QTreeWidgetItem(parent);
					t2->setText(0, docFile);
					t2->setSizeHint(0, QSize(100, 50));
					t2->setToolTip(0, docPath);
					t2->setData(0, Qt::UserRole, FileItemType::PROJECT_FILE);
					t2->setData(0, Qt::UserRole + 1, docPath);
					t2->setSizeHint(0, QSize(100, px));

					CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc);
					if (modelDoc && modelDoc->FEBioJobs())
					{
						t2->setExpanded(true);
						for (int n = 0; n < modelDoc->FEBioJobs(); ++n)
						{
							CFEBioJob* job = modelDoc->GetFEBioJob(n);

							std::string plotFile = job->GetPlotFileName();
							QString xpltPath(doc->ToAbsolutePath(plotFile));

							QFileInfo xpltFile(xpltPath);

							CPostDocument* postDoc = dynamic_cast<CPostDocument*>(ui->m_wnd->FindDocument(xpltPath.toStdString()));

							QTreeWidgetItem* t3 = new QTreeWidgetItem(t2);
							t3->setText(0, xpltFile.fileName());
							t3->setToolTip(0, xpltPath);
							t3->setData(0, Qt::UserRole, FileItemType::EXTERNAL_FILE);
							t3->setData(0, Qt::UserRole + 1, xpltPath);
							t3->setSizeHint(0, QSize(100, px));

							if (postDoc == nullptr)
							{
								QFont f = t3->font(0);
								f.setItalic(true);
								t3->setFont(0, f);
								t3->setForeground(0, Qt::gray);
							}
						}
					}
				}
				else
				{
					QFileInfo fi(filename_i);
					QString fileName = fi.fileName();

					QTreeWidgetItem* t2 = new QTreeWidgetItem(parent);
					t2->setText(0, fileName);

					QFont f = t2->font(0);
					f.setItalic(true);
					t2->setFont(0, f);
					t2->setForeground(0, Qt::gray);

					t2->setSizeHint(0, QSize(100, 50));
					t2->setToolTip(0, filename_i);
					t2->setData(0, Qt::UserRole, FileItemType::PROJECT_FILE);
					t2->setData(0, Qt::UserRole + 1, filename_i);
					t2->setSizeHint(0, QSize(100, px));
				}
			}
		}
	}
}

void CFileViewer::onCreateGroup()
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();

	QString groupName = QInputDialog::getText(this, "Create Group", "Group name:");
	if (groupName.isEmpty() == false)
	{
		prj->AddGroup(groupName);
		Update();
	}
}

void CFileViewer::onMoveToGroup(int i)
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (ui->m_activeFile.isEmpty() == false)
	{
		prj->MoveToGroup(ui->m_activeFile, i);
		Update();
	}
}

void CFileViewer::onRemoveGroup()
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (ui->m_activeFile.isEmpty() == false)
	{
		if (QMessageBox::question(this, "Remove Group", "Are you sure you want to remove this group?\nThis cannot be undone!") == QMessageBox::Yes)
		{
			prj->RemoveGroup(ui->m_activeFile);
			Update();
		}
	}
}

void CFileViewer::onRenameGroup()
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (ui->m_activeFile.isEmpty() == false)
	{
		QString newName = QInputDialog::getText(this, "Rename Group", "New name:");
		if (newName.isEmpty() == false)
		{
			prj->RenameGroup(ui->m_activeFile, newName);
			Update();
		}
	}
}
