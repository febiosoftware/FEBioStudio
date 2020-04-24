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

class Ui::CDlgNew
{
public:
	::CMainWindow*	m_wnd;

	// user choice for opening file:
	// 0 = open from recent file list
	// 1 = open another 
	int m_userChoice;

	int	m_ntemplate;
	QListWidget*	list;
	QTabWidget*		tab;
	QListWidget*	recentFilesList;
	QLineEdit*		projectFolder;
	QLineEdit*		projectName;
	QCheckBox*		createFolder;
	QCheckBox*		showOnStart;

public:
	void setup(::CMainWindow* wnd, QDialog* dlg)
	{
		m_wnd = wnd;

		m_ntemplate = 0;
		m_userChoice = 0;

		tab = new QTabWidget;

		QWidget* newProject = new QWidget;

		list = new QListWidget;
		QStackedWidget* s = new QStackedWidget;

		int ntemp = TemplateManager::Templates();
		for (int i = 0; i<ntemp; ++i)
		{
			const DocTemplate& doc = TemplateManager::GetTemplate(i);
			QLabel* label = new QLabel;
			label->setWordWrap(true);
			label->setText(QString("<h3>%1</h3><p>%2</p>").arg(doc.title.c_str()).arg(doc.description.c_str()));
			label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
			list->addItem(doc.title.c_str());
			s->addWidget(label);
		}

		list->setCurrentRow(0);

		QHBoxLayout* h = new QHBoxLayout;
		h->addWidget(list);
		h->addWidget(s);

		projectFolder = new QLineEdit;

		QToolButton* tb = new QToolButton;
		tb->setIcon(QIcon(":/icons/open.png"));
		tb->setToolTip(QString("Select the project folder."));

		QHBoxLayout* selectFolder = new QHBoxLayout;
		selectFolder->addWidget(projectFolder);
		selectFolder->addWidget(tb);
		selectFolder->setMargin(0);
		selectFolder->setSpacing(1);

		QFormLayout* f = new QFormLayout;
		f->addRow("Project name:"  , projectName   = new QLineEdit);
		f->addRow("Project folder:", selectFolder);

		projectName->setText("MyProject");

		QVBoxLayout* v = new QVBoxLayout;
		v->addLayout(h);
		v->addLayout(f);

		createFolder = new QCheckBox("Create project folder");
		v->addWidget(createFolder);
		createFolder->setChecked(true);

		newProject->setLayout(v);

		tab->addTab(newProject, "New Project");

		QWidget* openProject = new QWidget;
		QVBoxLayout* l = new QVBoxLayout;
		l->setMargin(0);
		recentFilesList = new QListWidget;
		QStringList recentFiles = wnd->GetRecentFileList();
		recentFilesList->addItems(recentFiles);

		l->addWidget(recentFilesList);

		QHBoxLayout* h2 = new QHBoxLayout;
		h2->setMargin(0);
		h2->addStretch();
		QPushButton* open = new QPushButton("Open another project ...");
		open->setObjectName("open");
		h2->addWidget(open);
		l->addLayout(h2);

		openProject->setLayout(l);

		tab->addTab(openProject, "Open Project");

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addWidget(tab);

		QHBoxLayout* bl = new QHBoxLayout;
		bl->addWidget(showOnStart = new QCheckBox("Show this dialog at startup")); showOnStart->setChecked(true);
		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		bl->addWidget(bb);

		mainLayout->addLayout(bl);

		dlg->setLayout(mainLayout);

		// Make the project name the focus
		projectName->setFocus();

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(list, SIGNAL(currentRowChanged(int)), s, SLOT(setCurrentIndex(int)));
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(recentFilesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
		QObject::connect(tb, SIGNAL(clicked()), dlg, SLOT(onProjectFolder()));
		QObject::connect(open, SIGNAL(clicked()), dlg, SLOT(onOpenClicked()));
	}
};

CDlgNew::CDlgNew(CMainWindow* parent ) : QDialog(parent), ui(new Ui::CDlgNew)
{
	setWindowTitle("Project Manager");
	ui->setup(parent, this);
}

void CDlgNew::showEvent(QShowEvent* ev)
{
	QList<QScreen*> screenList = QGuiApplication::screens();
	QRect screenGeometry = screenList.at(0)->geometry();
	int x = (screenGeometry.width() - width()) / 2;
	int y = (screenGeometry.height() - height()) / 2;
	move(x, y);
}

void CDlgNew::onOpenClicked()
{
	ui->m_userChoice = 1;
	accept();
}

bool CDlgNew::showOnStart()
{
	return ui->showOnStart->isChecked();
}

void CDlgNew::setShowOnStart(bool b)
{
	ui->showOnStart->setChecked(b);
}

void CDlgNew::accept()
{
	ui->m_ntemplate = ui->list->currentIndex().row();

	if (ui->projectName->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must enter a project name.");
		return;
	}

	if (ui->projectFolder->text().isEmpty())
	{
		QMessageBox::critical(this, "FEBio Studio", "You must select a valid project folder.");
		return;
	}

	// make sure the project folder exists
	QString projectFolder = ui->projectFolder->text();
	QDir dir(projectFolder);
	if (dir.exists() == false)
	{
		QMessageBox::critical(this, "FEBio Studio", QString("The folder \"%1\" does not exist.\nPlease choose a valid project folder.").arg(projectFolder));
		return;
	}

	QDialog::accept();
}

int CDlgNew::getTemplate()
{ 
	return ui->m_ntemplate;
}

bool CDlgNew::createNew()
{
	return (ui->tab->currentIndex() == 0);
}

bool CDlgNew::openRecentFile()
{
	return ((ui->tab->currentIndex() == 1) && (ui->m_userChoice == 0));
}

QString CDlgNew::getProjectName()
{
	return ui->projectName->text();
}

QString CDlgNew::getProjectFolder()
{
	return ui->projectFolder->text();
}

void CDlgNew::setProjectFolder(const QString& projectFolder)
{
	QString dir(projectFolder);
#ifdef WIN32
	dir.replace("/", "\\");
#endif
	ui->projectFolder->setText(dir);
}

QString CDlgNew::getRecentFileName()
{
	QListWidgetItem* item = ui->recentFilesList->currentItem();
	if (item) return item->text();
	else return QString();
}

void CDlgNew::onProjectFolder()
{
	QFileDialog dlg(this, "Project Folder");
	dlg.setFileMode(QFileDialog::Directory);
	dlg.setAcceptMode(QFileDialog::AcceptOpen);

	QString path = ui->projectFolder->text();
	if (path.isEmpty() == false) dlg.setDirectory(path);
	if (dlg.exec())
	{
		// get the file name
		QStringList files = dlg.selectedFiles();
		QString path = files.first();

		setProjectFolder(path);
	}
}

bool CDlgNew::createProjectFolder()
{
	return ui->createFolder->isChecked();
}
