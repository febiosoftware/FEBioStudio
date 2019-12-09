#include "DatabasePanel.h"

#ifdef MODEL_REPO
#include <QBoxLayout>
#include <QToolButton>
#include <QStackedLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValueRef>
#include "DatabaseHandler.h"
#include "MainWindow.h"
//#include "PropertyListForm.h"
//#include "PropertyList.h"

#include <iostream>

struct Project
{
	int id;
	int version;
	QString name;
	QString description;
	QString owner;
	QStringList files;
	std::vector<int> fileIDs;
	int fileID;
};

//class StackedWidget : public QStackedWidget
//{
//public:
//	StackedWidget(QWidget* parent) : QStackedWidget(parent) {}
//private:
//	QSize sizeHint() const override
//	{
//		return parentWidget()->sizeHint();
//	}
//
//	QSize minimumSizeHint() const override
//	{
//		return parentWidget()->minimumSizeHint();
//	}
//
//};

class ProjectTreeWidgetItem : public QTreeWidgetItem
{
public:
	ProjectTreeWidgetItem(const QStringList& strings) : QTreeWidgetItem(strings, 1001) {}

	void setProject(Project project) {m_project = project;}
	Project& getProject() {return m_project;}

private:
	Project m_project;
};

class Ui::CDatabasePanel
{
public:
	QStackedLayout* stack;

	QWidget* loginPage;
	QLabel* loginLabel;
	QFormLayout* loginForm;
	QLineEdit*	userName;
	QLineEdit*	password;
	QPushButton* loginButton;

	QWidget* modelPage;
	QTreeWidget* treeWidget;

	QLabel* projectName;
	QLabel* projectDesc;
	QLabel* projectOwner;

//	::CPropertyListForm* plView;

//	std::vector<Project> projects;
//	CDataPropertyList test;


public:
	void setupUi(QWidget* parent)
	{
//		QVBoxLayout* mainLayout = new QVBoxLayout(parent);

		stack = new QStackedLayout(parent);
//		mainLayout->addLayout(stack);

		// Login Page
		QVBoxLayout* loginVBLayout = new QVBoxLayout;
		loginVBLayout->setAlignment(Qt::AlignCenter);

		loginLabel = new QLabel("To access the model database, please login with your FEBio.org username and password.");
		loginLabel->setWordWrap(true);
		loginLabel->setAlignment(Qt::AlignHCenter);
		loginVBLayout->addWidget(loginLabel);

		loginForm = new QFormLayout;
		loginForm->addRow("Username:", userName = new QLineEdit);
		loginForm->addRow("Password:", password = new QLineEdit);
		password->setEchoMode(QLineEdit::Password);
		loginVBLayout->addLayout(loginForm);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addStretch();
		loginButton = new QPushButton("Login");
		loginButton->setObjectName("loginButton");
		buttonLayout->addWidget(loginButton);
		buttonLayout->addStretch();
		loginVBLayout->addLayout(buttonLayout);

		loginPage = new QWidget;
		loginPage->setLayout(loginVBLayout);

		stack->addWidget(loginPage);

		// Model view page
		QVBoxLayout* modelVBLayout = new QVBoxLayout;

		treeWidget = new QTreeWidget;
		treeWidget->setObjectName("treeWidget");
		treeWidget->setColumnCount(1);
		treeWidget->setHeaderLabel("Project Database");
		treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
		modelVBLayout->addWidget(treeWidget);

		QVBoxLayout* modelInfoLayout = new QVBoxLayout;

		modelVBLayout->addLayout(modelInfoLayout);
		modelInfoLayout->addWidget(projectName = new QLabel);
		modelInfoLayout->addWidget(projectDesc = new QLabel);
		modelInfoLayout->addWidget(projectOwner = new QLabel);

		modelPage = new QWidget;
		modelPage->setLayout(modelVBLayout);


		stack->addWidget(modelPage);



//		plView = new ::CPropertyListForm;

//		mainLayout->addWidget(plView);

	}

	void addProject(Project& project)
	{
		ProjectTreeWidgetItem* projectItem = new ProjectTreeWidgetItem(QStringList(project.name));

		projectItem->setProject(project);

		for(QString file : project.files)
		{
			projectItem->addChild(new QTreeWidgetItem(QStringList(file)));
		}

		treeWidget->addTopLevelItem(projectItem);
	}

	void updateCurrentProject(Project* project)
	{
		projectName->setText(project->name);
		projectDesc->setText(project->description);
		projectOwner->setText(project->owner);
	}

//	void updateProjectList()
//	{
//		for(Project project : projects)
//		{
//			ProjectTreeWidgetItem* projectItem = new ProjectTreeWidgetItem(QStringList(project.name));
//
//			projectItem->setProject(project);
//
//			for(QString file : project.files)
//			{
//				projectItem->addChild(new QTreeWidgetItem(QStringList(file)));
//			}
//
//			treeWidget->addTopLevelItem(projectItem);
//		}
//	}
};

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent) : QWidget(parent), m_wnd(pwnd), ui(new Ui::CDatabasePanel)
{
	// build Ui
	ui->setupUi(this);

	dbHandler = new CDatabaseHandler(this, pwnd);

//	ui->test = CDataPropertyList();
//	bool testBool = true;
//	ui->test.addBoolProperty(&testBool, "test");
//	ui->plView->setPropertyList(&ui->test);

	QMetaObject::connectSlotsByName(this);
	QObject::connect(ui->treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(displayProjectData()));
}

CDatabasePanel::~CDatabasePanel()
{
	delete dbHandler;
	delete ui;
}

void CDatabasePanel::SetModelList(QJsonDocument& jsonDoc)
{
//	ui->projects.clear();

	QJsonArray jsonProjects = jsonDoc.array();
	for(QJsonValueRef jsonProject : jsonProjects)
	{
//		ui->projects.push_back(Project());

		Project project; //= &ui->projects[ui->projects.size() - 1];

		QJsonObject projectObj = jsonProject.toObject();

		project.id = projectObj.value("id").toInt();
		project.name = projectObj.value("name").toString();

		project.description = projectObj.value("description").toString();
		project.owner = projectObj.value("owner").toString();
		project.version = projectObj.value("version").toInt();
		project.fileID = projectObj.value("fileID").toInt();

		QJsonArray files = projectObj.value("files").toArray();
		for(QJsonValueRef file : files)
		{
			project.files.append(file.toString());
		}

		QJsonArray fileIDs = projectObj.value("fileIDs").toArray();
		for(QJsonValueRef fileID : fileIDs)
		{
			project.fileIDs.push_back(fileID.toInt());
		}

		ui->addProject(project);
	}

//	ui->updateProjectList();

	ui->stack->setCurrentIndex(1);
}



void CDatabasePanel::on_loginButton_clicked()
{
	dbHandler->authenticate(ui->userName->text(), ui->password->text());
}

void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
//	dbHandler->getFile(ui->projects[ui->treeWidget->indexOfTopLevelItem(item)].fileIDs[0]);

	ProjectTreeWidgetItem* projItem;

	if(item->type() == 1001)
	{
		projItem = static_cast<ProjectTreeWidgetItem*>(item);
	}
	else
	{
		projItem = static_cast<ProjectTreeWidgetItem*>(item->parent());
	}

	dbHandler->getFile(projItem->getProject().fileID);

//	dbHandler->getFile(1);
}

void CDatabasePanel::displayProjectData()
{
	QTreeWidgetItem* item = ui->treeWidget->selectedItems()[0];

	ProjectTreeWidgetItem* projItem;

	if(item->type() == 1001)
	{
		projItem = static_cast<ProjectTreeWidgetItem*>(item);
	}
	else
	{
		projItem = static_cast<ProjectTreeWidgetItem*>(item->parent());
	}

	ui->updateCurrentProject(&projItem->getProject());

}

#else

CDatabasePanel::CDatabasePanel(CMainWindow* pwnd, QWidget* parent){}
CDatabasePanel::~CDatabasePanel(){}
void CDatabasePanel::SetModelList(QJsonDocument& jsonDoc){}
void CDatabasePanel::on_loginButton_clicked(){}
void CDatabasePanel::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column){}
void CDatabasePanel::displayProjectData(){}

#endif







