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

#ifdef MODEL_REPO
#include <unordered_map>
#include <QApplication>
#include <QLocale>
#include <QPalette>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QToolButton>
#include <QToolBar>
#include <QCompleter>
#include <QScrollBar>
#include <QBoxLayout>
#include <QSplitter>
#include <QCheckBox>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QFormLayout>
#include <QLineEdit>
#include <QKeyEvent>
#include <QTextBrowser>
#include <QProgressBar>
#include <QLabel>
#include <QFont>
#include <QPushButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QJsonDocument>
#include <QByteArray>
#include <QDir>
#include <QList>
#include <QUrl>
#include <QMimeData>
#include <QFileIconProvider>
#include <QStandardPaths>
#include "ToolBox.h"
#include "PublicationWidgetView.h"
#include "IconProvider.h"
#include "MultiLineLabel.h"
#include "WrapLabel.h"
#include "TagLabel.h"
#include "CustomLineEdit.h"

#include <iostream>
#include <QDebug>

enum ITEMTYPES {PROJECTITEM = 1001, FOLDERITEM = 1002, FILEITEM = 1003};

class CustomTreeWidgetItem : public QTreeWidgetItem
{
public:
	CustomTreeWidgetItem(QString name, int type)
		: QTreeWidgetItem(QStringList(name), type), localCopy(0), totalCopies(0), m_size(0)
	{

	}

	virtual CustomTreeWidgetItem* getProjectItem() = 0;
    virtual QList<CustomTreeWidgetItem*> getFileItems() = 0;
    virtual void justDownloaded(qint64 time) = 0;

	bool LocalCopy() { return localCopy >= totalCopies; }

	int GetLocalCopy() { return localCopy; }
	int GetTotalCopies() { return totalCopies; }

	virtual void UpdateLocalCopyColor()
	{
		if(LocalCopy())
		{
			setForeground(0, qApp->palette().color(QPalette::Active, QPalette::Text));
			setForeground(1, qApp->palette().color(QPalette::Active, QPalette::Text));
		}
		else
		{
			setForeground(0, qApp->palette().color(QPalette::Disabled, QPalette::Text));
			setForeground(1, qApp->palette().color(QPalette::Disabled, QPalette::Text));
		}
	}


	void AddLocalCopy()
	{
		localCopy++;
		UpdateLocalCopyColor();

		if(type() != PROJECTITEM)
		{
			static_cast<CustomTreeWidgetItem*>(parent())->AddLocalCopy();

		}
	}

	void SubtractLocalCopy()
	{
		localCopy--;
		UpdateLocalCopyColor();

		if(type() != PROJECTITEM)
		{
			static_cast<CustomTreeWidgetItem*>(parent())->SubtractLocalCopy();

		}
	}

	void setLocalCopyRecursive(bool lc)
	{
		for(int index = 0; index < childCount(); index++)
		{
			static_cast<CustomTreeWidgetItem*>(child(index))->setLocalCopyRecursive(lc);
		}

		if(lc)
		{
			AddLocalCopy();
		}
		else
		{
			SubtractLocalCopy();
		}

	}

	void AddTotalCopy()
	{
		totalCopies++;
		UpdateLocalCopyColor();
	}

	void UpdateCopies()
	{
		int lc = 0;
		int tc = 0;

		for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));
			current->UpdateCopies();

			lc += current->GetLocalCopy();
			tc += current->GetTotalCopies();
		}

		localCopy += lc;
		totalCopies += tc;
		UpdateLocalCopyColor();
	}

	void UpdateSize()
	{
		int currentSize = 0;

		for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));
			current->UpdateSize();

			currentSize += current->m_size;
		}

		m_size += currentSize;

		setText(1, qApp->topLevelWidgets()[0]->locale().formattedDataSize(m_size, 2, QLocale::DataSizeTraditionalFormat));
	}

protected:
	int localCopy;
	int totalCopies;

	qint64 m_size;

};

class ProjectItem : public CustomTreeWidgetItem
{
public:
	ProjectItem(QString name, int projectID, bool owned, bool authorized)
		: CustomTreeWidgetItem(name, PROJECTITEM), m_projectID(projectID), m_ownedByUser(owned), m_authorized(authorized)
	{
		setIcon(0, CIconProvider::GetIcon("FEBioStudio"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return this;
	}

    QList<CustomTreeWidgetItem*> getFileItems()
    {
        QList<CustomTreeWidgetItem*> fileItems;

        for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));

            QList<CustomTreeWidgetItem*> tempList = current->getFileItems();

            for(auto item : tempList)
            {
                fileItems.append(item);
            }
        }

        return fileItems;
    }

    void justDownloaded(qint64 time)
    {
        for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));

            current->justDownloaded(time);
        }
    }

	void setProjectID(int project) {m_projectID = project;}
	int getProjectID() {return m_projectID;}
	bool ownedByUser() {return m_ownedByUser;}
	bool isAuthorized() {return m_authorized;}

private:
	int m_projectID;
	bool m_ownedByUser;
	bool m_authorized;
};

class FolderItem : public CustomTreeWidgetItem
{
public:
	FolderItem(QString name)
		: CustomTreeWidgetItem(name, FOLDERITEM)
	{
		setIcon(0, CIconProvider::GetIcon("folder"));
	}

	CustomTreeWidgetItem* getProjectItem()
	{
		return ((CustomTreeWidgetItem*) parent())->getProjectItem();
	}

    QList<CustomTreeWidgetItem*> getFileItems()
    {
        QList<CustomTreeWidgetItem*> fileItems;

        for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));

            QList<CustomTreeWidgetItem*> tempList = current->getFileItems();

            for(auto item : tempList)
            {
                fileItems.append(item);
            }
        }

        return fileItems;
    }

    void justDownloaded(qint64 time)
    {
        for(int index = 0; index < childCount(); index++)
		{
			CustomTreeWidgetItem* current = static_cast<CustomTreeWidgetItem*>(child(index));

            current->justDownloaded(time);
        }
    }

};

class FileItem : public CustomTreeWidgetItem
{
public:
	FileItem(QString name, int fileID, bool lc, qint64 size, qint64 uploadTime, qint64 downloadTime)
		: CustomTreeWidgetItem(name, FILEITEM), m_fileID(fileID),
            m_downloadTime(downloadTime), m_outOfDate(lc && (uploadTime >= downloadTime))
	{
		if(name.endsWith(".fsp"))
		{
			iconName = "FEBioStudio";
		}
		else if(name.endsWith(".fs2") || name.endsWith(".fsm") || name.endsWith(".fsprj") || name.endsWith(".prv"))
		{
			iconName = "PreView";
		}
		else if(name.endsWith(".feb"))
		{
			iconName = "febio";
		}
		else if(name.endsWith(".xplt"))
		{
			iconName = "PostView";
		}
		else
		{
			iconName = "new";
		}

        Update();

		localCopy = (lc ? 1 : 0);

		totalCopies = 1;

		UpdateLocalCopyColor();

        m_size = size;
	}

    void Update()
    {
        if(m_outOfDate)
        {
            setIcon(0, CIconProvider::GetIcon(iconName, Emblem::Caution));
            setToolTip(0,"Out of Date");
        }
        else
        {
            setIcon(0, CIconProvider::GetIcon(iconName));
            setToolTip(0,"");
        }
    }

	virtual CustomTreeWidgetItem* getProjectItem()
	{
		return ((CustomTreeWidgetItem*) parent())->getProjectItem();
	}
    
    QList<CustomTreeWidgetItem*> getFileItems()
    {
        return QList<CustomTreeWidgetItem*>() << this;
    }

    void justDownloaded(qint64 time)
    {
        m_downloadTime = time;
        m_outOfDate = false;

        Update();

        if(!localCopy) AddLocalCopy();
    }

    void justDeleted()
    {
        m_downloadTime = 0;
        m_outOfDate = false;

        Update();

        SubtractLocalCopy();
    }

	int getFileID()
	{
		return m_fileID;
	}

    bool outOfDate()
    {
        return m_outOfDate;
    }

    qint64 downloadTime()
    {
        return m_downloadTime;
    }

private:
	int m_fileID;
    bool m_outOfDate;
    qint64 m_downloadTime;
    QString iconName;

};

class SearchItem : public QTreeWidgetItem
{
public:
	SearchItem(CustomTreeWidgetItem* item)
		: QTreeWidgetItem(), realItem(item)
	{
		setText(0, realItem->text(0));
		setText(1, realItem->text(1));

        for(int index = 0; index < realItem->childCount(); index++)
        {
            addChild(new SearchItem(static_cast<CustomTreeWidgetItem*>(realItem->child(index))));
        }

		Update();
	}

	void Update()
	{
        for(int index = 0; index < childCount(); index++)
        {
            static_cast<SearchItem*>(child(index))->Update();
        }

		setForeground(0, realItem->foreground(0));
		setForeground(1, realItem->foreground(1));
        setIcon(0,realItem->icon(0));
	}

	CustomTreeWidgetItem* getRealItem()
	{
		return realItem;
	}

private:
	CustomTreeWidgetItem* realItem;
};

class SearchBox : public QWidget
{
public:
    SearchBox(QString name) : name(name)
    {
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);

        layout->addWidget(label = new QLabel(name + ":"));
        layout->addWidget(lineEdit = new CustomLineEdit);

        setLayout(layout);
    }

public:
    QString name;
    QLabel* label;
    CustomLineEdit* lineEdit;
};

class AdvancedSearchBox : public QWidget
{
public:
    AdvancedSearchBox()
    {
        layout = new QVBoxLayout;

        layout->addWidget(new QLabel("General:"));

        toolbar = new QToolBar;
        toolbar->setContentsMargins(0,0,0,0);
		toolbar->addWidget(general = new QLineEdit);
		actionSearch = new QAction(CIconProvider::GetIcon("search"), "Search");
		toolbar->addAction(actionSearch);
		actionClear = new QAction(CIconProvider::GetIcon("clear"), "Clear");
		toolbar->addAction(actionClear);
        actionHide = new QAction(CIconProvider::GetIcon("collapse"), "Hide");
		toolbar->addAction(actionHide);
        layout->addWidget(toolbar);

        gridLayout = new QGridLayout;
        gridLayout->setContentsMargins(0,0,0,0);

        layout->addLayout(gridLayout);

        setLayout(layout);
    }

    void Reset(std::vector<std::pair<QString, QStringList>> typeInfo)
    {
        QList<SearchBox*> boxes;
        
        for(auto child : children())
        {
            SearchBox* box = dynamic_cast<SearchBox*>(child);
            if(box)
            {
                boxes.append(box);
            }
        }

        for(auto box : boxes)
        {
            delete box;
        }

        AddSearchBoxes(typeInfo);
    }

    void AddSearchBoxes(std::vector<std::pair<QString, QStringList>> typeInfo)
    {
        int boxes = 0;

        for(auto child : children())
        {
            SearchBox* box = dynamic_cast<SearchBox*>(child);
            if(box)
            {
                boxes++;
            }
        }

        for(auto info : typeInfo)
        {
            int row = (boxes)/2;
            int col = (boxes)%2;

            SearchBox* box = new SearchBox(info.first);

            if(!info.second.empty())
            {
                QCompleter* completer = new QCompleter(info.second);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                completer->setFilterMode(Qt::MatchContains);

                box->lineEdit->setMultipleCompleter(completer);
            }

            QObject::connect(box->lineEdit, &QLineEdit::returnPressed, this, [this]{emit actionSearch->triggered();});

            gridLayout->addWidget(box, row, col);

            boxes++;
        }
    }

    void Clear()
    {
        general->clear();

        for(auto child : children())
        {
            SearchBox* box = dynamic_cast<SearchBox*>(child);
            if(box)
            {
                box->lineEdit->clear();
            }
        }
    }

    QString GetSearchTerm()
    {
        QString searchTerm;

        QString generalText = general->text().trimmed();

        if(!generalText.isEmpty())
        {
            searchTerm += "all:" + generalText;
        }

        for(auto child : children())
        {
            SearchBox* box = dynamic_cast<SearchBox*>(child);
            if(box)
            {
                QString boxText = box->lineEdit->text().trimmed();

                if(!boxText.isEmpty())
                {
                    searchTerm += " " + box->name.toLower() + ":" + boxText;
                }

            }
        }

        return searchTerm.trimmed();
    }

public:
    QToolBar* toolbar;
    QAction* actionSearch;
    QAction* actionClear;
    QAction* actionHide;


private:
    QVBoxLayout* layout;
    QLineEdit* general;
    QGridLayout* gridLayout;
};

class CustomTreeWidget : public QTreeWidget
{
public:
    CustomTreeWidget(CRepositoryPanel* repoPanel) : repoPanel(repoPanel) {}

protected:
    QStringList mimeTypes() const override
    {
        QStringList types;
        types << "text/uri-list";
        return types;
    }

    QMimeData* mimeData(const QList<QTreeWidgetItem *> &items) const override
    {
        QList<QUrl> urls;

        for(auto item : items)
        {
            FileItem* fileItem = nullptr;

            if(dynamic_cast<SearchItem*>(item))
            {
                fileItem = dynamic_cast<FileItem*>(dynamic_cast<SearchItem*>(item)->getRealItem());
            }
            else
            {
                fileItem = dynamic_cast<FileItem*>(item);
            }

            if(fileItem && fileItem->LocalCopy())
            {
                urls.append(QUrl::fromLocalFile(repoPanel->GetFilePathFromID(fileItem->getFileID())));
            }
        }

        if(urls.isEmpty())
        {
            return nullptr;
        }

        QMimeData* mimeData = new QMimeData;
        mimeData->setUrls(urls);

        return mimeData;
    }

private:
    CRepositoryPanel* repoPanel;

};


class Ui::CRepositoryPanel
{
public:
	QStackedLayout* stack;

	QWidget* welcomePage;
	QPushButton* connectButton;

	// QPushButton* loginButton;
	// QAction* loginAction;

	QWidget* modelPage;
	QStackedWidget* treeStack;
	CustomTreeWidget* projectTree;
	CustomTreeWidget* searchTree;
    QCheckBox* showProjectsCB;
    QCheckBox* showFilesCB;

	CToolBox* projectInfoBox;

	QLabel* unauthorized;

	QFormLayout* projectInfoForm;
	QLabel* projectName;
	MultiLineLabel* projectDesc;
	QLabel* projectOwner;
	TagLabel* projectTags;

	QFormLayout* fileInfoForm;
	MultiLineLabel* filenameLabel;
	MultiLineLabel* fileDescLabel;
	TagLabel* fileTags;

	::CPublicationWidgetView* projectPubs;

	QToolBar* toolbar;

	QAction* actionRefresh;
	QAction* actionDownload;
	QAction* actionOpen;
	QAction* actionOpenFileLocation;
	QAction* actionDelete;
	QAction* actionCopyPermalink;

	QAction* actionUpload;

	QAction* actionDeleteRemote;
	QAction* actionModify;

	QAction* actionFindInTree;

    QToolBar* searchBar;
	QLineEdit* searchLineEdit;
	QAction* actionSearch;
	QAction* actionClearSearch;
    QAction* actionShowAdvanced;

    AdvancedSearchBox* advancedSearch;

	QWidget* loadingPage;
	QLabel* loadingLabel;
	QProgressBar* loadingBar;
	QPushButton* loadingCancel;

public:
	CRepositoryPanel() : currentProject(nullptr), openAfterDownload(nullptr), projectInfo(nullptr){}

	void setupUi(::CRepositoryPanel* parent)
	{
		stack = new QStackedLayout(parent);

		// Weclome Page
		QVBoxLayout* welcomeVBLayout = new QVBoxLayout;
		welcomeVBLayout->setAlignment(Qt::AlignCenter);

		QLabel* welcomeLabel = new QLabel("To access the project repository, please click the Connect button below.");
		welcomeLabel->setWordWrap(true);
		welcomeLabel->setAlignment(Qt::AlignCenter);
		welcomeVBLayout->addWidget(welcomeLabel);

		QHBoxLayout* connectButtonLayout = new QHBoxLayout;
		connectButtonLayout->addStretch();
		connectButton = new QPushButton("Connect");
		connectButton->setObjectName("connectButton");
		connectButtonLayout->addWidget(connectButton);
		connectButtonLayout->addStretch();
		welcomeVBLayout->addLayout(connectButtonLayout);

		welcomePage = new QWidget;
		welcomePage->setLayout(welcomeVBLayout);

		stack->addWidget(welcomePage);

		// Model view page
		QVBoxLayout* modelVBLayout = new QVBoxLayout;

		toolbar = new QToolBar();

		actionRefresh = new QAction(CIconProvider::GetIcon("refresh"), "Refresh", parent);
		actionRefresh->setObjectName("actionRefresh");
		actionRefresh->setIconVisibleInMenu(false);
		toolbar->addAction(actionRefresh);

		actionDownload = new QAction(CIconProvider::GetIcon("download"), "Download", parent);
		actionDownload->setObjectName("actionDownload");
		actionDownload->setIconVisibleInMenu(false);
		toolbar->addAction(actionDownload);

		actionOpen = new QAction(CIconProvider::GetIcon("open"), "Open Local Copy", parent);
		actionOpen->setObjectName("actionOpen");
		actionOpen->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpen);

		actionOpenFileLocation = new QAction(CIconProvider::GetIcon("openContaining"), "Open File Location", parent);
		actionOpenFileLocation->setObjectName("actionOpenFileLocation");
		actionOpenFileLocation->setIconVisibleInMenu(false);
		toolbar->addAction(actionOpenFileLocation);

		actionDelete = new QAction(CIconProvider::GetIcon("delete"), "Delete Local Copy", parent);
		actionDelete->setObjectName("actionDelete");
		actionDelete->setIconVisibleInMenu(false);
		toolbar->addAction(actionDelete);

		actionCopyPermalink = new QAction("Copy Permalink", parent);
		actionCopyPermalink->setObjectName("actionCopyPermalink");

		toolbar->addSeparator();
		QWidget* empty = new QWidget();
		empty->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		toolbar->addWidget(empty);

		actionDeleteRemote = new QAction(CIconProvider::GetIcon("deleteRemote"), "Delete From Repository", parent);
		actionDeleteRemote->setObjectName("actionDeleteRemote");
		toolbar->addAction(actionDeleteRemote);

		actionModify = new QAction(CIconProvider::GetIcon("edit"), "Modify Project", parent);
		actionModify->setObjectName("actionModify");
		toolbar->addAction(actionModify);


		actionUpload = new QAction(CIconProvider::GetIcon("upload"), "Upload", parent);
		actionUpload->setObjectName("actionUpload");
		toolbar->addAction(actionUpload);

		modelVBLayout->addWidget(toolbar);

		searchBar = new QToolBar;
        searchBar->setContentsMargins(0,0,0,0);
		searchBar->addWidget(searchLineEdit = new QLineEdit);
		actionSearch = new QAction(CIconProvider::GetIcon("search"), "Search", parent);
		actionSearch->setObjectName("actionSearch");
		searchBar->addAction(actionSearch);
		actionClearSearch = new QAction(CIconProvider::GetIcon("clear"), "Clear", parent);
		actionClearSearch->setObjectName("actionClearSearch");
        searchBar->addAction(actionClearSearch);
        actionShowAdvanced = new QAction(CIconProvider::GetIcon("expand"), "Show", parent);
		actionShowAdvanced->setObjectName("actionShowAdvanced");
        searchBar->addAction(actionShowAdvanced);

        modelVBLayout->addWidget(searchBar);

        advancedSearch = new AdvancedSearchBox;
        advancedSearch->hide();
        modelVBLayout->addWidget(advancedSearch);
		
		actionFindInTree = new QAction("Show in Project Tree", parent);
		actionFindInTree->setObjectName("actionFindInTree");

		QSplitter* splitter = new QSplitter;            
		splitter->setOrientation(Qt::Vertical);

		treeStack = new QStackedWidget;

		projectTree = new CustomTreeWidget(parent);
		projectTree->setObjectName("treeWidget");
		projectTree->setColumnCount(2);
		projectTree->setHeaderLabels(QStringList() << "Projects" << "Size");
		projectTree->setSelectionMode(QAbstractItemView::SingleSelection);
        projectTree->setDragEnabled(true);
		projectTree->setContextMenuPolicy(Qt::CustomContextMenu);
		projectTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
		projectTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
		projectTree->header()->setStretchLastSection(false);
		treeStack->addWidget(projectTree);

        QWidget* searchTreeWidget = new QWidget;
        QVBoxLayout* searchTreeLayout = new QVBoxLayout;
        searchTreeLayout->setContentsMargins(0,0,0,0);

        QHBoxLayout* checkboxLayout = new QHBoxLayout;
        checkboxLayout->setContentsMargins(0,0,0,0);
        checkboxLayout->addStretch();

        showProjectsCB = new QCheckBox("Projects");
        showProjectsCB->setObjectName("showProjectsCB");
        showProjectsCB->setChecked(true);

        checkboxLayout->addWidget(showProjectsCB);

        checkboxLayout->addStretch();

        showFilesCB = new QCheckBox("Files");
        showFilesCB->setObjectName("showFilesCB");
        showFilesCB->setChecked(true);

        checkboxLayout->addWidget(showFilesCB);

        checkboxLayout->addStretch();

        searchTreeLayout->addLayout(checkboxLayout);

		searchTree = new CustomTreeWidget(parent);
		searchTree->setObjectName("searchTree");
		searchTree->setColumnCount(2);
		searchTree->setHeaderLabels(QStringList() << "Results" << "Size");
		searchTree->setSelectionMode(QAbstractItemView::SingleSelection);
		searchTree->setContextMenuPolicy(Qt::CustomContextMenu);
		searchTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
		searchTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
		searchTree->header()->setStretchLastSection(false);
		searchTreeLayout->addWidget(searchTree);

        searchTreeWidget->setLayout(searchTreeLayout);

        treeStack->addWidget(searchTreeWidget);

		splitter->addWidget(treeStack);

		projectInfoBox = new CToolBox;
		QWidget* projectDummy = new QWidget;
		QVBoxLayout* modelInfoLayout = new QVBoxLayout;
        modelInfoLayout->setContentsMargins(0,0,0,10);
		projectDummy->setLayout(modelInfoLayout);

		modelInfoLayout->addWidget(unauthorized = new QLabel("<font color='red'>This project has not yet been approved by our "
				"reviewers. It is visible only to you. For now, you may only modify the metadata or delete the project. Once "
				"approved, it will available for all users.</font>"));
		unauthorized->setWordWrap(true);
		unauthorized->hide();

        projectName = new QLabel;
        projectName->setWordWrap(true);
        projectName->setAlignment(Qt::AlignCenter);

		QFont font = projectName->font();
		font.setBold(true);
		font.setPointSize(14);
		projectName->setFont(font);

        modelInfoLayout->addWidget(projectName);

		modelInfoLayout->addWidget(projectDesc = new MultiLineLabel);

		QFrame* line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		modelInfoLayout->addWidget(line);

		projectInfoForm = new QFormLayout;
		projectInfoForm->setHorizontalSpacing(10);
		projectInfoForm->addRow("Owner:", projectOwner = new QLabel);

		modelInfoLayout->addLayout(projectInfoForm);

		modelInfoLayout->addWidget(projectTags = new TagLabel);
		projectTags->setObjectName("projectTags");

		projectInfoBox->addTool("Project Info", projectDummy);

		projectInfoBox->addTool("Publications", projectPubs = new ::CPublicationWidgetView(::CPublicationWidgetView::LIST, false));
		projectInfoBox->getToolItem(1)->hide();

		QWidget* fileDummy = new QWidget;
		QVBoxLayout* fileInfoLayout = new QVBoxLayout;
        fileInfoLayout->setContentsMargins(0,0,0,0);
		fileDummy->setLayout(fileInfoLayout);

		fileInfoForm = new QFormLayout;
		fileInfoForm->setHorizontalSpacing(10);
		fileInfoForm->addRow("Filename:", filenameLabel = new MultiLineLabel);
		fileInfoLayout->addLayout(fileInfoForm);

        fileInfoLayout->addWidget(fileDescLabel = new MultiLineLabel);

		fileInfoLayout->addWidget(fileTags = new TagLabel);
		fileTags->setObjectName("fileTags");

		projectInfoBox->addTool("File Info", fileDummy);
		projectInfoBox->getToolItem(2)->hide();

		splitter->addWidget(projectInfoBox);

		modelVBLayout->addWidget(splitter);

		modelPage = new QWidget;
		modelPage->setLayout(modelVBLayout);

		stack->addWidget(modelPage);

		// Loading Page
		loadingPage = new QWidget;
		QVBoxLayout* loadingLayout = new QVBoxLayout;
		loadingLayout->setAlignment(Qt::AlignCenter);

		loadingLayout->addWidget(loadingLabel = new QLabel);
		loadingLayout->setAlignment(loadingLabel, Qt::AlignCenter);
		loadingLayout->addWidget(loadingBar = new QProgressBar);
		loadingLayout->addWidget(loadingCancel = new QPushButton("Cancel"));
		loadingCancel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
		loadingLayout->setAlignment(loadingCancel, Qt::AlignCenter);

		loadingPage->setLayout(loadingLayout);
		stack->addWidget(loadingPage);

		// setLoginVisible(true);
	}

	CustomTreeWidgetItem* addFile(QString &path, int index, int fileID, bool localCopy, qint64 size, qint64 uploadTime, qint64 downloadTime)
	{
		int pos = path.right(path.length() - index).indexOf("/");

		if(pos == -1)
		{
			FileItem* child = new FileItem(path.right(path.length() - index), fileID, localCopy, size, uploadTime, downloadTime);

			fileItemsByID[fileID] = child;

			return child;
		}

		CustomTreeWidgetItem* child = addFile(path, index + (pos + 1), fileID, localCopy, size, uploadTime, downloadTime);
		CustomTreeWidgetItem* parent;

		try
		{
			parent = currentProjectFolders.at(path.left(pos + index).toStdString());
		}
		catch(std::out_of_range& e)
		{
			parent = new FolderItem(path.right(path.length() - index).left(pos));

			currentProjectFolders[path.left(pos + index).toStdString()] = parent;
		}

		parent->addChild(child);

		return parent;
	}

	void showLoadingPage(QString message, bool progress = false)
	{
		loadingLabel->setText(message);

		loadingBar->setVisible(progress);
		loadingBar->setValue(0);
		loadingCancel->setVisible(progress);

		stack->setCurrentIndex(2);
	}

	void setProjectTags()
	{
		if(currentTags.isEmpty())
		{
			projectTags->hide();
		}
		else
		{
			projectTags->show();
			projectTags->setTagList(currentTags);
		}
	}

	void setFileDescription(QString description)
	{
        if(description.isEmpty())
        {
            fileDescLabel->hide();
        }
        else
        {
            fileDescLabel->setText(description);
            fileDescLabel->show();
        }
	}

	void setFileTags()
	{
		if(currentFileTags.isEmpty())
		{
			fileTags->hide();
		}
		else
		{
			fileTags->show();
			fileTags->setTagList(currentFileTags);
		}
	}

	void selectProjectByID(int ID)
	{
		for(int cat = 0; cat < projectTree->topLevelItemCount(); cat++)
		{
			QTreeWidgetItem* current = projectTree->topLevelItem(cat);
			for(int child = 0; child < current->childCount(); child++)
			{
				QTreeWidgetItem* childItem = current->child(child);
				if(childItem->type() == PROJECTITEM)
				{
					if(ID == static_cast<ProjectItem*>(childItem)->getProjectID())
					{
						projectTree->setCurrentItem(childItem);
						childItem->setExpanded(true);
						return;
					}
				}
			}
		}

	}

public:
	ProjectItem* currentProject;
	std::unordered_map<std::string, CustomTreeWidgetItem*> currentProjectFolders;
	QStringList currentTags;
	QStringList currentFileTags;
	std::unordered_map<int, ProjectItem*> projectItemsByID;
	std::unordered_map<int, FileItem*> fileItemsByID;
	QByteArray* projectInfo;

	CustomTreeWidgetItem* openAfterDownload;
};

#endif