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
#include "ProjectViewer.h"
#include "MainWindow.h"
#include "DocManager.h"
#include "Document.h"
#include "ModelDocument.h"
#include "PostDocument.h"
#include "FEBioStudioProject.h"
#include <QApplication>
#include <QTreeWidget>
#include <QFileSystemModel>
#include <QBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QSignalMapper>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QLabel>
#include <QSplitter>
#include <QFileIconProvider>
#include <QFileDialog>
#include <QDirIterator>
#include "IconProvider.h"
#include "DlgCreatePlugin.h"
#include "PluginManager.h"

enum FileItemType {
	OPEN_FILES,
	OPEN_FILE,
	PROJECT,
	PROJECT_GROUP,
	PROJECT_FILE,
	PROJECT_PLUGIN,
	EXTERNAL_FILE
};

class Ui::CProjectViewer
{
public:
	::CMainWindow*	m_wnd;
	QTreeWidget*	m_tree;
	QTextEdit*		m_info;

	CPluginProcess* m_process = nullptr;

public:
	void setupUi(QWidget* parent)
	{
		QSplitter* s = new QSplitter;
		s->setOrientation(Qt::Vertical);

		m_tree = new QTreeWidget;
		m_tree->setColumnCount(1);
		m_tree->header()->hide();
		m_tree->setObjectName("fileList");
		m_tree->setUniformRowHeights(true);
		s->addWidget(m_tree);

		QWidget* w = new QWidget;
		QVBoxLayout* lw = new QVBoxLayout;

		QLabel* notes = new QLabel("Notes:");
		notes->setAlignment(Qt::AlignLeft);
		lw->addWidget(notes);

		m_info = new QTextEdit;
		m_info->setObjectName("info");
		QFont f = m_info->font();
		f.setPointSize(11);
		m_info->setFont(f);
		lw->addWidget(m_info);
		w->setLayout(lw);
		s->addWidget(w);

		QVBoxLayout* l = new QVBoxLayout(parent);
		l->setContentsMargins(0,0,0,0);
		l->addWidget(s);

		parent->setLayout(l);
	}
};

CProjectViewer::CProjectViewer(CMainWindow* pwnd, QWidget* parent) : QWidget(parent), ui(new Ui::CProjectViewer)
{
	ui->m_wnd = pwnd;
	// build Ui
	ui->setupUi(this);

	QMetaObject::connectSlotsByName(this);
}

void CProjectViewer::on_fileList_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (current == nullptr)
	{
		ui->m_info->clear();
		ui->m_info->setDisabled(true);
		return;
	}

	int itemId = current->data(0, Qt::UserRole + 1).toInt();
	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	const FEBioStudioProject::ProjectItem* item = prj->FindItem(itemId);
	if (item)
	{
		ui->m_info->setText(item->Info());
		ui->m_info->setEnabled(true);
	}
	else
	{
		ui->m_info->clear();
		ui->m_info->setDisabled(true);
	}
}

void CProjectViewer::on_fileList_itemDoubleClicked(QTreeWidgetItem* item, int column)
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
	{
		int itemId = item->data(0, Qt::UserRole + 1).toInt();
		const FEBioStudioProject* prj = ui->m_wnd->GetProject();
		const FEBioStudioProject::ProjectItem* file = prj->FindFile(itemId); assert(file);
		QString filePath = file->Name();
		CDocument* doc = ui->m_wnd->FindDocument(filePath.toStdString());
		if (doc)
			ui->m_wnd->SetActiveDocument(doc);
		else
			ui->m_wnd->OpenFile(filePath, false);
	}
	break;
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

void CProjectViewer::on_info_textChanged()
{
	QTreeWidgetItem* current = ui->m_tree->currentItem();
	if (current == nullptr) return;

	int itemId = current->data(0, Qt::UserRole + 1).toInt();
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	FEBioStudioProject::ProjectItem* item = prj->FindItem(itemId);
	if (item)
	{
		item->SetInfo(ui->m_info->toPlainText());
	}
}

void CProjectViewer::contextMenuEvent(QContextMenuEvent* ev)
{
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

		if (prj->ContainsFile(file) == false)
		{
			QSignalMapper* addMap = new QSignalMapper(&menu);
			QAction* ac = menu.addAction("Add to project", addMap, SLOT(map())); addMap->setMapping(ac, file);
			connect(addMap, SIGNAL(mapped(QString)), ui->m_wnd, SLOT(on_addToProject(const QString&)));
		}

#ifdef _WIN32
		menu.addAction("Show in Explorer", this, SLOT(onShowInExplorer()));
#endif

		menu.exec(ev->globalPos());
	}
	if (ntype == FileItemType::PROJECT)
	{
		QMenu menu(this);
		menu.addAction("Save Project As ...", ui->m_wnd, SLOT(on_actionSaveProject_triggered()));
		menu.addAction("Create Group ...", this, SLOT(onCreateGroup()));
		menu.addAction("Close project", ui->m_wnd, SLOT(on_closeProject()));
		menu.addAction("Clear project", ui->m_wnd, SLOT(on_clearProject()));
		menu.addAction("Add file ...", this, SLOT(onAddFile()));
		menu.addAction("Import folder ...", this, SLOT(onImportFolder()));

#ifdef _WIN32
		const FEBioStudioProject* prj = ui->m_wnd->GetProject();
		if (prj->GetProjectFileName().isEmpty() == false)
		{
			menu.addAction("Show in Explorer", this, SLOT(onShowInExplorer()));
		}
#endif

		menu.exec(ev->globalPos());
	}
	else if (ntype == FileItemType::PROJECT_GROUP)
	{
		QMenu menu(this);
		menu.addAction("Remove Group", this, SLOT(onRemoveGroup()));
		menu.addAction("Rename Group ...", this, SLOT(onRenameGroup()));
		menu.addAction("Add Group ...", this, SLOT(onCreateGroup()));
		menu.addAction("Add file ...", this, SLOT(onAddFile()));
		menu.exec(ev->globalPos());
	}
	else if (ntype == FileItemType::PROJECT_PLUGIN)
	{
		QMenu menu(this);
		menu.addAction("Build", this, SLOT(onBuildPlugin()));
		menu.addAction("Load", this, SLOT(onLoadPlugin()));
		menu.addSeparator();
		menu.addAction("Add FEBio feature ...", this, SLOT(onAddFEBioFeature()));
		menu.addSeparator();
//		menu.addAction("Remove Group", this, SLOT(onRemoveGroup()));
//		menu.addAction("Rename Group ...", this, SLOT(onRenameGroup()));
		menu.addAction("Add Group ...", this, SLOT(onCreateGroup()));
		menu.addAction("Add file ...", this, SLOT(onAddFile()));
		menu.exec(ev->globalPos());
	}
	else if (ntype == FileItemType::PROJECT_FILE)
	{
		QMenu menu(this);
		int fileId = sel[0]->data(0, Qt::UserRole + 1).toInt();


		QAction* ac = menu.addAction("Remove from project", this, SLOT(onRemoveFromProject()));

		const FEBioStudioProject::ProjectItem* prjItem = prj->FindFile(fileId);
		if (prjItem && (prjItem->Parent()))
		{
			QSignalMapper* groupmap = new QSignalMapper;
			QMenu* groupMenu = new QMenu("Move to group");
			std::vector<int> groups = prjItem->Parent()->AllGroups();
			for (int i = 0; i < groups.size(); ++i)
			{
				const FEBioStudioProject::ProjectItem* group_i = prj->FindGroup(groups[i]);
				QAction* aci = groupMenu->addAction(group_i->Name(), groupmap, SLOT(map())); groupmap->setMapping(aci, group_i->Id());
			}
			QAction* aci = groupMenu->addAction("(none)", groupmap, SLOT(map())); groupmap->setMapping(aci, -1);
			menu.addAction(groupMenu->menuAction());
			connect(groupmap, SIGNAL(mappedInt(int)), this, SLOT(onMoveToGroup(int)));
		}

		menu.addAction("Add file ...", this, SLOT(onAddFile()));

#ifdef _WIN32
		menu.addAction("Show in Explorer", this, SLOT(onShowInExplorer()));
#endif
		menu.exec(ev->globalPos());
	}
}

void CProjectViewer::onShowInExplorer()
{
#ifdef WIN32
	QList<QTreeWidgetItem*> itemList = ui->m_tree->selectedItems();
	if (itemList.size() != 1) return;

	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	QString file;
	int ntype = itemList[0]->data(0, Qt::UserRole).toInt();
	if (ntype == FileItemType::OPEN_FILE)
	{
		CDocManager* dm = ui->m_wnd->GetDocManager();
		int n = itemList[0]->data(0, Qt::UserRole + 1).toInt();
		file = QString::fromStdString(dm->GetDocument(n)->GetDocFilePath());
	}
	else if (ntype == FileItemType::PROJECT_FILE)
	{
		int id = itemList[0]->data(0, Qt::UserRole + 1).toInt();
		file = prj->FindFile(id)->Name();
	}
	else if (ntype == FileItemType::EXTERNAL_FILE)
	{
		file = itemList[0]->data(0, Qt::UserRole + 1).toString();
	}
	else if (ntype == FileItemType::PROJECT)
	{
		file = prj->GetProjectFileName();
	}

	if (file.isEmpty() == false)
	{
		QProcess::startDetached("explorer.exe", { "/select,", QDir::toNativeSeparators(file) });
	}
#endif
}

QTreeWidgetItem* addProjectItem(QTreeWidgetItem* treeItem, const FEBioStudioProject::ProjectItem& item, CDocument* doc)
{
	QFileIconProvider iconProvider;
	QTreeWidgetItem* t2 = nullptr;

	QString filename_i = item.Name();
	QFileInfo fi(filename_i);
	QString fileName = fi.fileName();
	QString ext = fi.suffix();

	QIcon icon = iconProvider.icon(QFileIconProvider::File);
	if      (ext == "fsm" ) icon = QIcon(":/icons/FEBioStudio.png");
	else if (ext == "fs2" ) icon = QIcon(":/icons/FEBioStudio.png");
	else if (ext == "xplt") icon = QIcon(":/icons/PostView.png");
	else if (ext == "feb" ) icon = QIcon(":/icons/febio.png");
	else if (ext == "pdf" ) icon = QIcon(":/icons/pdf.png");
	else if (fileName.indexOf("CMakeLists.txt") != -1) icon = QIcon(":/icons/cmake.png");
	else if (ext == "txt" ) icon = QIcon(":/icons/txt.png");
	else if (ext == "mpg" ) icon = QIcon(":/icons/video.png");
	else if (ext == "html") icon = QIcon(":/icons/html.png");
	else if ((ext == "cpp") || (ext == "cxx")) icon = QIcon(":/icons/cpp_src.png");
	else if ((ext == "h"  ) || (ext == "hpp")) icon = QIcon(":/icons/cpp_hdr.png");

	if (doc && (doc->GetDocFilePath().empty() == false))
	{
		QString docFile = QString::fromStdString(doc->GetDocFileName());
		QString docPath = QString::fromStdString(doc->GetDocFilePath());

		t2 = new QTreeWidgetItem(treeItem);
		t2->setText(0, docFile);
		t2->setIcon(0, icon);
		t2->setToolTip(0, docPath);
		t2->setData(0, Qt::UserRole, FileItemType::PROJECT_FILE);
		t2->setData(0, Qt::UserRole + 1, item.Id());
	}
	else
	{
		t2 = new QTreeWidgetItem(treeItem);
		t2->setText(0, fileName);
		t2->setIcon(0, icon);

		t2->setSizeHint(0, QSize(100, 50));
		t2->setToolTip(0, filename_i);
		t2->setData(0, Qt::UserRole, FileItemType::PROJECT_FILE);
		t2->setData(0, Qt::UserRole + 1, item.Id());
	}

	return t2;
}

void addProjectGroup(const FEBioStudioProject::ProjectItem& parent, QTreeWidgetItem* treeItem, CMainWindow* wnd)
{
	QFileIconProvider iconProvider;

	// add groups first
	for (int n = 0; n < parent.Items(); ++n)
	{
		const FEBioStudioProject::ProjectItem& item = parent.Item(n);

		if (item.IsGroup())
		{
			QTreeWidgetItem* t2 = new QTreeWidgetItem(treeItem);
			t2->setText(0, item.Name());
			t2->setData(0, Qt::UserRole, FileItemType::PROJECT_GROUP);
			t2->setData(0, Qt::UserRole + 1, item.Id());
			t2->setIcon(0, iconProvider.icon(QFileIconProvider::Folder));
			QFont f = t2->font(0);
			f.setBold(true);
			t2->setFont(0, f);
			addProjectGroup(item, t2, wnd);
		}
		else if (item.IsPlugin())
		{
			QTreeWidgetItem* t2 = new QTreeWidgetItem(treeItem);
			t2->setText(0, item.Name());
			t2->setData(0, Qt::UserRole, FileItemType::PROJECT_PLUGIN);
			t2->setData(0, Qt::UserRole + 1, item.Id());
			t2->setIcon(0, CIconProvider::GetIcon("plugin"));
			QFont f = t2->font(0);
			f.setBold(true);
			t2->setFont(0, f);
			addProjectGroup(item, t2, wnd);
		}
	}

	// add files next
	for (int n = 0; n < parent.Items(); ++n)
	{
		const FEBioStudioProject::ProjectItem& item = parent.Item(n);

		if (item.IsFile())
		{
			QString filename_i = item.Name();
			QFileInfo fi(filename_i);
			QString fileName = fi.fileName();
			QString ext = fi.suffix();

			CDocument* doc = wnd->FindDocument(filename_i.toStdString());
			QTreeWidgetItem* t2 = addProjectItem(treeItem, item, doc);

			if (item.Items() > 0)
			{
				addProjectGroup(item, t2, wnd);
			}
		}
	}
}

void CProjectViewer::Update()
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

		QFileIconProvider iconProvider;

		string iconString = doc->GetIcon();
		QIcon icon(QString::fromStdString(iconString));

		QTreeWidgetItem* t2 = new QTreeWidgetItem(it);
		t2->setText(0, QString::fromStdString(doc->GetDocTitle()));
		t2->setIcon(0, icon);
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

	const FEBioStudioProject::ProjectItem& root = prj->RootItem();
	addProjectGroup(root, it, ui->m_wnd);

	ui->m_tree->expandAll();
}

QTreeWidgetItem* CProjectViewer::currentItem()
{
	QList<QTreeWidgetItem*> sel = ui->m_tree->selectedItems();
	if (sel.size() != 1) return nullptr;
	return sel[0];
}

void CProjectViewer::SelectItem(int itemId)
{
	QTreeWidgetItemIterator it(ui->m_tree);
	while (*it) 
	{
		int Id = (*it)->data(0, Qt::UserRole + 1).toInt();
		if (Id == itemId)
		{
			ui->m_tree->setCurrentItem((*it));
			break;
		}
		++it;
	}
}

void CProjectViewer::onCreateGroup()
{
	QTreeWidgetItem* item = CProjectViewer::currentItem();
	if (item == nullptr) return;

	int ntype = item->data(0, Qt::UserRole).toInt();
	if (( ntype == FileItemType::PROJECT) || 
		( ntype == FileItemType::PROJECT_GROUP) ||
		( ntype == FileItemType::PROJECT_PLUGIN))
	{
		QString groupName = QInputDialog::getText(this, "Create Group", "Group name:");
		if (groupName.isEmpty() == false)
		{
			FEBioStudioProject* prj = ui->m_wnd->GetProject();
			int groupId = item->data(0, Qt::UserRole + 1).toInt();
			prj->AddGroup(groupName, groupId);

			Update();
			QList<QTreeWidgetItem*> itemList = ui->m_tree->findItems(groupName, Qt::MatchExactly | Qt::MatchRecursive);
			if (itemList.empty() == false)
			{
				QTreeWidgetItem* it = itemList[0];
				it->setSelected(true);
				it = it->parent();
				while (it) { it->setExpanded(true); it = it->parent(); }
			}
		}
	}
}

void CProjectViewer::onMoveToGroup(int i)
{
	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype == PROJECT_FILE)
	{
		int itemId = item->data(0, Qt::UserRole + 1).toInt();
		FEBioStudioProject* prj = ui->m_wnd->GetProject();
		prj->MoveToGroup(itemId, i);

		QString txt = item->text(0);
		Update();
		QList<QTreeWidgetItem*> itemList = ui->m_tree->findItems(txt, Qt::MatchExactly | Qt::MatchRecursive);
		if (itemList.empty() == false)
		{
			QTreeWidgetItem* it = itemList[0];
			it->setSelected(true);
			it = it->parent();
			while (it) { it->setExpanded(true); it = it->parent(); }
		}
	}
}

void CProjectViewer::onRemoveGroup()
{
	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype == PROJECT_GROUP)
	{
		if (QMessageBox::question(this, "Remove Group", "Are you sure you want to remove this group?\nThis cannot be undone!") == QMessageBox::Yes)
		{
			int itemId = item->data(0, Qt::UserRole + 1).toInt();
			FEBioStudioProject* prj = ui->m_wnd->GetProject();
			prj->RemoveGroup(itemId);
			Update();
		}
	}
}

void CProjectViewer::onRenameGroup()
{
	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype == PROJECT_GROUP)
	{
		QString newName = QInputDialog::getText(this, "Rename Group", "New name:");
		if (newName.isEmpty() == false)
		{
			int itemId = item->data(0, Qt::UserRole + 1).toInt();
			FEBioStudioProject* prj = ui->m_wnd->GetProject();
			prj->RenameGroup(itemId, newName);
			item->setText(0, newName);
		}
	}
}

void CProjectViewer::onRemoveFromProject()
{
	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype == PROJECT_FILE)
	{
		if (QMessageBox::question(this, "Remove", "Are you sure you want to remove this file from the project?\nThis cannot be undone.") == QMessageBox::Yes)
		{
			FEBioStudioProject* prj = ui->m_wnd->GetProject();
			int itemId = item->data(0, Qt::UserRole + 1).toInt();
			prj->RemoveFile(itemId);
			Update();
		}
	}
}

void CProjectViewer::onAddFile()
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr) return;

	QString projectPath = prj->GetProjectPath();

	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	QString fileName = QFileDialog::getOpenFileName(this, "Add File", projectPath);
	if (fileName.isEmpty() == false)
	{
		FEBioStudioProject* prj = ui->m_wnd->GetProject();
		
		// see if this file is already part of the project
		FEBioStudioProject::ProjectItem* newItem = prj->FindFile(fileName);
		if (newItem)
		{
			QMessageBox::warning(this, "FEBio Studio", "File already exists in project.");
			SelectItem(newItem->Id());
		}
		else
		{
			int itemId = item->data(0, Qt::UserRole + 1).toInt();
			newItem = prj->AddFile(fileName, itemId);
			if (newItem)
			{
				Update();
				SelectItem(newItem->Id());
			}
		}
	}
}

void CProjectViewer::onImportFolder()
{
	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr) return;

	QFileDialog dlg;
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);
	if (dlg.exec())
	{
		QStringList folders = dlg.selectedFiles();
		if (folders.size() == 1)
		{
			QString path = folders.at(0);

			QDirIterator it(path, { "*.fsm", "*.fs2", "*.feb", "*.xplt" }, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot);
			while (it.hasNext())
			{
				QString file = it.next();
				prj->AddFile(file);
			}

			Update();
		}
	}
}

void CProjectViewer::onBuildPlugin()
{
	if (ui->m_process)
	{
		QMessageBox::critical(this, "FEBio Studio", "A build is already in progress.\nPlease wait until the build finishes.");
		return;
	}

	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr)
	{ 
		QMessageBox::critical(this, "FEBio Studio", "No active project.");
		return;
	}

	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype != PROJECT_PLUGIN)
	{
		QMessageBox::critical(this, "FEBio Studio", "A plugin project was not selected.");
		return;
	}

	// make sure the project is saved
	QString projectPath = prj->GetProjectPath();
	if (projectPath.isEmpty())
	{
		QMessageBox::information(this, "FEBio Studio", "Please save the project file first.");
		return;
	}

	// make sure log window is visible
	ui->m_wnd->ShowLogPanel();
	ui->m_wnd->ClearBuildLog();
	ui->m_wnd->AddBuildLogEntry(QString("Building plugin %1:\n").arg(item->text(0)));

	ui->m_process = new CConfigurePluginProcess(this);
	ui->m_process->setWorkingDirectory(projectPath + "/" + item->text(0));
	ui->m_process->run();
}

void CProjectViewer::onLoadPlugin()
{
	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No active project.");
		return;
	}

	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype != PROJECT_PLUGIN)
	{
		QMessageBox::critical(this, "FEBio Studio", "A plugin project was not selected.");
		return;
	}

	QString pluginName = item->text(0);
	const FEBioStudioProject::ProjectItem* prjItem = prj->FindPlugin(pluginName);
	if (prjItem == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "The plugin could not be located in the project.");
		return;
	}

	// we need to figure out if the plugin was built. 
	// We are just going to look for the library file
	// It is assumed that the file name is .\build\(config)\name.dll" relative to the projects folder.
#ifdef WIN32
#ifndef NDEBUG
	QString dllpath = QString("./%1/build/Debug/%1.dll").arg(pluginName);
#else
	QString dllpath = QString("./%1/build/Release/%1.dll").arg(pluginName);
#endif
#elif __APPLE__
    QString dllpath = QString("./%1/build/lib/lib%1.dylib").arg(pluginName);
#else
    QString dllpath = QString("./%1/build/lib/lib%1.so").arg(pluginName);
#endif

	dllpath = prj->ToAbsolutePath(dllpath);

	ui->m_wnd->AddLogEntry(QString("Loading %1 ... ").arg(dllpath));

    std::string stdPath = dllpath.toStdString();
	bool b = ui->m_wnd->GetPluginManager()->LoadNonRepoPlugin(stdPath);
	if (b)
	{
		ui->m_wnd->AddLogEntry(QString("success\n"));
		QMessageBox::information(this, "FEBio Studio", "Plugin loaded successfully");
	}
	else
	{
		ui->m_wnd->AddLogEntry(QString("failed\n"));
		QMessageBox::critical(this, "FEBio Studio", "Failed to load plugin.");
	}
}

void CProjectViewer::onAddFEBioFeature()
{
	if (ui->m_process)
	{
		QMessageBox::critical(this, "FEBio Studio", "A build is in progress.\nPlease wait until the build finishes.");
		return;
	}

	FEBioStudioProject* prj = ui->m_wnd->GetProject();
	if (prj == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "No active project.");
		return;
	}

	QTreeWidgetItem* item = CProjectViewer::currentItem();
	int ntype = item->data(0, Qt::UserRole).toInt();
	if (ntype != PROJECT_PLUGIN)
	{
		QMessageBox::critical(this, "FEBio Studio", "A plugin project was not selected.");
		return;
	}

	QString pluginName = item->text(0);
	FEBioStudioProject::ProjectItem* prjItem = prj->FindPlugin(pluginName);
	if (prjItem == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "A plugin project was not selected.");
		return;
	}

	CDlgAddPluginClass dlg(ui->m_wnd, prj, prjItem);
	if (dlg.exec())
	{
		Update();
	}
}

void CProjectViewer::onConfigureFinished(int exitCode, QProcess::ExitStatus es)
{
	if ((exitCode != 0) || (es != QProcess::NormalExit))
	{
		QString msg = QString("Exitcode = %1, Exit status = %2").arg(exitCode).arg(es == QProcess::NormalExit ? "Normal exit" : "Crash exit");
		QMessageBox::information(this, "FEBio Studio", msg);
		delete ui->m_process;
		ui->m_process = nullptr;
		return;
	}

	delete ui->m_process;

	QTreeWidgetItem* item = CProjectViewer::currentItem();
	QString relPath = item->text(0);

	const FEBioStudioProject* prj = ui->m_wnd->GetProject();
	QString projectPath = prj->GetProjectPath();
	ui->m_process = new CBuildPluginProcess(this);
	ui->m_process->setWorkingDirectory(projectPath + "/" + relPath);
	ui->m_process->run();
}

void CProjectViewer::onBuildFinished(int exitCode, QProcess::ExitStatus es)
{
	if (es == QProcess::NormalExit)
	{
		ui->m_wnd->AddBuildLogEntry(exitCode == 0 ? "\n--- Build succeeded\n\n" : "\n--- Build failed\n\n");
		if (exitCode == 0)
			QMessageBox::information(this, "FEBio Studio", "The build was successful.");
		else
			QMessageBox::critical(this, "FEBio Studio", "The build failed.\nSee build log for details.");
	}
	else
	{
		QString msg = QString("Exitcode = %1, Exit status = %2").arg(exitCode).arg(es == QProcess::NormalExit ? "Normal exit" : "Crash exit");
		QMessageBox::information(this, "FEBio Studio", msg);
	}

	delete ui->m_process;
	ui->m_process = nullptr;
}

void CProjectViewer::onReadyRead()
{
	if (ui->m_process == nullptr) return;

	QByteArray output = ui->m_process->readAll();
	QString s(output);
	ui->m_wnd->AddBuildLogEntry(output);
}

void CProjectViewer::onErrorOccurred(QProcess::ProcessError err)
{
	QString errString;
	switch (err)
	{
	case QProcess::FailedToStart: errString = "Failed to start"; break;
	case QProcess::Crashed      : errString = "Crashed"; break;
	case QProcess::Timedout     : errString = "Timed out"; break;
	case QProcess::WriteError   : errString = "Write error"; break;
	case QProcess::ReadError    : errString = "Read error"; break;
	case QProcess::UnknownError : errString = "Unknown error"; break;
	default:
		errString = QString("Error code = %1").arg(err);
	}

	QString t = "An error has occurred.\nError = " + errString;
	QMessageBox::critical(this, "FEBio Studio", t);
}

CConfigurePluginProcess::CConfigurePluginProcess(QObject* parent) : CPluginProcess(parent)
{
	setProcessChannelMode(QProcess::MergedChannels);

	QObject::connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), parent, SLOT(onConfigureFinished(int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(readyRead()), parent, SLOT(onReadyRead()));
	QObject::connect(this, SIGNAL(errorOccurred(QProcess::ProcessError)), parent, SLOT(onErrorOccurred(QProcess::ProcessError)));
}

void CConfigurePluginProcess::run()
{
    CMainWindow* wnd = dynamic_cast<CMainWindow*>(QApplication::activeWindow());
    if (wnd == nullptr) return;

    QString sdkPath = "-DCMAKE_PREFIX_PATH=" + wnd->GetSDKPath();

	start("cmake", QStringList() << "-S" << "." << "-B" << "./build" << sdkPath);
}

CBuildPluginProcess::CBuildPluginProcess(QObject* parent) : CPluginProcess(parent)
{
	setProcessChannelMode(QProcess::MergedChannels);

	QObject::connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), parent, SLOT(onBuildFinished(int, QProcess::ExitStatus)));
	QObject::connect(this, SIGNAL(readyRead()), parent, SLOT(onReadyRead()));
	QObject::connect(this, SIGNAL(errorOccurred(QProcess::ProcessError)), parent, SLOT(onErrorOccurred(QProcess::ProcessError)));
}

void CBuildPluginProcess::run()
{
#ifndef NDEBUG
	start("cmake", QStringList() << "--build" << "./build" << "--config" << "Debug");
#else
	start("cmake", QStringList() << "--build" << "./build" << "--config" << "Release");
#endif
}
